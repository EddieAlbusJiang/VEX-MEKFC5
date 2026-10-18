#include "autonomous.h"
#include "HiDebug.h"
#include "instance.h"

volatile AutonomousMode Auto_Mode = AutonomousMode::Right;

namespace {
vex::mutex run_mutex;
AutonomousTelemetry report{false, false, 0, "READY", 0, 0, AutoResult::Idle};
unsigned started_ms = 0;
unsigned step_started_ms = 0;
bool cancel_requested = false;
bool deadline_expired = false;

void step(const char* action) {
    run_mutex.lock();
    ++report.step;
    report.action = action;
    step_started_ms = vex::timer::system();
    report.step_elapsed_ms = 0;
    const unsigned number = report.step;
    run_mutex.unlock();
    Debug::Log("[V3 S" + Debug::doubleToString(number, 0) + "] " + action,
               Debug::LogLevel::Info);
}

bool checked(AutoResult result) {
    const bool cancelled = autonomousStopRequested();
    if (result == AutoResult::Reached && !cancelled) return true;
    run_mutex.lock();
    report.result = deadline_expired ? AutoResult::Timeout :
        (cancelled ? AutoResult::Cancelled : result);
    cancel_requested = true;
    run_mutex.unlock();
    return false;
}

bool move(double x, double y, const char* name) {
    step(name);
    return checked(Chassis.AutoMoveTo(x, y));
}

bool turn(double heading, double speed = 45) {
    step("TURN");
    return checked(Chassis.AutoRotateTo(heading, speed));
}

bool liftTo(double height, const char* name) {
    step(name);
    return checked(Mech_Lift.AutoLiftTo(height));
}

bool waitLift(const char* name) {
    step(name);
    return checked(Mech_Lift.waitUntilDone());
}

bool openClamp() {
    step("OPEN");
    return checked(Mech_Clamp.AutoClampTo(90));
}

bool closeClamp() {
    step("CLOSE");
    // Preserve target/limits, but allow near-closed contact with the object.
    return checked(Mech_Clamp.AutoClampGrip(-9));
}

bool runLeftRoute() {
    // Startup mechanisms overlap; grip and low position must finish before drive.
    step("START");
    // Keep the team's manual zero reference; do not establish a new zero here.
    Mech_Clamp.AutoClampTo(0, nullptr);
    Mech_Lift.AutoLiftTo(0, nullptr);
    if (!checked(Mech_Clamp.waitUntilDone()) ||
        !checked(Mech_Lift.waitUntilDone())) return false;

    if (!move(7, 0, "CLEAR")) return false;
    Mech_Lift.AutoLiftTo(574, nullptr);
    if (!turn(270)) return false;
    if (Mech_Lift.autoDone()) {
        // Blend the two moves only after the lift has actually reached 574.
        // A stale result from the previous lift command is not sufficient.
        if (!checked(Mech_Lift.autoResult())) return false;
        step("PLACE1");
        if (!checked(Chassis.AutoMoveVia(7, -46, 19, -51))) return false;
    } else {
        // Continue lifting during travel; wait at the clear point if needed.
        if (!move(7, -46, "TRAVEL")) return false;
        if (!waitLift("L574")) return false;
        if (!move(19, -51, "PLACE1")) return false;
    }
    if (!liftTo(179, "L179")) return false;
    if (!openClamp()) return false;
    if (!move(6, -46, "BACK1")) return false;

    if (!turn(90, P_AUTO_RETURN_TURN_SPEED)) return false;
    step("PICK2");
    if (!checked(Chassis.AutoMoveVia(4, 47, 3, 47))) return false;
    if (!closeClamp()) return false;
    // Let the held object settle before lifting and rotating. Keep cancellation
    // and the overall deadline responsive during this intentional pause.
    step("HOLD");
    vex::timer grip_pause;
    while (grip_pause.time() < P_AUTO_GRIP_PAUSE_MS) {
        if (autonomousStopRequested()) return checked(AutoResult::Cancelled);
        vex::this_thread::sleep_for(10);
    }
    if (!checked(AutoResult::Reached)) return false;
    Mech_Lift.AutoLiftTo(1797, nullptr);
    if (!turn(267)) return false;
    // Ensure the carried load is high enough before the final approach.
    if (!waitLift("L1797")) return false;
    if (!move(18, 45, "PLACE2")) return false;
    if (!liftTo(1534, "L1534")) return false;
    if (!openClamp()) return false;
    if (!move(8, 45, "BACK2")) return false;

    // Preserve all recorded tail waypoints and their order.
    if (!move(8, 0, "EXIT")) return false;
    if (!move(42, 0, "PUSH1")) return false;
    if (!move(-19, 0, "BACK3")) return false;
    return move(102, -17, "PUSH2");
}
} // namespace

