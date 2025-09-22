// arduino_timer_leds_fixed.ino
// Two-player game timer using a single button, LEDs (G/Y/R) and a buzzer.
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>

const uint8_t BTN_PIN = 2;         // interrupt-capable
const uint8_t LED_G   = 5, LED_Y = 6, LED_R = 7;
const uint8_t BUZZER  = 9;

const unsigned long TURN_MS   = 60000;
const unsigned long WARN_Y_MS = 20000;
const unsigned long WARN_R_MS = 5000;
const unsigned long DEBOUNCE_MS = 80;

volatile bool btnInterruptFlag = false;   // set in ISR, handled in loop
unsigned long lastBtnTime = 0;

enum State { IDLE, P1_RUNNING, P2_RUNNING, TIMEOUT } state = IDLE;
uint8_t activePlayer = 1;
unsigned long deadline = 0;

// --- ISR: keep tiny (no millis/Serial) ---
void onButtonISR() { btnInterruptFlag = true; }

void beep(int freq, int ms, int pause=60) {
  tone(BUZZER, freq, ms);
  delay(ms + pause);
}

void startTurn(uint8_t p) {
  activePlayer = p;
  deadline = millis() + TURN_MS;
  state = (p == 1) ? P1_RUNNING : P2_RUNNING;
  int count = (p == 1) ? 2 : 3;
  int f = (p == 1) ? 1200 : 900;
  for (int i = 0; i < count; i++) { beep(f, 80, 70); }
}

void lightsFor(unsigned long remain) {
  digitalWrite(LED_G, remain > WARN_Y_MS);
  digitalWrite(LED_Y, (remain <= WARN_Y_MS && remain > WARN_R_MS));
  digitalWrite(LED_R, remain <= WARN_R_MS);
}

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), onButtonISR, FALLING);

  pinMode(LED_G, OUTPUT); pinMode(LED_Y, OUTPUT); pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  // Debounce & handle press in the main loop (not in ISR)
  if (btnInterruptFlag) {
    if (millis() - lastBtnTime >= DEBOUNCE_MS) {
      if (digitalRead(BTN_PIN) == LOW) {
        lastBtnTime = millis();

        if (state == IDLE || state == TIMEOUT) {
          startTurn( (state == IDLE) ? 1 : (activePlayer == 1 ? 2 : 1) );
        } else {
          startTurn( activePlayer == 1 ? 2 : 1 );
        }

        // wait for release to avoid repeats
        while (digitalRead(BTN_PIN) == LOW) { delay(5); }
      }
    }
    btnInterruptFlag = false;
  }

  if (state == P1_RUNNING || state == P2_RUNNING) {
    long remain = (long)deadline - (long)millis();
    if (remain <= 0) {
      state = TIMEOUT;

      // REPLACEMENT for brace-initializer loop:
      int tones[] = {1200, 1000, 800, 600, 400};
      for (uint8_t i = 0; i < 5; i++) { beep(tones[i], 120, 30); }

      // Blink red while waiting
      for (uint8_t i = 0; i < 6 && state == TIMEOUT; i++) {
        digitalWrite(LED_R, HIGH); delay(120);
        digitalWrite(LED_R, LOW);  delay(120);
      }
    } else {
      lightsFor((unsigned long)remain);
    }
  }
}
