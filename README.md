# Gesture-Controlled Smart Home System with NRF24L01 Wireless Communication

A two-unit wireless home automation system that lets you control four household appliances (two lamps, a fan, and a socket) using hand gestures. The APDS9960 gesture sensor on the transmitter unit detects swipe directions, which are encoded as text commands and sent over 2.4 GHz RF to the receiver unit, which drives four AC appliances through active-LOW relays.

**Portfolio page:** https://damilareadekeye.com/works/embedded-systems/gesture-smart-home/

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

| Gesture    | Command | Device Controlled |
|------------|---------|-------------------|
| Swipe UP   | U ON/OFF | Lamp 1           |
| Swipe DOWN | D ON/OFF | Lamp 2           |
| Swipe LEFT | L ON/OFF | Fan              |
| Swipe RIGHT| R ON/OFF | Socket           |
| (Serial)   | ALL ON/OFF | All devices   |

Each gesture toggles the target device; the current state is tracked on both the OLED (transmitter) and LCD (receiver).

---

## Hardware

### Transmitter

| Component | Part | Notes |
|-----------|------|-------|
| Microcontroller | Arduino Nano | |
| Gesture sensor | APDS9960 | I2C address 0x39, Chip ID 0x9E |
| Display | SH1106 1.3" OLED 128x64 | I2C, U8g2 library |
| RF module | NRF24L01 | CE=D9, CSN=D10 |
| Status LEDs | 4x yellow LEDs | A0-A3, mirror relay states |
| Battery charger | TP4056 | |
| Battery | Tecno BL-38AT 3850mAh Li-Polymer | |

### Receiver

| Component | Part | Notes |
|-----------|------|-------|
| Microcontroller | Arduino Nano | |
| Display | 20x4 I2C LCD | Address 0x27 |
| RF module | NRF24L01 | CE=D9, CSN=D10 |
| Relay module | 4-channel active-LOW | A0=Socket, A1=Fan, A2=Lamp2, A3=Lamp1 |
| Buzzer | Passive buzzer | D2 |
| Regulator | LM2596 Buck Converter | Steps down from AC-DC PSU |
| Power supply | AC-DC PSU | Mains-powered |

---

## Libraries Required

Install all libraries via the Arduino Library Manager before uploading.

**Transmitter:**
- RF24 by TMRh20
- U8g2 by oliver
- Adafruit APDS9960 (use Adafruit library, not SparkFun; the module is a counterfeit variant with Chip ID 0x9E that only works with the Adafruit driver)

**Receiver:**
- RF24 by TMRh20
- LiquidCrystal I2C by Frank de Brabander

---

## Quick Start

### 1. Upload transmitter firmware

Open `firmware/transmitter/transmitter_main/transmitter_main.ino` in the Arduino IDE.

Select board: **Arduino Nano**, processor: **ATmega328P (Old Bootloader)** if needed.

Upload to the transmitter Nano.

### 2. Upload receiver firmware

Open `firmware/receiver/receiver_main/receiver_main.ino`.

Select the same board settings.

Upload to the receiver Nano.

### 3. Power on

Power the receiver unit from the mains via the AC-DC PSU and LM2596. Power the transmitter from the Li-Po battery via the TP4056.

Both units show a 6-second splash screen with the student name and matric number on startup, then enter ready state.

### 4. Control

Swipe your hand over the APDS9960 sensor in the UP, DOWN, LEFT, or RIGHT direction. The OLED confirms the detected gesture; the LCD on the receiver unit confirms the executed command and the buzzer beeps once per command.

---

## Known Issues and Fixes

**Relay RF interference:** The relay switching generates noise that disrupts NRF24L01 acknowledgements. Fixed by setting `radio.setRetries(15, 15)` on the transmitter, which retries each packet up to 15 times with 15 retry delays.

**Relay settle time:** Added `delay(50)` between `applyOutputs()` and `beep()` on the receiver so the relay has time to stabilize before the buzzer draws current.

**APDS9960 library:** The module used is a counterfeit variant. The SparkFun APDS9960 library does not detect gestures on it. Use the Adafruit APDS9960 library instead.

---

## Repository Structure

```
firmware/
  transmitter/
    transmitter_main/
      transmitter_main.ino    <- production transmitter sketch
  receiver/
    receiver_main/
      receiver_main.ino       <- production receiver sketch
  reference-sketches/
    nrf24l01-tx-rx-text-test/ <- basic NRF24L01 TX/RX test used during dev
      Receiver/Receiver.ino
      Transmitter/Transmitter.ino
docs/
  project-knowledge-base.pdf  <- full project documentation
```

---

## Student Information

**Name:** Gbenga-Idowu Afeoluwa  
**Matric:** 21ck029291
