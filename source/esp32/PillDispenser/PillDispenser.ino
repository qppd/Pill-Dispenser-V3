// PillDispenser.ino - Main file for Pill Dispenser V3
// ESP32-based pill dispenser with modular components
// Created: 2025-11-01

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include "PINS_CONFIG.h"
#include "FirebaseConfig.h"
#include "ArduinoServoController.h"  // Servo control via Arduino Uno serial communication
#include "LCDDisplay.h"  // Reactivated LCDDisplay
#include "TimeManager.h"
#include "FirebaseManager.h"
#include "ScheduleManager.h"
#include "SIM800L.h"
#include "VoltageSensor.h"
#include "WiFiManager.h"
#include "UserConfig.h"

// ===== DEVELOPMENT MODE CONFIGURATION =====
#define DEVELOPMENT_MODE true  // Set to false for production
#define PRODUCTION_MODE false  // Will implement later

// ===== COMPONENT INSTANCES =====
ArduinoServoController servoController(PIN_UNO_RX, PIN_UNO_TX);  // Serial communication with Arduino Uno
LCDDisplay lcd;  // Reactivate LCDDisplay instance
TimeManager timeManager;
FirebaseManager firebase;
ScheduleManager scheduleManager;
SIM800L sim800(PIN_SIM800_RX, PIN_SIM800_TX, PIN_SIM800_RST, Serial2);
VoltageSensor voltageSensor(PIN_VOLTAGE_SENSOR);

// ===== SYSTEM VARIABLES =====
bool systemInitialized = false;
String currentMode = "DEVELOPMENT";
unsigned long lastHeartbeat = 0;
unsigned long lastLcdUpdate = 0;  // Reactivate LCD time update
unsigned long lastTimeDebug = 0;   // For debug time output
int pillCount = 0;

// WiFi credentials (for development - move to secure storage in production)
const String WIFI_SSID = "jayron";
const String WIFI_PASSWORD = "12345678";

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

// Notification helpers
void playDispenseBuzzer();
void playReminderBuzzer();
void sendSMSNotification(String message);
void handleReminderNotification(int dispenserId, String pillSize, String medication, String patient);

void setup() {
  // Initialize buzzer first to prevent noise (BEFORE Serial.begin)
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  
  Serial.begin(115200);
  delay(2000);
  
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
  
  // Sound buzzer to indicate system ready (DEACTIVATED)
  // digitalWrite(PIN_BUZZER, HIGH);
  // delay(500);
  // digitalWrite(PIN_BUZZER, LOW);
}

