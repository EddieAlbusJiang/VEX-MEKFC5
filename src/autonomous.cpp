#include "autonomous.h"
#include "HiDebug.h"
#include "instance.h"

void autonomous() {
    Debug::Log("[Auto] start", Debug::LogLevel::Info);
    Controller.Screen.clearScreen();
    Controller.Screen.setCursor(2, 1);
    Controller.Screen.print("Autonomous Override");
    
    vex::timer timer;
    // Mark the autonomous phase so driverControl() stops commanding hardware.
    Auto_Active = true;
    // Zero the chassis pose and heading so the routine's absolute targets are
    // relative to the start of this autonomous run.
    Odom.resetRotation();
    // Odom.resetImu();
    Odom.resetWheel();
    // Per-wheel idle-brake distorts holonomic motion, so disable it for the
    // whole autonomous routine and restore it at the end.
    Chassis.setIdleBrake(false);

    // Placeholder autonomous routine. Example calls (currently not executed):
    //   Chassis.AutoMoveTo(50.0, 0.0);      // move to (x=50, y=0) cm
    //   Chassis.AutoRotateTo(90.0);         // rotate to heading 90 deg
    //   Chassis.AutoMoveFor(-50.0, 0.0);    // relative move
    //   Chassis.AutoRotateFor(-90.0);       // relative rotate

    // First Stage
    // Mech_Clamp.AutoClampOpen();
    // Chassis.AutoMoveTo(0.0, -9.0);          // Clear Point
    // Chassis.AutoMoveTo(47.0, -9.0);         // Pin 1
    // Chassis.AutoMoveTo(47.0, -1.0);          // Approach
    // Mech_Clamp.AutoClampClose();
    // Mech_Lift.AutoLiftTo(1812, nullptr);
    // Chassis.AutoRotateTo(-90.0);
    // // Chassis.AutoMoveTo(47.0, -6.0);  //deserted
    // Chassis.AutoMoveTo(17.0, -9.0); 
    // Chassis.AutoMoveTo(34.0, -30.0);
    // Mech_Clamp.AutoClampOpen();

    // First Stage Optimized        // -3
    Mech_Clamp.AutoClampOpen();
    Mech_Lift.AutoLiftTo(30, nullptr);
    Chassis.AutoMoveTo(0.0, -9.0);          // Clear Point
    Chassis.AutoMoveTo(47.5, -9.0);         // Pin 1
    Chassis.AutoMoveTo(47.5, -1.5);          // Approach
    Mech_Clamp.AutoClampClose();
    Mech_Lift.AutoLiftTo(1812, nullptr);
    Chassis.AutoRotateTo(-180, 45);
    Chassis.AutoMoveTo(47.5, -17);
    Mech_Clamp.AutoClampOpen();
    Chassis.AutoMoveTo(47.5, -14);

    // Second Stage
    // Chassis.AutoMoveTo(4, -28);             // Clear Point
    // Chassis.AutoRotateTo(0);
    // Chassis.AutoMoveTo(-40, -2);             // Pin 2
    // Chassis.AutoMoveTo(-40, 7);            // Approach
    // Mech_Lift.AutoLiftFor(0); 
    // Mech_Clamp.AutoClampClose();
    // Mech_Lift.AutoLiftTo(1812, nullptr);
    // Chassis.AutoMoveTo(-40, -5);
    // Chassis.AutoRotateTo(-180);
    // Mech_Clamp.AutoClampOpen();

    // Second Stage Optimized
    // Mech_Lift.AutoLiftTo(30, nullptr);
    // Chassis.AutoMoveTo(-47.0, -14.0);
    // Chassis.AutoMoveTo(-54.0, -13.0);
    // Chassis.AutoRotateTo(0);
    // Chassis.AutoMoveTo(-54.0, -11.0);
    // Mech_Clamp.AutoClampClose();
    // Mech_Lift.AutoLiftTo(1812, nullptr);
    // Chassis.AutoRotateTo(-180, 45);
    // Chassis.AutoMoveTo(-54.0, -20);
    // Mech_Clamp.AutoClampOpen();
    
    // Third Stage Optimized (Not Toggling yet)
    // Chassis.AutoMoveTo(-40.0, -6);
    // Mech_Lift.AutoLiftTo(1812, nullptr);
    // Chassis.AutoMoveTo(0, 2);
    // Chassis.AutoRotateTo(0);

    Chassis.AutoMoveTo(0, -14.0);
    Chassis.AutoRotateTo(-180);
    // Chassis.AutoMoveFor(0, 2.0);
    Chassis.AutoMoveTo(-10, -80.0);
    Chassis.AutoFastMoveFor(0, 100);
    Chassis.AutoFastMoveFor(0, -80);
    Chassis.AutoMoveFor(-20, 0);
    // Chassis.AutoFastMoveFor(0, -20);
    // Chassis.AutoFastMoveFor(0, 20);
    // Chassis.AutoFastMoveFor(0, -20);



    printf(" [AUTON] Time: %dms\n", timer.time());
    Chassis.setIdleBrake(true);
    Auto_Active = false;
    Controller.Screen.clearScreen();
    Debug::Log("[Auto] done", Debug::LogLevel::Info);
}
