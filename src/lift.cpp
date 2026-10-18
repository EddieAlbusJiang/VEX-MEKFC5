#include "lift.h"
#include "calc.h"
#include "mechanism-motion.h"
#include "HiDebug.h"
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
    const double requested_target = target_deg;
    target_deg = std::max(min_angle_, std::min(target_deg, max_angle_));
    speed = std::max(0.0, std::min(speed, 100.0));
    vex::timer timer;
    auto_result_ = AutoResult::Running;
    auto_result_ = runMechanismPosition(motors_, target_deg, speed,
                                        P_LIFT_AUTO_TOL, P_LIFT_AUTO_TIMEOUT_MS);
    const auto level = auto_result_ == AutoResult::Reached ?
        Debug::LogLevel::Info : Debug::LogLevel::Warning;
    Debug::Log("[Lift] " + std::string(autoResultName(auto_result_)) + " " +
                   Debug::doubleToString(timer.time(), 0) + "ms", level);
    Debug::Log("[Lift] req=" + Debug::doubleToString(requested_target, 1) +
                   " T=" + Debug::doubleToString(target_deg, 1) +
                   " Lt=" + Debug::doubleToString(position(), 1), level);
}

AutoResult Lift::AutoLiftTo(double target_deg, double speed) { blockingAuto(AutoKind::To, target_deg, speed); return auto_result_; }

AutoResult Lift::AutoLiftFor(double delta_deg, double speed) { blockingAuto(AutoKind::For, delta_deg, speed); return auto_result_; }

AutoResult Lift::AutoLiftUp(double speed) { blockingAuto(AutoKind::Up, 0.0, speed); return auto_result_; }

AutoResult Lift::AutoLiftDown(double speed) { blockingAuto(AutoKind::Down, 0.0, speed); return auto_result_; }

// ---------------------------------------------------------------------------
// Auto command queue + worker thread (see x-drive.cpp for the same pattern).
// Blocking overloads enqueue and wait (ticket); non-blocking overloads enqueue
// (waiting for queue space if full) and return. NOTE: do NOT call a blocking Auto from
// inside a callback (deadlock).
// ---------------------------------------------------------------------------

bool Lift::autoDone() {
    auto_queue_mutex_.lock();
    const bool done = auto_completed_ >= auto_enqueued_;
    auto_queue_mutex_.unlock();
    return done;
}

AutoResult Lift::waitUntilDone() {
    auto_queue_mutex_.lock();
    const int ticket = auto_enqueued_;
    auto_queue_mutex_.unlock();
    while (auto_completed_ < ticket) {
        vex::this_thread::sleep_for(10);
    }
    return auto_result_;
}

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
    while (enqueueAuto(AutoCmd{AutoKind::To, target_deg, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}

void Lift::AutoLiftFor(double delta_deg, void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::For, delta_deg, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}

void Lift::AutoLiftUp(void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::Up, 0.0, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}

void Lift::AutoLiftDown(void (*onDone)(void), double speed) {
    ensureAutoWorker();
    while (enqueueAuto(AutoCmd{AutoKind::Down, 0.0, 0.0, speed, onDone}) < 0) {
        vex::this_thread::sleep_for(10);
    }
}
