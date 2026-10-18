#pragma once

#include "vex.h"

// Odometry: wraps the V5 Inertial sensor and provides heading, plus wheel
// odometry displacement in a frame fixed to the robot's orientation at power-up
// (origin = power-up position, units = cm).
//   - Heading (inertial rotation * coefficient)
//   - Wheel odometry (drive-motor positions via X-drive inverse kinematics)
//   - IMU dead reckoning: DISABLED (see the #if 0 blocks in odometry.cpp).
class Odometry {
  private:
    vex::inertial& inertial_;
    const double coefficient_;
    vex::motor& l1_;
    vex::motor& l2_;
    vex::motor& r1_;
    vex::motor& r2_;
    const double cm_per_deg_;

    // --- IMU dead reckoning (DISABLED) ---
#if 0
    vex::timer imu_timer_;
    int imu_last_time_ = 0; // ms
    bool imu_ready_ = false;
    double imu_vx_ = 0.0, imu_vy_ = 0.0; // cm/s
    double imu_px_ = 0.0, imu_py_ = 0.0; // cm
    double imu_prev_wax_ = 0.0, imu_prev_way_ = 0.0;
    int imu_still_frames_ = 0;
    // Mounting rotation (IMU frame -> body frame), computed by calibrateImuBias().
    // Rows are the body x/y/z axes expressed in the IMU frame; a_body = R * a_imu.
    // Default is identity (no correction).
    double imu_R_[3][3] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
#endif

    // --- Wheel odometry ---
    bool wheel_ready_ = false;
    double wheel_px_ = 0.0, wheel_py_ = 0.0;         // cm
    double wheel_prev_[4] = {0.0, 0.0, 0.0, 0.0};    // last motor positions (deg)

  public:
    Odometry(vex::inertial& inertial, double coefficient,
             vex::motor& l1, vex::motor& l2, vex::motor& r1, vex::motor& r2,
             double wheel_diameter_cm, double motor_to_wheel_ratio = 1.0);

    // Heading (as before).
    double rotation();
    double heading();
    void resetRotation();
    bool calibrate();

    // IMU dead reckoning (DISABLED).
#if 0
    // Calibrate the IMU mounting (tilt + yaw): samples gravity while stationary,
    // then drives forward briefly to resolve the body forward axis. Replaces the
    // former x/y bias subtraction.
    void calibrateImuBias();
    // drive_braked: when true (wheels stopped) the integration is paused.
    void updateImu(bool drive_braked);
    double imuX() const;
    double imuY() const;
    void resetImu();
#endif

    // Wheel odometry.
    void updateWheel();
    double wheelX() const;
    double wheelY() const;
    void resetWheel();
};

// Background task: periodically integrates both estimators.
void odometryThread();
