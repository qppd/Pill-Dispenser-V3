#include "FirebaseManager.h"
#include "FirebaseConfig.h"
#include "ScheduleManager.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Static instance for callbacks
FirebaseManager* FirebaseManager::instance = nullptr;

FirebaseManager::FirebaseManager() {
  isConnected = false;
  isAuthenticated = false;
  signupOk = false;
  lastHeartbeat = 0;
  sendDataPrevMillis = 0;
  lastScheduleSync = 0;
  dispenseCommandReceived = false;
  lastDispenseCommand = 0;
  scheduleManager = nullptr;
  deviceId = "PILL_DISPENSER_" + String(ESP.getEfuseMac(), HEX);
  deviceParentPath = "pilldispenser/device/" + deviceId;
  userId = "";
  
  // Set static instance for callbacks
  instance = this;
}

void FirebaseManager::printNetworkDiagnostics() {
    Serial.println("\n=== Network Diagnostics ===");
    
    // WiFi Status
    Serial.printf("WiFi Status: %s\n", 
                  WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS Server: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());
    
    // DNS Resolution Test
    Serial.println("\nTesting DNS resolution...");
    IPAddress ip;
    if (WiFi.hostByName("pool.ntp.org", ip)) {
        Serial.printf("DNS Test: SUCCESS - pool.ntp.org resolved to %s\n", ip.toString().c_str());
    } else {
        Serial.println("DNS Test: FAILED - Could not resolve pool.ntp.org");
    }
    
    // NTP Time Check
    time_t now = time(nullptr);
    if (now > 1000000000) { // Valid timestamp (after year 2001)
        Serial.printf("NTP Time: SUCCESS - %s", ctime(&now));
    } else {
        Serial.println("NTP Time: FAILED - Time not synchronized");
    }
    
    // Memory Status
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    
    Serial.println("=== End Diagnostics ===\n");
}

bool FirebaseManager::begin(String apiKey, String databaseURL) {
  Serial.println("\nFirebaseManager: Initializing Firebase...");
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  
  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    isConnected = true;
  } else {
    Serial.println("FirebaseManager: WiFi not connected!");
    isConnected = false;
    return false;
  }
  
  // Assign Firebase credentials
  config.api_key = apiKey;
  config.database_url = databaseURL;
  
  config.service_account.data.client_email = PillDispenserConfig::getClientEmail();
  config.service_account.data.project_id = PillDispenserConfig::getProjectId();
  config.service_account.data.private_key = PillDispenserConfig::getPrivateKey();
  
  // Print network diagnostics for debugging
  printNetworkDiagnostics();
  
  return initializeFirebase();
}

bool FirebaseManager::connectWiFi(String ssid, String password) {
  Serial.print("FirebaseManager: Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("FirebaseManager: WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
    isConnected = true;
    return true;
  } else {
    Serial.println();
    Serial.println("FirebaseManager: WiFi connection failed!");
    isConnected = false;
    return false;
  }
}

