#pragma once

#include <cmath>
#include <cstdio>
#include <string>

namespace Debug {

// Convert a double to a string with the given number of decimal places.
// Special values: NaN -> "NaN", +Inf -> "Infinity", -Inf -> "-Infinity".
inline std::string doubleToString(double value, unsigned int precision = 2) {
    if (std::isnan(value)) {
        return "NaN";
    }
    if (std::isinf(value)) {
        return value > 0.0 ? "Infinity" : "-Infinity";
    }
    std::string buffer(precision + 32, '\0');
    snprintf(&buffer[0], buffer.size(), "%.*f", static_cast<int>(precision), value);
    return std::string(buffer.c_str()); // trims at the terminating '\0'
}
} // namespace Debug
