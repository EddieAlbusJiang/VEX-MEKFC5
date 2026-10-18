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

// Producers only hold this mutex while copying a line into the queue.
// Screen/USB I/O belongs to the logger worker, never the motion worker.
vex::mutex log_mutex;
constexpr unsigned kLogCapacity = 128;
std::string pending[kLogCapacity];
unsigned log_head = 0, log_count = 0, dropped = 0;
bool log_worker_started = false;

void outputLine(const std::string& line) {
#ifdef HD_LOG_OUTPUT_CONSOLE
    printf("%s", line.c_str());
#endif
#ifdef HD_LOG_OUTPUT_BRAIN
    HiIO::BrainScreen::printLine(line);
#endif
#ifdef HD_LOG_OUTPUT_CONTROLLER
    Controller.Screen.print("%s", line.c_str());
#endif
}

bool drainLogOnce() {
    log_mutex.lock();
    if (log_count == 0) {
        log_mutex.unlock();
        return false;
    }
    const std::string line = pending[log_head];
    pending[log_head].clear();
    log_head = (log_head + 1) % kLogCapacity;
    --log_count;
    const unsigned lost = dropped;
    dropped = 0;
    log_mutex.unlock();
    if (lost) {
        char warning[72];
        snprintf(warning, sizeof(warning), "[W][Log] queue full: dropped %u older lines\r\n", lost);
        outputLine(warning);
    }
    outputLine(line);
    return true;
}

void logWorker() {
    while (true) {
        if (!drainLogOnce()) vex::this_thread::sleep_for(10);
    }
}
} // namespace

#ifdef HD_LOG_ENABLED
void Debug::Log(const std::string& content, LogLevel level) {
    if (level < HD_LOG_MIN_LEVEL) return;
    const std::string line = "[" + std::string(levelTag(level)) + "]" + content + "\r\n";
    log_mutex.lock();
    if (!log_worker_started) {
        new vex::thread(logWorker);
        log_worker_started = true;
    }
    // Preserve recent fault messages on overload instead of stalling motion.
    if (log_count == kLogCapacity) {
        log_head = (log_head + 1) % kLogCapacity;
        --log_count;
        ++dropped;
    }
    pending[(log_head + log_count) % kLogCapacity] = line;
    ++log_count;
    log_mutex.unlock();
}
#else
inline void Debug::Log(const std::string& content, LogLevel level) { return; }
#endif
