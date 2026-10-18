#include "pid-controller.h"

PIDController::PIDController(PIDParam params) : params_(params) { reset(); }

void PIDController::setGains(PIDParam params) { params_ = params; }

void PIDController::setUseSeconds(bool use) { use_seconds_ = use; }

void PIDController::reset() {
    prev_time_ = timer_.time();
    prev_error_ = 0.0;
    error_integral_ = 0.0;
    output_ = 0.0;
}

void PIDController::update(double error) {
    const int now = timer_.time();
    const double delta_ms = static_cast<double>(now - prev_time_); // ms
    const double dt = use_seconds_ ? (delta_ms / 1000.0) : delta_ms;

    // P: proportional
    const double P = params_.kP * error;

    // D: derivative (guarded against a non-positive time step)
    const double D = (dt > 0.0) ? (params_.kD * (error - prev_error_) / dt) : 0.0;

    // I: integral
    error_integral_ += error * dt;
    const double I = params_.kI * error_integral_;

    output_ = P + D + I;

    prev_time_ = now;
    prev_error_ = error;
}

double PIDController::output() const { return output_; }
