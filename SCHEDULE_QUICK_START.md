# Schedule Quick Start - Pill Dispenser V3

## Creating Your First Schedule

### Prerequisites
- ✅ ESP32 firmware uploaded and running
- ✅ Web dashboard accessible
- ✅ Firebase connection established
- ✅ At least one dispenser tested

### Step 1: Access Web Dashboard
1. Open browser to `http://localhost:3000`
2. Login or register new account
3. Navigate to **Schedule** page

### Step 2: Add New Schedule
1. Click **"Add Schedule"** button
2. Select **Container 0** (first dispenser)
3. Configure schedule details:
   - **Time**: 08:00 (8:00 AM)
   - **Patient**: John Doe
   - **Medication**: Aspirin 81mg
   - **Pill Size**: Small
   - **Days**: Monday, Tuesday, Wednesday, Thursday, Friday
4. Click **"Save"** button

### Step 3: Verify Schedule Sync
1. Check ESP32 serial monitor
2. Look for sync confirmation message
3. Run `schedules` command to list active schedules

**Expected Serial Output**:
```
📅 SCHEDULE SYNC - New schedule added
Container: 0, Time: 08:00, Medication: Aspirin 81mg
✅ Schedule synchronized successfully
```

## Testing the Schedule

### Manual Testing
1. Wait for scheduled time OR use manual dispense
2. Check serial monitor for dispensing activity
3. Verify SMS notification sent (if configured)
4. Confirm Firebase logging

### Serial Commands for Testing
```
schedules         # List all schedules
status           # Show system status
dispense 0 small # Manual dispense test
battery          # Check battery status
```

## Schedule Management Basics

### Schedule Components
- **Container**: Dispenser number (0-4)
- **Time**: Hour and minute (24-hour format)
- **Patient**: Person receiving medication
- **Medication**: Drug name and dosage
- **Pill Size**: Small/Medium/Large timing
- **Days**: Active days of the week

### Basic Operations
- **Add**: Create new medication schedules
- **Edit**: Modify existing schedule details
- **Delete**: Remove unwanted schedules
- **Enable/Disable**: Control schedule activation

## Common Schedule Scenarios

### Daily Medication (Mon-Fri)
- **Time**: 08:00, 14:00, 20:00
- **Days**: Monday through Friday
- **Patient**: Regular daily medication

### Weekly Medication
- **Time**: 09:00
- **Days**: Sunday only
- **Patient**: Weekly supplement

### Multiple Patients
- **Patient A**: Morning medications
- **Patient B**: Evening medications
- **Separate Schedules**: Track individually

## Schedule Limits and Best Practices

### System Limits
- **Max Schedules**: 15 total (3 per dispenser)
- **Containers**: 5 available (0-4)
- **Timing**: Minute precision
- **Days**: Full week selection

### Best Practices
- **Consistent Timing**: Use standard times when possible
- **Patient Organization**: Group schedules by patient
- **Regular Review**: Check schedules weekly
- **Backup Planning**: Have caregiver backup contacts

## Troubleshooting Schedules

### Schedule Not Syncing
**Problem**: Changes not appearing on ESP32
**Solution**:
- Check Firebase connection
- Verify web dashboard saves successfully
- Restart ESP32 to force sync
- Check serial monitor for error messages

### Missed Dispensing
**Problem**: Scheduled time passed without dispensing
**Solution**:
- Verify ESP32 time is correct (NTP sync)
- Check schedule is enabled
- Confirm dispenser is functional
- Review Firebase logs for errors

### SMS Not Received
**Problem**: No notification for scheduled dispense
**Solution**:
- Verify phone number format (+1234567890)
- Check GSM signal strength
- Confirm SIM card is active
- Test SMS with manual dispense

## Advanced Schedule Features

### Multiple Daily Doses
- **Container 0**: Morning medication
- **Container 1**: Afternoon medication
- **Container 2**: Evening medication

### Different Pill Sizes
- **Small**: Vitamins, regular pills
- **Medium**: Standard tablets
- **Large**: Large capsules, supplements

### Day-Specific Scheduling
- **Weekdays**: Work/school day medications
- **Weekends**: Different timing or medications
- **Custom**: Specific days only

## Monitoring and Maintenance

### Daily Checks
- [ ] Schedules are active and correct
- [ ] Next dispense time is reasonable
- [ ] Battery level is adequate
- [ ] System status is normal

### Weekly Maintenance
- [ ] Review medication compliance
- [ ] Update medication information
- [ ] Test dispenser functionality
- [ ] Check SMS delivery

### Monthly Review
- [ ] Audit all active schedules
- [ ] Verify patient information
- [ ] Update contact phone numbers
- [ ] Review system performance

## Emergency Procedures

### Manual Dispense
1. Access web dashboard
2. Go to dispenser control
3. Click "Dispense Now"
4. Select pill size
5. Confirm dispensing

### Schedule Override
1. Disable problematic schedule
2. Perform manual dispense if needed
3. Re-enable schedule when resolved
4. Monitor for proper operation

### System Reset
1. Restart ESP32 device
2. Verify Firebase reconnection
3. Check schedule sync status
4. Test dispensing functionality

## Getting Help

### Quick Reference
- **Web Dashboard**: Schedule management interface
- **Serial Monitor**: Real-time system status
- **Firebase Console**: Data verification and logs

### Support Commands
```
help             # Show all available commands
status           # System health check
schedules        # List active schedules
diagnostics      # Network and system diagnostics
```

---

**Schedule Quick Start Complete!** Your medication schedules are now active and monitored.