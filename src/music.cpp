#include "music.h"

#include <Arduino.h>
#include <MIDI.h>
#include <math.h>

#include <cmath>
#include <string>
#include <vector>

#include "rescale.h"

using namespace std;

// How many centimetres away from the last note transition point before a transition is allowed
#define HYSTERESIS_THRESHOLD 2.0

#define SCALE_TO_USE WHOLE_TONE_SCALE

double ratio = pow(2.0, 1.0 / 12.0);

string NOTE_STRINGS[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
};

PitchHandler::PitchHandler(float maxDistance, int baseNote, int rangeSize, MIDI_TYPE midi)
    : maxDistance(maxDistance), baseNote(baseNote), rangeSize(rangeSize), activeScale(SCALE_TO_USE), midi(midi){};

/**
 * @brief Calculate the midi note number from the given distance value.
 *
 * @param distance
 * @return int
 */
int PitchHandler::midi_note_from_distance(float distance) {
  boolean insideThreshold = abs(distance - lastTransitionDistance) <= HYSTERESIS_THRESHOLD;
  boolean noDistance = distance == 0;
  // HC-SR04 returns too many invalid out-of-range (distance = 0): when this happens, don't update the current note
  if (!insideThreshold && !noDistance) {
    // A transition is possible: check if it has occurred, and if so update the required values
    int scaleNoteNumber = scale_note_number(distance);
    int newNote = midi_note_from_scale_note(scaleNoteNumber);
    if (newNote != currentNote) {
      currentNote = newNote;
      lastTransitionDistance = distance;
    }
  }

  return currentNote;
};

void PitchHandler::set_active_scale(const vector<int>& scale) { activeScale = scale; }

/**
 * @brief See https://en.wikipedia.org/wiki/MIDI_tuning_standard
 *
 * @param midi_note
 * @return float
 */
float PitchHandler::midi_note_to_frequency(int midi_note) {
  float frequency = pow(2, static_cast<double>((midi_note - 69) / 12.0)) * REFERENCE;
  return frequency;
}

string PitchHandler::midi_note_string(int midi_note) {
  string& note_name = NOTE_STRINGS[midi_note % 12];
  int octave = midi_note / 12 - 1;
  char outString[6];
  snprintf(outString, sizeof(outString), "%s%d", note_name.c_str(), octave);
  return string(outString);
}

#define GAIN_ZERO_BOUND 0.07
#define GAIN_MAX_BOUND 0.93

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
    float thisGain = static_cast<float>((maxDistance - distance) / maxDistance);
    gainArray[currentIndex] = thisGain;
    currentIndex = (currentIndex + 1) % gainArraySize;
  }
  float sumGain = 0;
  for (int i = 0; i < gainArraySize; i++) {
    sumGain += gainArray[i];
  }
  float averagedGain = sumGain / (float)gainArraySize;
  // Renormalize pitch bend
  float rescaledGain;
  if (averagedGain <= GAIN_ZERO_BOUND) {
    // Values <= GAIN_ZERO_BOUND are set to zero to make selecting zero easier
    rescaledGain = 0.0;
  } else if (averagedGain >= GAIN_MAX_BOUND) {
    // Values >= GAIN_MAX_BOUND are set to 1.0 to make selecting max easier
    rescaledGain = 1.0;
  } else {
    // Rescale values from (GAIN_ZERO_BOUND, GAIN_MAX_BOUND) to (0.0, 1.0)
    rescaledGain = rescale(GAIN_ZERO_BOUND, GAIN_MAX_BOUND, 0.0, 1.0, averagedGain);
  }
  return rescaledGain;
}