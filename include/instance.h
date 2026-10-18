#pragma once

#include "clamp.h"
#include "lift.h"
#include "odometry.h"
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
extern Odometry Odom;

// True while the autonomous routine is running; driverControl() suspends its
// hardware commands meanwhile to avoid fighting the autonomous routine.
extern volatile bool Auto_Active;

// Root mode: when true, Clamp/Lift manual motion ignores software limits.
extern volatile bool Root_Mode;

extern XDrive Chassis;

extern vex::motor Motor_Lift_1;
extern vex::motor Motor_Lift_2;
extern vex::motor_group Motor_Lift;
extern vex::motor Motor_Clamp;
extern Lift Mech_Lift;
extern Clamp Mech_Clamp;
