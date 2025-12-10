# Installation & Setup Guide - Pill Dispenser V3

## 📦 Required Libraries

### ESP32 Arduino Libraries

Install these via Arduino Library Manager (`Tools` → `Manage Libraries...`):

1. **Time** by Michael Margolis
   - Version: 1.6.1 or higher
   - Used for: Time handling and NTP integration
   ```
   Search: "Time" by Michael Margolis
   ```

2. **TimeAlarms** by Michael Margolis
   - Version: 1.5.0 or higher
   - Used for: Schedule-based alarm triggering
   ```
   Search: "TimeAlarms" by Michael Margolis
   ```

3. **Adafruit PWM Servo Driver Library**
   - Version: 2.4.0 or higher
   - Used for: PCA9685 servo control
   ```
   Search: "Adafruit PWM Servo Driver"
   ```

4. **LiquidCrystal I2C** by Frank de Brabander
   - Version: 1.1.2 or higher
   - Used for: LCD display control
   ```
   Search: "LiquidCrystal I2C"
   ```

5. **Firebase ESP Client** by Mobizt
   - Version: 4.3.0 or higher
   - Used for: Firebase Realtime Database
   ```
   Search: "Firebase ESP Client" by Mobizt
   ```

### Manual Installation (if needed)

Some libraries might need manual installation:

```bash
# Download and install to Arduino/libraries/ folder
# TimeAlarms: https://github.com/PaulStoffregen/TimeAlarms
# Firebase: https://github.com/mobizt/Firebase-ESP-Client
```

---

## ⚙️ Arduino IDE Configuration

### 1. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to `File` → `Preferences`
3. Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to `Tools` → `Board` → `Boards Manager`
5. Search for "esp32" by Espressif Systems
6. Install version 2.0.11 or higher

### 2. Board Settings

Select these settings in Arduino IDE:

- **Board**: "ESP32 Dev Module"
- **Upload Speed**: 921600
- **CPU Frequency**: 240MHz (WiFi/BT)
- **Flash Frequency**: 80MHz
- **Flash Mode**: QIO
- **Flash Size**: 4MB (32Mb)
- **Partition Scheme**: "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
- **Core Debug Level**: "None" (for production) or "Info" (for debugging)
- **PSRAM**: "Disabled"

### 3. Compilation Flags (if needed)

If you encounter compilation issues, add these to `platform.txt` or as build flags:

```cpp
-DARDUINO_LOOP_STACK_SIZE=16384
-DCORE_DEBUG_LEVEL=0
```

---

## 🔧 Hardware Setup

### Pin Connections

#### ESP32 Pin Assignments (from PINS_CONFIG.h)

| Component | ESP32 Pin | Notes |
|-----------|-----------|-------|
| SDA (I2C) | GPIO 21 | PCA9685 & LCD |
| SCL (I2C) | GPIO 22 | PCA9685 & LCD |
| SIM800L RX | GPIO 16 | Connect to SIM800L TX |
| SIM800L TX | GPIO 17 | Connect to SIM800L RX |
| SIM800L RST | GPIO 4 | Reset pin |
| Voltage Sensor | GPIO 34 | ADC1_CH5 |
| Status LED | GPIO 2 | Built-in LED |

#### I2C Devices

| Device | I2C Address |
|--------|-------------|
| PCA9685 Servo Driver | 0x40 |
| LCD Display (20x4) | 0x27 or 0x3F |

#### Power Requirements

- **ESP32**: 3.3V (USB or regulated supply)
- **PCA9685**: 5V for servos, 3.3V for logic (ESP32 compatible)
- **Servos**: External 5-6V power supply (2-3A recommended for 5 servos)
- **SIM800L**: 3.7-4.2V (use dedicated power supply, 2A peak)
- **LCD**: 5V via I2C module

### Wiring Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         ESP32 DevKit                             │
│                                                                   │
│  GPIO21 (SDA) ────────┬──────────── PCA9685 SDA                 │
│  GPIO22 (SCL) ────────┼──────────── PCA9685 SCL                 │
│                        │                                          │
│                        └──────────── LCD SDA                     │
│                        └──────────── LCD SCL                     │
│                                                                   │
│  GPIO16 ────────────────────────── SIM800L TX                   │
│  GPIO17 ────────────────────────── SIM800L RX                   │
│  GPIO4  ────────────────────────── SIM800L RST                  │
│                                                                   │
│  GPIO34 ────────────── Voltage Divider ──── Battery +           │
│  GND    ────────────────────────────────── Battery -            │
│                                                                   │
│  GPIO2  ────────────── Status LED                               │
└─────────────────────────────────────────────────────────────────┘

PCA9685:
  Channel 0 ────── Dispenser 0 Servo
  Channel 1 ────── Dispenser 1 Servo
  Channel 2 ────── Dispenser 2 Servo
  Channel 3 ────── Dispenser 3 Servo
  Channel 4 ────── Dispenser 4 Servo
  V+ ────────────── External 5V Power
  GND ──────────── Common Ground
