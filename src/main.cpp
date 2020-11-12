#include <Arduino.h>
#include <Audio.h>
#include <NewPing.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Wire.h>
#include <math.h>

#include "wait.h"

AudioSynthWaveform waveform1;
AudioOutputI2S i2s1;
AudioConnection patchCord1(waveform1, 0, i2s1, 0);
AudioConnection patchCord2(waveform1, 0, i2s1, 1);

#define TRIGGER_PIN 12   // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 11      // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 60  // Maximum distance we want to ping for (in centimeters).
// Maximum sensor distance is rated at 400-500cm.

#define MIN_FREQ 220
#define MAX_FREQ 880

NewPing sonar(TRIGGER_PIN, ECHO_PIN,
              MAX_DISTANCE);  // NewPing setup of pins and maximum distance.

void setup() {
  AudioMemory(10);
  Serial.begin(115200);
  waveform1.begin(WAVEFORM_SINE);
}

void loop() {
  // waveform1.frequency(440);
  waveform1.amplitude(0.1);
  unsigned int rawDistance = sonar.convert_cm(sonar.ping_median(5));
  unsigned int distance = rawDistance > 0 ? rawDistance : MAX_DISTANCE;
  // Frequency: lower the further away you get, higher the closer you get
  float frequency = MIN_FREQ + static_cast<float>(MAX_FREQ - MIN_FREQ) * (log(distance) / log(MAX_DISTANCE));
  waveform1.frequency(frequency);
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println("Hz");
}