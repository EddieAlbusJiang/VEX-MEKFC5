#pragma once

enum class AutoResult { Idle, Running, Reached, Timeout, Stalled, Cancelled };

inline const char* autoResultName(AutoResult result) {
    switch (result) {
        case AutoResult::Idle: return "IDLE";
        case AutoResult::Running: return "RUN";
        case AutoResult::Reached: return "OK";
        case AutoResult::Timeout: return "TIMEOUT";
        case AutoResult::Stalled: return "STALLED";
        case AutoResult::Cancelled: return "STOP";
    }
    return "UNKNOWN";
}

// True only during a cancelled/expired autonomous run. Standalone mechanism
// calibration and manual control are not subject to the route deadline.
bool autonomousStopRequested();
