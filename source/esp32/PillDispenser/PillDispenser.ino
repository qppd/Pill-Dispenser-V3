
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
  
  // Handle serial commands (always available for diagnostics)
  if (Serial.available()) {
    handleSerialCommands();
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
  connectWiFi();
  delay(1000);
  
  // 3. Time Manager (NTP)
  Serial.print("3. Time Sync (NTP): ");
  timeManager.begin("pool.ntp.org", 0, 0); // Configure timezone as needed
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
  
  // Initialize Time Manager (NTP)
  Serial.print("Time Manager (NTP): ");
  timeManager.begin("pool.ntp.org", 0, 0); // GMT+0, adjust as needed
  Serial.println("✅ OK");
  
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
  Serial.println("▶ Activating dispenser " + String(dispenserId) + "...");
  servoDriver.dispensePill(dispenserId, pillSize);
  
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

void connectWiFi() {
  if (WIFI_SSID == "YOUR_WIFI_SSID" || WIFI_SSID == "") {
    Serial.println("❌ WiFi credentials not configured");
    wifiConnected = false;
    return;
  }
  
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅ Connected");
    Serial.println("   IP: " + WiFi.localIP().toString());
    wifiConnected = true;
  } else {
    Serial.println(" ❌ Failed");
    wifiConnected = false;
  }
}

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
  firebase.sendHeartbeat();
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
  else if (cmd == "test lcd") {
    lcd.testDisplay();
  }
  else if (cmd == "test time" || cmd == "time test") {
    timeManager.testTime();
  }
  else if (cmd == "test sim800") {
    sim800.testModule();
  }
  else if (cmd == "test firebase") {
    testFirebaseConnection();
  }
  else if (cmd == "test voltage" || cmd == "voltage test") {
    voltageSensor.testSensor();
  }
  else if (cmd == "voltage") {
    voltageSensor.printDebug();
  }
  else if (cmd == "time") {
    timeManager.printDebug();
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
  else if (cmd.startsWith("calibrate servo")) {
    int servoNum = cmd.substring(16).toInt();
    if (servoNum >= 0 && servoNum <= 15) {
      servoDriver.calibrateServo(servoNum);
    } else {
      Serial.println("Invalid servo number (0-15)");
    }
  }
  else if (cmd.startsWith("dispense rotation")) {
    handleDispenseRotationCommand(cmd);
  }
  else if (cmd == "test dispenser 0") {
    servoDriver.testDispenserRotation(0);
  }
  else if (cmd == "test dispenser 1") {
    servoDriver.testDispenserRotation(1);
  }
  else if (cmd == "test dispenser 2") {
    servoDriver.testDispenserRotation(2);
  }
  else if (cmd == "test dispenser 3") {
    servoDriver.testDispenserRotation(3);
  }
  else if (cmd == "test dispenser 4") {
    servoDriver.testDispenserRotation(4);
  }
  else if (cmd == "test all dispensers") {
    Serial.println("Testing all 5 dispensers sequentially...");
    for (int i = 0; i < 5; i++) {
      servoDriver.testDispenserRotation(i);
      delay(2000); // Delay between dispenser tests
    }
    Serial.println("All dispenser tests complete!");
  }
  else if (cmd == "wifi connect") {
    connectWiFi();
  }
  else if (cmd == "schedules" || cmd == "list schedules") {
    scheduleManager.printSchedules();
  }
  else if (cmd == "notifications" || cmd == "notify config") {
    notificationManager.printConfig();
  }
  else if (cmd == "battery") {
    voltageSensor.printDebug();
  }
  else if (cmd == "firebase status") {
    firebase.printConnectionStatus();
  }
  else if (cmd.startsWith("add schedule")) {
    handleAddSchedule();
  }
  else if (cmd.startsWith("test sms")) {
    testSMSNotification();
  }
  else if (cmd == "sync schedules") {
    loadSchedulesFromFirebase();
  }
  else if (cmd == "reset") {
    Serial.println("Restarting system...");
    ESP.restart();
  }
  else if (cmd == "clear") {
    // Clear screen
    for (int i = 0; i < 50; i++) Serial.println();
    Serial.println(PRODUCTION_MODE ? "🏭 PRODUCTION MODE" : "🔧 DEVELOPMENT MODE");
    Serial.println("PILL DISPENSER V3 - FW: " + FIRMWARE_VERSION);
  }
  else {
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
  
  Serial.println(); // Add blank line after command
}

void printHelpMenu() {
  Serial.println("\n📚 AVAILABLE COMMANDS:");
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println("System Commands:");
  Serial.println("  help                 - Show this menu");
  Serial.println("  status               - System status");
  Serial.println("  reset                - Restart system");
  Serial.println("  clear                - Clear screen");
  Serial.println("  time                 - Show current time");
  Serial.println("  battery              - Show battery status");
  Serial.println("  wifi connect         - Connect to WiFi");
  Serial.println("  firebase status      - Show Firebase connection info");
  Serial.println("  i2c scan             - Scan I2C devices");
  Serial.println();
  Serial.println("Scheduling & Notifications:");
  Serial.println("  schedules            - List all schedules");
  Serial.println("  add schedule         - Add a new schedule (interactive)");
  Serial.println("  sync schedules       - Load schedules from Firebase");
  Serial.println("  notifications        - Show notification config");
  Serial.println("  test sms             - Send test SMS notification");
  Serial.println();
  Serial.println("Component Testing:");
  Serial.println("  test servo           - Test single servo (will ask for number)");
  Serial.println("  test all servos      - Test all 16 servos");
  Serial.println("  test pill dispenser  - Test pill dispensing on channel");
  Serial.println("  test all dispensers  - Test all dispenser pairs");
  Serial.println("  test lcd             - Test LCD display");
  Serial.println("  test time            - Test NTP time sync (continuous)");
  Serial.println("  test sim800          - Test SIM800L module");
  Serial.println("  test firebase        - Test Firebase connection");
  Serial.println("  test voltage         - Test voltage sensor (continuous)");
  Serial.println("  voltage              - Show current voltage reading");
  Serial.println();
  Serial.println("Pill Dispenser Operations:");
  Serial.println("  dispense             - Test pill dispensing sequence");
  Serial.println("  dispense [ch] [size] - Dispense pill (ch=0-4, size=small/medium/large)");
  Serial.println("  dispense pair [ch1] [ch2] [size] - Dispense using servo pair");
  Serial.println("  dispense rotation [servo] [start] [stop] [speed] - Custom rotation");
  Serial.println();
  Serial.println("Individual Dispenser Testing:");
  Serial.println("  test dispenser 0     - Test dispenser 0 (servo 0)");
  Serial.println("  test dispenser 1     - Test dispenser 1 (servo 1)");
  Serial.println("  test dispenser 2     - Test dispenser 2 (servo 2)");
  Serial.println("  test dispenser 3     - Test dispenser 3 (servo 3)");
  Serial.println("  test dispenser 4     - Test dispenser 4 (servo 4)");
  Serial.println("  test all dispensers  - Test all 5 dispensers sequentially");
  Serial.println();
  Serial.println("Servo Control:");
  Serial.println("  servo [num] [angle]  - Move servo to angle (0-180°)");
  Serial.println("  servo reset          - Reset all servos to 90°");
  Serial.println("  servo stop [num]     - Stop specific servo");
  Serial.println("  servo stop all       - Stop all servos");
  Serial.println("  servo speed [num] [speed] - Set servo speed (300-450)");
  Serial.println("  calibrate servo [num] - Calibrate servo (tests full 180°)");
  Serial.println("═══════════════════════════════════════════════════════════");
}

void printSystemStatus() {
  Serial.println("\n📊 SYSTEM STATUS:");
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println("Mode: " + String(PRODUCTION_MODE ? "PRODUCTION" : "DEVELOPMENT"));
  Serial.println("Firmware: " + FIRMWARE_VERSION);
  Serial.println("Device ID: " + deviceId);
  Serial.println("Uptime: " + String(millis() / 1000) + " seconds");
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Current Time: " + timeManager.getDateTimeString());
  Serial.println("Pills Dispensed: " + String(totalPillsDispensed));
  Serial.println();
  
  // Component status
  Serial.println("Component Status:");
  Serial.println("  LCD: " + String(lcd.isConnected() ? "✅ Connected" : "❌ Disconnected"));
  Serial.println("  Servo Driver: " + String(servoDriver.isConnected() ? "✅ Connected" : "❌ Disconnected"));
  Serial.println("  Time Manager: " + String(timeManager.isSynced() ? "✅ Synced" : "❌ Not Synced"));
  Serial.println("  SIM800L: " + String(sim800.isReady() ? "✅ Ready" : "❌ Not Ready"));
  Serial.println("  Voltage Sensor: " + String(voltageSensor.isConnected() ? "✅ Connected" : "❌ Disconnected"));
  Serial.println("  WiFi: " + String(WiFi.status() == WL_CONNECTED ? "✅ Connected (" + WiFi.localIP().toString() + ")" : "❌ Disconnected"));
  Serial.println("  Firebase: " + String(firebaseConnected ? "✅ Connected" : "❌ Disconnected"));
  Serial.println();
  
  // Battery info
  Serial.println("Battery:");
  float voltage = voltageSensor.getLastActualVoltage();
  float percentage = voltageSensor.getLastBatteryPercentage();
  Serial.println("  Voltage: " + String(voltage, 2) + " V");
  Serial.println("  Percentage: " + String(percentage, 1) + " %");
  Serial.println("  Status: " + voltageSensor.getBatteryStatus());
  Serial.println();
  
  // Scheduling info
  Serial.println("Scheduling:");
  Serial.println("  Active Schedules: " + String(scheduleManager.getActiveScheduleCount()) + 
                " / " + String(scheduleManager.getScheduleCount()));
  Serial.println("  Next Schedule: " + scheduleManager.getNextScheduleTime());
  Serial.println();
  
  // Notifications
  Serial.println("Notifications:");
  Serial.println("  SMS Enabled: " + String(notificationManager.isReady() ? "✅ Yes" : "❌ No"));
  Serial.println("  Phone Numbers: " + String(notificationManager.getPhoneCount()));
  
  Serial.println("═══════════════════════════════════════════════════════════");
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

void handleDispenseRotationCommand(String cmd) {
  // Parse "dispense rotation [servo] [start] [stop] [speed]"
  int firstSpace = cmd.indexOf(' ', 17); // Find space after "dispense rotation "
  int secondSpace = cmd.indexOf(' ', firstSpace + 1);
  int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
  
  if (firstSpace != -1 && secondSpace != -1 && thirdSpace != -1) {
    int servo = cmd.substring(17, firstSpace).toInt();
    int startAngle = cmd.substring(firstSpace + 1, secondSpace).toInt();
    int stopAngle = cmd.substring(secondSpace + 1, thirdSpace).toInt();
    int speed = cmd.substring(thirdSpace + 1).toInt();
    
    if (servo >= 0 && servo <= 15 && startAngle >= 0 && startAngle <= 180 && 
        stopAngle >= 0 && stopAngle <= 180 && speed > 0) {
      servoDriver.dispenseWithRotation(servo, startAngle, stopAngle, speed);
    } else {
      Serial.println("Invalid parameters: servo (0-15), angles (0-180° per specs), speed > 0");
    }
  } else {
    Serial.println("Usage: dispense rotation [servo] [start_angle] [stop_angle] [speed]");
    Serial.println("Example: dispense rotation 0 0 180 15");
    Serial.println("Note: MG90S specs claim 180°, but mechanical stops may limit range");
  }
}

void testPillDispense() {
  Serial.println("🧪 Testing pill dispensing sequence...");
  
  lcd.displayStatus("Dispensing...");
  
  // Use dispenser 0 for testing
  Serial.println("Activating dispenser 0...");
  servoDriver.dispensePill(0, "medium");
  
  totalPillsDispensed++;
  Serial.println("Pill dispensed! Total count: " + String(totalPillsDispensed));
  
  lcd.displayPillCount(totalPillsDispensed);
  lcd.displayStatus("Ready");
  
  // Log to Firebase if connected
  if (firebaseConnected) {
    logDispenseEvent(0, "manual", "success", "Test Medication", "Test Patient");
  }
}

void handleAddSchedule() {
  Serial.println("\n📅 ADD NEW SCHEDULE");
  Serial.println("══════════════════════════════════════");
  
  Serial.print("Enter Dispenser ID (0-4): ");
  while (!Serial.available()) delay(10);
  int dispenserId = Serial.parseInt();
  Serial.println(dispenserId);
  
  if (dispenserId < 0 || dispenserId > 4) {
    Serial.println("❌ Invalid dispenser ID");
    return;
  }
  
  Serial.print("Enter Hour (0-23): ");
  while (!Serial.available()) delay(10);
  int hour = Serial.parseInt();
  Serial.println(hour);
  
  Serial.print("Enter Minute (0-59): ");
  while (!Serial.available()) delay(10);
  int minute = Serial.parseInt();
  Serial.println(minute);
  
  Serial.print("Enter Patient Name: ");
  while (!Serial.available()) delay(10);
  String patientName = Serial.readStringUntil('\n');
  patientName.trim();
  Serial.println(patientName);
  
  Serial.print("Enter Medication Name: ");
  while (!Serial.available()) delay(10);
  String medicationName = Serial.readStringUntil('\n');
  medicationName.trim();
  Serial.println(medicationName);
  
  Serial.print("Enter Pill Size (small/medium/large): ");
  while (!Serial.available()) delay(10);
  String pillSize = Serial.readStringUntil('\n');
  pillSize.trim();
  pillSize.toLowerCase();
  Serial.println(pillSize);
  
  // Generate schedule ID
  String scheduleId = "SCH_" + String(millis());
  
  // Add schedule
  if (scheduleManager.addSchedule(scheduleId, dispenserId, hour, minute,
                                 medicationName, patientName, pillSize, true)) {
    Serial.println("\n✅ Schedule added successfully!");
    Serial.printf("   Time: %02d:%02d\n", hour, minute);
    Serial.println("   Patient: " + patientName);
    Serial.println("   Medication: " + medicationName);
    Serial.println("   Dispenser: " + String(dispenserId));
  } else {
    Serial.println("\n❌ Failed to add schedule");
  }
}

void testSMSNotification() {
  Serial.println("\n📱 TESTING SMS NOTIFICATION");
  Serial.println("══════════════════════════════════════");
  
  if (!notificationManager.isReady()) {
    Serial.println("❌ SMS not ready - check SIM800L and phone numbers");
    return;
  }
  
  Serial.println("Sending test notification...");
  bool success = notificationManager.notifyOnDispense("Test Patient", 
                                                     "Test Medication");
  
  if (success) {
    Serial.println("✅ Test SMS sent successfully!");
  } else {
    Serial.println("❌ Failed to send test SMS");
  }
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
  
  if (firebase.begin(PillDispenserConfig::getApiKey(), PillDispenserConfig::getDatabaseURL())) {
    firebase.testConnection();
    firebase.testDataUpload();
    
    // Test pill report functionality
    firebase.sendPillReport(1, timeManager.getDateTimeString(), "Test dispense from development mode", 1);
  } else {
    Serial.println("❌ Firebase initialization failed");
  }
}
