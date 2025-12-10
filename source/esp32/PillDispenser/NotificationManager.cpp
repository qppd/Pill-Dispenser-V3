#include "NotificationManager.h"

NotificationManager::NotificationManager(SIM800L* sim800Module, TimeManager* timeMgr) {
  sim800 = sim800Module;
  timeManager = timeMgr;
  phoneCount = 0;
  notificationsEnabled = true;
  sendBeforeDispense = true;
  sendOnDispense = true;
  sendOnPillTaken = true;
  sendOnMissedDose = true;
  sendOnLowBattery = true;
  lastNotificationTime = 0;
  
  // Initialize phone numbers
  for (int i = 0; i < MAX_PHONE_NUMBERS; i++) {
    phoneNumbers[i].number = "";
    phoneNumbers[i].name = "";
    phoneNumbers[i].enabled = false;
  }
}

void NotificationManager::begin() {
  Serial.println("NotificationManager: Initialized");
  Serial.println("NotificationManager: Max phone numbers: " + String(MAX_PHONE_NUMBERS));
}

bool NotificationManager::addPhoneNumber(String number, String name) {
  if (phoneCount >= MAX_PHONE_NUMBERS) {
    Serial.println("NotificationManager: Max phone numbers reached");
    return false;
  }
  
  // Check if number already exists
  for (int i = 0; i < phoneCount; i++) {
    if (phoneNumbers[i].number == number) {
      Serial.println("NotificationManager: Phone number already exists");
      return false;
    }
  }
  
  phoneNumbers[phoneCount].number = number;
  phoneNumbers[phoneCount].name = name;
  phoneNumbers[phoneCount].enabled = true;
  phoneCount++;
  
  Serial.println("NotificationManager: Added " + name + " (" + number + ")");
  return true;
}

bool NotificationManager::removePhoneNumber(String number) {
  for (int i = 0; i < phoneCount; i++) {
    if (phoneNumbers[i].number == number) {
      // Shift remaining numbers
      for (int j = i; j < phoneCount - 1; j++) {
        phoneNumbers[j] = phoneNumbers[j + 1];
      }
      phoneCount--;
      Serial.println("NotificationManager: Removed " + number);
      return true;
    }
  }
  return false;
}

void NotificationManager::clearPhoneNumbers() {
  phoneCount = 0;
  Serial.println("NotificationManager: All phone numbers cleared");
}

int NotificationManager::getPhoneCount() {
  return phoneCount;
}

void NotificationManager::setNotificationsEnabled(bool enabled) {
  notificationsEnabled = enabled;
  Serial.println("NotificationManager: Notifications " + String(enabled ? "enabled" : "disabled"));
}

void NotificationManager::setBeforeDispenseEnabled(bool enabled) {
  sendBeforeDispense = enabled;
}

void NotificationManager::setOnDispenseEnabled(bool enabled) {
  sendOnDispense = enabled;
}

void NotificationManager::setOnPillTakenEnabled(bool enabled) {
  sendOnPillTaken = enabled;
}

void NotificationManager::setOnMissedDoseEnabled(bool enabled) {
  sendOnMissedDose = enabled;
}

void NotificationManager::setOnLowBatteryEnabled(bool enabled) {
  sendOnLowBattery = enabled;
}

bool NotificationManager::isReady() {
  return (sim800 != nullptr && sim800->isReady() && phoneCount > 0);
}

bool NotificationManager::canSendNow() {
  return (millis() - lastNotificationTime >= NOTIFICATION_COOLDOWN);
}

String NotificationManager::formatBeforeDispenseMessage(String patientName, String medicationName, String time) {
  String message = "PILL REMINDER\n";
  message += "Patient: " + patientName + "\n";
  message += "Medication: " + medicationName + "\n";
  message += "Scheduled: " + time + "\n";
  message += "Time remaining: 30 minutes\n";
  message += "Please be ready to take your medication.";
  return message;
}

String NotificationManager::formatDispenseMessage(String patientName, String medicationName, String time) {
  String message = "MEDICATION DISPENSED\n";
  message += "Patient: " + patientName + "\n";
  message += "Medication: " + medicationName + "\n";
  message += "Time: " + time + "\n";
  message += "Please take your medication now.";
  return message;
}

String NotificationManager::formatPillTakenMessage(String patientName, String medicationName, String time) {
  String message = "MEDICATION CONFIRMED\n";
  message += "Patient: " + patientName + "\n";
  message += "Medication: " + medicationName + "\n";
  message += "Taken at: " + time + "\n";
  message += "Thank you for taking your medication on time.";
  return message;
}

String NotificationManager::formatMissedDoseMessage(String patientName, String medicationName, String scheduledTime) {
  String message = "MISSED DOSE ALERT\n";
  message += "Patient: " + patientName + "\n";
  message += "Medication: " + medicationName + "\n";
  message += "Scheduled: " + scheduledTime + "\n";
  message += "Status: NOT TAKEN\n";
  message += "Please contact patient immediately.";
  return message;
}

