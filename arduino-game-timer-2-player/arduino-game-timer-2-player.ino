// Two-player game timer: LCD (I2C) + LEDs (G/Y/R) + buzzer
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Pins ----------
const uint8_t BTN_PIN = 2;         // interrupt-capable pin on Uno/Nano
const uint8_t LED_G   = 5;
const uint8_t LED_Y   = 6;
const uint8_t LED_R   = 7;
const uint8_t BUZZER  = 9;

// ---------- LCD ----------
const uint8_t LCD_ADDR = 0x27;     // change to 0x3F if your module uses that
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// ---------- Timing (ms) ----------
const unsigned long TURN_MS     = 60000;   // 60 seconds per turn
const unsigned long WARN_Y_MS   = 20000;   // yellow under 20s
const unsigned long WARN_R_MS   = 5000;    // red under 5s
const unsigned long DEBOUNCE_MS = 80;      // debounce window

// ---------- State ----------
volatile bool btnInterruptFlag = false;    // set in ISR, handled in loop
unsigned long lastBtnTime = 0;

enum State { IDLE, P1_RUNNING, P2_RUNNING, TIMEOUT };
State state = IDLE;

uint8_t activePlayer = 1;
unsigned long deadline = 0;

// ---------- Forward decls ----------
void onButtonISR();
void startTurn(uint8_t p);
void lightsFor(unsigned long remain);
void beep(int freq, int ms, int pause = 60);
void lcdIdle();
void lcdShow(uint8_t p, unsigned long remainMs);

void setup() {
  // Button + ISR
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), onButtonISR, FALLING);

  // LEDs + buzzer
  pinMode(LED_G, OUTPUT);
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_Y, LOW);
  digitalWrite(LED_R, LOW);

  // LCD
  lcd.init();
  lcd.backlight();
  lcdIdle();
}

void loop() {
  // Handle button with debounce in loop (not in ISR)
  if (btnInterruptFlag) {
    if (millis() - lastBtnTime >= DEBOUNCE_MS) {
      if (digitalRead(BTN_PIN) == LOW) {
        lastBtnTime = millis();

        if (state == IDLE || state == TIMEOUT) {
          // From TIMEOUT start the other player; from IDLE start P1
          startTurn( (state == IDLE) ? 1 : (activePlayer == 1 ? 2 : 1) );
        } else {
          // Switch turns mid-countdown
          startTurn( activePlayer == 1 ? 2 : 1 );
        }

        // Wait for release to avoid auto-repeat
        while (digitalRead(BTN_PIN) == LOW) { delay(5); }
      }
    }
    btnInterruptFlag = false;
  }

  // Countdown
  if (state == P1_RUNNING || state == P2_RUNNING) {
    long remain = (long)deadline - (long)millis();
    if (remain <= 0) {
      state = TIMEOUT;

      // Timeout alarm (descending tones)
      int tones[] = {1200, 1000, 800, 600, 400};
      for (uint8_t i = 0; i < 5; i++) { beep(tones[i], 120, 30); }

      // Red blink a few times
      for (uint8_t i = 0; i < 6 && state == TIMEOUT; i++) {
        digitalWrite(LED_R, HIGH); delay(120);
        digitalWrite(LED_R, LOW);  delay(120);
      }

      // LCD message
      lcd.clear();
      lcd.setCursor(0,0); lcd.print("   TIME IS UP   ");
      lcd.setCursor(0,1); lcd.print("Press for next ");
      // Wait for next press to flip to the other player
    } else {
      lightsFor((unsigned long)remain);
      lcdShow(activePlayer, (unsigned long)remain);
    }
  }
}

// --- ISR: minimal work only ---
void onButtonISR() {
  btnInterruptFlag = true;
}

// --- Helpers ---
void beep(int freq, int ms, int pause) {
  tone(BUZZER, freq, ms);
  delay(ms + pause);
}

void startTurn(uint8_t p) {
  activePlayer = p;
  deadline = millis() + TURN_MS;
  state = (p == 1) ? P1_RUNNING : P2_RUNNING;

  // Start sound: 2 beeps for P1, 3 for P2
  int count = (p == 1) ? 2 : 3;
  int f = (p == 1) ? 1200 : 900;
  for (int i = 0; i < count; i++) { beep(f, 80, 70); }

  // Immediate UI refresh
  lightsFor(TURN_MS);
  lcdShow(activePlayer, TURN_MS);
}

void lightsFor(unsigned long remain) {
  digitalWrite(LED_G, remain > WARN_Y_MS);
  digitalWrite(LED_Y, (remain <= WARN_Y_MS && remain > WARN_R_MS));
  digitalWrite(LED_R, remain <= WARN_R_MS);
}

void lcdIdle() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Press to start ");
  lcd.setCursor(0,1); lcd.print("   Game Timer  ");
}

void lcdShow(uint8_t p, unsigned long remainMs) {
  // Line 1: Player X
  lcd.setCursor(0,0);
  lcd.print("Player ");
  lcd.print(p);
  lcd.print("        "); // pad/clear to end

  // Line 2: Time: ###s
  unsigned long s = remainMs / 1000UL;
  lcd.setCursor(0,1);
  lcd.print("Time: ");
  if (s < 100) lcd.print(' ');
  if (s < 10)  lcd.print(' ');
  lcd.print(s);
  lcd.print("s      ");
}
