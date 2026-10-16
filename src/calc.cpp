#include "calc.h"
#include "config.h"

#include <cmath>

int sgn(double val) { return (0 < val) - (val < 0); }

double deadZone(double input, double limit) {
    if (input > limit) {
        return input;
    } else if (input < -limit) {
        return input;
    } else {
        return 0;
    }
}

double map(double input, double in_min, double in_max, double out_min, double out_max) { return (input - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }

double pctToVolt(double pct) {
    // Convert a percentage (0-100) to a voltage (0.3-12.8)
    const int sign = sgn(pct);
    return sign * map(pct, 0, 100, 0.3, 12.8);
}

double degNorm360(double input) {
    while (input >= 360.0) {
        input -= 360.0;
    }
    while (input < 0.0) {
        input += 360.0;
    }
    return input;
}

double degNorm180(double input) {
    while (input >= 180.0) {
        input -= 360.0;
    }
    while (input < -180.0) {
        input += 360.0;
    }
    return input;
}

double approachSpeed(double max_speed, double error, double decel_tol, double min_output) {
    const double mag = std::fabs(error);
    double cmd = max_speed;
    if (decel_tol > 0.0 && mag < decel_tol) {
        cmd = max_speed * (mag / decel_tol);
    }
    if (cmd < min_output) {
        cmd = min_output;
    }
    if (cmd > max_speed) {
        cmd = max_speed;
    }
    return cmd;
}

double driveCurve(double input) {
    const double n = input / 100.0;
    return sgn(n) * std::pow(std::fabs(n), P_DRIVE_CURVE_EXP) * 100.0;
}
