#include "afterTouch.h"

#include "rescale.h"

#define AFTER_TOUCH_ZERO_BOUND 0.07
#define AFTER_TOUCH_MAX_BOUND 0.93
#define NUM_AFTER_TOUCH_TO_AVERAGE 5

float afterTouchValuesToAverage[NUM_AFTER_TOUCH_TO_AVERAGE] = {0};
int afterTouchIndex = 0;

/**
 * @brief Calculate aftertouch (0.0-1.0) using a HC-SR04.
 *
 * @param distance
 * @param maxDistance
 * @return float
 */
float aftertouch_from_distance(float distance, float maxDistance) {
  bool noDistance = distance == 0;
  // If distance is registered, update gainArray
  // If distance is not registered, do not update gainArray
  if (!noDistance) {
    float thisGain = static_cast<float>((maxDistance - distance) / maxDistance);
    afterTouchValuesToAverage[afterTouchIndex] = thisGain;
    afterTouchIndex = (afterTouchIndex + 1) % NUM_AFTER_TOUCH_TO_AVERAGE;
  }
  float sum = 0;
  for (int i = 0; i < NUM_AFTER_TOUCH_TO_AVERAGE; i++) {
    sum += afterTouchValuesToAverage[i];
  }
  float averagedAfterTouch = sum / (float)NUM_AFTER_TOUCH_TO_AVERAGE;
  // Rescale aftertouch
  float rescaledAfterTouch;
  if (averagedAfterTouch <= AFTER_TOUCH_ZERO_BOUND) {
    // Values <= AFTER_TOUCH_ZERO_BOUND are set to zero to make selecting zero easier
    rescaledAfterTouch = 0.0;
  } else if (averagedAfterTouch >= AFTER_TOUCH_MAX_BOUND) {
    // Values >= AFTER_TOUCH_MAX_BOUND are set to 1.0 to make selecting max easier
    rescaledAfterTouch = 1.0;
  } else {
    // Rescale values from (AFTER_TOUCH_ZERO_BOUND, AFTER_TOUCH_MAX_BOUND) to (0.0, 1.0)
    rescaledAfterTouch = rescale(AFTER_TOUCH_ZERO_BOUND, AFTER_TOUCH_MAX_BOUND, 0.0, 1.0, averagedAfterTouch);
  }
  return rescaledAfterTouch;
}