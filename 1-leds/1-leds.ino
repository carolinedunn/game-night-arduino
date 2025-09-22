// Two-player game timer: LCD (I2C) + LEDs (G/Y/R) + buzzer
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>
#include <Wire.h>

// ---------- Pins ----------
const uint8_t BTN_PIN = 2;         // interrupt-capable pin on Uno/Nano
const uint8_t LED_G   = 5;
const uint8_t LED_Y   = 6;
const uint8_t LED_R   = 7;
const uint8_t BUZZER  = 9;

void setup() {

  // LEDs + buzzer
  pinMode(LED_G, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_Y, LOW);
  digitalWrite(LED_R, LOW);
}

void loop() {
  digitalWrite(LED_G, HIGH); 
  delay(500);
  digitalWrite(LED_G, LOW); 
  digitalWrite(LED_Y, HIGH); 
  delay(500);
  digitalWrite(LED_Y, LOW); 
  digitalWrite(LED_R, HIGH); 
  delay(500);
  digitalWrite(LED_R, LOW); 
}