bool autonomousStopRequested() {
    run_mutex.lock();
    if (report.active && vex::timer::system() - started_ms >= P_AUTO_RUN_LIMIT_MS) {
        deadline_expired = true;
        cancel_requested = true;
    }
    const bool stop = report.active && cancel_requested;
    run_mutex.unlock();
    return stop;
}

void requestAutonomousStop() {
    run_mutex.lock();
    if (report.active) cancel_requested = true;
    run_mutex.unlock();
}

AutonomousTelemetry autonomousTelemetry() {
    run_mutex.lock();
    AutonomousTelemetry snapshot = report;
    if (snapshot.active) {
        const unsigned now = vex::timer::system();
        snapshot.elapsed_ms = now - started_ms;
        snapshot.step_elapsed_ms = now - step_started_ms;
    }
    run_mutex.unlock();
    return snapshot;
}

void autonomous() {
    if (Auto_Mode != AutonomousMode::Left) {
        Debug::Log("[V3] Select L1; L2 inactive", Debug::LogLevel::Info);
        return;
    }
    // Ignore duplicate competition/manual triggers while a run owns the motors.
    run_mutex.lock();
    if (report.active) {
        run_mutex.unlock();
        return;
    }
    report = AutonomousTelemetry{true, true, 0, "START", 0, 0, AutoResult::Running};
    started_ms = vex::timer::system();
    step_started_ms = started_ms;
    cancel_requested = false;
    deadline_expired = false;
    Auto_Active = true;
    run_mutex.unlock();

    Debug::Log("[AUTO V3] START L1 / 30s", Debug::LogLevel::Info);
    Odom.resetRotation();
    Odom.resetWheel();
    Chassis.setIdleBrake(false);
    const bool completed = runLeftRoute();

    // On failure/deadline, drain cancelled background commands before handing
    // hardware back to manual control. A 30s cutoff is never reported as DONE.
    const bool stop_requested = autonomousStopRequested();
    run_mutex.lock();
    const bool success = completed && !stop_requested;
    if (!success && report.result == AutoResult::Running) {
        report.result = deadline_expired ? AutoResult::Timeout : AutoResult::Cancelled;
    }
    cancel_requested = true;
    run_mutex.unlock();
    Mech_Lift.waitUntilDone();
    Mech_Clamp.waitUntilDone();
    Chassis.waitUntilDone();
    Chassis.stop(vex::brakeType::hold);
    Mech_Lift.stop();
    Motor_Clamp.stop(vex::brakeType::hold);
    Chassis.setIdleBrake(true);

    run_mutex.lock();
    report.elapsed_ms = vex::timer::system() - started_ms;
    report.step_elapsed_ms = vex::timer::system() - step_started_ms;
    report.active = false;
    if (success) report.result = AutoResult::Reached;
    const AutonomousTelemetry finished = report;
    Auto_Active = false;
    run_mutex.unlock();
    Debug::Log("[AUTO V3] " + std::string(success ? "DONE" : "STOP") + " " +
                   Debug::doubleToString(finished.elapsed_ms / 1000.0, 2) + "s S" +
                   Debug::doubleToString(finished.step, 0) + " " + autoResultName(finished.result),
               success ? Debug::LogLevel::Info : Debug::LogLevel::Warning);
}
