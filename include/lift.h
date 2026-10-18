#pragma once

#include "vex.h"
#include "auto-motion.h"

// Winch-driven lift. Two synchronized motors turn a winch that raises/lowers the
// load on a rope. The load rests at the bottom and is manually reset before each
// power-up, so the motor position is zeroed at init; the travel is bounded by a
// minimum and maximum motor-shaft angle. Manual motion uses a constant speed;
// automatic motion uses native position control, then brakeType::hold.
class Lift {
  private:
    vex::motor_group& motors_;
    double min_angle_; // bottom limit (motor degrees)
    double max_angle_; // top limit (motor degrees)
    double speed_;     // constant rate (percent)
    volatile AutoResult auto_result_ = AutoResult::Idle;

    // Moves at constant speed toward +max (direction > 0) or -min (direction < 0);
    // brakes when the corresponding limit is reached.
    void moveBy(int direction);

    // Native position move with arrival, stall, timeout and cancellation checks.
    void runToAngle(double target_deg, double speed);

    // ----- Non-blocking auto: command queue + worker thread -----
    enum class AutoKind { To, For, Up, Down };
    struct AutoCmd {
        AutoKind kind;
        double a;
        double b;
        double speed;
        void (*cb)(void);
    };
    static constexpr int kQueueSize = P_AUTO_QUEUE;
    AutoCmd auto_queue_[kQueueSize];
    int auto_head_ = 0;
    int auto_tail_ = 0;
    int auto_count_ = 0;
    int auto_enqueued_ = 0;
    volatile int auto_completed_ = 0;
    bool auto_worker_started_ = false;
    vex::mutex auto_queue_mutex_;
    static Lift* s_instance_;
    void ensureAutoWorker();
    void autoWorkerLoop();
    int enqueueAuto(const AutoCmd& cmd);
    void blockingAuto(AutoKind kind, double a, double speed);
    static void workerTrampoline();

  public:
    Lift(vex::motor_group& motors, double min_angle, double max_angle, double speed);

    double position() const;
    AutoResult autoResult() const { return auto_result_; }
    // Includes queued work, so a previous command's result cannot imply ready.
    bool autoDone();
    AutoResult waitUntilDone();
    bool atBottom() const;
    bool atTop() const;

    void up();
    void down();
    void stop();          // always holds with brakeType::hold
    void resetPosition(); // current angle becomes zero

    // ----- Autonomous helpers (blocking; move to an angle, then hold) -----
    AutoResult AutoLiftTo(double target_deg, double speed = P_LIFT_AUTO_SPEED);
    AutoResult AutoLiftFor(double delta_deg, double speed = P_LIFT_AUTO_SPEED);
    AutoResult AutoLiftUp(double speed = P_LIFT_AUTO_SPEED);
    AutoResult AutoLiftDown(double speed = P_LIFT_AUTO_SPEED);

    // ----- Non-blocking variants (enqueue and return; onDone may be null) -----
    void AutoLiftTo(double target_deg, void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
    void AutoLiftFor(double delta_deg, void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
    void AutoLiftUp(void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);
    void AutoLiftDown(void (*onDone)(void), double speed = P_LIFT_AUTO_SPEED);

    // Set the constant rate (percent), clamped to [0, 100].
    void setSpeed(double speed);
};
