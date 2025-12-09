#include "FirebaseManager.h"
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
  deviceId = "PILL_DISPENSER_" + String(ESP.getEfuseMac(), HEX);
  deviceParentPath = "pilldispenser/device/" + deviceId;
  
  // Set static instance for callbacks
  instance = this;
}

bool FirebaseManager::begin(String apiKey, String databaseURL) {
  config.api_key = apiKey;
  config.database_url = databaseURL;
  
  Serial.println("FirebaseManager: Initializing Firebase...");
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
  
  Serial.println("FirebaseManager: Setting up Firebase authentication...");
  
  // Use anonymous authentication (no email/password required)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("FirebaseManager: Anonymous signup successful");
    signupOk = true;
  } else {
    Serial.printf("FirebaseManager: Signup failed: %s\n", config.signer.signupError.message.c_str());
    signupOk = true; // Continue anyway for development
  }
  
  // Set token status callback
  config.token_status_callback = tokenStatusCallback;
  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  // Wait for Firebase to be ready
  unsigned long startTime = millis();
  while (!Firebase.ready() && millis() - startTime < 10000) {
    delay(100);
  }
  
  if (Firebase.ready()) {
    Serial.println("FirebaseManager: Firebase initialized successfully");
    isAuthenticated = true;
    
    // Start data streaming
    beginDataStream();
    
    // Send initial heartbeat
    sendHeartbeat();
    return true;
  } else {
    Serial.println("FirebaseManager: Firebase initialization failed");
    isAuthenticated = false;
    return false;
  }
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
        
      } else if (stream.dataPath == "/dispense_command") {
        String command = stream.value;
        Serial.print("FirebaseManager: Dispense command received: ");
        Serial.println(command);
        // Handle dispense commands here
        
      } else if (stream.dataPath == "/pill_schedule") {
        String schedule = stream.value;
        Serial.print("FirebaseManager: Schedule updated: ");
        Serial.println(schedule);
        // Handle schedule updates here
        
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

bool FirebaseManager::sendHeartbeat() {
  unsigned long currentTime = millis();
  if (currentTime - lastHeartbeat < HEARTBEAT_INTERVAL) {
    return true; // Not time for heartbeat yet
  }
  
  lastHeartbeat = currentTime;
  
  if (!isFirebaseReady()) {
    return false;
  }
  
  String path = deviceParentPath + "/heartbeat";
  
  FirebaseJson json;
  json.set("timestamp", String(currentTime));
  json.set("uptime", String(currentTime));
  json.set("wifi_strength", WiFi.RSSI());
  json.set("free_heap", ESP.getFreeHeap());
  json.set("device_status", "online");
  
  if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
    Serial.println("FirebaseManager: Heartbeat sent");
    return true;
  } else {
    Serial.print("FirebaseManager: Heartbeat failed - ");
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
      
      // Clear the command after reading
      Firebase.RTDB.deleteNode(&fbdo, path);
      return true;
    }
  }
  return false;
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