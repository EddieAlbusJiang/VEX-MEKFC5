#pragma once

#include "vex.h"

// PID parameter set, ordered (kP, kD, kI).
struct PIDParam {
    double kP;
    double kD;
    double kI;
};

// Minimal reusable PID controller. Feed an error each control cycle and read the
// output. The time delta is measured internally in milliseconds.
class PIDController {
  private:
    PIDParam params_;

    vex::timer timer_;
    int prev_time_ = 0; // ms
    double prev_error_ = 0.0;
    double error_integral_ = 0.0;
    double output_ = 0.0;
    // If true, dt is measured in seconds (D/I use per-second units); default ms.
    bool use_seconds_ = false;

  public:
    explicit PIDController(PIDParam params);

    void setGains(PIDParam params);
    // Select the time unit for D/I: false = milliseconds (default), true = seconds.
    void setUseSeconds(bool use);
    // Clears integral/derivative/output state and re-baselines the timer. Call
    // before starting a new control run (or when leaving the active region).
    void reset();
    void update(double error);
    double output() const;
};
