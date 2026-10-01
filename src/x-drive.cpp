#include "x-drive.h"

#include <algorithm>
#include <cmath>

XDrive::XDrive(vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2,
               double forward_gain, double lateral_gain, double rotational_gain)
    : l1_(l1), l2_(l2), r1_(r1), r2_(r2), forward_gain_(forward_gain),
      lateral_gain_(lateral_gain), rotational_gain_(rotational_gain) {}

void XDrive::setGains(double forward_gain, double lateral_gain, double rotational_gain) {
    forward_gain_ = forward_gain;
    lateral_gain_ = lateral_gain;
    rotational_gain_ = rotational_gain;
}

void XDrive::drive(double forward, double lateral, double rotational) {
    const double f = forward * forward_gain_;
    const double l = lateral * lateral_gain_;
    const double r = rotational * rotational_gain_;

    // Linear superposition of the three base motions:
    //   forward   : ( +1, +1, +1, +1 )
    //   right     : ( +1, -1, -1, +1 )
    //   clockwise : ( +1, +1, -1, -1 )
    const double output_l1 = f + l + r;
    const double output_l2 = f - l + r;
    const double output_r1 = f - l - r;
    const double output_r2 = f + l - r;

    // Scale all outputs down equally if any wheel exceeds full speed so the
    // resulting motion direction is preserved instead of being clipped.
    const double peak = std::max({std::fabs(output_l1), std::fabs(output_l2), std::fabs(output_r1), std::fabs(output_r2)});
    const double scale = (peak > 100.0) ? (100.0 / peak) : 1.0;

    l1_.spin(vex::directionType::fwd, output_l1 * scale, vex::velocityUnits::pct);
    l2_.spin(vex::directionType::fwd, output_l2 * scale, vex::velocityUnits::pct);
    r1_.spin(vex::directionType::fwd, output_r1 * scale, vex::velocityUnits::pct);
    r2_.spin(vex::directionType::fwd, output_r2 * scale, vex::velocityUnits::pct);
}

void XDrive::stop(vex::brakeType type) {
    l1_.stop(type);
    l2_.stop(type);
    r1_.stop(type);
    r2_.stop(type);
}
