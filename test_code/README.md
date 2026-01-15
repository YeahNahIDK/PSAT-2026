# PSAT Payload – ESP32-C3 Test Code

## What this repo is

This repository contains **test code** for the PSAT rocket payload based on an  
**ESP32-C3 Super Mini**.

This is **not flight code**.

The purpose of this project is to:
- test wiring
- test communication between boards
- test sensors and storage
- make sure everything can run together **before** writing real flight logic

Think of this as a **hardware + communication sandbox**.

---

## What the ESP32 is doing

The ESP32 acts as the **central coordinator** during testing.

It:
- reads sensors (BME680, MPU6050)
- writes data to an SD card
- sends telemetry over UART
- sends simple commands over UART

It intentionally **does not**:
- drive servos directly
- control a LoRa radio directly
- implement deployment or flight logic

Those responsibilities live on **other boards** (e.g. MSP430).

---

## Why the project is structured this way

The project contains **multiple test modes**, but only **one runs at a time**.

Each mode exists to answer a specific question, for example:
- “Does UART communication work?”
- “Can the SD card log data reliably?”
- “Do sensors still work when everything runs together?”

This avoids mixing too many variables while debugging.

---

## Test modes (selected at compile time)

The active test mode is chosen in `platformio.ini`.

Available test modes include:

- **Servo UART test**  
  ESP32 sends simple text commands over UART to another board  
  (used to validate communication, not real servo control).

- **LoRa telemetry UART test**  
  ESP32 sends telemetry data over UART to another board  
  (which later handles SPI + radio transmission).

- **Sensors test**  
  ESP32 reads the BME680 and MPU6050 and prints values to serial.

- **SD card test**  
  ESP32 mounts the SD card and writes timestamped data to a file.

- **Combined test**  
  Sensors, SD logging, and telemetry run together to check system behaviour.

Only one test mode is enabled at a time to keep behaviour predictable.

---

## Important design decisions

- The ESP32 only **produces data** and **sends commands**
- Servo control and LoRa radio control are handled by **separate boards**
- Communication between boards uses **simple UART messages**
- No blocking `delay()` calls are used
- Tasks are separated so components don’t block each other

This makes it easier to:
- debug issues
- swap hardware
- reuse code later

---

## What is intentionally missing

This repo does **not** contain:
- real servo control code
- real LoRa SPI/radio code
- flight or deployment logic

Those will live in **separate firmware** on their respective boards.

This repo only tests that the **interfaces between boards work correctly**.

---

## How to use this repo

1. Select a test mode in `platformio.ini`
2. Build and upload to the ESP32-C3
3. Observe behaviour via:
   - serial output
   - SD card files
   - connected test hardware

When all tests are passing, real flight firmware can be written with confidence.

---

## Final note

If something feels “unfinished” — that is intentional.

This project exists to reduce risk before flight code is written.
