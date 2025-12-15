#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <Arduino.h>

// User Configuration File
// This file contains user-specific settings like phone numbers, user IDs, etc.

// Phone Numbers for SMS Notifications
const String CAREGIVER_1_PHONE = "+639510198338";  // Primary caregiver phone number
const String CAREGIVER_2_PHONE = "+639634905586";  // Secondary caregiver phone number (optional)

// Caregiver Names
const String CAREGIVER_1_NAME = "Pill Dispenser Caregiver 1";  // Name for primary caregiver
const String CAREGIVER_2_NAME = "Pill Dispenser Caregiver 2";  // Name for secondary caregiver

// User ID for Firebase
const String USER_ID = "MmRwYAQXJiZcA7jgvkjCOw0m5Uz2";  // Firebase auth user ID

// Device Configuration
const String DEVICE_NAME = "PillDispenser_V3";

// Emergency Contact (used for system error notifications)
const String EMERGENCY_PHONE = CAREGIVER_1_PHONE;

#endif // USER_CONFIG_H