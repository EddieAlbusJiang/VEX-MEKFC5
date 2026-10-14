#include "odometry.h"
#include "calc.h"
#include "HiDebug.h"
#include "instance.h"

#include <cmath>

namespace {
constexpr double kGravity = 980.665;    // cm/s^2 per g
constexpr double kAccelLpf = 0.2;       // first-order low-pass on world accel
constexpr double kStillAccelEps = 0.03; // g, allowed deviation of |a| from 1g
constexpr double kStillGyroEps = 2.0;   // deg/s
constexpr int kStillFrames = 10;        // consecutive still frames before ZUPT
constexpr int kOdomPeriodMs = 10;       // background update period
constexpr int kCalibSamples = 50;       // acceleration bias samples
constexpr int kCalibPeriodMs = 10;      // acceleration bias sample period
} // namespace

Odometry::Odometry(vex::inertial& inertial, double coefficient,
                   vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2,
                   double wheel_diameter_cm, double motor_to_wheel_ratio)
    : inertial_(inertial), coefficient_(coefficient), l1_(l1), l2_(l2), r1_(r1), r2_(r2),
      cm_per_deg_((M_PI * wheel_diameter_cm) /
                  (360.0 * ((motor_to_wheel_ratio == 0.0) ? 1.0 : motor_to_wheel_ratio))) {}

double Odometry::rotation() { return inertial_.rotation() * coefficient_; }
double Odometry::heading() { return degNorm360(rotation()); }
void Odometry::resetRotation() { inertial_.resetRotation(); }

bool Odometry::calibrate() {
    inertial_.calibrate();
    waitUntil(!inertial_.isCalibrating());
    vex::wait(200, vex::timeUnits::msec);
    const double rotation_200 = inertial_.rotation();
    vex::wait(800, vex::timeUnits::msec);
    const double rotation_1000 = inertial_.rotation();
    const double rotation_diff = rotation_1000 - rotation_200;
    Debug::Log("[Odometry] calibration rotation_diff: " + Debug::doubleToString(rotation_diff, 3),
               Debug::LogLevel::Debug);
    const bool stable = std::fabs(rotation_diff) < 0.05;
    if (!stable) {
        Debug::Log("[Odometry] calibration failed (sensor drifting)", Debug::LogLevel::Warning);
    }
    return stable;
}

void Odometry::calibrateImuBias() {
    double sx = 0.0;
    double sy = 0.0;
    for (int i = 0; i < kCalibSamples; ++i) {
        sx += inertial_.acceleration(vex::axisType::xaxis);
        sy += inertial_.acceleration(vex::axisType::yaxis);
        vex::this_thread::sleep_for(kCalibPeriodMs);
    }
    imu_bias_x_g_ = sx / kCalibSamples;
    imu_bias_y_g_ = sy / kCalibSamples;
    Debug::Log("[Odometry] accel bias: x=" + Debug::doubleToString(imu_bias_x_g_, 3) +
                   " y=" + Debug::doubleToString(imu_bias_y_g_, 3),
               Debug::LogLevel::Info);
}

