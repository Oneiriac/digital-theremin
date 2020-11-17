#include "ldrPitchBend.h"

#include <Arduino.h>

#include <cmath>

#include "rescale.h"

#define PITCH_BEND_MIN_BOUND 0.15F
#define PITCH_BEND_MAX_BOUND 0.9F
#define PITCH_BEND_SCALE_FACTOR 10.0F

#define NUM_PITCH_BENDS_TO_AVERAGE 5
int pitchBendsToAverage[NUM_PITCH_BENDS_TO_AVERAGE] = {0};
unsigned int pitchBendIndex = 0;

double clip(double n, double lower, double upper) { return max(lower, min(n, upper)); }

float read_pitch_bend(unsigned int bend_up_pin, unsigned int bend_down_pin) {
  // Read LDRs for pitch bend
  // TODO: set min/max range using button press
  int ldrBendUp = analogRead(bend_up_pin);
  int ldrBendDown = analogRead(bend_down_pin);
  int pitchBendInt = ldrBendUp - ldrBendDown;  // Delta between readings of ldrBendUp and ldrBendDown
  pitchBendsToAverage[pitchBendIndex] = pitchBendInt;
  pitchBendIndex = (pitchBendIndex + 1) % NUM_PITCH_BENDS_TO_AVERAGE;

  int sum = 0;
  for (int i = 0; i < NUM_PITCH_BENDS_TO_AVERAGE; i++) {
    sum += pitchBendsToAverage[pitchBendIndex];
  }
  double pitchBend = PITCH_BEND_SCALE_FACTOR * (double)sum / (1024.0 * (double)NUM_PITCH_BENDS_TO_AVERAGE);
  // Rescale pitch bend using linear interpolation
  double rescaledPitchBend;
  if (std::abs(pitchBend) <= PITCH_BEND_MIN_BOUND) {  // Use std::abs, otherwise will be cast to int
    rescaledPitchBend =
        0.0;  // Values within [-PITCH_BEND_MIN_BOUND, +PITCH_BEND_MIN_BOUND] are set to zero to stop fluctuations

  } else {
    double oldMin, oldMax, newMin, newMax;
    if (pitchBend > 0) {
      // Rescale values from (+PITCH_BEND_MIN_BOUND, 1.0] to (0.0, 1.0]
      oldMin = PITCH_BEND_MIN_BOUND;
      oldMax = PITCH_BEND_MAX_BOUND;
      newMin = 0.0;
      newMax = 1.0;
    } else {
      // Rescale values from [-1.0, -PITCH_BEND_MIN_BOUND) to [-1.0, 0.0)
      oldMin = -PITCH_BEND_MAX_BOUND;
      oldMax = -PITCH_BEND_MIN_BOUND;
      newMin = -1.0;
      newMax = 0.0;
    }
    rescaledPitchBend = clip(rescale(oldMin, oldMax, newMin, newMax, pitchBend), newMin, newMax);
  }
  return rescaledPitchBend;
}
