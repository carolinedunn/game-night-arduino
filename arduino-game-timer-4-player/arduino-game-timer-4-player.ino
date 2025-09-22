// 4-Player Game Timer: LCD (I2C) + LEDs (G/Y/R) + Buzzer
// Board: Arduino Uno/Nano (AVR)

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------- Pins ----------
const uint8_t BTN_PIN = 2;         // interrupt-capable
const uint8_t LED_G   = 5;
const uint8_t LED_Y   = 6;
const uint8_t LED_R   = 7;
const uint8_t BUZZER  = 9;

// ---------- LCD ----------
const uint8_t LCD_ADDR = 0x27;     // change to 0x3F if needed
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// ---------- Game Settings ----------
const uint8_t  NUM_PLAYERS   = 4;      // <<< 4 players
const unsigned long TURN_MS  = 60000;  // 60 s per player
const unsigned long WARN_Y_MS = 20000;
const unsigned long WARN_R_MS = 5000;
const unsigned long DEBOUNCE_MS = 80;

// Optional: show custom names (max 16 chars)
// Leave empty strings to fall back to "Player N"
const char* PLAYER_NAMES[NUM_PLAYERS + 1] = {
  "",        // unused index 0
  "Player 1",
  "Player 2",
  "Player 3",
  "Player 4"
};

// ---------- State ----------
volatile bool btnInterruptFlag = false;    // set in ISR
unsigned long lastBtnTime = 0;

enum State { IDLE, RUNNING, TIMEOUT };
State state = IDLE;

uint8_t activePlayer = 1;    // 1..NUM_PLAYERS
unsigned long deadline = 0;

// ---------- Forward decls ----------
void onButtonISR();
void startTurn(uint8_t p);
void startNextPlayer();
void lightsFor(unsigned long remain);
void beep(int freq, int ms, int pause = 60);
void lcdIdle();
void lcdShow(uint8_t p, unsigned long remainMs);
const char* nameFor(uint8_t p);

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
  // Handle button with debounce in loop
  if (btnInterruptFlag) {
    if (millis() - lastBtnTime >= DEBOUNCE_MS) {
      if (digitalRead(BTN_PIN) == LOW) {
        lastBtnTime = millis();

        if (state == IDLE) {
          // Start with Player 1
          startTurn(1);
        } else if (state == TIMEOUT) {
          // After timeout, move to next player
          startNextPlayer();
        } else { // RUNNING
          // Switch immediately to next player
          startNextPlayer();
        }

        // Wait for release
        while (digitalRead(BTN_PIN) == LOW) { delay(5); }
      }
    }
    btnInterruptFlag = false;
  }

  // Countdown
  if (state == RUNNING) {
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
      // Wait for next press to advance to next player
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
  if (p < 1) p = 1;
  if (p > NUM_PLAYERS) p = NUM_PLAYERS;

  activePlayer = p;
  deadline = millis() + TURN_MS;
  state = RUNNING;

  // Start sound: p short beeps (P1=1 beep, P2=2 beeps, etc.)
  // If you prefer always 2/3 like before, set count to fixed values.
  int count = p;
  if (count > 5) count = 5; // cap to keep it quick if NUM_PLAYERS grows
  int f = 1100;             // same pitch for all players here
  for (int i = 0; i < count; i++) { beep(f, 70, 60); }

  // Immediate UI refresh
  lightsFor(TURN_MS);
  lcdShow(activePlayer, TURN_MS);
}

void startNextPlayer() {
  uint8_t next = activePlayer + 1;
  if (next > NUM_PLAYERS) next = 1;
  startTurn(next);
}

void lightsFor(unsigned long remain) {
  digitalWrite(LED_G, remain > WARN_Y_MS);
  digitalWrite(LED_Y, (remain <= WARN_Y_MS && remain > WARN_R_MS));
  digitalWrite(LED_R, remain <= WARN_R_MS);
}

const char* nameFor(uint8_t p) {
  if (p >= 1 && p <= NUM_PLAYERS) {
    if (PLAYER_NAMES[p][0] != '\0') return PLAYER_NAMES[p];
  }
  return "";
}

void lcdIdle() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Press to start ");
  lcd.setCursor(0,1); lcd.print("  4-Player GT  ");
}

void lcdShow(uint8_t p, unsigned long remainMs) {
  // Name or "Player N"
  lcd.setCursor(0,0);
  const char* nm = nameFor(p);
  if (nm[0] != '\0') {
    // Print name, then pad to clear line
    char buf[17]; // 16 chars + null
    snprintf(buf, sizeof(buf), "%-16s", nm);
    lcd.print(buf);
  } else {
    // "Player N"
    char buf[17];
    snprintf(buf, sizeof(buf), "Player %d        ", p);
    lcd.print(buf);
  }

  // "Time: ###s"
  unsigned long s = remainMs / 1000UL;
  lcd.setCursor(0,1);
  lcd.print("Time: ");
  if (s < 100) lcd.print(' ');
  if (s < 10)  lcd.print(' ');
  lcd.print(s);
  lcd.print("s      ");
}
