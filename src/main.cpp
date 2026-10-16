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
#include "HiTest.h"
#include "instance.h"
#include "HiDebug.h"
#include "HiIO.h"
#include "vex.h"

void initialize() {
    // Inject the Brain into the HiIO module (this also clears the screen and
    // resets its buffer), then emit one startup log to every enabled output.
    HiIO::BrainScreen::setBrain(Brain);
    Controller.Screen.clearScreen();
    Debug::Log("[Main] MEKFC5 Starting Up...", Debug::LogLevel::Info);

    // Calibrate the inertial sensor until it reports a stable reading.
    while (!Odom.calibrate()) {
        vex::this_thread::sleep_for(10);
    }
    Debug::Log("[Main] Inertial Sensor Calibrated.", Debug::LogLevel::Info);

    // Sample the at-rest acceleration bias (car assumed stationary at power-up).
    Odom.calibrateImuBias();

    // Establish the odometry origin and start the background estimator.
    Odom.resetImu();
    Odom.resetWheel();
    static vex::thread odometry_task(odometryThread);

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
    HiTest(); // displacement unit test (blocks)
    // autonomous();
    driverControl();
    // Keep the program alive so competition control can dispatch callbacks.
    while (true) {
        vex::this_thread::sleep_for(10);
    }
    
    return 0;
}