void loop() {
  if (DEVELOPMENT_MODE) {
    // Firebase.ready() should be called repeatedly to handle authentication tasks and stream processing
    Firebase.ready(); // No logging unless error occurs
    
    // Update time manager (auto-sync every 6 hours)
    timeManager.update();
    
    // Update schedule manager (checks alarms)
    scheduleManager.update();
    
    // Update SIM800L for background network reconnection
    sim800.update();
    
    // Check for realtime dispense commands from web app
    checkDispenseCommands();
    
    // Sync schedules from Firebase periodically
    if (firebase.shouldSyncSchedules()) {
      firebase.syncSchedulesFromFirebase();
    }
    
    // Send Firebase heartbeat every 1 minute to indicate device is online
    firebase.sendHeartbeat(&voltageSensor);
    
    // Update LCD time display continuously (update every second)
    static unsigned long lastLcdUpdate = 0;
    if (millis() - lastLcdUpdate >= 1000) { // Update every 1 second
      String currentTimeString = timeManager.getTimeString();
      lcd.displayTime(currentTimeString);
      lastLcdUpdate = millis();
    }
    
    // Update servo controller to process async messages
    servoController.update();
    
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
        Serial.println("\n========== SERVO CONTROLLER STATUS ==========");
        if (servoController.isConnected()) {
          Serial.println("✅ Arduino Uno connected and responding");
        } else {
          Serial.println("❌ Arduino Uno not responding");
        }
        Serial.println("=============================================");
      } else if (command.startsWith("servo test ")) {
        int servoNum = command.substring(11).toInt();
        if (servoNum >= 0 && servoNum <= 15) {
          Serial.println("Testing servo " + String(servoNum) + "...");
          servoController.testServo(servoNum);
          Serial.println("✅ Servo test complete");
        } else {
          Serial.println("❌ Invalid servo number (0-4)");
        }
      } else if (command == "servo sweep") {
        Serial.println("Testing all servos with sweep...");
        for (int i = 0; i <= 4; i++) {
          Serial.println("Sweeping servo " + String(i) + "...");
          servoController.setServoAngle(i, 0);
          delay(500);
          servoController.setServoAngle(i, 180);
          delay(500);
          servoController.setServoAngle(i, 90);
          delay(500);
        }
        Serial.println("✅ Servo sweep complete");
      } else if (command == "servo reset") {
        Serial.println("Resetting all servos to 90 degrees...");
        servoController.resetAllServos();
        Serial.println("✅ All servos reset");
      } else if (command == "servo release") {
        Serial.println("Moving CH5/CH6 servos to release position...");
        servoController.moveServosToRelease();
        Serial.println("✅ CH5/CH6 moved to release position");
      } else if (command == "servo home") {
        Serial.println("Moving CH5/CH6 servos to home position...");
        servoController.moveServosToHome();
        Serial.println("✅ CH5/CH6 moved to home position");
      } else if (command == "servo stop") {
        Serial.println("Stopping all servos...");
        servoController.stopAllServos();
        Serial.println("✅ All servos stopped");
      } else if (command.startsWith("dispense ")) {
        int containerNum = command.substring(9).toInt();
        if (containerNum >= 1 && containerNum <= 5) {
          Serial.println("\n" + String('=', 60));
          Serial.println("👊 MANUAL DISPENSE TRIGGERED (Serial Command)");
          Serial.println(String('=', 60));
          Serial.println("Container: " + String(containerNum));
          Serial.println("Time: " + timeManager.getTimeString());
          
          // Play buzzer for manual dispense (DEACTIVATED)
          // playDispenseBuzzer();
          
          // Perform dispense
          dispenseFromContainer(containerNum - 1);
          
          // Send SMS notification
          String smsMessage = "[PILL DISPENSER] Manual dispense from Container " + String(containerNum) + 
                             " via serial command at " + timeManager.getTimeString();
          sendSMSNotification(smsMessage);
          
          Serial.println("✅ Manual dispense complete");
          Serial.println(String('=', 60) + "\n");
        } else {
          Serial.println("❌ Invalid container number (1-5)");
        }
      } else if (command.startsWith("calibrate ")) {
        int servoNum = command.substring(10).toInt();
        if (servoNum >= 0 && servoNum <= 15) {
          Serial.println("Calibrating servo " + String(servoNum) + "...");
          servoController.calibrateServo(servoNum);
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
        Serial.println("servo release - Move CH5/CH6 to release position");
        Serial.println("servo home - Move CH5/CH6 to home position");
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
  
  // Debug: Print current time EVERY SECOND to monitor alarm triggering
  static unsigned long lastSecondDebug = 0;
  if (millis() - lastSecondDebug > 1000) { // Every 1 second
    lastSecondDebug = millis();
    Serial.printf("⏰ TimeLib: %02d:%02d:%02d | TimeManager: %s | Alarms: %d | Next: %s\n", 
                  hour(), minute(), second(),
                  timeManager.getTimeString().c_str(),
                  Alarm.count(),
                  scheduleManager.getNextScheduleTime().c_str());
  }
  
  // Detailed debug every minute
  static unsigned long lastTimeDebug = 0;
  if (millis() - lastTimeDebug > 60000) { // Every 60 seconds
    lastTimeDebug = millis();
    Serial.println("\n" + String('=', 70));
    Serial.println("⏰ DETAILED TIME & ALARM STATUS");
    Serial.println(String('=', 70));
    Serial.printf("TimeLib (TimeAlarms): %02d:%02d:%02d\n", hour(), minute(), second());
    Serial.println("TimeManager: " + timeManager.getTimeString());
    Serial.println("Active schedules: " + String(scheduleManager.getActiveScheduleCount()) + 
                   " / " + String(scheduleManager.getScheduleCount()));
    Serial.println("Total alarms registered: " + String(Alarm.count()));
    Serial.println("Next schedule: " + scheduleManager.getNextScheduleTime());
    scheduleManager.printSchedules();
    Serial.println(String('=', 70) + "\n");
  }
}

void initializeDevelopmentMode() {
  Serial.println("\n📋 Initializing components for development...");
  
  // Initialize LCD first for status display
  Serial.print("LCD Display: ");
  if (lcd.begin()) {
    Serial.println("✅ OK");
    lcd.displayMainScreen();
  } else {
    Serial.println("❌ FAILED");
  }
  
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
  
  // Initialize Arduino Servo Controller
  Serial.print("Arduino Servo Controller: ");
  if (servoController.begin()) {
    Serial.println("✅ OK");
    
    // Set all servos from ch0 to ch4 to angle 0 as starting point
    Serial.println("Setting servos ch0-ch4 to angle 0...");
    for (int ch = 0; ch <= 4; ch++) {
      servoController.setServoAngle(ch, 0);
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
  scheduleManager.setReminderCallback(handleReminderNotification);
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
  
  // Play buzzer for dispense event (DEACTIVATED)
  // playDispenseBuzzer();
  
  // Display on LCD
  lcd.displayDispenseInfo(dispenserId + 1, medication);
  
  // Perform dispense
  dispenseFromContainer(dispenserId);
  
  // Update dispenser data in Firebase
  firebase.updateDispenserAfterDispense(dispenserId, &timeManager);
  
  // Send SMS notification to caregivers
  String smsMessage = "[PILL DISPENSER] Medication dispensed from Container " + String(dispenserId + 1) + 
                     " - " + medication + " for " + patient + " at " + timeManager.getTimeString();
  sendSMSNotification(smsMessage);
  
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
  
  Serial.println("\n" + String('=', 60));
  Serial.println("🔄 DISPENSING FROM CONTAINER " + String(dispenserId + 1));
  Serial.println(String('=', 60));
  Serial.printf("Sending DP%d command to Arduino...\n", dispenserId);
  Serial.println(String('=', 60));
  
  // Use the dispensePill method via Arduino servo controller
  // This sends DP0-DP4 command which triggers testServo() on Arduino
  bool success = servoController.dispensePill(dispenserId);
  
  Serial.println(String('=', 60));
  if (success) {
    pillCount++;
    Serial.println("✅ DISPENSE SUCCESSFUL");
    Serial.println("   Total pills dispensed: " + String(pillCount));
    
    // Wait 10 seconds before releasing
    Serial.println("⏳ Waiting 10 seconds before release...");
    delay(10000);
    
    // Move to release position (CH5/CH6)
    Serial.println("🔓 Moving to RELEASE position...");
    if (servoController.moveServosToRelease()) {
      Serial.println("✅ Release position reached");
    } else {
      Serial.println("❌ Release movement failed");
    }
    
    // Wait 10 seconds before returning home
    Serial.println("⏳ Waiting 10 seconds before returning home...");
    delay(10000);
    
    // Move back to home position
    Serial.println("🏠 Moving to HOME position...");
    if (servoController.moveServosToHome()) {
      Serial.println("✅ Home position reached");
    } else {
      Serial.println("❌ Home movement failed");
    }
    
  } else {
    Serial.println("❌ DISPENSE FAILED");
    Serial.println("   Arduino communication error or timeout");
  }
  Serial.println(String('=', 60) + "\n");
}

// Check for realtime dispense commands from web app
void checkDispenseCommands() {
  if (firebase.hasDispenseCommand()) {
    int dispenserId = firebase.getLastDispenseCommand();
    
    if (dispenserId >= 1 && dispenserId <= 5) {
      Serial.println("\n📱 Realtime dispense command received!");
      Serial.println("Container: " + String(dispenserId));
      
      // Play buzzer for manual dispense (DEACTIVATED)
      // playDispenseBuzzer();
      
      // Perform dispense (convert to 0-based index)
      dispenseFromContainer(dispenserId - 1);
      
      // Update dispenser data in Firebase
      firebase.updateDispenserAfterDispense(dispenserId - 1, &timeManager);
      
      // Send SMS notification to caregivers
      String smsMessage = "[PILL DISPENSER] Manual dispense from Container " + String(dispenserId) + 
                         " at " + timeManager.getTimeString();
      sendSMSNotification(smsMessage);
      
      // Log to Firebase
      firebase.sendPillReport(dispenserId, timeManager.getDateTimeString(), 
                             "Manual dispense via web app", 1);
    }
  }
}

// Play professional buzzer sound for dispense event
void playDispenseBuzzer() {
  // Short professional buzz pattern: beep-beep
  digitalWrite(PIN_BUZZER, HIGH);
  delay(150);
  digitalWrite(PIN_BUZZER, LOW);
  delay(100);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(150);
  digitalWrite(PIN_BUZZER, LOW);
  Serial.println("🔊 Dispense buzzer activated");
}

// Play reminder buzzer sound (15 minutes before)
void playReminderBuzzer() {
  // Three short beeps for reminder
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(100);
    digitalWrite(PIN_BUZZER, LOW);
    delay(100);
  }
  Serial.println("🔔 Reminder buzzer activated");
}

// Send SMS to all caregivers
void sendSMSNotification(String message) {
  if (sim800.isNetworkConnected()) {
    Serial.println("📤 Sending SMS notifications...");
    
    // Send to Caregiver 1
    if (sim800.sendSMS(CAREGIVER_1_PHONE, message)) {
      Serial.println("✅ SMS sent to " + CAREGIVER_1_NAME + ": " + CAREGIVER_1_PHONE);
    } else {
      Serial.println("❌ Failed to send SMS to " + CAREGIVER_1_NAME);
    }
    
    delay(2000); // Delay between SMS sends
    
    // Send to Caregiver 2
    if (sim800.sendSMS(CAREGIVER_2_PHONE, message)) {
      Serial.println("✅ SMS sent to " + CAREGIVER_2_NAME + ": " + CAREGIVER_2_PHONE);
    } else {
      Serial.println("❌ Failed to send SMS to " + CAREGIVER_2_NAME);
    }
  } else {
    Serial.println("⚠️ GSM not connected - SMS not sent");
  }
}

// Handle 15-minute reminder notification
void handleReminderNotification(int dispenserId, String pillSize, String medication, String patient) {
  Serial.println("\n" + String('=', 60));
  Serial.println("🔔 15-MINUTE REMINDER");
  Serial.println(String('=', 60));
  Serial.printf("Container: %d\n", dispenserId + 1);
  Serial.println("Medication: " + medication);
  Serial.println("Patient: " + patient);
  Serial.println("Time: " + timeManager.getTimeString());
  Serial.println(String('=', 60));
  
  // Play reminder buzzer (DEACTIVATED)
  // playReminderBuzzer();
  
  // Send SMS reminder to caregivers
  String smsMessage = "[PILL DISPENSER REMINDER] Upcoming medication in 15 minutes - Container " + 
                     String(dispenserId + 1) + ": " + medication + " for " + patient;
  sendSMSNotification(smsMessage);
  
  Serial.println("✅ Reminder notification completed\n");
}
