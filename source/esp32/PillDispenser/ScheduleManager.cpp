#include "ScheduleManager.h"
#include <Arduino.h>

// Static instance for callbacks
ScheduleManager* ScheduleManager::instance = nullptr;

ScheduleManager::ScheduleManager() {
  scheduleCount = 0;
  onDispenseCallback = nullptr;
  onNotifyCallback = nullptr;
  timeManager = nullptr;
  instance = this;
  
  // Initialize all schedules
  for (int i = 0; i < MAX_SCHEDULES; i++) {
    schedules[i].id = "";
    schedules[i].dispenserId = -1;
    schedules[i].hour = 0;
    schedules[i].minute = 0;
    schedules[i].enabled = false;
    schedules[i].alarmId = dtINVALID_ALARM_ID;
    for (int j = 0; j < 7; j++) {
      schedules[i].weekdays[j] = true; // Default: all days enabled
    }
  }
}

void ScheduleManager::begin(String deviceId) {
  this->deviceId = deviceId;
  Serial.println("ScheduleManager: Initialized for device " + deviceId);
  Serial.println("ScheduleManager: Max schedules: " + String(MAX_SCHEDULES));
}

void ScheduleManager::update() {
  // TimeAlarms service routine is called via Alarm.delay() in main loop
  // No need to call it here
  
  // Debug: Print current time every 30 seconds
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 30000) {
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════");
    Serial.print("⏰ ScheduleManager: TimeLib shows: ");
    Serial.printf("%02d:%02d:%02d", hour(), minute(), second());
    if (timeManager) {
      Serial.print(" | Software RTC: ");
      Serial.print(timeManager->getTimeString());
    }
    Serial.println();
    Serial.print("📋 Active alarms in TimeAlarms library: ");
    Serial.println(Alarm.count());
    Serial.print("📅 Schedules in ScheduleManager: ");
    Serial.println(scheduleCount);
    Serial.println("═══════════════════════════════════════════════════");
    Serial.println();
    lastDebug = millis();
  }
}

bool ScheduleManager::addSchedule(String id, int dispenserId, int hour, int minute,
                                  String medicationName, String patientName,
                                  String pillSize, bool enabled) {
  if (scheduleCount >= MAX_SCHEDULES) {
    Serial.println("ScheduleManager: Cannot add schedule - maximum reached");
    return false;
  }
  
  if (dispenserId < 0 || dispenserId > 4) {
    Serial.println("ScheduleManager: Invalid dispenser ID (must be 0-4)");
    return false;
  }
  
  if (hour < 0 || hour > 23 || minute < 0 || minute > 59) {
    Serial.println("ScheduleManager: Invalid time");
    return false;
  }
  
  // Check if schedule ID already exists - update it instead of rejecting
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].id == id) {
      Serial.println("ScheduleManager: Schedule ID exists - updating instead");
      // Free the old alarm first
      if (schedules[i].alarmId != dtINVALID_ALARM_ID) {
        Alarm.free(schedules[i].alarmId);
      }
      
      // Update the schedule
      schedules[i].dispenserId = dispenserId;
      schedules[i].hour = hour;
      schedules[i].minute = minute;
      schedules[i].enabled = enabled;
      schedules[i].medicationName = medicationName;
      schedules[i].patientName = patientName;
      schedules[i].pillSize = pillSize;
      
      // Create new alarm if enabled
      if (enabled) {
        OnTick_t callback = getCallbackFunction(i);
        if (callback != nullptr) {
          schedules[i].alarmId = Alarm.alarmRepeat(hour, minute, 0, callback);
          Serial.printf("ScheduleManager: ✅ Alarm updated for %02d:%02d - %s (%s) - AlarmID: %d\n", 
                        schedules[i].hour, schedules[i].minute, medicationName.c_str(), patientName.c_str(), schedules[i].alarmId);
        }
      } else {
        schedules[i].alarmId = dtINVALID_ALARM_ID;
      }
      
      return true;
    }
  }
  
  int index = scheduleCount;
  schedules[index].id = id;
  schedules[index].dispenserId = dispenserId;
  schedules[index].hour = hour;
  schedules[index].minute = minute;
  schedules[index].enabled = enabled;
  schedules[index].medicationName = medicationName;
  schedules[index].patientName = patientName;
  schedules[index].pillSize = pillSize;
  
  // Create alarm if enabled
  if (enabled) {
    OnTick_t callback = getCallbackFunction(index);
    if (callback != nullptr) {
      schedules[index].alarmId = Alarm.alarmRepeat(hour, minute, 0, callback);
      Serial.println("\n" + String('─', 60));
      Serial.println("✅ ALARM CREATED SUCCESSFULLY");
      Serial.printf("   Schedule Index: %d\n", index);
      Serial.printf("   Trigger Time: %02d:%02d:00\n", schedules[index].hour, schedules[index].minute);
      Serial.printf("   Medication: %s\n", medicationName.c_str());
      Serial.printf("   Patient: %s\n", patientName.c_str());
      Serial.printf("   Dispenser ID: %d\n", dispenserId);
      Serial.printf("   AlarmID: %d\n", schedules[index].alarmId);
      Serial.printf("   Current TimeLib time: %02d:%02d:%02d\n", ::hour(), ::minute(), ::second());
      Serial.printf("   Total alarms active: %d\n", Alarm.count());
      Serial.println(String('─', 60) + "\n");
    } else {
      Serial.println("ScheduleManager: ❌ Warning - No callback available for schedule " + String(index));
    }
  } else {
    Serial.println("⚠️  Schedule disabled, no alarm created");
  }
  
  scheduleCount++;
  
  Serial.println("ScheduleManager: Schedule added - Total: " + String(scheduleCount));
  return true;
}

