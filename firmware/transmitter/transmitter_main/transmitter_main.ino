#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <U8g2lib.h>
#include "Adafruit_APDS9960.h"

// Gesture Sensor
Adafruit_APDS9960 apds;

// NRF Configuration
RF24 radio(9, 10);  // CE, CSN
const byte address[10] = "ADDRESS01";

// OLED
U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// LED states
bool stateA0 = false;  // Socket
bool stateA1 = false;  // Fan
bool stateA2 = false;  // Lamp2
bool stateA3 = false;  // Lamp1

// OLED display helper
void showOLED(const char* gestureName, const char* device, const char* status) {
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(0, 10, "GESTURE CONTROL");
    oled.drawHLine(0, 13, 128);

    oled.drawStr(0, 26, "Action: ");
    oled.drawStr(45, 26, gestureName);

    oled.setFont(u8g2_font_10x20_tf);
    oled.drawStr(0, 46, device);

    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(0, 62, status);

  } while (oled.nextPage());
}

void setup() {
  Serial.begin(115200);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);

  Wire.begin();
  oled.begin();

  // --- SPLASH SCREEN: NAME & MATRIC NUMBER ---
  oled.firstPage();
  do {
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(0, 10, "Name: GBENGA-IDOWU");
    oled.drawStr(0, 24, "Afeoluwa");
    oled.drawStr(0, 38, "Matric:");

    oled.setFont(u8g2_font_10x20_tf);
    oled.drawStr(0, 60, "21ck029291");
  } while (oled.nextPage());

  delay(6000);

  // NRF setup
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);

  // CRITICAL NEW LINE: Aggressively retry 15 times to cut through Relay interference!
  radio.setRetries(15, 15);

  radio.stopListening();

  // APDS9960 Setup
  if (!apds.begin()) {
    Serial.println("Gesture Sensor Failed! Check wiring.");
  } else {
    Serial.println("Gesture Sensor Initialized!");
    apds.enableProximity(true);
    apds.enableGesture(true);
  }

  showOLED("WAITING", "SYSTEM READY", "Status: Swipe Hand!");

  Serial.println("=== CONTROL READY ===");
}

void loop() {

  String cmdStr = "";
  const char* gestureDetected = "NONE";

  // 1. CHECK GESTURE
  uint8_t gesture = apds.readGesture();

  if (gesture > 0) {
    if (gesture == APDS9960_UP) {
      stateA3 = !stateA3;
      cmdStr = stateA3 ? "U ON" : "U OFF";
      gestureDetected = "SWIPE UP";
    } else if (gesture == APDS9960_DOWN) {
      stateA2 = !stateA2;
      cmdStr = stateA2 ? "D ON" : "D OFF";
      gestureDetected = "SWIPE DOWN";
    } else if (gesture == APDS9960_LEFT) {
      stateA1 = !stateA1;
      cmdStr = stateA1 ? "L ON" : "L OFF";
      gestureDetected = "SWIPE LEFT";
    } else if (gesture == APDS9960_RIGHT) {
      stateA0 = !stateA0;
      cmdStr = stateA0 ? "R ON" : "R OFF";
      gestureDetected = "SWIPE RIGHT";
    }
  }

  // 2. CHECK SERIAL
  if (Serial.available()) {
    cmdStr = Serial.readStringUntil('\n');
    cmdStr.trim();
    cmdStr.toUpperCase();
    gestureDetected = "TYPED MANUAL";
  }

  // 3. PROCESS COMMAND
  if (cmdStr.length() > 0) {
    const char* device = "";
    const char* status = "";
    bool valid = true;

    if (cmdStr == "U ON") {
      stateA3 = true;
      device = "LAMP 1";
      status = "State: ON";
    } else if (cmdStr == "U OFF") {
      stateA3 = false;
      device = "LAMP 1";
      status = "State: OFF";
    } else if (cmdStr == "D ON") {
      stateA2 = true;
      device = "LAMP 2";
      status = "State: ON";
    } else if (cmdStr == "D OFF") {
      stateA2 = false;
      device = "LAMP 2";
      status = "State: OFF";
    } else if (cmdStr == "L ON") {
      stateA1 = true;
      device = "FAN";
      status = "State: ON";
    } else if (cmdStr == "L OFF") {
      stateA1 = false;
      device = "FAN";
      status = "State: OFF";
    } else if (cmdStr == "R ON") {
      stateA0 = true;
      device = "SOCKET";
      status = "State: ON";
    } else if (cmdStr == "R OFF") {
      stateA0 = false;
      device = "SOCKET";
      status = "State: OFF";
    } else if (cmdStr == "ALL ON") {
      stateA0 = stateA1 = stateA2 = stateA3 = true;
      device = "ALL DEVICES";
      status = "State: ON";
    } else if (cmdStr == "ALL OFF") {
      stateA0 = stateA1 = stateA2 = stateA3 = false;
      device = "ALL DEVICES";
      status = "State: OFF";
    } else {
      valid = false;
    }

    if (valid) {
      digitalWrite(A3, stateA3);
      digitalWrite(A2, stateA2);
      digitalWrite(A1, stateA1);
      digitalWrite(A0, stateA0);

      Serial.print("Executing: ");
      Serial.println(cmdStr);

      // CRITICAL FIX: Ensure the array starts completely empty so no memory garbage is sent!
      char payload[10] = { 0 };
      cmdStr.toCharArray(payload, sizeof(payload));

      // SEND COMMAND VIA NRF
      radio.write(&payload, sizeof(payload));
      Serial.println("NRF: Command Sent!");

      showOLED(gestureDetected, device, status);

      delay(200);
    }
  }
}

