#ifndef MUSIC_H
#define MUSIC_H
#include <Arduino.h>
#include <Midi.h>

#include <cmath>
#include <string>
#include <vector>

using namespace std;
#define SCALE_TYPE const vector<int>

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

// How many extra buckets should be allocated to the lowest and highest divisions in a range
// (to allow them to be selected more easily)
#define LOWEST_EXTRA_BUCKETS 2
#define HIGHEST_EXTRA_BUCKETS 1
#define MIDI_TYPE midi::MidiInterface<midi::SerialMIDI<HardwareSerial>>

class PitchHandler {
 public:
  explicit PitchHandler(float maxDistance, int baseNote, int rangeSize, MIDI_TYPE midi);
  int midi_note_from_distance(float distance);
  void set_active_scale(const vector<int>& scale);
  float midi_note_to_frequency(int midi_note);
  static string midi_note_string(int midi_note);

 private:
  /**
   * @brief Calculate the MIDI note number from the base note and the note number in the active scale.
   *
   * @param scaleNoteNumber
   * @return int
   */
  int midi_note_from_scale_note(int scaleNoteNumber) {
    int scaleSize = activeScale.size();
    int octavesAbove = scaleNoteNumber / scaleSize;
    int noteInOctave = activeScale[scaleNoteNumber % scaleSize];
    return baseNote + (octavesAbove * 12) + noteInOctave;
  }

  /**
   * @brief Calculate the number of notes above the base note in the active scale.
   * Note numbers are assigned equal linear divisions (i.e. buckets), except for the lowest and highest notes:
   * Lowest note gets # buckets = normal # buckets * (1 + LOWEST_EXTRA_BUCKETS)
   * Highest note gets # buckets = normal # buckets  * (1 + HIGHEST_EXTRA_BUCKETS)
   *
   * @param distance
   * @return int
   */
  int scale_note_number(float distance) {
    int numBuckets = rangeSize + LOWEST_EXTRA_BUCKETS + HIGHEST_EXTRA_BUCKETS;
    int thisBucket = round(static_cast<double>(numBuckets * (maxDistance - distance) / maxDistance));
    if (thisBucket <= 1 + LOWEST_EXTRA_BUCKETS) {
      return 0;
    } else if (thisBucket >= rangeSize - 1 - HIGHEST_EXTRA_BUCKETS) {
      return rangeSize;
    } else {
      return thisBucket;
    }
  }

  float REFERENCE{440.0};
  float maxDistance{0};
  int baseNote{60};
  int currentNote{60};
  int rangeSize{13};
  float lastTransitionDistance{-1};
  SCALE_TYPE& activeScale;
  MIDI_TYPE midi;
};
float gain_from_distance(float distance, float maxDistance);

#endif