void Odometry::updateImu() {
    const int now = imu_timer_.time();
    if (!imu_ready_) {
        imu_last_time_ = now;
        imu_ready_ = true;
        return;
    }
    const double dt = static_cast<double>(now - imu_last_time_) / 1000.0; // s
    imu_last_time_ = now;
    if (dt <= 0.0) {
        return;
    }

    const double ax_g = inertial_.acceleration(vex::axisType::xaxis);
    const double ay_g = inertial_.acceleration(vex::axisType::yaxis);
    const double az_g = inertial_.acceleration(vex::axisType::zaxis);

    // Remove the at-rest horizontal bias before integrating.
    const double axc = ax_g - imu_bias_x_g_;
    const double ayc = ay_g - imu_bias_y_g_;

    // Rotate the body-frame horizontal acceleration into the power-up frame.
    const double th = rotation() * (M_PI / 180.0);
    const double c = std::cos(th);
    const double s = std::sin(th);
    double wax = (axc * c + ayc * s) * kGravity;
    double way = (-axc * s + ayc * c) * kGravity;

    // Low-pass filter to tame noise before integrating.
    wax = kAccelLpf * wax + (1.0 - kAccelLpf) * imu_prev_wax_;
    way = kAccelLpf * way + (1.0 - kAccelLpf) * imu_prev_way_;

    // Trapezoidal integration: velocity then position.
    imu_vx_ += 0.5 * (imu_prev_wax_ + wax) * dt;
    imu_vy_ += 0.5 * (imu_prev_way_ + way) * dt;
    imu_px_ += imu_vx_ * dt;
    imu_py_ += imu_vy_ * dt;

    imu_prev_wax_ = wax;
    imu_prev_way_ = way;

    // Zero-velocity update: snap velocity to zero when clearly stationary.
    const double a_mag = std::sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
    const double gyro = std::fabs(inertial_.gyroRate(vex::axisType::zaxis, vex::velocityUnits::dps));
    if (std::fabs(a_mag - 1.0) < kStillAccelEps && gyro < kStillGyroEps) {
        if (imu_still_frames_ < kStillFrames) {
            ++imu_still_frames_;
        }
    } else {
        imu_still_frames_ = 0;
    }
    if (imu_still_frames_ >= kStillFrames) {
        imu_vx_ = 0.0;
        imu_vy_ = 0.0;
    }
}

double Odometry::imuX() const { return imu_px_; }
double Odometry::imuY() const { return imu_py_; }

void Odometry::resetImu() {
    imu_timer_.reset();
    imu_last_time_ = 0;
    imu_ready_ = false;
    imu_vx_ = 0.0;
    imu_vy_ = 0.0;
    imu_px_ = 0.0;
    imu_py_ = 0.0;
    imu_prev_wax_ = 0.0;
    imu_prev_way_ = 0.0;
    imu_still_frames_ = 0;
}

void Odometry::updateWheel() {
    const double d[4] = {
        l1_.position(vex::rotationUnits::deg),
        l2_.position(vex::rotationUnits::deg),
        r1_.position(vex::rotationUnits::deg),
        r2_.position(vex::rotationUnits::deg),
    };

    if (!wheel_ready_) {
        for (int i = 0; i < 4; ++i) {
            wheel_prev_[i] = d[i];
        }
        wheel_ready_ = true;
        return;
    }

    const double dd[4] = {
        d[0] - wheel_prev_[0],
        d[1] - wheel_prev_[1],
        d[2] - wheel_prev_[2],
        d[3] - wheel_prev_[3],
    };
    for (int i = 0; i < 4; ++i) {
        wheel_prev_[i] = d[i];
    }

    // X-drive inverse kinematics: body-frame forward (f) and right (l) travel.
    const double f = (dd[0] + dd[1] + dd[2] + dd[3]) / 4.0 * cm_per_deg_;
    const double l = (dd[0] - dd[1] - dd[2] + dd[3]) / 4.0 * cm_per_deg_;

    // Rotate into the power-up frame.
    const double th = rotation() * (M_PI / 180.0);
    const double c = std::cos(th);
    const double s = std::sin(th);
    wheel_px_ += f * c - l * s;
    wheel_py_ += f * s + l * c;
}

double Odometry::wheelX() const { return wheel_px_; }
double Odometry::wheelY() const { return wheel_py_; }

void Odometry::resetWheel() {
    wheel_ready_ = false;
    wheel_px_ = 0.0;
    wheel_py_ = 0.0;
    for (int i = 0; i < 4; ++i) {
        wheel_prev_[i] = 0.0;
    }
}

void odometryThread() {
    while (true) {
        Odom.updateImu();
        Odom.updateWheel();
        vex::this_thread::sleep_for(kOdomPeriodMs);
    }
}
