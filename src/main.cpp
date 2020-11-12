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
AudioSynthWaveform waveform1;  // xy=450.0056686401367,191.99994659423828
AudioAmplifier amp1;           // xy=745.0056076049805,175.005615234375
AudioOutputI2S i2s1;           // xy=1010.0057220458984,184.99980354309082
AudioConnection patchCord1(waveform1, amp1);
AudioConnection patchCord2(amp1, 0, i2s1, 0);
AudioConnection patchCord3(amp1, 0, i2s1, 1);
// GUItool: end automatically generated code

#define TRIGGER_PIN 12   // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 11      // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 50  // Maximum distance we want to ping for (in centimeters).
// Maximum sensor distance is rated at 400-500cm.

#define MIN_FREQ 440
#define RANGE_SIZE 13

NewPing sonar(TRIGGER_PIN, ECHO_PIN,
              MAX_DISTANCE);  // NewPing setup of pins and maximum distance.

void setup() {
  AudioMemory(10);
  Serial.begin(115200);
  waveform1.begin(WAVEFORM_SINE);
  waveform1.amplitude(1.0);
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
  waveform1.frequency(frequency);
  // amp1.gain(gain);
  // Print output to Serial
  char output[64];
  snprintf(output, sizeof(output), "Frequency (Hz): %.2f\t\tDistance (cm): %.1f", frequency, distance);
  Serial.println(output);
}