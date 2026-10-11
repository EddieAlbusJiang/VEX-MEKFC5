#pragma once

#include "vex.h"

class HeadingTracker {
  private:
    // combines the inertial sensor using reference
    vex::inertial& inertial_;
    // stores the coefficient to convert the inertial sensor's rotation value to a more accurate value
    const double coefficient_; // 3600.0 / 3544.0
  public:
    // use an inertial sensor and a coefficient to create a heading tracker object
    HeadingTracker(vex::inertial& inertial, double coefficient);

    // gets the rotation value from the inertial sensor and multiplies it by the coefficient to get a more accurate value
    double rotation();
    // gets the heading value from the inertial sensor and normalizes it to be between 0 and 360 degrees
    double heading();
    // resets the rotation value of the inertial sensor to 0
    void resetRotation();
    // calibrates the inertial sensor and returns true if the calibration is successful, false otherwise
    bool calibrate();
};
