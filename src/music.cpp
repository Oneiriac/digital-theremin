#include "music.h"

#include <math.h>

#include <cmath>

#define HYSTERESIS_THRESHOLD 3

double ratio = pow(2.0, 1.0 / 12.0);
float lastTransitionDistance;
int currentNote = -1;

int ordinal_note_from_distance(float distance, float maxDistance, int rangeSize) {
  return round(static_cast<double>(rangeSize * (maxDistance - distance) / maxDistance));
}

double frequency_from_base(double baseFrequency, double notesAboveBase) {
  return baseFrequency * pow(2.0, notesAboveBase / 12.0);
}

float frequency_from_distance(float distance, float maxDistance, double minFrequency, int rangeSize) {
  if (currentNote >= 0 && std::abs(distance - lastTransitionDistance) <= HYSTERESIS_THRESHOLD) {
    return frequency_from_base(minFrequency, currentNote);
  }
  int notesAboveBase = ordinal_note_from_distance(distance, maxDistance, rangeSize);
  if (notesAboveBase != currentNote) {
    currentNote = notesAboveBase;
    lastTransitionDistance = distance;
  }
  return frequency_from_base(minFrequency, notesAboveBase);
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