#include "HiIO/brain_screen.h"

#include <string>

namespace {
constexpr int kBrainRows = 12;
std::string brain_lines[kBrainRows];
int brain_line_count = 0;
vex::brain* brain_ptr = nullptr;
vex::mutex brain_mutex;

void redrawBrainScreen() {
    brain_ptr->Screen.clearScreen();
    brain_ptr->Screen.setCursor(1, 1);
    for (int i = 0; i < brain_line_count; ++i) {
        brain_ptr->Screen.print("%s", brain_lines[i].c_str());
        brain_ptr->Screen.newLine();
    }
}
} // namespace

void HiIO::BrainScreen::setBrain(vex::brain& brain) {
    brain_ptr = &brain;
    clear(); // reset the screen and the rolling buffer
}

void HiIO::BrainScreen::printLine(const std::string& line) {
    if (brain_ptr == nullptr) {
        return;
    }
    brain_mutex.lock();
    if (brain_line_count < kBrainRows) {
        // Still filling the screen: append and print incrementally.
        brain_lines[brain_line_count++] = line;
        brain_ptr->Screen.print("%s", line.c_str());
        brain_ptr->Screen.newLine();
    } else {
        // Full: drop the oldest line, append the new one at the bottom, then
        // repaint the rolling window.
        for (int i = 1; i < kBrainRows; ++i) {
            brain_lines[i - 1] = brain_lines[i];
        }
        brain_lines[kBrainRows - 1] = line;
        redrawBrainScreen();
    }
    brain_mutex.unlock();
}

void HiIO::BrainScreen::clear() {
    if (brain_ptr == nullptr) {
        return;
    }
    brain_mutex.lock();
    brain_ptr->Screen.clearScreen();
    brain_line_count = 0;
    for (int i = 0; i < kBrainRows; ++i) {
        brain_lines[i].clear();
    }
    brain_mutex.unlock();
}
