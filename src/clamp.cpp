#include "clamp.h"
#include "calc.h"
#include "mechanism-motion.h"
#include "HiDebug.h"
#include "instance.h"

#include <algorithm>
#include <cmath>

Clamp* Clamp::s_instance_ = nullptr;
constexpr int Clamp::kQueueSize;

Clamp::Clamp(vex::motor& motor, double open_angle, double close_angle, double speed)
    : motor_(motor), open_angle_(open_angle), close_angle_(close_angle),
      base_close_angle_(close_angle),
      pid_(PIDParam{P_CLAMP_PID_KP, P_CLAMP_PID_KD, P_CLAMP_PID_KI}) {
    // The clamp must hold at intermediate positions, so always hold.
    motor_.setStopping(vex::brakeType::hold);
    pid_.setUseSeconds(true); // seconds-based dt so kD is meaningful (damping)
    setSpeed(speed);
}

double Clamp::position() const { return motor_.position(vex::rotationUnits::deg); }

bool Clamp::isOpen() const { return position() >= open_angle_ - tolerance_; }
bool Clamp::isClosed() const { return position() <= close_angle_ + tolerance_; }

void Clamp::moveBy(int direction) {
    // Advance the recorded target by the distance travelled in one tick,
    // converting the percent speed into motor-shaft degrees.
    const double delta_deg = direction * (speed_ / 100.0) * 3600 * (TICK_MS / 1000.0) * 1.5;

    if (direction < 0) {
        if (!Root_Mode && position() + delta_deg <= close_angle_) {
            locked_position_ = close_angle_;
            stop();
            return;
        }
        locked_position_ = position() + delta_deg;
        motor_.spin(vex::directionType::rev, speed_, vex::velocityUnits::pct);
    } else {
        if (!Root_Mode && position() + delta_deg >= open_angle_) {
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
    const bool at_open_limit = !Root_Mode && error > 0.0 && current >= open_angle_;
    const bool at_close_limit = !Root_Mode && error < 0.0 && current <= close_angle_;

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

void Clamp::assumeOpen() {
    motor_.setPosition(open_angle_, vex::rotationUnits::deg); // current angle = max (open)
    locked_position_ = open_angle_;                            // keep the lock target in sync
    pid_.reset();
    locking_ = false;
}

void Clamp::setCloseAngle(double angle) {
    // Never shallower than the predefined close angle (guards bad calibration).
    close_angle_ = std::min(angle, base_close_angle_);
    // Setter only: do not command the motor here.
    locked_position_ = close_angle_;
    pid_.reset();
    locking_ = false;
}

void Clamp::calibrateCloseAngle() {
    Debug::Log("[Clamp] close-angle calibration start", Debug::LogLevel::Info);

    const double prev_speed = speed_;
    setSpeed(P_CLAMP_CALIB_SPEED);
    vex::timer timer;
    while (timer.time() < P_CLAMP_CALIB_MS) {
        // Drive toward close at a slow rate, ignoring the close limit.
        motor_.spin(vex::directionType::rev, speed_, vex::velocityUnits::pct);
        vex::this_thread::sleep_for(10);
    }

    const double measured = position();
    setCloseAngle(measured + P_CLAMP_CLOSE_CALIB_OFFSET); // set the close limit only
    motor_.stop(vex::brakeType::hold);
    setSpeed(prev_speed);

    // Open back to open_angle_ (calibration drives the clamp; setCloseAngle does not).
    runToAngle(open_angle_, speed_);
    locked_position_ = open_angle_;
    pid_.reset();
    locking_ = false;

    Debug::Log("[Clamp] close-angle calibration done: measured=" + Debug::doubleToString(measured, 2) +
                   " offset=" + Debug::doubleToString(P_CLAMP_CLOSE_CALIB_OFFSET, 1) +
                   " close=" + Debug::doubleToString(close_angle_, 2),
               Debug::LogLevel::Info);
}

void Clamp::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }

void Clamp::setLockTolerance(double tolerance) { tolerance_ = std::fabs(tolerance); }

void Clamp::setLockGains(double kP, double kD, double kI) { pid_.setGains(PIDParam{kP, kD, kI}); }

void Clamp::runToAngle(double target_deg, double speed, bool loaded_grip) {
    const double requested_target = target_deg;
    target_deg = std::max(close_angle_, std::min(target_deg, open_angle_));
    speed = std::max(0.0, std::min(speed, 100.0));
    // Relax only the full-open endpoint; closing/intermediate targets retain
    // the original tolerance and the encoder reference is unchanged.
    const double tolerance = std::fabs(target_deg - open_angle_) < 1e-6 ?
        P_CLAMP_AUTO_OPEN_TOL : P_CLAMP_AUTO_TOL;
    vex::timer timer;
    const GripContactRule grip{
        target_deg - tolerance,
        base_close_angle_ + P_CLAMP_GRIP_MAX_POSITION,
        P_CLAMP_GRIP_MIN_TRAVEL,
        P_CLAMP_GRIP_STABLE_DEG,
        P_CLAMP_GRIP_SETTLE_MS
    };
    bool contact_reached = false;
    const bool allow_contact = loaded_grip && target_deg <= base_close_angle_;
    auto_result_ = AutoResult::Running;
    auto_result_ = runMechanismPosition(motor_, target_deg, speed,
                                        tolerance, P_CLAMP_AUTO_TIMEOUT_MS,
                                        allow_contact ? &grip : nullptr, &contact_reached);
    locked_position_ = position();
    locking_ = false;
    pid_.reset();
    const auto level = auto_result_ == AutoResult::Reached ?
        Debug::LogLevel::Info : Debug::LogLevel::Warning;
    Debug::Log("[Clamp] " + std::string(contact_reached ? "CONTACT" : autoResultName(auto_result_)) + " " +
                   Debug::doubleToString(timer.time(), 0) + "ms tol=" +
                   Debug::doubleToString(tolerance, 1), level);
    Debug::Log("[Clamp] req=" + Debug::doubleToString(requested_target, 1) +
                   " T=" + Debug::doubleToString(target_deg, 1) +
                   " Cp=" + Debug::doubleToString(position(), 1), level);
}

AutoResult Clamp::AutoClampTo(double target_deg, double speed) { blockingAuto(AutoKind::To, target_deg, speed); return auto_result_; }

AutoResult Clamp::AutoClampFor(double delta_deg, double speed) { blockingAuto(AutoKind::For, delta_deg, speed); return auto_result_; }

AutoResult Clamp::AutoClampOpen(double speed) { blockingAuto(AutoKind::Open, 0.0, speed); return auto_result_; }

AutoResult Clamp::AutoClampClose(double speed) { blockingAuto(AutoKind::Close, 0.0, speed); return auto_result_; }

AutoResult Clamp::AutoClampGrip(double target_deg, double speed) { blockingAuto(AutoKind::Grip, target_deg, speed); return auto_result_; }

// ---------------------------------------------------------------------------
// Auto command queue + worker thread (see x-drive.cpp for the same pattern).
// Blocking overloads enqueue and wait (ticket); non-blocking overloads enqueue
// (waiting for queue space if full) and return. NOTE: do NOT call a blocking Auto from
// inside a callback (deadlock).
// ---------------------------------------------------------------------------

AutoResult Clamp::waitUntilDone() {
    auto_queue_mutex_.lock();
    const int ticket = auto_enqueued_;
    auto_queue_mutex_.unlock();
    while (auto_completed_ < ticket) {
        vex::this_thread::sleep_for(10);
    }
    return auto_result_;
}

void Clamp::workerTrampoline() { s_instance_->autoWorkerLoop(); }

void Clamp::ensureAutoWorker() {
    if (auto_worker_started_) {
        return;
    }
    auto_queue_mutex_.lock();
    if (!auto_worker_started_) {
        s_instance_ = this;
        new vex::thread(&Clamp::workerTrampoline);
        auto_worker_started_ = true;
    }
    auto_queue_mutex_.unlock();
}

int Clamp::enqueueAuto(const AutoCmd& cmd) {
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

void Clamp::blockingAuto(AutoKind kind, double a, double speed) {
    ensureAutoWorker();
    const AutoCmd cmd{kind, a, 0.0, speed, nullptr};
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

void Clamp::autoWorkerLoop() {
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
            case AutoKind::To:
                runToAngle(cmd.a, cmd.speed);
                break;
            case AutoKind::Grip:
                runToAngle(cmd.a, cmd.speed, true);
                break;
            case AutoKind::For:
                runToAngle(position() + cmd.a, cmd.speed);
                break;
            case AutoKind::Open:
                runToAngle(open_angle_, cmd.speed);
                break;
            case AutoKind::Close:
                runToAngle(close_angle_, cmd.speed);
                break;
        }

        ++auto_completed_;
        if (cmd.cb != nullptr) {
            cmd.cb();
        }
    }
}

void Clamp::AutoClampTo(double target_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::To, target_deg, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}

void Clamp::AutoClampFor(double delta_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::For, delta_deg, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}

void Clamp::AutoClampOpen(void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::Open, 0.0, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}

void Clamp::AutoClampClose(void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::Close, 0.0, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}
