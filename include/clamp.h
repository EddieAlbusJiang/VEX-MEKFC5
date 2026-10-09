#pragma once

#include "vex.h"

// Geared clamp. A single motor drives a gear set that opens/closes the clamp.
// Travel is bounded by an open and a closed motor-shaft angle, and the clamp
// must be able to lock at any intermediate position, so the motor always brakes
// with brakeType::brake.
class Clamp {
  private:
    vex::motor& motor_;
    double open_angle_;  // open limit (motor degrees)
    double close_angle_; // closed limit (motor degrees)
    double speed_;       // constant rate (percent)

    // Moves at constant speed toward the closed (+1) or open (-1) limit; brakes
    // when the corresponding limit is reached. Assumes open_angle_ < close_angle_.
    void moveBy(int direction);

  public:
    Clamp(vex::motor& motor, double open_angle, double close_angle, double speed);

    double position() const;
    bool isOpen() const;
    bool isClosed() const;

    void open();
    void close();
    void stop();          // always brakes with brakeType::brake
    void resetPosition(); // current angle becomes zero

    // Set the constant rate (percent), clamped to [0, 100].
    void setSpeed(double speed);
};
