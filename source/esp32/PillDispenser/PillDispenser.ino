
// PillDispenser.ino - Main file for Pill Dispenser V3
// ESP32-based pill dispenser with modular components
// Created: 2025-11-01

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "ServoDriver.h"
#include "IRSensor.h"
#include "LCDDisplay.h"
#include "RTClock.h"
#include "FirebaseManager.h"
#include "SIM800L.h"

// ===== DEVELOPMENT MODE CONFIGURATION =====
#define DEVELOPMENT_MODE true  // Set to false for production
#define PRODUCTION_MODE false  // Will implement later

// ===== PIN DEFINITIONS =====
// I2C pins (default for ESP32)
#define SDA_PIN 21
#define SCL_PIN 22

// IR Sensor pins
#define IR_SENSOR1_PIN 34
#define IR_SENSOR2_PIN 35
#define IR_SENSOR3_PIN 32

// RTC DS1302 pins
#define RTC_CLK_PIN 18
#define RTC_DAT_PIN 19
#define RTC_RST_PIN 5

// SIM800L pins
#define SIM800_RX_PIN 16
#define SIM800_TX_PIN 17
#define SIM800_RST_PIN 4

// Status LED
#define STATUS_LED_PIN 2

// ===== COMPONENT INSTANCES =====
ServoDriver servoDriver;
IRSensor irSensors(IR_SENSOR1_PIN, IR_SENSOR2_PIN, IR_SENSOR3_PIN);
LCDDisplay lcd;
RTClock rtclock(RTC_CLK_PIN, RTC_DAT_PIN, RTC_RST_PIN);
FirebaseManager firebase;
SIM800L sim800(SIM800_RX_PIN, SIM800_TX_PIN, SIM800_RST_PIN);

// ===== SYSTEM VARIABLES =====
bool systemInitialized = false;
String currentMode = "DEVELOPMENT";
unsigned long lastSensorCheck = 0;
unsigned long lastHeartbeat = 0;
int pillCount = 0;

// WiFi credentials (for development - move to secure storage in production)
const String WIFI_SSID = "YOUR_WIFI_SSID";
const String WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Firebase credentials (for development - move to secure storage in production)
const String FIREBASE_API_KEY = "YOUR_API_KEY";
const String FIREBASE_URL = "YOUR_DATABASE_URL";
const String FIREBASE_EMAIL = "YOUR_EMAIL";
const String FIREBASE_PASSWORD = "YOUR_PASSWORD";

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n" + String('=', 50));
  Serial.println("    PILL DISPENSER V3 - STARTING UP");
  Serial.println(String('=', 50));
  
  // Initialize status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized");
  
  if (DEVELOPMENT_MODE) {
    Serial.println("\n🔧 DEVELOPMENT MODE ENABLED 🔧");
    Serial.println("Serial commands available - type 'help' for list");
    initializeDevelopmentMode();
  } else {
    Serial.println("Production mode (not implemented yet)");
  }
  
  Serial.println("\n" + String('=', 50));
  Serial.println("    SYSTEM READY");
  Serial.println(String('=', 50));
  digitalWrite(STATUS_LED_PIN, HIGH);
}

void loop() {
  if (DEVELOPMENT_MODE) {
    handleSerialCommands();
    
    // Basic sensor monitoring in development mode
    if (millis() - lastSensorCheck > 500) {
      irSensors.readAllSensors();
      lastSensorCheck = millis();
    }
    
    // Heartbeat every 30 seconds in development
    if (millis() - lastHeartbeat > 30000) {
      Serial.println("💓 System heartbeat - " + rtclock.getTimeString());
      lastHeartbeat = millis();
    }
  }
  
  delay(100);
}

