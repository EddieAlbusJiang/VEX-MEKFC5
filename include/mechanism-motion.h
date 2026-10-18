#pragma once

#include "auto-motion.h"
#include "vex.h"
#include <cmath>

// Opt-in loaded-clamp contact inference, not a general stall-as-success rule.
struct GripContactRule {
    double min_position;
    double max_position;
    double min_travel;
    double stable_delta;
    unsigned settle_ms;
};

// Submit one native position command. Avoid a second outer speed loop that
// repeatedly reverses at the setpoint with a fixed minimum motor speed.
template <typename Motors>
AutoResult runMechanismPosition(Motors& motors, double target, double speed,
                                double tolerance, int timeout_ms,
                                const GripContactRule* grip = nullptr,
                                bool* contact_reached = nullptr) {
    if (contact_reached) *contact_reached = false;
    if (autonomousStopRequested()) {
        motors.stop(vex::brakeType::hold);
        return AutoResult::Cancelled;
    }

    vex::timer timer;
    vex::timer settle;
    vex::timer progress;
    double last_progress = motors.position(vex::rotationUnits::deg);
    const double start_position = last_progress;
    vex::timer contact_timer;
    double contact_anchor = start_position;
    bool contact_candidate = false;
    bool holding = std::fabs(target - last_progress) <= tolerance;
    if (holding) {
        motors.stop(vex::brakeType::hold);
    } else {
        motors.setStopping(vex::brakeType::hold);
        motors.spinToPosition(target, vex::rotationUnits::deg,
                              std::fabs(speed), vex::velocityUnits::pct, false);
    }

    AutoResult result = AutoResult::Timeout;
    while (timer.time() < static_cast<unsigned>(timeout_ms)) {
        if (autonomousStopRequested()) {
            result = AutoResult::Cancelled;
            break;
        }
        const double pos = motors.position(vex::rotationUnits::deg);
        const double error = std::fabs(target - pos);
        const bool in_band = error <= tolerance ||
            (holding && error <= tolerance + P_MECH_HYSTERESIS);

        if (in_band) {
            contact_candidate = false;
            if (!holding) {
                // Capture the reached position once instead of alternating
                // spin/stop on every tick near the target.
                motors.stop(vex::brakeType::hold);
                settle.reset();
                holding = true;
            }
            if (settle.time() >= P_MECH_AUTO_SETTLE_MS) {
                result = AutoResult::Reached;
                break;
            }
            progress.reset();
            last_progress = pos;
        } else {
            if (holding) {
                holding = false;
                progress.reset();
                last_progress = pos;
                motors.spinToPosition(target, vex::rotationUnits::deg,
                                      std::fabs(speed), vex::velocityUnits::pct, false);
            }
            const bool near_closed_after_travel = grip &&
                start_position > target + tolerance &&
                start_position - pos >= grip->min_travel &&
                pos >= grip->min_position && pos <= grip->max_position;
            if (near_closed_after_travel) {
                if (!contact_candidate || std::fabs(pos - contact_anchor) > grip->stable_delta) {
                    contact_candidate = true;
                    contact_anchor = pos;
                    contact_timer.reset();
                } else if (contact_timer.time() >= grip->settle_ms) {
                    // Keep closing during confirmation; hold the reached
                    // position on exit. This infers contact, not object presence.
                    if (contact_reached) *contact_reached = true;
                    result = AutoResult::Reached;
                    break;
                }
            } else {
                contact_candidate = false;
            }
            // Outside the explicit loaded-contact rule, a blockage is failure.
            if (progress.time() >= P_MECH_PROGRESS_MS) {
                if (std::fabs(pos - last_progress) < P_MECH_PROGRESS_DEG) {
                    result = AutoResult::Stalled;
                    break;
                }
                progress.reset();
                last_progress = pos;
            }
        }
        vex::this_thread::sleep_for(10);
    }
    motors.stop(vex::brakeType::hold);
    return result;
}