bool ScheduleManager::removeSchedule(String id) {
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].id == id) {
      // Free the alarm
      if (schedules[i].alarmId != dtINVALID_ALARM_ID) {
        Alarm.free(schedules[i].alarmId);
      }
      
      // Shift remaining schedules
      for (int j = i; j < scheduleCount - 1; j++) {
        schedules[j] = schedules[j + 1];
      }
      
      scheduleCount--;
      Serial.println("ScheduleManager: Schedule removed - " + id);
      return true;
    }
  }
  
  Serial.println("ScheduleManager: Schedule not found - " + id);
  return false;
}

bool ScheduleManager::updateSchedule(String id, int hour, int minute, bool enabled) {
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].id == id) {
      // Remove old alarm
      if (schedules[i].alarmId != dtINVALID_ALARM_ID) {
        Alarm.free(schedules[i].alarmId);
        schedules[i].alarmId = dtINVALID_ALARM_ID;
      }
      
      // Update schedule
      schedules[i].hour = hour;
      schedules[i].minute = minute;
      schedules[i].enabled = enabled;
      
      // Create new alarm if enabled
      if (enabled) {
        OnTick_t callback = getCallbackFunction(i);
        if (callback != nullptr) {
          schedules[i].alarmId = Alarm.alarmRepeat(hour, minute, 0, callback);
          Serial.printf("ScheduleManager: Schedule updated - %02d:%02d\n", schedules[i].hour, schedules[i].minute);
        }
      } else {
        Serial.println("ScheduleManager: Schedule disabled - " + id);
      }
      
      return true;
    }
  }
  
  return false;
}

void ScheduleManager::clearAllSchedules() {
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].alarmId != dtINVALID_ALARM_ID) {
      Alarm.free(schedules[i].alarmId);
    }
  }
  scheduleCount = 0;
  Serial.println("ScheduleManager: All schedules cleared");
}

int ScheduleManager::getScheduleCount() {
  return scheduleCount;
}

int ScheduleManager::getActiveScheduleCount() {
  int count = 0;
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].enabled) count++;
  }
  return count;
}

MedicationSchedule* ScheduleManager::getSchedule(int index) {
  if (index >= 0 && index < scheduleCount) {
    return &schedules[index];
  }
  return nullptr;
}

MedicationSchedule* ScheduleManager::getScheduleById(String id) {
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].id == id) {
      return &schedules[i];
    }
  }
  return nullptr;
}

