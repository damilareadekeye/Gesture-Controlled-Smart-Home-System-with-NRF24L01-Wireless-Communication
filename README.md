# Gesture-Controlled Smart Home System with NRF24L01 Wireless Communication

![Platform](https://img.shields.io/badge/Platform-Arduino%20Nano-00979D?logo=arduino&logoColor=white)
![RF](https://img.shields.io/badge/RF-NRF24L01%202.4GHz-blue)
![Sensor](https://img.shields.io/badge/Sensor-APDS9960-purple)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20(Arduino)-orange)

A two-unit wireless home automation system that controls four AC appliances using hand gestures. The APDS9960 gesture sensor on the handheld transmitter detects swipe directions, encodes them as short text commands, and transmits them over 2.4 GHz RF via NRF24L01 to a mains-powered receiver unit, which drives the appliances through a 4-channel active-LOW relay module.

**Portfolio page:** https://damilareadekeye.com/works/embedded-systems/gesture-smart-home/

---

## Features

- Touchless, contactless control of four household appliances
- Four gesture directions mapped to four independent devices: Lamp 1, Lamp 2, Fan, and Socket
- Each gesture toggles its device; state is retained across gestures
- OLED display on the transmitter confirms gesture detected, device name, and ON/OFF state
- 20x4 LCD on the receiver confirms each received command
- Buzzer beep on the receiver provides audio confirmation per command
- Battery-powered transmitter (Li-Polymer + TP4056 charger) for cordless handheld use
- Mains-powered receiver with LM2596 buck converter for stable supply to the Nano
- Serial monitor override on the transmitter for manual command entry during development

---

## System Architecture

```
TRANSMITTER UNIT                           RECEIVER UNIT
+---------------------------+              +---------------------------+
| Arduino Nano              |              | Arduino Nano              |
|                           |              |                           |
| APDS9960 Gesture Sensor   |              | 20x4 I2C LCD (0x27)       |
|   I2C 0x39                |              |                           |
|                           |              | 4-Channel Relay Module    |
| SH1106 OLED 128x64        |              |   A0 = Socket (active-LOW)|
|   I2C (U8g2)              |              |   A1 = Fan    (active-LOW)|
|                           |              |   A2 = Lamp 2 (active-LOW)|
| 4 Status LEDs             |              |   A3 = Lamp 1 (active-LOW)|
|   A0=Socket A1=Fan        |              |                           |
|   A2=Lamp2  A3=Lamp1      |              | Buzzer (D2)               |
|                           |              |                           |
| NRF24L01   CE=D9 CSN=D10  |---[2.4GHz]-->| NRF24L01  CE=D9 CSN=D10  |
|   setRetries(15,15)       |              |                           |
|                           |              | LM2596 Buck Converter     |
| TP4056 + 3850mAh Li-Po    |              | AC-DC PSU                 |
+---------------------------+              +---------------------------+
```

---

## Gesture Mapping

| Gesture     | Command     | Device    | Relay Pin |
|-------------|-------------|-----------|-----------|
| Swipe UP    | U ON / U OFF  | Lamp 1  | A3        |
| Swipe DOWN  | D ON / D OFF  | Lamp 2  | A2        |
| Swipe LEFT  | L ON / L OFF  | Fan     | A1        |
| Swipe RIGHT | R ON / R OFF  | Socket  | A0        |
| (Serial)    | ALL ON / ALL OFF | All devices | A0-A3 |

Each gesture sends a 10-byte null-terminated string payload over NRF24L01. The receiver parses the command string and toggles the appropriate relay.

---

## Hardware

### Transmitter Unit

| Component | Part | Connection |
|-----------|------|------------|
| Microcontroller | Arduino Nano (ATmega328P) | |
| Gesture sensor | APDS9960 | I2C: SDA=A4, SCL=A5 (address 0x39) |
| OLED display | SH1106 1.3" 128x64 | I2C: SDA=A4, SCL=A5 (U8g2 library) |
| RF transceiver | NRF24L01 | SPI: CE=D9, CSN=D10, MOSI=D11, MISO=D12, SCK=D13, VCC=3.3V |
| LED - Socket status | Yellow LED | A0 via current-limiting resistor |
| LED - Fan status | Yellow LED | A1 via current-limiting resistor |
| LED - Lamp 2 status | Yellow LED | A2 via current-limiting resistor |
| LED - Lamp 1 status | Yellow LED | A3 via current-limiting resistor |
| Battery charger | TP4056 module | |
| Battery | Tecno BL-38AT 3850mAh Li-Polymer | |

### Receiver Unit

| Component | Part | Connection |
|-----------|------|------------|
| Microcontroller | Arduino Nano (ATmega328P) | |
| LCD display | 20x4 I2C LCD | I2C: SDA=A4, SCL=A5 (address 0x27) |
| RF transceiver | NRF24L01 | SPI: CE=D9, CSN=D10, MOSI=D11, MISO=D12, SCK=D13, VCC=3.3V |
| Relay - Socket | 4-channel active-LOW relay, CH1 | A0 (LOW = ON) |
| Relay - Fan | 4-channel active-LOW relay, CH2 | A1 (LOW = ON) |
| Relay - Lamp 2 | 4-channel active-LOW relay, CH3 | A2 (LOW = ON) |
| Relay - Lamp 1 | 4-channel active-LOW relay, CH4 | A3 (LOW = ON) |
| Buzzer | Passive buzzer | D2 |
| Voltage regulator | LM2596 buck converter | Steps down from AC-DC PSU to 7-9V for Nano VIN |
| Power supply | AC-DC PSU | Mains input, DC output |

---

## Libraries Required

Install via **Sketch > Include Library > Manage Libraries** in the Arduino IDE.

### Transmitter

| Library | Author | Notes |
|---------|--------|-------|
| RF24 | TMRh20 | NRF24L01 driver |
| U8g2 | oliver | SH1106 OLED driver |
| Adafruit APDS9960 | Adafruit | Gesture sensor driver. Use this, NOT the SparkFun library (see Troubleshooting) |

### Receiver

| Library | Author | Notes |
|---------|--------|-------|
| RF24 | TMRh20 | NRF24L01 driver |
| LiquidCrystal I2C | Frank de Brabander | 20x4 I2C LCD driver |

---

## Quick Start

### 1. Wire both units

Follow the connection tables above. On the NRF24L01, add a 10uF electrolytic capacitor across VCC and GND close to the module to stabilize the 3.3V supply.

### 2. Install libraries

Open the Arduino IDE Library Manager and install all libraries listed above for each unit.

### 3. Upload transmitter firmware

Open `firmware/transmitter/transmitter_main/transmitter_main.ino`.

Select board: **Arduino Nano**, processor: **ATmega328P (Old Bootloader)** if your Nano uses the old bootloader.

Upload to the transmitter Nano.

### 4. Upload receiver firmware

Open `firmware/receiver/receiver_main/receiver_main.ino`.

Use the same board settings. Upload to the receiver Nano.

### 5. Power on

Power the receiver from mains via the AC-DC PSU and LM2596. Power the transmitter from the Li-Polymer battery via the TP4056 output.

Both units display a 6-second splash screen on startup (student name and matric number), then enter ready state.

### 6. Operate

Hold your hand about 5-10 cm above the APDS9960 sensor, then swipe in a direction. The OLED on the transmitter confirms the gesture and device state; the LCD on the receiver confirms the received command; the buzzer beeps once.

---

## Serial Command Reference

The transmitter also accepts typed commands over the serial monitor at **115200 baud**. Useful for testing the RF link without needing gestures.

| Command   | Action                        |
|-----------|-------------------------------|
| `U ON`    | Turn Lamp 1 ON                |
| `U OFF`   | Turn Lamp 1 OFF               |
| `D ON`    | Turn Lamp 2 ON                |
| `D OFF`   | Turn Lamp 2 OFF               |
| `L ON`    | Turn Fan ON                   |
| `L OFF`   | Turn Fan OFF                  |
| `R ON`    | Turn Socket ON                |
| `R OFF`   | Turn Socket OFF               |
| `ALL ON`  | Turn all four devices ON      |
| `ALL OFF` | Turn all four devices OFF     |

Commands are case-insensitive (the firmware calls `.toUpperCase()` before parsing).

---

## How It Works

1. The APDS9960 sensor is polled each loop iteration via `apds.readGesture()`.
2. A detected gesture direction maps to a toggle of the corresponding boolean state variable (`stateA0` through `stateA3`).
3. The state is encoded into a command string (e.g., `"U ON"` or `"R OFF"`), zero-padded into a 10-byte `char` array, and written to the NRF24L01 via `radio.write()`.
4. The receiver listens continuously. On `radio.available()`, it reads the 10-byte payload, trims whitespace, and compares the string against known commands.
5. Matching commands update the relay state booleans and call `applyOutputs()`, which writes the inverted boolean to each relay pin (active-LOW logic: `true` state drives the pin `LOW`, energising the relay coil).
6. After a short settle delay, the LCD updates and the buzzer beeps.

---

## Troubleshooting

**APDS9960 not detecting gestures**

The module used in this project is a counterfeit variant. Its I2C address is 0x39 and its Chip ID register reads 0x9E. The SparkFun APDS9960 library does not work with it. You must use the **Adafruit APDS9960** library. If gestures still fail, confirm the sensor is initialising via `apds.begin()` returning `true`.

**NRF24L01 packets not arriving or dropping**

Relay switching injects noise onto the power rails, which disrupts NRF24L01 ACK packets. The transmitter uses `radio.setRetries(15, 15)` to retry each packet up to 15 times. If drops persist, add a 10uF capacitor across the NRF24L01 VCC/GND pins and ensure the module is powered from 3.3V (not 5V).

**Relay chatters or buzzer fires at wrong time**

The receiver adds `delay(50)` between `applyOutputs()` and `beep()` so the relay coil has time to settle before the buzzer draws current. Removing this delay can cause false triggers on the buzzer.

**LCD shows garbage or nothing**

Confirm the I2C address is 0x27. Run an I2C scanner sketch if unsure. Adjust the contrast potentiometer on the back of the LCD module.

**Commands work via serial but not via gesture**

Ensure the APDS9960 is fully initialised (check serial output for "Gesture Sensor Initialized!"). The sensor requires proximity mode enabled before gesture mode will trigger. This is handled in the firmware but will silently fail if `apds.begin()` returns false.

---

## Repository Structure

```
firmware/
  transmitter/
    transmitter_main/
      transmitter_main.ino      <- production transmitter sketch
  receiver/
    receiver_main/
      receiver_main.ino         <- production receiver sketch
  reference-sketches/
    nrf24l01-tx-rx-text-test/   <- minimal NRF24L01 TX/RX text test (dev reference)
      Receiver/
        Receiver.ino
      Transmitter/
        Transmitter.ino
docs/
  project-knowledge-base.pdf   <- full project documentation
README.md
```

---

## Student Information

**Name:** Gbenga-Idowu Afeoluwa  
**Matric:** 21ck029291
