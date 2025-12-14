#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "VoltageSensor.h"
#include "FirebaseConfig.h"
#include "ScheduleManager.h"

class FirebaseManager {
private:
  FirebaseData fbdo;
  FirebaseData scheduleStream;
  FirebaseAuth auth;
  FirebaseConfig config;
  
  bool isConnected;
  bool isAuthenticated;
  String deviceId;
  String userId;
  unsigned long lastHeartbeat;
  unsigned long lastScheduleSync;
  bool dispenseCommandReceived;
  int lastDispenseCommand;
  ScheduleManager* scheduleManager;
  
  static const unsigned long HEARTBEAT_INTERVAL = 60000;
  static const unsigned long SCHEDULE_SYNC_INTERVAL = 10000;
  String deviceParentPath;
  
  static FirebaseManager* instance;
  
  static void scheduleStreamCallback(FirebaseStream data) {
    if (!instance) return;
    Serial.println("🔥 Schedule changed!");
    instance->syncSchedulesFromFirebase();
  }
  
  static void scheduleStreamTimeoutCallback(bool timeout) {
    if (timeout) Serial.println("Schedule stream timeout");
  }
  
  void processCommand(String command) {
    command.trim();
    command.toUpperCase();
    if (command.startsWith("DISPENSE:")) {
      int colonIndex = command.indexOf(':');
      if (colonIndex > 0) {
        int dispenserId = command.substring(colonIndex + 1).toInt();
        if (dispenserId >= 1 && dispenserId <= 5) {
          lastDispenseCommand = dispenserId;
          dispenseCommandReceived = true;
          Serial.println("Dispense command: " + String(dispenserId));
        }
      }
    }
  }
  
public:
  FirebaseManager() : isConnected(false), isAuthenticated(false), lastHeartbeat(0),
                       lastScheduleSync(0), dispenseCommandReceived(false), 
                       lastDispenseCommand(0), scheduleManager(nullptr) {
    deviceId = "PILL_DISPENSER_" + String(ESP.getEfuseMac(), HEX);
    deviceParentPath = "pilldispenser/device/" + deviceId;
    instance = this;
  }
  
  bool begin(String apiKey, String databaseURL) {
    Serial.println("FirebaseManager: Initializing...");
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi not connected");
      return false;
    }
    isConnected = true;
    
    config.api_key = apiKey;
    config.database_url = databaseURL;
    config.service_account.data.client_email = PillDispenserConfig::getClientEmail();
    config.service_account.data.project_id = PillDispenserConfig::getProjectId();
    config.service_account.data.private_key = PillDispenserConfig::getPrivateKey();
    
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(8192, 1024);
    fbdo.setResponseSize(2048);
    scheduleStream.setBSSLBufferSize(8192, 1024);
    scheduleStream.setResponseSize(2048);
    
    config.timeout.serverResponse = 10 * 1000;
    config.timeout.socketConnection = 10 * 1000;
    config.timeout.sslHandshake = 30 * 1000;
    
    Firebase.begin(&config, &auth);
    scheduleStream.keepAlive(5, 5, 1);
    
    int wait = 0;
    while (!Firebase.ready() && wait < 30) {
      Serial.print(".");
      delay(1000);
      wait++;
    }
    Serial.println();
    
