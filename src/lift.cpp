#include "lift.h"

#include <algorithm>
#include <utility>

Lift::Lift(vex::motor& motor, double min_angle, double max_angle, double speed)
    : motor_(motor), min_angle_(min_angle), max_angle_(max_angle) {
    if (min_angle_ > max_angle_) {
        std::swap(min_angle_, max_angle_);
    }
    // The load must be able to hold position, so always brake.
    motor_.setStopping(vex::brakeType::brake);
    setSpeed(speed);
}

double Lift::position() const { return motor_.position(vex::rotationUnits::deg); }

bool Lift::atBottom() const { return position() <= min_angle_; }
bool Lift::atTop() const { return position() >= max_angle_; }

void Lift::moveBy(int direction) {
    if (direction > 0) {
        if (position() >= max_angle_) {
            stop();
            return;
        }
        motor_.spin(vex::directionType::fwd, speed_, vex::velocityUnits::pct);
    } else {
        if (position() <= min_angle_) {
            stop();
            return;
        }
        motor_.spin(vex::directionType::fwd, -speed_, vex::velocityUnits::pct);
    }
}

void Lift::up() { moveBy(+1); }
void Lift::down() { moveBy(-1); }

void Lift::stop() { motor_.stop(vex::brakeType::brake); }

void Lift::resetPosition() { motor_.resetPosition(); }

void Lift::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }
