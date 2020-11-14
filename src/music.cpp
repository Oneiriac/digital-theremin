#include "music.h"

#include <Arduino.h>
#include <math.h>

#include <cmath>
#include <string>
#include <vector>

#include "scales.h"

using namespace std;

// How many centimetres away from the last note transition point before a transition is allowed
#define HYSTERESIS_THRESHOLD 2
// How many extra buckets should be allocated to the base note (increasing the range of distances for the base note)
#define BASE_NOTE_BUCKET_OFFSET 2

double ratio = pow(2.0, 1.0 / 12.0);
float lastTransitionDistance;
int currentNote = 0;

#define SCALE_TO_USE MAJOR_7TH_CHORD

/**
 * @brief From the note number (i.e. notes above a root note in the given scale), calculate the chromatic note number
 * (i.e. notes above the root note in the chromatic scale).
 *
 * @param scale
 * @param noteNumber
 * @return int
 */
int chromatic_note_number(vector<int> scale, int noteNumber) {
  int octavesAbove = noteNumber / scale.size();
  int baseChromaticNote = scale[noteNumber % scale.size()];
  return octavesAbove * 12 + baseChromaticNote;
}

string noteStrings[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
};

/**
 * @brief Get the note letter from the frequency (snapped to the nearest whole tone).
 *
 * @param frequency
 * @return string
 */
string frequency_to_note(float frequency) {
  int midiNote = (12 * log2(frequency / 440.0) + 57) + 0.5;
  return noteStrings[midiNote % 12];
}

/**
 * @brief Calculate the note number.
 *
 * @param distance
 * @param maxDistance
 * @param rangeSize
 * @return int
 */
int note_number_from_distance(float distance, float maxDistance, int rangeSize) {
  int numBuckets = rangeSize + BASE_NOTE_BUCKET_OFFSET;
  int thisBucket = round(static_cast<double>(numBuckets * (maxDistance - distance) / maxDistance));
  return max(thisBucket - BASE_NOTE_BUCKET_OFFSET, 0);
}

/**
 * @brief Caculate the frequency from the base/root frequency and note number.
 *
 * @param baseFrequency
 * @param noteNumber
 * @return double
 */
double calc_frequency(double baseFrequency, int noteNumber) {
  return baseFrequency * pow(2.0, (double)chromatic_note_number(SCALE_TO_USE, noteNumber) / 12.0);
}

/**
 * @brief Calculate frequency (from a minFrequency) using a HC-SR04.
 *
 * @param distance
 * @param maxDistance
 * @param minFrequency
 * @param rangeSize
 * @return float
 */
float frequency_from_distance(float distance, float maxDistance, double minFrequency, int rangeSize) {
  boolean insideThreshold = std::abs(distance - lastTransitionDistance) <= HYSTERESIS_THRESHOLD;
  boolean noDistance = distance == 0;
  // HC-SR04 returns too many invalid out-of-range (distance = 0): when this happens, stop the frequency from changing
  if (insideThreshold || noDistance) {
    Serial.println(distance);
    return calc_frequency(minFrequency, currentNote);
  }
  int noteNumber = note_number_from_distance(distance, maxDistance, rangeSize);
  if (noteNumber != currentNote) {
    currentNote = noteNumber;
    lastTransitionDistance = distance;
  }
  float frequency = calc_frequency(minFrequency, noteNumber);
  char output[96];
  snprintf(output, sizeof(output), "distance: %.2f\t\tlastTransitionDistance: %.2f\t\tnote: %s", distance,
           lastTransitionDistance, frequency_to_note(frequency).c_str());
  Serial.println(output);
  return frequency;
}

float gainArray[5] = {0};
int gainArraySize = sizeof(gainArray) / sizeof(float);
int currentIndex = 0;

/**
 * @brief Calculate gain (0.0-1.0) using a HC-SR04.
 *
 * @param distance
 * @param maxDistance
 * @return float
 */
float gain_from_distance(float distance, float maxDistance) {
  boolean noDistance = distance == 0;
  // If distance is registered, update gainArray
  // If distance is not registered, do not update gainArray
  if (!noDistance) {
    float thisVolume = static_cast<float>((maxDistance - distance) / maxDistance);
    gainArray[currentIndex] = thisVolume;
    currentIndex = (currentIndex + 1) % gainArraySize;
  }
  float sumVolumes = 0;
  for (int i = 0; i < gainArraySize; i++) {
    sumVolumes += gainArray[i];
  }
  float averagedVolumes = sumVolumes / (float)gainArraySize;
  return averagedVolumes;
}