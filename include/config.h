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

// ===== Auto (autonomous move/rotate) =====
#define P_AUTO_SPEED 60.0        // percent 0..100 (PID output cap)
#define P_AUTO_POS_TOL 2.0       // cm
#define P_AUTO_ANGLE_TOL 2.0     // deg
#define P_AUTO_TIMEOUT_MS 5000   // give up after this long
#define P_AUTO_SETTLE_MS 100     // must stay within tolerance this long
#define P_AUTO_MOVE_PID_KP 2.0
#define P_AUTO_MOVE_PID_KD 0.1
#define P_AUTO_MOVE_PID_KI 0.0
#define P_AUTO_ROT_PID_KP 1.5
#define P_AUTO_ROT_PID_KD 0.2
#define P_AUTO_ROT_PID_KI 0.0

// ===== Auto mechanisms (Clamp/Lift auto routines) =====
#define P_LIFT_TOL 2.0        // deg (Clamp reuses P_CLAMP_TOLERANCE)
#define P_MECH_TIMEOUT_MS 4000
#define P_MECH_SETTLE_MS 100
