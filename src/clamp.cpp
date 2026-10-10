#include "clamp.h"

#include <algorithm>
#include <cmath>

Clamp::Clamp(vex::motor& motor, double open_angle, double close_angle, double speed)
    : motor_(motor), open_angle_(open_angle), close_angle_(close_angle),
      pid_(PIDParam{1.0, 0.05, 0.0}) {
    // The clamp must hold at intermediate positions, so always hold.
    motor_.setStopping(vex::brakeType::hold);
    setSpeed(speed);
}

double Clamp::position() const { return motor_.position(vex::rotationUnits::deg); }

bool Clamp::isOpen() const { return position() <= open_angle_; }
bool Clamp::isClosed() const { return position() >= close_angle_; }

void Clamp::moveBy(int direction) {
    // Advance the recorded target by the distance travelled in one tick,
    // converting the percent speed into motor-shaft degrees.
    const double delta_deg = direction * (speed_ / 100.0) * 3600 * (TICK_MS / 1000.0) * 1.5;

    if (direction < 0) {
        if (position() <= close_angle_) {
            printf("[Clamp] close fail\n");
            locked_position_ = position();
            stop();
            return;
        }
        locked_position_ = position() + delta_deg;
        motor_.spin(vex::directionType::rev, speed_, vex::velocityUnits::pct);
        printf("[Clamp] close\n");
    } else {
        if (position() >= open_angle_) {
            printf("[Clamp] open fail\n");
            locked_position_ = position();
            stop();
            return;
        }
        locked_position_ = position() + delta_deg;
        motor_.spin(vex::directionType::fwd, speed_, vex::velocityUnits::pct);
        printf("[Clamp] open\n");
    }
}

void Clamp::close() { moveBy(-1); }
void Clamp::open() { moveBy(+1); }

void Clamp::stop() {
    const double current = position();
    const double error = locked_position_ - current;

    const bool within_tolerance = std::fabs(error) <= tolerance_;
    // Do not drive further past a limit while trying to reach the target.
    const bool at_open_limit = error > 0.0 && current >= open_angle_;
    const bool at_close_limit = error < 0.0 && current <= close_angle_;

    if (!within_tolerance && !at_open_limit && !at_close_limit) {
        if (!locking_) {
            pid_.reset();
            locking_ = true;
        }
        pid_.update(error);

        // Compensation is capped at the normal motion speed.
        double output = pid_.output();
        if (output > speed_) {
            output = speed_;
        } else if (output < -speed_) {
            output = -speed_;
        }

        // Larger angle = open (fwd), smaller angle = close (rev).
        if (output >= 0.0) {
            motor_.spin(vex::directionType::fwd, output, vex::velocityUnits::pct);
        } else {
            motor_.spin(vex::directionType::rev, -output, vex::velocityUnits::pct);
        }
    } else {
        if (locking_) {
            pid_.reset();
            locking_ = false;
        }
        motor_.stop(vex::brakeType::hold);
        printf("[Clamp] stop\n");
    }
}

void Clamp::resetPosition() {
    motor_.resetPosition();
    locked_position_ = 0.0;
    pid_.reset();
    locking_ = false;
}

void Clamp::setSpeed(double speed) { speed_ = std::max(0.0, std::min(speed, 100.0)); }

void Clamp::setLockTolerance(double tolerance) { tolerance_ = std::fabs(tolerance); }

void Clamp::setLockGains(double kP, double kD, double kI) { pid_.setGains(PIDParam{kP, kD, kI}); }
