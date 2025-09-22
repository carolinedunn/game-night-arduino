// arduino_timer_lcd_simple.ino
// Two-player timer with 16x2 I2C LCD, LEDs, and buzzer.
// LCD shows only time remaining (no player number).

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const uint8_t BTN_PIN = 2;
const uint8_t LED_G = 5, LED_Y = 6, LED_R = 7;
const uint8_t BUZZER = 9;

// Uno/Nano: A4=SDA, A5=SCL
LiquidCrystal_I2C lcd(0x27, 16, 2); // change to 0x3F if needed

const unsigned long TURN_MS = 10000;
const unsigned long WARN_Y_MS = 4000;
const unsigned long WARN_R_MS = 2000;

volatile bool btnPressed = false;
unsigned long lastBtnTime = 0;

enum State { IDLE, RUNNING, TIMEOUT } state = IDLE;
unsigned long deadline = 0;

void onButton() {
  unsigned long now = millis();
  if (now - lastBtnTime > 80) { // debounce
    btnPressed = true;
    lastBtnTime = now;
  }
}

void beep(int freq, int ms, int pause=60) {
  tone(BUZZER, freq, ms);
  delay(ms + pause);
}

void startTurn() {
  deadline = millis() + TURN_MS;
  state = RUNNING;

  // Start sound
  for (int i=0; i<2; i++) { beep(1000, 80, 70); }
}

void lightsFor(unsigned long remain) {
  digitalWrite(LED_G, remain > WARN_Y_MS);
  digitalWrite(LED_Y, (remain <= WARN_Y_MS && remain > WARN_R_MS));
  digitalWrite(LED_R, remain <= WARN_R_MS);
}

void lcdShow(unsigned long remainMs) {
  lcd.setCursor(0,0); lcd.print("   Timer Run   ");
  lcd.setCursor(0,1);
  char line2[17];
  snprintf(line2, 17, "Time: %3lus      ", remainMs/1000);
  lcd.print(line2);
}

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), onButton, FALLING);

  pinMode(LED_G, OUTPUT); pinMode(LED_Y, OUTPUT); pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.init(); lcd.backlight();
  lcd.setCursor(0,0); lcd.print("Press to start ");
  lcd.setCursor(0,1); lcd.print("   Game Timer  ");
}

void loop() {
  if (btnPressed) {
    btnPressed = false;
    // Start fresh each press (whether IDLE, TIMEOUT, or RUNNING)
    startTurn();
  }

  if (state == RUNNING) {
    long remain = (long)deadline - (long)millis();
    if (remain <= 0) {
      state = TIMEOUT;

      // Timeout alarm
      int tones[] = {1200, 1000, 800, 600, 400};
      for (uint8_t i = 0; i < 5; i++) { beep(tones[i], 120, 30); }

      // Red blink
      for (int i=0; i<3 && state==TIMEOUT; i++) {
        digitalWrite(LED_R, HIGH); delay(150);
        digitalWrite(LED_R, LOW);  delay(150);
      }

      lcd.clear();
      lcd.setCursor(0,0); lcd.print("   TIME IS UP ");
      lcd.setCursor(0,1); lcd.print("Press for next");
    } else {
      lightsFor(remain);
      lcdShow(remain);
    }
  }
}