// /*
//   COMPLETE TRANSMITTER

//   OLED 1.3"  → SDA=A4, SCL=A5
//   nRF24L01   → CE=D9, CSN=D10

//   LEDs:
//     A0 → Socket
//     A1 → Fan
//     A2 → Lamp2
//     A3 → Lamp1
// */
// #include <Wire.h>
// #include <SPI.h>
// #include <RF24.h>
// #include <nRF24L01.h>
// #include <U8g2lib.h>

// // NRF Configuration
// RF24 radio(9, 10);  // CE, CSN
// const byte address[10] = "ADDRESS01";

// // OLED
// U8G2_SH1106_128X64_NONAME_1_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

// // LED states
// bool stateA0 = false;  // Socket
// bool stateA1 = false;  // Fan
// bool stateA2 = false;  // Lamp2
// bool stateA3 = false;  // Lamp1

// // OLED display helper
// void showOLED(const char* device, const char* status) {
//   oled.firstPage();
//   do {
//     oled.setFont(u8g2_font_6x10_tf);
//     oled.drawStr(0, 12, "GESTURE CONTROL");
//     oled.drawHLine(0, 15, 128);

//     oled.setFont(u8g2_font_10x20_tf);
//     oled.drawStr(0, 42, device);

//     oled.setFont(u8g2_font_6x10_tf);
//     oled.drawStr(0, 62, status);

//   } while (oled.nextPage());
// }

// void setup() {
//   Serial.begin(115200);

//   pinMode(A0, OUTPUT);
//   pinMode(A1, OUTPUT);
//   pinMode(A2, OUTPUT);
//   pinMode(A3, OUTPUT);

//   digitalWrite(A0, LOW);
//   digitalWrite(A1, LOW);
//   digitalWrite(A2, LOW);
//   digitalWrite(A3, LOW);

//   Wire.begin();
//   oled.begin();

//   // NRF setup
//   radio.begin();
//   radio.openWritingPipe(address);
//   radio.setPALevel(RF24_PA_MIN);
//   radio.stopListening();

//   showOLED("READY", "Use Serial");

//   Serial.println("=== CONTROL READY ===");
//   Serial.println("Commands:");
//   Serial.println(" U ON  / U OFF  (Lamp 1)");
//   Serial.println(" D ON  / D OFF  (Lamp 2)");
//   Serial.println(" L ON  / L OFF  (Fan)");
//   Serial.println(" R ON  / R OFF  (Socket)");
//   Serial.println(" ALL ON / ALL OFF");
// }

// void loop() {
//   // Check if string data is being typed in the Serial Monitor
//   if (Serial.available()) {

//     String cmdStr = Serial.readStringUntil('\n');  // Read entire typed string
//     cmdStr.trim();                                 // Remove invisible newline spaces
//     cmdStr.toUpperCase();                          // Convert "u on" to "U ON" automatically!

//     if (cmdStr.length() == 0) return;  // Ignore blank lines

//     const char* device = "";
//     const char* status = "";
//     bool valid = true;

