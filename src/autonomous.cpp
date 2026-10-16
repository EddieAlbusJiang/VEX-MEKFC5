#include "autonomous.h"
#include "HiDebug.h"
#include "instance.h"

void autonomous() {
    Debug::Log("[Auto] start", Debug::LogLevel::Info);
    // Mark the autonomous phase so driverControl() stops commanding hardware.
    Auto_Active = true;
    // Zero the chassis pose and heading so the routine's absolute targets are
    // relative to the start of this autonomous run.
    Odom.resetRotation();
    Odom.resetImu();
    Odom.resetWheel();
    // Per-wheel idle-brake distorts holonomic motion, so disable it for the
    // whole autonomous routine and restore it at the end.
    Chassis.setIdleBrake(false);

    // Placeholder autonomous routine. Example calls (currently not executed):
    //   Chassis.AutoMoveTo(50.0, 0.0);      // move to (x=50, y=0) cm
    //   Chassis.AutoRotateTo(90.0);         // rotate to heading 90 deg
    //   Chassis.AutoMoveFor(-50.0, 0.0);    // relative move
    //   Chassis.AutoRotateFor(-90.0);       // relative rotate
    Mech_Clamp.AutoClampOpen();
    Chassis.AutoMoveTo(0.0, -6.0);
    Chassis.AutoMoveTo(47.0, -6.0);
    Chassis.AutoMoveTo(47.0, 2.0);
    Mech_Clamp.AutoClampClose();
    Mech_Lift.AutoLiftFor(1812);
    Chassis.AutoRotateTo(-90.0);
    // Chassis.AutoMoveTo(47.0, -6.0);
    Chassis.AutoMoveTo(17.0, -6.0);
    Chassis.AutoMoveTo(17.0, -27.0);
    Chassis.AutoMoveTo(33.0, -27.0);
    Mech_Clamp.AutoClampOpen();

    vex::timer timer;
    printf(" [AUTON] Time: %dms\n", timer.time());

    Chassis.setIdleBrake(true);
    Auto_Active = false;
    Debug::Log("[Auto] done", Debug::LogLevel::Info);
}
