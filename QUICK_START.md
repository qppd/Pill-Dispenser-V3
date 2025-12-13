# Quick Start Guide - Pill Dispenser V3

## 🚀 5-Minute Setup

### Prerequisites
- ✅ Arduino IDE installed
- ✅ ESP32 board support installed
- ✅ All libraries installed
- ✅ Hardware assembled and connected
- ✅ Firebase project created

### Step 1: Configure Credentials (2 minutes)

**WiFi Settings** - Edit `source/esp32/PillDispenser/PillDispenser.ino`:
```cpp
const String WIFI_SSID = "YourWiFiName";
const String WIFI_PASSWORD = "YourWiFiPassword";
```

**Firebase Settings** - Copy `FirebaseConfig.template.cpp` to `FirebaseConfig.cpp` and fill in your credentials:
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

**Phone Number** - Edit `PillDispenser.ino`:
```cpp
notificationManager.addPhoneNumber("+1234567890", "Caregiver Name");
```

### Step 2: Upload Firmware (1 minute)

1. Open `PillDispenser.ino` in Arduino IDE
2. Select:
   - Board: **ESP32 Dev Module**
   - Port: Your ESP32 COM port
   - Upload Speed: **921600**
3. Click **Upload** (Ctrl+U)

### Step 3: Verify System (1 minute)

1. Open **Serial Monitor** (Ctrl+Shift+M)
2. Set baud rate to **115200**
3. Press **Reset** button on ESP32

**Expected Output**:
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

### Step 4: Test Components
```
status          # Check all systems
battery         # Check battery reading
test dispenser 0    # Test one dispenser
```

### Step 5: Create First Schedule

1. **Start Web App**:
```bash
cd source/web
npm install
npm run dev
```

2. **Access Dashboard**: Open [http://localhost:3000](http://localhost:3000)

3. **Create Schedule**:
   - Login/Register
   - Go to **Schedule** page
   - Select **Container 0**
   - Set time, patient, medication, pill size
   - Enable schedule
   - Save changes

4. **Verify**: Check ESP32 serial monitor for schedule sync confirmation

## System Requirements

### Hardware
- ESP32 Development Board
- PCA9685 PWM Servo Driver
- 5x Continuous Rotation Servos
- I2C LCD Display (20x4)
- SIM800L GSM Module
- Voltage Sensor Circuit
- Power Supply (5V/2A)

### Software
- Arduino IDE with ESP32 support
- Required Arduino libraries
- Node.js 18+ (for web dashboard)
- Firebase project with Realtime Database

## First Time Setup Checklist

### ESP32 Firmware
- [ ] Arduino IDE installed
- [ ] ESP32 board support added
- [ ] Required libraries installed
- [ ] WiFi credentials configured
- [ ] Firebase credentials set up
- [ ] Phone number added
- [ ] Firmware uploaded successfully

### Hardware Assembly
- [ ] ESP32 connected to computer
- [ ] I2C devices wired correctly
- [ ] Servo motors connected
- [ ] GSM module installed
- [ ] Power supply connected
- [ ] Voltage sensor circuit complete

### Web Dashboard
- [ ] Node.js installed
- [ ] Dependencies installed (`npm install`)
- [ ] Firebase config added
- [ ] Development server running
- [ ] Authentication working

### Testing
- [ ] Serial monitor shows proper initialization
- [ ] All components report OK status
- [ ] WiFi connection established
- [ ] Firebase connection confirmed
- [ ] Test schedule created and synced

## Common Quick Start Issues

### ESP32 Won't Upload
**Problem**: Upload fails or times out
**Solution**:
- Check COM port selection
- Verify board type (ESP32 Dev Module)
- Try different upload speeds
- Press and hold BOOT button during upload

### WiFi Won't Connect
**Problem**: ESP32 can't connect to WiFi
**Solution**:
- Verify SSID and password
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 limitation)
- Test with different network

### Firebase Connection Failed
**Problem**: Can't connect to Firebase
**Solution**:
- Verify API key and database URL
- Check Firebase project settings
- Ensure Realtime Database is enabled
- Test with Firebase console

### Web App Won't Start
**Problem**: Development server fails to start
**Solution**:
- Check Node.js version (18+ required)
- Run `npm install` to install dependencies
- Verify Firebase configuration
- Check for port conflicts (default 3000)

## Next Steps

### After Quick Start
1. **Configure Additional Schedules**: Set up medication routines
2. **Test SMS Notifications**: Verify caregiver alerts work
3. **Monitor Battery Levels**: Track device power consumption
4. **Review System Logs**: Check Firebase for activity logs
5. **Customize Settings**: Adjust timing and notification preferences

### Advanced Configuration
- Set up multiple phone numbers for notifications
- Configure pill size timing for different medications
- Set up production mode for live deployment
- Implement backup power solutions
- Configure remote monitoring alerts

### Maintenance
- Regularly check battery levels
- Test servo motors weekly
- Update firmware when available
- Monitor Firebase usage and costs
- Backup configuration settings

## Getting Help

### Documentation
- **README.md**: Complete system documentation
- **INSTALLATION_GUIDE.md**: Detailed setup instructions
- **IMPLEMENTATION_SUMMARY.md**: Technical architecture details

### Support
- **Serial Commands**: Use `help` command for ESP32 assistance
- **Web Dashboard**: Built-in help and tooltips
- **Firebase Console**: Monitor system activity and logs

---

**Quick Start Complete!** Your Pill Dispenser V3 is now ready for medication management.