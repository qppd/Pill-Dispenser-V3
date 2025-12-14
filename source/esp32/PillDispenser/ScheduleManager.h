#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H

#include <Arduino.h>
#include <TimeAlarms.h>
#include "TimeManager.h"

#define MAX_SCHEDULES 15

struct MedicationSchedule {
  String id;
  int dispenserId;
  int hour;
  int minute;
  bool enabled;
  String medicationName;
  String patientName;
  String pillSize;
  AlarmId alarmId;
  bool weekdays[7];
};

class ScheduleManager {
private:
  MedicationSchedule schedules[MAX_SCHEDULES];
  int scheduleCount;
  String deviceId;
  TimeManager* timeManager;
  void (*onDispenseCallback)(int, String, String, String);
  
  static ScheduleManager* instance;
  static void alarmCallback0() { if(instance) instance->triggerSchedule(0); }
  static void alarmCallback1() { if(instance) instance->triggerSchedule(1); }
  static void alarmCallback2() { if(instance) instance->triggerSchedule(2); }
  static void alarmCallback3() { if(instance) instance->triggerSchedule(3); }
  static void alarmCallback4() { if(instance) instance->triggerSchedule(4); }
  static void alarmCallback5() { if(instance) instance->triggerSchedule(5); }
  static void alarmCallback6() { if(instance) instance->triggerSchedule(6); }
  static void alarmCallback7() { if(instance) instance->triggerSchedule(7); }
  static void alarmCallback8() { if(instance) instance->triggerSchedule(8); }
  static void alarmCallback9() { if(instance) instance->triggerSchedule(9); }
  static void alarmCallback10() { if(instance) instance->triggerSchedule(10); }
  static void alarmCallback11() { if(instance) instance->triggerSchedule(11); }
  static void alarmCallback12() { if(instance) instance->triggerSchedule(12); }
  static void alarmCallback13() { if(instance) instance->triggerSchedule(13); }
  static void alarmCallback14() { if(instance) instance->triggerSchedule(14); }
  
  OnTick_t getCallbackFunction(int index) {
    switch(index) {
      case 0: return alarmCallback0; case 1: return alarmCallback1;
      case 2: return alarmCallback2; case 3: return alarmCallback3;
      case 4: return alarmCallback4; case 5: return alarmCallback5;
      case 6: return alarmCallback6; case 7: return alarmCallback7;
      case 8: return alarmCallback8; case 9: return alarmCallback9;
      case 10: return alarmCallback10; case 11: return alarmCallback11;
      case 12: return alarmCallback12; case 13: return alarmCallback13;
      case 14: return alarmCallback14; default: return nullptr;
    }
  }
  
  void triggerSchedule(int scheduleIndex) {
    if (scheduleIndex < 0 || scheduleIndex >= scheduleCount) return;
    MedicationSchedule* schedule = &schedules[scheduleIndex];
    if (!schedule->enabled) return;
    Serial.printf("⏰ ALARM: %02d:%02d - %s\\n", schedule->hour, schedule->minute, schedule->medicationName.c_str());
    if (onDispenseCallback) {
      onDispenseCallback(schedule->dispenserId, schedule->pillSize, 
                        schedule->medicationName, schedule->patientName);
    }
  }
  
public:
  ScheduleManager() : scheduleCount(0), timeManager(nullptr), onDispenseCallback(nullptr) {
    instance = this;
    for (int i = 0; i < MAX_SCHEDULES; i++) {
      schedules[i].enabled = false;
      schedules[i].alarmId = dtINVALID_ALARM_ID;
      for (int j = 0; j < 7; j++) schedules[i].weekdays[j] = true;
    }
  }
  
  void begin(String devId) { deviceId = devId; Serial.println("ScheduleManager: Initialized"); }
  void setTimeManager(TimeManager* tm) { timeManager = tm; }
  void setDispenseCallback(void (*callback)(int, String, String, String)) { onDispenseCallback = callback; }
  void update() {}
  
  bool addSchedule(String id, int dispenserId, int hour, int minute, 
                   String medicationName, String patientName, 
                   String pillSize = "medium", bool enabled = true) {
    if (scheduleCount >= MAX_SCHEDULES || dispenserId < 0 || dispenserId > 4 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59) return false;
    
    for (int i = 0; i < scheduleCount; i++) {
      if (schedules[i].id == id) {
        if (schedules[i].alarmId != dtINVALID_ALARM_ID) Alarm.free(schedules[i].alarmId);
        schedules[i].dispenserId = dispenserId; schedules[i].hour = hour;
        schedules[i].minute = minute; schedules[i].enabled = enabled;
        schedules[i].medicationName = medicationName; schedules[i].patientName = patientName;
        schedules[i].pillSize = pillSize;
        if (enabled) {
          OnTick_t callback = getCallbackFunction(i);
          if (callback) {
            schedules[i].alarmId = Alarm.alarmRepeat(hour, minute, 0, callback);
          }
        }
        return true;
      }
    }
    
    int index = scheduleCount;
    schedules[index].id = id; schedules[index].dispenserId = dispenserId;
    schedules[index].hour = hour; schedules[index].minute = minute;
    schedules[index].enabled = enabled; schedules[index].medicationName = medicationName;
    schedules[index].patientName = patientName; schedules[index].pillSize = pillSize;
    
    if (enabled) {
      OnTick_t callback = getCallbackFunction(index);
      if (callback) {
        schedules[index].alarmId = Alarm.alarmRepeat(hour, minute, 0, callback);
      }
    }
    scheduleCount++;
    return true;
  }
  
  void clearAllSchedules() {
    for (int i = 0; i < scheduleCount; i++) {
      if (schedules[i].alarmId != dtINVALID_ALARM_ID) Alarm.free(schedules[i].alarmId);
    }
    scheduleCount = 0;
    Serial.println("ScheduleManager: Cleared");
  }
  
  void printSchedules() {
    Serial.println("\\n📋 SCHEDULES: " + String(scheduleCount));
    for (int i = 0; i < scheduleCount; i++) {
      MedicationSchedule* s = &schedules[i];
      Serial.printf("%d. %s %02d:%02d | D%d | %s\\n", i + 1, s->enabled ? "✅" : "❌", 
                   s->hour, s->minute, s->dispenserId, s->medicationName.c_str());
    }
  }
  
  String getNextScheduleTime() {
    int currentTime = hour() * 60 + minute();
    int nextTime = 24 * 60;
    String nextSchedule = "None";
    for (int i = 0; i < scheduleCount; i++) {
      if (schedules[i].enabled) {
        int schedTime = schedules[i].hour * 60 + schedules[i].minute;
        if (schedTime > currentTime && schedTime < nextTime) {
          nextTime = schedTime;
          nextSchedule = String(schedules[i].hour) + ":" + 
                        (schedules[i].minute < 10 ? "0" : "") + String(schedules[i].minute);
        }
      }
    }
    return nextSchedule;
  }
  
  void testTriggerSchedule(int scheduleIndex) {
    if (scheduleIndex < 0 || scheduleIndex >= scheduleCount) return;
    Serial.println("🧪 Test: " + String(scheduleIndex));
    triggerSchedule(scheduleIndex);
  }
  
  int getScheduleCount() { return scheduleCount; }
};

ScheduleManager* ScheduleManager::instance = nullptr;

#endif
