#pragma once

#include "vex.h"

// Winch-driven lift. Two synchronized motors turn a winch that raises/lowers the
// load on a rope. The load rests at the bottom and is manually reset before each
// power-up, so the motor position is zeroed at init; the travel is bounded by a
// minimum and maximum motor-shaft angle. The load moves at a constant rate and
// can hover, so the motors always brake with brakeType::brake.
class Lift {
  private:
    vex::motor_group& motors_;
    double min_angle_; // bottom limit (motor degrees)
    double max_angle_; // top limit (motor degrees)
    double speed_;     // constant rate (percent)

    // Moves at constant speed toward +max (direction > 0) or -min (direction < 0);
    // brakes when the corresponding limit is reached.
    void moveBy(int direction);

    // Blocking constant-speed move to a target angle (used by the Auto helpers).
    void runToAngle(double target_deg, double speed);

  public:
    Lift(vex::motor_group& motors, double min_angle, double max_angle, double speed);

    double position() const;
    bool atBottom() const;
    bool atTop() const;

    void up();
    void down();
    void stop();          // always brakes with brakeType::brake
    void resetPosition(); // current angle becomes zero

    // ----- Autonomous helpers (blocking; move to an angle, then hold) -----
    void AutoLiftTo(double target_deg, double speed = P_LIFT_SPEED);
    void AutoLiftFor(double delta_deg, double speed = P_LIFT_SPEED);
    void AutoLiftUp(double speed = P_LIFT_SPEED);
    void AutoLiftDown(double speed = P_LIFT_SPEED);

    // Set the constant rate (percent), clamped to [0, 100].
    void setSpeed(double speed);
};
