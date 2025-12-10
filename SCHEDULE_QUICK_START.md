# Quick Start: Schedule Management

## 🚀 Getting Started in 5 Minutes

### Step 1: Configure ESP32 User ID
Open `source/esp32/PillDispenser/PillDispenser.ino` and set your user ID:

```cpp
const String USER_ID = "default_user";  // Change this to your Firebase user ID
```

> **Note**: In production, this should match the authenticated user's UID from Firebase Auth.

### Step 2: Upload ESP32 Code
1. Open Arduino IDE
2. Load `PillDispenser.ino`
3. Select ESP32 board
4. Upload to device
5. Open Serial Monitor (115200 baud)

You should see:
```
📅 Loading schedules from Firebase...
FirebaseManager: Syncing schedules from Firebase...
✅ Schedules loaded successfully
```

### Step 3: Start Web Application
```bash
cd source/web
npm install
npm run dev
```

Open: http://localhost:3000

### Step 4: Create Your First Schedule

1. **Login** to the web app
2. **Navigate** to Schedule page
3. **Select** Container 1
4. **Click** "Add Schedule"
5. **Set** time (e.g., 2 minutes from now for testing)
6. **Enable** the checkbox
7. **Click** "Save Changes"

### Step 5: Watch It Work!

Monitor the ESP32 Serial output:

**Before scheduled time:**
```
💓 System heartbeat - 14:28:30
Next schedule: 14:30
```

**At scheduled time:**
```
============================================================
⏰ SCHEDULED DISPENSE TRIGGERED
============================================================
Container: 1
Medication: Container 1 Medication
Time: 14:30:00
============================================================
🔄 Dispensing from container 1...
✅ Dispense complete
```

## 🎯 Common Use Cases

### Daily Medication Schedule
```
Container 1: Morning pills - 08:00
Container 2: Afternoon pills - 14:00
Container 3: Evening pills - 20:00
```

### Multiple Daily Doses
```
Container 1:
  - 08:00 (Morning dose)
  - 12:00 (Midday dose)
  - 20:00 (Evening dose)
```

### Mixed Schedule
```
Container 1: 08:00 (enabled)
Container 2: 12:00 (enabled)
Container 3: 16:00 (enabled)
Container 4: 20:00 (enabled)
Container 5: 22:00 (disabled - weekend only, manually enable)
```

## 🔧 Testing Tips

### Test Real-time Sync
1. Open Serial Monitor
2. Add schedule in web app
3. Save changes
4. Watch ESP32 immediately sync:
```
FirebaseManager: Updated Path: /pill_schedule
FirebaseManager: Triggering schedule sync due to update...
```

### Test Schedule Trigger
Set a schedule for 1-2 minutes in the future to quickly verify it works.

### Test Manual Override
Use "Dispense Now" button alongside schedules - both work independently!

## 📱 Monitoring

### ESP32 Serial Monitor
- System heartbeat every 30 seconds
- Next scheduled time displayed
- Detailed logging of all events

### Firebase Console
- Check `schedules/{userId}` for user data
- Check `pilldispenser/device/schedules/{userId}` for device data
- View logs in real-time

### Web App
- Current schedules displayed
- Enable/disable toggle
- Visual feedback on save

## ⚠️ Important Notes

1. **Time Sync**: ESP32 must have correct time via NTP
2. **WiFi**: Device must be connected to internet
3. **User ID**: Must match between web app and ESP32
4. **Maximum**: 3 schedules per container (15 total)
5. **Persistence**: Schedules reload automatically after ESP32 reboot

## 🐛 Troubleshooting

### Schedule Not Triggering
- ✅ Check ESP32 time is correct
- ✅ Verify schedule is enabled
- ✅ Check `Next schedule:` in heartbeat
- ✅ Ensure alarm was created successfully

### Schedule Not Syncing
- ✅ Verify WiFi connection
- ✅ Check USER_ID matches
- ✅ Confirm Firebase rules allow access
- ✅ Check Serial Monitor for error messages

### Web App Not Saving
- ✅ Check browser console for errors
- ✅ Verify Firebase config in `.env`
- ✅ Ensure user is logged in
- ✅ Check network tab for Firebase requests

## 📚 Additional Resources

- `SCHEDULE_MANAGEMENT_GUIDE.md` - Complete feature documentation
- `SCHEDULE_IMPLEMENTATION.md` - Technical implementation details
- `API_REFERENCE.md` - Full API documentation

## ✨ Success Indicators

You know it's working when:
- ✅ Serial Monitor shows "Schedules loaded successfully"
- ✅ Web app displays your saved schedules
- ✅ ESP32 dispenses at scheduled times
- ✅ LCD shows "DISPENSING" message
- ✅ Firebase logs show dispense events

---
**Ready to go!** Your pill dispenser is now fully automated with smart scheduling! 🎉
