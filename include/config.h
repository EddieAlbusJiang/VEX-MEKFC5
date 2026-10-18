#pragma once

// Central tuning parameters (macros) for the robot subsystems.
// Included by vex.h so every translation unit sees the same single source.

// ===== Lift =====
#define P_LIFT_MIN_ANGLE 0
#define P_LIFT_MAX_ANGLE 2350.0
#define P_LIFT_SPEED 33.0
// Automatic lift speed is independent of the manual button speed.
#define P_LIFT_AUTO_SPEED 45.0
#define P_LIFT_AUTO_TOL 5.0
#define P_LIFT_AUTO_TIMEOUT_MS 4500

// ===== Clamp =====
#define P_CLAMP_OPEN_ANGLE 90.0
#define P_CLAMP_CLOSE_ANGLE (0.0)
#define P_CLAMP_SPEED 20.0
#define P_CLAMP_TOLERANCE 1.0
#define P_CLAMP_AUTO_TOL 2.0
// Full-open only: the team confirmed release at displayed Cp=85 (rounded).
#define P_CLAMP_AUTO_OPEN_TOL 6.0
#define P_CLAMP_AUTO_TIMEOUT_MS 2000
// Loaded pickup only, using the team's manually established closed Cp=0.
#define P_CLAMP_GRIP_MAX_POSITION 6.0
#define P_CLAMP_GRIP_MIN_TRAVEL 10.0
#define P_CLAMP_GRIP_STABLE_DEG 0.5
#define P_CLAMP_GRIP_SETTLE_MS 200
#define P_AUTO_GRIP_PAUSE_MS 250 // hold the object after closing, before lifting
#define P_CLAMP_PID_KP 0.5
#define P_CLAMP_PID_KD 0.03
#define P_CLAMP_PID_KI 0.0
#define P_CLAMP_CALIB_SPEED 9.0           // percent; slow speed while calibrating close angle
#define P_CLAMP_CALIB_MS 1200             // ms; how long to drive toward close while calibrating
#define P_CLAMP_CLOSE_CALIB_OFFSET (-3.0) // deg; added to the measured close position

// ===== Odometry =====
// #define P_ODOM_COEFFICIENT (3600.0 / 3600.0)
#define P_ODOM_COEFFICIENT 1
#define P_ODOM_WHEEL_DIAMETER_CM 8.25
#define P_ODOM_WHEEL_RATIO 1.0
#define P_ODOM_DIFF_TOL 0.065

// ===== XDrive =====
#define P_XDRIVE_MAX_OUTPUT 0.75
#define P_XDRIVE_MIN_OUTPUT 1.0
#define P_XDRIVE_HYSTERESIS 1.0

// ===== Auto (autonomous move/rotate) =====
#define P_AUTO_SPEED 100.0        // percent 0..100 (PID output cap)
#define P_AUTO_TRANSIT_MAX_OUTPUT 0.85 // long-distance automatic translation only
#define P_AUTO_TRANSIT_BLEND_START_CM 6.0 // original output cap inside this range
#define P_AUTO_TRANSIT_BLEND_END_CM 20.0  // full transit output beyond this range
#define P_AUTO_RETURN_TURN_SPEED 65.0 // unloaded turn after first placement/retreat
#define P_AUTO_POS_TOL 1.0       // cm
#define P_AUTO_ANGLE_TOL 1.5     // deg
#define P_AUTO_TIMEOUT_MS 5000   // give up after this long
#define P_AUTO_SETTLE_MS 20      // final endpoints only; two control ticks
#define P_AUTO_PASS_MIN_OUTPUT 20.0 // continuous via-point approach (percent)
#define P_AUTO_MIN_OUTPUT 8.0   // percent; minimum translation command (anti-crawl)
#define P_AUTO_MOVE_PID_KP 2.5
#define P_AUTO_MOVE_PID_KD 0.06
#define P_AUTO_MOVE_PID_KI 0.0
#define P_AUTO_ROT_PID_KP 0.8
#define P_AUTO_ROT_PID_KD 0.05
#define P_AUTO_ROT_PID_KI 0.0
#define P_AUTO_ROT_MIN_OUTPUT 7.0
#define P_AUTO_POS_HYSTERESIS 0.5
#define P_AUTO_ANGLE_HYSTERESIS 0.5
#define P_AUTO_RUN_LIMIT_MS 30000

// ===== Auto mechanisms (Clamp/Lift auto routines) =====
// Legacy outer-speed-loop settings, retained for source compatibility only.
// Native position moves below do not use these five settings.
#define P_LIFT_TOL 2.0        // deg (Clamp reuses P_CLAMP_TOLERANCE)
#define P_MECH_TIMEOUT_MS 4000
#define P_MECH_SETTLE_MS 100
#define P_MECH_DECEL_TOL 8.0  // deg; start slowing within this distance of target
#define P_MECH_MIN_OUTPUT 12.0 // percent; minimum effective drive to avoid stall
// Active native-position arrival and progress settings.
#define P_MECH_HYSTERESIS 2.0 // deg; extra margin before re-engaging after hold
#define P_MECH_AUTO_SETTLE_MS 60
#define P_MECH_PROGRESS_MS 700
#define P_MECH_PROGRESS_DEG 0.5

// ===== Driver input =====
#define P_DRIVE_CURVE_EXP 1.9 // exponent k for f(x)=sign(x)*|x/100|^k*100

// ===== Auto command queue =====
#define P_AUTO_QUEUE 4 // ring-buffer capacity per subsystem (worker thread)
