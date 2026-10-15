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
Odometry Odom(Inertial, P_ODOM_COEFFICIENT, Motor_Drive_L1, Motor_Drive_L2, Motor_Drive_R1,
              Motor_Drive_R2, P_ODOM_WHEEL_DIAMETER_CM, P_ODOM_WHEEL_RATIO);

XDrive Chassis(Motor_Drive_L1, Motor_Drive_L2, Motor_Drive_R1, Motor_Drive_R2);

vex::motor Motor_Lift_1(vex::PORT9, vex::gearSetting::ratio18_1, false);
vex::motor Motor_Lift_2(vex::PORT8, vex::gearSetting::ratio18_1, true);
vex::motor_group Motor_Lift(Motor_Lift_1, Motor_Lift_2);
vex::motor Motor_Clamp(vex::PORT10, vex::gearSetting::ratio18_1, false);

Lift Mech_Lift(Motor_Lift, P_LIFT_MIN_ANGLE, P_LIFT_MAX_ANGLE, P_LIFT_SPEED);
Clamp Mech_Clamp(Motor_Clamp, P_CLAMP_OPEN_ANGLE, P_CLAMP_CLOSE_ANGLE, P_CLAMP_SPEED);
