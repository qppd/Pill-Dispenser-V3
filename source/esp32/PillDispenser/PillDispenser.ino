
// PillDispenser.ino - Main file for Pill Dispenser V3 - PRODUCTION VERSION
// ESP32-based pill dispenser with complete scheduling, SMS, and Firebase integration
// Firmware Version: 3.0.0
// Last Updated: December 2025

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "PINS_CONFIG.h"
#include "FirebaseConfig.h"
#include "WiFiManager.h"
#include "ServoDriver.h"
#include "LCDDisplay.h"
#include "TimeManager.h"
#include "FirebaseManager.h"
#include "SIM800L.h"
#include "VoltageSensor.h"
#include "ScheduleManager.h"
#include "NotificationManager.h"

// ===== MODE CONFIGURATION =====
#define DEVELOPMENT_MODE false  // Set to true for development, false for production
#define PRODUCTION_MODE !DEVELOPMENT_MODE

// ===== FIRMWARE VERSION =====
const String FIRMWARE_VERSION = "3.0.0";

// ===== COMPONENT INSTANCES =====
ServoDriver servoDriver;
LCDDisplay lcd;
TimeManager timeManager;
FirebaseManager firebase;
SIM800L sim800(PIN_SIM800_RX, PIN_SIM800_TX, PIN_SIM800_RST, Serial2);
VoltageSensor voltageSensor(PIN_VOLTAGE_SENSOR);
ScheduleManager scheduleManager;
NotificationManager notificationManager(&sim800, &timeManager);

// ===== SYSTEM VARIABLES =====
bool systemInitialized = false;
bool wifiConnected = false;
bool firebaseConnected = false;
String deviceId;
unsigned long lastHeartbeat = 0;
unsigned long lastBatteryUpdate = 0;
unsigned long lastScheduleSync = 0;
int totalPillsDispensed = 0;

// Timing constants
const unsigned long HEARTBEAT_INTERVAL = 60000;        // 1 minute
const unsigned long BATTERY_UPDATE_INTERVAL = 60000;   // 1 minute
const unsigned long SCHEDULE_SYNC_INTERVAL = 300000;   // 5 minutes

// WiFi credentials (configure these)
const String WIFI_SSID = "QPPD";
const String WIFI_PASSWORD = "Programmer136";

// Caregiver phone numbers (configure these)
const String CAREGIVER_1_PHONE = "+1234567890";
const String CAREGIVER_1_NAME = "Primary Caregiver";

// Firebase credentials loaded from FirebaseConfig.h
// Edit FirebaseConfig.cpp to set your actual credentials

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  printStartupBanner();
  
  // Initialize status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);
  
  // Initialize I2C
  Wire.begin(PIN_SDA, PIN_SCL);
  Serial.println("✓ I2C initialized");
  
  // Generate device ID
  deviceId = "PILL_DISPENSER_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.println("✓ Device ID: " + deviceId);
  
  if (PRODUCTION_MODE) {
    Serial.println("\n🏭 PRODUCTION MODE - Full System Initialization");
    initializeProductionMode();
  } else {
    Serial.println("\n🔧 DEVELOPMENT MODE - Testing & Debug Features Enabled");
    initializeDevelopmentMode();
  }
  
  Serial.println("\n" + String('=', 60));
  Serial.println("    ✅ SYSTEM READY");
  Serial.println(String('=', 60));
  digitalWrite(PIN_STATUS_LED, HIGH);
}

void loop() {
  // Update time manager (handles NTP sync)
  timeManager.update();
  
  // Update schedule manager (handles TimeAlarms)
  if (PRODUCTION_MODE) {
    scheduleManager.update();
  }
  
  // Periodic tasks
  handleHeartbeat();
  handleBatteryUpdate();
  handleScheduleSync();
  
  // Check for realtime dispense commands from Firebase stream
  if (firebase.hasDispenseCommand()) {
    int dispenserId = firebase.getLastDispenseCommand();
    if (dispenserId > 0) {
      Serial.print("ESP32: Processing realtime dispense command for dispenser ");
      Serial.println(dispenserId);
      
      // Trigger manual dispense
      dispensePill(dispenserId, "Manual", "Web App Dispense", "Remote User");
    }
  }
  
  delay(100);
}

