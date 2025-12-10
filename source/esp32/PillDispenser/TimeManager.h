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
  
  static const unsigned long SYNC_INTERVAL = 21600000; // 6 hours in milliseconds
  
  struct tm timeinfo;
  
public:
  TimeManager();
  void begin(const char* server = "pool.ntp.org", long gmtOffset = 0, int daylightOffset = 0);
  void update();
  bool syncTime();
  
  // Time retrieval functions
  String getTimeString();
  String getDateString();
  String getDateTimeString();
  time_t getTimestamp();
  
  // Enhanced time functions with fallback
  time_t getTimestampWithFallback();
  String getFormattedDateTime();
  String getFormattedDateTimeWithFallback();
  String getCurrentLogPrefix();
  bool isNTPSynced();
  
  // Individual components
  int getHour();
  int getMinute();
  int getSecond();
  int getDay();
  int getMonth();
  int getYear();
  
  // Status functions
  bool isSynced();
  unsigned long getLastSyncTime();
  void printDebug();
  void testTime();
  
  // Utility functions
  bool isTimeValid();
  String getFormattedTime(const char* format);
};

#endif
