#include "calc.h"

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
