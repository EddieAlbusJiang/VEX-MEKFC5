#include "pid-controller.h"

PIDController::PIDController(PIDParam params) : params_(params) { reset(); }

void PIDController::setGains(PIDParam params) { params_ = params; }

void PIDController::reset() {
    prev_time_ = timer_.time();
    prev_error_ = 0.0;
    error_integral_ = 0.0;
    output_ = 0.0;
}

void PIDController::update(double error) {
    const int now = timer_.time();
    const double delta_time = static_cast<double>(now - prev_time_); // ms

    // P: proportional
    const double P = params_.kP * error;

    // D: derivative (guarded against a non-positive time step)
    const double D = (delta_time > 0.0) ? (params_.kD * (error - prev_error_) / delta_time) : 0.0;

    // I: integral
    error_integral_ += error * delta_time;
    const double I = params_.kI * error_integral_;

    output_ = P + D + I;

    prev_time_ = now;
    prev_error_ = error;
}

double PIDController::output() const { return output_; }
