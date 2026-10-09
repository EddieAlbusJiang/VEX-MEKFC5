#include "clamp.h"

#include <algorithm>

Clamp::Clamp(vex::motor& motor, double open_angle, double close_angle, double speed)
    : motor_(motor), open_angle_(open_angle), close_angle_(close_angle) {
    // The clamp must hold at intermediate positions, so always brake.
    motor_.setStopping(vex::brakeType::hold);
    setSpeed(speed);
}

double Clamp::position() const { return motor_.position(vex::rotationUnits::deg); }

bool Clamp::isOpen() const { return position() <= open_angle_; }
bool Clamp::isClosed() const { return position() >= close_angle_; }

void Clamp::moveBy(int direction) {
    if (direction < 0) {
        if (position() <= close_angle_) {
                    printf("[Clamp] close fail\n");
            stop();
            return;
        }
        motor_.spin(vex::directionType::rev, speed_, vex::velocityUnits::pct);
        printf("[Clamp] close\n");
    } else {
        if (position() >= open_angle_) {
                    printf("[Clamp] open fail\n");
            stop();
            return;
        }
        motor_.spin(vex::directionType::fwd, speed_, vex::velocityUnits::pct);
        printf("[Clamp] open\n");
    }
}

void Clamp::close() { moveBy(-1); }
void Clamp::open() { moveBy(+1); }

void Clamp::stop() { motor_.stop(vex::brakeType::brake); printf("[Clamp] stop\n");}

void Clamp::resetPosition() { motor_.resetPosition(); }

void Clamp::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }
