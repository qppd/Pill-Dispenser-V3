# Schedule Management Guide

## Overview
The Pill Dispenser V3 now supports complete schedule management with real-time synchronization between the web app and ESP32 device. You can add, delete, and manage medication schedules that automatically dispense pills at the scheduled times.

## Features

### ✅ Web App Schedule Management
- **Add Schedules**: Up to 3 schedules per container (5 containers = 15 total schedules)
- **Delete Schedules**: Remove individual schedules as needed
- **Enable/Disable**: Toggle schedules on/off without deleting them
- **Real-time Sync**: Changes are immediately sent to the ESP32 device

### ✅ ESP32 Automatic Dispensing
- **Time-based Triggers**: Automatically dispenses pills at scheduled times
- **Schedule Sync**: Loads schedules from Firebase on startup and when updated
- **Manual Override**: "Dispense Now" button works independently of schedules
- **Status Logging**: All dispenses are logged to Firebase with timestamps

## How It Works

### Web App (Frontend)
1. User logs in to the web application
2. Navigates to the **Schedule** page
3. Selects a container (1-5)
4. Adds schedules with specific times
5. Enables/disables schedules as needed
6. Clicks **Save Changes**

### Firebase (Backend)
- Schedules are saved to two locations:
  - `schedules/{userId}` - User's personal schedule data
  - `pilldispenser/device/schedules/{userId}` - Device-readable format

### ESP32 (Device)
1. On startup, loads schedules from Firebase
2. Creates time-based alarms for each enabled schedule
3. Listens for schedule updates via Firebase Realtime Database streaming
4. When schedule time arrives:
   - Triggers the scheduled dispense
   - Displays info on LCD
   - Rotates servo to dispense pill
   - Sends SMS notification (if configured)
   - Logs dispense event to Firebase

## Usage Instructions

### Setting Up Schedules (Web App)

1. **Login to the Web Application**
   ```
   Navigate to: http://localhost:3000/login
   ```

2. **Go to Schedule Page**
   ```
   Click "Schedule" in the navigation menu
   ```

3. **Select a Container**
   - Click on any container (1-5) from the left sidebar
   - The selected container will highlight

4. **Add a Schedule**
   - Click the "Add Schedule" button
   - Set the time using the time picker
   - Toggle the "Enabled" checkbox
   - Maximum 3 schedules per container

5. **Delete a Schedule**
   - Click the trash icon (🗑️) next to any schedule
   - The schedule is removed immediately

6. **Save Changes**
   - Click the "Save Changes" button
   - Schedules are saved to Firebase
   - ESP32 device will sync automatically

### Schedule Data Format

#### Firebase Structure
```json
{
  "pilldispenser/device/schedules/{userId}": {
    "1_0": {
      "dispenserId": 0,
      "hour": 8,
      "minute": 0,
      "enabled": true,
      "medicationName": "Container 1 Medication",
      "patientName": "user@example.com",
      "pillSize": "medium"
    },
    "2_1": {
      "dispenserId": 1,
      "hour": 14,
      "minute": 30,
      "enabled": true,
      "medicationName": "Container 2 Medication",
      "patientName": "user@example.com",
      "pillSize": "medium"
    }
  }
}
```

### ESP32 Configuration

#### Required Setup
```cpp
// In PillDispenser.ino
const String USER_ID = "your_user_id_here";  // Set to your Firebase user ID
```

#### Schedule Manager Integration
The ESP32 code automatically:
- Initializes the ScheduleManager on startup
- Links it with FirebaseManager
- Syncs schedules from Firebase
- Sets up dispense callbacks
- Updates schedules when changed in web app

### Testing the Schedule System

#### Test 1: Add a Schedule
1. Add a schedule for 2 minutes from now
2. Enable the schedule
3. Save changes
4. Wait for the scheduled time
5. Verify ESP32 dispenses automatically

#### Test 2: Delete a Schedule
1. Add a test schedule
2. Save changes
3. Delete the schedule
4. Save again
5. Verify schedule no longer triggers

#### Test 3: Real-time Sync
1. Open ESP32 serial monitor
2. Add/modify a schedule in web app
3. Save changes
4. Watch serial monitor for sync messages
5. Verify schedules are updated on ESP32

