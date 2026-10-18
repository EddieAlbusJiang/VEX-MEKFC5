#pragma once

#include "vex.h"
#include <string>

namespace HiIO {

// Manages the rolling text buffer shown on the V5 Brain screen. The default
// Brain font (mono20) shows 12 rows; when a new line would exceed that, the
// oldest is dropped and the screen is repainted.
class BrainScreen {
  public:
    // Inject the Brain to use. Must be called before any output.
    static void setBrain(vex::brain& brain);

    static void printLine(const std::string& line); // append a line, scroll if full
    static void clear();                            // clear the screen and the buffer
};

} // namespace HiIO