String NotificationManager::formatLowBatteryMessage(float batteryPercent) {
  String message = "LOW BATTERY WARNING\n";
  message += "Battery Level: " + String(batteryPercent, 1) + "%\n";
  message += "System Time: " + timeManager->getDateTimeString() + "\n";
  message += "Please charge the dispenser soon to avoid interruption.";
  return message;
}

String NotificationManager::formatSystemErrorMessage(String errorDescription) {
  String message = "SYSTEM ERROR\n";
  message += "Error: " + errorDescription + "\n";
  message += "Time: " + timeManager->getDateTimeString() + "\n";
  message += "Please check the dispenser system.";
  return message;
}

bool NotificationManager::notifyBeforeDispense(String patientName, String medicationName, String scheduleTime) {
  if (!notificationsEnabled || !sendBeforeDispense) {
    return false;
  }
  
  String message = formatBeforeDispenseMessage(patientName, medicationName, scheduleTime);
  return sendSMSToAll(message);
}

bool NotificationManager::notifyOnDispense(String patientName, String medicationName) {
  if (!notificationsEnabled || !sendOnDispense) {
    return false;
  }
  
  String currentTime = timeManager->getDateTimeString();
  String message = formatDispenseMessage(patientName, medicationName, currentTime);
  return sendSMSToAll(message);
}

bool NotificationManager::notifyPillTaken(String patientName, String medicationName) {
  if (!notificationsEnabled || !sendOnPillTaken) {
    return false;
  }
  
  String currentTime = timeManager->getDateTimeString();
  String message = formatPillTakenMessage(patientName, medicationName, currentTime);
  return sendSMSToAll(message);
}

bool NotificationManager::notifyMissedDose(String patientName, String medicationName, String scheduledTime) {
  if (!notificationsEnabled || !sendOnMissedDose) {
    return false;
  }
  
  String message = formatMissedDoseMessage(patientName, medicationName, scheduledTime);
  return sendSMSToAll(message);
}

bool NotificationManager::notifyLowBattery(float batteryPercent) {
  if (!notificationsEnabled || !sendOnLowBattery) {
    return false;
  }
  
  String message = formatLowBatteryMessage(batteryPercent);
  return sendSMSToAll(message);
}

bool NotificationManager::notifySystemError(String errorDescription) {
  if (!notificationsEnabled) {
    return false;
  }
  
  String message = formatSystemErrorMessage(errorDescription);
  return sendSMSToAll(message);
}

bool NotificationManager::sendSMSToAll(String message) {
  if (!isReady()) {
    Serial.println("NotificationManager: Cannot send SMS - not ready");
    return false;
  }
  
  if (!canSendNow()) {
    Serial.println("NotificationManager: Cooldown active, skipping SMS");
    return false;
  }
  
  bool allSuccess = true;
  int sentCount = 0;
  
  Serial.println("\n" + String('=', 50));
  Serial.println("📱 SENDING SMS NOTIFICATIONS");
  Serial.println(String('=', 50));
  Serial.println("Message:");
  Serial.println(message);
  Serial.println(String('-', 50));
  
  for (int i = 0; i < phoneCount; i++) {
    if (phoneNumbers[i].enabled) {
      Serial.print("Sending to " + phoneNumbers[i].name + " (" + phoneNumbers[i].number + ")... ");
      
      if (sim800->sendSMS(phoneNumbers[i].number, message)) {
        Serial.println("✅ Sent");
        sentCount++;
      } else {
        Serial.println("❌ Failed");
        allSuccess = false;
      }
      
      delay(2000);  // Delay between SMS sends
    }
  }
  
  Serial.println(String('=', 50));
  Serial.println("Sent " + String(sentCount) + " / " + String(phoneCount) + " messages");
  Serial.println(String('=', 50) + "\n");
  
  lastNotificationTime = millis();
  return allSuccess;
}

void NotificationManager::printConfig() {
  Serial.println("\n" + String('=', 50));
  Serial.println("📱 NOTIFICATION CONFIGURATION");
  Serial.println(String('=', 50));
  Serial.println("Enabled: " + String(notificationsEnabled ? "YES" : "NO"));
  Serial.println("Phone Numbers: " + String(phoneCount));
  
  for (int i = 0; i < phoneCount; i++) {
    Serial.println("  " + String(i + 1) + ". " + phoneNumbers[i].name + 
                  " - " + phoneNumbers[i].number +
                  " [" + String(phoneNumbers[i].enabled ? "Active" : "Disabled") + "]");
  }
  
  Serial.println("\nNotification Types:");
  Serial.println("  Before Dispense (30min): " + String(sendBeforeDispense ? "ON" : "OFF"));
  Serial.println("  On Dispense: " + String(sendOnDispense ? "ON" : "OFF"));
  Serial.println("  Pill Taken: " + String(sendOnPillTaken ? "ON" : "OFF"));
  Serial.println("  Missed Dose: " + String(sendOnMissedDose ? "ON" : "OFF"));
  Serial.println("  Low Battery: " + String(sendOnLowBattery ? "ON" : "OFF"));
  Serial.println(String('=', 50) + "\n");
}
