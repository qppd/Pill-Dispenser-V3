# Installation Guide - Pill Dispenser V3

## ESP32 Firmware Installation

### 1. Install Arduino IDE
- Download from [arduino.cc](https://www.arduino.cc/en/software)
- Install ESP32 board support via Boards Manager

### 2. Install Required Libraries
Open Arduino IDE → Tools → Manage Libraries:
- Time by Michael Margolis
- TimeAlarms by Michael Margolis
- Adafruit PWM Servo Driver Library
- LiquidCrystal I2C by Frank de Brabander
- Firebase ESP Client by Mobizt

### 3. Configure Board Settings
- Board: ESP32 Dev Module
- Upload Speed: 921600
- CPU Frequency: 240MHz
- Flash Frequency: 80MHz
- Partition Scheme: Default 4MB

### 4. Hardware Assembly
**I2C Connections**:
- ESP32 GPIO 21 (SDA) → PCA9685 SDA, LCD SDA
- ESP32 GPIO 22 (SCL) → PCA9685 SCL, LCD SCL

**Serial Connections**:
- ESP32 GPIO 16 → SIM800L TX
- ESP32 GPIO 17 → SIM800L RX
- ESP32 GPIO 4 → SIM800L RST

**Analog Input**:
- ESP32 GPIO 34 → Voltage divider (battery monitoring)

**Servo Connections**:
- PCA9685 Channels 0-4 → 5 servo motors

## Web Application Installation

### 1. Install Dependencies
```bash
cd source/web
npm install
```

### 2. Configure Environment
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

### 3. Start Development Server
```bash
npm run dev
```

### 4. Build for Production
```bash
npm run build
npm start
```

## Hardware Requirements

### ESP32 Main Controller
- **ESP32 Development Board**: ESP32-WROOM-32 or compatible
- **PCA9685 PWM Servo Driver**: 16-channel servo controller
- **I2C LCD Display**: 20x4 character LCD with I2C backpack
- **SIM800L GSM Module**: Quad-band GSM/GPRS with antenna
- **Voltage Sensor**: Analog divider circuit for battery monitoring
- **Servo Motors**: 5x continuous rotation servos
- **Power Supply**: 5V/2A for servos, 3.7-4.2V for SIM800L

### Web Dashboard
- **Node.js**: Version 18+ runtime
- **npm**: Package manager
- **Firebase Project**: With Realtime Database enabled

## Software Dependencies

### ESP32 Libraries (Arduino IDE)
```cpp
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <Adafruit_PWMServoDriver.h>
#include <LiquidCrystal_I2C.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
```

### Web Application (package.json)
```json
{
  "name": "pill-dispenser-web",
  "version": "3.0.0",
  "dependencies": {
    "next": "^14.0.0",
    "react": "^18.2.0",
    "firebase": "^10.0.0",
    "lucide-react": "latest"
  }
}
```

## Configuration Steps

### ESP32 Configuration

#### WiFi Credentials
```cpp
const String WIFI_SSID = "YOUR_WIFI_SSID";
const String WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

#### Firebase Service Account
Copy `FirebaseConfig.template.cpp` to `FirebaseConfig.cpp` and configure:
```cpp
const char* PillDispenserConfig::getClientEmail() {
    return "firebase-adminsdk-xxxxx@your-project.iam.gserviceaccount.com";
}

const char* PillDispenserConfig::getPrivateKey() {
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_PRIVATE_KEY_HERE\n"
           "-----END PRIVATE KEY-----\n";
}
```

#### SMS Notifications
```cpp
notificationManager.addPhoneNumber("+1234567890", "Caregiver Name");
notificationManager.addPhoneNumber("+0987654321", "Family Member");
```

#### Device Identification
```cpp
const String DEVICE_ID = "PILL_DISPENSER_V3";
```

## Firebase Database Rules

**Development Rules**:
```json
{
  "rules": {
    "pilldispenser": {
      ".read": true,
      ".write": true
    }
  }
}
```

**Production Rules**:
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

## Testing Installation

### ESP32 Testing
1. Open Serial Monitor (115200 baud)
2. Check initialization messages
3. Run `status` command
4. Test individual components

### Web Application Testing
1. Start development server
2. Access http://localhost:3000
3. Test authentication
4. Create test schedule
5. Verify Firebase connection

## Troubleshooting Installation

### Common Issues

#### Arduino IDE Issues
- **Board not found**: Install ESP32 board support
- **Library errors**: Check library versions
- **Upload failed**: Check COM port and board selection

#### ESP32 Connection Issues
- **WiFi not connecting**: Verify credentials and network
- **Firebase errors**: Check API key and database URL
- **I2C errors**: Verify wiring and addresses

#### Web Application Issues
- **Build errors**: Check Node.js version
- **Firebase config**: Verify environment variables
- **Port conflicts**: Change default port if needed

## Production Deployment

### ESP32 Deployment
1. Configure all credentials
2. Set production mode
3. Upload firmware
4. Test all functions
5. Monitor serial output

### Web Application Deployment
1. Build for production
2. Configure production Firebase
3. Deploy to hosting platform
4. Test all features
5. Monitor performance