//     // Check which explicit text was typed
//     if (cmdStr == "U ON") {
//       stateA3 = true;
//       device = "LAMP 1";
//       status = "ON";
//     } else if (cmdStr == "U OFF") {
//       stateA3 = false;
//       device = "LAMP 1";
//       status = "OFF";
//     } else if (cmdStr == "D ON") {
//       stateA2 = true;
//       device = "LAMP 2";
//       status = "ON";
//     } else if (cmdStr == "D OFF") {
//       stateA2 = false;
//       device = "LAMP 2";
//       status = "OFF";
//     } else if (cmdStr == "L ON") {
//       stateA1 = true;
//       device = "FAN";
//       status = "ON";
//     } else if (cmdStr == "L OFF") {
//       stateA1 = false;
//       device = "FAN";
//       status = "OFF";
//     } else if (cmdStr == "R ON") {
//       stateA0 = true;
//       device = "SOCKET";
//       status = "ON";
//     } else if (cmdStr == "R OFF") {
//       stateA0 = false;
//       device = "SOCKET";
//       status = "OFF";
//     } else if (cmdStr == "ALL ON") {
//       stateA0 = stateA1 = stateA2 = stateA3 = true;
//       device = "ALL DEVICES";
//       status = "ON";
//     } else if (cmdStr == "ALL OFF") {
//       stateA0 = stateA1 = stateA2 = stateA3 = false;
//       device = "ALL DEVICES";
//       status = "OFF";
//     } else {
//       Serial.println("Invalid! Type 'U ON', 'D OFF', 'ALL ON', etc.");
//       valid = false;
//     }

//     if (valid) {
//       // Hardware writes
//       digitalWrite(A3, stateA3);
//       digitalWrite(A2, stateA2);
//       digitalWrite(A1, stateA1);
//       digitalWrite(A0, stateA0);

//       // Print what is happening locally
//       Serial.print(device);
//       Serial.print(": ");
//       Serial.println(status);

//       // 📡 Prepare NRF string payload (Max 10 chars)
//       char payload[10];
//       cmdStr.toCharArray(payload, sizeof(payload));

//       // SEND COMMAND VIA NRF
//       radio.write(&payload, sizeof(payload));
//       Serial.println("NRF: Command Sent!");

//       // Update OLED
//       showOLED(device, status);
//     }
//   }
// }

// // -------i2c Scanner------------------------------------------------------------------------------------------------------------
// /*
//   Rui Santos & Sara Santos - Random Nerd Tutorials
//   Complete project details at https://RandomNerdTutorials.com/arduino-apds9960-gesture-sensor/
//   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
//   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// */

// #include <Wire.h>

// // Define APDS9960 chip ID register
// #define APDS9960_ID_REG 0x92 // Chip ID register address

// // Function to read chip ID from a given I2C address
// uint8_t readChipID(uint8_t i2cAddress) {
//   Wire.beginTransmission(i2cAddress);
//   Wire.write(APDS9960_ID_REG); // Request chip ID register
//   Wire.endTransmission(false);
//   Wire.requestFrom(i2cAddress, (uint8_t)1);
//   if (Wire.available()) {
//     return Wire.read(); // Read chip ID
//   }
//   return 0; // Return 0 if no data (invalid or no response)
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);
//   Serial.println("Starting I2C Scanner and Chip ID Reader");

//   // Initialize I2C
//   Wire.begin();
//   Wire.setClock(100000);

//   // Scan I2C bus for devices
//   Serial.println("\nScanning I2C bus...");
//   bool deviceFound = false;

//   for (uint8_t address = 0x08; address <= 0x7F; address++) {
//     Wire.beginTransmission(address);
//     uint8_t error = Wire.endTransmission();

//     if (error == 0) {
//       deviceFound = true;
//       Serial.print("I2C device found at address 0x");
//       if (address < 16) Serial.print("0");
//       Serial.print(address, HEX);

//       // Read and print chip ID
//       uint8_t chipID = readChipID(address);
//       Serial.print(" - Chip ID: 0x");
//       if (chipID < 16) Serial.print("0");
//       Serial.println(chipID, HEX);
//     }
//   }

//   if (!deviceFound) {
//     Serial.println("No I2C devices found. Check wiring or try again.");
//   } else {
//     Serial.println("\nScan complete.");
//   }
// }

// void loop() {
// }

// /*
// Scanning I2C bus...
// I2C device found at address 0x39 - Chip ID: 0x9E
// I2C device found at address 0x3C - Chip ID: 0x56
// */
// // --------------------------------------------------------------------------------------------------------------------



// /* This is a library for the APDS9960 digital proximity, ambient light, RGB, and gesture sensor
//    This sketch puts the sensor in gesture mode and decodes gestures. To use this, first put your hand close to the sensor to enable gesture mode.
//   Then move your hand about 6" from the sensor in the up -> down, down -> up, left -> right, or right -> left direction.
//   Designed specifically to work with the Adafruit APDS9960 breakout ----> http://www.adafruit.com/products/3595
//   These sensors use I2C to communicate. The device's I2C address is 0x39 Adafruit invests time and resources providing this open source code, please support Adafruit andopen-source hardware by purchasing products from Adafruit! Written by Dean Miller for Adafruit Industries. BSD license, all text above must be included in any redistribution */