void initializeDevelopmentMode() {
  Serial.println("\n📋 Initializing components for development...");
  
  // Initialize LCD first for status display
  Serial.print("LCD Display: ");
  if (lcd.begin()) {
    Serial.println("✅ OK");
    lcd.displayTestMenu();
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize RTC
  Serial.print("RTC Clock: ");
  if (rtclock.begin()) {
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize Servo Driver
  Serial.print("Servo Driver: ");
  if (servoDriver.begin()) {
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize IR Sensors
  Serial.print("IR Sensors: ");
  irSensors.begin();
  Serial.println("✅ OK");
  
  // Initialize SIM800L
  Serial.print("SIM800L Module: ");
  if (sim800.begin()) {
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ FAILED");
  }
  
  Serial.println("\n🎯 Development mode ready!");
  Serial.println("Type 'help' to see available commands");
  
  systemInitialized = true;
}

void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    
    processSerialCommand(command);
  }
}

void processSerialCommand(String cmd) {
  Serial.println("\n> " + cmd);
  
  if (cmd == "help") {
    printHelpMenu();
  }
  else if (cmd == "status") {
    printSystemStatus();
  }
  else if (cmd == "i2c scan") {
    servoDriver.scanI2CDevices();
  }
  else if (cmd == "test servo") {
    Serial.println("Enter servo number (0-15):");
    while (!Serial.available()) delay(10);
    int servoNum = Serial.parseInt();
    servoDriver.testServo(servoNum);
  }
  else if (cmd == "test all servos") {
    servoDriver.testAllServos();
  }
  else if (cmd == "test pill dispenser") {
    Serial.println("Enter channel number (0-15):");
    while (!Serial.available()) delay(10);
    int channel = Serial.parseInt();
    servoDriver.testPillDispenser(channel);
  }
  else if (cmd == "test all dispensers") {
    servoDriver.testAllDispenserPairs();
  }
  else if (cmd == "test ir") {
    irSensors.testAllSensors();
  }
  else if (cmd.startsWith("test ir")) {
    int sensorNum = cmd.substring(8).toInt();
    if (sensorNum >= 1 && sensorNum <= 3) {
      irSensors.testSensor(sensorNum);
    } else {
      Serial.println("Invalid sensor number. Use 1, 2, or 3");
    }
  }
  else if (cmd == "test lcd") {
    lcd.testDisplay();
  }
  else if (cmd == "test rtc") {
    rtclock.testRTC();
  }
  else if (cmd == "test sim800") {
    sim800.testModule();
  }
  else if (cmd == "test firebase") {
    testFirebaseConnection();
  }
  else if (cmd == "time") {
    Serial.println("Current time: " + rtclock.getDateTimeString());
  }
  else if (cmd == "sensors") {
    irSensors.printSensorStates();
  }
  else if (cmd == "dispense") {
    testPillDispense();
  }
  else if (cmd.startsWith("dispense pair")) {
    handleDispensePairCommand(cmd);
  }
  else if (cmd.startsWith("dispense ")) {
    handleDispenseCommand(cmd);
  }
  else if (cmd.startsWith("servo")) {
    handleServoCommand(cmd);
  }
  else if (cmd == "wifi connect") {
    connectWiFi();
  }
  else if (cmd == "reset") {
    Serial.println("Restarting system...");
    ESP.restart();
  }
  else if (cmd == "clear") {
    // Clear screen
    for (int i = 0; i < 50; i++) Serial.println();
    Serial.println("🔧 DEVELOPMENT MODE - PILL DISPENSER V3");
  }
  else {
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
  
  Serial.println(); // Add blank line after command
}

void printHelpMenu() {
  Serial.println("\n📚 AVAILABLE COMMANDS:");
  Serial.println("─────────────────────────────────────");
  Serial.println("System Commands:");
  Serial.println("  help                 - Show this menu");
  Serial.println("  status              - System status");
  Serial.println("  reset               - Restart system");
  Serial.println("  clear               - Clear screen");
  Serial.println("  time                - Show current time");
  Serial.println("  wifi connect        - Connect to WiFi");
  Serial.println("  i2c scan            - Scan I2C devices");
  Serial.println();
  Serial.println("Component Testing:");
  Serial.println("  test servo          - Test single servo (will ask for number)");
  Serial.println("  test all servos     - Test all 16 servos");
  Serial.println("  test pill dispenser - Test pill dispensing on channel");
  Serial.println("  test all dispensers - Test all dispenser pairs");
  Serial.println("  test ir             - Test all IR sensors");
  Serial.println("  test ir [1-3]       - Test specific IR sensor");
  Serial.println("  test lcd            - Test LCD display");
  Serial.println("  test rtc            - Test RTC clock");
  Serial.println("  test sim800         - Test SIM800L module");
  Serial.println("  test firebase       - Test Firebase connection");
  Serial.println();
  Serial.println("Pill Dispenser Operations:");
  Serial.println("  dispense            - Test pill dispensing sequence");
  Serial.println("  dispense [ch] [size] - Dispense pill (ch=0-15, size=small/medium/large)");
  Serial.println("  dispense pair [ch1] [ch2] [size] - Dispense using servo pair");
  Serial.println("  sensors             - Show sensor states");
  Serial.println();
  Serial.println("Servo Control:");
  Serial.println("  servo [num] [angle] - Move servo to angle (0-180)");
  Serial.println("  servo reset         - Reset all servos to 90°");
  Serial.println("  servo stop [num]    - Stop specific servo");
  Serial.println("  servo stop all      - Stop all servos");
  Serial.println("  servo speed [num] [speed] - Set servo speed (300-450)");
  Serial.println("─────────────────────────────────────");
}

void printSystemStatus() {
  Serial.println("\n📊 SYSTEM STATUS:");
  Serial.println("─────────────────────────────────────");
  Serial.println("Mode: " + currentMode);
  Serial.println("Uptime: " + String(millis() / 1000) + " seconds");
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Current Time: " + rtclock.getDateTimeString());
  Serial.println("Pills Dispensed: " + String(pillCount));
  Serial.println();
  
  // Component status
  Serial.println("Component Status:");
  Serial.println("  LCD: " + String(lcd.isConnected() ? "✅ Connected" : "❌ Disconnected"));
  Serial.println("  Servo Driver: " + String(servoDriver.isConnected() ? "✅ Connected" : "❌ Disconnected"));
  Serial.println("  RTC: " + String(rtclock.isValidTime() ? "✅ Valid Time" : "❌ Invalid Time"));
  Serial.println("  SIM800L: " + String(sim800.isReady() ? "✅ Ready" : "❌ Not Ready"));
  Serial.println("  WiFi: " + String(WiFi.status() == WL_CONNECTED ? "✅ Connected" : "❌ Disconnected"));
  
  // Sensor readings
  Serial.println();
  Serial.println("Sensor Readings:");
  Serial.print("  IR Sensor 1: "); Serial.println(irSensors.isBlocked(1) ? "🔴 BLOCKED" : "🟢 CLEAR");
  Serial.print("  IR Sensor 2: "); Serial.println(irSensors.isBlocked(2) ? "🔴 BLOCKED" : "🟢 CLEAR");
  Serial.print("  IR Sensor 3: "); Serial.println(irSensors.isBlocked(3) ? "🔴 BLOCKED" : "🟢 CLEAR");
  Serial.println("─────────────────────────────────────");
}

void handleServoCommand(String cmd) {
  // Parse servo commands like "servo 0 90", "servo reset", "servo stop 5", "servo speed 2 400"
  if (cmd == "servo reset") {
    servoDriver.resetAllServos();
    return;
  }
  else if (cmd == "servo stop all") {
    servoDriver.stopAllServos();
    return;
  }
  else if (cmd.startsWith("servo stop ")) {
    int servoNum = cmd.substring(11).toInt();
    if (servoNum >= 0 && servoNum <= 15) {
      servoDriver.stopServo(servoNum);
    } else {
      Serial.println("Invalid servo number (0-15)");
    }
    return;
  }
  else if (cmd.startsWith("servo speed ")) {
    // Parse "servo speed [num] [speed]"
    int firstSpace = cmd.indexOf(' ', 12); // Find space after "servo speed "
    if (firstSpace != -1) {
      int servoNum = cmd.substring(12, firstSpace).toInt();
      int speed = cmd.substring(firstSpace + 1).toInt();
      
      if (servoNum >= 0 && servoNum <= 15 && speed >= 300 && speed <= 450) {
        servoDriver.setServoSpeed(servoNum, speed);
      } else {
        Serial.println("Invalid servo number (0-15) or speed (300-450)");
      }
    } else {
      Serial.println("Usage: servo speed [0-15] [300-450]");
    }
    return;
  }
  
  // Parse "servo [num] [angle]"
  int firstSpace = cmd.indexOf(' ', 6); // Find space after "servo "
  if (firstSpace != -1) {
    int servoNum = cmd.substring(6, firstSpace).toInt();
    int angle = cmd.substring(firstSpace + 1).toInt();
    
    if (servoNum >= 0 && servoNum <= 15 && angle >= 0 && angle <= 180) {
      servoDriver.setServoAngle(servoNum, angle);
    } else {
      Serial.println("Invalid servo number (0-15) or angle (0-180)");
    }
  } else {
    Serial.println("Servo commands:");
    Serial.println("  servo [0-15] [0-180]  - Move servo to angle");
    Serial.println("  servo reset           - Reset all servos to 90°");
    Serial.println("  servo stop [0-15]     - Stop specific servo");
    Serial.println("  servo stop all        - Stop all servos");
    Serial.println("  servo speed [0-15] [300-450] - Set servo speed");
  }
}

void handleDispenseCommand(String cmd) {
  // Parse "dispense [channel] [size]"
  int firstSpace = cmd.indexOf(' ', 9); // Find space after "dispense "
  if (firstSpace != -1) {
    int channel = cmd.substring(9, firstSpace).toInt();
    String size = cmd.substring(firstSpace + 1);
    size.trim();
    
    if (channel >= 0 && channel <= 15) {
      servoDriver.dispensePill(channel, size);
    } else {
      Serial.println("Invalid channel number (0-15)");
    }
  } else {
    Serial.println("Usage: dispense [0-15] [small/medium/large]");
  }
}

void handleDispensePairCommand(String cmd) {
  // Parse "dispense pair [ch1] [ch2] [size]"
  int firstSpace = cmd.indexOf(' ', 14); // Find space after "dispense pair "
  int secondSpace = cmd.indexOf(' ', firstSpace + 1);
  
  if (firstSpace != -1 && secondSpace != -1) {
    int channel1 = cmd.substring(14, firstSpace).toInt();
    int channel2 = cmd.substring(firstSpace + 1, secondSpace).toInt();
    String size = cmd.substring(secondSpace + 1);
    size.trim();
    
    if (channel1 >= 0 && channel1 <= 15 && channel2 >= 0 && channel2 <= 15) {
      servoDriver.dispensePillPair(channel1, channel2, size);
    } else {
      Serial.println("Invalid channel numbers (0-15)");
    }
  } else {
    Serial.println("Usage: dispense pair [0-15] [0-15] [small/medium/large]");
  }
}

void testPillDispense() {
  Serial.println("🧪 Testing pill dispensing sequence...");
  
  lcd.displayStatus("Dispensing...");
  
  // Example dispensing sequence using servo 0
  Serial.println("Opening dispenser...");
  servoDriver.setServoAngle(0, 180);
  delay(1000);
  
  Serial.println("Closing dispenser...");
  servoDriver.setServoAngle(0, 90);
  delay(500);
  
  pillCount++;
  Serial.println("Pill dispensed! Total count: " + String(pillCount));
  
  lcd.displayPillCount(pillCount);
  lcd.displayStatus("Ready");
}

void connectWiFi() {
  if (WIFI_SSID == "YOUR_WIFI_SSID") {
    Serial.println("❌ Please configure WiFi credentials in the code");
    return;
  }
  
  Serial.println("🌐 Connecting to WiFi...");
  firebase.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
}

void testFirebaseConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected. Use 'wifi connect' first.");
    return;
  }
  
  Serial.println("🔥 Testing Firebase connection...");
  
  if (firebase.begin(FIREBASE_API_KEY, FIREBASE_URL)) {
    firebase.testConnection();
    firebase.testDataUpload();
    
    // Test pill report functionality
    firebase.sendPillReport(1, rtclock.getDateTimeString(), "Test dispense from development mode", 1);
  } else {
    Serial.println("❌ Firebase initialization failed");
  }
}