bool FirebaseManager::initializeFirebase() {
  if (!isConnected) {
    Serial.println("FirebaseManager: Cannot initialize - WiFi not connected");
    return false;
  }
  
  Serial.println("FirebaseManager: Setting up Firebase with service account authentication...");
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  
  // Assign Firebase credentials (using service account)
  config.service_account.data.client_email = PillDispenserConfig::getClientEmail();
  config.service_account.data.project_id = PillDispenserConfig::getProjectId();
  config.service_account.data.private_key = PillDispenserConfig::getPrivateKey();
  
  // Set network reconnection
  Firebase.reconnectNetwork(true);
  
  // Configure buffer sizes for ESP32
  fbdo.setBSSLBufferSize(8192, 1024);
  fbdo.setResponseSize(2048);
  deviceStream.setBSSLBufferSize(8192, 1024);
  deviceStream.setResponseSize(2048);
  scheduleStream.setBSSLBufferSize(8192, 1024);
  scheduleStream.setResponseSize(2048);
  
  // Set timeouts to handle slow connections
  config.timeout.serverResponse = 10 * 1000; // 10 seconds
  config.timeout.socketConnection = 10 * 1000; // 10 seconds
  config.timeout.sslHandshake = 30 * 1000; // 30 seconds
  config.timeout.rtdbKeepAlive = 45 * 1000; // 45 seconds
  config.timeout.rtdbStreamReconnect = 1 * 1000; // 1 second
  config.timeout.rtdbStreamError = 3 * 1000; // 3 seconds
  
  // Begin Firebase with retry logic
  Serial.println("Initializing Firebase with retry logic...");
  
  int retryCount = 0;
  const int maxRetries = 5;
  const int baseDelay = 2000; // 2 seconds
  
  while (retryCount < maxRetries) {
    Firebase.begin(&config, &auth);
    
    // Enable TCP KeepAlive for reliable streaming
    deviceStream.keepAlive(5, 5, 1);
    
    // Wait for Firebase to initialize with timeout
    int initWait = 0;
    const int maxInitWait = 30; // 30 seconds max wait
    
    while (!Firebase.ready() && initWait < maxInitWait) {
      Serial.print(".");
      delay(1000);
      initWait++;
    }
    Serial.println();
    
    if (Firebase.ready()) {
      Serial.println("FirebaseManager: ✅ Firebase initialized successfully!");
      isAuthenticated = true;
      signupOk = true;
      
      // Start data streaming
      beginDataStream();
      
      // Start schedule stream (must be done after userId is set)
      if (!userId.isEmpty()) {
        beginScheduleStream();
      } else {
        Serial.println("FirebaseManager: UserId not set yet, schedule stream will start when setUserId() is called");
      }
      
      // Send initial heartbeat
      sendHeartbeat();
      return true;
    } else {
      retryCount++;
      if (retryCount < maxRetries) {
        int delayTime = baseDelay * retryCount;
        Serial.printf("FirebaseManager: Retry %d/%d failed. Waiting %d ms before retry...\n", 
                     retryCount, maxRetries, delayTime);
        delay(delayTime);
      }
    }
  }
  
  Serial.println("FirebaseManager: ❌ Failed to initialize Firebase after all retries!");
  isAuthenticated = false;
  return false;
}

bool FirebaseManager::beginDataStream() {
  Serial.println("FirebaseManager: Starting device stream...");
  
  if (!Firebase.RTDB.beginMultiPathStream(&deviceStream, deviceParentPath)) {
    Serial.printf("FirebaseManager: Stream initialization failed: %s\n", deviceStream.errorReason().c_str());
    return false;
  } else {
    Firebase.RTDB.setMultiPathStreamCallback(&deviceStream, deviceStreamCallback, deviceStreamTimeoutCallback);
    Serial.println("FirebaseManager: Device stream initialized successfully!");
    return true;
  }
}

void FirebaseManager::deviceStreamCallback(MultiPathStream stream) {
  if (!instance) return;
  
  size_t numChild = sizeof(instance->devicePaths) / sizeof(instance->devicePaths[0]);
  
  // Loop through each path to check if an update has occurred
  for (size_t i = 0; i < numChild; i++) {
    if (stream.get(instance->devicePaths[i])) {
      Serial.println("FirebaseManager: Updated Path: " + stream.dataPath);
      Serial.println("FirebaseManager: New Value: " + stream.value);
      
      if (stream.dataPath == "/device_status") {
        int deviceStatus = stream.value.toInt();
        Serial.print("FirebaseManager: Device status changed to: ");
        Serial.println(deviceStatus);
        // Handle device status changes here
        
      } else if (stream.dataPath == "/commands") {
        String command = stream.value;
        Serial.print("FirebaseManager: Command received via stream: ");
        Serial.println(command);
        // Process command in realtime
        instance->processCommand(command);
        
      } else if (stream.dataPath == "/pill_schedule") {
        String schedule = stream.value;
        Serial.print("FirebaseManager: Schedule updated: ");
        Serial.println(schedule);
        // Trigger schedule sync
        Serial.println("FirebaseManager: Triggering schedule sync due to update...");
        instance->syncSchedulesFromFirebase();
        
      } else if (stream.dataPath == "/system_config") {
        String config = stream.value;
        Serial.print("FirebaseManager: System config updated: ");
        Serial.println(config);
        // Handle system configuration changes here
      }
    }
  }
}

void FirebaseManager::deviceStreamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("FirebaseManager: Stream timed out, attempting to resume...");
  }
  if (instance && !instance->deviceStream.httpConnected()) {
    Serial.printf("FirebaseManager: Stream error code: %d, reason: %s\n", 
                  instance->deviceStream.httpCode(), 
                  instance->deviceStream.errorReason().c_str());
  }
}

