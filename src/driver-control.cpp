#include "driver-control.h"
#include "HiDebug.h"

#include <cmath>
#include <cstdio>

namespace {
// Ignore small joystick noise near the center.
constexpr double kDeadZone = 3.0;
// Screen refresh period (ms). The screen updates over I2C, so throttle it.
constexpr int kScreenRefreshMs = 150;
// Time since driverControl() started; reset on entry.
vex::timer control_timer;
} // namespace

void driverControl() {
    Debug::Log("Manual Control Override.\n", Debug::LogLevel::Debug);
    control_timer.reset();
    // Keep running while the driver control period is active.
    while (true) {
        updateChassis();
        updateLift();
        updateClamp();
        updateControllerScreen();

        // Real-time mechanism telemetry (angles in motor degrees).
        printf("Lift %.1f Clamp %.1f\n", Mech_Lift.position(), Mech_Clamp.position());

        vex::this_thread::sleep_for(10);
    }
}

void updateChassis() {
    // Left stick: forward/backward (Axis3) and left/right translation (Axis4).
    // Right stick: horizontal axis (Axis1) rotates the robot in place.
    const double forward_input = deadZone(Controller.Axis3.position(), kDeadZone);
    const double lateral_input = deadZone(Controller.Axis4.position(), kDeadZone);
    const double rotational_input = deadZone(Controller.Axis1.position(), kDeadZone);

    Chassis.drive(forward_input, lateral_input, rotational_input);
}

void updateLift() {
    if (Controller.ButtonL1.pressing()) {
        printf("Controller L1\n");
        Mech_Lift.up();
    } else if (Controller.ButtonL2.pressing()) {
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

    // Row 3: elapsed time since driverControl() started, whole seconds.
    int seconds = now_ms / 1000;
    if (seconds > 999) {
        seconds = 999;
    }
    snprintf(row3, sizeof(row3), "Manual Override %3d", seconds);

    // Fixed-width fields overwrite the previous frame, so no clear is needed.
    Controller.Screen.setCursor(1, 1);
    Controller.Screen.print("%s", row1);
    Controller.Screen.setCursor(2, 1);
    Controller.Screen.print("%s", row2);
    Controller.Screen.setCursor(3, 1);
    Controller.Screen.print("%s", row3);
}
