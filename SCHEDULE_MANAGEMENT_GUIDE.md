# Schedule Management Guide - Pill Dispenser V3

## Overview

The Pill Dispenser V3 supports comprehensive schedule management with:
- Up to 15 concurrent schedules (3 per dispenser × 5 dispensers)
- Day-of-week filtering
- Patient and medication tracking
- Real-time sync between web and ESP32

## Web Interface Features

### Schedule Management
- **Add Schedules**: Create new medication schedules
- **Delete Schedules**: Remove unwanted schedules
- **Enable/Disable**: Toggle schedule activation
- **Real-time Sync**: Changes immediately sent to ESP32

### User Interface
- **Container Selection**: Choose dispenser (0-4)
- **Time Configuration**: Set hour and minute
- **Patient Information**: Track medication recipients
- **Medication Details**: Record drug names and dosages
- **Pill Size Settings**: Small, medium, or large
- **Day Selection**: Choose active days of the week

## ESP32 Schedule System

### TimeAlarms Integration
- **Automatic Dispensing**: Scheduled medication delivery
- **Firebase Sync**: Loads schedules from cloud on startup
- **Manual Override**: "Dispense Now" button works independently
- **Status Logging**: All dispenses logged with timestamps

### Schedule Limits
- **Maximum Schedules**: 15 total (3 per dispenser)
- **Time Precision**: Minute-level accuracy
- **Day Filtering**: 7-day week support
- **Patient Tracking**: Multiple patients per device

## Creating Schedules

### Via Web Dashboard

#### Step-by-Step Process
1. **Login**: Access the web dashboard
2. **Navigate**: Go to Schedule page
3. **Select Container**: Choose dispenser (0-4)
4. **Add Schedule**: Click "Add Schedule" button
5. **Configure Details**:
   - Time (HH:MM format)
   - Patient name
   - Medication name
   - Pill size (small/medium/large)
   - Days of week (checkboxes)
   - Enable/disable toggle
6. **Save**: Click save to store and sync

#### Schedule Fields
- **Container**: Dispenser number (0-4)
- **Time**: 24-hour format (00:00 - 23:59)
- **Patient**: Recipient name (optional)
- **Medication**: Drug name and dosage
- **Pill Size**: Small/Medium/Large timing
- **Days**: Monday through Sunday selection
- **Status**: Enabled/Disabled state

### Via Serial Commands (Development)

#### Basic Commands
```
add schedule dispenser_id hour minute
enable schedule schedule_id
disable schedule schedule_id
delete schedule schedule_id
```

#### Advanced Commands
```
list schedules          # Show all active schedules
show schedule ID        # Display specific schedule details
edit schedule ID        # Modify existing schedule
clear schedules         # Remove all schedules
```

## Schedule Data Structure

### Firebase Format
```json
{
  "dispenserId": 0,
  "hour": 8,
  "minute": 0,
  "enabled": true,
  "medicationName": "Aspirin",
  "patientName": "John Doe",
  "pillSize": "medium",
  "days": [1, 2, 3, 4, 5]
}
```

### ESP32 Storage
- **Array Storage**: Schedules stored in memory arrays
- **ID Mapping**: Unique IDs for each schedule
- **TimeAlarms**: Integrated with Arduino TimeAlarms library
- **Persistence**: Survives reboots via Firebase sync

## Monitoring Schedules

### Serial Commands
```
schedules       # List all active schedules
status         # Show next schedule time
schedule log   # Display recent dispensing history
```

### Web Dashboard
- **Real-time Display**: Current active schedules
- **Next Dispense**: Upcoming medication times
- **History View**: Past dispensing events
- **Status Indicators**: Schedule health and sync status

### Firebase Console
- **Data Browser**: View raw schedule data
- **Real-time Updates**: Live synchronization monitoring
- **Audit Trail**: Schedule creation and modification logs

## Schedule Synchronization

### Sync Process
1. **Web Dashboard**: User creates/modifies schedule
2. **Firebase Update**: Data written to cloud database
3. **ESP32 Detection**: Device detects database changes
4. **Schedule Reload**: ESP32 updates internal schedule list
5. **Confirmation**: Success/failure feedback to user

### Sync Timing
- **Immediate**: Changes sync within seconds
- **Reliable**: Retry logic for failed syncs
- **Conflict Resolution**: Last-write-wins strategy
- **Offline Support**: Local operation during outages

## Advanced Scheduling

### Multiple Patients
- **Patient Tracking**: Associate schedules with individuals
- **Caregiver Alerts**: SMS notifications to specific contacts
- **Medication Logs**: Separate tracking per patient
- **Schedule Groups**: Organize by patient or medication type

### Complex Timing
- **Multiple Daily Doses**: Up to 3 schedules per dispenser
- **Irregular Schedules**: Custom day selections
- **Time Zone Support**: NTP-based accurate timing
- **DST Handling**: Automatic daylight saving adjustments

### Pill Size Optimization
- **Small Pills**: 800ms dispensing time
- **Medium Pills**: 1000ms dispensing time
- **Large Pills**: 1200ms dispensing time
- **Custom Timing**: Adjustable for specific medications

## Schedule Maintenance

### Regular Tasks
- **Review Schedules**: Weekly schedule audit
- **Update Medications**: Change drug information
- **Adjust Timing**: Modify dispense times as needed
- **Clean Up**: Remove completed or expired schedules

### Troubleshooting
- **Missed Doses**: Check time synchronization
- **Sync Issues**: Verify Firebase connectivity
- **Timing Errors**: Confirm NTP time accuracy
- **Schedule Conflicts**: Review overlapping times

## Schedule Templates

### Common Patterns
- **Standard Regimen**: Morning, afternoon, evening doses
- **BID Schedule**: Twice daily (morning/evening)
- **TID Schedule**: Three times daily
- **PRN Schedule**: As needed medications

### Template Creation
1. **Define Pattern**: Set standard times and days
2. **Save Template**: Store for reuse
3. **Apply Template**: Quick setup for new patients
4. **Customize**: Adjust for individual needs

## Emergency Features

### Manual Dispensing
- **Override Button**: Immediate dispensing via web
- **Serial Commands**: Direct control in development mode
- **Emergency Access**: Quick medication access
- **Audit Logging**: All manual dispenses recorded

### Schedule Override
- **Temporary Disable**: Pause specific schedules
- **Early Dispensing**: Advance schedule timing
- **Skip Dose**: Postpone scheduled dispensing
- **Resume Normal**: Return to regular schedule

## Performance Considerations

### System Limits
- **Schedule Capacity**: 15 concurrent schedules maximum
- **Timing Precision**: ±1 minute accuracy
- **Sync Latency**: <5 seconds typical
- **Memory Usage**: Minimal impact on ESP32 resources

### Optimization Tips
- **Consolidate Schedules**: Group similar timings
- **Minimize Changes**: Reduce frequent schedule updates
- **Batch Updates**: Modify multiple schedules together
- **Monitor Performance**: Track sync timing and success rates

## Integration with Healthcare

### Medical Compliance
- **Prescription Tracking**: Link to medication orders
- **Dosage Verification**: Confirm correct pill counts
- **Timing Adherence**: Monitor schedule compliance
- **Alert System**: Notify caregivers of issues

### Caregiver Management
- **Multiple Contacts**: SMS alerts to healthcare team
- **Schedule Handover**: Transfer patient responsibilities
- **Emergency Protocols**: Quick access procedures
- **Documentation**: Maintain medication administration records

---

**Schedule Management**: Comprehensive medication timing and tracking for reliable healthcare delivery.