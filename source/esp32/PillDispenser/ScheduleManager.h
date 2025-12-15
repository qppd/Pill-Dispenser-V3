#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H

#include <Arduino.h>
#include <TimeAlarms.h>
#include <Firebase_ESP_Client.h>
#include "TimeManager.h"

#define MAX_SCHEDULES 15  // Maximum number of schedules (3 per dispenser x 5 dispensers)

struct MedicationSchedule {
  String id;
  int dispenserId;        // 0-4 (which of the 5 dispensers)
  int hour;               // 0-23
  int minute;             // 0-59
  bool enabled;
  String medicationName;
  String patientName;
  String pillSize;        // "small", "medium", "large"
  AlarmId alarmId;        // TimeAlarms library alarm ID for dispense
  AlarmId reminderAlarmId; // TimeAlarms library alarm ID for 15-min reminder
  bool weekdays[7];       // Monday=0, Sunday=6
};

class ScheduleManager {
private:
  MedicationSchedule schedules[MAX_SCHEDULES];
  int scheduleCount;
  String deviceId;
  TimeManager* timeManager;
  
  // Callback function pointers
  void (*onDispenseCallback)(int dispenserId, String pillSize, String medication, String patient);
  void (*onReminderCallback)(int dispenserId, String pillSize, String medication, String patient);
  void (*onNotifyCallback)(String message, String phone);
  
  // Alarm callbacks must be static
  static ScheduleManager* instance;
  static void alarmCallback0();
  static void alarmCallback1();
  static void alarmCallback2();
  static void alarmCallback3();
  static void alarmCallback4();
  static void alarmCallback5();
  static void alarmCallback6();
  static void alarmCallback7();
  static void alarmCallback8();
  static void alarmCallback9();
  static void alarmCallback10();
  static void alarmCallback11();
  static void alarmCallback12();
  static void alarmCallback13();
  static void alarmCallback14();
  
  // Reminder alarm callbacks (15 minutes before)
  static void reminderCallback0();
  static void reminderCallback1();
  static void reminderCallback2();
  static void reminderCallback3();
  static void reminderCallback4();
  static void reminderCallback5();
  static void reminderCallback6();
  static void reminderCallback7();
  static void reminderCallback8();
  static void reminderCallback9();
  static void reminderCallback10();
  static void reminderCallback11();
  static void reminderCallback12();
  static void reminderCallback13();
  static void reminderCallback14();
  
  // Helper functions
  void triggerSchedule(int scheduleIndex);
  void triggerReminder(int scheduleIndex);
  OnTick_t getCallbackFunction(int index);
  OnTick_t getReminderCallbackFunction(int index);
  bool isTodayScheduled(int scheduleIndex);
  void calculateReminderTime(int hour, int minute, int& reminderHour, int& reminderMinute);
  
public:
  ScheduleManager();
  void begin(String deviceId);
  void setTimeManager(TimeManager* tm) { timeManager = tm; }
  void update();  // Call in loop() to process alarms
  
  // Schedule management
  bool addSchedule(String id, int dispenserId, int hour, int minute, 
                   String medicationName, String patientName, 
                   String pillSize = "medium", bool enabled = true);
  bool removeSchedule(String id);
  bool updateSchedule(String id, int hour, int minute, bool enabled);
  void clearAllSchedules();
  int getScheduleCount();
  MedicationSchedule* getSchedule(int index);
  MedicationSchedule* getScheduleById(String id);
  
  // Firebase integration
  bool syncSchedulesFromFirebase(FirebaseData* fbdo, String basePath);
  bool uploadScheduleStatus(FirebaseData* fbdo, String basePath, String scheduleId, String status);
  
  // Callbacks
  void setDispenseCallback(void (*callback)(int, String, String, String));
  void setReminderCallback(void (*callback)(int, String, String, String));
  void setNotifyCallback(void (*callback)(String, String));
  
  // Utilities
  void printSchedules();
  String getNextScheduleTime();
  int getActiveScheduleCount();
  bool isScheduleTime(int hour, int minute);
  void testTriggerSchedule(int scheduleIndex); // Manual trigger for testing
};

#endif
