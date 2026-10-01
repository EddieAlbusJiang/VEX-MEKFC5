#pragma once

#include "vex.h"

// Holonomic "X-drive": four drive wheels mounted at 45 degrees with their axes
// pointing toward the robot center. The four wheels can be combined linearly to
// produce translation in any direction plus in-place rotation.
//
// Inputs follow one consistent convention (all positive = to the right / forward):
//   forward    : + drives the robot forward,  - backward
//   lateral    : + drives the robot right,    - left
//   rotational : + rotates the robot clockwise (right turn), - counter-clockwise
class XDrive {
  private:
    vex::motor& l1_;
    vex::motor& l2_;
    vex::motor& r1_;
    vex::motor& r2_;

    // Per-axis gains, applied before mixing. Lets a weak axis be scaled up
    // without touching the mixing math. Default 1.0.
    double forward_gain_;
    double lateral_gain_;
    double rotational_gain_;

  public:
    XDrive(vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2,
           double forward_gain = 1.0, double lateral_gain = 1.0, double rotational_gain = 1.0);

    // Adjust the per-axis gains at runtime.
    void setGains(double forward_gain, double lateral_gain, double rotational_gain);

    // Command the chassis. Inputs are in percent (-100..100). The combined
    // wheel outputs are scaled proportionally so no wheel saturates, preserving
    // the requested direction of travel.
    void drive(double forward, double lateral, double rotational);

    // Stop all drive motors.
    void stop(vex::brakeType type = vex::brakeType::coast);
};
