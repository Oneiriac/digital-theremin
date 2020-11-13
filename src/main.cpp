#include <Arduino.h>
#include <Audio.h>
#include <NewPing.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>

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

#define TRIGGER_PIN 12   // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 11      // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50  // Maximum distance we want to ping for (in centimeters).
// Maximum sensor distance is rated at 400-500cm.

#define MIN_FREQ 440
#define RANGE_SIZE 13

NewPing sonar(TRIGGER_PIN, ECHO_PIN,
              MAX_DISTANCE);  // NewPing setup of pins and maximum distance.

void play_frequency(float freq) {
  waveform1.frequency(freq / 2.0);
  waveformMod1.frequency(freq);
  waveform2.frequency(freq * 1.5);
}

void setup() {
  AudioMemory(10);
  Serial.begin(115200);
  // Configure osc 1
  waveform1.begin(WAVEFORM_SQUARE);
  waveform1.amplitude(0.1);
  // Configure osc 2
  waveformMod1.begin(WAVEFORM_SINE);
  waveformMod1.amplitude(0.2);
  // Configure osc 3
  waveform2.begin(WAVEFORM_TRIANGLE);
  waveform2.amplitude(0.25);
  sine1.frequency(0.5);
  sine1.amplitude(0.1);
  waveformMod1.frequencyModulation(1.0 / 192.0);
  // Misc
  freeverb1.roomsize(0.2);
  freeverb1.damping(0.1);
  filter1.frequency(8 * MIN_FREQ);
  amp1.gain(1.0);
}

void loop() {
  // waveform1.frequency(440);
  delay(29);
  float rawDistance = (float)sonar.ping_median(5) / (float)US_ROUNDTRIP_CM;
  float distance = rawDistance > 0 ? rawDistance : (float)MAX_DISTANCE;
  // Frequency: lower the further away you get, higher the closer you get
  float frequency = frequency_from_distance(distance, MAX_DISTANCE, MIN_FREQ, RANGE_SIZE);
  float gain = gain_from_distance(distance, MAX_DISTANCE);  // Gain: lower when further away, higher when closer
  play_frequency(frequency);
  // amp1.gain(gain);
  // Print output to Serial
  char output[64];
  snprintf(output, sizeof(output), "Frequency (Hz): %.2f\t\tDistance (cm): %.1f", frequency, distance);
  Serial.println(output);
}