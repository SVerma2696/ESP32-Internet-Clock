Internet-connected digital clock project I made to learn ESP-IDF and C.

# ESP32 Internet Clock (C)

A simple C program that simulates an **internet-connected digital clock.**
It utilizes the **ESP32**, a local **Wi-Fi network**, and an **NTP server**, allowing for exact global time synchronization and display on an I2C OLED screen. It includes push-buttons for interactive configurations!

---

## 📂 Project Structure

```
ESP32_Clock_Toggle/
├── CMakeLists.txt          # Root build configuration
├── main/
│   ├── CMakeLists.txt      # Main component build configuration
│   ├── Kconfig.projbuild   # Safe Wi-Fi menuconfig definitions
│   ├── analog_clock.c      # Algorithm for drawing the rectangular clock face, hands, and numbers
│   ├── analog_clock.h      # Header for Analog drawing module
│   ├── buttons.c           # FreeRTOS task handling button inputs, debounce, and state logic
│   ├── buttons.h           # Header for Button handler module
│   └── main_clock_toggle.c # Entry point of the program (app_main)
└── README.md               # Project documentation
```

---

## ⚙️ Features

* Connect to **Wi-Fi** safely using ESP-IDF `menuconfig`
* Synchronize accurate global time via **SNTP** (`pool.ntp.org`)
* Switch seamlessly between a **Rectangular Analog mode** (hands and stylized numbers fitting the 128x64 display) and Digital mode via physical button push.
* Switch between **12-Hour Standard** (with a centered AM/PM indicator) and **24-Hour Military** Time representations.
* **Smart Button Logic:** Pressing *either* button while in analog mode gracefully returns the display to digital mode.
* Drive a 0.96" OLED using a **custom I2C display driver**
* Render text using a custom **5x8 bitmap font** mapped in C arrays

---

## 🚀 How to Run

### 1. Clone this repository

```
git clone [https://github.com/SVerma2696/ESP32-Internet-Clock.git](https://github.com/SVerma2696/ESP32-Internet-Clock.git)
cd ESP32-Internet-Clock
```

### 2. Switch to the Advanced Branch Version

The default `main` branch contains a basic digital clock. To use this version with buttons and analog modes, checkout this branch:
```
git checkout feature/analog-toggle
```

### 3. Configure Wi-Fi safely

Make sure you are in the ESP-IDF environment, then run:
```
idf.py menuconfig
```

Navigate to **Clock Wi-Fi Configuration** and enter your SSID and Password.

### 4. Build and flash the program

```
idf.py -p COM5 flash monitor
```
*(Note: Change `COM5` to match your ESP32's actual serial port).*

---

## 🔌 Hardware Wiring Guide

### Screen (I2C)
```
OLED GND -> ESP32 GND
OLED VCC -> ESP32 3V3 (Do NOT use VIN/5V)
OLED SCL -> ESP32 GPIO 22
OLED SDA -> ESP32 GPIO 21
```

### Buttons

**Note:** Internal pull-up resistors are enabled via software. You do not need to wire external resistors!
```
BTN 1 (12h/24h toggle / Exit Analog) -> One side to GPIO 18, the other to GND.
BTN 2 (Analog/Digital Toggle)        -> One side to GPIO 19, the other to GND.
```

---

## 📘 Concepts Demonstrated

* Embedded **C programming** (Modular Multi-File setup)
* **I2C Protocol** and hardware interfacing
* **FreeRTOS** event groups, task management, and GPIO polling
* **Software Debouncing** for physical hardware switches
* Mathematics in embedded systems (Polar/Cartesian conversion for Analog Hands, elliptical scaling)
* Basic **network programming** (Wi-Fi STA mode & SNTP)
* **Secure configuration** using Kconfig to hide sensitive data

---

## 🔧 Requirements

* ELEGOO ESP-32 Super Starter Kit (or equivalent ESP32 board)
* 0.96" I2C OLED Display Module (Yellow/Blue dual-color or single color)
* 2x Physical Push Buttons
* Visual Studio Code with the ESP-IDF Extension (v6.0.1+)

---

## UML Diagram:
```
classDiagram
class ESP32 {
+app_main() void
+wifi_init_sta() void
+initialize_sntp() void
}

class Buttons {
+buttons_init() void
+get_12h_mode() bool
+get_analog_mode() bool
-button_task() void
}

class AnalogClock {
+draw_analog_clock(h, m, s) void
-draw_line(x0, y0, x1, y1) void
}

class OLED {
    -I2C_ADDRESS : int
    -SDA_PIN : int
    -SCL_PIN : int
    +ssd1306_init() void
    +ssd1306_clear() void
    +draw_text(x, y, str, scale) void
    +draw_pixel(x, y, color) void
}

class SNTP {
    -server : String
}

ESP32 --> OLED : drives via I2C
ESP32 --> SNTP : syncs via Wi-Fi
ESP32 --> Buttons : spawns background task
ESP32 --> AnalogClock : utilizes drawing algorithms
AnalogClock --> OLED : plots coordinates & text
```