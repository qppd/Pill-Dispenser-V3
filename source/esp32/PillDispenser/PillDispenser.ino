
// PillDispenser.ino - Main file for Pill Dispenser V3
// ESP32-based pill dispenser with modular components
// Created: 2025-11-01

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include "PINS_CONFIG.h"
#include "FirebaseConfig.h"
#include "ServoDriver.h"
// #include "LCDDisplay.h"  // Temporarily removed to prevent NACK detection
#include "TimeManager.h"
#include "FirebaseManager.h"
#include "ScheduleManager.h"
#include "SIM800L.h"
#include "VoltageSensor.h"
#include "WiFiManager.h"

// ===== DEVELOPMENT MODE CONFIGURATION =====
#define DEVELOPMENT_MODE true  // Set to false for production
#define PRODUCTION_MODE false  // Will implement later

// ===== COMPONENT INSTANCES =====
ServoDriver servoDriver;
// LCDDisplay lcd;  // Temporarily removed to prevent NACK detection
TimeManager timeManager;
FirebaseManager firebase;
ScheduleManager scheduleManager;
SIM800L sim800(PIN_SIM800_RX, PIN_SIM800_TX, PIN_SIM800_RST, Serial2);
VoltageSensor voltageSensor(PIN_VOLTAGE_SENSOR);

// ===== SYSTEM VARIABLES =====
bool systemInitialized = false;
String currentMode = "DEVELOPMENT";
unsigned long lastHeartbeat = 0;
// unsigned long lastLcdUpdate = 0;  // Removed - LCD temporarily disabled
int pillCount = 0;

// WiFi credentials (for development - move to secure storage in production)
const String WIFI_SSID = "jayron";
const String WIFI_PASSWORD = "12345678";
const String USER_ID = "d1SdACjSzbZBNzfhMOFhZixVEX82";  // This should come from Firebase auth in production

// Firebase credentials loaded from FirebaseConfig.h
// Edit FirebaseConfig.cpp to set your actual credentials

// ===== FUNCTION PROTOTYPES =====
// Core system functions
void initializeDevelopmentMode();
void setupWiFi(const char* ssid, const char* password, TimeManager* timeManager);
void testFirebaseConnection();
void handleScheduledDispense(int dispenserId, String pillSize, String medication, String patient);
void dispenseFromContainer(int dispenserId);
void checkDispenseCommands();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  // Initialize buzzer first to prevent noise
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  
  Serial.println("\n" + String('=', 50));
  Serial.println("    PILL DISPENSER V3 - STARTING UP");
  Serial.println(String('=', 50));
  
  // Initialize status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);
  
  // Initialize I2C
  Wire.begin(PIN_SDA, PIN_SCL);
  Serial.println("I2C initialized");

  // Small delay to ensure ESP32 is fully ready after I2C init
  delay(100);

  if (DEVELOPMENT_MODE) {
    Serial.println("\n🔧 DEVELOPMENT MODE ENABLED 🔧");
    initializeDevelopmentMode();
  } else {
    Serial.println("Production mode (not implemented yet)");
  }
  
  Serial.println("\n" + String('=', 50));
  Serial.println("    SYSTEM READY");
  Serial.println(String('=', 50));
  digitalWrite(PIN_STATUS_LED, HIGH);
  
  // Sound buzzer to indicate system ready
  digitalWrite(PIN_BUZZER, HIGH);
  delay(500);
  digitalWrite(PIN_BUZZER, LOW);
}

