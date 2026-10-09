#pragma once

#include "vex.h"

// Winch-driven lift. A motor turns a winch that raises/lowers the load on a
// rope. The load rests at the bottom and is manually reset before each power-up,
// so the motor position is zeroed at init; the travel is bounded by a minimum
// and maximum motor-shaft angle. The load moves at a constant rate and can
// hover, so the motor always brakes with brakeType::brake.
class Lift {
  private:
    vex::motor& motor_;
    double min_angle_; // bottom limit (motor degrees)
    double max_angle_; // top limit (motor degrees)
    double speed_;     // constant rate (percent)

    // Moves at constant speed toward +max (direction > 0) or -min (direction < 0);
    // brakes when the corresponding limit is reached.
    void moveBy(int direction);

  public:
    Lift(vex::motor& motor, double min_angle, double max_angle, double speed);

    double position() const;
    bool atBottom() const;
    bool atTop() const;

    void up();
    void down();
    void stop();          // always brakes with brakeType::brake
    void resetPosition(); // current angle becomes zero

    // Set the constant rate (percent), clamped to [0, 100].
    void setSpeed(double speed);
};
