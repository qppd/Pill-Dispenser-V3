# Schedule Implementation - Pill Dispenser V3

## Technical Architecture

### Schedule Data Structure

#### Firebase Storage Format
```json
{
  "schedules": {
    "schedule_001": {
      "dispenserId": 0,
      "hour": 8,
      "minute": 30,
      "enabled": true,
      "medicationName": "Lisinopril",
      "patientName": "Jane Smith",
      "pillSize": "medium",
      "days": [1, 2, 3, 4, 5, 6, 7],
      "created": 1703123456789,
      "lastModified": 1703123456789
    }
  }
}
```

#### ESP32 Memory Structure
```cpp
struct ScheduleData {
    int dispenserId;
    int hour;
    int minute;
    bool enabled;
    String medicationName;
    String patientName;
    String pillSize;
    std::vector<int> days;
    time_t created;
    time_t lastModified;
};
```

### TimeAlarms Integration

#### Schedule Registration
```cpp
// Register alarm callback
Alarm.alarmRepeat(hour, minute, dispenseCallback);

// Callback function
void dispenseCallback() {
    int scheduleId = getCurrentScheduleId();
    servoDriver.dispensePill(scheduleId, pillSize);
    firebaseManager.logDispenseEvent(scheduleId, medication, patient);
    notificationManager.sendDispenseNotification(scheduleId, medication, patient);
}
```

#### Alarm Management
- **Maximum Alarms**: 15 concurrent TimeAlarms
- **Callback System**: Individual function per schedule
- **Time Synchronization**: NTP-based accurate timing
- **Memory Efficiency**: Minimal RAM usage

## Schedule Synchronization

### Firebase Listener Implementation

#### Real-time Database Listener
```cpp
void setupFirebaseListener() {
    firebase.on("schedules", FirebaseEventType::PUT, [](const String& path, const String& data) {
        parseScheduleData(data);
        updateLocalSchedules();
        syncSchedulesToAlarms();
    });
}
```

#### Sync Process Flow
1. **Web Dashboard**: User modifies schedule
2. **Firebase Update**: Data written to database
3. **ESP32 Trigger**: onValueChanged callback fires
4. **Data Parsing**: JSON data converted to ScheduleData
5. **Local Update**: Internal schedule array updated
6. **Alarm Sync**: TimeAlarms updated with new schedules
7. **Confirmation**: Success logged and displayed

### Conflict Resolution

#### Last-Write-Wins Strategy
- **Timestamp Comparison**: Most recent modification wins
- **Version Control**: Firebase server timestamps used
- **Rollback Support**: Previous versions maintained
- **Audit Trail**: All changes logged with timestamps

## Pill Dispensing Logic

### Size-Based Timing

#### Timing Constants
```cpp
const int SMALL_PILL_TIME = 800;   // milliseconds
const int MEDIUM_PILL_TIME = 1000; // milliseconds
const int LARGE_PILL_TIME = 1200;  // milliseconds
```

#### Dispensing Algorithm
```cpp
void dispensePill(int dispenserId, String pillSize) {
    int channel = dispenserId;
    int duration = getPillDuration(pillSize);

    pwm.setPWM(channel, 0, SERVO_FORWARD);
    delay(duration);
    pwm.setPWM(channel, 0, SERVO_STOP);

    logDispenseEvent(dispenserId, pillSize);
}
```

### Multi-Dispenser Coordination

#### Sequential Dispensing
- **Single Dispenser**: One container at a time
- **Queue Management**: FIFO dispensing order
- **Conflict Prevention**: No simultaneous dispensing
- **Status Tracking**: Busy/free state monitoring

## Notification System Integration

### SMS Notification Types

#### Pre-Dispense Reminder (30 minutes before)
```cpp
void sendReminder(int dispenserId, String medication, String patient, int minutesUntil) {
    String message = "Reminder: " + medication + " for " + patient + " in " + minutesUntil + " minutes";
    sim800l.sendSMS(caregiverNumber, message);
}
```

#### Dispense Confirmation
```cpp
void sendDispenseNotification(int dispenserId, String medication, String patient) {
    String message = "Dispensed: " + medication + " for " + patient + " at " + getCurrentTimeString();
    sim800l.sendSMS(caregiverNumber, message);
}
```

#### Post-Dispense Completion
```cpp
void sendCompletionNotification(int dispenserId, String medication, String patient) {
    String message = "Completed: " + medication + " administration for " + patient;
    sim800l.sendSMS(caregiverNumber, message);
}
```

### Notification Scheduling

#### Time-Based Triggers
- **Reminder Timer**: Set 30 minutes before schedule
- **Dispense Timer**: Triggered at exact schedule time
- **Completion Timer**: Set after successful dispensing
- **Missed Dose Timer**: Alert for missed schedules

