#pragma once

#include "heading-tracker.h"
#include "vex.h"
#include "x-drive.h"

extern vex::competition Competition;
extern vex::brain Brain;
extern vex::controller Controller;

extern vex::motor Motor_Drive_L1;
extern vex::motor Motor_Drive_L2;
extern vex::motor Motor_Drive_R1;
extern vex::motor Motor_Drive_R2;

extern vex::inertial Inertial;
extern HeadingTracker Heading_Tracker;

extern XDrive Chassis;
