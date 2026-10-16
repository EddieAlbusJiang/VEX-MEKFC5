#include "clamp.h"
#include "calc.h"
#include "instance.h"

#include <algorithm>
#include <cmath>

Clamp::Clamp(vex::motor& motor, double open_angle, double close_angle, double speed)
    : motor_(motor), open_angle_(open_angle), close_angle_(close_angle),
      pid_(PIDParam{P_CLAMP_PID_KP, P_CLAMP_PID_KD, P_CLAMP_PID_KI}) {
    // The clamp must hold at intermediate positions, so always hold.
    motor_.setStopping(vex::brakeType::hold);
    pid_.setUseSeconds(true); // seconds-based dt so kD is meaningful (damping)
    setSpeed(speed);
}

double Clamp::position() const { return motor_.position(vex::rotationUnits::deg); }

bool Clamp::isOpen() const { return position() <= open_angle_; }
bool Clamp::isClosed() const { return position() >= close_angle_; }

void Clamp::moveBy(int direction) {
    // Advance the recorded target by the distance travelled in one tick,
    // converting the percent speed into motor-shaft degrees.
    const double delta_deg = direction * (speed_ / 100.0) * 3600 * (TICK_MS / 1000.0) * 1.5;

    if (direction < 0) {
        if (!Debug_LimitsOff && position() + delta_deg <= close_angle_) {
            locked_position_ = close_angle_;
            stop();
            return;
        }
        locked_position_ = position() + delta_deg;
        motor_.spin(vex::directionType::rev, speed_, vex::velocityUnits::pct);
    } else {
        if (!Debug_LimitsOff && position() + delta_deg >= open_angle_) {
            locked_position_ = open_angle_;
            stop();
            return;
        }
        locked_position_ = position() + delta_deg;
        motor_.spin(vex::directionType::fwd, speed_, vex::velocityUnits::pct);
    }
}

void Clamp::close() { moveBy(-1); }
void Clamp::open() { moveBy(+1); }

void Clamp::stop() {
    const double current = position();
    const double error = locked_position_ - current;

    const bool within_tolerance = std::fabs(error) <= tolerance_;
    // Do not drive further past a limit while trying to reach the target
    // (unless the debug limits-off mode is enabled).
    const bool at_open_limit = !Debug_LimitsOff && error > 0.0 && current >= open_angle_;
    const bool at_close_limit = !Debug_LimitsOff && error < 0.0 && current <= close_angle_;

    if (!within_tolerance && !at_open_limit && !at_close_limit) {
        if (!locking_) {
            pid_.reset();
            locking_ = true;
        }
        pid_.update(error);

        // Compensation is capped at the normal motion speed.
        double output = pid_.output();
        if (output > speed_) {
            output = speed_;
        } else if (output < -speed_) {
            output = -speed_;
        }

        // Larger angle = open (fwd), smaller angle = close (rev).
        if (output >= 0.0) {
            motor_.spin(vex::directionType::fwd, output, vex::velocityUnits::pct);
        } else {
            motor_.spin(vex::directionType::rev, -output, vex::velocityUnits::pct);
        }
    } else {
        if (locking_) {
            pid_.reset();
            locking_ = false;
        }
        motor_.stop(vex::brakeType::hold);
    }
}

void Clamp::resetPosition() {
    motor_.resetPosition();
    locked_position_ = 0.0;
    pid_.reset();
    locking_ = false;
}

void Clamp::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }

void Clamp::setLockTolerance(double tolerance) { tolerance_ = std::fabs(tolerance); }

void Clamp::setLockGains(double kP, double kD, double kI) { pid_.setGains(PIDParam{kP, kD, kI}); }

void Clamp::runToAngle(double target_deg, double speed) {
    // Clamp the target within the reachable range.
    target_deg = std::max(close_angle_, std::min(target_deg, open_angle_));

    const double prev_speed = speed_;
    setSpeed(speed);

    vex::timer timer;
    vex::timer settle;
    bool holding = false;
    while (timer.time() < P_MECH_TIMEOUT_MS) {
        const double pos = position();
        const double err = target_deg - pos;
        const double mag = std::fabs(err);

        if (mag <= tolerance_) {
            holding = true;
            motor_.stop(vex::brakeType::hold);
            if (settle.time() >= P_MECH_SETTLE_MS) {
                break;
            }
        } else if (holding && mag <= tolerance_ + P_MECH_HYSTERESIS) {
            // Within the hysteresis band while holding: stay put (no chatter).
            motor_.stop(vex::brakeType::hold);
        } else if ((err > 0.0 && pos >= open_angle_) || (err < 0.0 && pos <= close_angle_)) {
            motor_.stop(vex::brakeType::hold); // blocked by a limit before reaching target
            break;
        } else {
            holding = false;
            settle.reset();
            const double cmd = approachSpeed(speed_, err, P_MECH_DECEL_TOL, P_MECH_MIN_OUTPUT);
            if (err > 0.0) {
                motor_.spin(vex::directionType::fwd, cmd, vex::velocityUnits::pct);
            } else {
                motor_.spin(vex::directionType::rev, cmd, vex::velocityUnits::pct);
            }
        }
        vex::this_thread::sleep_for(10);
    }

    motor_.stop(vex::brakeType::hold);
    locked_position_ = position(); // keep the lock consistent with the new position
    locking_ = false;
    setSpeed(prev_speed);
}

void Clamp::AutoClampTo(double target_deg, double speed) { runToAngle(target_deg, speed); }

void Clamp::AutoClampFor(double delta_deg, double speed) { runToAngle(position() + delta_deg, speed); }

void Clamp::AutoClampOpen(double speed) { runToAngle(open_angle_, speed); }

void Clamp::AutoClampClose(double speed) { runToAngle(close_angle_, speed); }