    if (Firebase.ready()) {
      Serial.println("✅ Firebase ready");
      isAuthenticated = true;
      if (!userId.isEmpty()) beginScheduleStream();
      sendHeartbeat();
      return true;
    }
    Serial.println("❌ Firebase failed");
    return false;
  }
  
  bool isFirebaseReady() { return isConnected && isAuthenticated && Firebase.ready(); }
  String getDeviceId() { return deviceId; }
  void setScheduleManager(ScheduleManager* manager) { scheduleManager = manager; }
  void setUserId(String uid) { 
    userId = uid; 
    if (isAuthenticated) beginScheduleStream();
  }
  
  bool beginScheduleStream() {
    String schedulePath = deviceParentPath + "/schedules";
    Serial.println("Starting schedule stream: " + schedulePath);
    if (!Firebase.RTDB.beginStream(&scheduleStream, schedulePath)) {
      Serial.println("❌ Stream failed: " + scheduleStream.errorReason());
      return false;
    }
    Firebase.RTDB.setStreamCallback(&scheduleStream, scheduleStreamCallback, scheduleStreamTimeoutCallback);
    Serial.println("✅ Schedule stream ready");
    return true;
  }
  
  bool sendHeartbeat(VoltageSensor* voltageSensor = nullptr) {
    if (millis() - lastHeartbeat < HEARTBEAT_INTERVAL) return true;
    lastHeartbeat = millis();
    
    if (!isFirebaseReady()) return false;
    
    String path = deviceParentPath + "/heartbeat";
    FirebaseJson json;
    json.set("timestamp", String(millis()));
    json.set("uptime", String(millis()));
    json.set("wifi_strength", WiFi.RSSI());
    json.set("free_heap", ESP.getFreeHeap());
    json.set("device_status", "online");
    
    if (voltageSensor) {
      json.set("battery_voltage", String(voltageSensor->readActualVoltage()));
      json.set("battery_percentage", String(voltageSensor->readBatteryPercentage()));
    }
    
    if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
      Serial.println("💓 Heartbeat sent");
      return true;
    }
    return false;
  }
  
  bool sendPillReport(int pillCount, String datetime, String description, int status) {
    if (!isFirebaseReady()) return false;
    
    FirebaseJson json;
    json.set("pill_count", pillCount);
    json.set("datetime", datetime);
    json.set("description", description);
    json.set("status", status);
    json.set("device_id", deviceId);
    
    if (Firebase.RTDB.pushJSON(&fbdo, "/pilldispenser/reports", &json)) {
      Serial.println("✅ Report sent");
      return true;
    }
    return false;
  }
  
  bool hasDispenseCommand() { return dispenseCommandReceived; }
  
  int getLastDispenseCommand() {
    if (dispenseCommandReceived) {
      dispenseCommandReceived = false;
      return lastDispenseCommand;
    }
    return 0;
  }
  
  bool shouldSyncSchedules() { return (millis() - lastScheduleSync > SCHEDULE_SYNC_INTERVAL); }
  
  bool syncSchedulesFromFirebase() {
    if (!isFirebaseReady() || !scheduleManager) {
      Serial.println("Cannot sync schedules");
      return false;
    }
    
    String schedulePath = deviceParentPath + "/schedules";
    Serial.println("Syncing schedules: " + schedulePath);
    
    if (Firebase.RTDB.getJSON(&fbdo, schedulePath)) {
      FirebaseJson* json = fbdo.to<FirebaseJson*>();
      scheduleManager->clearAllSchedules();
      
      size_t len = json->iteratorBegin();
      Serial.println("Found " + String(len) + " schedules");
      String key, value = "";
      int type = 0;
      int addedCount = 0;
      
      for (size_t i = 0; i < len; i++) {
        json->iteratorGet(i, type, key, value);
        FirebaseJson scheduleJson;
        scheduleJson.setJsonData(value);
        FirebaseJsonData data;
        
        int dispenserId = 0, hour = 0, minute = 0;
        bool enabled = true;
        String medicationName = "", patientName = "", pillSize = "medium", timeStr = "";
        
        if (scheduleJson.get(data, "dispenserId") || scheduleJson.get(data, "dispenser_id")) {
          dispenserId = data.to<int>();
        }
        if (scheduleJson.get(data, "time")) {
          timeStr = data.to<String>();
          int colonIndex = timeStr.indexOf(':');
          if (colonIndex > 0) {
            hour = timeStr.substring(0, colonIndex).toInt();
            minute = timeStr.substring(colonIndex + 1).toInt();
          }
        } else {
          if (scheduleJson.get(data, "hour")) hour = data.to<int>();
          if (scheduleJson.get(data, "minute")) minute = data.to<int>();
        }
        if (scheduleJson.get(data, "enabled")) enabled = data.to<bool>();
        if (scheduleJson.get(data, "medicationName") || scheduleJson.get(data, "medication_name")) {
          medicationName = data.to<String>();
        }
        if (scheduleJson.get(data, "patientName") || scheduleJson.get(data, "patient_name")) {
          patientName = data.to<String>();
        }
        if (scheduleJson.get(data, "pillSize") || scheduleJson.get(data, "pill_size")) {
          pillSize = data.to<String>();
        }
        
        // Skip invalid schedules: empty medication name or 00:00 time
        if (medicationName.length() == 0 || (hour == 0 && minute == 0)) {
          continue;
        }
        
        if (scheduleManager->addSchedule(key, dispenserId, hour, minute, 
                                         medicationName, patientName, pillSize, enabled)) {
          addedCount++;
        }
      }
      
      json->iteratorEnd();
      lastScheduleSync = millis();
      Serial.printf("✅ Schedule sync complete: %d valid schedules loaded (skipped %d empty/invalid)\\n", 
                   addedCount, len - addedCount);
      scheduleManager->printSchedules();
      return true;
    }
    Serial.println("❌ Sync failed: " + fbdo.errorReason());
    return false;
  }
};

FirebaseManager* FirebaseManager::instance = nullptr;

#endif