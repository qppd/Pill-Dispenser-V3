# Pill Dispenser V3

**An Advanced Multi-Component IoT Automated Medication Dispensing System**

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware Requirements](#hardware-requirements)
- [Software Dependencies](#software-dependencies)
- [Installation Guide](#installation-guide)
- [Configuration](#configuration)
- [API Reference](#api-reference)
- [Development Mode](#development-mode)
- [Testing Framework](#testing-framework)
- [Deployment](#deployment)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Changelog](#changelog)

## Overview

The Pill Dispenser V3 is a sophisticated, multi-component IoT-enabled medication dispensing system designed for healthcare applications. The system integrates ESP32 microcontrollers, Raspberry Pi machine learning processing, ESP32-CAM image capture, and a modern Next.js web dashboard to provide comprehensive automated pill dispensing with real-time monitoring, cloud connectivity, and advanced safety features.

### Key Capabilities

- **Automated Dispensing**: Precision servo-controlled pill dispensing with multiple medication types
- **AI-Powered Detection**: Machine learning-based pill verification using YOLOv8
- **Real-time Monitoring**: Continuous sensor monitoring and status reporting
- **Cloud Integration**: Firebase-based data logging and remote monitoring
- **Communication**: GSM/GPRS connectivity for alerts and notifications
- **User Interface**: Modern web dashboard with authentication and scheduling
- **Safety Features**: Multiple IR sensors for pill detection and verification
- **Modular Architecture**: Independent components for scalability and maintenance

## Features

### Core Functionality

- **Multi-Channel Dispensing**: Support for up to 16 independent servo channels
- **Pill Size Recognition**: Configurable timing for small, medium, and large pills
- **AI Verification**: Computer vision-based pill detection and counting
- **Real-time Clock**: Accurate timekeeping with DS1302 RTC module
- **Sensor Array**: Triple IR sensor system for pill detection and verification
- **Cloud Connectivity**: Real-time data streaming and remote command execution
- **GSM Communication**: SMS notifications and remote monitoring capabilities
- **Web Dashboard**: User-friendly interface for monitoring and control

### Advanced Features

- **Modular Architecture**: Object-oriented design with independent component classes
- **Machine Learning**: YOLOv8-based pill detection with ensemble voting
- **Development Mode**: Comprehensive testing framework with serial command interface
- **Error Handling**: Robust error detection and recovery mechanisms
- **I2C Device Management**: Automatic device discovery and status monitoring
- **Remote Configuration**: Cloud-based configuration updates and scheduling
- **Multi-Device Coordination**: ESP32, Raspberry Pi, and ESP32-CAM integration

## System Architecture

### Component Overview

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   ESP32 Main    │    │  PCA9685 PWM    │    │   Servo Motors  │
│   Controller    │◄──►│    Driver       │◄──►│   (16 Channel)  │
│                 │    │                 │    │                 │
│ - Pill Dispensing│    │ - Servo Control │    │ - Medication   │
│ - Sensor Monitoring│   │ - I2C Interface │   │   Delivery     │
│ - LCD Display    │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │
         ├─── I2C Bus ──┬── LCD Display (20x4)
         │              └── PCA9685 Driver
         │
         ├─── GPIO ─────┬── IR Sensor 1 (Entrance)
         │              ├── IR Sensor 2 (Middle)
         │              └── IR Sensor 3 (Exit)
         │
         ├─── SPI ──────── DS1302 RTC Module
         │
         ├─── UART ─────── SIM800L GSM Module
         │
         ├─── WiFi ─────── Firebase Cloud Service
         │
         └─── Serial ───── Raspberry Pi (ML Processing)
```

### Software Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    User Interface Layer                     │
├─────────────────────────────────────────────────────────────┤
│              Next.js Web Dashboard                          │
│            (Authentication, Monitoring, Control)           │
├─────────────────────────────────────────────────────────────┤
│                    Cloud Services Layer                     │
├─────────────────────────────────────────────────────────────┤
│              Firebase Realtime Database                     │
│            (Data Storage, Real-time Sync)                   │
├─────────────────────────────────────────────────────────────┤
│                    Processing Layer                         │
├─────────────────────────────────────────────────────────────┤
│   Raspberry Pi 4     │   ESP32-CAM        │   ESP32 Main    │
│   - YOLOv8 ML        │   - Image Capture  │   - Dispensing  │
│   - Pill Detection   │   - HTTP Stream    │   - Sensors     │
│   - Data Analysis    │   - Flash Control  │   - Communication│
├─────────────────────────────────────────────────────────────┤
│                    Hardware Layer                           │
│        (Arduino Core, OpenCV, Firebase SDK)                 │
└─────────────────────────────────────────────────────────────┘
```

## Hardware Requirements

### ESP32 Main Controller Components

| Component | Specification | Quantity | Purpose |
|-----------|---------------|----------|---------|
| ESP32 Development Board | ESP32-WROOM-32 | 1 | Main microcontroller for dispensing |
| PCA9685 PWM Driver | 16-Channel, 12-bit | 1 | Servo motor control |
| Servo Motors | MG996R or equivalent | 1-16 | Pill dispensing mechanism |
| IR Obstacle Sensors | HC-SR04 compatible | 3 | Pill detection |
| LCD Display | I2C 20x4 Character Display | 1 | Status information |
| RTC Module | DS1302 Real-time Clock | 1 | Timing and scheduling |
| GSM Module | SIM800L | 1 | Communication |

### Raspberry Pi ML Processing Components

| Component | Specification | Quantity | Purpose |
|-----------|---------------|----------|---------|
| Raspberry Pi 4 | 4GB RAM or higher | 1 | Machine learning processing |
| MicroSD Card | 32GB Class 10 | 1 | OS and storage |
| Power Supply | 5V 3A USB-C | 1 | Raspberry Pi power |
| Camera Cable | Raspberry Pi Camera Cable | 1 | ESP32-CAM connection |

### ESP32-CAM Image Capture Components

| Component | Specification | Quantity | Purpose |
|-----------|---------------|----------|---------|
| ESP32-CAM Module | AI-Thinker or compatible | 1 | Image capture for pill detection |
| Antenna | External WiFi antenna | 1 | Improved connectivity |
| Power Supply | 3.3V-5V regulated | 1 | Camera module power |

### General Components

| Component | Specification | Quantity | Purpose |
|-----------|---------------|----------|---------|
| Power Supply | 5V DC 3A+ | 1 | Main system power |
| Jumper Wires | Male-Female, Male-Male | Various | Connections |
| Breadboard | Full-size | 1 | Prototyping |
| Enclosure | 3D printed or plastic | 1 | System housing |

### Power Requirements

- **ESP32 System**: 5V DC, 2-5A (depending on servo load)
- **Raspberry Pi**: 5V DC, 3A via USB-C
- **ESP32-CAM**: 3.3V-5V DC, 500mA
- **Backup Power**: CR2032 battery for RTC

### Connectivity Requirements

- **WiFi Network**: 2.4GHz IEEE 802.11 b/g/n for ESP32 devices
- **Ethernet**: Optional for Raspberry Pi (recommended for stability)
- **Internet Access**: Required for Firebase connectivity
- **SIM Card**: For GSM functionality (optional)

### Wiring Diagrams

The wiring diagrams for the system are available in the `diagram/` directory:

- `Wiring.fzz`: Fritzing diagram file for circuit design
- `Wiring.png`: PNG image of the wiring diagram

## Software Dependencies

### ESP32 Arduino Dependencies

```cpp
// Core Libraries
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

// Component Libraries
#include <Adafruit_PWMServoDriver.h>  // Servo control
#include <LiquidCrystal_I2C.h>        // LCD display
#include <Firebase_ESP_Client.h>      // Firebase connectivity
#include <ThreeWire.h>                // DS1302 communication
#include <RtcDS1302.h>                // DS1302 RTC
#include <SoftwareSerial.h>           // SIM800L communication
```

### Raspberry Pi Dependencies

```python
# Core Python packages
opencv-python>=4.8.0
numpy>=1.24.0
pillow>=10.0.0
requests>=2.31.0
pyserial>=3.5

# Machine Learning
ultralytics>=8.0.0
torch>=2.0.0
torchvision>=0.15.0

# Data processing
pandas>=2.0.0
sqlite3  # Built-in

# System
RPi.GPIO>=0.7.0
```

### Web Application Dependencies

```json
{
  "next": ">=16.0.0",
  "react": ">=19.0.0",
  "firebase": ">=12.6.0",
  "lucide-react": ">=0.554.0",
  "tailwindcss": ">=4.0.0"
}
```

### Required Software Versions

#### ESP32 Libraries
- **Adafruit PWM Servo Driver Library**: >= 2.4.0
- **LiquidCrystal I2C**: >= 1.1.2
- **Firebase ESP32 Client**: >= 4.3.0
- **RTC Library**: >= 2.3.5

#### Raspberry Pi
- **Python**: 3.9+
- **OpenCV**: 4.8+
- **PyTorch**: 2.0+
- **YOLOv8**: Latest ultralytics

#### Web Application
- **Node.js**: 18+
- **Next.js**: 16+
- **React**: 19+
- **Firebase SDK**: 12.6+

## Installation Guide

### 1. Hardware Setup

#### Pin Configuration

| Component | ESP32 Pin | Function |
|-----------|-----------|----------|
| I2C SDA | GPIO 21 | I2C Data |
| I2C SCL | GPIO 22 | I2C Clock |
| IR Sensor 1 | GPIO 34 | Entrance Detection |
| IR Sensor 2 | GPIO 35 | Middle Detection |
| IR Sensor 3 | GPIO 32 | Exit Detection |
| DS1302 CLK | GPIO 18 | RTC Clock |
| DS1302 DAT | GPIO 19 | RTC Data |
| DS1302 RST | GPIO 5 | RTC Reset |
| SIM800L RX | GPIO 16 | GSM Receive |
| SIM800L TX | GPIO 17 | GSM Transmit |
| SIM800L RST | GPIO 4 | GSM Reset |
| Status LED | GPIO 2 | System Status |

#### I2C Device Addresses

| Device | Address | Purpose |
|--------|---------|---------|
| PCA9685 | 0x40 | Servo Driver |
| LCD Display | 0x27 | Display Controller |

### 2. Software Installation

#### Arduino IDE Setup

1. Install Arduino IDE 2.0 or later
2. Add ESP32 board support:
   ```
   File → Preferences → Additional Board Manager URLs
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
3. Install ESP32 board package via Board Manager

#### Library Installation

Install required libraries via Library Manager:

```bash
Tools → Manage Libraries → Search and Install:
- Adafruit PWM Servo Driver Library
- LiquidCrystal I2C
- Firebase ESP32 Client
- RTC by Makuna
```

### 3. Code Deployment

1. Clone the repository:
   ```bash
   git clone https://github.com/qppd/Pill-Dispenser-V3.git
   ```

2. Open `source/esp32/PillDispenser/PillDispenser.ino` in Arduino IDE

3. Configure network credentials in the main file:
   ```cpp
   const String WIFI_SSID = "YOUR_NETWORK_NAME";
   const String WIFI_PASSWORD = "YOUR_PASSWORD";
   const String FIREBASE_API_KEY = "YOUR_API_KEY";
   const String FIREBASE_URL = "YOUR_DATABASE_URL";
   ```

4. Select board configuration:
   - Board: "ESP32 Dev Module"
   - Upload Speed: "921600"
   - CPU Frequency: "240MHz (WiFi/BT)"

5. Upload the firmware

#### Raspberry Pi Setup

1. Install Raspberry Pi OS (64-bit) on your microSD card
2. Boot the Raspberry Pi and connect to network
3. Update system packages:
   ```bash
   sudo apt update && sudo apt upgrade -y
   ```

4. Install Python dependencies:
   ```bash
   cd Pill-Dispenser-V3/source/rpi/PillDispenser
   pip install -r requirements.txt
   ```

5. Install YOLOv8:
   ```bash
   pip install ultralytics
   ```

6. Configure serial communication with ESP32:
   - Ensure `/dev/ttyAMA0` or appropriate serial port is available
   - Update configuration in `main.py` if needed

7. Run the ML service:
   ```bash
   python main.py
   ```

#### ESP32-CAM Setup

1. Flash ESP32-CAM with the provided firmware:
   ```bash
   # Using esptool.py
   esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash -z 0x1000 esp32cam_firmware.bin
   ```

2. Configure WiFi credentials in the firmware
3. Ensure camera is connected and accessible via HTTP stream

#### Web Application Setup

1. Install Node.js 18+ on your development machine
2. Navigate to the web directory:
   ```bash
   cd Pill-Dispenser-V3/source/web
   ```

3. Install dependencies:
   ```bash
   npm install
   ```

4. Configure Firebase:
   - Copy your Firebase config to `src/lib/firebase.ts`
   - Update authentication and database settings

5. Run the development server:
   ```bash
   npm run dev
   ```

6. Build for production:
   ```bash
   npm run build
   npm start
   ```

## Configuration

### System Configuration

The system operates in two modes:

#### Development Mode (Default)
- Comprehensive serial command interface
- Individual component testing
- Real-time debugging information
- Safety features for development

#### Production Mode
- Automated operation
- Optimized power consumption
- Enhanced security features
- Remote monitoring only

### Firebase Configuration

1. Create a Firebase project at https://console.firebase.google.com
2. Enable Realtime Database
3. Configure authentication (anonymous auth recommended for development)
4. Update configuration in the main file

### Network Configuration

Configure WiFi credentials and ensure network connectivity:
- 2.4GHz WiFi network required
- Internet access for Firebase connectivity
- Stable connection recommended for real-time features

## API Reference

### ServoDriver Class

#### Methods

```cpp
class ServoDriver {
public:
    bool begin();                                    // Initialize servo driver
    void setServoAngle(uint8_t channel, uint16_t angle);  // Standard servo control
    void setServoSpeed(uint8_t channel, int speed);       // Continuous servo control
    void dispensePill(uint8_t channel, String size);      // Dispense single pill
    void dispensePillPair(uint8_t ch1, uint8_t ch2, String size); // Dual dispense
    void stopServo(uint8_t channel);                 // Stop specific servo
    void stopAllServos();                            // Emergency stop
    bool isConnected();                              // Connection status
    void testServo(uint8_t channel);                 // Test individual servo
    void testAllServos();                            // Test all servos
};
```

#### Pill Size Constants

| Size | Duration (ms) | Application |
|------|---------------|-------------|
| small | 800 | Small tablets |
| medium | 1000 | Standard pills |
| large | 1200 | Large capsules |
| custom | 1050 | User-defined |

### IRSensor Class

#### Methods

```cpp
class IRSensor {
public:
    void begin();                                    // Initialize sensors
    bool isBlocked(uint8_t sensorNum);              // Check sensor status
    bool hasStateChanged(uint8_t sensorNum);        // Detect state changes
    void readAllSensors();                          // Update all sensors
    void printSensorStates();                       // Display status
    void testSensor(uint8_t sensorNum);             // Test specific sensor
    void testAllSensors();                          // Test all sensors
};
```

### LCDDisplay Class

#### Methods

```cpp
class LCDDisplay {
public:
    bool begin();                                    // Initialize display
    void clear();                                    // Clear screen
    void print(String text, uint8_t col, uint8_t row); // Print at position
    void printLine(String text, uint8_t row);       // Print full line
    void centerText(String text, uint8_t row);      // Center text
    void displayStatus(String status);              // Show status
    void displayTime(String timeStr);               // Show time
    void displayPillCount(int count);               // Show pill count
    void testDisplay();                             // Test all functions
};
```

### RTClock Class

#### Methods

```cpp
class RTClock {
public:
    bool begin();                                    // Initialize RTC
    void setDateTime(uint16_t year, uint8_t month, uint8_t day, 
                    uint8_t hour, uint8_t minute, uint8_t second);
    String getTimeString();                         // Get formatted time
    String getDateString();                         // Get formatted date
    String getDateTimeString();                     // Get complete datetime
    bool isTimeMatch(uint8_t hour, uint8_t minute); // Schedule matching
    void testRTC();                                 // Test functionality
};
```

### FirebaseManager Class

#### Methods

```cpp
class FirebaseManager {
public:
    bool begin(String apiKey, String databaseURL);  // Initialize Firebase
    bool connectWiFi(String ssid, String password); // WiFi connection
    bool sendPillDispenseLog(int count, String timestamp); // Log dispense
    bool updateDeviceStatus(String status);         // Update status
    bool sendHeartbeat();                           // Send heartbeat
    bool uploadSensorData(String sensor, String value); // Upload data
    bool testConnection();                          // Test connectivity
    void printConnectionStatus();                   // Show status
};
```

### SIM800L Class

#### Methods

```cpp
class SIM800L {
public:
    bool begin(long baudRate = 9600);               // Initialize module
    bool sendSMS(String phoneNumber, String message); // Send SMS
    bool makeCall(String phoneNumber);              // Make voice call
    bool checkNetworkRegistration();                // Check network
    String getSignalStrength();                     // Get signal info
    void testModule();                              // Test functionality
};
```

## Development Mode

### Serial Command Interface

The development mode provides a comprehensive command-line interface accessible via serial monitor (115200 baud).

#### System Commands

| Command | Description |
|---------|-------------|
| `help` | Display complete command reference |
| `status` | Show comprehensive system status |
| `reset` | Restart the system |
| `clear` | Clear serial monitor display |
| `time` | Show current date and time |
| `i2c scan` | Scan for I2C devices |
| `wifi connect` | Initiate WiFi connection |

#### Component Testing Commands

| Command | Description |
|---------|-------------|
| `test servo` | Test single servo (will ask for number) |
| `test all servos` | Test all 16 servos |
| `test pill dispenser` | Test pill dispensing on channel |
| `test all dispensers` | Test all dispenser pairs |
| `test lcd` | Test LCD display |
| `test time` | Test NTP time sync (continuous) |
| `test sim800` | Test SIM800L module |
| `test firebase` | Test Firebase connection |
| `test voltage` | Test voltage sensor (continuous) |
| `voltage` | Show current voltage reading |

#### Pill Dispensing Commands

| Command | Description |
|---------|-------------|
| `dispense` | Test pill dispensing sequence |
| `dispense [ch] [size]` | Dispense pill (ch=0-15, size=small/medium/large) |
| `dispense pair [ch1] [ch2] [size]` | Dispense using servo pair |

#### Advanced Servo Control

| Command | Description |
|---------|-------------|
| `servo [num] [angle]` | Move servo to angle (0-180) |
| `servo reset` | Reset all servos to 90° |
| `servo stop [num]` | Stop specific servo |
| `servo stop all` | Stop all servos |
| `servo speed [num] [speed]` | Set servo speed (300-450) |

### Example Development Session

```
Pill Dispenser V3 - Development Mode
====================================

> help
Available Commands:
[Command list displayed]

> status
System Status:
Mode: DEVELOPMENT
Uptime: 127 seconds
Free Heap: 234,567 bytes
Current Time: 01/11/2025 14:30:25

Component Status:
  LCD: Connected
  Servo Driver: Connected (16 channels)
  RTC: Valid Time
  SIM800L: Ready
  WiFi: Connected (192.168.1.100)
  Firebase: Connected

Sensor Readings:
  IR Sensor 1: CLEAR
  IR Sensor 2: CLEAR  
  IR Sensor 3: CLEAR

> i2c scan
Scanning I2C bus for devices...
I2C device found at address 0x27 (LCD Display)
I2C device found at address 0x40 (PCA9685 Servo Driver)
Found 2 I2C device(s)

> test pill dispenser
Enter channel number (0-15): 0
Testing pill dispenser on channel 0
Dispensing small pill for 800ms
Dispensing medium pill for 1000ms  
Dispensing large pill for 1200ms
Test complete

> dispense 2 medium
Dispensing medium pill on channel 2 for 1000ms
Channel 2 stopped

> sensors
IR Sensor States:
S1: CLEAR, S2: CLEAR, S3: CLEAR
```

## Testing Framework

### Automated Testing

The system includes comprehensive automated testing capabilities:

#### Component-Level Testing

Each component class includes dedicated test methods:

- **ServoDriver**: Individual servo testing, range testing, speed testing
- **IRSensor**: State detection, debouncing, response time testing
- **LCDDisplay**: Character display, backlight, positioning testing
- **RTClock**: Time accuracy, date handling, scheduling testing
- **FirebaseManager**: Connectivity, data upload/download testing
- **SIM800L**: Network registration, SMS, call testing

#### Integration Testing

System-level testing validates component interactions:

- **Dispensing Workflow**: Complete pill dispensing sequence
- **Sensor Integration**: Multi-sensor pill detection
- **Communication Stack**: End-to-end data transmission
- **Error Recovery**: Fault detection and recovery

#### Performance Testing

Benchmarking and performance validation:

- **Response Time**: Sensor-to-dispense latency measurement
- **Accuracy Testing**: Dispensing precision validation
- **Reliability Testing**: Long-term operation validation
- **Power Consumption**: Current draw measurement

### Test Execution

Execute tests via serial commands:

```bash
# Individual component tests
test servo
test ir
test lcd
test rtc
test firebase
test sim800

# System integration tests  
test all servos
test all dispensers
test all sensors

# Performance tests
dispense 0 medium    # Measure dispense time
sensors             # Monitor sensor response
```

## Deployment

### Production Configuration

1. **Disable Development Mode**:
   ```cpp
   #define DEVELOPMENT_MODE false
   #define PRODUCTION_MODE true
   ```

2. **Configure Security**:
   - Use secure WiFi networks
   - Enable Firebase security rules
   - Implement device authentication

3. **Optimize Performance**:
   - Reduce serial output
   - Optimize power consumption
   - Enable watchdog timer

### Monitoring and Maintenance

#### Real-time Monitoring

- **Firebase Dashboard**: Real-time device status
- **SMS Alerts**: Critical error notifications
- **Heartbeat Monitoring**: Device connectivity status
- **Sensor Data Logging**: Historical sensor data

#### Maintenance Schedule

| Task | Frequency | Description |
|------|-----------|-------------|
| Sensor Calibration | Monthly | Verify IR sensor accuracy |
| Servo Inspection | Quarterly | Check mechanical operation |
| Power Supply Check | Monthly | Verify voltage stability |
| Software Update | As needed | Deploy firmware updates |

### Over-the-Air Updates

The system supports OTA updates via Firebase:

1. Upload new firmware to Firebase Storage
2. System checks for updates on startup
3. Automatic download and installation
4. Rollback capability for failed updates

## Troubleshooting

### Common Issues

#### Hardware Issues

**Servo Not Responding**

```
Symptoms: Servo not moving, no PWM signal
Diagnosis: Check connections, power supply, I2C communication
Solution: 
1. Verify I2C wiring (SDA: GPIO21, SCL: GPIO22)
2. Check 5V power supply to servos
3. Run 'i2c scan' command
4. Test with 'test servo' command
```

**IR Sensor False Readings**

```
Symptoms: Incorrect pill detection, erratic sensor behavior
Diagnosis: Environmental interference, calibration issues
Solution:
1. Check sensor mounting and alignment
2. Verify 3.3V power supply
3. Run 'test ir' command for 15 seconds
4. Adjust sensor sensitivity if needed
```

**LCD Display Not Working**

```
Symptoms: Blank screen, garbled text, no backlight
Diagnosis: I2C communication, power, addressing
Solution:
1. Verify I2C address (default 0x27)
2. Check I2C wiring integrity
3. Run 'i2c scan' to detect device
4. Test with 'test lcd' command
```

#### Software Issues

**WiFi Connection Failed**

```
Symptoms: Cannot connect to WiFi network
Diagnosis: Network credentials, signal strength, compatibility
Solution:
1. Verify SSID and password
2. Ensure 2.4GHz network (ESP32 limitation)
3. Check signal strength
4. Use 'wifi connect' command
```

**Firebase Connection Issues**

```
Symptoms: Data not uploading, authentication failed
Diagnosis: API key, database URL, network connectivity
Solution:
1. Verify Firebase configuration
2. Check API key and database URL
3. Ensure internet connectivity
4. Run 'test firebase' command
```

**RTC Time Incorrect**

```
Symptoms: Wrong time display, scheduling issues
Diagnosis: Battery backup, crystal oscillator, initial setup
Solution:
1. Replace CR2032 backup battery
2. Set correct time via serial commands
3. Verify crystal connections
4. Run 'test rtc' command
```

### Diagnostic Commands

Use these commands for system diagnosis:

```bash
status              # Overall system health check
i2c scan           # I2C device detection
test [component]   # Component-specific testing
sensors            # Real-time sensor monitoring
wifi connect       # Network connectivity test
```

### Log Analysis

System logs provide detailed diagnostic information:

#### Serial Output Analysis

```
ServoDriver: PCA9685 initialized successfully
IRSensor: 3 IR sensors initialized
LCDDisplay: I2C LCD initialized successfully
RTClock: DS1302 initialized successfully
FirebaseManager: Firebase initialized successfully
SIM800L: Module responding to AT commands
```

#### Firebase Log Analysis

Monitor cloud logs for:
- Device heartbeat timestamps
- Sensor data trends
- Error frequency patterns
- Network connectivity issues

### Support Resources

#### Debug Information Collection

When reporting issues, provide:

1. **Hardware Configuration**:
   - ESP32 board type and revision
   - Component specifications
   - Wiring diagram

2. **Software Information**:
   - Firmware version
   - Library versions
   - Configuration settings

3. **System Status**:
   - Serial output from 'status' command
   - Error messages and timestamps
   - Reproduction steps

4. **Environment Details**:
   - Network configuration
   - Power supply specifications
   - Operating conditions

## Contributing

### Development Guidelines

#### Code Standards

- **Language**: C++ (Arduino framework)
- **Style**: PascalCase for classes, camelCase for methods
- **Documentation**: Comprehensive inline comments
- **Testing**: All methods must include test functions

#### Contribution Process

1. **Fork Repository**: Create personal fork on GitHub
2. **Create Branch**: Use descriptive branch names
3. **Implement Changes**: Follow coding standards
4. **Add Tests**: Include test cases for new features
5. **Documentation**: Update README and inline docs
6. **Submit PR**: Provide detailed description

#### Component Development

When adding new components:

1. **Create Header File**: Define class interface
2. **Implement Methods**: Core functionality and testing
3. **Update Main File**: Integration and command handlers
4. **Add Documentation**: Usage examples and API reference
5. **Test Integration**: Verify system compatibility

### Issue Reporting

Use GitHub Issues for:
- Bug reports with reproduction steps
- Feature requests with use cases
- Documentation improvements
- Performance optimization suggestions

### Code Review Process

All contributions undergo review for:
- Code quality and standards compliance
- Test coverage and functionality
- Documentation completeness
- System integration compatibility

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### Third-Party Libraries

This project uses the following open-source libraries:

- **Arduino Core for ESP32**: LGPL 2.1
- **Adafruit PWM Servo Driver**: BSD License
- **Firebase ESP32 Client**: MIT License
- **RTC Library**: MIT License
- **LiquidCrystal I2C**: GPL v3

## Changelog

### Version 3.0.0 (2025-12-10)

#### Added
- Complete modular architecture redesign
- Enhanced 360-degree servo support
- Real-time Firebase data streaming
- Comprehensive development mode
- Advanced serial command interface
- Professional testing framework
- I2C device auto-detection
- Production deployment support

#### Changed
- Migrated to object-oriented design
- Improved error handling and recovery
- Enhanced documentation structure
- Optimized memory usage

#### Fixed
- I2C communication stability issues
- Servo timing accuracy problems
- WiFi reconnection handling
- Firebase authentication flow

### Version 2.1.0 (Previous Release)

#### Added
- Basic pill dispensing functionality
- Simple LCD display support
- WiFi connectivity
- Basic Firebase integration

#### Known Issues
- Limited error recovery
- Manual configuration required
- No real-time monitoring

---

**Pill Dispenser V3** - Advanced Automated Medication Dispensing System  
Copyright (c) 2025 QPPD. All rights reserved.