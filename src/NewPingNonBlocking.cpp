#include "NewPingNonBlocking.h"

#include <Arduino.h>
#include <NewPing.h>

#include <cmath>

/**
 * @brief Calculates the average (mean) echo time from the last ITERATIONS pings (discounting NO_ECHO pings).
 * NOTE: This does NOT use any delays. If you need to delay, do it in the loop that calls this function.
 *
 * @param max_cm_distance
 * @return unsigned long
 */
unsigned long NewPingNonBlocking::ping_average(unsigned int max_cm_distance = 0) {
  auto pingTime = ping(max_cm_distance);

  // If NO_ECHO, skip updating the average
  if (pingTime != NO_ECHO) {
    _echoTimesToAverage[_echoIndexToOverwrite] = pingTime;
    _echoIndexToOverwrite = (_echoIndexToOverwrite + 1) % ITERATIONS;
    // Recalculate the average
    int sum = 0;
    for (int j = 0; j < ITERATIONS; j++) {
      sum += _echoTimesToAverage[j];
    }
    int averageEchoTime = round((float)sum / (float)ITERATIONS);
    _lastPingAverage = averageEchoTime;
  }
  return _lastPingAverage;
}