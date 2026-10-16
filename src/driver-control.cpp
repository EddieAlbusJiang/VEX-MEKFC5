#include "driver-control.h"
#include "HiDebug.h"

#include <cmath>
#include <cstdio>

namespace {
// Ignore small joystick noise near the center.
constexpr double kDeadZone = 2.0;
// Screen refresh period (ms). The screen updates over I2C, so throttle it.
constexpr int kScreenRefreshMs = 150;
// Time since driverControl() started; reset on entry.
vex::timer control_timer;

// Drive reference frame.
//   HeadingUp (mode 1): joystick forward follows the robot's current heading.
//   NorthUp   (mode 2): joystick forward is locked to heading 0 (the "north").
enum class DriveMode : int { HeadingUp = 1, NorthUp = 2 };

DriveMode drive_mode = DriveMode::HeadingUp; // default mode 1
vex::mutex drive_mode_mutex;                 // shared with the button callbacks
bool heading_reset_requested = false;
// Axis-aligned translation (Button A): keep only the dominant translation axis.
bool axis_lock_enabled = false;

DriveMode getDriveMode() {
    drive_mode_mutex.lock();
    const DriveMode mode = drive_mode;
    drive_mode_mutex.unlock();
    return mode;
}

bool getAxisLock() {
    drive_mode_mutex.lock();
    const bool enabled = axis_lock_enabled;
    drive_mode_mutex.unlock();
    return enabled;
}

void onAxisLockToggle() {
    drive_mode_mutex.lock();
    axis_lock_enabled = !axis_lock_enabled;
    drive_mode_mutex.unlock();
}

void onModeToggle() {
    drive_mode_mutex.lock();
    drive_mode = (drive_mode == DriveMode::HeadingUp) ? DriveMode::NorthUp : DriveMode::HeadingUp;
    heading_reset_requested = true;// Automatically Reset Heading.
    drive_mode_mutex.unlock();
}

void onHeadingReset() {
    // Defer the sensor call to the control loop to avoid touching the inertial
    // sensor from the callback thread.
    drive_mode_mutex.lock();
    heading_reset_requested = true;
    drive_mode_mutex.unlock();
}

void serviceHeadingReset() {
    drive_mode_mutex.lock();
    const bool requested = heading_reset_requested;
    heading_reset_requested = false;
    drive_mode_mutex.unlock();
    if (requested) {
        Odom.resetRotation();
    }
}
} // namespace

void driverControl() {
    Debug::Log("[DriverControl] Manual Control Override.\n", Debug::LogLevel::Debug);
    control_timer.reset();

    // Register the drive-mode and heading-reset button callbacks once.
    static bool callbacks_registered = false;
    if (!callbacks_registered) {
        Controller.ButtonX.pressed(onModeToggle);
        Controller.ButtonY.pressed(onHeadingReset);
        Controller.ButtonA.pressed(onAxisLockToggle);
        callbacks_registered = true;
    }

    // Keep running while the driver control period is active.
    while (true) {
        serviceHeadingReset();
        updateChassis();
        updateLift();
        updateClamp();
        updateControllerScreen();

        // Real-time mechanism telemetry (angles in motor degrees).
        printf("[Mech] Lift %.1f Clamp %.1f\n", Mech_Lift.position(), Mech_Clamp.position());
        printf("[Inertial] Heading %.1f Rotation %.1f\n", Odom.heading(), Odom.rotation());
        vex::this_thread::sleep_for(10);
    }
}

