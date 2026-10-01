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
