#include "HiDebug/conf.h"
#include "HiDebug/log.h"
#include "instance.h"
using Debug::LogLevel;
#include <cstdio>

namespace {
const char* levelTag(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "D";
        case LogLevel::Info:
            return "I";
        case LogLevel::Warning:
            return "W";
        case LogLevel::Critical:
            return "C";
        case LogLevel::Fatal:
            return "F";
    }
    return "?";
}

// Guards the output destinations
vex::mutex log_mutex;

// Rolling text buffer for the Brain screen. The default Brain font (mono20)
// shows 12 rows, so the buffer keeps the most recent 12 lines: when a new line
// would exceed that, the oldest is dropped and the screen is redrawn.
constexpr int kBrainRows = 12;
std::string brain_lines[kBrainRows];
int brain_line_count = 0;

void redrawBrainScreen() {
    Brain.Screen.clearScreen();
    Brain.Screen.setCursor(1, 1);
    for (int i = 0; i < brain_line_count; ++i) {
        Brain.Screen.print("%s", brain_lines[i].c_str());
        Brain.Screen.newLine();
    }
}
} // namespace

void Debug::Log(const std::string& content, LogLevel level) {
#ifndef HD_LOG_ENABLED
        return;
#endif

    // Filter first: skip the string construction entirely when suppressed.
    if (level < HD_LOG_MIN_LEVEL) {
        return;
    }

    const std::string line = "[" + std::string(levelTag(level)) + "]" + content + "\r\n";

    log_mutex.lock();
#ifdef HD_LOG_OUTPUT_CONSOLE
    printf("%s", line.c_str());
#endif
#ifdef HD_LOG_OUTPUT_BRAIN
    if (brain_line_count < kBrainRows) {
        // Still filling the screen: append and print incrementally.
        brain_lines[brain_line_count++] = line;
        Brain.Screen.print("%s", line.c_str());
        Brain.Screen.newLine();
    } else {
        // Full: drop the oldest line, append the new one at the bottom, then
        // repaint the rolling window.
        for (int i = 1; i < kBrainRows; ++i) {
            brain_lines[i - 1] = brain_lines[i];
        }
        brain_lines[kBrainRows - 1] = line;
        redrawBrainScreen();
    }
#endif
#ifdef HD_LOG_OUTPUT_CONTROLLER
    Controller.Screen.print("%s", line.c_str());
#endif
    log_mutex.unlock();
}