bool ScheduleManager::isTodayScheduled(int scheduleIndex) {
  if (scheduleIndex < 0 || scheduleIndex >= scheduleCount) {
    return false;
  }
  
  // Get current day of week (0=Sunday, 1=Monday, etc.)
  // Adjust to our format (0=Monday, 6=Sunday)
  int dow = (weekday() + 5) % 7;  // weekday() is from Time library
  
  bool scheduled = schedules[scheduleIndex].weekdays[dow];
  
  // Debug: Print weekday check
  static unsigned long lastWeekdayDebug = 0;
  if (millis() - lastWeekdayDebug > 60000) { // Every minute
    const char* dayNames[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    Serial.printf("ScheduleManager: Today is %s (dow=%d), Schedule %d enabled for today: %s\n", 
                  dayNames[dow], dow, scheduleIndex, scheduled ? "YES" : "NO");
    lastWeekdayDebug = millis();
  }
  
  return scheduled;
}

void ScheduleManager::triggerSchedule(int scheduleIndex) {
  Serial.println("\n" + String('=', 50));
  Serial.println("⏰ ALARM CALLBACK TRIGGERED - Schedule Index: " + String(scheduleIndex));
  Serial.println(String('=', 50));
  
  if (scheduleIndex < 0 || scheduleIndex >= scheduleCount) {
    Serial.println("❌ Invalid schedule index: " + String(scheduleIndex));
    return;
  }
  
  MedicationSchedule* schedule = &schedules[scheduleIndex];
  
  if (!schedule->enabled) {
    Serial.println("❌ Schedule disabled, skipping");
    return;
  }
  
  // Check if today is a scheduled day
  if (!isTodayScheduled(scheduleIndex)) {
    Serial.println("❌ Not scheduled for today, skipping");
    return;
  }
  
  Serial.println("✅ Schedule validation passed - proceeding with dispense");
  Serial.println("\n" + String('=', 50));
  Serial.println("⏰ SCHEDULED DISPENSING TRIGGERED");
  Serial.println(String('=', 50));
  Serial.printf("Time: %02d:%02d\n", schedule->hour, schedule->minute);
  Serial.println("Patient: " + schedule->patientName);
  Serial.println("Medication: " + schedule->medicationName);
  Serial.println("Dispenser: " + String(schedule->dispenserId));
  Serial.println("Size: " + schedule->pillSize);
  Serial.println(String('=', 50) + "\n");
  
  // Trigger dispense callback
  if (onDispenseCallback != nullptr) {
    onDispenseCallback(schedule->dispenserId, schedule->pillSize, 
                      schedule->medicationName, schedule->patientName);
  }
}

void ScheduleManager::setDispenseCallback(void (*callback)(int, String, String, String)) {
  onDispenseCallback = callback;
}

void ScheduleManager::setNotifyCallback(void (*callback)(String, String)) {
  onNotifyCallback = callback;
}

bool ScheduleManager::syncSchedulesFromFirebase(FirebaseData* fbdo, String basePath) {
  // This will be called to load schedules from Firebase
  // Format: basePath/schedules/{scheduleId}
  Serial.println("ScheduleManager: Syncing schedules from Firebase...");
  
  // Implementation depends on Firebase structure
  // For now, return true to indicate ready for sync
  return true;
}

bool ScheduleManager::uploadScheduleStatus(FirebaseData* fbdo, String basePath, 
                                          String scheduleId, String status) {
  // Upload execution status to Firebase
  Serial.println("ScheduleManager: Uploading schedule status: " + scheduleId + " -> " + status);
  return true;
}

void ScheduleManager::printSchedules() {
  Serial.println("\n" + String('=', 60));
  Serial.println("📋 ACTIVE SCHEDULES (" + String(getActiveScheduleCount()) + 
                " / " + String(scheduleCount) + ")");
  Serial.println(String('=', 60));
  
  if (scheduleCount == 0) {
    Serial.println("No schedules configured");
  } else {
    for (int i = 0; i < scheduleCount; i++) {
      MedicationSchedule* s = &schedules[i];
      Serial.printf("%2d. %s %02d:%02d | Dispenser %d | %s\n",
                   i + 1,
                   s->enabled ? "✅" : "❌",
                   s->hour,
                   s->minute,
                   s->dispenserId,
                   s->medicationName.c_str());
      Serial.println("    Patient: " + s->patientName + " | Size: " + s->pillSize);
    }
  }
  
  Serial.println(String('=', 60) + "\n");
}

String ScheduleManager::getNextScheduleTime() {
  // Find the next upcoming schedule
  int currentHour = hour();
  int currentMinute = minute();
  int currentTime = currentHour * 60 + currentMinute;
  
  int nextTime = 24 * 60; // Max minutes in a day
  String nextSchedule = "None";
  
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].enabled && isTodayScheduled(i)) {
      int schedTime = schedules[i].hour * 60 + schedules[i].minute;
      if (schedTime > currentTime && schedTime < nextTime) {
        nextTime = schedTime;
        nextSchedule = String(schedules[i].hour) + ":" + 
                      (schedules[i].minute < 10 ? "0" : "") + 
                      String(schedules[i].minute);
      }
    }
  }
  
  return nextSchedule;
}