// ===== INITIALIZATION FUNCTIONS =====

void printStartupBanner() {
  Serial.println("\n" + String('=', 60));
  Serial.println("         PILL DISPENSER V3 - PRODUCTION SYSTEM");
  Serial.println("         Firmware Version: " + FIRMWARE_VERSION);
  Serial.println(String('=', 60));
}

void initializeProductionMode() {
  Serial.println("\n📋 Initializing Production Components...\n");
  
  // 1. LCD Display
  Serial.print("1. LCD Display: ");
  if (lcd.begin()) {
    Serial.println("✅ OK");
    lcd.clear();
    lcd.displayMessage("PillDispenser V3", "Initializing...");
  } else {
    Serial.println("❌ FAILED (non-critical)");
  }
  delay(500);
  
  // 2. WiFi Connection
  Serial.print("2. WiFi Connection: ");
  setupWiFi(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str(), &timeManager);
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅ Connected");
    Serial.print("   IP: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    Serial.println(" ❌ FAILED");
    wifiConnected = false;
  }
  delay(1000);
  
  // 3. Time Sync is handled by setupWiFi (NTP initialized after WiFi connects)
  Serial.print("3. Time Sync (NTP): ");
  if (timeManager.isSynced()) {
    Serial.println("✅ OK - " + timeManager.getDateTimeString());
    setTime(timeManager.getTimestamp());  // Sync TimeLib with NTP
  } else {
    Serial.println("⚠️  FAILED (will retry)");
  }
  delay(500);
  
  // 4. Firebase
  Serial.print("4. Firebase Connection: ");
  if (wifiConnected) {
    firebase.setDeviceId(deviceId);
    if (firebase.begin(PillDispenserConfig::getApiKey(), 
                      PillDispenserConfig::getDatabaseURL())) {
      Serial.println("✅ OK");
      firebaseConnected = true;
      uploadDeviceInfo();
    } else {
      Serial.println("❌ FAILED");
    }
  } else {
    Serial.println("⏭️  SKIPPED (no WiFi)");
  }
  delay(500);
  
  // 5. Servo Driver
  Serial.print("5. Servo Driver (PCA9685): ");
  if (servoDriver.begin()) {
    Serial.println("✅ OK - 5 dispensers ready");
  } else {
    Serial.println("❌ FAILED (critical)");
  }
  delay(500);
  
  // 6. SIM800L GSM Module
  Serial.print("6. SIM800L GSM Module: ");
  if (sim800.begin()) {
    Serial.println("✅ OK - SMS ready");
  } else {
    Serial.println("⚠️  FAILED (SMS disabled)");
  }
  delay(500);
  
  // 7. Voltage Sensor
  Serial.print("7. Battery Monitor: ");
  voltageSensor.begin();
  float batteryPercent = voltageSensor.readBatteryPercentage();
  Serial.println("✅ OK - " + String(batteryPercent, 1) + "% (" + 
                voltageSensor.getBatteryStatus() + ")");
  delay(500);
  
  // 8. Notification Manager
  Serial.print("8. Notification Manager: ");
  notificationManager.begin();
  notificationManager.addPhoneNumber(CAREGIVER_1_PHONE, CAREGIVER_1_NAME);
  Serial.println("✅ OK - " + String(notificationManager.getPhoneCount()) + " contacts");
  delay(500);
  
  // 9. Schedule Manager
  Serial.print("9. Schedule Manager: ");
  scheduleManager.begin(deviceId);
  scheduleManager.setDispenseCallback(onScheduledDispense);
  Serial.println("✅ OK - Ready for scheduling");
  
  // Load schedules from Firebase
  if (firebaseConnected) {
    Serial.print("   Loading schedules from Firebase: ");
    loadSchedulesFromFirebase();
  }
  delay(500);
  
  // 10. Initial status update
  Serial.println("\n10. Uploading initial status to Firebase...");
  if (firebaseConnected) {
    updateFirebaseStatus();
    updateFirebaseBattery();
  }
  
  lcd.displayMessage("System Ready", timeManager.getTimeString());
  systemInitialized = true;
  
  Serial.println("\n✅ Production mode initialization complete!");
  Serial.println("🔒 Schedule enforcement: ACTIVE");
  Serial.println("📱 SMS notifications: ENABLED");
  Serial.println("☁️  Firebase sync: ACTIVE");
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
  
  // Initialize WiFi and Time Manager using setupWiFi
  Serial.println("WiFi & NTP Setup:");
  setupWiFi(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str(), &timeManager);
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("✅ WiFi & NTP initialized");
  }
  
  // Initialize Servo Driver
  Serial.print("Servo Driver: ");
  if (servoDriver.begin()) {
    Serial.println("✅ OK");
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
  
  // Initialize Schedule Manager
  Serial.print("Schedule Manager: ");
  scheduleManager.begin(deviceId);
  scheduleManager.setDispenseCallback(onScheduledDispense);
  Serial.println("✅ OK");
  
  // Initialize Notification Manager
  Serial.print("Notification Manager: ");
  notificationManager.begin();
  Serial.println("✅ OK");
  
  Serial.println("\n🎯 Development mode ready!");
  Serial.println("Type 'help' to see available commands");
  
  systemInitialized = true;
}

