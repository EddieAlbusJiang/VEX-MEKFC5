#include "x-drive.h"
#include "calc.h"
#include "odometry.h"
#include "pid-controller.h"
#include <HiDebug.h>
#include <algorithm>
#include <cmath>

namespace {
// Wheel speed (pct) below which a wheel is considered stopped.
constexpr double kBrakeVelEps = 0.5;

// Symmetric clamp to [-limit, limit].
double clampSym(double value, double limit) {
    if (value > limit) {
        return limit;
    }
    if (value < -limit) {
        return -limit;
    }
    return value;
}

// Shared PID controllers for the auto routines. Constructed once and reused
// across the (possibly multi-step) autonomous sequence; reset() before each run.
PIDController pid_move_forward(PIDParam{P_AUTO_MOVE_PID_KP, P_AUTO_MOVE_PID_KD, P_AUTO_MOVE_PID_KI});
PIDController pid_move_lateral(PIDParam{P_AUTO_MOVE_PID_KP, P_AUTO_MOVE_PID_KD, P_AUTO_MOVE_PID_KI});
PIDController pid_rot(PIDParam{P_AUTO_ROT_PID_KP, P_AUTO_ROT_PID_KD, P_AUTO_ROT_PID_KI});
} // namespace

// std::string doubleToString(double value, unsigned int precision) {
//     if (std::isnan(value)) return "NaN";
//     if (std::isinf(value)) return value > 0 ? "Infinity" : "-Infinity";

//     // Precision + sign + integer part + '.' + '\0' + safety margin
//     const size_t bufSize = precision + 64;
//     std::vector<char> buf(bufSize);
//     snprintf(buf.data(), bufSize, "%.*f",
//                   static_cast<int>(precision), value);
//     return std::string(buf.data());
// }

XDrive::XDrive(vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2, Odometry& odom,
               double forward_gain, double lateral_gain, double rotational_gain)
    : l1_(l1), l2_(l2), r1_(r1), r2_(r2), odom_(odom), forward_gain_(forward_gain),
      lateral_gain_(lateral_gain), rotational_gain_(rotational_gain) {}

void XDrive::setGains(double forward_gain, double lateral_gain, double rotational_gain) {
    forward_gain_ = forward_gain;
    lateral_gain_ = lateral_gain;
    rotational_gain_ = rotational_gain;
}

void XDrive::setIdleBrake(bool enabled) { idle_brake_ = enabled; }

void XDrive::setOutputInf(double min_output, double hysteresis) {
    min_output_ = min_output;
    hysteresis_ = hysteresis;
}

void XDrive::setOutputSup(double max_output) {
    max_output_ = std::max(0.0, std::min(max_output, 1.0)); // clamp to [0, 1]
}

double XDrive::applyMotor(vex::motor& motor, bool& held, double output) {
    if (!idle_brake_) {
        held = false;
        motor.spin(vex::directionType::fwd, output, vex::velocityUnits::pct);
        return output;
    }

    const double magnitude = std::fabs(output);
    if (held) {
        // Already held: only resume once the output clears the hysteresis band.
        if (magnitude >= min_output_ + hysteresis_) {
            held = false;
            motor.spin(vex::directionType::fwd, output, vex::velocityUnits::pct);
            return output;
        }
        motor.stop(vex::brakeType::hold);
        return 0.0;
    }
    // Spinning: engage the hold once the output drops below the threshold.
    if (magnitude < min_output_) {
        held = true;
        motor.stop(vex::brakeType::hold);
        return 0.0;
    }
    motor.spin(vex::directionType::fwd, output, vex::velocityUnits::pct);
    return output;
}

