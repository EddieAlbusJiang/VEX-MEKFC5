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

    // Global output cap (0..1) applied after normalization. 0.4 = 40% speed cap.
    double max_output_ = 0.5;

    // Idle-brake configuration. When a wheel's final output stays below
    // min_output_ it is held (braked) instead of spinning near zero; it only
    // resumes once the output rises above min_output_ + hysteresis_. This
    // hysteresis prevents chatter when the command hovers around the threshold.
    double min_output_ = 1.0;
    double hysteresis_ = 1.0;
    bool idle_brake_ = true;
    // Whether each wheel (l1, l2, r1, r2) is currently held.
    bool held_[4] = {false, false, false, false};
    // The effective value last applied to each wheel (l1, l2, r1, r2); zero when
    // a wheel is held by the idle-brake logic.
    double last_output_[4] = {0.0, 0.0, 0.0, 0.0};
    // Whether the drive is effectively braked/stationary (all wheel speeds ~0).
    // Read from other threads, so kept volatile.
    volatile bool braking_ = true;

    // Applies the idle-brake logic to a single motor and returns the effective
    // value applied (zero when held/stopped).
    double applyMotor(vex::motor& motor, bool& held, double output);

  public:
    XDrive(vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2,
           double forward_gain = 1.0, double lateral_gain = 1.0, double rotational_gain = 1.0);

    // Adjust the per-axis gains at runtime.
    void setGains(double forward_gain, double lateral_gain, double rotational_gain);

    // Enable/disable the idle-brake behavior (enabled by default).
    void setIdleBrake(bool enabled);

    // Set the hold threshold (percent) and the hysteresis margin used to resume.
    void setOutputInf(double min_output, double hysteresis = 1.0);

    // Set the global output cap (0..1), clamped to a sane range.
    void setOutputSup(double max_output);

    // The effective output last applied to a wheel: index 0..3 => l1, l2, r1, r2.
    // Returns 0 for out-of-range indices.
    double wheelOutput(int index) const;

    // True when all four wheels are effectively stopped (braked/stationary).
    bool isBraking() const;

    // Command the chassis. Inputs are in percent (-100..100). The combined
    // wheel outputs are scaled proportionally so no wheel saturates, preserving
    // the requested direction of travel.
    void drive(double forward, double lateral, double rotational);

    // Stop all drive motors.
    void stop(vex::brakeType type = vex::brakeType::coast);
};
