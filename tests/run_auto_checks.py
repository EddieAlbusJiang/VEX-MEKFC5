#!/usr/bin/env python3
"""Host checks for the real V3 control code; requires Python 3 and clang++.
Sensor traces and action durations are simulated, not a real-robot timing test.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
VEX = r'''
#pragma once
#include <cstdint>
#include <string>
#include "config.h"
extern unsigned fake_ms;
namespace vex {
enum class rotationUnits { deg };
enum class velocityUnits { pct };
enum class brakeType { hold, brake, coast };
struct mutex { void lock() {} void unlock() {} };
struct timer {
    unsigned start = fake_ms;
    unsigned time() const { return fake_ms - start; }
    void reset() { start = fake_ms; }
    static unsigned system() { return fake_ms; }
};
namespace this_thread { inline void sleep_for(int n) { fake_ms += n; } }
}
'''
DEBUG = r'''
#pragma once
#include <string>
#include <sstream>
#include <iomanip>
namespace Debug {
enum class LogLevel { Info, Warning };
inline void Log(const std::string&, LogLevel) {}
inline std::string doubleToString(double x, int n) {
    std::ostringstream s; s << std::fixed << std::setprecision(n) << x; return s.str();
}
}
'''
MECHANISMS = r'''
#include "mechanism-motion.h"
#include "pid-controller.h"
#include <vector>
#include <utility>
#include <cstdio>
#include <cstdlib>
unsigned fake_ms = 0;
int cancel_at = -1;
bool autonomousStopRequested() { return cancel_at >= 0 && fake_ms >= unsigned(cancel_at); }
struct Motors {
    std::vector<std::pair<unsigned,double>> trace;
    int commands = 0;
    bool stopped = false;
    double last_target = 0, last_speed = 0;
    double position(vex::rotationUnits) {
        double value = trace.front().second;
        for (const auto& s : trace) { if (s.first > fake_ms) break; value = s.second; }
        return value;
    }
    void stop(vex::brakeType) { stopped = true; }
    void setStopping(vex::brakeType) {}
    bool spinToPosition(double t, vex::rotationUnits, double v, vex::velocityUnits, bool wait) {
        if (wait) std::abort();
        ++commands; last_target = t; last_speed = v; stopped = false; return true;
    }
};
int checks = 0;
void check(bool value, const char* what) {
    ++checks; if (!value) { std::fprintf(stderr,"FAIL: %s\n",what); std::exit(1); }
}
void run(const char* name, double target, double tol,
         std::initializer_list<std::pair<unsigned,double>> trace,
         AutoResult expected, unsigned ms, int calls, int cancel = -1) {
    fake_ms = 0; cancel_at = cancel;
    Motors m; m.trace = trace;
    const AutoResult result = runMechanismPosition(m,target,45,tol,2000);
    check(result == expected,name); check(fake_ms == ms,"duration");
    check(m.commands == calls,"no repeated native position commands near target");
    check(m.stopped,"stopped at exit");
    if (calls) check(m.last_target == target && m.last_speed == 45,"native command parameters");
}
void runGrip(const char* name, double target,
             std::initializer_list<std::pair<unsigned,double>> trace,
             AutoResult expected, unsigned ms, bool contact, int cancel=-1) {
    fake_ms=0;cancel_at=cancel;
    Motors m;m.trace=trace;
    const GripContactRule rule{target-P_CLAMP_AUTO_TOL,P_CLAMP_GRIP_MAX_POSITION,
        P_CLAMP_GRIP_MIN_TRAVEL,P_CLAMP_GRIP_STABLE_DEG,P_CLAMP_GRIP_SETTLE_MS};
    bool detected=false;
    const AutoResult result=runMechanismPosition(m,target,20,P_CLAMP_AUTO_TOL,
        P_CLAMP_AUTO_TIMEOUT_MS,&rule,&detected);
    check(result==expected,name);check(fake_ms==ms,"grip confirmation timing");
    check(detected==contact,"contact distinguished from angle arrival");
    check(m.stopped,"grip exit holds motor");
    check(m.commands<=1,"grip does not repeatedly reverse the motor");
}
int main() {
    run("already at target",90,2,{{0,90}},AutoResult::Reached,60,0);
    run("open with rebound",90,2,{{0,0},{200,89},{210,88}},AutoResult::Reached,260,1);
    run("confirmed release at rounded Cp85",90,P_CLAMP_AUTO_OPEN_TOL,
        {{0,0},{200,84.6},{210,85}},AutoResult::Reached,260,1);
    run("insufficient opening still fails",90,P_CLAMP_AUTO_OPEN_TOL,
        {{0,80}},AutoResult::Stalled,700,1);
    run("closing retains tight arrival tolerance",0,P_CLAMP_AUTO_TOL,
        {{0,5}},AutoResult::Stalled,700,1);
    run("close with rebound",0,2,{{0,90},{200,1},{210,3}},AutoResult::Reached,260,1);
    run("lift holds without reversing",179,5,{{0,574},{200,181},{210,185}},AutoResult::Reached,260,1);
    run("large rebound restarts native controller",90,2,
        {{0,0},{200,89},{220,85},{300,89},{310,88}},AutoResult::Reached,360,2);
    run("unreachable open is failure",90,2,{{0,67}},AutoResult::Stalled,700,1);
    run("wide band alone is not arrival",90,2,{{0,87}},AutoResult::Stalled,700,1);
    run("slow unfinished motion times out",90,2,{{0,0},{600,1},{1200,2},{1800,3}},
        AutoResult::Timeout,2000,1);
    run("cancel before motion",90,2,{{0,0}},AutoResult::Cancelled,0,0,0);
    run("cancel active native motion",90,2,{{0,0}},AutoResult::Cancelled,150,1,150);
    runGrip("loaded pickup at Cp3",-9,{{0,90},{200,3}},AutoResult::Reached,400,true);
    runGrip("loaded pickup with close limit at zero",0,{{0,90},{200,3}},AutoResult::Reached,400,true);
    runGrip("small contact vibration allowed",-9,{{0,90},{200,3},{250,3.4}},AutoResult::Reached,400,true);
    runGrip("passing through closed band is not contact",-9,
        {{0,90},{200,6},{280,4},{340,3}},AutoResult::Reached,540,true);
    runGrip("leaving contact band restarts confirmation",-9,
        {{0,90},{200,3},{300,8},{400,3}},AutoResult::Reached,600,true);
    runGrip("jam at large opening still fails",-9,{{0,90},{200,40}},AutoResult::Stalled,1400,false);
    runGrip("no closing travel is not inferred grip",-9,{{0,3}},AutoResult::Stalled,700,false);
    runGrip("cancellation wins over pending contact",-9,{{0,90},{200,3}},AutoResult::Cancelled,350,false,350);
    runGrip("empty close at zero still finishes normally",0,{{0,90},{200,1}},AutoResult::Reached,260,false);
    runGrip("empty close at negative target still finishes normally",-9,{{0,90},{200,-8}},AutoResult::Reached,260,false);
    fake_ms=0;
    PIDController pid(PIDParam{2.5,0.06,0});
    pid.setUseSeconds(true);
    fake_ms=10;pid.update(20);
    check(std::fabs(pid.output()-50)<1e-9,"first PID sample has no derivative kick");
    fake_ms=20;pid.update(19);
    check(std::fabs(pid.output()-41.5)<1e-9,"subsequent derivative still damps motion");
    pid.reset();fake_ms=30;pid.update(-20);
    check(std::fabs(pid.output()+50)<1e-9,"reset clears derivative history");
    std::printf("Mechanism/PID: %d checks passed\n",checks);
}
'''
INSTANCE = r'''
#pragma once
#include "autonomous.h"
#include <utility>
#include <vector>
extern volatile bool Auto_Active;
extern int per_action_ms, move_count, fail_move, open_count, fail_open, cancel_at;
extern bool lift_pending, clamp_pending, all_stopped;
extern unsigned lift_finish, clamp_finish;
extern std::vector<std::pair<double,double>> positions;
extern std::vector<double> turns;
extern std::vector<double> turn_speeds;
extern std::vector<double> clamp_targets;
extern int via_count, lift_action_factor;
extern bool fail_lift574;
extern bool fail_grip;
extern int grip_count;
inline AutoResult spend(int duration) {
    unsigned start = fake_ms;
    while (fake_ms-start < unsigned(duration)) {
        if (cancel_at >= 0 && fake_ms >= unsigned(cancel_at)) requestAutonomousStop();
        if (autonomousStopRequested()) return AutoResult::Cancelled;
        vex::this_thread::sleep_for(10);
    }
    return AutoResult::Reached;
}
struct FakeChassis {
    AutoResult AutoMoveTo(double x,double y) {
        ++move_count;positions.push_back({x,y});
        AutoResult r=spend(per_action_ms);
        return move_count==fail_move ? AutoResult::Timeout : r;
    }
    AutoResult AutoRotateTo(double h,double speed) { turns.push_back(h);turn_speeds.push_back(speed);return spend(per_action_ms); }
    AutoResult AutoMoveVia(double x,double y,double end_x,double end_y) {
        ++via_count;
        const AutoResult first=AutoMoveTo(x,y);
        return first==AutoResult::Reached ? AutoMoveTo(end_x,end_y) : first;
    }
    AutoResult waitUntilDone() { return AutoResult::Reached; }
    void stop(vex::brakeType) { all_stopped=true; }
    void setIdleBrake(bool) {}
};
struct FakeLift {
    double target=0;
    AutoResult AutoLiftTo(double) { return spend(per_action_ms); }
    void AutoLiftTo(double height,void (*)(void)) {
        target=height;lift_pending=true;lift_finish=fake_ms+lift_action_factor*per_action_ms;
    }
    bool autoDone() { return !lift_pending || fake_ms>=lift_finish; }
    AutoResult autoResult() { return target==574 && fail_lift574 ? AutoResult::Timeout : AutoResult::Reached; }
    AutoResult waitUntilDone() {
        while(lift_pending && fake_ms<lift_finish) {
            if(autonomousStopRequested()) {lift_pending=false;return AutoResult::Cancelled;}
            vex::this_thread::sleep_for(10);
        }
        lift_pending=false;return autoResult();
    }
    void stop() {}
};
struct FakeClamp {
    AutoResult AutoClampGrip(double target) {
        ++grip_count;
        const AutoResult result=AutoClampTo(target);
        return fail_grip ? AutoResult::Stalled : result;
    }
    AutoResult AutoClampTo(double target) {
        clamp_targets.push_back(target);
        if (target == 90) ++open_count;
        AutoResult r=spend(per_action_ms);
        return target == 90 && open_count==fail_open ? AutoResult::Stalled : r;
    }
    void AutoClampTo(double target,void (*)(void)) {
        clamp_targets.push_back(target);
        clamp_pending=true;clamp_finish=fake_ms+per_action_ms;
    }
    AutoResult waitUntilDone() {
        while(clamp_pending && fake_ms<clamp_finish) {
            if(autonomousStopRequested()) {clamp_pending=false;return AutoResult::Cancelled;}
            vex::this_thread::sleep_for(10);
        }
        clamp_pending=false;return AutoResult::Reached;
    }
};
struct FakeOdom { void resetRotation() {} void resetWheel() {} };
struct FakeMotor { void stop(vex::brakeType) {} };
extern FakeChassis Chassis;
extern FakeLift Mech_Lift;
extern FakeClamp Mech_Clamp;
extern FakeOdom Odom;
extern FakeMotor Motor_Clamp;
'''
ROUTE = r'''
#include "instance.h"
#include <cstdio>
#include <cstdlib>
unsigned fake_ms=0;
volatile bool Auto_Active=false;
int per_action_ms=100,move_count=0,fail_move=-1,open_count=0,fail_open=-1,cancel_at=-1;
bool lift_pending=false,clamp_pending=false,all_stopped=false;
unsigned lift_finish=0,clamp_finish=0;
std::vector<std::pair<double,double>> positions;
std::vector<double> turns;
std::vector<double> turn_speeds;
std::vector<double> clamp_targets;
int via_count=0,lift_action_factor=2;
bool fail_lift574=false;
bool fail_grip=false;
int grip_count=0;
FakeChassis Chassis;FakeLift Mech_Lift;FakeClamp Mech_Clamp;FakeOdom Odom;FakeMotor Motor_Clamp;
int checks=0;
void check(bool ok,const char* label) {++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",label);std::exit(1);}}
void reset() {
    fake_ms=0;per_action_ms=100;move_count=open_count=0;fail_move=fail_open=cancel_at=-1;
    lift_pending=clamp_pending=all_stopped=false;positions.clear();turns.clear();clamp_targets.clear();
    turn_speeds.clear();
    via_count=0;lift_action_factor=2;fail_lift574=false;
    fail_grip=false;grip_count=0;
    Auto_Mode=AutonomousMode::Left;
}
void exited() {
    check(!Auto_Active && !autonomousTelemetry().active,"manual control released");
    check(!lift_pending && !clamp_pending,"no background mechanism after exit");
    check(all_stopped,"chassis stopped at exit");
}
int main() {
    reset();autonomous();
    const std::vector<std::pair<double,double>> expected={{7,0},{7,-46},{19,-51},{6,-46},
        {4,47},{3,47},{18,45},{8,45},{8,0},{42,0},{-19,0},{102,-17}};
    check(positions==expected,"updated pickup via point and remaining waypoint order");
    check(turns==std::vector<double>({270,90,267}),"headings preserved");
    check(turn_speeds==std::vector<double>({45,65,45}),"only first-placement return turn is faster");
    check(clamp_targets==std::vector<double>({0,90,-9,90}),"original clamp targets preserved");
    check(grip_count==1,"loaded grip used only for second pickup");
    check(open_count==2,"both placements release");
    check(via_count==1,"pending lift keeps separate first approach despite stale OK result");
    check(autonomousTelemetry().result==AutoResult::Reached,"DONE only after entire route");
    const unsigned finished_step_ms=autonomousTelemetry().step_elapsed_ms;
    fake_ms+=500;
    check(autonomousTelemetry().step_elapsed_ms==finished_step_ms,"final step time remains visible");
    exited();
    reset();lift_action_factor=1;autonomous();
    check(via_count==2 && positions==expected,"completed lift enables first-placement continuous route");
    check(autonomousTelemetry().result==AutoResult::Reached,"blended route completes normally");exited();
    reset();lift_action_factor=1;fail_lift574=true;autonomous();
    check(move_count==1 && open_count==0,"completed but failed lift blocks first placement");
    check(autonomousTelemetry().result==AutoResult::Timeout,"lift failure remains visible");exited();
    reset();fail_move=2;autonomous();
    check(move_count==2 && open_count==0,"move failure prevents later release");
    check(autonomousTelemetry().result==AutoResult::Timeout,"move failure surfaced");exited();
    reset();fail_grip=true;autonomous();
    check(turns==std::vector<double>({270,90}) && open_count==1,
          "failed grip prevents loaded turn and second release");
    check(autonomousTelemetry().result==AutoResult::Stalled,"grip failure remains visible");exited();
    reset();fail_open=1;autonomous();
    check(move_count==3 && open_count==1,"failed opening prevents retreat");
    check(autonomousTelemetry().result==AutoResult::Stalled,"open stall surfaced");exited();
    reset();fail_move=5;autonomous();
    check(move_count==5 && clamp_targets==std::vector<double>({0,90}),
          "failed pickup via point prevents final approach and gripping");exited();
    reset();per_action_ms=2000;autonomous();
    check(fake_ms==30000,"deadline stops active action");
    check(autonomousTelemetry().result==AutoResult::Timeout,"deadline is not DONE");
    check(positions.size()<expected.size(),"deadline does not claim full route");exited();
    reset();cancel_at=250;autonomous();
    check(autonomousTelemetry().result==AutoResult::Cancelled,"B stop cancellation");exited();
    reset();Auto_Mode=AutonomousMode::Right;autonomous();
    check(move_count==0 && open_count==0 && !Auto_Active,"L2 remains inactive");
    std::printf("Route: %d checks passed\n",checks);
}
'''

CHASSIS_VEX = r'''
#include <vector>
#include <utility>
namespace vex {
enum class directionType { fwd, rev };
struct thread { explicit thread(void (*)(void)) {} };
struct motor {
    std::vector<std::pair<unsigned,double>> commands;
    std::vector<unsigned> stops;
    double value=0;
    void spin(directionType d,double v,velocityUnits) {
        value=d==directionType::fwd?v:-v;commands.push_back({fake_ms,value});
    }
    void stop(brakeType) {value=0;stops.push_back(fake_ms);}
    double velocity(velocityUnits) {return value;}
};
}
'''
ODOM = r'''
#pragma once
#include "vex.h"
struct Pose {unsigned ms;double x,y,h;};
class Odometry {
  public:
    std::vector<Pose> trace;
    Pose pose() const {
        Pose p=trace.front();for(const auto& s:trace){if(s.ms>fake_ms)break;p=s;}return p;
    }
    double wheelX() const {return pose().x;}
    double wheelY() const {return pose().y;}
    double heading() const {return pose().h;}
};
'''
CHASSIS = r'''
#include "x-drive.h"
#include "odometry.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
unsigned fake_ms=0;
int cancel_at=-1,checks=0;
bool autonomousStopRequested(){return cancel_at>=0 && fake_ms>=unsigned(cancel_at);}
void check(bool ok,const char* label){++checks;if(!ok){std::fprintf(stderr,"FAIL: %s\n",label);std::exit(1);}}
void run(bool turn, double target, std::initializer_list<Pose> trace,
         AutoResult result, unsigned duration, int cancel=-1, double speed=45) {
    fake_ms=0;cancel_at=cancel;
    vex::motor a,b,c,d;Odometry odom;odom.trace=trace;
    XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
    if(turn)chassis.runRotate(target,speed);else chassis.runMove(target,0,speed);
    check(chassis.autoResult()==result,"chassis result");
    check(fake_ms==duration,"chassis duration/settle reset");
    check(a.value==0&&b.value==0&&c.value==0&&d.value==0,"all wheels stopped on exit");
    if(cancel==0)check(a.commands.empty(),"cancel before first motor command");
}
int main(){
    run(false,10,{{0,0,0,0},{100,9.5,0,0},{110,8.6,0,0}},AutoResult::Reached,120);
    run(false,10,{{0,0,0,0},{100,9.5,0,0},{120,8.4,0,0},{200,9.5,0,0}},AutoResult::Reached,220);
    run(false,10,{{0,8.6,0,0}},AutoResult::Timeout,5000);
    run(false,10,{{0,0,0,0}},AutoResult::Cancelled,0,0);
    run(false,10,{{0,0,0,0}},AutoResult::Cancelled,150,150);
    run(true,0,{{0,0,0,359},{10,0,0,358.2}},AutoResult::Reached,20);
    run(true,0,{{0,0,0,359},{20,0,0,357.9},{100,0,0,359}},AutoResult::Reached,120);
    run(true,0,{{0,0,0,358.2}},AutoResult::Timeout,5000);
    run(true,90,{{0,0,0,0}},AutoResult::Cancelled,0,0);
    run(true,90,{{0,0,0,0}},AutoResult::Cancelled,150,150);
    for(double speed : {45.0,3.0}){
        fake_ms=0;cancel_at=10;
        vex::motor a,b,c,d;Odometry odom;odom.trace={{0,0,0,2}};
        XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
        chassis.runRotate(0,speed);
        const double expected=-(speed<7?speed:7)*P_XDRIVE_MAX_OUTPUT;
        check(a.commands.size()==1&&std::fabs(a.commands[0].second-expected)<1e-9,
              "rotation minimum overcomes friction but respects speed cap/direction");
        check(std::fabs(c.commands[0].second+expected)<1e-9,"opposite wheels rotate correctly");
    }
    fake_ms=0;cancel_at=20;
    vex::motor a,b,c,d;Odometry odom;odom.trace={{0,0,0,10},{10,0,0,8}};
    XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);chassis.runRotate(0,45);
    check(a.commands.size()==2 && a.commands[1].second>0 && a.commands[1].second<7*P_XDRIVE_MAX_OUTPUT,
          "derivative braking is not amplified by minimum turn output");
    {
        fake_ms=0;cancel_at=-1;
        vex::motor a,b,c,d;Odometry odom;
        odom.trace={{0,6,-46,90},{100,3,37,90},{200,2,47,90}};
        XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
        chassis.runMoveVia(3,37,2,47,45);
        check(chassis.autoResult()==AutoResult::Reached && fake_ms==220,"via then final endpoint settle");
        check(a.stops==std::vector<unsigned>({200,220}),"no braking at via point");
        bool handoff=false;for(const auto& cmd:a.commands)if(cmd.first==100 && cmd.second!=0)handoff=true;
        check(handoff,"next movement commanded in same control tick");
        check(a.value==0&&b.value==0&&c.value==0&&d.value==0,"final endpoint stops all motors");
    }
    {
        fake_ms=0;cancel_at=-1;
        vex::motor a,b,c,d;Odometry odom;
        odom.trace={{0,6,-46,90},{100,4,47,90},{200,3,47,90}};
        XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
        chassis.runMoveVia(4,47,3,47,45);
        check(chassis.autoResult()==AutoResult::Reached && fake_ms==220,
              "one-centimeter pickup adjustment is not skipped");
        check(a.stops==std::vector<unsigned>({200,220}),"short via only brakes at final target");
        bool adjustment=false;for(const auto& cmd:a.commands)if(cmd.first==100 && cmd.second!=0)adjustment=true;
        check(adjustment,"one-centimeter adjustment issues a motion command");
    }
    {
        fake_ms=0;cancel_at=-1;
        vex::motor a,b,c,d;Odometry odom;odom.trace={{0,4,47,90}};
        XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
        chassis.runMoveVia(4,47,4,47,45);
        check(chassis.autoResult()==AutoResult::Reached && fake_ms==20,
              "identical via and target use normal stopping tolerance");
    }
    for(int cancel : {-1,50,150}) {
        fake_ms=0;cancel_at=cancel;
        vex::motor a,b,c,d;Odometry odom;
        odom.trace=cancel<0 ? std::vector<Pose>{{0,6,-46,90}} :
                             std::vector<Pose>{{0,6,-46,90},{100,3,37,90}};
        XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
        chassis.runMoveVia(3,37,2,47,45);
        check(chassis.autoResult()==(cancel<0?AutoResult::Timeout:AutoResult::Cancelled),
              "via failure or cancellation before/after handoff propagated");
        check(fake_ms==unsigned(cancel<0?5000:cancel),"via failure ends without another move");
        check(a.value==0&&b.value==0&&c.value==0&&d.value==0,"via failure stops all motors");
    }
    {
        fake_ms=0;cancel_at=10;
        vex::motor a,b,c,d;Odometry odom;odom.trace={{0,0,0,0}};
        XDrive chassis(a,b,c,d,odom);chassis.setIdleBrake(false);
        chassis.runMove(100,0,100);
        check(a.commands.front().second>75 && a.commands.front().second<=85,
              "long straight travel gains speed within new automatic cap");
        a.commands.clear();fake_ms=0;
        chassis.runMove(5,0,100);
        check(std::fabs(a.commands.front().second-9.375)<1e-9,
              "final approach retains original proportional output and cap");
        a.commands.clear();chassis.drive(100,0,0);
        check(std::fabs(a.commands.front().second-75)<1e-9,"manual drive cap unchanged");
        a.commands.clear();b.commands.clear();c.commands.clear();d.commands.clear();fake_ms=0;
        chassis.runMove(100,100,100);
        check(std::fabs(a.commands.front().second)<=85 && std::fabs(b.commands.front().second)<=85 &&
              std::fabs(c.commands.front().second)<=85 && std::fabs(d.commands.front().second)<=85,
              "diagonal transit still normalizes wheel outputs");
    }
    std::printf("Chassis: %d checks passed\n",checks);
}
'''

with tempfile.TemporaryDirectory(prefix='vex-auto-check-') as directory:
    work=Path(directory)
    for name in ['config.h','auto-motion.h','mechanism-motion.h','autonomous.h','pid-controller.h','calc.h']:
        shutil.copyfile(ROOT/'include'/name,work/name)
    for name in ['pid-controller.cpp','autonomous.cpp','x-drive.cpp','calc.cpp']:
        shutil.copyfile(ROOT/'src'/name,work/name)
    (work/'vex.h').write_text(VEX+CHASSIS_VEX)
    (work/'odometry.h').write_text(ODOM)
    # Test access to the actual blocking control loops without spawning workers.
    (work/'x-drive.h').write_text((ROOT/'include/x-drive.h').read_text().replace('  private:', '  public:'))
    (work/'HiDebug.h').write_text(DEBUG)
    (work/'instance.h').write_text(INSTANCE)
    (work/'mechanism-test.cpp').write_text(MECHANISMS)
    (work/'route-test.cpp').write_text(ROUTE)
    (work/'chassis-test.cpp').write_text(CHASSIS)
    for name,sources in [('mechanisms',['mechanism-test.cpp','pid-controller.cpp']),
                         ('route',['route-test.cpp','autonomous.cpp']),
                         ('chassis',['chassis-test.cpp','x-drive.cpp','pid-controller.cpp','calc.cpp'])]:
        output=work/name
        subprocess.run(['clang++','-std=c++11','-Wall','-Wextra','-I',str(work),
                        *[str(work/s) for s in sources],'-o',str(output)],check=True)
        subprocess.run([str(output)],check=True)

    # Exercise the production logger with slow output replaced by a recording
    # sink. The fake VEX thread does not run; explicitly drain queued output.
    logs=work/'logging'
    (logs/'HiDebug').mkdir(parents=True)
    (logs/'vex.h').write_text(VEX+CHASSIS_VEX)
    shutil.copyfile(ROOT/'include/config.h',logs/'config.h')
    shutil.copyfile(ROOT/'src/HiDebug/log.cpp',logs/'log.cpp')
    (logs/'HiDebug/log.h').write_text((ROOT/'include/HiDebug/log.h').read_text().replace('#include <bits/basic_string.h>',''))
    (logs/'HiDebug/conf.h').write_text('#define HD_LOG_ENABLED\n#define HD_LOG_OUTPUT_BRAIN\n')
    (logs/'instance.h').write_text('#pragma once\n')
    (logs/'HiIO.h').write_text(r'''#pragma once
#include <string>
#include <vector>
extern std::vector<std::string> output_lines;
namespace HiIO { namespace BrainScreen {
inline void printLine(const std::string& s) {output_lines.push_back(s);}
}}
''')
    (logs/'test.cpp').write_text(r'''
#include "log.cpp"
#include <cstdlib>
unsigned fake_ms=0;
std::vector<std::string> output_lines;
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok){fprintf(stderr,"FAIL: %s\n",label);exit(1);}}
int main(){
    Debug::Log("first",LogLevel::Info);
    Debug::Log("filtered",LogLevel::Debug);
    Debug::Log("second",LogLevel::Warning);
    check(output_lines.empty(),"logging caller never writes to slow output");
    while(drainLogOnce()){}
    check(output_lines==std::vector<std::string>({"[I]first\r\n","[W]second\r\n"}),
          "background output preserves order and filtering");
    output_lines.clear();
    for(int i=0;i<300;++i){char s[20];snprintf(s,sizeof(s),"line %d",i);Debug::Log(s,LogLevel::Info);}
    check(output_lines.empty(),"full queue never falls back to blocking output");
    while(drainLogOnce()){}
    check(output_lines.front().find("[W][Log] queue full") == 0,"overflow is visible");
    check(output_lines.back()=="[I]line 299\r\n","newest messages survive overload");
    check(!drainLogOnce(),"drained queue stays empty");
    printf("Logging: %d checks passed\n",checks);
}
''')
    subprocess.run(['clang++','-std=c++11','-Wall','-Wextra','-I',str(logs),str(logs/'test.cpp'),'-o',str(logs/'test')],check=True)
    subprocess.run([str(logs/'test')],check=True)
