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
    // Brain.Screen.setFillColor()
    Debug::Log("[Main] MEKFC5 AUTO V3 PRE Starting Up", Debug::LogLevel::Info);
    // Calibrate the inertial sensor until it reports a stable reading.
    while (!Odom.calibrate()) {
        vex::this_thread::sleep_for(10);
    }
    Debug::Log("[Main] Inertial Sensor Calibrated.", Debug::LogLevel::Info);

    // IMU displacement disabled.
    // Odom.calibrateImuBias();

    // Establish the odometry origin and start the background estimator.
    // Odom.resetImu(); // IMU displacement disabled
    Odom.resetWheel();
    static vex::thread odometry_task(odometryThread);

    // Upper mechanisms at boot: the load is manually placed at the bottom; the
    // clamp is manually opened, so treat it as being at its maximum angle.
    Mech_Lift.resetPosition();
    Mech_Clamp.assumeOpen();          // establish reference: open = open_angle
    Mech_Clamp.calibrateCloseAngle(); // boot calibration of the close angle
    //

    Debug::Log("[Main] Initialized.", Debug::LogLevel::Info);
    // L2 remains the default but is inactive during the stage-one test.
    Auto_Mode = AutonomousMode::Right;
    Controller.ButtonL2.pressed([]() {
        Auto_Mode = AutonomousMode::Right;
        Debug::Log("[Auto] Selected L2 (inactive).", Debug::LogLevel::Info);
    });
    Controller.ButtonL1.pressed([]() {
        Auto_Mode = AutonomousMode::Left;
        Debug::Log("[V3] Selected L1 / 30s", Debug::LogLevel::Info);
    });
    Debug::Log("[V3] L2 inactive. Select L1.", Debug::LogLevel::Info);
    Controller.Screen.setCursor(1, 1);
    Controller.Screen.print("Hi Summoner!");
    startControllerTelemetry();

}

int main() {
    printf("Hello MEKFC5 AUTO V3-PRE\r\n");
    initialize();

    // Register the competition callbacks.
    Competition.autonomous(autonomous);
    Competition.drivercontrol(driverControl);
    HiTest(); // displacement unit test (blocks)
    // autonomous();
    // driverControl();
    // Keep the program alive so competition control can dispatch callbacks.
    while (true) {
        vex::this_thread::sleep_for(10);
    }
    
    return 0;
}
