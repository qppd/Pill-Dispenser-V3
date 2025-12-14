#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <TimeLib.h>

class TimeManager {
private:
  const char* ntpServer;
  long gmtOffset_sec;
  int daylightOffset_sec;
  unsigned long lastSyncTime;
  bool isTimeSynced;
  static const unsigned long SYNC_INTERVAL = 21600000;
  struct tm timeinfo;
  
public:
  TimeManager() : ntpServer("pool.ntp.org"), gmtOffset_sec(0), daylightOffset_sec(0), 
                  lastSyncTime(0), isTimeSynced(false) {}
  
  void begin(const char* server = "pool.ntp.org", long gmtOffset = 0, int daylightOffset = 0) {
    ntpServer = server;
    gmtOffset_sec = gmtOffset;
    daylightOffset_sec = daylightOffset;
    Serial.println("TimeManager: Initializing NTP");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    syncTime();
  }
  
  bool syncTime() {
    Serial.println("TimeManager: Syncing time with NTP...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(2000);
    
    if (getLocalTime(&timeinfo)) {
      isTimeSynced = true;
      lastSyncTime = millis();
      setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
              timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      Serial.printf("TimeManager: Time synced - %02d:%02d:%02d\n", 
                   timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      return true;
    }
    Serial.println("TimeManager: Time sync failed");
    return false;
  }
  
  void update() {
    if (millis() - lastSyncTime > SYNC_INTERVAL) {
      syncTime();
    }
  }
  
  String getTimeString() {
    char buffer[16];
    sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
    return String(buffer);
  }
  
  String getDateTimeString() {
    char buffer[32];
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", 
            year(), month(), day(), hour(), minute(), second());
    return String(buffer);
  }
  
  bool isSynced() { return isTimeSynced; }
};

#endif