## ESP32 Serial Monitor Output

### Successful Schedule Load
```
📅 Loading schedules from Firebase...
FirebaseManager: Syncing schedules from Firebase...
Added schedule: 1_0 - 08:00 for dispenser 0
Added schedule: 2_1 - 14:30 for dispenser 1
FirebaseManager: Schedule sync complete - 2 schedules loaded

============================================================
📋 ACTIVE SCHEDULES (2 / 2)
============================================================
 1. ✅ 08:00 | Dispenser 0 | Container 1 Medication
    Patient: user@example.com | Size: medium
 2. ✅ 14:30 | Dispenser 1 | Container 2 Medication
    Patient: user@example.com | Size: medium
============================================================
```

### Scheduled Dispense Trigger
```
============================================================
⏰ SCHEDULED DISPENSE TRIGGERED
============================================================
Container: 1
Medication: Container 1 Medication
Patient: user@example.com
Pill Size: medium
Time: 08:00:00
============================================================
🔄 Dispensing from container 1...
✅ Dispense complete. Total pills dispensed: 1
✅ Scheduled dispense completed
```

### Real-time Schedule Update
```
FirebaseManager: Updated Path: /pill_schedule
FirebaseManager: New Value: ...
FirebaseManager: Schedule updated: ...
FirebaseManager: Triggering schedule sync due to update...
FirebaseManager: Syncing schedules from Firebase...
```

## Troubleshooting

### Schedules Not Syncing
1. **Check WiFi Connection**
   - ESP32 must be connected to WiFi
   - Firebase requires internet access

2. **Verify User ID**
   - Ensure USER_ID in ESP32 code matches your Firebase user
   - Check Firebase console for correct path

3. **Check Firebase Rules**
   - Read/write permissions must be set correctly
   - Test with Firebase console directly

### Schedules Not Triggering
1. **Time Synchronization**
   - ESP32 must have correct time from NTP
   - Check serial monitor for current time

2. **Schedule Status**
   - Verify schedules are enabled
   - Check schedule count in serial monitor

3. **Alarm System**
   - ScheduleManager uses TimeAlarms library
   - Maximum 15 alarms (3 per container × 5 containers)

### Manual Dispense Still Works
- The "Dispense Now" button is independent
- It sends a real-time command via Firebase
- Works even if schedules are not configured

## Advanced Configuration

### Customizing Medication Names
Edit the schedule save function in `schedule/page.tsx`:
```typescript
medicationName: `Custom Medication Name`,
```

### Adjusting Dispense Duration
Edit the dispense function in `PillDispenser.ino`:
```cpp
delay(1000);  // Adjust time servo stays at 90°
```

### SMS Notifications
Replace the phone number in `handleScheduledDispense()`:
```cpp
sim800.sendSMS("+1234567890", smsMessage);  // Your phone number
```

### Schedule Sync Interval
Adjust in `FirebaseManager.h`:
```cpp
static const unsigned long SCHEDULE_SYNC_INTERVAL = 60000; // 1 minute
```

## API Reference

### Web App Functions
- `addSchedule(dispenserId)` - Add new schedule to container
- `removeSchedule(dispenserId, index)` - Remove schedule
- `updateScheduleTime(dispenserId, index, time)` - Update time
- `toggleSchedule(dispenserId, index)` - Enable/disable
- `saveSchedules()` - Save to Firebase

### ESP32 Functions
- `scheduleManager.begin(deviceId)` - Initialize manager
- `scheduleManager.addSchedule(...)` - Add schedule
- `scheduleManager.removeSchedule(id)` - Remove schedule
- `scheduleManager.update()` - Check alarms (call in loop)
- `firebase.syncSchedulesFromFirebase()` - Sync from cloud

## Future Enhancements
- [ ] Multi-user support with proper authentication
- [ ] Custom medication names and dosages
- [ ] Weekly schedule patterns (different times per day)
- [ ] Push notifications via mobile app
- [ ] Schedule history and compliance tracking
- [ ] Voice reminders via speaker
- [ ] Missed dose alerts

## Support
For issues or questions:
1. Check the serial monitor output
2. Verify Firebase console data
3. Review the implementation files
4. Open an issue on GitHub

---
**Last Updated**: December 11, 2025
**Version**: 3.0
