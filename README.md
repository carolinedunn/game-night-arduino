# Game Night Timer — Arduino Edition

Do your friends or family take **way too long** on their turn during board or card games?  
This project is a DIY **two-player turn timer** using an Arduino, an I²C 16×2 LCD, traffic-light style LEDs (green/yellow/red), and a buzzer.  
Think of it as a one-button **chess clock** for game night: press the button, your turn ends, and the countdown for the next player begins.

[![DIY Board Game Timer with Arduino Uno (LEDs, LCD, Buzzer)](Arduino.jpg)](https://youtu.be/IcNpnKvu7IY)
---

## Features
- One button to flip turns between Player 1 and Player 2
- **LCD 16×2 (I²C)** shows active player and time remaining
- **Green / Yellow / Red LEDs** for status:
  - Green: plenty of time
  - Yellow: running low
  - Red: nearly out
- **Buzzer tones**:
  - Start beeps (2 for Player 1, 3 for Player 2)
  - Descending alarm when time runs out
- Timeout message on LCD (“TIME IS UP – Press for next”)

---

## Hardware

- Arduino Uno, Nano (ATmega328P), or compatible
- 1 × I²C 16×2 LCD module (PCF8574 backpack at address `0x27` or `0x3F`)
- 1 × Pushbutton (wired to pin 2 with pull-up)
- 3 × LEDs (green, yellow, red)
- 3 × Current-limiting resistors (220–330 Ω)
- 1 × Piezo buzzer (on pin 9)
- Breadboard + jumper wires

**Pin mapping (default in code):**
- Button → D2 (interrupt capable)
- Green LED → D5
- Yellow LED → D6
- Red LED → D7
- Buzzer → D9
- LCD → I²C (A4 = SDA, A5 = SCL on Uno/Nano)

[Wiring Diagram](Arduino.png)

<img src="Arduino.png" alt="Wiring diagram: Arduino UNO, LEDs, button, buzzer" width="720"/>

---

## Software Setup

1. **Install Arduino IDE** (https://www.arduino.cc/en/software)

   If using a Raspberry Pi install with command
   ```bash
   sudo apt install arduino
   ```
3. **Install the LCD library**  
   Open **Sketch → Include Library → Manage Libraries**, search for **LiquidCrystal I2C**, and install.  
   (Tested with **Frank de Brabander’s LiquidCrystal_I2C** library.)
4. Clone or download this repository:
   ```bash
   git clone https://github.com/carolinedunn/game-night-arduino.git
   ```

   ---
   ## 📂 Project Series
This is part of my **Game Night Timer** series:  
- [Episode 1: Raspberry Pi LED Timer](https://youtu.be/0G3-ISume2o)  
- [Episode 2: Raspberry Pi LCD Timer](https://youtu.be/WSQV_xoQzLM)  
- [Episode 3: Raspberry Pi Bluetooth Speaker Timer](https://youtu.be/rIc2U7KOW9k)
- [Episode 4: Raspberry Pi Pico Game Night Timer ](https://youtu.be/uNjdEHFaG68))  
- [Episode 5: DIY Board Game Timer with Arduino Uno (LEDs, LCD, Buzzer)](https://youtu.be/IcNpnKvu7IY)
- [Episode 6: Hacking the Google AIY Kit v1 for Game Night](https://youtu.be/ric9_GkfvCg)

---

## 📖 License
MIT License — free to use, remix, and share.  
Attribution appreciated: link back to [Caroline Dunn’s channel](https://www.youtube.com/caroline).  

---

## 📚 Author
Created by **Caroline Dunn**  
- 🌐 [winningintech.com](https://winningintech.com/)  
- 📺 [YouTube.com/Caroline](https://www.youtube.com/caroline)  
- 📘 [A Woman’s Guide to Winning in Tech](https://amzn.to/3YxHVO7)  

