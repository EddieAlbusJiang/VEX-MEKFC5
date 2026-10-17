#pragma once

#include "pid-controller.h"
#include "vex.h"

// Geared clamp. A single motor drives a gear set that opens/closes the clamp.
// Travel is bounded by an open (large angle) and a closed (small angle) limit.
// The clamp can lock at any intermediate position and holds it against drift,
// so stopping always uses brakeType::hold.
class Clamp {
  private:
    vex::motor& motor_;
    double open_angle_;  // open limit (large motor angle)
    double close_angle_; // closed limit (small motor angle)
    const double base_close_angle_; // predefined close angle, used to clamp calibration
    double speed_;       // constant rate (percent)

    // Position lock: while moving, locked_position_ continuously records the
    // current position. On stop(), if the clamp has drifted away from it beyond
    // tolerance_, a PID loop drives it back; otherwise it holds in place.
    double locked_position_ = 0.0;
    double tolerance_ = P_CLAMP_TOLERANCE; // deadband (degrees)
    PIDController pid_;      // position loop, gains from P_CLAMP_PID_*
    bool locking_ = false;   // whether the compensation loop is active

    // Moves at constant speed toward open (+1) or close (-1); records the
    // position and stops (holds) at the corresponding limit.
    void moveBy(int direction);

    // Blocking constant-speed move to a target angle (used by the Auto helpers).
    void runToAngle(double target_deg, double speed);

    // ----- Non-blocking auto: command queue + worker thread -----
    enum class AutoKind { To, For, Open, Close };
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
    static Clamp* s_instance_;
    void ensureAutoWorker();
    void autoWorkerLoop();
    int enqueueAuto(const AutoCmd& cmd);
    void blockingAuto(AutoKind kind, double a, double speed);
    static void workerTrampoline();

  public:
    Clamp(vex::motor& motor, double open_angle, double close_angle, double speed);

    double position() const;
    bool isOpen() const;
    bool isClosed() const;

    void open();
    void close();
    // Holds in place, or compensates back to the locked position if it drifted.
    void stop();
    void resetPosition(); // current angle becomes zero (and clears the lock)
    // Assume the clamp is currently at its maximum (open) angle; used at boot so
    // the position reference matches the mechanically-open state.
    void assumeOpen();

    // Set the closed limit only (clamped so it is never shallower than the
    // predefined P_CLAMP_CLOSE_ANGLE). Does not command the motor.
    void setCloseAngle(double angle);

    // Boot calibration of the close angle: drive slowly toward close for
    // P_CLAMP_CALIB_MS (ignoring the close limit), then set the close angle to
    // (measured position + P_CLAMP_CLOSE_CALIB_OFFSET) and open back up here.
    void calibrateCloseAngle();

    // ----- Autonomous helpers (blocking; move to an angle, then hold) -----
    void AutoClampTo(double target_deg, double speed = P_CLAMP_SPEED);
    void AutoClampFor(double delta_deg, double speed = P_CLAMP_SPEED);
    void AutoClampOpen(double speed = P_CLAMP_SPEED);
    void AutoClampClose(double speed = P_CLAMP_SPEED);

    // ----- Non-blocking variants (enqueue and return; onDone may be null) -----
    void AutoClampTo(double target_deg, void (*onDone)(void), double speed = P_CLAMP_SPEED);
    void AutoClampFor(double delta_deg, void (*onDone)(void), double speed = P_CLAMP_SPEED);
    void AutoClampOpen(void (*onDone)(void), double speed = P_CLAMP_SPEED);
    void AutoClampClose(void (*onDone)(void), double speed = P_CLAMP_SPEED);

    // Set the constant rate (percent), clamped to [0, 100].
    void setSpeed(double speed);
    // Set the position-lock deadband (degrees). Negative input is made positive.
    void setLockTolerance(double tolerance);
    // Set the position-lock PID gains.
    void setLockGains(double kP, double kD, double kI);
};
