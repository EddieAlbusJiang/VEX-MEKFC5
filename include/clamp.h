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

    // ----- Autonomous helpers (blocking; move to an angle, then hold) -----
    void AutoClampTo(double target_deg, double speed = P_CLAMP_SPEED);
    void AutoClampFor(double delta_deg, double speed = P_CLAMP_SPEED);
    void AutoClampOpen(double speed = P_CLAMP_SPEED);
    void AutoClampClose(double speed = P_CLAMP_SPEED);

    // Set the constant rate (percent), clamped to [0, 100].
    void setSpeed(double speed);
    // Set the position-lock deadband (degrees). Negative input is made positive.
    void setLockTolerance(double tolerance);
    // Set the position-lock PID gains.
    void setLockGains(double kP, double kD, double kI);
};