bool ScheduleManager::isScheduleTime(int hour, int minute) {
  for (int i = 0; i < scheduleCount; i++) {
    if (schedules[i].enabled && 
        schedules[i].hour == hour && 
        schedules[i].minute == minute &&
        isTodayScheduled(i)) {
      return true;
    }
  }
  return false;
}

void ScheduleManager::testTriggerSchedule(int scheduleIndex) {
  Serial.println("\n" + String('=', 60));
  Serial.println("🧪 MANUAL SCHEDULE TRIGGER TEST - Schedule Index: " + String(scheduleIndex));
  Serial.println(String('=', 60));
  
  if (scheduleIndex < 0 || scheduleIndex >= scheduleCount) {
    Serial.println("❌ Invalid schedule index: " + String(scheduleIndex));
    return;
  }
  
  MedicationSchedule* schedule = &schedules[scheduleIndex];
  Serial.printf("Schedule ID: %s\n", schedule->id.c_str());
  Serial.printf("Time: %02d:%02d\n", schedule->hour, schedule->minute);
  Serial.println("Patient: " + schedule->patientName);
  Serial.println("Medication: " + schedule->medicationName);
  Serial.println("Dispenser: " + String(schedule->dispenserId));
  Serial.println("Size: " + schedule->pillSize);
  Serial.println("Enabled: " + String(schedule->enabled ? "YES" : "NO"));
  
  // Check if today is scheduled
  bool todayScheduled = isTodayScheduled(scheduleIndex);
  Serial.println("Today scheduled: " + String(todayScheduled ? "YES" : "NO"));
  
  Serial.println("Triggering dispense callback...");
  Serial.println(String('=', 60) + "\n");
  
  // Trigger the callback regardless of time/day checks for testing
  if (onDispenseCallback != nullptr) {
    onDispenseCallback(schedule->dispenserId, schedule->pillSize, 
                      schedule->medicationName, schedule->patientName);
  } else {
    Serial.println("❌ No dispense callback set!");
  }
}

// Static callback functions
OnTick_t ScheduleManager::getCallbackFunction(int index) {
  switch(index) {
    case 0: return alarmCallback0;
    case 1: return alarmCallback1;
    case 2: return alarmCallback2;
    case 3: return alarmCallback3;
    case 4: return alarmCallback4;
    case 5: return alarmCallback5;
    case 6: return alarmCallback6;
    case 7: return alarmCallback7;
    case 8: return alarmCallback8;
    case 9: return alarmCallback9;
    case 10: return alarmCallback10;
    case 11: return alarmCallback11;
    case 12: return alarmCallback12;
    case 13: return alarmCallback13;
    case 14: return alarmCallback14;
    default: return nullptr;
  }
}

void ScheduleManager::alarmCallback0() { if(instance) instance->triggerSchedule(0); }
void ScheduleManager::alarmCallback1() { if(instance) instance->triggerSchedule(1); }
void ScheduleManager::alarmCallback2() { if(instance) instance->triggerSchedule(2); }
void ScheduleManager::alarmCallback3() { if(instance) instance->triggerSchedule(3); }
void ScheduleManager::alarmCallback4() { if(instance) instance->triggerSchedule(4); }
void ScheduleManager::alarmCallback5() { if(instance) instance->triggerSchedule(5); }
void ScheduleManager::alarmCallback6() { if(instance) instance->triggerSchedule(6); }
void ScheduleManager::alarmCallback7() { if(instance) instance->triggerSchedule(7); }
void ScheduleManager::alarmCallback8() { if(instance) instance->triggerSchedule(8); }
void ScheduleManager::alarmCallback9() { if(instance) instance->triggerSchedule(9); }
void ScheduleManager::alarmCallback10() { if(instance) instance->triggerSchedule(10); }
void ScheduleManager::alarmCallback11() { if(instance) instance->triggerSchedule(11); }
void ScheduleManager::alarmCallback12() { if(instance) instance->triggerSchedule(12); }
void ScheduleManager::alarmCallback13() { if(instance) instance->triggerSchedule(13); }
void ScheduleManager::alarmCallback14() { if(instance) instance->triggerSchedule(14); }
