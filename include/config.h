#pragma once

// Central tuning parameters (macros) for the robot subsystems.
// Included by vex.h so every translation unit sees the same single source.

// ===== Lift =====
#define P_LIFT_MIN_ANGLE 0.0
#define P_LIFT_MAX_ANGLE 1250.0
#define P_LIFT_SPEED 30.0

// ===== Clamp =====
#define P_CLAMP_OPEN_ANGLE 40.0
#define P_CLAMP_CLOSE_ANGLE (-10.0)
#define P_CLAMP_SPEED 20.0
#define P_CLAMP_TOLERANCE 1.0
#define P_CLAMP_PID_KP 0.9
#define P_CLAMP_PID_KD 0.05
#define P_CLAMP_PID_KI 0.0

// ===== Odometry =====
#define P_ODOM_COEFFICIENT (3600.0 / 3544.0)
#define P_ODOM_WHEEL_DIAMETER_CM 8.25
#define P_ODOM_WHEEL_RATIO 1.0

// ===== XDrive =====
#define P_XDRIVE_MAX_OUTPUT 0.5
#define P_XDRIVE_MIN_OUTPUT 1.0
#define P_XDRIVE_HYSTERESIS 1.0
