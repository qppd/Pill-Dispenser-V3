#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <Arduino.h>
#include "SIM800L.h"
#include "TimeManager.h"

#define MAX_PHONE_NUMBERS 3

enum NotificationType {
  NOTIFY_BEFORE_DISPENSE,   // 30 minutes before
  NOTIFY_ON_DISPENSE,        // When dispensing occurs
  NOTIFY_PILL_TAKEN,         // When pill detected as taken
  NOTIFY_MISSED_DOSE,        // When schedule missed
  NOTIFY_LOW_BATTERY,        // Battery low warning
  NOTIFY_SYSTEM_ERROR        // System errors
};

struct PhoneNumber {
  String number;
  String name;
  bool enabled;
};

class NotificationManager {
private:
  SIM800L* sim800;
  TimeManager* timeManager;
  PhoneNumber phoneNumbers[MAX_PHONE_NUMBERS];
  int phoneCount;
  
  bool notificationsEnabled;
  bool sendBeforeDispense;
  bool sendOnDispense;
  bool sendOnPillTaken;
  bool sendOnMissedDose;
  bool sendOnLowBattery;
  
  unsigned long lastNotificationTime;
  static const unsigned long NOTIFICATION_COOLDOWN = 30000;  // 30 seconds between SMS
  
  // Formatting helpers
  String formatDispenseMessage(String patientName, String medicationName, String time);
  String formatBeforeDispenseMessage(String patientName, String medicationName, String time);
  String formatPillTakenMessage(String patientName, String medicationName, String time);
  String formatMissedDoseMessage(String patientName, String medicationName, String scheduledTime);
  String formatLowBatteryMessage(float batteryPercent);
  String formatSystemErrorMessage(String errorDescription);
  
public:
  NotificationManager(SIM800L* sim800Module, TimeManager* timeMgr);
  void begin();
  
  // Phone number management
  bool addPhoneNumber(String number, String name);
  bool removePhoneNumber(String number);
  void clearPhoneNumbers();
  int getPhoneCount();
  
  // Notification settings
  void setNotificationsEnabled(bool enabled);
  void setBeforeDispenseEnabled(bool enabled);
  void setOnDispenseEnabled(bool enabled);
  void setOnPillTakenEnabled(bool enabled);
  void setOnMissedDoseEnabled(bool enabled);
  void setOnLowBatteryEnabled(bool enabled);
  
  // Send notifications
  bool notifyBeforeDispense(String patientName, String medicationName, String scheduleTime);
  bool notifyOnDispense(String patientName, String medicationName);
  bool notifyPillTaken(String patientName, String medicationName);
  bool notifyMissedDose(String patientName, String medicationName, String scheduledTime);
  bool notifyLowBattery(float batteryPercent);
  bool notifySystemError(String errorDescription);
  
  // Generic send function
  bool sendNotification(NotificationType type, String message);
  bool sendSMSToAll(String message);
  
  // Utilities
  bool isReady();
  bool canSendNow();
  void printConfig();
};

#endif
