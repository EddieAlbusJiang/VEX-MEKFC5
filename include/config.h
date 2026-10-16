#pragma once

// Central tuning parameters (macros) for the robot subsystems.
// Included by vex.h so every translation unit sees the same single source.

// ===== Lift =====
#define P_LIFT_MIN_ANGLE 0
#define P_LIFT_MAX_ANGLE 2350.0
#define P_LIFT_SPEED 33.0

// ===== Clamp =====
#define P_CLAMP_OPEN_ANGLE 85.0
#define P_CLAMP_CLOSE_ANGLE (-20.0)
#define P_CLAMP_SPEED 20.0
#define P_CLAMP_TOLERANCE 1.0
#define P_CLAMP_PID_KP 0.5
#define P_CLAMP_PID_KD 0.03
#define P_CLAMP_PID_KI 0.0

// ===== Odometry =====
#define P_ODOM_COEFFICIENT (3600.0 / 3544.0)
#define P_ODOM_WHEEL_DIAMETER_CM 8.25
#define P_ODOM_WHEEL_RATIO 1.0

// ===== XDrive =====
#define P_XDRIVE_MAX_OUTPUT 0.75
#define P_XDRIVE_MIN_OUTPUT 1.0
#define P_XDRIVE_HYSTERESIS 1.0

// ===== Auto (autonomous move/rotate) =====
#define P_AUTO_SPEED 100.0        // percent 0..100 (PID output cap)
#define P_AUTO_POS_TOL 1.0       // cm
#define P_AUTO_ANGLE_TOL 1.5     // deg
#define P_AUTO_TIMEOUT_MS 5000   // give up after this long
#define P_AUTO_SETTLE_MS 50      // must stay within tolerance this long
#define P_AUTO_MIN_OUTPUT 12.0   // percent; minimum translation command (anti-crawl)
#define P_AUTO_MOVE_PID_KP 2.5
#define P_AUTO_MOVE_PID_KD 0.06
#define P_AUTO_MOVE_PID_KI 0.0
#define P_AUTO_ROT_PID_KP 0.8
#define P_AUTO_ROT_PID_KD 0.05
#define P_AUTO_ROT_PID_KI 0.0

// ===== Auto mechanisms (Clamp/Lift auto routines) =====
#define P_LIFT_TOL 2.0        // deg (Clamp reuses P_CLAMP_TOLERANCE)
#define P_MECH_TIMEOUT_MS 4000
#define P_MECH_SETTLE_MS 100
#define P_MECH_DECEL_TOL 8.0  // deg; start slowing within this distance of target
#define P_MECH_MIN_OUTPUT 12.0 // percent; minimum effective drive to avoid stall
#define P_MECH_HYSTERESIS 2.0 // deg; extra margin before re-engaging after hold

// ===== Driver input =====
#define P_DRIVE_CURVE_EXP 1.9 // exponent k for f(x)=sign(x)*|x/100|^k*100

// ===== Auto command queue =====
#define P_AUTO_QUEUE 4 // ring-buffer capacity per subsystem (worker thread)