void loop() {
  if (DEVELOPMENT_MODE) {
    // Firebase.ready() should be called repeatedly to handle authentication tasks and stream processing
    bool firebaseReadyResult = Firebase.ready();
    if (!firebaseReadyResult) {
      // Only log occasionally to avoid spam
      static unsigned long lastFirebaseLog = 0;
      if (millis() - lastFirebaseLog > 10000) { // Every 10 seconds
        Serial.println("Firebase.ready() returned false");
        lastFirebaseLog = millis();
      }
    }
    
    // Update time manager (auto-sync every 6 hours)
    timeManager.update();
    
    // Update schedule manager (checks alarms)
    scheduleManager.update();
    
    // Check for realtime dispense commands from web app
    checkDispenseCommands();
    
    // Sync schedules from Firebase periodically
    if (firebase.shouldSyncSchedules()) {
      firebase.syncSchedulesFromFirebase();
    }
    
    // Send Firebase heartbeat every 1 minute to indicate device is online
    firebase.sendHeartbeat(&voltageSensor);
    
    // Update LCD time display continuously (update every second)
    // Temporarily disabled to prevent NACK detection
    // static unsigned long lastLcdUpdate = 0;
    // static unsigned long lastTimeDebug = 0;
    // if (millis() - lastLcdUpdate >= 1000) { // Update every 1 second
    //   String currentTimeString = timeManager.getTimeString();
    //   lcd.displayTime(currentTimeString);
    //   lastLcdUpdate = millis();

    //   // Debug time every 30 seconds
    //   if (millis() - lastTimeDebug >= 30000) {
    //     Serial.print("Software RTC Time: ");
    //     Serial.println(currentTimeString);
    //     lastTimeDebug = millis();
    //   }
    // }
    
    // Heartbeat every 30 seconds in development
    if (millis() - lastHeartbeat > 30000) {
      Serial.println("💓 System heartbeat - " + timeManager.getTimeString());
      Serial.println("Next schedule: " + scheduleManager.getNextScheduleTime());
      
      // Check servo driver connection
      if (servoDriver.isConnected()) {
        Serial.println("Servo Driver: Connected");
      } else {
        Serial.println("⚠️  WARNING: Servo Driver not responding! Check connections.");
      }
      
      lastHeartbeat = millis();
    }
    
    // Serial command handler for testing
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      command.toLowerCase();
      
      if (command.startsWith("test ")) {
        int scheduleIndex = command.substring(5).toInt();
        Serial.println("Testing schedule trigger for index: " + String(scheduleIndex));
        scheduleManager.testTriggerSchedule(scheduleIndex);
      } else if (command == "schedules") {
        scheduleManager.printSchedules();
      } else if (command == "time") {
        Serial.println("Current NTP time: " + timeManager.getTimeString());
        Serial.printf("TimeAlarms time: %02d:%02d:%02d\n", hour(), minute(), second());
      } else if (command == "servo status") {
        Serial.println("\n========== SERVO DRIVER STATUS ==========");
        if (servoDriver.isConnected()) {
          Serial.println("✅ PCA9685 connected and responding");
        } else {
          Serial.println("❌ PCA9685 not responding");
        }
        Serial.println("=========================================");
      } else if (command.startsWith("servo test ")) {
        int servoNum = command.substring(11).toInt();
        if (servoNum >= 0 && servoNum <= 4) {
          Serial.println("Testing servo " + String(servoNum) + "...");
          servoDriver.testServo(servoNum);
          Serial.println("✅ Servo test complete");
        } else {
          Serial.println("❌ Invalid servo number (0-4)");
        }
      } else if (command == "servo sweep") {
        Serial.println("Testing all servos with sweep...");
        for (int i = 0; i <= 4; i++) {
          Serial.println("Sweeping servo " + String(i) + "...");
          servoDriver.setServoAngle(i, 0);
          delay(500);
          servoDriver.setServoAngle(i, 180);
          delay(500);
          servoDriver.setServoAngle(i, 90);
          delay(500);
        }
        Serial.println("✅ Servo sweep complete");
      } else if (command == "servo reset") {
        Serial.println("Resetting all servos to 90 degrees...");
        servoDriver.resetAllServos();
        Serial.println("✅ All servos reset");
      } else if (command == "servo stop") {
        Serial.println("Stopping all servos...");
        servoDriver.stopAllServos();
        Serial.println("✅ All servos stopped");
      } else if (command.startsWith("dispense ")) {
        int containerNum = command.substring(9).toInt();
        if (containerNum >= 1 && containerNum <= 5) {
          Serial.println("Manually dispensing from container " + String(containerNum) + "...");
          dispenseFromContainer(containerNum - 1);
          Serial.println("✅ Manual dispense complete");
        } else {
          Serial.println("❌ Invalid container number (1-5)");
        }
      } else if (command.startsWith("calibrate ")) {
        int servoNum = command.substring(10).toInt();
        if (servoNum >= 0 && servoNum <= 4) {
          Serial.println("Calibrating servo " + String(servoNum) + "...");
          servoDriver.calibrateServo(servoNum);
          Serial.println("✅ Calibration complete");
        } else {
          Serial.println("❌ Invalid servo number (0-4)");
        }
      } else if (command == "help") {
        Serial.println("\n========== AVAILABLE COMMANDS ==========");
        Serial.println("schedules - List all schedules");
        Serial.println("time - Show current time");
        Serial.println("test <index> - Test schedule trigger");
        Serial.println("servo status - Check servo driver status");
        Serial.println("servo test <0-4> - Test specific servo");
        Serial.println("servo sweep - Sweep all servos");
        Serial.println("servo reset - Reset all servos to 90°");
        Serial.println("servo stop - Stop all servos");
        Serial.println("dispense <1-5> - Manual dispense from container");
        Serial.println("calibrate <0-4> - Calibrate specific servo");
        Serial.println("help - Show this help message");
        Serial.println("=========================================");
      }
    }
  }
  
  // CRITICAL: Use Alarm.delay() instead of delay() to process TimeAlarms
  // This ensures alarm callbacks are triggered at the right time
  Alarm.delay(100);
}