void XDrive::drive(double forward, double lateral, double rotational) {
    const double f = forward * forward_gain_;
    const double l = lateral * lateral_gain_;
    const double r = rotational * rotational_gain_;

    // Linear superposition of the three base motions:
    //   forward   : ( +1, +1, +1, +1 )
    //   right     : ( +1, -1, -1, +1 )
    //   clockwise : ( +1, +1, -1, -1 )
    const double output_l1 = f + l + r;
    const double output_l2 = f - l + r;
    const double output_r1 = f - l - r;
    const double output_r2 = f + l - r;

    // Scale all outputs down equally if any wheel exceeds full speed so the
    // resulting motion direction is preserved instead of being clipped.
    const double peak = std::max({std::fabs(output_l1), std::fabs(output_l2), std::fabs(output_r1), std::fabs(output_r2)});
    double scale = (peak > 100.0) ? (100.0 / peak) : 1.0;
    scale *= max_output_; // speed cap

    last_output_[0] = applyMotor(l1_, held_[0], output_l1 * scale);
    last_output_[1] = applyMotor(l2_, held_[1], output_l2 * scale);
    last_output_[2] = applyMotor(r1_, held_[2], output_r1 * scale);
    last_output_[3] = applyMotor(r2_, held_[3], output_r2 * scale);

    // Update the braked/stationary flag from the measured wheel speeds.
    braking_ = std::fabs(l1_.velocity(vex::velocityUnits::pct)) < kBrakeVelEps &&
               std::fabs(l2_.velocity(vex::velocityUnits::pct)) < kBrakeVelEps &&
               std::fabs(r1_.velocity(vex::velocityUnits::pct)) < kBrakeVelEps &&
               std::fabs(r2_.velocity(vex::velocityUnits::pct)) < kBrakeVelEps;
    // Debug::Log("[x-drive]: scale = " + doubleToString(scale, 2), Debug::LogLevel::Info);
    printf("%.2f %.2f %.2f %.2f %.2f \n", scale, output_l1, output_l2, output_r1, output_r2);
}

void XDrive::stop(vex::brakeType type) {
    l1_.stop(type);
    l2_.stop(type);
    r1_.stop(type);
    r2_.stop(type);
    // Forget the hold state so the next drive() re-evaluates from scratch.
    for (bool& held : held_) {
        held = false;
    }
    for (double& output : last_output_) {
        output = 0.0;
    }
    // Brake/hold leaves the wheels stationary; coast does not.
    braking_ = (type != vex::brakeType::coast);
}

double XDrive::wheelOutput(int index) const {
    if (index < 0 || index >= 4) {
        return 0.0;
    }
    return last_output_[index];
}

bool XDrive::isBraking() const { return braking_; }

void XDrive::runMove(double target_x, double target_y, double speed) {
    const double entry_heading = odom_.heading();

    pid_move_forward.reset();
    pid_move_lateral.reset();
    pid_rot.reset();

    const double limit = std::fabs(speed);
    vex::timer timer;
    vex::timer settle;
    while (timer.time() < P_AUTO_TIMEOUT_MS) {
        const double ex = target_x - odom_.wheelX();
        const double ey = target_y - odom_.wheelY();

        // Rotate the field-frame error into the body frame.
        const double th = odom_.heading() * (M_PI / 180.0);
        const double c = std::cos(th);
        const double s = std::sin(th);
        const double bf = ex * c + ey * s;
        const double bl = -ex * s + ey * c;

        pid_move_forward.update(bf);
        pid_move_lateral.update(bl);
        pid_rot.update(degNorm180(entry_heading - odom_.heading()));

        const double fwd = clampSym(pid_move_forward.output(), limit);
        const double lat = clampSym(pid_move_lateral.output(), limit);
        const double rot = clampSym(pid_rot.output(), limit);

        drive(fwd, lat, rot);

        if (std::fabs(ex) < P_AUTO_POS_TOL && std::fabs(ey) < P_AUTO_POS_TOL) {
            if (settle.time() >= P_AUTO_SETTLE_MS) {
                break;
            }
        } else {
            settle.reset();
        }
        vex::this_thread::sleep_for(10);
    }

    stop(vex::brakeType::hold);
}

void XDrive::runRotate(double target_deg, double speed) {
    pid_rot.reset();

    const double limit = std::fabs(speed);
    vex::timer timer;
    vex::timer settle;
    while (timer.time() < P_AUTO_TIMEOUT_MS) {
        const double herr = degNorm180(target_deg - odom_.heading());
        pid_rot.update(herr);
        const double rot = clampSym(pid_rot.output(), limit);
        drive(0.0, 0.0, rot);

        if (std::fabs(herr) < P_AUTO_ANGLE_TOL) {
            if (settle.time() >= P_AUTO_SETTLE_MS) {
                break;
            }
        } else {
            settle.reset();
        }
        vex::this_thread::sleep_for(10);
    }

    stop(vex::brakeType::hold);
}

void XDrive::AutoMoveTo(double target_x, double target_y, double speed) { runMove(target_x, target_y, speed); }

void XDrive::AutoMoveFor(double diff_x, double diff_y, double speed) {
    runMove(odom_.wheelX() + diff_x, odom_.wheelY() + diff_y, speed);
}

void XDrive::AutoRotateTo(double target_deg, double speed) { runRotate(target_deg, speed); }

void XDrive::AutoRotateFor(double diff_deg, double speed) { runRotate(odom_.heading() + diff_deg, speed); }