bool FirebaseManager::beginScheduleStream() {
  String schedulePath = "pilldispenser/device/" + deviceId + "/schedules";
  Serial.println("FirebaseManager: 🚀 Starting schedule stream on path: " + schedulePath);
  Serial.println("FirebaseManager: Firebase ready status: " + String(isAuthenticated ? "YES" : "NO"));
  
  // Try to begin stream
  if (!Firebase.RTDB.beginStream(&scheduleStream, schedulePath)) {
    Serial.printf("FirebaseManager: ❌ Schedule stream initialization failed: %s\n", 
                  scheduleStream.errorReason().c_str());
    Serial.println("FirebaseManager: Trying alternative stream configuration...");
    
    // Try with a test path to see if streaming works at all
    String testPath = schedulePath + "/test";
    if (!Firebase.RTDB.beginStream(&scheduleStream, testPath)) {
      Serial.printf("FirebaseManager: ❌ Even test stream failed: %s\n", 
                    scheduleStream.errorReason().c_str());
      return false;
    } else {
      Serial.println("FirebaseManager: ✅ Test stream works, but using test path");
    }
  }
  
  Firebase.RTDB.setStreamCallback(&scheduleStream, scheduleStreamCallback, scheduleStreamTimeoutCallback);
  Serial.println("FirebaseManager: ✅ Schedule stream initialized successfully!");
  Serial.println("FirebaseManager: Listening for real-time schedule changes...");
  
  // Enable TCP KeepAlive for reliable streaming
  scheduleStream.keepAlive(5, 5, 1);
  
  // Check if stream is connected immediately
  delay(100); // Small delay to let stream establish
  Serial.println("FirebaseManager: Schedule stream connected: " + String(scheduleStream.httpConnected() ? "YES" : "NO"));
  
  // Test the stream by manually triggering a read (for debugging)
  Serial.println("FirebaseManager: Testing stream with manual read...");
  if (Firebase.RTDB.getJSON(&fbdo, schedulePath)) {
    Serial.println("FirebaseManager: Manual read successful - stream path is accessible");
  } else {
    Serial.printf("FirebaseManager: Manual read failed: %s\n", fbdo.errorReason().c_str());
  }
  
  return true;
}

void FirebaseManager::scheduleStreamCallback(FirebaseStream data) {
  if (!instance) {
    Serial.println("FirebaseManager: Schedule callback - no instance!");
    return;
  }
  
  Serial.println("FirebaseManager: 🔥 SCHEDULE DATA CHANGED!");
  Serial.printf("Stream path: %s\n", data.streamPath().c_str());
  Serial.printf("Data path: %s\n", data.dataPath().c_str());
  Serial.printf("Data type: %s\n", data.dataType().c_str());
  Serial.printf("Event type: %s\n", data.eventType().c_str());
  
  // Print the payload for debugging
  Serial.println("Payload length: " + String(data.payloadLength()));
  if (data.payloadLength() > 0) {
    Serial.println("Payload: " + data.jsonString());
  }
  
  // Trigger schedule resync whenever any schedule data changes
  Serial.println("FirebaseManager: 🔄 Syncing schedules due to real-time update...");
  instance->syncSchedulesFromFirebase();
}

void FirebaseManager::scheduleStreamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("FirebaseManager: Schedule stream timed out, resuming...");
  }
  if (instance && !instance->scheduleStream.httpConnected()) {
    Serial.printf("FirebaseManager: Schedule stream error code: %d, reason: %s\n", 
                  instance->scheduleStream.httpCode(), 
                  instance->scheduleStream.errorReason().c_str());
    
    // Try to restart the stream if it's disconnected
    Serial.println("FirebaseManager: Attempting to restart schedule stream...");
    instance->beginScheduleStream();
  }
  
  // Debug: Check stream status periodically
  static unsigned long lastStatusCheck = 0;
  if (millis() - lastStatusCheck > 30000) { // Every 30 seconds
    Serial.println("FirebaseManager: Schedule stream status - Connected: " + 
                   String(instance && instance->scheduleStream.httpConnected() ? "YES" : "NO"));
    lastStatusCheck = millis();
  }
}

void FirebaseManager::handleStreamUpdates() {
  // This function can be called in the main loop to handle any pending stream updates
  // The actual handling is done in the callback functions
}

bool FirebaseManager::isFirebaseReady() {
  return isConnected && isAuthenticated && Firebase.ready();
}

bool FirebaseManager::shouldSendData() {
  return (millis() - sendDataPrevMillis > SEND_DATA_INTERVAL);
}

