#include "odometry.h"
#include "calc.h"
#include "HiDebug.h"
#include "instance.h"

#include <cmath>

namespace {
#if 0 // IMU displacement disabled
constexpr double kGravity = 980.665;    // cm/s^2 per g
constexpr double kAccelLpf = 0.2;       // first-order low-pass on world accel
constexpr double kStillAccelEps = 0.03; // g, allowed deviation of |a| from 1g
constexpr double kStillGyroEps = 2.0;   // deg/s
constexpr int kStillFrames = 10;        // consecutive still frames before ZUPT
constexpr int kCalibSamples = 50;       // gravity samples while stationary
constexpr int kCalibPeriodMs = 10;      // calibration sample period
// Yaw (forward-drive) calibration is temporarily disabled.
// constexpr double kDriveSpeedPct = 40.0; // forward burst speed during calibration
// constexpr int kDriveMs = 300;           // forward burst duration during calibration
// constexpr double kForwardSignalEps = 0.02; // g; min mean dynamic accel to trust yaw

struct Vec3 {
    double x, y, z;
};

double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

double norm(const Vec3& v) { return std::sqrt(dot(v, v)); }

Vec3 normalize(const Vec3& v) {
    const double n = norm(v);
    if (n < 1e-9) {
        return {0.0, 0.0, 0.0};
    }
    return {v.x / n, v.y / n, v.z / n};
}

Vec3 readAccel(vex::inertial& inertial) {
    return {inertial.acceleration(vex::axisType::xaxis),
            inertial.acceleration(vex::axisType::yaxis),
            inertial.acceleration(vex::axisType::zaxis)};
}
#endif // IMU displacement disabled

constexpr int kOdomPeriodMs = 10; // background update period
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

#if 0 // IMU displacement disabled
void Odometry::calibrateImuBias() {
    // 1) Average the gravity direction while the robot is stationary.
    Vec3 g_sum{0.0, 0.0, 0.0};
    for (int i = 0; i < kCalibSamples; ++i) {
        const Vec3 a = readAccel(inertial_);
        g_sum.x += a.x;
        g_sum.y += a.y;
        g_sum.z += a.z;
        vex::this_thread::sleep_for(kCalibPeriodMs);
    }
    const Vec3 z_body = normalize(g_sum); // vertical (gravity) axis in IMU frame

    // 2) [DISABLED] Yaw (forward-drive) calibration: temporarily disabled so the
    //    robot does not drive forward at boot. Tilt-only leveling is used below.
    //    Re-enable this block to restore yaw correction.
    /*
    l1_.spin(vex::directionType::fwd, kDriveSpeedPct, vex::velocityUnits::pct);
    l2_.spin(vex::directionType::fwd, kDriveSpeedPct, vex::velocityUnits::pct);
    r1_.spin(vex::directionType::fwd, kDriveSpeedPct, vex::velocityUnits::pct);
    r2_.spin(vex::directionType::fwd, kDriveSpeedPct, vex::velocityUnits::pct);

    Vec3 f_sum{0.0, 0.0, 0.0};
    int f_samples = 0;
    const int drive_samples = kDriveMs / kCalibPeriodMs;
    for (int i = 0; i < drive_samples; ++i) {
        const Vec3 a = readAccel(inertial_);
        const double vertical = dot(a, z_body);
        f_sum.x += a.x - vertical * z_body.x;
        f_sum.y += a.y - vertical * z_body.y;
        f_sum.z += a.z - vertical * z_body.z;
        ++f_samples;
        vex::this_thread::sleep_for(kCalibPeriodMs);
    }

    l1_.stop(vex::brakeType::brake);
    l2_.stop(vex::brakeType::brake);
    r1_.stop(vex::brakeType::brake);
    r2_.stop(vex::brakeType::brake);

    const double signal = (f_samples > 0) ? norm(f_sum) / f_samples : 0.0;
    const bool fallback = signal < kForwardSignalEps;
    */

    // 3) Tilt-only leveling: body x = IMU x-axis projected onto the horizontal plane.
    Vec3 ref{1.0, 0.0, 0.0};
    double d = dot(ref, z_body);
    Vec3 proj{ref.x - d * z_body.x, ref.y - d * z_body.y, ref.z - d * z_body.z};
    if (norm(proj) < 1e-6) {
        ref = Vec3{0.0, 1.0, 0.0};
        d = dot(ref, z_body);
        proj = Vec3{ref.x - d * z_body.x, ref.y - d * z_body.y, ref.z - d * z_body.z};
    }
    Vec3 x_body = normalize(proj);

    Vec3 y_body = normalize(cross(z_body, x_body));
    x_body = normalize(cross(y_body, z_body)); // re-orthogonalize

    // NOTE: verify on hardware that a_body.x is "forward" and a_body.y is
    // "right"; if the lateral sign is inverted, swap the cross product order.
    imu_R_[0][0] = x_body.x;
    imu_R_[0][1] = x_body.y;
    imu_R_[0][2] = x_body.z;
    imu_R_[1][0] = y_body.x;
    imu_R_[1][1] = y_body.y;
    imu_R_[1][2] = y_body.z;
    imu_R_[2][0] = z_body.x;
    imu_R_[2][1] = z_body.y;
    imu_R_[2][2] = z_body.z;

    Debug::Log("[Odometry] mounting calib: tilt-only (yaw disabled)", Debug::LogLevel::Info);
}

void Odometry::updateImu(bool drive_braked) {
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

    // Rotate the IMU-frame acceleration into the body frame (mounting correction).
    const double bx = imu_R_[0][0] * ax_g + imu_R_[0][1] * ay_g + imu_R_[0][2] * az_g;
    const double by = imu_R_[1][0] * ax_g + imu_R_[1][1] * ay_g + imu_R_[1][2] * az_g;

    // Rotate the body-frame horizontal acceleration into the power-up frame.
    const double th = rotation() * (M_PI / 180.0);
    const double c = std::cos(th);
    const double s = std::sin(th);
    double wax = (bx * c + by * s) * kGravity;
    double way = (-bx * s + by * c) * kGravity;

    // Low-pass filter to tame noise before integrating.
    wax = kAccelLpf * wax + (1.0 - kAccelLpf) * imu_prev_wax_;
    way = kAccelLpf * way + (1.0 - kAccelLpf) * imu_prev_way_;

    // Pause integration while the drive is braked (robot commanded stationary):
    // zero the velocity, freeze the position, and re-baseline to avoid a jump.
    if (drive_braked) {
        imu_vx_ = 0.0;
        imu_vy_ = 0.0;
        imu_prev_wax_ = wax;
        imu_prev_way_ = way;
        return;
    }

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
#endif // IMU displacement disabled

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
        // Odom.updateImu(Chassis.isBraking()); // IMU displacement disabled
        Odom.updateWheel();
        vex::this_thread::sleep_for(kOdomPeriodMs);
    }
}
