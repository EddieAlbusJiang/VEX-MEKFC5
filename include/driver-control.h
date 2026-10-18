#pragma once

#include "calc.h"
#include "instance.h"
#include "vex.h"

// Driver control function, registered in main.cpp via
// Competition.drivercontrol(driverControl). Runs while the driver control
// period is active.
void driverControl();

// Reads the controller and commands the chassis.
void updateChassis();

// Reads the controller and commands the lift (L1 up, L2 down).
void updateLift();

// Reads the controller and commands the clamp (R2 close, R1 open).
void updateClamp();

// Start one display-only worker after initialization. Keeps refreshing during
// blocking autonomous routines, including a manual ROOT + B trigger.
void startControllerTelemetry();

// Writes telemetry (wheel commands, mechanism angles, position and elapsed
// time since the display worker started). Called only by the display worker.
void updateControllerScreen();
