#include "music.h"

#include <Arduino.h>
#include <MIDI.h>
#include <math.h>

#include <cmath>
#include <string>
#include <vector>

#include "rescale.h"

using namespace std;

SCALE_TYPE MAJOR_SCALE = {
    0, 2, 4, 5, 7, 9, 11,
};

SCALE_TYPE PENTATONIC_SCALE = {
    0, 2, 4, 7, 9,
};

SCALE_TYPE MINOR_SCALE = {0, 2, 3, 5, 7, 8, 10};
SCALE_TYPE WHOLE_TONE_SCALE = {0, 2, 4, 6, 8, 10};
SCALE_TYPE MAJOR_CHORD = {0, 4, 7};
SCALE_TYPE MINOR_CHORD = {0, 3, 7};
SCALE_TYPE MAJOR_7TH_CHORD = {0, 4, 7, 11};
SCALE_TYPE DOMINANT_7TH_CHORD = {0, 4, 7, 10};
SCALE_TYPE CHROMATIC_SCALE = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

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

void PitchHandler::set_active_scale(SCALE_TYPE& scale) { activeScale = scale; }

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