String FirebaseManager::getDeviceId() {
  return deviceId;
}

void FirebaseManager::setDeviceId(String id) {
  deviceId = id;
  deviceParentPath = "pilldispenser/device/" + deviceId;
}

bool FirebaseManager::sendPillDispenseLog(int pillCount, String timestamp) {
  if (!isFirebaseReady()) {
    Serial.println("FirebaseManager: Cannot send log - Firebase not ready");
    return false;
  }
  
  String path = deviceParentPath + "/pill_logs/" + String(millis());
  
  FirebaseJson json;
  json.set("timestamp", timestamp);
  json.set("pill_count", pillCount);
  json.set("device_id", deviceId);
  json.set("status", "dispensed");
  json.set("uptime", String(millis()));
  
  if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
    Serial.println("FirebaseManager: Pill dispense log sent successfully");
    sendDataPrevMillis = millis();
    return true;
  } else {
    Serial.print("FirebaseManager: Failed to send pill log - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::updateDeviceStatus(String status) {
  if (!isFirebaseReady()) {
    Serial.println("FirebaseManager: Cannot update status - Firebase not ready");
    return false;
  }
  
  String path = deviceParentPath + "/status";
  
  FirebaseJson json;
  json.set("status", status);
  json.set("last_update", String(millis()));
  json.set("ip_address", WiFi.localIP().toString());
  json.set("wifi_strength", WiFi.RSSI());
  json.set("free_heap", ESP.getFreeHeap());
  
  if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
    Serial.print("FirebaseManager: Device status updated to: ");
    Serial.println(status);
    return true;
  } else {
    Serial.print("FirebaseManager: Failed to update status - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::sendHeartbeat(VoltageSensor* voltageSensor) {
  unsigned long currentTime = millis();
  if (currentTime - lastHeartbeat < HEARTBEAT_INTERVAL) {
    return true; // Not time for heartbeat yet
  }
  
  Serial.println("FirebaseManager: Attempting to send heartbeat...");
  lastHeartbeat = currentTime;
  
  if (!isFirebaseReady()) {
    Serial.println("FirebaseManager: Cannot send heartbeat - Firebase not ready");
    return false;
  }
  
  String path = deviceParentPath + "/heartbeat";
  Serial.print("FirebaseManager: Sending heartbeat to path: ");
  Serial.println(path);
  
  FirebaseJson json;
  json.set("timestamp", String(currentTime));
  json.set("uptime", String(currentTime));
  json.set("wifi_strength", WiFi.RSSI());
  json.set("free_heap", ESP.getFreeHeap());
  json.set("device_status", "online");
  
  // Add battery data if voltage sensor is available
  if (voltageSensor != nullptr) {
    json.set("battery_voltage", String(voltageSensor->readActualVoltage()));
    json.set("battery_percentage", String(voltageSensor->readBatteryPercentage()));
    Serial.print("FirebaseManager: Battery voltage: ");
    Serial.print(voltageSensor->readActualVoltage());
    Serial.print("V, Percentage: ");
    Serial.print(voltageSensor->readBatteryPercentage());
    Serial.println("%");
  } else {
    Serial.println("FirebaseManager: No voltage sensor available");
  }
  
  if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
    Serial.println("FirebaseManager: ✅ Heartbeat sent successfully!");
    return true;
  } else {
    Serial.print("FirebaseManager: ❌ Heartbeat failed - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::uploadSensorData(String sensorName, String value) {
  if (!isFirebaseReady()) {
    return false;
  }
  
  String path = deviceParentPath + "/sensors/" + sensorName;
  
  FirebaseJson json;
  json.set("value", value);
  json.set("timestamp", String(millis()));
  
  return Firebase.RTDB.setJSON(&fbdo, path, &json);
}

bool FirebaseManager::sendPillReport(int pillCount, String datetime, String description, int status) {
  if (!isFirebaseReady()) {
    return false;
  }
  
  FirebaseJson json;
  json.set("pill_count", pillCount);
  json.set("datetime", datetime);
  json.set("description", description);
  json.set("status", status);
  json.set("device_id", deviceId);
  
  // Push data with unique Firebase key
  if (Firebase.RTDB.pushJSON(&fbdo, "/pilldispenser/reports", &json)) {
    Serial.println("FirebaseManager: Pill report sent successfully!");
    Serial.println("FirebaseManager: Generated Key: " + fbdo.pushName());
    return true;
  } else {
    Serial.println("FirebaseManager: Report failed: " + fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::downloadSchedule() {
  if (!isFirebaseReady()) {
    return false;
  }
  
  String path = deviceParentPath + "/schedule";
  
  if (Firebase.RTDB.getJSON(&fbdo, path)) {
    Serial.println("FirebaseManager: Schedule data retrieved");
    FirebaseJson* json = fbdo.to<FirebaseJson*>();
    // Process schedule data here
    return true;
  } else {
    Serial.print("FirebaseManager: Failed to get schedule - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::checkForCommands() {
  if (!isFirebaseReady()) {
    return false;
  }
  
  String path = deviceParentPath + "/commands";
  
  if (Firebase.RTDB.getString(&fbdo, path)) {
    String command = fbdo.to<String>();
    if (command.length() > 0) {
      Serial.print("FirebaseManager: Command received: ");
      Serial.println(command);
      
      // Process the command
      processCommand(command);
      
      // Clear the command after reading (only for polling mode)
      Firebase.RTDB.deleteNode(&fbdo, path);
      return true;
    }
  }
  return false;
}

void FirebaseManager::processCommand(String command) {
  command.trim();
  command.toUpperCase();
  
  if (command.startsWith("DISPENSE:")) {
    // Extract dispenser ID from command like "DISPENSE:1"
    int colonIndex = command.indexOf(':');
    if (colonIndex > 0) {
      String dispenserIdStr = command.substring(colonIndex + 1);
      int dispenserId = dispenserIdStr.toInt();
      
      if (dispenserId >= 1 && dispenserId <= 5) {
        Serial.print("FirebaseManager: Processing dispense command for dispenser ");
        Serial.println(dispenserId);
        
        // Trigger dispense - this will be handled by the main sketch
        // For now, we'll set a flag that the main loop can check
        lastDispenseCommand = dispenserId;
        dispenseCommandReceived = true;
      } else {
        Serial.println("FirebaseManager: Invalid dispenser ID in command");
      }
    }
  } else {
    Serial.print("FirebaseManager: Unknown command: ");
    Serial.println(command);
  }
}

bool FirebaseManager::hasDispenseCommand() {
  return dispenseCommandReceived;
}

int FirebaseManager::getLastDispenseCommand() {
  if (dispenseCommandReceived) {
    dispenseCommandReceived = false; // Reset the flag
    return lastDispenseCommand;
  }
  return 0;
}

void FirebaseManager::printConnectionStatus() {
  Serial.println("=== Firebase Connection Status ===");
  Serial.print("WiFi Connected: ");
  Serial.println(isConnected ? "YES" : "NO");
  if (isConnected) {
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }
  Serial.print("Firebase Ready: ");
  Serial.println(isFirebaseReady() ? "YES" : "NO");
  Serial.print("Device ID: ");
  Serial.println(deviceId);
  Serial.print("Device Path: ");
  Serial.println(deviceParentPath);
  Serial.println("================================");
}

bool FirebaseManager::testConnection() {
  Serial.println("FirebaseManager: Testing Firebase connection...");
  
  if (!isFirebaseReady()) {
    Serial.println("FirebaseManager: Connection test failed - Firebase not ready");
    return false;
  }
  
  String testPath = deviceParentPath + "/test";
  String testData = "Connection test at " + String(millis());
  
  if (Firebase.RTDB.setString(&fbdo, testPath, testData)) {
    Serial.println("FirebaseManager: Connection test PASSED");
    return true;
  } else {
    Serial.print("FirebaseManager: Connection test FAILED - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::testDataUpload() {
  Serial.println("FirebaseManager: Testing data upload...");
  
  FirebaseJson testJson;
  testJson.set("test_string", "Hello Firebase");
  testJson.set("test_number", 42);
  testJson.set("test_timestamp", String(millis()));
  testJson.set("test_device", deviceId);
  
  String testPath = "/test_uploads/" + String(millis());
  
  if (Firebase.RTDB.setJSON(&fbdo, testPath, &testJson)) {
    Serial.println("FirebaseManager: Data upload test PASSED");
    return true;
  } else {
    Serial.print("FirebaseManager: Data upload test FAILED - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

bool FirebaseManager::testDataDownload() {
  Serial.println("FirebaseManager: Testing data download...");
  
  String testPath = deviceParentPath + "/test";
  
  if (Firebase.RTDB.getString(&fbdo, testPath)) {
    Serial.print("FirebaseManager: Downloaded data: ");
    Serial.println(fbdo.to<String>());
    Serial.println("FirebaseManager: Data download test PASSED");
    return true;
  } else {
    Serial.print("FirebaseManager: Data download test FAILED - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}

void FirebaseManager::setScheduleManager(ScheduleManager* manager) {
  scheduleManager = manager;
  Serial.println("FirebaseManager: Schedule manager linked");
}

void FirebaseManager::setUserId(String uid) {
  userId = uid;
  Serial.println("FirebaseManager: User ID set to " + userId + " (optional - not used for device paths)");
}

bool FirebaseManager::shouldSyncSchedules() {
  return (millis() - lastScheduleSync > SCHEDULE_SYNC_INTERVAL);
}

bool FirebaseManager::syncSchedulesFromFirebase() {
  if (!isFirebaseReady()) {
    Serial.println("FirebaseManager: Cannot sync schedules - Firebase not ready");
    return false;
  }

  if (!scheduleManager) {
    Serial.println("FirebaseManager: Cannot sync schedules - ScheduleManager not set");
    return false;
  }

  Serial.println("FirebaseManager: Syncing schedules from Firebase...");
  
  String schedulePath = "pilldispenser/device/" + deviceId + "/schedules";
  Serial.println("FirebaseManager: Schedule path: " + schedulePath);
  
  if (Firebase.RTDB.getJSON(&fbdo, schedulePath)) {
    Serial.println("FirebaseManager: Successfully retrieved data from Firebase");
    FirebaseJson* json = fbdo.to<FirebaseJson*>();
    
    // Clear existing schedules
    scheduleManager->clearAllSchedules();
    
    // Parse and add schedules
    size_t len = json->iteratorBegin();
    Serial.println("FirebaseManager: Found " + String(len) + " schedule entries");
    String key, value = "";
    int type = 0;
    int addedCount = 0;
    
    for (size_t i = 0; i < len; i++) {
      json->iteratorGet(i, type, key, value);
      
      // Parse individual schedule
      FirebaseJson scheduleJson;
      scheduleJson.setJsonData(value);
      
      FirebaseJsonData data;
      
      int dispenserId = 0;
      int hour = 0;
      int minute = 0;
      bool enabled = true;
      String medicationName = "";
      String patientName = "";
      String pillSize = "medium";
      String timeStr = "";
      
      // Try both field name formats (original schedule page vs schedule-v2)
      if (scheduleJson.get(data, "dispenserId") || scheduleJson.get(data, "dispenser_id")) {
        dispenserId = data.to<int>();
      }
      if (scheduleJson.get(data, "time")) {
        timeStr = data.to<String>();
        // Parse time string "HH:MM"
        int colonIndex = timeStr.indexOf(':');
        if (colonIndex > 0) {
          hour = timeStr.substring(0, colonIndex).toInt();
          minute = timeStr.substring(colonIndex + 1).toInt();
        }
      } else {
        // Try separate hour/minute fields
        if (scheduleJson.get(data, "hour")) {
          hour = data.to<int>();
        }
        if (scheduleJson.get(data, "minute")) {
          minute = data.to<int>();
        }
      }
      if (scheduleJson.get(data, "enabled")) {
        enabled = data.to<bool>();
      }
      if (scheduleJson.get(data, "medicationName") || scheduleJson.get(data, "medication_name")) {
        medicationName = data.to<String>();
      }
      if (scheduleJson.get(data, "patientName") || scheduleJson.get(data, "patient_name")) {
        patientName = data.to<String>();
      }
      if (scheduleJson.get(data, "pillSize") || scheduleJson.get(data, "pill_size")) {
        pillSize = data.to<String>();
      }
      
      // Add schedule to manager
      if (scheduleManager->addSchedule(key, dispenserId, hour, minute, 
                                       medicationName, patientName, pillSize, enabled)) {
        addedCount++;
        Serial.printf("Added schedule: %s - %02d:%02d for dispenser %d\n", 
                     key.c_str(), hour, minute, dispenserId);
      }
    }
    
    json->iteratorEnd();
    
    lastScheduleSync = millis();
    Serial.printf("FirebaseManager: Schedule sync complete - %d schedules loaded\n", addedCount);
    
    // Print all schedules
    scheduleManager->printSchedules();
    
    return true;
  } else {
    Serial.print("FirebaseManager: Failed to sync schedules - ");
    Serial.println(fbdo.errorReason());
    return false;
  }
}