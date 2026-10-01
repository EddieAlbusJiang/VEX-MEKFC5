#include "driver-control.h"

namespace {
// Ignore small joystick noise near the center.
constexpr double kDeadZone = 3.0;
} // namespace

void driverControl() {
    // Keep running while the driver control period is active.
    while (true) {
        updateChassis();

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
