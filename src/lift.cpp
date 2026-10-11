#include "lift.h"

#include <algorithm>
#include <utility>

Lift::Lift(vex::motor_group& motors, double min_angle, double max_angle, double speed)
    : motors_(motors), min_angle_(min_angle), max_angle_(max_angle) {
    if (min_angle_ > max_angle_) {
        std::swap(min_angle_, max_angle_);
    }
    // The load must be able to hold position, so always brake.
    motors_.setStopping(vex::brakeType::brake);
    setSpeed(speed);
}

double Lift::position() const { return motors_.position(vex::rotationUnits::deg); }

bool Lift::atBottom() const { return position() <= min_angle_; }
bool Lift::atTop() const { return position() >= max_angle_; }

void Lift::moveBy(int direction) {
    if (direction > 0) {
        if (position() >= max_angle_) {
            stop();
            return;
        }
        motors_.spin(vex::directionType::fwd, speed_, vex::velocityUnits::pct);
    } else {
        if (position() <= min_angle_) {
            stop();
            return;
        }
        motors_.spin(vex::directionType::fwd, -speed_, vex::velocityUnits::pct);
    }
}

void Lift::up() { moveBy(+1); }
void Lift::down() { moveBy(-1); }

void Lift::stop() { motors_.stop(vex::brakeType::hold); }

void Lift::resetPosition() { motors_.resetPosition(); }

void Lift::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }
