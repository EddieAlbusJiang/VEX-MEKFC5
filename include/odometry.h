#pragma once

#include "vex.h"

// Odometry: wraps the V5 Inertial sensor and provides heading plus two parallel
// displacement estimates in a frame fixed to the robot's orientation at
// power-up (origin = power-up position, units = cm):
//   - IMU dead reckoning (double-integrated acceleration), for reference/testing
//   - Wheel odometry (drive-motor positions via X-drive inverse kinematics)
class Odometry {
  private:
    vex::inertial& inertial_;
    const double coefficient_;
    vex::motor& l1_;
    vex::motor& l2_;
    vex::motor& r1_;
    vex::motor& r2_;
    const double cm_per_deg_;

    // --- IMU dead reckoning ---
    vex::timer imu_timer_;
    int imu_last_time_ = 0; // ms
    bool imu_ready_ = false;
    double imu_vx_ = 0.0, imu_vy_ = 0.0; // cm/s
    double imu_px_ = 0.0, imu_py_ = 0.0; // cm
    double imu_prev_wax_ = 0.0, imu_prev_way_ = 0.0;
    int imu_still_frames_ = 0;

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

    // IMU dead reckoning.
    void updateImu();
    double imuX() const;
    double imuY() const;
    void resetImu();

    // Wheel odometry.
    void updateWheel();
    double wheelX() const;
    double wheelY() const;
    void resetWheel();
};

// Background task: periodically integrates both estimators.
void odometryThread();
