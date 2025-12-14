#ifndef FIREBASE_MANAGER_H
#define FIREBASE_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "VoltageSensor.h"

// Forward declaration
class ScheduleManager;

class FirebaseManager {
private:
  FirebaseData fbdo;
  FirebaseData deviceStream;
  FirebaseData scheduleStream;
  FirebaseAuth auth;
  FirebaseConfig config;
  
  bool isConnected;
  bool isAuthenticated;
  bool signupOk;
  String deviceId;
  String userId;
  unsigned long lastHeartbeat;
  unsigned long sendDataPrevMillis;
  unsigned long lastScheduleSync;
  
  // Command processing
  bool dispenseCommandReceived;
  int lastDispenseCommand;
  
  // Schedule manager reference
  ScheduleManager* scheduleManager;
  
  static const unsigned long HEARTBEAT_INTERVAL = 60000; // 1 minute
  static const unsigned long SEND_DATA_INTERVAL = 5000;  // 5 seconds
  static const unsigned long SCHEDULE_SYNC_INTERVAL = 10000; // 10 seconds (reduced for testing)
  
  // Device paths for streaming
  String deviceParentPath;
  String devicePaths[4] = { "/device_status", "/pill_schedule", "/commands", "/system_config" };
  
  // Callback functions
  static void deviceStreamCallback(MultiPathStream stream);
  static void deviceStreamTimeoutCallback(bool timeout);
  static void scheduleStreamCallback(FirebaseStream data);
  static void scheduleStreamTimeoutCallback(bool timeout);
  
  // Command processing
  void processCommand(String command);
  
public:
  FirebaseManager();
  bool begin(String apiKey, String databaseURL);
  bool connectWiFi(String ssid, String password);
  bool initializeFirebase();
  bool isFirebaseReady();
  
  // Data operations
  bool sendPillDispenseLog(int pillCount, String timestamp);
  bool updateDeviceStatus(String status);
  bool sendHeartbeat(VoltageSensor* voltageSensor = nullptr);
  bool uploadSensorData(String sensorName, String value);
  bool sendPillReport(int pillCount, String datetime, String description, int status);
  bool updateDispenserAfterDispense(int dispenserId, class TimeManager* timeManager);
  
  // Streaming and real-time updates
  bool beginDataStream();
  bool beginScheduleStream();
  void handleStreamUpdates();
  
  // Configuration
  bool downloadSchedule();
  bool checkForCommands();
  bool hasDispenseCommand();
  int getLastDispenseCommand();
  
  // Schedule management
  void setScheduleManager(ScheduleManager* manager);
  void setUserId(String uid);
  bool syncSchedulesFromFirebase();
  bool shouldSyncSchedules();
  
  // Testing functions
  bool testConnection();
  bool testDataUpload();
  bool testDataDownload();
  void printConnectionStatus();
  void printNetworkDiagnostics();
  
  // Utility functions
  String getDeviceId();
  void setDeviceId(String id);
  bool shouldSendData();
  
  // Static instance for callbacks
  static FirebaseManager* instance;
};

#endif