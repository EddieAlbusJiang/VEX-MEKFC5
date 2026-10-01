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
#include "vex.h"

void initialize() {
    // Controller and brain startup message.
    Controller.Screen.clearScreen();
    Controller.Screen.setCursor(1, 1);
    Controller.Screen.print("MEKFC5 ready");

    Brain.Screen.clearScreen();
    Brain.Screen.setCursor(1, 1);
    Brain.Screen.print("MEKFC5 ready");

    printf("MEKFC5 ready\n");

    // Calibrate the inertial sensor until it reports a stable reading.
    while (!Heading_Tracker.calibrate()) {
        vex::this_thread::sleep_for(10);
    }
}

int main() {
    initialize();

    // Register the competition callbacks.
    Competition.autonomous(autonomous);
    Competition.drivercontrol(driverControl);

    // Keep the program alive so competition control can dispatch callbacks.
    while (true) {
        vex::this_thread::sleep_for(10);
    }
    return 0;
}
