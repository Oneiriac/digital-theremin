#include <Arduino.h>
#include <Audio.h>
#include <MIDI.h>
#include <NewPing.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

#include <string>

#include "NewPingNonBlocking.h"
#include "ldrPitchBend.h"
#include "music.h"
#include "wait.h"

// GUItool: begin automatically generated code
AudioSynthWaveformSine sine1;              // xy=489.0056266784668,603.0000200271606
AudioSynthWaveform waveform1;              // xy=663.0056076049805,451.99992752075195
AudioSynthWaveformModulated waveformMod1;  // xy=664.0056304931641,558.0000143051147
AudioSynthWaveform waveform2;              // xy=665.4602279663086,499.9147253036499
AudioMixer4 mixer1;                        // xy=902.0054550170898,450.9999313354492
AudioEffectFreeverb freeverb1;             // xy=1100.0056838989258,399.99995613098145
AudioAmplifier amp1;                       // xy=1386.0057678222656,387.9999465942383
AudioFilterStateVariable filter1;          // xy=1578.8239822387695,380.99988555908203
AudioOutputI2S i2s1;                       // xy=1799.0056610107422,367.9999599456787
AudioConnection patchCord1(sine1, 0, waveformMod1, 0);
AudioConnection patchCord2(waveform1, 0, mixer1, 0);
AudioConnection patchCord3(waveformMod1, 0, mixer1, 1);
AudioConnection patchCord4(waveform2, 0, mixer1, 3);
AudioConnection patchCord5(mixer1, freeverb1);
AudioConnection patchCord6(freeverb1, amp1);
AudioConnection patchCord7(amp1, 0, filter1, 0);
AudioConnection patchCord8(filter1, 0, i2s1, 0);
AudioConnection patchCord9(filter1, 0, i2s1, 1);
// GUItool: end automatically generated code

#define MAX_DISTANCE 45  // Maximum distance we want to ping for (in centimeters).
// Maximum sensor distance is rated at 400-500cm.

#define BASE_NOTE 48
#define RANGE_SIZE 13
#define POT1_LOWER_OFFSET 10U
#define POT1_HIGHER_OFFSET 10U

unsigned int POT1_RANGE = 1024 - POT1_LOWER_OFFSET - POT1_HIGHER_OFFSET;

NewPingNonBlocking pitch_sensor(11, 12, MAX_DISTANCE);
NewPingNonBlocking gain_sensor(13, 14, MAX_DISTANCE);

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
PitchHandler pitch_handler(MAX_DISTANCE, BASE_NOTE, RANGE_SIZE, MIDI);
const int channel = 1;

void play_frequency(float freq) {
  waveform1.frequency(freq / 2.0);
  waveformMod1.frequency(freq);
  waveform2.frequency(freq * 3.0 / 2.0);
}

void setup() {
  AudioMemory(10);
  Serial.begin(115200);
  MIDI.begin();
  // Configure osc 1
  waveform1.begin(WAVEFORM_SQUARE);
  waveform1.amplitude(0.1);
  // Configure osc 2
  waveformMod1.begin(WAVEFORM_TRIANGLE_VARIABLE);
  waveformMod1.amplitude(0.2);
  // Configure osc 3
  waveform2.begin(WAVEFORM_SAWTOOTH);
  waveform2.amplitude(0.25);
  sine1.frequency(0.5);
  sine1.amplitude(0.1);
  waveformMod1.frequencyModulation(1.0 / 192.0);
  // Misc
  freeverb1.roomsize(0.2);
  freeverb1.damping(0.1);
  filter1.frequency(pitch_handler.midi_note_to_frequency(80));
  amp1.gain(1.0);
}

int currentNote = 0;
int currentAfterTouch = 0;
int currentVelocity = 0;
float currentPitchBend = 0.0;

void loop() {
  // waveform1.frequency(440);
  boolean logChangedValues = false;
  delay(29);
  // Frequency using HC-SR04: lower the further away you get, higher the closer you get
  int pitch_distance = pitch_sensor.ping_average() / US_ROUNDTRIP_CM;
  int note_number = pitch_handler.midi_note_from_distance(pitch_distance);
  string note_string = pitch_handler.midi_note_string(note_number);
  float frequency = pitch_handler.midi_note_to_frequency(note_number);
  // Gain/aftertouch using HC-SR04: lower further away, higher closer
  int gain_distance = gain_sensor.ping_average() / US_ROUNDTRIP_CM;
  float gain = gain_from_distance(gain_distance, MAX_DISTANCE);  // Gain: lower when further away, higher when closer
  int afterTouch = round(gain * 127.0);

  // Set MIDI velocity using potentiometer
  int velocity = analogRead(18) / 8;   // From 0-1023 to 0-127
  boolean noteActive = velocity >= 2;  // Disable volume out and MIDI note on when potentiometer is off/nearly off
  if (currentVelocity != velocity) {
    currentVelocity = velocity;
    if (!noteActive) MIDI.sendNoteOff(currentNote, velocity, channel);
  }

  // Read LDRs for pitch bend
  float pitchBend = read_pitch_bend(15, 16);
  if (abs(currentPitchBend - pitchBend) >= 1.0 / 8192.0) {
    MIDI.sendPitchBend(pitchBend, channel);
    currentPitchBend = pitchBend;
  }

  // Send MIDI
  if (note_number != currentNote) {
    if (noteActive) MIDI.sendNoteOn(note_number, velocity, channel);
    MIDI.sendNoteOff(currentNote, velocity, channel);
    currentNote = note_number;
    logChangedValues = true;
  }

  if (currentAfterTouch != afterTouch) {
    MIDI.sendAfterTouch(afterTouch, channel);
    currentAfterTouch = afterTouch;
    logChangedValues = true;
  }

  // Output audio
  play_frequency(frequency);
  amp1.gain(noteActive ? gain : 0);

  // Print output to Serial
  if (logChangedValues) {
    char output[96];
    snprintf(
        output, sizeof(output),
        "Note: %s\tFrequency (Hz): %.1f\t Pitch distance (cm): %d\tGain/Aftertouch: %d\tftertouch distance (cm): %d",
        note_string.c_str(), frequency, pitch_distance, afterTouch, gain_distance);
    // Serial.println(output);
  }
}