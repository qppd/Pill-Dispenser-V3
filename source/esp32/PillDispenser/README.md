# Pill Dispenser V3 - ESP32 Project

A comprehensive Arduino project for an ESP32-based automated pill dispenser with multiple sensors, connectivity options, and modular component architecture.

## 🏗️ Project Structure

```
PillDispenser/
├── PillDispenser.ino      # Main Arduino sketch
├── ServoDriver.h/.cpp     # 16-channel PWM servo driver (PCA9685)
├── IRSensor.h/.cpp        # 3x IR sensor management
├── LCDDisplay.h/.cpp      # I2C 20x4 LCD display
├── RTClock.h/.cpp         # DS1302 real-time clock
├── FirebaseManager.h/.cpp # Firebase connectivity
├── SIM800L.h/.cpp         # GSM/GPRS module
└── README.md              # This file
```

## 🔧 Hardware Components

| Component | Quantity | Description |
|-----------|----------|-------------|
| ESP32 Development Board | 1 | Main microcontroller |
| PCA9685 16-Channel PWM Driver | 1 | Controls up to 16 servos |
| IR Obstacle Sensors | 3 | Pill detection sensors |
| I2C LCD 20x4 | 1 | Status display |
| DS1302 RTC Module | 1 | Real-time clock |
| SIM800L GSM Module | 1 | SMS/Call/GPRS connectivity |
| Servo Motors | Variable | Pill dispensing mechanisms |

## 📌 Pin Configuration

### I2C Bus (LCD & Servo Driver)
- **SDA**: GPIO 21
- **SCL**: GPIO 22

### IR Sensors
- **Sensor 1**: GPIO 34
- **Sensor 2**: GPIO 35  
- **Sensor 3**: GPIO 32

### DS1302 RTC
- **CLK**: GPIO 18
- **DAT**: GPIO 19
- **RST**: GPIO 5

### SIM800L GSM
- **RX**: GPIO 16
- **TX**: GPIO 17
- **RST**: GPIO 4

### Status LED
- **LED**: GPIO 2

## 🚀 Getting Started

### 1. Hardware Setup
1. Connect all components according to the pin configuration above
2. Ensure proper power supply for all components
3. Connect ESP32 to computer via USB

### 2. Software Setup
1. Install Arduino IDE with ESP32 board support
2. Install required libraries:
   ```
   - Adafruit PWM Servo Driver Library
   - LiquidCrystal I2C
   - Firebase ESP32 Client
   - RtcDS1302 Library
   ```

3. Configure credentials in `PillDispenser.ino`:
   ```cpp
   const String WIFI_SSID = "Your_WiFi_Network";
   const String WIFI_PASSWORD = "Your_WiFi_Password";
   const String FIREBASE_API_KEY = "Your_Firebase_API_Key";
   const String FIREBASE_URL = "Your_Firebase_Database_URL";
   ```

### 3. Upload and Test
1. Select ESP32 board in Arduino IDE
2. Upload the sketch
3. Open Serial Monitor (115200 baud)
4. Type `help` to see available commands

## 🔧 Development Mode

The project includes a comprehensive development mode with serial command interface.

### Available Commands

#### System Commands
- `help` - Show command menu
- `status` - Display system status
- `reset` - Restart the system
- `clear` - Clear serial monitor
- `time` - Show current date/time
- `wifi connect` - Connect to WiFi
- `i2c scan` - Scan for I2C devices

#### Component Testing
- `test servo` - Test individual servo (will prompt for number)
- `test all servos` - Test all 16 servos sequentially
- `test pill dispenser` - Test pill dispensing on specific channel
- `test all dispensers` - Test all dispenser pairs
- `test ir` - Test all IR sensors for 15 seconds
- `test ir [1-3]` - Test specific IR sensor
- `test lcd` - Full LCD display test
- `test rtc` - RTC functionality test
- `test sim800` - SIM800L module test
- `test firebase` - Firebase connection test

#### Pill Dispenser Operations
- `dispense` - Test pill dispensing sequence
- `dispense [0-15] [small/medium/large]` - Dispense specific pill size on channel
- `dispense pair [ch1] [ch2] [size]` - Dispense using servo pair
- `sensors` - Show current sensor states

#### Advanced Servo Control
- `servo [0-15] [0-180]` - Move specific servo to angle
- `servo reset` - Reset all servos to 90 degrees
- `servo stop [0-15]` - Stop specific servo (360° mode)
- `servo stop all` - Stop all servos
- `servo speed [0-15] [300-450]` - Set servo speed for continuous rotation

