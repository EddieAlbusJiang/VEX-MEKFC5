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

// Switch the auto PIDs to seconds-based time (so kD is meaningful). Runs once.
void ensureAutoPidUnits() {
    static bool done = false;
    if (!done) {
        pid_move_forward.setUseSeconds(true);
        pid_move_lateral.setUseSeconds(true);
        pid_rot.setUseSeconds(true);
        done = true;
    }
}
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

XDrive* XDrive::s_instance_ = nullptr;
constexpr int XDrive::kQueueSize;

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

    ensureAutoPidUnits();
    pid_move_forward.reset();
    pid_move_lateral.reset();
    pid_rot.reset();

    const double limit = std::fabs(speed);
    vex::timer timer;
    vex::timer settle;
    static int dbg = 0;
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

        double fwd = clampSym(pid_move_forward.output(), limit);
        double lat = clampSym(pid_move_lateral.output(), limit);
        const double rot = clampSym(pid_rot.output(), limit);

        // At the target: hold translation (no crawl during settle); heading
        // correction still applies. Otherwise enforce a minimum translation
        // magnitude so the final approach does not slow to a crawl.
        const bool at_target = (std::fabs(ex) < P_AUTO_POS_TOL && std::fabs(ey) < P_AUTO_POS_TOL);
        if (at_target) {
            fwd = 0.0;
            lat = 0.0;
        } else {
            const double m = std::sqrt(fwd * fwd + lat * lat);
            if (m > 1e-6 && m < P_AUTO_MIN_OUTPUT) {
                const double k = P_AUTO_MIN_OUTPUT / m;
                fwd = clampSym(fwd * k, limit);
                lat = clampSym(lat * k, limit);
            }
        }

        // TEMP diagnostic (console), throttled.
        if (++dbg % 5 == 0) {
            printf("[AUTO] ex=%.1f ey=%.1f bf=%.1f bl=%.1f fwd=%.1f lat=%.1f rot=%.1f h=%.1f\n",
                   ex, ey, bf, bl, fwd, lat, rot, odom_.heading());
        }

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
    ensureAutoPidUnits();
    pid_rot.reset();

    const double limit = std::fabs(speed);
    vex::timer timer;
    vex::timer settle;
    static int dbg = 0;
    while (timer.time() < P_AUTO_TIMEOUT_MS) {
        const double herr = degNorm180(target_deg - odom_.heading());
        pid_rot.update(herr);
        const double rot = clampSym(pid_rot.output(), limit);

        // TEMP diagnostic (console), throttled.
        // if (++dbg % 5 == 0) {
        //     printf("[AUTO] tdeg=%.1f h=%.1f herr=%.1f rot=%.1f\n", target_deg, odom_.heading(), herr, rot);
        // }

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

void XDrive::runFastMove(double dx, double dy) {
    const double sx = odom_.wheelX();
    const double sy = odom_.wheelY();
    const double D = std::sqrt(dx * dx + dy * dy);
    if (D < P_AUTO_POS_TOL) {
        stop(vex::brakeType::hold);
        return;
    }

    // Direction and magnitude are computed once (world frame -> body frame,
    // using the current heading). The motors are commanded only once below;
    // the loop just polls the odometry, so the chassis does not chatter.
    const double th = odom_.heading() * (M_PI / 180.0);
    const double c = std::cos(th);
    const double s = std::sin(th);
    double bf = dx * c + dy * s;
    double bl = -dx * s + dy * c;
    const double m = std::max(std::fabs(bf), std::fabs(bl));
    if (m > 1e-6) {
        const double k = 100.0 / m; // max command; drive() then caps by max_output_
        bf *= k;
        bl *= k;
    }

    drive(bf, bl, 0.0); // single command (no heading hold; rot = 0)

    vex::timer timer;
    while (timer.time() < P_AUTO_TIMEOUT_MS) {
        const double rx = (sx + dx) - odom_.wheelX(); // remaining (world frame)
        const double ry = (sy + dy) - odom_.wheelY();
        const double proj = (rx * dx + ry * dy) / D;  // remaining along the target direction
        if (proj <= P_AUTO_POS_TOL) {                 // reached / passed -> stop
            break;
        }
        vex::this_thread::sleep_for(10);
    }
    stop(vex::brakeType::hold);
}

void XDrive::AutoMoveTo(double target_x, double target_y, double speed) { blockingAuto(AutoKind::MoveTo, target_x, target_y, speed); }

void XDrive::AutoMoveFor(double diff_x, double diff_y, double speed) { blockingAuto(AutoKind::MoveFor, diff_x, diff_y, speed); }

void XDrive::AutoRotateTo(double target_deg, double speed) { blockingAuto(AutoKind::RotateTo, target_deg, 0.0, speed); }

void XDrive::AutoRotateFor(double diff_deg, double speed) { blockingAuto(AutoKind::RotateFor, diff_deg, 0.0, speed); }

void XDrive::AutoFastMoveFor(double diff_x, double diff_y) { blockingAuto(AutoKind::FastFor, diff_x, diff_y, 0.0); }

// ---------------------------------------------------------------------------
// Non-blocking auto: a per-object worker thread drains a fixed-size command
// queue. Blocking overloads enqueue and wait on a ticket; non-blocking overloads
// enqueue (dropping when full) and return immediately.
// NOTE: do NOT call a blocking Auto from inside a callback (would deadlock the
// worker thread that runs the callback).
// ---------------------------------------------------------------------------

void XDrive::workerTrampoline() { s_instance_->autoWorkerLoop(); }

void XDrive::ensureAutoWorker() {
    if (auto_worker_started_) {
        return;
    }
    auto_queue_mutex_.lock();
    if (!auto_worker_started_) {
        s_instance_ = this;
        new vex::thread(&XDrive::workerTrampoline); // detached; runs for program lifetime
        auto_worker_started_ = true;
    }
    auto_queue_mutex_.unlock();
}

int XDrive::enqueueAuto(const AutoCmd& cmd) {
    int ticket = -1;
    auto_queue_mutex_.lock();
    if (auto_count_ < kQueueSize) {
        auto_queue_[auto_tail_] = cmd;
        auto_tail_ = (auto_tail_ + 1) % kQueueSize;
        ++auto_count_;
        ticket = ++auto_enqueued_;
    }
    auto_queue_mutex_.unlock();
    return ticket;
}

void XDrive::blockingAuto(AutoKind kind, double a, double b, double speed) {
    ensureAutoWorker();
    const AutoCmd cmd{kind, a, b, speed, nullptr};
    int ticket = -1;
    do {
        ticket = enqueueAuto(cmd);
        if (ticket < 0) {
            vex::this_thread::sleep_for(10);
        }
    } while (ticket < 0);
    while (auto_completed_ < ticket) {
        vex::this_thread::sleep_for(10);
    }
}

void XDrive::autoWorkerLoop() {
    while (true) {
        AutoCmd cmd{};
        bool have = false;
        auto_queue_mutex_.lock();
        if (auto_count_ > 0) {
            cmd = auto_queue_[auto_head_];
            auto_head_ = (auto_head_ + 1) % kQueueSize;
            --auto_count_;
            have = true;
        }
        auto_queue_mutex_.unlock();

        if (!have) {
            vex::this_thread::sleep_for(10);
            continue;
        }

        switch (cmd.kind) {
            case AutoKind::MoveTo:
                runMove(cmd.a, cmd.b, cmd.speed);
                break;
            case AutoKind::MoveFor:
                runMove(odom_.wheelX() + cmd.a, odom_.wheelY() + cmd.b, cmd.speed);
                break;
            case AutoKind::RotateTo:
                runRotate(cmd.a, cmd.speed);
                break;
            case AutoKind::RotateFor:
                runRotate(odom_.heading() + cmd.a, cmd.speed);
                break;
            case AutoKind::FastFor:
                runFastMove(cmd.a, cmd.b);
                break;
        }

        ++auto_completed_; // release the blocking waiter (if any)
        if (cmd.cb != nullptr) {
            cmd.cb();
        }
    }
}

void XDrive::AutoMoveTo(double target_x, double target_y, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::MoveTo, target_x, target_y, speed, onDone}); // drop if full
}

void XDrive::AutoMoveFor(double diff_x, double diff_y, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::MoveFor, diff_x, diff_y, speed, onDone});
}

void XDrive::AutoRotateTo(double target_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::RotateTo, target_deg, 0.0, speed, onDone});
}

void XDrive::AutoRotateFor(double diff_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::RotateFor, diff_deg, 0.0, speed, onDone});
}