```

---

## 🔥 Firebase Setup

### 1. Create Firebase Project

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click "Add project"
3. Enter project name: "pill-dispenser-v3"
4. Disable Google Analytics (optional)
5. Create project

### 2. Enable Realtime Database

1. In Firebase Console, go to "Realtime Database"
2. Click "Create Database"
3. Choose location (closest to you)
4. Start in **test mode** for development:
   ```json
   {
     "rules": {
       ".read": true,
       ".write": true
     }
   }
   ```

5. For **production**, use authenticated rules:
   ```json
   {
     "rules": {
       "pilldispenser": {
         "devices": {
           "$deviceId": {
             ".read": "auth != null",
             ".write": "auth != null"
           }
         }
       }
     }
   }
   ```

### 3. Get Firebase Credentials

1. Go to Project Settings (gear icon)
2. Scroll to "Your apps"
3. Click "Web" (</> icon)
4. Register app name: "pill-dispenser-web"
5. Copy the config values:
   - `apiKey`
   - `authDomain`
   - `databaseURL`
   - `projectId`
   - `storageBucket`
   - `messagingSenderId`
   - `appId`

### 4. Configure ESP32

Edit `source/esp32/PillDispenser/FirebaseConfig.cpp`:

```cpp
const char* PillDispenserConfig::getApiKey() {
    return "AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
}

const char* PillDispenserConfig::getDatabaseURL() {
    return "https://pill-dispenser-v3-default-rtdb.firebaseio.com";
}

const char* PillDispenserConfig::getProjectId() {
    return "pill-dispenser-v3";
}
```

### 5. Configure Web App

Create `source/web/.env.local`:

```bash
NEXT_PUBLIC_FIREBASE_API_KEY=AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=pill-dispenser-v3.firebaseapp.com
NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://pill-dispenser-v3-default-rtdb.firebaseio.com
NEXT_PUBLIC_FIREBASE_PROJECT_ID=pill-dispenser-v3
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=pill-dispenser-v3.appspot.com
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=123456789012
NEXT_PUBLIC_FIREBASE_APP_ID=1:123456789012:web:abcdef123456
```

---

## 📱 SIM800L Configuration

### SIM Card Setup

1. Insert activated SIM card into SIM800L
2. Ensure SIM card has:
   - Active cellular plan
   - SMS capability enabled
   - Sufficient credit/balance
   - PIN disabled (or configure in code)

### Antenna

- **Required**: Connect GSM antenna to SIM800L
- Signal quality affects SMS reliability
- Check signal with AT command: `AT+CSQ`

### Power

- SIM800L requires **2A peak current**
- Use dedicated power supply (not from ESP32)
- Voltage: 3.7V - 4.2V
- Add capacitors (100µF + 10µF) near power pins

---

## 📱 Web App Setup

### 1. Install Dependencies

```bash
cd source/web
npm install
```

### 2. Configure Environment

Create `.env.local` with Firebase credentials (see above)

### 3. Run Development Server

```bash
npm run dev
```

Open [http://localhost:3000](http://localhost:3000)

### 4. Build for Production

```bash
npm run build
npm start
```

### 5. Deploy to Vercel (Recommended)

```bash
# Install Vercel CLI
npm i -g vercel

# Deploy
vercel --prod
```

Or connect GitHub repo to Vercel for automatic deployments.

---

## ✅ Verification & Testing

### ESP32 Firmware

1. **Upload Sketch**
   ```
   Arduino IDE → Upload (Ctrl+U)
   ```

2. **Open Serial Monitor** (115200 baud)

3. **Expected Output**:
   ```
   ═══════════════════════════════════════════════════════════
         PILL DISPENSER V3 - PRODUCTION SYSTEM
         Firmware Version: 3.0.0
   ═══════════════════════════════════════════════════════════

   🏭 PRODUCTION MODE - Full System Initialization

   📋 Initializing Production Components...

   1. LCD Display: ✅ OK
   2. WiFi Connection:  ✅ Connected
      IP: 192.168.1.100
   3. Time Sync (NTP): ✅ OK - 2025-12-10 15:30:00
   4. Firebase Connection: ✅ OK
   5. Servo Driver (PCA9685): ✅ OK - 5 dispensers ready
   6. SIM800L GSM Module: ✅ OK - SMS ready
   7. Battery Monitor: ✅ OK - 85.0% (Good)
   8. Notification Manager: ✅ OK - 1 contacts
   9. Schedule Manager: ✅ OK - Ready for scheduling
   10. Uploading initial status to Firebase...

   ✅ Production mode initialization complete!
   🔒 Schedule enforcement: ACTIVE
   📱 SMS notifications: ENABLED
   ☁️  Firebase sync: ACTIVE

   ═══════════════════════════════════════════════════════════
       ✅ SYSTEM READY
   ═══════════════════════════════════════════════════════════
   ```

4. **Test Commands**:
   ```
   status              # Check all systems
   battery             # Check battery
   schedules           # List schedules
   test dispenser 0    # Test dispenser
   test sms            # Test SMS notification
   ```

### Web App

1. **Create Account** via Firebase Authentication
2. **Login** to dashboard
3. **Add Schedule**:
   - Select Container 0
   - Set time
   - Enter patient/medication info
   - Enable schedule
4. **Verify**:
   - Check ESP32 Serial Monitor for schedule sync
   - Wait for scheduled time or manually test

### Firebase

Check Firebase Console → Realtime Database for structure:

```
pilldispenser/
  devices/
    PILL_DISPENSER_xxxxx/
      info/
      status/
      battery/
      schedules/
      logs/
```

---

## 🐛 Troubleshooting

See [PRODUCTION_README.md](./PRODUCTION_README.md#-troubleshooting) for common issues and solutions.

---

## 📚 Next Steps

1. ✅ Configure WiFi credentials
2. ✅ Configure Firebase
3. ✅ Add caregiver phone numbers
4. ✅ Test each component individually
5. ✅ Create first schedule
6. ✅ Test scheduled dispensing
7. ✅ Monitor logs and reports

---

**Setup Complete!** 🎉

The system is now ready for production use. Monitor via web dashboard and adjust schedules as needed.
