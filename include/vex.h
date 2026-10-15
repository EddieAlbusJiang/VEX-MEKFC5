#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v5.h"
#include "v5_vcs.h"

#include "config.h"

// Control-loop tick used for the clamp position-lock compensation (milliseconds).
#ifndef TICK_MS
#define TICK_MS 10
#endif


#define waitUntil(condition)                                                   \
  do {                                                                         \
    vex::wait(5, vex::timeUnits::msec);                                        \
  } while (!(condition))

#define repeat(iterations)                                                     \
  for (int iterator = 0; iterator < iterations; iterator++)