// // Complete project details at https://RandomNerdTutorials.com/arduino-apds9960-gesture-sensor/

// #include "Adafruit_APDS9960.h"
// Adafruit_APDS9960 apds;

// // the setup function runs once when you press reset or power the board
// void setup() {
//   Serial.begin(115200);

//   if(!apds.begin()){
//     Serial.println("failed to initialize device! Please check your wiring.");
//   }
//   else Serial.println("Device initialized!");

//   //gesture mode will be entered once proximity mode senses something close
//   apds.enableProximity(true);
//   apds.enableGesture(true);
// }

// // the loop function runs over and over again forever
// void loop() {
//   uint8_t gesture = apds.readGesture();
//   if (gesture == APDS9960_DOWN) {
//     Serial.println("v Moving DOWN");
//   }
//   if (gesture == APDS9960_UP) {
//      Serial.println("^ Moving UP");
//   }
//   if (gesture == APDS9960_LEFT) {
//     Serial.println("< Moving LEFT");
//   }
//   if (gesture == APDS9960_RIGHT) {
//     Serial.println("> Moving RIGHT");
//   }
// }
// --------------------------------------------------------------------------------------------------------------------
// #include <Wire.h>

// #define APDS 0x39

// void writeReg(byte reg, byte val) {
//   Wire.beginTransmission(APDS);
//   Wire.write(reg);
//   Wire.write(val);
//   Wire.endTransmission();
// }

// byte readReg(byte reg) {
//   Wire.beginTransmission(APDS);
//   Wire.write(reg);
//   Wire.endTransmission();
//   Wire.requestFrom(APDS, 1);
//   return Wire.read();
// }

// void setup() {
//   Serial.begin(115200);
//   Wire.begin();

//   delay(200);

//   // Power ON + Gesture enable
//   writeReg(0x80, 0b01000101);  // PON | GEN | PEN

//   // Gesture configuration
//   writeReg(0xA2, 0x40);  // GCONF1
//   writeReg(0xA3, 0x40);  // GCONF2
//   writeReg(0xA4, 0x00);  // GOFFSET U
//   writeReg(0xA5, 0x00);  // GOFFSET D
//   writeReg(0xA7, 0x00);  // GOFFSET L
//   writeReg(0xA9, 0x00);  // GOFFSET R

//   writeReg(0xAB, 0x01);  // GPULSE
//   writeReg(0xA6, 0xC9);  // GCONF3

//   Serial.println("Gesture engine forced ON");
// }
// void loop() {
//   byte status = readReg(0xAF);

//   if (status & 0x01) {

//     int gesture = decodeGesture();

//     switch (gesture) {
//       case 1:
//         Serial.println("UP");
//         digitalWrite(A3, HIGH);  // Lamp1
//         break;

//       case 2:
//         Serial.println("DOWN");
//         digitalWrite(A2, HIGH);  // Lamp2
//         break;

//       case 3:
//         Serial.println("LEFT");
//         digitalWrite(A1, HIGH);  // Fan
//         break;

//       case 4:
//         Serial.println("RIGHT");
//         digitalWrite(A0, HIGH);  // Socket
//         break;

//       default:
//         Serial.println("UNKNOWN");
//         break;
//     }

//     // turn off others
//     delay(200);
//     digitalWrite(A0, LOW);
//     digitalWrite(A1, LOW);
//     digitalWrite(A2, LOW);
//     digitalWrite(A3, LOW);
//   }
// }

// int decodeGesture() {
//   byte fifoLevel = readReg(0xAE);

//   if (fifoLevel < 4) return 0;

//   int up = 0, down = 0, left = 0, right = 0;

//   for (int i = 0; i < fifoLevel; i++) {
//     Wire.beginTransmission(APDS);
//     Wire.write(0xFC);  // GFIFO_U
//     Wire.endTransmission();

//     Wire.requestFrom(APDS, 4);

//     if (Wire.available() == 4) {
//       up += Wire.read();
//       down += Wire.read();
//       left += Wire.read();
//       right += Wire.read();
//     }
//   }

//   int ud_diff = up - down;
//   int lr_diff = left - right;

//   if (abs(ud_diff) > abs(lr_diff)) {
//     if (ud_diff > 30) return 1;   // UP
//     if (ud_diff < -30) return 2;  // DOWN
//   } else {
//     if (lr_diff > 30) return 3;   // LEFT
//     if (lr_diff < -30) return 4;  // RIGHT
//   }

//   return 0;
// }