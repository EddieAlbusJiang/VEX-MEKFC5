#include "vex.h"

#include "instance.h"

vex::competition Competition;
vex::brain Brain;
vex::controller Controller;

vex::motor Motor_Drive_L1(vex::PORT1, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_L2(vex::PORT2, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_R1(vex::PORT3, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_R2(vex::PORT4, vex::gearSetting::ratio18_1, false);

vex::inertial Inertial(vex::PORT20);
HeadingTracker Heading_Tracker(Inertial, 3600.0 / 3544.0);

XDrive Chassis(Motor_Drive_L1, Motor_Drive_L2, Motor_Drive_R1, Motor_Drive_R2);
