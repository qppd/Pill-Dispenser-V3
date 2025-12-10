#include "FirebaseManager.h"
#include "FirebaseConfig.h"
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
  Serial.println(\"\\nFirebaseManager: Initializing Firebase...\");
  Serial.printf(\"Firebase Client v%s\\n\\n\", FIREBASE_CLIENT_VERSION);
  
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
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);
  
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