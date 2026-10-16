#pragma once

// math utility functions

// returns the sign of a number: -1 for negative, 0 for zero, 1 for positive
int sgn(double val);

// returns the input if it is outside the dead zone, otherwise returns 0
double deadZone(double input, double limit);

// maps a value from one range to another
double map(double input, double in_min, double in_max, double out_min, double out_max);

// converts a percentage value to a voltage value (0-100% to 0-12V)
double pctToVolt(double pct);

// normalizes an angle in degrees to the range [-180, 180)
double degNorm180(double input);

// normalizes an angle in degrees to the range [0, 360)
double degNorm360(double input);

// Returns a drive magnitude (0..max_speed) that decelerates linearly within
// decel_tol of the target and never falls below min_output while moving.
double approachSpeed(double max_speed, double error, double decel_tol, double min_output);
