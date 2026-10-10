#include "vex.h"

#include "instance.h"

vex::competition Competition;
vex::brain Brain;
vex::controller Controller;

vex::motor Motor_Drive_L1(vex::PORT2, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_L2(vex::PORT14, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_R1(vex::PORT5, vex::gearSetting::ratio18_1, true);
vex::motor Motor_Drive_R2(vex::PORT18, vex::gearSetting::ratio18_1, true);

vex::inertial Inertial(vex::PORT20);
HeadingTracker Heading_Tracker(Inertial, 3600.0 / 3544.0);

XDrive Chassis(Motor_Drive_L1, Motor_Drive_L2, Motor_Drive_R1, Motor_Drive_R2);

vex::motor Motor_Lift(vex::PORT9, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Clamp(vex::PORT10, vex::gearSetting::ratio18_1, false);

Lift Mech_Lift(Motor_Lift, 0.0, 1080.0, 15.0);
Clamp Mech_Clamp(Motor_Clamp, 90.0, -20.0, 15.0);
