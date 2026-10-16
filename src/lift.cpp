#include "lift.h"
#include "calc.h"
#include "instance.h"

#include <algorithm>
#include <cmath>
#include <utility>

Lift* Lift::s_instance_ = nullptr;
constexpr int Lift::kQueueSize;

Lift::Lift(vex::motor_group& motors, double min_angle, double max_angle, double speed)
    : motors_(motors), min_angle_(min_angle), max_angle_(max_angle) {
    if (min_angle_ > max_angle_) {
        std::swap(min_angle_, max_angle_);
    }
    // The load must be able to hold position, so always brake.
    motors_.setStopping(vex::brakeType::brake);
    setSpeed(speed);
}

double Lift::position() const { return motors_.position(vex::rotationUnits::deg); }

bool Lift::atBottom() const { return position() <= min_angle_; }
bool Lift::atTop() const { return position() >= max_angle_; }

void Lift::moveBy(int direction) {
    if (direction > 0) {
        if (!Root_Mode && position() >= max_angle_) {
            stop();
            return;
        }
        motors_.spin(vex::directionType::fwd, speed_, vex::velocityUnits::pct);
    } else {
        if (!Root_Mode && position() <= min_angle_) {
            stop();
            return;
        }
        motors_.spin(vex::directionType::fwd, -speed_, vex::velocityUnits::pct);
    }
}

void Lift::up() { moveBy(+1); }
void Lift::down() { moveBy(-1); }

void Lift::stop() { motors_.stop(vex::brakeType::hold); }

void Lift::resetPosition() { motors_.resetPosition(); }

void Lift::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }

void Lift::runToAngle(double target_deg, double speed) {
    // Clamp the target within the reachable range.
    target_deg = std::max(min_angle_, std::min(target_deg, max_angle_));

    const double prev_speed = speed_;
    setSpeed(speed);

    vex::timer timer;
    vex::timer settle;
    bool holding = false;
    while (timer.time() < P_MECH_TIMEOUT_MS) {
        const double pos = position();
        const double err = target_deg - pos;
        const double mag = std::fabs(err);

        if (mag <= P_LIFT_TOL) {
            holding = true;
            motors_.stop(vex::brakeType::hold);
            if (settle.time() >= P_MECH_SETTLE_MS) {
                break;
            }
        } else if (holding && mag <= P_LIFT_TOL + P_MECH_HYSTERESIS) {
            // Within the hysteresis band while holding: stay put (no chatter).
            motors_.stop(vex::brakeType::hold);
        } else if ((err > 0.0 && pos >= max_angle_) || (err < 0.0 && pos <= min_angle_)) {
            motors_.stop(vex::brakeType::hold); // blocked by a limit before reaching target
            break;
        } else {
            holding = false;
            settle.reset();
            const double cmd = approachSpeed(speed_, err, P_MECH_DECEL_TOL, P_MECH_MIN_OUTPUT);
            const double sign = (err > 0.0) ? 1.0 : -1.0;
            motors_.spin(vex::directionType::fwd, sign * cmd, vex::velocityUnits::pct);
        }
        vex::this_thread::sleep_for(10);
    }

    motors_.stop(vex::brakeType::hold);
    setSpeed(prev_speed);
}

void Lift::AutoLiftTo(double target_deg, double speed) { blockingAuto(AutoKind::To, target_deg, speed); }

void Lift::AutoLiftFor(double delta_deg, double speed) { blockingAuto(AutoKind::For, delta_deg, speed); }

void Lift::AutoLiftUp(double speed) { blockingAuto(AutoKind::Up, 0.0, speed); }

void Lift::AutoLiftDown(double speed) { blockingAuto(AutoKind::Down, 0.0, speed); }

// ---------------------------------------------------------------------------
// Auto command queue + worker thread (see x-drive.cpp for the same pattern).
// Blocking overloads enqueue and wait (ticket); non-blocking overloads enqueue
// (dropping when full) and return. NOTE: do NOT call a blocking Auto from
// inside a callback (deadlock).
// ---------------------------------------------------------------------------

void Lift::workerTrampoline() { s_instance_->autoWorkerLoop(); }

void Lift::ensureAutoWorker() {
    if (auto_worker_started_) {
        return;
    }
    auto_queue_mutex_.lock();
    if (!auto_worker_started_) {
        s_instance_ = this;
        new vex::thread(&Lift::workerTrampoline);
        auto_worker_started_ = true;
    }
    auto_queue_mutex_.unlock();
}

int Lift::enqueueAuto(const AutoCmd& cmd) {
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

void Lift::blockingAuto(AutoKind kind, double a, double speed) {
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

void Lift::autoWorkerLoop() {
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
            case AutoKind::For:
                runToAngle(position() + cmd.a, cmd.speed);
                break;
            case AutoKind::Up:
                runToAngle(max_angle_, cmd.speed);
                break;
            case AutoKind::Down:
                runToAngle(min_angle_, cmd.speed);
                break;
        }

        ++auto_completed_;
        if (cmd.cb != nullptr) {
            cmd.cb();
        }
    }
}

void Lift::AutoLiftTo(double target_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::To, target_deg, 0.0, speed, onDone}); // drop if full
}

void Lift::AutoLiftFor(double delta_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::For, delta_deg, 0.0, speed, onDone});
}

void Lift::AutoLiftUp(void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::Up, 0.0, 0.0, speed, onDone});
}

void Lift::AutoLiftDown(void (*onDone)(void), double speed) {
    ensureAutoWorker();
    enqueueAuto(AutoCmd{AutoKind::Down, 0.0, 0.0, speed, onDone});
}
