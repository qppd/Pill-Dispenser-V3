# Quick Start Guide - Pill Dispenser V3

## 🚀 5-Minute Setup

### Prerequisites
- ✅ Arduino IDE installed
- ✅ ESP32 board support installed
- ✅ All libraries installed (see INSTALLATION_GUIDE.md)
- ✅ Hardware assembled and connected
- ✅ Firebase project created

---

## Step 1: Configure Credentials (2 minutes)

### A. WiFi Settings
Edit `source/esp32/PillDispenser/PillDispenser.ino`:

```cpp
// Line ~40
const String WIFI_SSID = "YourWiFiName";
const String WIFI_PASSWORD = "YourWiFiPassword";
```

### B. Firebase Settings
Edit `source/esp32/PillDispenser/FirebaseConfig.cpp`:

```cpp
const char* PillDispenserConfig::getApiKey() {
    return "YOUR_FIREBASE_API_KEY_HERE";
}

const char* PillDispenserConfig::getDatabaseURL() {
    return "https://your-project-default-rtdb.firebaseio.com";
}

const char* PillDispenserConfig::getProjectId() {
    return "your-project-id";
}
```

### C. Phone Number (Optional but recommended)
Edit `source/esp32/PillDispenser/PillDispenser.ino`:

```cpp
// Line ~55
const String CAREGIVER_1_PHONE = "+1234567890";  // Your phone number
const String CAREGIVER_1_NAME = "Caregiver Name";
```

---

## Step 2: Upload Firmware (1 minute)

1. Open `PillDispenser.ino` in Arduino IDE
2. Select:
   - Board: **ESP32 Dev Module**
   - Port: Your ESP32 COM port
   - Upload Speed: **921600**
3. Click **Upload** (Ctrl+U)
4. Wait for "Done uploading"

---

## Step 3: Verify System (1 minute)

1. Open **Serial Monitor** (Ctrl+Shift+M)
2. Set baud rate to **115200**
3. Press **Reset** button on ESP32

### Expected Output:
```
═══════════════════════════════════════════════════════════
     PILL DISPENSER V3 - PRODUCTION SYSTEM
     Firmware Version: 3.0.0
═══════════════════════════════════════════════════════════

🏭 PRODUCTION MODE - Full System Initialization

1. LCD Display: ✅ OK
2. WiFi Connection:  ✅ Connected
   IP: 192.168.1.XXX
3. Time Sync (NTP): ✅ OK - 2025-12-10 15:30:00
4. Firebase Connection: ✅ OK
5. Servo Driver (PCA9685): ✅ OK - 5 dispensers ready
6. SIM800L GSM Module: ✅ OK - SMS ready
7. Battery Monitor: ✅ OK - 85.0% (Good)
8. Notification Manager: ✅ OK - 1 contacts
9. Schedule Manager: ✅ OK - Ready for scheduling

✅ Production mode initialization complete!
```

### ❌ If Any Component Shows Failed:
- **WiFi**: Check credentials and signal
- **Time**: Check internet connection
- **Firebase**: Verify API key and database URL
- **Servo**: Check I2C connections (address 0x40)
- **SIM800L**: Check SIM card and antenna
- **LCD**: Check I2C connection (address 0x27 or 0x3F)

---

## Step 4: Test Components (1 minute)

Type these commands in Serial Monitor:

```
status          # Check all systems
battery         # Check battery reading
test dispenser 0    # Test one dispenser
```

### Expected Results:
- `status` shows all components ✅
- `battery` shows voltage and percentage
- `test dispenser 0` makes servo rotate

---

## Step 5: Create First Schedule (Web App)

### A. Setup Web App (if not done):
```bash
cd source/web
npm install
npm run dev
```

### B. Access Dashboard:
Open [http://localhost:3000](http://localhost:3000)

### C. Create Schedule:
1. Login/Register
2. Go to **Schedule** page
3. Select **Container 0**
4. Click **Add Schedule**
5. Set:
   - Time: `14:00`
   - Patient: `Test Patient`
   - Medication: `Test Med`
   - Pill Size: `Medium`
   - Days: All selected
   - Enabled: ✅ Checked
6. Schedule automatically saves to Firebase

### D. Verify on ESP32:
In Serial Monitor, type:
```
schedules
```

You should see your schedule listed!

---

## ✅ System Ready!

The system will now:
1. **30 minutes before** schedule: Send SMS reminder
2. **At scheduled time**: Dispense pill automatically
3. **After dispensing**: Send SMS confirmation
4. **Every minute**: Update battery status to Firebase
5. **Continuously**: Sync with web dashboard

---

## 🧪 Quick Tests

### Test Manual Dispense:
```
dispense 0 medium
```

### Test SMS:
```
test sms
```

### Check Next Schedule:
```
status
```
(Look for "Next Schedule" in output)

### View All Schedules:
```
schedules
```

---

## 🎯 Production Mode vs Development Mode

### Switch to Development Mode (for testing):
Edit `PillDispenser.ino` line ~17:
```cpp
#define DEVELOPMENT_MODE true   // Enable dev mode
```

Upload again. Now you have:
- Full serial command access
- Manual dispense anytime
- Verbose debugging output

### Switch Back to Production:
```cpp
#define DEVELOPMENT_MODE false  // Production mode
```

---

## 📱 Mobile Access

### Deploy Web App (Vercel):
```bash
cd source/web
vercel --prod
```

Access from anywhere: `https://your-app.vercel.app`

---

## 🆘 Quick Troubleshooting

| Issue | Solution |
|-------|----------|
| WiFi won't connect | Check SSID/password, signal strength |
| Time not syncing | Check internet, try `pool.ntp.org` |
| Firebase fails | Verify API key, check database rules |
| Servos don't move | Check power supply, I2C connection |
| SMS not sending | Check SIM card, signal, credits |
| Battery shows 0% | Check GPIO34 connection |

---

## 📞 Get Help

1. Check Serial Monitor output
2. Type `status` to see component status
3. Read `PRODUCTION_README.md` for detailed docs
4. Review `INSTALLATION_GUIDE.md` for setup details

---

## 🎉 You're Done!

System is now:
- ✅ Monitoring battery
- ✅ Ready to dispense on schedule
- ✅ Sending SMS notifications
- ✅ Syncing with web dashboard
- ✅ Logging all activities

Add more schedules via web dashboard and enjoy automated medication management!

---

**Next**: See `PRODUCTION_README.md` for advanced features and full documentation.
