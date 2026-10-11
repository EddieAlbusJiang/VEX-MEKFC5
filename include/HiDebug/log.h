#pragma once

#include "vex.h"
#include <string>
#include <bits/basic_string.h>

// ============================================================================
// Logging configuration
// ----------------------------------------------------------------------------
// Enable/disable each output destination. Set a macro to 0 to compile that
// output out entirely, or define it before including this header to override.
// ============================================================================
// #define HD_LOG_OUTPUT_BRAIN
// #define HD_LOG_OUTPUT_CONTROLLER
// #define HD_LOG_OUTPUT_CONSOLE

// ============================================================================
// Log filtering level
// ----------------------------------------------------------------------------
// Logs below this level are discarded. Must expand to one of the LogLevel
// enumerators below, e.g. LOG_MIN_LEVEL = LogLevel::Warning.
// ============================================================================
#ifndef HD_LOG_MIN_LEVEL
#define HD_LOG_MIN_LEVEL LogLevel::Info
#endif

namespace Debug {
// Log levels, ordered from lowest to highest severity.
enum class LogLevel : int {
    Debug = 0,
    Info,
    Warning,
    Critical,
    Fatal
};
// Unified logging entry point.
// Output format: [<Level>]<Content>\n  (e.g. "[W]High Velocity")
// levelTag: Debug->D, Info->I, Warning->W, Critical->C, Fatal->F
void Log(const std::string& content, LogLevel level);
} // namespace Debug
