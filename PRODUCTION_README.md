# Pill Dispenser V3 - Production System

## 📋 Overview

Complete production-ready medication dispenser system with:
- ✅ **NTP-based time synchronization**
- ✅ **TimeAlarms scheduling system**
- ✅ **SMS notifications (before/during/after dispensing)**
- ✅ **Battery telemetry with percentage calculation**
- ✅ **Firebase real-time synchronization**
- ✅ **Web dashboard for monitoring and control**
- ✅ **Comprehensive logging and reporting**
- ❌ **NO Machine Learning / Camera integration**
- ❌ **NO IR Sensors**
- ❌ **NO RTC Modules**

---

## 🏗️ System Architecture

### Hardware Components

1. **ESP32 DevKit** - Main controller
2. **PCA9685** - 16-channel servo driver (controls 5 dispensers using channels 0-4)
3. **SIM800L** - GSM module for SMS notifications
4. **LCD Display** (I2C) - Status display
5. **Voltage Sensor** - Battery monitoring (GPIO34)
6. **5x Continuous Rotation Servos** - Pill dispensing mechanisms

### Software Stack

- **ESP32 Firmware**: Arduino/ESP-IDF
- **Web Application**: Next.js 14 + TypeScript
- **Backend**: Firebase Realtime Database + Authentication
- **Notifications**: SIM800L GSM module

---

## 🔥 Firebase Schema

```
pilldispenser/
  devices/
    {deviceId}/
      info/
        name: "PILL_DISPENSER_V3"
        mac: "AABBCCDDEEFF"
        firmware_version: "3.0.0"
      
      status/
        online: boolean
        last_heartbeat: timestamp
        wifi_rssi: number
        ip_address: string
      
      battery/
        voltage: number (9.0 - 12.6V for 3S Li-ion)
        percentage: number (0-100%)
        last_update: timestamp
        status: "Full" | "Good" | "Low" | "Critical"
      
      schedules/
        {scheduleId}/
          dispenser_id: number (0-4)
          time: "HH:MM"
          enabled: boolean
          medication_name: string
          patient_name: string
          pill_size: "small" | "medium" | "large"
          days: [0,1,2,3,4,5,6]  // Mon-Sun
      
      logs/
        {timestamp}/
          type: "dispense" | "schedule" | "error" | "system"
          dispenser_id: number
          pill_count: number
          timestamp: ISO8601
          status: "success" | "failed" | "skipped"
          trigger: "schedule" | "manual" | "remote"
          message: string
          medication_name: string (optional)
          patient_name: string (optional)
      
      reports/
        daily/
          {YYYY-MM-DD}/
            total_dispensed: number
            scheduled: number
            manual: number
            missed: number
            battery_avg: number
            dispensers:
              0: { dispensed: number, missed: number }
              1: { dispensed: number, missed: number }
              2: { dispensed: number, missed: number }
              3: { dispensed: number, missed: number }
              4: { dispensed: number, missed: number }
```

---

## 🚀 Features

### 1. Time Synchronization
- Uses NTP servers for accurate time
- Auto-sync every 6 hours
- No RTC module required
- Timezone configurable

### 2. Schedule Management
- Up to 15 schedules (3 per dispenser × 5 dispensers)
- Each schedule includes:
  - Time (HH:MM)
  - Patient name
  - Medication name
  - Pill size
  - Day of week selection
  - Enable/disable flag

### 3. SMS Notifications
Send SMS notifications for:
- **30 minutes before** scheduled dispensing
- **During dispensing** with patient and medication details
- **After pill taken** (manual confirmation or future sensor)
- **Missed doses** when schedule not executed
- **Low battery** warnings
- **System errors**

### 4. Battery Monitoring
- Reads voltage from ADC (GPIO34)
- Converts to percentage (9.0V = 0%, 12.6V = 100%)
- Updates Firebase every 1 minute
- Web dashboard displays real-time battery status
- SMS alerts when battery < 20%

### 5. Pill Dispensing
- 5 independent dispensers (servo channels 0-4)
- 3 pill sizes: small, medium, large
- Rotation duration calibrated per size
- Manual and scheduled dispensing
- Logs every dispense event to Firebase

### 6. Web Dashboard
Features:
- Real-time device status (online/offline)
- Battery level display
- Schedule creation/editing/deletion
- Manual dispense buttons
- Activity logs viewer
- Daily/weekly reports
- CSV export functionality

### 7. Production Mode
- Schedule enforcement (no manual override)
- Development commands locked
- Automatic schedule execution
- Comprehensive logging
- SMS notifications enabled

### 8. Development Mode
Serial commands available:
```
help              - Show all commands
status            - System status
time              - Show current time
voltage           - Battery status
test servo        - Test servo
test all servos   - Test all servos
test dispenser N  - Test dispenser N (0-4)
dispense N size   - Manual dispense
wifi connect      - Connect to WiFi
test firebase     - Test Firebase connection
test sim800       - Test GSM module
```

---

## 📦 Dependencies

### ESP32 Libraries
```cpp
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <TimeAlarms.h>                  // NEW: For scheduling
#include <Adafruit_PWMServoDriver.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
```

Install via Arduino Library Manager:
- `TimeAlarms` by Michael Margolis
- `Adafruit PWM Servo Driver Library`
- `LiquidCrystal I2C`
- `Firebase ESP Client`

