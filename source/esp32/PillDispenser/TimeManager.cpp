#include "TimeManager.h"
#include <Arduino.h>
#include <WiFi.h>

TimeManager::TimeManager() {
  ntpServer = "pool.ntp.org";
  gmtOffset_sec = 0;
  daylightOffset_sec = 0;
  lastSyncTime = 0;
  isTimeSynced = false;
  memset(&timeinfo, 0, sizeof(timeinfo));
}

void TimeManager::begin(const char* server, long gmtOffset, int daylightOffset) {
  ntpServer = server;
  gmtOffset_sec = gmtOffset;
  daylightOffset_sec = daylightOffset;

  Serial.println("TimeManager: Initializing NTP time synchronization...");
  Serial.printf("TimeManager: NTP Server: %s, GMT Offset: %ld, Daylight Offset: %d\n", server, gmtOffset, daylightOffset);

  // Set timezone with configTime
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov", "ntp.ubuntu.com");

  Serial.print("TimeManager: Waiting for NTP time sync");
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 20) {
    Serial.print(".");
    delay(1000);
    retries++;
  }

  if (retries >= 20) {
    Serial.println("\nTimeManager: ❌ Failed to get time from NTP after 20 attempts");
    isTimeSynced = false;

    // Try to set a fallback time based on compile time
    Serial.println("TimeManager: Setting fallback time based on compilation time");
    struct tm compileTime = {0};
    compileTime.tm_year = 2025 - 1900; // Year since 1900
    compileTime.tm_mon = 11;  // December (0-based)
    compileTime.tm_mday = 11; // Day
    compileTime.tm_hour = 12; // Hour
    compileTime.tm_min = 0;   // Minute
    compileTime.tm_sec = 0;   // Second
    time_t fallbackTime = mktime(&compileTime);
    struct timeval tv = {fallbackTime, 0};
    settimeofday(&tv, nullptr);
    Serial.println("TimeManager: Fallback time set");
  } else {
    Serial.println("\nTimeManager: ✅ Time synced from NTP successfully!");
    isTimeSynced = true;
    lastSyncTime = millis();

    // Print current time for verification
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.print("TimeManager: Current time: ");
    Serial.println(timeStringBuff);

    // Note: Arduino Time library sync removed to avoid conflicts with system time
  }
}

bool TimeManager::syncTime() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("TimeManager: ❌ Cannot sync - WiFi not connected");
    return false;
  }

  Serial.println("TimeManager: 🔄 Re-syncing time with NTP server...");

  // Force reconfigure NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov", "ntp.ubuntu.com");

  // Wait a bit for NTP to update
  delay(2000);

  // Get current time
  if (!getLocalTime(&timeinfo)) {
    Serial.println("TimeManager: ❌ Failed to obtain time from NTP");
    isTimeSynced = false;
    return false;
  }

  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.print("TimeManager: ✅ Current DateTime: ");
  Serial.println(timeStringBuff);

  // Note: Arduino Time library sync removed to avoid conflicts with system time

  isTimeSynced = true;
  lastSyncTime = millis();
  return true;
}

void TimeManager::forceSync() {
  Serial.println("TimeManager: 🔄 Force syncing time...");
  syncTime();
}

void TimeManager::update() {
  // Check if time is valid, if not, try to sync immediately
  time_t currentTime = time(nullptr);
  if (currentTime < 1000000000) { // Invalid time (before year 2001)
    Serial.println("TimeManager: System time is invalid, attempting immediate NTP sync...");
    syncTime();
    return;
  }

  // Re-sync every 6 hours or if time seems stuck
  static time_t lastValidTime = 0;
  if (currentTime == lastValidTime) {
    // Time is not advancing, force a sync
    Serial.println("TimeManager: Time appears stuck, forcing NTP sync...");
    syncTime();
  } else {
    lastValidTime = currentTime;
  }

  if (isTimeSynced && (millis() - lastSyncTime > SYNC_INTERVAL)) {
    Serial.println("TimeManager: Auto-sync triggered (6 hour interval)");
    syncTime();
  }
}

