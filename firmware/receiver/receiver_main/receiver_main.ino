#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <LiquidCrystal_I2C.h>

// ---------------- NRF Configuration ----------------
RF24 radio(9, 10);  // CE, CSN
const byte address[10] = "ADDRESS01";

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ---------------- PINS ----------------
// Relays
#define RELAY_SOCKET A0
#define RELAY_FAN A1
#define RELAY_LAMP2 A2
#define RELAY_LAMP1 A3

// Buzzer
#define BUZZER 2

// ---------------- STATES ----------------
bool sSocket = false;
bool sFan = false;
bool sLamp2 = false;
bool sLamp1 = false;

// ---------------- HELPERS ----------------
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(100);  // Quick beep
  digitalWrite(BUZZER, LOW);
}

void applyOutputs() {
  // Active-LOW Relays: true = LOW (ON), false = HIGH (OFF)
  digitalWrite(RELAY_SOCKET, !sSocket);
  digitalWrite(RELAY_FAN, !sFan);
  digitalWrite(RELAY_LAMP2, !sLamp2);
  digitalWrite(RELAY_LAMP1, !sLamp1);
}

void updateLCD(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMART CONTROL");
  lcd.setCursor(0, 1);
  lcd.print(line1);
  lcd.setCursor(0, 2);
  lcd.print(line2);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(RELAY_SOCKET, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_LAMP2, OUTPUT);
  pinMode(RELAY_LAMP1, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Set initial states (All OFF)
  applyOutputs();

  // LCD init
  lcd.init();
  lcd.backlight();

  // --- SPLASH SCREEN: NAME & MATRIC NUMBER ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Name: GBENGA-IDOWU");
  lcd.setCursor(0, 1);
  lcd.print("Afeoluwa");
  lcd.setCursor(0, 2);
  lcd.print("Matric:");
  lcd.setCursor(0, 3);
  lcd.print("21ck029291");
  
  // 6 Second Delay to allow reading
  delay(6000); 

  // NRF init (Reverted to the proven working configuration)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INITIALIZING...");
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  updateLCD("READY", "WAITING SIGNAL");
  Serial.println("=== RECEIVER READY ===");

  // Startup beep
  digitalWrite(BUZZER, HIGH);
  delay(150);
  digitalWrite(BUZZER, LOW);
}

// ---------------- LOOP ----------------
void loop() {
  // Back to the working IF statement
  if (radio.available()) {
    
    // Create a 10-character array initialized to zeros
    char payload[10] = {0}; 
    
    radio.read(&payload, sizeof(payload));

    // Convert to Arduino String for easy comparisons
    String cmdStr(payload);
    cmdStr.trim(); // Remove any accidental spaces or hidden characters

    Serial.print("RX Command: ");
    Serial.println(cmdStr);

    const char* line1 = "";
    const char* line2 = "";
    bool validCommand = true;

    // Process explicit commands
    if (cmdStr == "U ON") {
      sLamp1 = true; line1 = "LAMP 1"; line2 = "ON";
    } else if (cmdStr == "U OFF") {
      sLamp1 = false; line1 = "LAMP 1"; line2 = "OFF";
    } else if (cmdStr == "D ON") {
      sLamp2 = true; line1 = "LAMP 2"; line2 = "ON";
    } else if (cmdStr == "D OFF") {
      sLamp2 = false; line1 = "LAMP 2"; line2 = "OFF";
    } else if (cmdStr == "L ON") {
      sFan = true; line1 = "FAN"; line2 = "ON";
    } else if (cmdStr == "L OFF") {
      sFan = false; line1 = "FAN"; line2 = "OFF";
    } else if (cmdStr == "R ON") {
      sSocket = true; line1 = "SOCKET"; line2 = "ON";
    } else if (cmdStr == "R OFF") {
      sSocket = false; line1 = "SOCKET"; line2 = "OFF";
    } else if (cmdStr == "ALL ON") {
      sLamp1 = sLamp2 = sFan = sSocket = true; line1 = "ALL DEVICES"; line2 = "ON";
    } else if (cmdStr == "ALL OFF") {
      sLamp1 = sLamp2 = sFan = sSocket = false; line1 = "ALL DEVICES"; line2 = "OFF";
    } else {
      validCommand = false; // Ignore unknown commands
    }

    if (validCommand) {
      applyOutputs();
      delay(50); // Tiny pause to let the relay power stabilize before beeping
      updateLCD(line1, line2);
      beep();
    }
  }
}

// #include <Wire.h>
// #include <SPI.h>
// #include <RF24.h>
// #include <nRF24L01.h>
// #include <LiquidCrystal_I2C.h>

// // ---------------- NRF Configuration ----------------
// RF24 radio(9, 10);  // CE, CSN
// const byte address[10] = "ADDRESS01";

// // ---------------- LCD ----------------
// LiquidCrystal_I2C lcd(0x27, 20, 4);

// // ---------------- PINS ----------------
// // Relays
// #define RELAY_SOCKET A0
// #define RELAY_FAN A1
// #define RELAY_LAMP2 A2
// #define RELAY_LAMP1 A3

// // Buzzer
// #define BUZZER 2

// // ---------------- STATES ----------------
// bool sSocket = false;
// bool sFan = false;
// bool sLamp2 = false;
// bool sLamp1 = false;

// // ---------------- HELPERS ----------------
// void beep() {
//   digitalWrite(BUZZER, HIGH);
//   delay(100);  // Quick beep
//   digitalWrite(BUZZER, LOW);
// }

// void applyOutputs() {
//   // Active-LOW Relays: true = LOW (ON), false = HIGH (OFF)
//   digitalWrite(RELAY_SOCKET, !sSocket);
//   digitalWrite(RELAY_FAN, !sFan);
//   digitalWrite(RELAY_LAMP2, !sLamp2);
//   digitalWrite(RELAY_LAMP1, !sLamp1);
// }

// void updateLCD(const char* line1, const char* line2) {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("SMART CONTROL");
//   lcd.setCursor(0, 1);
//   lcd.print(line1);
//   lcd.setCursor(0, 2);
//   lcd.print(line2);
// }

// // ---------------- SETUP ----------------
// void setup() {
//   Serial.begin(115200);

//   pinMode(RELAY_SOCKET, OUTPUT);
//   pinMode(RELAY_FAN, OUTPUT);
//   pinMode(RELAY_LAMP2, OUTPUT);
//   pinMode(RELAY_LAMP1, OUTPUT);
//   pinMode(BUZZER, OUTPUT);

//   // Set initial states (All OFF)
//   applyOutputs();

//   // LCD init
//   lcd.init();
//   lcd.backlight();
//   updateLCD("INITIALIZING", "PLEASE WAIT");

//   // NRF init
//   radio.begin();
//   radio.openReadingPipe(0, address);
//   radio.setPALevel(RF24_PA_MIN);
//   radio.startListening();

//   updateLCD("READY", "WAITING SIGNAL");
//   Serial.println("=== RECEIVER READY ===");

//   // Startup beep
//   digitalWrite(BUZZER, HIGH);
//   delay(150);
//   digitalWrite(BUZZER, LOW);
// }

// // ---------------- LOOP ----------------
// void loop() {
//   if (radio.available()) {

//     // Create a 10-character array to hold the incoming string (e.g., "U ON")
//     char payload[10] = { 0 };

//     radio.read(&payload, sizeof(payload));

//     // Convert to Arduino String for easy comparisons
//     String cmdStr(payload);
//     cmdStr.trim();  // Remove any accidental spaces or hidden characters

//     Serial.print("RX Command: ");
//     Serial.println(cmdStr);

//     const char* line1 = "";
//     const char* line2 = "";
//     bool validCommand = true;

//     // Process explicit explicit commands
//     if (cmdStr == "U ON") {
//       sLamp1 = true;
//       line1 = "LAMP 1";
//       line2 = "ON";
//     } else if (cmdStr == "U OFF") {
//       sLamp1 = false;
//       line1 = "LAMP 1";
//       line2 = "OFF";
//     } else if (cmdStr == "D ON") {
//       sLamp2 = true;
//       line1 = "LAMP 2";
//       line2 = "ON";
//     } else if (cmdStr == "D OFF") {
//       sLamp2 = false;
//       line1 = "LAMP 2";
//       line2 = "OFF";
//     } else if (cmdStr == "L ON") {
//       sFan = true;
//       line1 = "FAN";
//       line2 = "ON";
//     } else if (cmdStr == "L OFF") {
//       sFan = false;
//       line1 = "FAN";
//       line2 = "OFF";
//     } else if (cmdStr == "R ON") {
//       sSocket = true;
//       line1 = "SOCKET";
//       line2 = "ON";
//     } else if (cmdStr == "R OFF") {
//       sSocket = false;
//       line1 = "SOCKET";
//       line2 = "OFF";
//     } else if (cmdStr == "ALL ON") {
//       sLamp1 = sLamp2 = sFan = sSocket = true;
//       line1 = "ALL DEVICES";
//       line2 = "ON";
//     } else if (cmdStr == "ALL OFF") {
//       sLamp1 = sLamp2 = sFan = sSocket = false;
//       line1 = "ALL DEVICES";
//       line2 = "OFF";
//     } else {
//       validCommand = false;  // Ignore unknown commands
//     }

//     if (validCommand) {
//       applyOutputs();
//       updateLCD(line1, line2);
//       beep();
//     }
//   }
// }