### Example Testing Session

```
🔧 DEVELOPMENT MODE - PILL DISPENSER V3

> help
📚 AVAILABLE COMMANDS:
[Command list displayed]

> status
📊 SYSTEM STATUS:
Mode: DEVELOPMENT
Uptime: 45 seconds
Component Status:
  LCD: ✅ Connected
  Servo Driver: ✅ Connected
  [Additional status info]

> i2c scan
ServoDriver: Scanning I2C bus for devices...
ServoDriver: I2C device found at address 0x27 (LCD Display)
ServoDriver: I2C device found at address 0x40 (PCA9685 Servo Driver)

> test pill dispenser
Enter channel number (0-15): 0
ServoDriver: Testing pill dispenser on channel 0
ServoDriver: Dispensing small pill on channel 0 for 800ms
[Dispensing sequence runs]

> dispense 2 medium
ServoDriver: Dispensing medium pill on channel 2 for 1000ms

> dispense pair 0 1 large
ServoDriver: Dispensing large pills using channels 0 & 1 for 1200ms

> servo speed 3 400
ServoDriver: Operating channel 3 with speed 400

> servo stop 3
ServoDriver: Channel 3 stopped
```

## 🎯 Key Features

### Modular Architecture
- Each component is encapsulated in its own class
- Easy to test, maintain, and extend
- Clear separation of concerns

### Development Mode Testing
- Comprehensive serial command interface
- Individual component testing
- Real-time system monitoring
- Easy debugging and validation

### Component Classes

#### ServoDriver (Enhanced)
- Controls PCA9685 16-channel PWM driver
- **360-degree servo support** for pill dispensing
- **Timing-based control** with pill size presets
- Individual and pair servo operations
- **I2C device scanning** for debugging
- Speed control for continuous rotation servos
- Traditional angle control for standard servos
- Connection validation and error handling

#### IRSensor
- Manages 3 IR obstacle sensors
- Debouncing and state change detection
- Individual and group testing
- Event callbacks for pill detection

#### LCDDisplay
- I2C 20x4 LCD management
- Text positioning and formatting
- Status displays and menus
- Backlight control

#### RTClock
- DS1302 real-time clock interface
- Date/time formatting
- Scheduling helpers
- Time validation

#### FirebaseManager (Enhanced)
- **Real-time data streaming** capabilities
- **Callback system** for remote commands
- Anonymous authentication support
- Device status monitoring and heartbeat
- **Pill report system** with unique keys
- Enhanced error handling and reconnection
- WiFi management with better timeout handling
- Comprehensive connection status reporting

#### SIM800L
- GSM/GPRS module control
- SMS sending/receiving
- Voice call management
- Network status monitoring

## 📊 System Monitoring

The development mode provides comprehensive system monitoring:

- **Component Status**: Real-time connection status for all components
- **Sensor Readings**: Current state of all IR sensors
- **System Health**: Uptime, memory usage, WiFi status
- **Time Tracking**: RTC validation and current time display

## 🔮 Production Mode (Future)

Production mode will include:
- Automated pill scheduling
- Firebase synchronization
- SMS notifications
- Error handling and recovery
- Secure credential management
- OTA firmware updates

## 🛠️ Troubleshooting

### Common Issues

1. **LCD not responding**
   - Check I2C connections (SDA/SCL)
   - Verify I2C address (default 0x27)
   - Use `test lcd` command

2. **Servo not moving**
   - Check PCA9685 connections
   - Verify power supply to servos
   - Use `test servo` command

3. **RTC time invalid**
   - Check DS1302 connections
   - Replace CR2032 battery
   - Use `test rtc` command

4. **WiFi connection fails**
   - Update WiFi credentials
   - Check network availability
   - Use `wifi connect` command

### Debug Commands

Use these commands to diagnose issues:
```
> status          # Overall system health
> test [component] # Test specific component
> sensors         # Check sensor readings
> time            # Verify RTC functionality
```

## 📝 Notes

- **Development Mode**: Currently enabled by default for testing
- **Production Mode**: Will be implemented later (wag muna gawin)
- **Security**: Remember to change default credentials before deployment
- **Power**: Ensure adequate power supply for all servos and modules

## 🤝 Contributing

When adding new components:
1. Create separate .h and .cpp files
2. Include testing functions
3. Add serial commands for testing
4. Update this documentation

---

**Ready to test your pill dispenser!** 💊🤖

Upload the code, open Serial Monitor, and type `help` to get started!