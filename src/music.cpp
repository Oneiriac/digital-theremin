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