### Web App Dependencies
```json
{
  "next": "^14.0.0",
  "react": "^18.2.0",
  "firebase": "^10.0.0",
  "lucide-react": "latest"
}
```

---

## ⚙️ Configuration

### 1. ESP32 Configuration

**WiFi Credentials** (`PillDispenser.ino`):
```cpp
const String WIFI_SSID = "YOUR_WIFI_SSID";
const String WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

**Firebase Config** (`FirebaseConfig.cpp`):
```cpp
const char* PillDispenserConfig::getApiKey() {
    return "YOUR_FIREBASE_API_KEY";
}

const char* PillDispenserConfig::getDatabaseURL() {
    return "https://your-project-id-default-rtdb.firebaseio.com";
}
```

**SMS Phone Numbers** (Add in setup):
```cpp
notificationManager.addPhoneNumber("+1234567890", "Caregiver");
notificationManager.addPhoneNumber("+0987654321", "Family");
```

**Timezone** (`TimeManager::begin()`):
```cpp
timeManager.begin("pool.ntp.org", 0, 0);  // GMT+0
// For other timezones:
// GMT+8: timeManager.begin("pool.ntp.org", 28800, 0);
// EST: timeManager.begin("pool.ntp.org", -18000, 0);
```

### 2. Web App Configuration

Create `.env.local`:
```bash
NEXT_PUBLIC_FIREBASE_API_KEY=your_api_key
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your-project.firebaseapp.com
NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://your-project-default-rtdb.firebaseio.com
NEXT_PUBLIC_FIREBASE_PROJECT_ID=your-project-id
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your-project.appspot.com
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=123456789
NEXT_PUBLIC_FIREBASE_APP_ID=1:123456789:web:abcdef
```

---

## 🔨 Building & Deployment

### ESP32 Firmware

1. **Install Arduino IDE** and ESP32 board support
2. **Install all required libraries** (see Dependencies)
3. **Configure** WiFi and Firebase credentials
4. **Upload** to ESP32:
   - Board: ESP32 Dev Module
   - Upload Speed: 921600
   - Flash Frequency: 80MHz
   - Partition Scheme: Default 4MB

### Web Application

```bash
cd source/web
npm install
npm run build
npm start
```

For production deployment (Vercel recommended):
```bash
vercel --prod
```

---

## 📊 Operation Flow

### Schedule Execution Flow

```
1. User creates schedule on Web App
   ↓
2. Schedule saved to Firebase
   ↓
3. ESP32 detects schedule via Firebase listener
   ↓
4. ESP32 adds schedule to TimeAlarms
   ↓
5. [30 minutes before] → Send "Reminder" SMS
   ↓
6. [At scheduled time] → TimeAlarms triggers
   ↓
7. ESP32 activates dispenser servo
   ↓
8. Pill dispensed (rotation for X milliseconds)
   ↓
9. Log event to Firebase
   ↓
10. Send "Dispensed" SMS notification
   ↓
11. Web dashboard updates in real-time
```

### Battery Monitoring Flow

```
1. ESP32 reads voltage sensor (every 1 minute)
   ↓
2. Calculate percentage (9V-12.6V → 0-100%)
   ↓
3. Upload to Firebase: /devices/{id}/battery
   ↓
4. Web dashboard displays battery %
   ↓
5. [If < 20%] → Send "Low Battery" SMS
```

---

## 🧪 Testing

### ESP32 Testing Commands

```
# Test individual components
test servo          # Test specific servo
test all servos     # Test all 16 channels
test dispenser 0    # Test dispenser 0
test all dispensers # Test all 5 dispensers
test lcd            # Test LCD display
test time           # Test NTP sync (continuous)
test sim800         # Test GSM module
test firebase       # Test Firebase connection
test voltage        # Test battery sensor (continuous)

# Manual operations
dispense 0 medium   # Manually dispense from dispenser 0
voltage             # Show current battery reading
time                # Show current time
status              # Show full system status
```

### Integration Testing

1. **Time Sync**: `test time` - Verify NTP working
2. **Battery**: `test voltage` - Check readings are 9-12.6V
3. **Servos**: `test all dispensers` - All 5 should rotate
4. **SMS**: `test sim800` - Send test SMS
5. **Firebase**: `test firebase` - Upload/download test
6. **Scheduling**: Add schedule on web, verify ESP32 receives it
7. **Dispensing**: Wait for schedule time, verify dispense + SMS

---

## 🐛 Troubleshooting

### Common Issues

**1. Time not syncing**
- Check WiFi connection
- Verify NTP server accessible
- Try alternate server: `time.google.com`

**2. SMS not sending**
- Verify SIM card inserted and activated
- Check signal strength: AT+CSQ
- Ensure sufficient balance/credits

**3. Firebase connection failing**
- Verify API key and database URL
- Check Firebase security rules
- Ensure WiFi connected

**4. Servos not moving**
- Check I2C connection (address 0x40)
- Verify PCA9685 power supply
- Run `i2c scan` to detect devices

**5. Battery reading incorrect**
- Check GPIO34 connection
- Verify voltage divider ratio (5.0)
- Calibrate if using different battery

---

## 📝 License

See LICENSE file for details.

---

## 👥 Support

For issues, questions, or contributions:
- Create an issue on GitHub
- Check documentation in `/docs`
- Review code comments for details

---

**Last Updated**: December 2025  
**Firmware Version**: 3.0.0  
**Web App Version**: 3.0.0
