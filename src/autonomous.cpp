#include "autonomous.h"
#include "instance.h"

void autonomous() {
    // Per-wheel idle-brake distorts holonomic motion, so disable it for the
    // whole autonomous routine and restore it at the end.
    Chassis.setIdleBrake(false);

    // Placeholder autonomous routine. Example calls (currently not executed):
    //   Chassis.AutoMoveTo(50.0, 0.0);      // move to (x=50, y=0) cm
    //   Chassis.AutoRotateTo(90.0);         // rotate to heading 90 deg
    //   Chassis.AutoMoveFor(-50.0, 0.0);    // relative move
    //   Chassis.AutoRotateFor(-90.0);       // relative rotate

    vex::timer timer;
    printf(" [AUTON] Time: %dms\n", timer.time());

    Chassis.setIdleBrake(true);
}
