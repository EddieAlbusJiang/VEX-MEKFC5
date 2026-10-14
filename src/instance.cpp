#include "vex.h"

#include "instance.h"

vex::competition Competition;
vex::brain Brain;
vex::controller Controller;

vex::motor Motor_Drive_L1(vex::PORT2, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_L2(vex::PORT3, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Drive_R1(vex::PORT4, vex::gearSetting::ratio18_1, true);
vex::motor Motor_Drive_R2(vex::PORT5, vex::gearSetting::ratio18_1, true);

vex::inertial Inertial(vex::PORT20);
Odometry Odom(Inertial, 3600.0 / 3544.0, Motor_Drive_L1, Motor_Drive_L2, Motor_Drive_R1,
              Motor_Drive_R2, 8.25 /* wheel diameter (cm) */, 1.0 /* motor:wheel ratio */);

XDrive Chassis(Motor_Drive_L1, Motor_Drive_L2, Motor_Drive_R1, Motor_Drive_R2);

vex::motor Motor_Lift_1(vex::PORT9, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Lift_2(vex::PORT8, vex::gearSetting::ratio18_1, true);
vex::motor_group Motor_Lift(Motor_Lift_1, Motor_Lift_2);
vex::motor Motor_Clamp(vex::PORT10, vex::gearSetting::ratio18_1, false);

Lift Mech_Lift(Motor_Lift, 0.0, 1250.0, 30.0);
Clamp Mech_Clamp(Motor_Clamp, 40.0, -10.0, 20.0);
