Internet-connected digital clock project I made to learn ESP-IDF and C.

---

# ESP32 Internet Clock (C)

A simple C program that simulates an **internet-connected digital clock**.
It utilizes the **ESP32**, a local **Wi-Fi network**, and an **NTP server**, allowing for exact global time synchronization and display on an I2C OLED screen.

---

## 📂 Project Structure

``` 
ESP32_Clock/
├── CMakeLists.txt        # Root build configuration
├── main/
│   ├── CMakeLists.txt    # Main component build configuration
│   ├── Kconfig.projbuild # Safe Wi-Fi menuconfig definitions
│   └── main_clock.c      # Entry point of the program (app_main)
└── README.md             # Project documentation
``` 

---

## ⚙️ Features

* Connect to **Wi-Fi** safely using ESP-IDF `menuconfig`
* Synchronize accurate global time via **SNTP** (`pool.ntp.org`)
* Drive a 0.96" OLED using a **custom I2C display driver**
* Render text using a custom **5x8 bitmap font** mapped in C arrays

---

## 🚀 How to Run

### 1. Clone this repository

```
git clone [https://github.com/SVerma2696/ESP32-Internet-Clock.git](https://github.com/SVerma2696/ESP32-Internet-Clock.git)
cd ESP32-Internet-Clock
```

### 2. Configure Wi-Fi safely

Make sure you are in the ESP-IDF environment, then run:

```
idf.py menuconfig
```


Navigate to **Clock Wi-Fi Configuration** and enter your SSID and Password.

### 3. Build and flash the program

```
idf.py -p COM5 flash monitor
```
*(Note: Change `COM5` to match your ESP32's actual serial port).*

---

## 🖥️ Example Interaction (Console Output)

```
I (1124) CLOCK: Wi-Fi initialization complete. Waiting for connection...
I (2458) CLOCK: Connected to Wi-Fi!
I (2460) CLOCK: Initializing SNTP
I (8462) CLOCK: Time synchronized successfully!
```

---


## 🔌 Hardware Wiring Guide

```
OLED GND -> ESP32 GND
OLED VCC -> ESP32 3V3 (Do NOT use VIN/5V)
OLED SCL -> ESP32 GPIO 22
OLED SDA -> ESP32 GPIO 21
```

---

## 📘 Concepts Demonstrated

* Embedded **C programming**
* **I2C Protocol** and hardware interfacing
* **FreeRTOS** event groups and task management
* Basic **network programming** (Wi-Fi STA mode & SNTP)
* **Secure configuration** using Kconfig to hide sensitive data

---

## 🔧 Requirements

* ELEGOO ESP-32 Super Starter Kit (or equivalent ESP32 board)
* 0.96" I2C OLED Display Module
* Visual Studio Code with the **ESP-IDF Extension** (v6.0.1+)

---

## UML Diagram:

```
classDiagram
class ESP32 {
+app_main() void
+wifi_init_sta() void
+initialize_sntp() void
}

class OLED {
    -I2C_ADDRESS : int
    -SDA_PIN : int
    -SCL_PIN : int
    +ssd1306_init() void
    +ssd1306_clear() void
    +draw_text(x : int, y : int, str : char*, scale : int) void
}

class SNTP {
    -server : String
}

ESP32 --> OLED : drives via I2C
ESP32 --> SNTP : syncs via Wi-Fi
```
