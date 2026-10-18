#pragma once

#include "vex.h"
#include "auto-motion.h"

// Select the right (L2, default) or left (L1) autonomous route.
enum class AutonomousMode { Right, Left };
extern volatile AutonomousMode Auto_Mode;

struct AutonomousTelemetry {
    bool has_run;
    bool active;
    unsigned step;
    const char* action;
    unsigned elapsed_ms;
    unsigned step_elapsed_ms;
    AutoResult result;
};
AutonomousTelemetry autonomousTelemetry();
void requestAutonomousStop();

// Autonomous function, registered in main.cpp via
// Competition.autonomous(autonomous). Runs during the autonomous period.
void autonomous();