time_t TimeManager::getTimestampWithFallback() {
  time_t timestamp = getTimestamp();
  
  if (timestamp == 0) {
    // NTP failed, use fallback with warning
    Serial.println("TimeManager: ⚠️ NTP unavailable, using fallback timestamp");
    unsigned long fallbackTime = millis() / 1000 + 1692620000; // Fallback base
    return (time_t)fallbackTime;
  }
  
  // Log successful NTP time retrieval periodically
  static unsigned long lastNTPLog = 0;
  if (millis() - lastNTPLog > 300000) { // Log every 5 minutes
    Serial.println("TimeManager: ✅ NTP time synchronized: " + getDateTimeString());
    lastNTPLog = millis();
  }
  
  return timestamp;
}

bool TimeManager::isNTPSynced() {
  time_t now = time(nullptr);
  return (now > 1000000000); // Valid timestamp (after year 2001)
}

String TimeManager::getFormattedDateTime() {
  if (!getLocalTime(&timeinfo)) {
    // Return fallback formatted time if NTP fails
    unsigned long fallbackTimestamp = millis() / 1000 + 1692620000;
    time_t fallbackTime = (time_t)fallbackTimestamp;
    struct tm* fallbackTimeInfo = localtime(&fallbackTime);
    
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", fallbackTimeInfo);
    return String(timeStringBuff) + " (EST)"; // Estimated time
  }
  
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

String TimeManager::getFormattedDateTimeWithFallback() {
  if (isNTPSynced()) {
    return getFormattedDateTime();
  } else {
    // Use fallback with clear indication
    unsigned long fallbackTimestamp = millis() / 1000 + 1692620000;
    time_t fallbackTime = (time_t)fallbackTimestamp;
    struct tm* fallbackTimeInfo = localtime(&fallbackTime);
    
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", fallbackTimeInfo);
    return String(timeStringBuff) + " (EST)"; // Estimated time
  }
}

String TimeManager::getCurrentLogPrefix() {
  String datetime = getFormattedDateTimeWithFallback();
  return "[" + datetime + "] ";
}

String TimeManager::getTimeString() {
  // Always get fresh time from system using ESP32 native functions
  time_t now = time(nullptr);
  if (now < 1000000000) {
    // Invalid time, return fallback
    return "12/11/2025 12:00:00 PM";
  }

  struct tm timeInfo;
  localtime_r(&now, &timeInfo);

  // Update the class member for other functions
  timeinfo = timeInfo;

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%m/%d/%Y %I:%M:%S %p", &timeInfo);
  return String(buffer);
}

String TimeManager::getDateString() {
  if (!getLocalTime(&timeinfo)) {
    return "0000-00-00";
  }
  
  char buffer[11];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
  return String(buffer);
}

String TimeManager::getDateTimeString() {
  // Always get fresh time from system using ESP32 native functions
  time_t now = time(nullptr);
  if (now < 1000000000) {
    // Invalid time, return fallback
    return "2025-12-11 12:00:00";
  }

  struct tm timeInfo;
  localtime_r(&now, &timeInfo);

  // Update the class member for other functions
  timeinfo = timeInfo;

  char buffer[20];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
  return String(buffer);
}

time_t TimeManager::getTimestamp() {
  return time(nullptr);
}

int TimeManager::getHour() {
  if (!getLocalTime(&timeinfo)) return 0;
  return timeinfo.tm_hour;
}

int TimeManager::getMinute() {
  if (!getLocalTime(&timeinfo)) return 0;
  return timeinfo.tm_min;
}

int TimeManager::getSecond() {
  if (!getLocalTime(&timeinfo)) return 0;
  return timeinfo.tm_sec;
}

int TimeManager::getDay() {
  if (!getLocalTime(&timeinfo)) return 0;
  return timeinfo.tm_mday;
}

int TimeManager::getMonth() {
  if (!getLocalTime(&timeinfo)) return 0;
  return timeinfo.tm_mon + 1; // tm_mon is 0-11
}

int TimeManager::getYear() {
  if (!getLocalTime(&timeinfo)) return 0;
  return timeinfo.tm_year + 1900; // tm_year is years since 1900
}

bool TimeManager::isSynced() {
  return isTimeSynced && isTimeValid();
}

unsigned long TimeManager::getLastSyncTime() {
  return lastSyncTime;
}

bool TimeManager::isTimeValid() {
  time_t now = time(nullptr);
  // Consider time valid if it's after 2020-01-01 (timestamp > 1577836800)
  return (now > 1577836800);
}

String TimeManager::getFormattedTime(const char* format) {
  if (!getLocalTime(&timeinfo)) {
    return "N/A";
  }
  
  char buffer[64];
  strftime(buffer, sizeof(buffer), format, &timeinfo);
  return String(buffer);
}

void TimeManager::printDebug() {
  Serial.println("─────────────────────────────────────");
  Serial.println("[TIME MANAGER DEBUG]");
  Serial.print("Current Time:    ");
  Serial.println(getTimeString());
  Serial.print("Current Date:    ");
  Serial.println(getDateString());
  Serial.print("Timestamp:       ");
  Serial.println(getTimestamp());
  Serial.print("Sync Status:     ");
  Serial.println(isTimeSynced ? "✅ SYNCED" : "❌ NOT SYNCED");
  Serial.print("Time Valid:      ");
  Serial.println(isTimeValid() ? "✅ YES" : "❌ NO");
  
  if (lastSyncTime > 0) {
    unsigned long timeSinceSync = (millis() - lastSyncTime) / 1000;
    Serial.print("Last Sync:       ");
    Serial.print(timeSinceSync);
    Serial.println(" seconds ago");
  }
  
  Serial.print("WiFi Status:     ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "✅ Connected" : "❌ Disconnected");
  Serial.println("─────────────────────────────────────");
}

void TimeManager::testTime() {
  Serial.println("TimeManager: Starting time test");
  Serial.println("TimeManager: Displaying time every 1 second");
  Serial.println("TimeManager: Type 'exit' to stop test");
  Serial.println();
  
  unsigned long lastUpdate = 0;
  bool testRunning = true;
  
  while (testRunning) {
    // Check for exit command
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      input.toLowerCase();
      
      if (input == "exit") {
        Serial.println();
        Serial.println("TimeManager: Test stopped by user");
        testRunning = false;
        break;
      }
    }
    
    // Update display every 1 second
    if (millis() - lastUpdate >= 1000) {
      lastUpdate = millis();
      
      // Get current time
      if (!getLocalTime(&timeinfo)) {
        Serial.println("❌ Failed to get local time");
        continue;
      }
      
      // Print formatted output
      Serial.println("─────────────────────────────────────");
      Serial.println("[TIME TEST]");
      Serial.print("Current Time:    ");
      Serial.println(getTimeString());
      Serial.print("Current Date:    ");
      Serial.println(getDateString());
      Serial.print("Timestamp:       ");
      Serial.println(getTimestamp());
      Serial.print("Day of Week:     ");
      
      const char* daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
      Serial.println(daysOfWeek[timeinfo.tm_wday]);
      
      Serial.print("Status:          ");
      if (!isTimeSynced) {
        Serial.println("⚠️  NOT SYNCED");
      } else if (!isTimeValid()) {
        Serial.println("⚠️  INVALID TIME");
      } else {
        Serial.println("✅ OK");
      }
      
      Serial.println("─────────────────────────────────────");
      Serial.println();
    }
    
    delay(100);
  }
  
  Serial.println("TimeManager: Test complete");
}
