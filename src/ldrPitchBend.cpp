#include "ldrPitchBend.h"

#include <Arduino.h>

#include <cmath>
#include <tuple>

#include "rescale.h"

#define PITCH_BEND_MIN_BOUND 0.15F
#define PITCH_BEND_MAX_BOUND 0.9F

// Set up arrays for the last N readings which will be used for averaging pitch bend values
#define NUM_PITCH_BENDS_TO_AVERAGE 10
double pitchBendsToAverage[NUM_PITCH_BENDS_TO_AVERAGE] = {0};
int bendUpAnalogReads[NUM_PITCH_BENDS_TO_AVERAGE] = {0};
int bendDownAnalogReads[NUM_PITCH_BENDS_TO_AVERAGE] = {0};
unsigned int pitchBendIndex = 0;
// Assign default values for analog read bounds based on the resistors used in the LDR voltage divider
// These default values target 330K resistors
unsigned int minAnalogRead = 160;
unsigned int maxAnalogRead = 240;

double clip(double n, double lower, double upper) { return max(lower, min(n, upper)); }

double rescale_with_clipping(double oldMin, double oldMax, double newMin, double newMax, double x) {
  return clip(rescale(oldMin, oldMax, newMin, newMax, x), newMin, newMax);
}

float read_pitch_bend(unsigned int bend_up_pin, unsigned int bend_down_pin) {
  // Read LDRs for pitch bend
  // TODO: set min/max range using button press
  int ldrBendUpAnalog = analogRead(bend_up_pin);
  int ldrBendDownAnalog = analogRead(bend_down_pin);
  double ldrBendUp = rescale_with_clipping(minAnalogRead, maxAnalogRead, 0, 1, ldrBendUpAnalog);
  double ldrBendDown = rescale_with_clipping(minAnalogRead, maxAnalogRead, 0, 1, ldrBendDownAnalog);

  double pitchBendDelta =
      ldrBendUp - ldrBendDown;  // Delta between readings of ldrBendUp and ldrBendDown, in range [-1, 1]
  pitchBendsToAverage[pitchBendIndex] = pitchBendDelta;
  bendUpAnalogReads[pitchBendIndex] = ldrBendUpAnalog;
  bendDownAnalogReads[pitchBendIndex] = ldrBendDownAnalog;
  pitchBendIndex = (pitchBendIndex + 1) % NUM_PITCH_BENDS_TO_AVERAGE;

  double sum = 0;
  for (int i = 0; i < NUM_PITCH_BENDS_TO_AVERAGE; i++) {
    sum += pitchBendsToAverage[pitchBendIndex];
  }
  double averagePitchBend = sum / (double)NUM_PITCH_BENDS_TO_AVERAGE;
  char output[96];
  snprintf(output, 96, "Up (raw): %d, Down (raw): %d, minAnalogRead: %d, maxAnalogRead: %d, pitchBend: %.2f",
           ldrBendUpAnalog, ldrBendDownAnalog, minAnalogRead, maxAnalogRead, averagePitchBend);
  Serial.println(output);

  // Rescale pitch bend using linear interpolation
  double rescaledPitchBend;
  if (std::abs(averagePitchBend) <= PITCH_BEND_MIN_BOUND) {  // Use std::abs, otherwise will be cast to int
    rescaledPitchBend =
        0.0;  // Values within [-PITCH_BEND_MIN_BOUND, +PITCH_BEND_MIN_BOUND] are set to zero to stop fluctuations

  } else {
    double oldMin, oldMax, newMin, newMax;
    if (averagePitchBend > 0) {
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
    rescaledPitchBend = rescale_with_clipping(oldMin, oldMax, newMin, newMax, averagePitchBend);
  }
  return rescaledPitchBend;
}

int get_average_ldr_analog() {
  double sum = 0;
  for (int i = 0; i < NUM_PITCH_BENDS_TO_AVERAGE; i++) {
    sum += round(static_cast<double>(bendUpAnalogReads[i] + bendDownAnalogReads[i]) / 2);
  }
  return round(sum / float(NUM_PITCH_BENDS_TO_AVERAGE));
}

/**
 * @brief Set the minimum bound of the LDR analog reading range (corresponds to 0.0 pitch bend).
 * Call this when neither LDR is covered to get a baseline for ambient light (min light resistance).
 *
 * @param bend_up_pin
 * @param bend_down_pin
 */
int set_ldr_min() {
  double scalingFactor = 1.1;
  // Arrays are populated, calculate the average of the averages of the LDRs
  double sum = 0;
  for (int i = 0; i < NUM_PITCH_BENDS_TO_AVERAGE; i++) {
    sum += round(static_cast<double>(bendUpAnalogReads[i] + bendDownAnalogReads[i]) / 2.0);
  }
  double averageValue = round(sum / (double)NUM_PITCH_BENDS_TO_AVERAGE);
  minAnalogRead = round((double)averageValue * scalingFactor);  // Set minAnalogRead
  return minAnalogRead;
}

/**
 * @brief Set the max bound of the LDR analog reading range (corresponds to +-1.0 pitch bend)
 * Call this when at least one LDR is fully covered to get the maximum possible dark resistance.
 *
 * @param bend_up_pin
 * @param bend_down_pin
 */
int set_ldr_max() {
  double scalingFactor = 0.9;
  // Arrays are populated, calculate the average of the max readings
  double sum = 0;
  for (int i = 0; i < NUM_PITCH_BENDS_TO_AVERAGE; i++) {
    sum += max(bendUpAnalogReads[i], bendDownAnalogReads[i]);
  }
  double averageValue = round(sum / (double)NUM_PITCH_BENDS_TO_AVERAGE);
  maxAnalogRead = round((double)averageValue * scalingFactor);  // Set maxAnalogRead
  return maxAnalogRead;
}

bool firstSetDone = false;
void first_set_ldr_min(unsigned int bend_up_pin, unsigned int bend_down_pin) {
  if (firstSetDone) return;
  // Arrays are not populated, so let's do that
  for (int i = 0; i < NUM_PITCH_BENDS_TO_AVERAGE; i++) {
    // Read the values
    int ldrBendUpAnalog = analogRead(bend_up_pin);
    int ldrBendDownAnalog = analogRead(bend_down_pin);
    bendUpAnalogReads[pitchBendIndex] = ldrBendUpAnalog;
    bendDownAnalogReads[pitchBendIndex] = ldrBendDownAnalog;
  }
  set_ldr_min();
  firstSetDone = true;
}