void initializeDevelopmentMode() {
  Serial.println("\n📋 Initializing components for development...");
  
  // Initialize LCD first for status display
  Serial.println("LCD Display: ⏸️  Temporarily disabled to prevent NACK detection");
  // if (lcd.begin()) {
  //   Serial.println("✅ OK");
  //   lcd.displayMainScreen();
  // } else {
  //   Serial.println("❌ FAILED");
  // }
  
  // Setup WiFi for time synchronization
  Serial.print("WiFi Connection: ");
  setupWiFi(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str(), &timeManager);
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ OK");
    
    // Display initial time on LCD
    // delay(1000); // Wait a moment for NTP sync
    // lcd.displayTime(timeManager.getTimeString());
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize Servo Driver
  Serial.print("Servo Driver: ");
  if (servoDriver.begin()) {
    Serial.println("✅ OK");
    
    // Set all servos from ch0 to ch4 to angle 0 as starting point
    Serial.println("Setting servos ch0-ch4 to angle 0...");
    for (int ch = 0; ch <= 4; ch++) {
      servoDriver.setServoAngle(ch, 0);
      delay(100); // Small delay between servo movements
    }
    Serial.println("All servos initialized to 0 degrees");
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize SIM800L
  Serial.print("SIM800L Module: ");
  if (sim800.begin()) {
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize Voltage Sensor
  Serial.print("Voltage Sensor: ");
  voltageSensor.begin();
  Serial.println("✅ OK");
  
  // Initialize Firebase Manager
  Serial.print("Firebase Manager: ");
  if (firebase.begin(PillDispenserConfig::getApiKey(), PillDispenserConfig::getDatabaseURL())) {
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ FAILED");
  }
  
  // Initialize Schedule Manager
  Serial.print("Schedule Manager: ");
  scheduleManager.begin(firebase.getDeviceId());
  scheduleManager.setDispenseCallback(handleScheduledDispense);
  scheduleManager.setTimeManager(&timeManager);
  Serial.println("✅ OK");
  
  // Link Firebase and Schedule Manager
  firebase.setScheduleManager(&scheduleManager);
  firebase.setUserId(USER_ID);
  
  // Wait for Firebase to be ready before syncing schedules
  Serial.println("\n⏳ Waiting for Firebase to be ready...");
  int waitCount = 0;
  while (!firebase.isFirebaseReady() && waitCount < 30) {
    Serial.print(".");
    Firebase.ready(); // Feed watchdog and process Firebase tasks
    yield(); // Feed watchdog timer
    delay(1000);
    waitCount++;
  }
  Serial.println();
  
  if (firebase.isFirebaseReady()) {
    // Initial schedule sync from Firebase
    Serial.println("📅 Loading schedules from Firebase...");
    if (firebase.syncSchedulesFromFirebase()) {
      Serial.println("✅ Schedules loaded successfully");
    } else {
      Serial.println("⚠️ No schedules found or sync failed");
    }
  } else {
    Serial.println("❌ Firebase not ready - skipping schedule sync");
  }
  
  Serial.println("\n🎯 Development mode ready!");
  
  systemInitialized = true;
}


// Callback function for scheduled dispensing
void handleScheduledDispense(int dispenserId, String pillSize, String medication, String patient) {
  Serial.println("\n" + String('=', 60));
  Serial.println("⏰ SCHEDULED DISPENSE TRIGGERED");
  Serial.println(String('=', 60));
  Serial.printf("Container: %d\n", dispenserId + 1);
  Serial.println("Medication: " + medication);
  Serial.println("Patient: " + patient);
  Serial.println("Pill Size: " + pillSize);
  Serial.println("Time: " + timeManager.getTimeString());
  Serial.println(String('=', 60));
  
  // Display on LCD
  // lcd.displayDispenseInfo(dispenserId + 1, medication);  // Temporarily disabled to prevent NACK detection
  
  // Perform dispense
  dispenseFromContainer(dispenserId);
  
  // Update dispenser data in Firebase
  firebase.updateDispenserAfterDispense(dispenserId, &timeManager);
  
  // Send notification via SMS
  String smsMessage = "Medication dispensed from Container " + String(dispenserId + 1) + 
                     " - " + medication + " at " + timeManager.getTimeString();
  sim800.sendSMS("+1234567890", smsMessage);  // Replace with actual phone number
  
  // Log to Firebase
  firebase.sendPillReport(dispenserId + 1, timeManager.getDateTimeString(), 
                         "Scheduled dispense: " + medication, 1);
  
  Serial.println("✅ Scheduled dispense completed\n");
}

// Function to dispense from a specific container
void dispenseFromContainer(int dispenserId) {
  if (dispenserId < 0 || dispenserId > 4) {
    Serial.println("❌ Invalid dispenser ID: " + String(dispenserId));
    return;
  }
  
  Serial.println("🔄 Dispensing from container " + String(dispenserId + 1) + "...");
  
  // Use the dispensePill method from ServoDriver for proper pill dispensing
  // This moves servo to 180 degrees, waits 2 seconds, then returns to 0 degrees
  servoDriver.dispensePill(dispenserId, "medium");
  
  pillCount++;
  Serial.println("✅ Dispense complete. Total pills dispensed: " + String(pillCount));
}

// Check for realtime dispense commands from web app
void checkDispenseCommands() {
  if (firebase.hasDispenseCommand()) {
    int dispenserId = firebase.getLastDispenseCommand();
    
    if (dispenserId >= 1 && dispenserId <= 5) {
      Serial.println("\n📱 Realtime dispense command received!");
      Serial.println("Container: " + String(dispenserId));
      
      // Perform dispense (convert to 0-based index)
      dispenseFromContainer(dispenserId - 1);
      
      // Update dispenser data in Firebase
      firebase.updateDispenserAfterDispense(dispenserId - 1, &timeManager);
      
      // Log to Firebase
      firebase.sendPillReport(dispenserId, timeManager.getDateTimeString(), 
                             "Manual dispense via web app", 1);
    }
  }
}
