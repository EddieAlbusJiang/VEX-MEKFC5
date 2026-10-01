#include "heading-tracker.h"
#include "calc.h"
#include <cmath>

HeadingTracker::HeadingTracker(vex::inertial& inertial, double coefficient)
    : inertial_(inertial), coefficient_(coefficient) {}

double HeadingTracker::rotation() { return inertial_.rotation() * coefficient_; }
double HeadingTracker::heading() { return degNorm360(rotation()); }
void HeadingTracker::resetRotation() { inertial_.resetRotation(); }
bool HeadingTracker::calibrate() {
    inertial_.calibrate();
    waitUntil(!inertial_.isCalibrating());
    vex::wait(200, vex::timeUnits::msec);
    const double rotation_200 = inertial_.rotation();
    vex::wait(800, vex::timeUnits::msec);
    const double rotation_1000 = inertial_.rotation();
    const double rotation_diff = rotation_1000 - rotation_200;
    return std::fabs(rotation_diff) < 0.05;
}