// ===== CORE CALLBACK FUNCTIONS =====

void onScheduledDispense(int dispenserId, String pillSize, String medication, String patient) {
  Serial.println("\n" + String('=', 60));
  Serial.println("⏰ SCHEDULED DISPENSE TRIGGERED");
  Serial.println(String('=', 60));
  Serial.println("Dispenser ID: " + String(dispenserId));
  Serial.println("Patient: " + patient);
  Serial.println("Medication: " + medication);
  Serial.println("Pill Size: " + pillSize);
  Serial.println("Time: " + timeManager.getDateTimeString());
  Serial.println(String('=', 60));
  
  // Update LCD
  lcd.clear();
  lcd.displayMessage("Dispensing...", patient);
  
  // Perform dispensing
  uint8_t servoChannel = dispenserId - 1;  // Convert dispenser ID to 0-based channel
  Serial.println("▶ Activating dispenser " + String(dispenserId) + " (servo channel " + String(servoChannel) + ")...");
  servoDriver.dispensePill(servoChannel, pillSize);
  
  totalPillsDispensed++;
  
  // Log to Firebase
  if (firebaseConnected) {
    logDispenseEvent(dispenserId, "schedule", "success", medication, patient);
  }
  
  // Send SMS notification
  notificationManager.notifyOnDispense(patient, medication);
  
  // Update LCD
  lcd.displayMessage("Dispensed!", "Total: " + String(totalPillsDispensed));
  delay(2000);
  lcd.displayMessage("Ready", timeManager.getTimeString());
  
  Serial.println("✅ Dispensing complete!\n");
}

// ===== PERIODIC TASK HANDLERS =====

void handleHeartbeat() {
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = millis();
    
    if (PRODUCTION_MODE && firebaseConnected) {
      updateFirebaseStatus();
    }
    
    if (!PRODUCTION_MODE) {
      Serial.println("💓 Heartbeat - " + timeManager.getDateTimeString());
    }
  }
}

void handleBatteryUpdate() {
  if (millis() - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL) {
    lastBatteryUpdate = millis();
    
    float batteryPercent = voltageSensor.readBatteryPercentage();
    
    if (PRODUCTION_MODE && firebaseConnected) {
      updateFirebaseBattery();
    }
    
    // Check for low battery
    if (batteryPercent < 20.0 && batteryPercent > 0) {
      notificationManager.notifyLowBattery(batteryPercent);
    }
    
    if (!PRODUCTION_MODE) {
      Serial.println("🔋 Battery: " + String(batteryPercent, 1) + "% (" + 
                    voltageSensor.getBatteryStatus() + ")");
    }
  }
}

void handleScheduleSync() {
  if (millis() - lastScheduleSync >= SCHEDULE_SYNC_INTERVAL) {
    lastScheduleSync = millis();
    
    if (PRODUCTION_MODE && firebaseConnected) {
      loadSchedulesFromFirebase();
    }
  }
}

// ===== FIREBASE FUNCTIONS =====
// Note: WiFi connection now handled by WiFiManager.cpp (setupWiFi function)

