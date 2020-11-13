#include "music.h"

#include <Arduino.h>
#include <math.h>

#include <cmath>

#define HYSTERESIS_THRESHOLD 2
#define BASE_NOTE_BUCKET_OFFSET 2  // How many extra buckets should capture the base note

double ratio = pow(2.0, 1.0 / 12.0);
float lastTransitionDistance;
int currentNote = 0;

int ordinal_note_from_distance(float distance, float maxDistance, int rangeSize) {
  int numBuckets = rangeSize + BASE_NOTE_BUCKET_OFFSET;
  int thisBucket = round(static_cast<double>(numBuckets * (maxDistance - distance) / maxDistance));
  int ordinalNote = max(thisBucket - BASE_NOTE_BUCKET_OFFSET, 0);
  return ordinalNote;
}

double frequency_from_base(double baseFrequency, double notesAboveBase) {
  return baseFrequency * pow(2.0, notesAboveBase / 12.0);
}

float frequency_from_distance(float distance, float maxDistance, double minFrequency, int rangeSize) {
  boolean insideThreshold = std::abs(distance - lastTransitionDistance) <= HYSTERESIS_THRESHOLD;
  boolean noDistance = distance == 0;
  // HC-SR04 returns too many invalid out-of-range (distance = 0): when this happens, stop the frequency from changing
  if (insideThreshold || noDistance) {
    Serial.println(distance);
    return frequency_from_base(minFrequency, currentNote);
  }
  int notesAboveBase = ordinal_note_from_distance(distance, maxDistance, rangeSize);
  if (notesAboveBase != currentNote) {
    currentNote = notesAboveBase;
    lastTransitionDistance = distance;
  }
  float frequency = frequency_from_base(minFrequency, notesAboveBase);
  char output[96];
  snprintf(output, sizeof(output), "distance: %.2f\t\tlastTransitionDistance: %.2f\t\tfrequency: %.2f", distance,
           lastTransitionDistance, frequency);
  Serial.println(output);
  return frequency;
}

float gainArray[10] = {0};
int gainArraySize = sizeof(gainArray) / sizeof(float);
int currentIndex = 0;

float gain_from_distance(float distance, float maxDistance) {
  float thisVolume = static_cast<float>((maxDistance - distance) / maxDistance);
  gainArray[currentIndex] = thisVolume;
  currentIndex = (currentIndex + 1) % gainArraySize;
  float sumVolumes = 0;
  for (int i = 0; i < gainArraySize; i++) {
    sumVolumes += gainArray[i];
  }
  float averagedVolumes = sumVolumes / (float)gainArraySize;
  return averagedVolumes;
}