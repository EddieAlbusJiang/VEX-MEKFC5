#include "HiDebug/conf.h"
#include "HiDebug/log.h"
#include "HiIO.h"
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

// Guards the Console/Controller output destinations
vex::mutex log_mutex;
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
    HiIO::BrainScreen::printLine(line);
#endif
#ifdef HD_LOG_OUTPUT_CONTROLLER
    Controller.Screen.print("%s", line.c_str());
#endif
    log_mutex.unlock();
}