## Error Handling and Recovery

### Schedule Validation

#### Data Integrity Checks
```cpp
bool validateSchedule(ScheduleData schedule) {
    if (schedule.dispenserId < 0 || schedule.dispenserId > 4) return false;
    if (schedule.hour < 0 || schedule.hour > 23) return false;
    if (schedule.minute < 0 || schedule.minute > 59) return false;
    if (schedule.days.empty()) return false;
    return true;
}
```

#### Error Recovery
- **Invalid Data**: Skip and log error
- **Sync Failure**: Retry with exponential backoff
- **Time Error**: Re-sync NTP time
- **Memory Full**: Remove oldest schedules

### Logging and Monitoring

#### Firebase Event Logging
```cpp
void logScheduleEvent(String eventType, int scheduleId, String details) {
    FirebaseJson json;
    json.add("timestamp", getCurrentTimestamp());
    json.add("eventType", eventType);
    json.add("scheduleId", scheduleId);
    json.add("details", details);

    firebase.push("/logs/schedules", json);
}
```

#### System Health Monitoring
- **Schedule Count**: Track active schedules
- **Sync Status**: Monitor Firebase connectivity
- **Time Accuracy**: Verify NTP synchronization
- **Memory Usage**: Track heap allocation

## Performance Optimization

### Memory Management

#### Efficient Data Structures
- **Fixed Arrays**: Pre-allocated schedule storage
- **String Pooling**: Shared string references
- **Garbage Collection**: Periodic memory cleanup
- **Heap Monitoring**: Usage tracking and alerts

### Network Optimization

#### Firebase Polling Strategy
- **Real-time Updates**: Event-driven synchronization
- **Batch Operations**: Multiple schedule updates together
- **Compression**: Minimize data transfer
- **Caching**: Local schedule cache for offline operation

### CPU Optimization

#### Interrupt Handling
- **Timer Interrupts**: Efficient alarm processing
- **Background Tasks**: Non-blocking operations
- **Priority Scheduling**: Critical tasks first
- **Sleep Modes**: Power-saving when idle

## Testing and Validation

### Unit Testing

#### Schedule Manager Tests
```cpp
void testScheduleManager() {
    // Test schedule creation
    ScheduleData testSchedule = createTestSchedule();
    assert(scheduleManager.addSchedule(testSchedule));

    // Test alarm registration
    assert(Alarm.count() > 0);

    // Test dispensing trigger
    triggerTestAlarm();
    assert(dispenseLog.contains(testSchedule.medicationName));
}
```

### Integration Testing

#### End-to-End Schedule Flow
1. **Create Schedule**: Via web dashboard
2. **Verify Sync**: Check ESP32 serial output
3. **Wait for Trigger**: Allow scheduled time to arrive
4. **Confirm Dispensing**: Verify servo activation
5. **Check Notifications**: Validate SMS delivery
6. **Review Logs**: Confirm Firebase logging

### Load Testing

#### Maximum Schedule Capacity
- **15 Schedules**: Test all TimeAlarm slots
- **Concurrent Dispensing**: Multiple schedules triggering
- **Memory Stress**: Monitor heap usage under load
- **Network Load**: Firebase synchronization stress test

## Security Considerations

### Data Protection

#### Firebase Security Rules
```json
{
  "rules": {
    "schedules": {
      ".read": "auth != null",
      ".write": "auth != null",
      ".validate": "newData.hasChildren(['dispenserId', 'hour', 'minute', 'enabled'])"
    }
  }
}
```

#### Access Control
- **Authentication Required**: Firebase auth for all operations
- **User Isolation**: Separate data per user/device
- **Audit Logging**: All schedule changes tracked
- **Encryption**: Data encrypted in transit and at rest

## Future Enhancements

### Advanced Features

#### Smart Scheduling
- **Medication Interactions**: Check for conflicts
- **Dosage Optimization**: Adjust based on patient response
- **Predictive Analytics**: Anticipate refill needs
- **Integration APIs**: Connect with pharmacy systems

#### Enhanced Notifications
- **Multi-Channel**: Email, app notifications
- **Escalation**: Multiple caregiver contacts
- **Custom Messages**: Personalized notifications
- **Language Support**: Multi-language SMS

#### Machine Learning Integration
- **Adherence Prediction**: ML-based compliance forecasting
- **Optimal Timing**: AI-suggested medication times
- **Pattern Recognition**: Abnormal usage detection
- **Automated Adjustments**: Self-optimizing schedules

---

**Schedule Implementation**: Robust, scalable medication scheduling system with real-time synchronization and comprehensive error handling.