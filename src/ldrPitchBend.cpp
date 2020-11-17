#include "ldrPitchBend.h"

#include <cmath>

#include "Arduino.h"

#define ZERO_RANGE 0.15F
#define PITCH_BEND_SCALE_FACTOR 10.0F

#define NUM_PITCH_BENDS_TO_AVERAGE 5
int pitchBendsToAverage[NUM_PITCH_BENDS_TO_AVERAGE] = {0};
unsigned int pitchBendIndex = 0;

float clip(float n, float lower, float upper) { return max(lower, min(n, upper)); }

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
  float pitchBend =
      clip(PITCH_BEND_SCALE_FACTOR * (float)sum / (1024.0 * (float)NUM_PITCH_BENDS_TO_AVERAGE), -1.0, +1.0);
  // Renormalize pitch bend
  float renormalizedPitchBend;
  if (abs(pitchBend) <= ZERO_RANGE) {
    // Values within [-ZERO_RANGE, ZERO_RANGE] are set to zero to stop fluctuations
    renormalizedPitchBend = 0.0;
  } else if (pitchBend > ZERO_RANGE) {
    // Renormalize values from (ZERO_RANGE, 1.0] to (0.0, 1.0]
    renormalizedPitchBend = (pitchBend - ZERO_RANGE) / (1 - ZERO_RANGE);
  } else {
    // Renormalize values from [-1.0, ZERO_RANGE) to [-1.0, 0.0)
    renormalizedPitchBend = -(pitchBend + ZERO_RANGE) / (-1 + ZERO_RANGE);
  }
  return renormalizedPitchBend;
}