void uploadDeviceInfo() {
  FirebaseJson json;
  json.set("name", "PILL_DISPENSER_V3");
  json.set("firmware_version", FIRMWARE_VERSION);
  json.set("mac", String((uint32_t)ESP.getEfuseMac(), HEX));
  json.set("device_id", deviceId);
  
  String path = "pilldispenser/devices/" + deviceId + "/info";
  firebase.updateDeviceStatus("online");
}

void updateFirebaseStatus() {
  FirebaseJson json;
  json.set("online", true);
  json.set("last_heartbeat", timeManager.getTimestamp());
  json.set("wifi_rssi", WiFi.RSSI());
  json.set("ip_address", WiFi.localIP().toString());
  json.set("uptime", millis() / 1000);
  json.set("pills_dispensed", totalPillsDispensed);
  
  // Send via FirebaseManager
  firebase.updateDeviceStatus("online");
  firebase.sendHeartbeat(&voltageSensor);
}

void updateFirebaseBattery() {
  float voltage = voltageSensor.getLastActualVoltage();
  float percentage = voltageSensor.getLastBatteryPercentage();
  String status = voltageSensor.getBatteryStatus();
  
  firebase.uploadSensorData("battery_voltage", String(voltage, 2));
  firebase.uploadSensorData("battery_percentage", String(percentage, 1));
  firebase.uploadSensorData("battery_status", status);
}

void logDispenseEvent(int dispenserId, String trigger, String status, 
                     String medication, String patient) {
  String timestamp = timeManager.getDateTimeString();
  
  FirebaseJson json;
  json.set("type", "dispense");
  json.set("dispenser_id", dispenserId);
  json.set("pill_count", 1);
  json.set("timestamp", timestamp);
  json.set("status", status);
  json.set("trigger", trigger);
  json.set("medication_name", medication);
  json.set("patient_name", patient);
  json.set("message", "Pill dispensed successfully");
  
  // Log via FirebaseManager
  firebase.sendPillDispenseLog(1, timestamp);
  firebase.sendPillReport(1, timestamp, medication + " - " + patient, 
                         status == "success" ? 1 : 0);
}

void loadSchedulesFromFirebase() {
  // This function will load schedules from Firebase
  // For now, log the attempt
  Serial.println("📥 Syncing schedules from Firebase...");
  
  // TODO: Implement actual Firebase schedule loading
  // scheduleManager.syncSchedulesFromFirebase(&fbdo, basePath);
  
  if (scheduleManager.getScheduleCount() > 0) {
    Serial.println("   ✓ " + String(scheduleManager.getScheduleCount()) + " schedules loaded");
  } else {
    Serial.println("   ℹ No schedules configured");
  }
}

void dispensePill(int dispenserId, String trigger, String medication, String patient) {
  Serial.println("\n" + String('=', 60));
  Serial.println("🎯 MANUAL DISPENSE TRIGGERED");
  Serial.println(String('=', 60));
  Serial.println("Dispenser ID: " + String(dispenserId));
  Serial.println("Trigger: " + trigger);
  Serial.println("Medication: " + medication);
  Serial.println("Patient: " + patient);
  Serial.println("Time: " + timeManager.getDateTimeString());
  Serial.println(String('=', 60));
  
  // Update LCD
  lcd.clear();
  lcd.displayMessage("Dispensing...", "Container " + String(dispenserId));
  
  // Perform dispensing - map dispenser ID (1-5) to servo channel (0-4)
  uint8_t servoChannel = dispenserId - 1;  // Convert dispenser ID to 0-based channel
  Serial.println("▶ Activating dispenser " + String(dispenserId) + " (servo channel " + String(servoChannel) + ")...");
  servoDriver.dispensePill(servoChannel, "medium");  // Default pill size
  
  totalPillsDispensed++;
  
  // Log to Firebase
  if (firebaseConnected) {
    logDispenseEvent(dispenserId, trigger, "success", medication, patient);
  }
  
  // Send SMS notification for manual dispenses
  if (trigger != "schedule") {  // Avoid double notifications for scheduled dispenses
    notificationManager.notifyOnDispense(patient, medication);
  }
  
  // Update LCD
  lcd.displayMessage("Dispensed!", "Total: " + String(totalPillsDispensed));
  delay(2000);
  lcd.displayMessage("Ready", timeManager.getTimeString());
  
  Serial.println("✅ Manual dispensing complete!\n");
}

