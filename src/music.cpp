#include "music.h"

#include <math.h>

double ratio = pow(2.0, 1.0 / 12.0);

int ordinal_note_from_distance(int distance, int maxDistance, int rangeSize) {
  return round(static_cast<double>(rangeSize * (maxDistance - distance) / maxDistance));
}

double frequency_from_base(double baseFrequency, double notesAboveBase) {
  return baseFrequency * pow(2.0, notesAboveBase / 12.0);
}

float frequency_from_distance(int distance, int maxDistance, double minFrequency, int rangeSize = 13) {
  int notesAboveBase = ordinal_note_from_distance(distance, maxDistance, rangeSize);
  return frequency_from_base(minFrequency, notesAboveBase);
}