void updateChassis() {
    // Left stick: forward/backward (Axis3) and left/right translation (Axis4).
    // Right stick: horizontal axis (Axis1) rotates the robot in place.
    const double f = deadZone(Controller.Axis3.position(), kDeadZone);
    const double l = deadZone(Controller.Axis4.position(), kDeadZone);
    const double r = deadZone(Controller.Axis1.position(), kDeadZone);

    double body_forward = f;
    double body_lateral = l;

    if (getDriveMode() == DriveMode::NorthUp) {
        // Rotate the field-frame (north/east) input into the robot's body frame.
        const double heading_rad = Odom.heading() * (M_PI / 180.0);
        const double c = std::cos(heading_rad);
        const double s = std::sin(heading_rad);
        body_forward = f * c + l * s;
        body_lateral = -f * s + l * c;
    }

    // Axis-aligned mode: after the frame transform, keep only the dominant
    // translation axis so the robot travels in a straight line.
    if (getAxisLock()) {
        if (std::fabs(body_forward) >= std::fabs(body_lateral)) {
            body_lateral = 0.0; // ties keep forward
        } else {
            body_forward = 0.0;
        }
    }

    Chassis.drive(body_forward, body_lateral, r);
}

void updateLift() {
    if (Controller.ButtonUp.pressing()) {
        printf("Controller L1\n");
        Mech_Lift.up();
    } else if (Controller.ButtonDown.pressing()) {
        printf("Controller L2\n");
        Mech_Lift.down();
    } else {
        Mech_Lift.stop();
    }
}

void updateClamp() {
    if (Controller.ButtonR2.pressing()) {
        printf("Controller R2\n");
        Mech_Clamp.close();
    } else if (Controller.ButtonR1.pressing()) {
        printf("Controller R1\n");
        Mech_Clamp.open();
    } else {
        Mech_Clamp.stop();
    }
}

void updateControllerScreen() {
    const int now_ms = control_timer.time();

    // Throttle the (slow) screen writes.
    static int last_refresh_ms = -kScreenRefreshMs;
    if (now_ms - last_refresh_ms < kScreenRefreshMs) {
        return;
    }
    last_refresh_ms = now_ms;

    // Wheel speed magnitude, clamped to two digits.
    const auto magnitude2 = [](double value) {
        const int magnitude = static_cast<int>(std::lround(std::fabs(value)));
        return magnitude > 99 ? 99 : magnitude;
    };

    // Integer cm clamped to what a 4-char field can hold (-999..9999).
    const auto clamp4 = [](double value) {
        int v = static_cast<int>(std::lround(value));
        if (v > 9999) {
            v = 9999;
        }
        if (v < -999) {
            v = -999;
        }
        return v;
    };

    char row1[20];
    char row2[20];
    char row3[20];

    // Row 1: final commanded wheel speeds (L1 L2 R1 R2 -> Lf Lb Rf Rb), 2 digits.
    snprintf(row1, sizeof(row1), "Lf%02d Lb%02d Rf%02d Rb%02d",
                  magnitude2(Chassis.wheelOutput(0)), magnitude2(Chassis.wheelOutput(1)),
                  magnitude2(Chassis.wheelOutput(2)), magnitude2(Chassis.wheelOutput(3)));

    // Row 2: mechanism angles in degrees, 4 digits.
    snprintf(row2, sizeof(row2), "Lift%4d  Clamp%4d",
                  static_cast<int>(std::lround(Mech_Lift.position())),
                  static_cast<int>(std::lround(Mech_Clamp.position())));

    // Row 3: wheel-odometry position (x forward, y right, cm), axis-lock flag,
    // drive mode, and elapsed time (seconds).
    int seconds = now_ms / 1000;
    if (seconds > 999) {
        seconds = 999;
    }
    const int x = clamp4(Odom.wheelX());
    const int y = clamp4(Odom.wheelY());
    const char axis_char = getAxisLock() ? 'A' : ' ';
    const char mode_char = (getDriveMode() == DriveMode::NorthUp) ? 'Y' : 'X';
    snprintf(row3, sizeof(row3), "Man %4d,%4d %c%c%03d", x, y, axis_char, mode_char, seconds);
                                  
    // Fixed-width fields overwrite the previous frame, so no clear is needed.
    Controller.Screen.setCursor(1, 1);
    Controller.Screen.print("%s", row1);
    Controller.Screen.setCursor(2, 1);
    Controller.Screen.print("%s", row2);
    Controller.Screen.setCursor(3, 1);
    Controller.Screen.print("%s", row3);
}
