# Deployment Guide - Pill Dispenser V3

This document provides comprehensive deployment procedures for the Pill Dispenser V3 ESP32-based medication dispensing system.

## Table of Contents

- [Pre-Deployment Checklist](#pre-deployment-checklist)
- [Hardware Assembly](#hardware-assembly)
- [Software Installation](#software-installation)
- [System Configuration](#system-configuration)
- [Production Setup](#production-setup)
- [Quality Assurance](#quality-assurance)
- [Go-Live Procedures](#go-live-procedures)
- [Post-Deployment Support](#post-deployment-support)

## Pre-Deployment Checklist

### ESP32 Main Controller Requirements

**ESP32 Development Board**:
- [ ] ESP32-WROOM-32 or compatible (ESP32 Dev Module)
- [ ] Built-in WiFi capability
- [ ] Minimum 4MB Flash memory
- [ ] 520KB SRAM
- [ ] 30+ GPIO pins

**PCA9685 PWM Servo Driver**:
- [ ] 16-channel PWM output
- [ ] I2C interface capability
- [ ] 5V power supply compatibility
- [ ] External power terminal for servos

**LCD Display**:
- [ ] 20x4 character I2C LCD
- [ ] HD44780 compatible controller
- [ ] I2C backpack module
- [ ] 5V power supply

**SIM800L GSM Module**:
- [ ] SIM800L GSM/GPRS module
- [ ] Quad-band compatibility
- [ ] SMS capability
- [ ] External antenna connection

**Voltage Sensor**:
- [ ] Analog voltage divider circuit
- [ ] Connected to ESP32 ADC pin (GPIO34)
- [ ] 9V-12.6V input range for Li-ion battery monitoring

**Servo Motors**:
- [ ] 5x Continuous rotation servos
- [ ] 5V power supply compatibility
- [ ] Compatible with PCA9685 control

### Web Dashboard Requirements

**Development Environment**:
- [ ] Node.js 18+ runtime
- [ ] npm package manager
- [ ] Git for version control

**Firebase Services**:
- [ ] Firebase project created
- [ ] Authentication enabled
- [ ] Realtime Database enabled
- [ ] Service account configured

### Documentation and Tools

**Documentation**:
- [ ] Hardware setup guide
- [ ] Software installation guide
- [ ] User operation manual
- [ ] Troubleshooting guide

**Tools Required**:
- [ ] Arduino IDE with ESP32 support
- [ ] Soldering iron and solder
- [ ] Wire strippers and cutters
- [ ] Multimeter for testing
- [ ] Screwdrivers
- [ ] Computer for programming

## Hardware Assembly

### Step 1: Power Distribution Setup

**Objective**: Establish stable power distribution for all components

**Components**: Power supply, breadboard/PCB, jumper wires

**Procedure**:

1. **Main Power Rails**:
   ```
   Connect power sources to appropriate rails:
   - 5V rail: For servos, LCD, PCA9685 logic
   - 3.3V rail: For ESP32, SIM800L (through regulator)
   - Ground rail: Common ground for all components
   ```

2. **ESP32 Power Connection**:
   ```
   ESP32 VIN  -> 5V power rail (or USB power)
   ESP32 GND  -> Ground rail
   ```

3. **Component Power Distribution**:
   ```
   PCA9685 VCC   -> 3.3V rail (logic power)
   PCA9685 V+    -> 5V rail (servo power)
   PCA9685 GND   -> Ground rail
   LCD VCC       -> 5V rail
   LCD GND       -> Ground rail
   SIM800L VCC   -> 3.7-4.2V (dedicated supply)
   SIM800L GND   -> Ground rail
   Servo VCC     -> 5V rail (all 5 servos)
   Servo GND     -> Ground rail (all 5 servos)
   ```

**Safety Check**:
- Verify all connections before applying power
- Check for short circuits with multimeter
- Ensure proper polarity on all connections
- SIM800L requires clean power supply (add capacitors)

### Step 2: I2C Bus Configuration

**Objective**: Establish I2C communication bus for digital components

**Components**: ESP32, PCA9685, LCD Display

**Wiring Diagram**:
```
ESP32 GPIO 21 (SDA) -> PCA9685 SDA -> LCD SDA
ESP32 GPIO 22 (SCL) -> PCA9685 SCL -> LCD SCL

Add 4.7kΩ pullup resistors:
- SDA line to 3.3V
- SCL line to 3.3V
```

**I2C Address Configuration**:
- PCA9685 Servo Driver: 0x40 (default)
- LCD Display: 0x27 or 0x3F (check with I2C scanner)

### Step 3: Serial Communication Setup

**Objective**: Connect GSM module for SMS notifications

**SIM800L Connections**:
```
SIM800L TX  -> ESP32 GPIO 16 (RX)
SIM800L RX  -> ESP32 GPIO 17 (TX)
SIM800L RST -> ESP32 GPIO 4 (Reset)
SIM800L GND -> Ground rail
```

**SIM Card Setup**:
- Insert activated SIM card into SIM800L
- Ensure SIM card has SMS capability
- Disable PIN if required
- Connect GSM antenna

### Step 4: Analog Sensor Connection

**Objective**: Connect battery monitoring circuit

**Voltage Sensor Connections**:
```
Battery + -> Voltage Divider -> ESP32 GPIO 34 (ADC)
Battery - -> Ground rail
```

**Voltage Divider Circuit**:
```
Battery + --[10kΩ]--|--[10kΩ]-- GND
                   |
                   +--> ESP32 GPIO 34
```

**Calibration**:
- Ratio: 5.0 (10kΩ + 10kΩ) / 10kΩ = 2:1, but effective ratio is 5:1 due to ESP32 ADC
- Input range: 9.0V - 12.6V (3S Li-ion)
- ADC reading: ~372 (9V) to ~516 (12.6V)

### Step 5: Servo Motor Connections

**Objective**: Connect dispenser servo motors

**Servo Connections**:
```
PCA9685 Channel 0 -> Dispenser 0 Servo
PCA9685 Channel 1 -> Dispenser 1 Servo
PCA9685 Channel 2 -> Dispenser 2 Servo
PCA9685 Channel 3 -> Dispenser 3 Servo
PCA9685 Channel 4 -> Dispenser 4 Servo
```

**Servo Power**:
- Signal wires: Connected to PCA9685 channels
- Power wires: Connected to external 5V 2-3A supply
- Ground wires: Connected to common ground

### Step 6: Status LED Connection

**Objective**: Connect status indicator

**LED Connection**:
```
ESP32 GPIO 2 -> 220Ω Resistor -> LED Anode
LED Cathode -> Ground rail
```

## Software Installation

### ESP32 Firmware Installation

**Prerequisites**:
- Arduino IDE 2.0+ installed
- ESP32 board support package installed
- All required libraries installed

**Library Installation**:
1. Open Arduino IDE
2. Go to Tools → Manage Libraries
3. Install the following libraries:
   - Time by Michael Margolis
   - TimeAlarms by Michael Margolis
   - Adafruit PWM Servo Driver Library
   - LiquidCrystal I2C by Frank de Brabander
   - Firebase ESP Client by Mobizt

**Firmware Upload**:
1. Open `source/esp32/PillDispenser/PillDispenser.ino`
2. Select board: ESP32 Dev Module
3. Configure settings:
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Frequency: 80MHz
   - Partition Scheme: Default 4MB
4. Click Upload

### Web Dashboard Installation

**Prerequisites**:
- Node.js 18+ installed
- npm package manager

**Installation Steps**:
```bash
cd source/web
npm install
```

**Environment Configuration**:
Create `.env.local` with Firebase credentials:
```bash
NEXT_PUBLIC_FIREBASE_API_KEY=your_api_key
NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your-project.firebaseapp.com
NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://your-project-default-rtdb.firebaseio.com
NEXT_PUBLIC_FIREBASE_PROJECT_ID=your-project-id
NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your-project.appspot.com
NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=123456789
NEXT_PUBLIC_FIREBASE_APP_ID=1:123456789:web:abcdef
```

**Development Server**:
```bash
npm run dev
```

**Production Build**:
```bash
npm run build
npm start
```

## System Configuration

### ESP32 Configuration

**WiFi Setup**:
Edit `PillDispenser.ino`:
```cpp
const String WIFI_SSID = "YourWiFiName";
const String WIFI_PASSWORD = "YourWiFiPassword";
```

**Firebase Service Account**:
Follow `FIREBASE_SERVICE_ACCOUNT_SETUP.md` to configure service account authentication.

**Phone Numbers for SMS**:
```cpp
notificationManager.addPhoneNumber("+1234567890", "Caregiver");
```

**Device Identification**:
```cpp
const String DEVICE_ID = "PILL_DISPENSER_V3";
```

### Firebase Database Rules

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

### Web Dashboard Configuration

**Firebase Configuration**:
Update `source/web/src/lib/firebase.ts` with your Firebase config.

**Authentication Setup**:
- Enable Email/Password authentication in Firebase Console
- Configure authorized domains

## Production Setup

### ESP32 Production Mode

**Enable Production Mode**:
Edit `PillDispenser.ino`:
```cpp
#define DEVELOPMENT_MODE false
```

**Production Features**:
- Schedule enforcement active
- SMS notifications enabled
- Development commands locked
- Automatic error recovery

### Web Dashboard Deployment

**Vercel Deployment** (Recommended):
```bash
npm i -g vercel
vercel --prod
```

**Manual Deployment**:
```bash
npm run build
npm start
```

### Monitoring Setup

**Firebase Monitoring**:
- Enable Firebase Analytics (optional)
- Set up billing alerts
- Monitor Realtime Database usage

**System Monitoring**:
- ESP32 sends heartbeat every 30 seconds
- Battery status updates every minute
- Dispense events logged in real-time

## Quality Assurance

### ESP32 Testing

**Component Tests**:
```
status              # Check all systems
battery             # Verify battery monitoring
test dispenser 0    # Test servo motors
test sim800         # Test GSM module
test firebase       # Test cloud connectivity
```

**Integration Tests**:
- Create schedule via web app
- Verify ESP32 receives schedule
- Test automatic dispensing
- Verify SMS notifications

### Web Dashboard Testing

**Authentication Tests**:
- User registration/login
- Password reset functionality
- Session management

**Schedule Management Tests**:
- Add/edit/delete schedules
- Real-time sync with ESP32
- Schedule validation

**Monitoring Tests**:
- Real-time status updates
- Battery level display
- Activity log viewing

### Performance Testing

**ESP32 Performance**:
- Memory usage monitoring
- WiFi stability testing
- NTP sync reliability
- Firebase connection stability

**Web Dashboard Performance**:
- Page load times
- Real-time update latency
- Concurrent user handling

## Go-Live Procedures

### Pre-Launch Checklist

**Hardware Verification**:
- [ ] All connections secure
- [ ] Power supplies stable
- [ ] SIM card activated
- [ ] Antenna connected
- [ ] Battery voltage normal

**Software Verification**:
- [ ] Firmware uploaded successfully
- [ ] WiFi connected
- [ ] Firebase authenticated
- [ ] Time synchronized
- [ ] All components initialized

**Configuration Verification**:
- [ ] WiFi credentials correct
- [ ] Firebase credentials valid
- [ ] Phone numbers configured
- [ ] Device ID set

### Launch Sequence

1. **Power On System**:
   - Connect power supplies
   - Monitor serial output
   - Verify initialization sequence

2. **Initial Testing**:
   - Run status command
   - Test manual dispense
   - Verify SMS capability

3. **Web Dashboard Setup**:
   - Access dashboard URL
   - Create administrator account
   - Configure initial settings

4. **First Schedule Creation**:
   - Add test schedule
   - Verify sync with ESP32
   - Test automatic dispensing

5. **System Validation**:
   - Monitor for 24 hours
   - Verify all notifications
   - Check battery monitoring

### Rollback Procedures

**Firmware Rollback**:
- Keep backup of previous firmware version
- Use Arduino IDE to upload previous version
- Verify system functionality

**Configuration Rollback**:
- Backup configuration files
- Document all changes
- Have restore procedures ready

## Post-Deployment Support

### Monitoring Procedures

**Daily Checks**:
- Battery level monitoring
- Schedule execution verification
- SMS delivery confirmation
- Firebase connectivity status

**Weekly Maintenance**:
- Firmware update checks
- Database cleanup
- Performance monitoring
- User feedback review

**Monthly Reviews**:
- System reliability assessment
- Feature usage analysis
- Security update implementation
- Documentation updates

### User Support

**Common Issues**:
- WiFi connectivity problems
- Schedule not executing
- SMS delivery failures
- Battery monitoring issues

**Support Channels**:
- User documentation
- Troubleshooting guides
- Issue tracking system
- Direct technical support

### System Updates

**Firmware Updates**:
- Regular security patches
- Feature enhancements
- Bug fixes
- Performance improvements

**Web Dashboard Updates**:
- UI/UX improvements
- New features
- Security updates
- Compatibility fixes

### Backup and Recovery

**Data Backup**:
- Firebase automatic backups
- Configuration file backups
- User data export capabilities

**Disaster Recovery**:
- System restore procedures
- Data recovery processes
- Failover mechanisms
- Business continuity plans

---

**Last Updated**: December 2025
**Version**: 3.0.0

**Tools Required**:
- [ ] Soldering iron and solder
- [ ] Wire strippers and cutters
- [ ] Multimeter for testing
- [ ] Screwdrivers and hex keys
- [ ] Computer for programming

## Hardware Assembly

### Step 1: Power Distribution Setup

**Objective**: Establish stable power distribution for all components

**Components**: Power supply, breadboard/PCB, jumper wires

**Procedure**:

1. **Main Power Rails**:
   ```
   Connect 5V power supply to:
   - Positive rail: 5V output
   - Negative rail: Ground (GND)
   ```

2. **ESP32 Power Connection**:
   ```
   ESP32 VIN  -> 5V power rail
   ESP32 GND  -> Ground rail
   ```

3. **Component Power Distribution**:
   ```
   PCA9685 VCC   -> 5V rail
   PCA9685 GND   -> Ground rail
   LCD VCC       -> 5V rail
   LCD GND       -> Ground rail
   DS1302 VCC    -> 5V rail
   DS1302 GND    -> Ground rail
   SIM800L VCC   -> 5V rail (through 3.3V regulator)
   SIM800L GND   -> Ground rail
   IR Sensors VCC -> 5V rail (all 3 sensors)
   IR Sensors GND -> Ground rail (all 3 sensors)
   ```

**Safety Check**:
- Verify all connections before applying power
- Check for short circuits with multimeter
- Ensure proper polarity on all connections

### Step 2: I2C Bus Configuration

**Objective**: Establish I2C communication bus for digital components

**Components**: ESP32, PCA9685, LCD Display

**Wiring Diagram**:
```
ESP32 Pin 21 (SDA) -> PCA9685 SDA -> LCD SDA
ESP32 Pin 22 (SCL) -> PCA9685 SCL -> LCD SCL

Add 4.7kΩ pullup resistors:
- SDA line to 5V
- SCL line to 5V
```

**I2C Address Configuration**:
- PCA9685 Servo Driver: 0x40 (default)
- LCD Display: 0x27 or 0x3F (check with I2C scanner)

### Step 3: Digital I/O Connections

**Objective**: Connect digital sensors and control signals

**IR Sensor Connections**:
```
IR Sensor 1 (Entrance):
- VCC -> 5V rail
- GND -> Ground rail
- OUT -> ESP32 GPIO 34

IR Sensor 2 (Middle):
- VCC -> 5V rail
- GND -> Ground rail
- OUT -> ESP32 GPIO 35

IR Sensor 3 (Exit):
- VCC -> 5V rail
- GND -> Ground rail
- OUT -> ESP32 GPIO 32
```

**DS1302 RTC Connections**:
```
DS1302 CLK (Clock)  -> ESP32 GPIO 18
DS1302 DAT (Data)   -> ESP32 GPIO 19
DS1302 RST (Reset)  -> ESP32 GPIO 5
DS1302 VCC          -> 5V rail
DS1302 GND          -> Ground rail
```

**SIM800L GSM Module Connections**:
```
SIM800L RX  -> ESP32 GPIO 17 (TX)
SIM800L TX  -> ESP32 GPIO 16 (RX)
SIM800L RST -> ESP32 GPIO 4
SIM800L VCC -> 3.7-4.2V (use voltage regulator)
SIM800L GND -> Ground rail
```

### Step 4: Servo Motor Setup

**Objective**: Connect servo motors to PCA9685 driver

**Servo Connections**:
```
Servo Power (Red):    -> External 5V supply
Servo Ground (Black): -> Common ground
Servo Signal (White): -> PCA9685 PWM outputs

Channel Assignment:
- Channel 0: Pill Dispenser 1
- Channel 1: Pill Dispenser 2
- Channel 2: Pill Dispenser 3
- ... (up to 16 channels)
```

**Important Notes**:
- Use external power supply for servos (separate from logic power)
- Common ground between ESP32 and servo power supply
- Adequate current capacity for all servos

### Step 5: Assembly Verification

**Power-On Test**:
1. Apply power to system
2. Verify LED indicators on all modules
3. Check voltage levels with multimeter
4. Monitor for abnormal heating

**Communication Test**:
1. Upload basic I2C scanner sketch
2. Verify all I2C devices detected
3. Test basic GPIO pin functionality
4. Verify serial communication with SIM800L

## Software Installation

### Step 1: Development Environment Setup

**Arduino IDE Installation**:

1. **Download Arduino IDE**:
   - Visit https://www.arduino.cc/en/software
   - Download Arduino IDE 1.8.19 or newer
   - Install with default settings

2. **ESP32 Board Package Installation**:
   ```
   File -> Preferences
   Additional Boards Manager URLs:
   https://dl.espressif.com/dl/package_esp32_index.json
   
   Tools -> Board -> Boards Manager
   Search: "ESP32"
   Install: "ESP32 by Espressif Systems"
   ```

3. **Board Configuration**:
   ```
   Tools -> Board -> ESP32 Arduino -> ESP32 Dev Module
   Tools -> Upload Speed -> 115200
   Tools -> CPU Frequency -> 240MHz (WiFi/BT)
   Tools -> Flash Frequency -> 80MHz
   Tools -> Flash Mode -> QIO
   Tools -> Flash Size -> 4MB
   Tools -> Partition Scheme -> Default 4MB with spiffs
   ```

### Step 2: Library Installation

**Required Libraries Installation**:

1. **Adafruit PWM Servo Driver**:
   ```
   Tools -> Manage Libraries
   Search: "Adafruit PWM Servo Driver"
   Install: Latest version by Adafruit
   ```

2. **LiquidCrystal I2C**:
   ```
   Search: "LiquidCrystal I2C"
   Install: by Frank de Brabander
   ```

3. **RtcDS1302**:
   ```
   Search: "RtcDS1302"
   Install: by Makuna
   ```

4. **Firebase ESP Client**:
   ```
   Search: "Firebase ESP Client"
   Install: by Mobizt
   ```

5. **Additional Dependencies**:
   ```
   ArduinoJson (automatically installed with Firebase)
   ESP32 Time library (built-in)
   WiFi library (built-in)
   SoftwareSerial (built-in)
   ```

**Library Verification**:
- Compile a simple sketch including all libraries
- Verify no compilation errors
- Check library versions are compatible

### Step 3: Source Code Deployment

**File Structure Setup**:

1. **Create Project Directory**:
   ```
   PillDispenser/
   ├── PillDispenser.ino
   ├── ServoDriver.h
   ├── ServoDriver.cpp
   ├── IRSensor.h
   ├── IRSensor.cpp
   ├── LCDDisplay.h
   ├── LCDDisplay.cpp
   ├── RTClock.h
   ├── RTClock.cpp
   ├── FirebaseManager.h
   ├── FirebaseManager.cpp
   ├── SIM800L.h
   └── SIM800L.cpp
   ```

2. **Copy Source Files**:
   - Copy all source files to project directory
   - Verify file integrity and completeness
   - Check for any missing dependencies

3. **Configuration Updates**:
   - Update WiFi credentials in main file
   - Set Firebase API key and database URL
   - Configure device-specific settings

### Step 4: Initial Compilation and Upload

**Pre-Compilation Checks**:

1. **Hardware Configuration**:
   ```cpp
   // Verify pin assignments match hardware
   #define IR_SENSOR_1_PIN 34
   #define IR_SENSOR_2_PIN 35
   #define IR_SENSOR_3_PIN 32
   #define RTC_CLK_PIN 18
   #define RTC_DAT_PIN 19
   #define RTC_RST_PIN 5
   #define SIM800L_RX_PIN 16
   #define SIM800L_TX_PIN 17
   #define SIM800L_RST_PIN 4
   ```

2. **Development Mode Configuration**:
   ```cpp
   // Enable for initial deployment
   #define DEVELOPMENT_MODE true
   ```

**Compilation Process**:

1. **Open Project**:
   - Open PillDispenser.ino in Arduino IDE
   - Verify all .h and .cpp files are detected

2. **Compile Code**:
   ```
   Sketch -> Verify/Compile
   ```
   - Check for compilation errors
   - Resolve any library or syntax issues

3. **Upload to ESP32**:
   ```
   Connect ESP32 via USB
   Select correct COM port
   Sketch -> Upload
   ```

**Post-Upload Verification**:
- Open Serial Monitor (115200 baud)
- Verify initialization messages
- Check for any error codes
- Confirm component detection

## System Configuration

### Step 1: Network Configuration

**WiFi Setup**:

1. **Credentials Configuration**:
   ```cpp
   // Update in main configuration section
   const char* WIFI_SSID = "YourNetworkName";
   const char* WIFI_PASSWORD = "YourNetworkPassword";
   ```

2. **Connection Verification**:
   - Monitor serial output for WiFi connection status
   - Verify IP address assignment
   - Test internet connectivity

**Firebase Configuration**:

1. **Project Setup**:
   - Create Firebase project at https://console.firebase.google.com
   - Enable Realtime Database
   - Set database rules for device access

2. **Credentials Configuration**:
   ```cpp
   // Update Firebase credentials
   #define FIREBASE_API_KEY "your-api-key-here"
   #define FIREBASE_DATABASE_URL "https://your-project.firebaseio.com/"
   ```

3. **Database Structure Setup**:
   ```json
   {
     "devices": {
       "device_id": {
         "status": "ready",
         "last_heartbeat": timestamp,
         "pill_logs": {
           "log_id": {
             "timestamp": "datetime",
             "pill_count": number,
             "description": "text"
           }
         },
         "commands": {
           "dispense": false,
           "reset": false
         },
         "schedule": {
           "morning": "08:00",
           "afternoon": "14:00",
           "evening": "20:00"
         }
       }
     }
   }
   ```

### Step 2: Real-Time Clock Configuration

**Time Setting**:

1. **Manual Time Setting**:
   ```cpp
   // Use serial command interface
   // Send: "rtc_set YYYY MM DD HH MM SS"
   // Example: "rtc_set 2025 11 01 14 30 00"
   ```

2. **Network Time Synchronization** (if available):
   ```cpp
   // Automatic NTP synchronization
   // Implemented in production version
   ```

3. **Time Verification**:
   - Check current time display on LCD
   - Verify time progression accuracy
   - Test scheduling functionality

### Step 3: Component Calibration

**Servo Calibration**:

1. **Position Calibration**:
   ```
   Serial Command: "servo_test 0"
   - Verify 0° position (fully closed)
   - Verify 90° position (half open)
   - Verify 180° position (fully open)
   ```

2. **Dispensing Calibration**:
   ```
   Serial Command: "dispense_test 0 small"
   Serial Command: "dispense_test 0 medium"
   Serial Command: "dispense_test 0 large"
   ```
   - Adjust timing parameters if needed
   - Verify consistent pill dispensing

**Sensor Calibration**:

1. **IR Sensor Sensitivity**:
   ```
   Serial Command: "ir_test"
   - Test with actual pills
   - Verify detection range (2-10cm)
   - Adjust positioning if needed
   ```

2. **Detection Threshold Adjustment**:
   - Fine-tune debouncing parameters
   - Test false positive rejection
   - Verify reliable detection

### Step 4: Communication Testing

**GSM Configuration**:

1. **SIM Card Setup**:
   - Insert activated SIM card
   - Verify PIN if required
   - Check network provider compatibility

2. **Network Registration**:
   ```
   Serial Command: "sim_test"
   - Verify network registration
   - Check signal strength
   - Test SMS functionality
   ```

3. **Emergency Contact Configuration**:
   ```cpp
   // Configure emergency contacts
   const char* EMERGENCY_CONTACT = "+1234567890";
   const char* CAREGIVER_CONTACT = "+0987654321";
   ```

## Production Setup

### Step 1: Security Configuration

**Network Security**:

1. **WiFi Security**:
   - Use WPA2/WPA3 encryption
   - Strong network passwords
   - Regular password rotation

2. **Firebase Security Rules**:
   ```json
   {
     "rules": {
       "devices": {
         "$device_id": {
           ".read": "auth != null",
           ".write": "auth != null && auth.uid == $device_id"
         }
       }
     }
   }
   ```

3. **Authentication Implementation**:
   ```cpp
   // Device-specific authentication
   // Unique device tokens
   // Secure credential storage
   ```

**Physical Security**:

1. **Enclosure Security**:
   - Tamper-resistant enclosure
   - Locked access panels
   - Security seal indicators

2. **Component Protection**:
   - Secure mounting of all components
   - Cable strain relief
   - Environmental protection

### Step 2: Production Mode Configuration

**Mode Switch**:

1. **Development to Production**:
   ```cpp
   // Change in main configuration
   #define DEVELOPMENT_MODE false
   ```

2. **Logging Configuration**:
   ```cpp
   // Reduce verbose output
   // Enable error-only logging
   // Optimize performance settings
   ```

3. **Watchdog Timer**:
   ```cpp
   // Enable automatic reset on hang
   // Configure timeout periods
   // Implement health monitoring
   ```

### Step 3: Data Management

**Local Data Storage**:

1. **EEPROM Configuration**:
   ```cpp
   // Store critical settings
   // Backup schedules
   // Emergency contacts
   ```

2. **Data Retention Policies**:
   - Local log retention: 30 days
   - Cloud synchronization: Real-time
   - Backup frequency: Daily

**Cloud Data Management**:

1. **Data Synchronization**:
   - Real-time upload for critical events
   - Batch upload for historical data
   - Conflict resolution procedures

2. **Data Backup**:
   - Automated cloud backups
   - Multiple backup locations
   - Recovery procedures

### Step 4: Monitoring and Alerts

**Health Monitoring**:

1. **System Health Checks**:
   ```cpp
   // Component status monitoring
   // Performance metrics collection
   // Error rate tracking
   ```

2. **Alert Configuration**:
   - Critical error notifications
   - Maintenance reminders
   - Low battery warnings

**Remote Monitoring**:

1. **Dashboard Setup**:
   - Real-time status display
   - Historical data visualization
   - Trend analysis

2. **Notification System**:
   - SMS alerts for critical events
   - Email notifications for maintenance
   - Push notifications to mobile app

## Quality Assurance

### Step 1: Comprehensive Testing

**System Integration Testing**:

1. **End-to-End Testing**:
   ```
   Test Sequence:
   1. Power on system
   2. Verify initialization
   3. Test manual dispensing
   4. Verify data logging
   5. Test scheduled dispensing
   6. Verify notifications
   7. Test error recovery
   ```

2. **Performance Testing**:
   - Response time measurements
   - Memory usage monitoring
   - Network latency testing
   - Battery life assessment

**Safety Testing**:

1. **Overdose Prevention**:
   - Test rapid command rejection
   - Verify scheduling constraints
   - Test manual override limits

2. **Fail-Safe Testing**:
   - Power failure scenarios
   - Network disconnection handling
   - Component failure responses

### Step 2: User Acceptance Testing

**Operational Testing**:

1. **Daily Use Simulation**:
   - 24-hour continuous operation
   - Multiple dispensing cycles
   - Various user scenarios

2. **Edge Case Testing**:
   - Power interruptions
   - Network outages
   - Time zone changes
   - Daylight saving time

**Usability Testing**:

1. **User Interface Testing**:
   - LCD display readability
   - Status indication clarity
   - Error message comprehension

2. **Maintenance Procedures**:
   - Pill refill procedures
   - System reset operations
   - Basic troubleshooting

### Step 3: Documentation Verification

**Technical Documentation**:

1. **Installation Guide Review**:
   - Step-by-step accuracy
   - Required tools and materials
   - Safety precautions

2. **User Manual Validation**:
   - Operation procedures
   - Troubleshooting guides
   - Maintenance instructions

**Compliance Documentation**:

1. **Safety Certifications**:
   - Electrical safety compliance
   - Medical device regulations
   - Data privacy compliance

2. **Quality Standards**:
   - ISO 13485 (Medical Devices)
   - IEC 62304 (Medical Device Software)
   - FDA guidelines (if applicable)

## Go-Live Procedures

### Step 1: Pre-Production Checklist

**Final System Verification**:

- [ ] All hardware components properly installed
- [ ] Software uploaded and configured
- [ ] Network connectivity verified
- [ ] Cloud services operational
- [ ] All sensors calibrated
- [ ] Emergency contacts configured
- [ ] Documentation complete
- [ ] User training completed

**Stakeholder Approval**:

- [ ] Technical team sign-off
- [ ] Quality assurance approval
- [ ] User acceptance confirmation
- [ ] Regulatory compliance verified
- [ ] Support team readiness

### Step 2: Production Deployment

**Phased Rollout**:

1. **Phase 1: Limited Deployment**:
   - Deploy to controlled environment
   - Monitor for 48 hours
   - Gather initial feedback
   - Address any issues

2. **Phase 2: Extended Testing**:
   - Expand to broader test group
   - Monitor for 1 week
   - Performance optimization
   - User training refinement

3. **Phase 3: Full Production**:
   - Complete deployment
   - 24/7 monitoring setup
   - Support team activation
   - User training completion

**Launch Activities**:

1. **System Activation**:
   ```
   1. Final hardware inspection
   2. Power on and initialization
   3. Network connectivity test
   4. Cloud synchronization verification
   5. Complete system test cycle
   ```

2. **User Handover**:
   - System demonstration
   - Training completion
   - Documentation delivery
   - Emergency contact setup

### Step 3: Initial Monitoring Period

**First 24 Hours**:

- [ ] Continuous system monitoring
- [ ] Real-time performance tracking
- [ ] User support availability
- [ ] Issue escalation procedures
- [ ] Regular status updates

**First Week**:

- [ ] Daily system health checks
- [ ] User feedback collection
- [ ] Performance optimization
- [ ] Documentation updates
- [ ] Support process refinement

**First Month**:

- [ ] Weekly system reviews
- [ ] Performance trend analysis
- [ ] User satisfaction surveys
- [ ] Maintenance scheduling
- [ ] Long-term optimization planning

## Post-Deployment Support

### Step 1: Ongoing Support Structure

**Technical Support Levels**:

1. **Level 1: Basic Support**:
   - User operation questions
   - Basic troubleshooting
   - System status inquiries
   - Schedule modifications

2. **Level 2: Advanced Support**:
   - Component failure diagnosis
   - Configuration changes
   - Software updates
   - Performance optimization

3. **Level 3: Engineering Support**:
   - Hardware modifications
   - Software development
   - System redesign
   - Integration issues

**Support Channels**:

- Phone support: Primary contact method
- Email support: Non-urgent inquiries
- Remote assistance: System diagnostics
- On-site support: Hardware issues

### Step 2: Maintenance Procedures

**Scheduled Maintenance**:

1. **Daily Tasks**:
   - Visual system inspection
   - Status indicator check
   - Error log review
   - Basic functionality test

2. **Weekly Tasks**:
   - Complete system test
   - Component calibration check
   - Data backup verification
   - Performance monitoring

3. **Monthly Tasks**:
   - Comprehensive system check
   - Software update evaluation
   - Hardware inspection
   - User training refresher

4. **Quarterly Tasks**:
   - Complete system overhaul
   - Component replacement assessment
   - Performance optimization
   - Security audit

**Preventive Maintenance**:

1. **Component Replacement Schedule**:
   - Servo motors: 2-3 years
   - Sensors: 3-5 years
   - LCD display: 5-7 years
   - RTC battery: 2-3 years

2. **Calibration Schedule**:
   - Servo positioning: Monthly
   - Sensor sensitivity: Quarterly
   - Time synchronization: Monthly
   - Communication settings: As needed

### Step 3: Continuous Improvement

**Performance Monitoring**:

1. **Key Performance Indicators**:
   - System uptime percentage
   - Dispensing accuracy rate
   - Communication reliability
   - User satisfaction scores

2. **Data Collection**:
   - Usage pattern analysis
   - Error frequency tracking
   - Performance trend monitoring
   - User feedback compilation

**Update Management**:

1. **Software Updates**:
   - Security patches: As needed
   - Feature updates: Quarterly
   - Bug fixes: As required
   - Library updates: Annually

2. **Hardware Updates**:
   - Component upgrades: As available
   - System expansion: Based on needs
   - Technology refresh: Every 3-5 years

**User Training and Support**:

1. **Training Programs**:
   - Initial user training
   - Refresher courses
   - New feature training
   - Advanced operation training

2. **Documentation Maintenance**:
   - Regular updates to user manuals
   - Troubleshooting guide expansion
   - FAQ development
   - Video tutorial creation

This comprehensive deployment guide ensures successful implementation and ongoing operation of the Pill Dispenser V3 system with proper support and maintenance procedures.