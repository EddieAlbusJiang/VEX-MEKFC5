/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       Eddie                                                     */
/*    Created:      9/15/2026, 11:06:43 PM                                    */
/*    Description:  V5 project (MEKFC5, X-drive)                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
#include "autonomous.h"
#include "driver-control.h"
#include "instance.h"
#include "HiDebug.h"
#include "vex.h"

void initialize() {
    // Clear the displays, then emit one startup log to every enabled output.
    Controller.Screen.clearScreen();
    Brain.Screen.clearScreen();
    Debug::Log("[Main] MEKFC5 Starting Up...", Debug::LogLevel::Info);

    // Calibrate the inertial sensor until it reports a stable reading.
    while (!Heading_Tracker.calibrate()) {
        vex::this_thread::sleep_for(10);
    }
    Debug::Log("[Main] Inertial Sensor Calibrated.", Debug::LogLevel::Info);

    // Zero the upper mechanisms: the load is manually placed at the bottom and
    // the clamp manually opened before power-up.
    Mech_Lift.resetPosition();
    Mech_Clamp.resetPosition();
}

int main() {
    printf("Hello MEKFC5\r\n");
    initialize();

    // Register the competition callbacks.
    Competition.autonomous(autonomous);
    Competition.drivercontrol(driverControl);
    driverControl();
    // Keep the program alive so competition control can dispatch callbacks.
    while (true) {
        vex::this_thread::sleep_for(10);
    }
    
    return 0;
}
