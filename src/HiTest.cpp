#include <HiTest/conf.h>
#include <HiTest.h>
#include "HiDebug.h"
#include "instance.h"

namespace {
// Unit test: print heading + wheel-odometry + IMU position every 500 ms.
void displacementTest() {
    while (true) {
        // Line 1: heading + wheel odometry.
        Debug::Log("[Test] H=" + Debug::doubleToString(Odom.heading(), 1) +
                       " W=(" + Debug::doubleToString(Odom.wheelX(), 1) + "," +
                       Debug::doubleToString(Odom.wheelY(), 1) + ")",
                   Debug::LogLevel::Debug);

        // Line 2: raw IMU acceleration (g).
        Debug::Log("[Test] A=(" +
                       Debug::doubleToString(Inertial.acceleration(vex::axisType::xaxis), 3) + "," +
                       Debug::doubleToString(Inertial.acceleration(vex::axisType::yaxis), 3) + "," +
                       Debug::doubleToString(Inertial.acceleration(vex::axisType::zaxis), 3) + ")",
                   Debug::LogLevel::Debug);

        vex::this_thread::sleep_for(1000);
    }
}
} // namespace
#ifdef HT_TEST_ENABLED
void HiTest() {
    // Run the test on its own thread so it does not block the caller.
    static vex::thread test_thread(displacementTest);
}
#else
inline void HiTest(){return; }
#endif
