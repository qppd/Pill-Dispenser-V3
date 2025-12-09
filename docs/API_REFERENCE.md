# API Documentation

This document provides comprehensive API documentation for the Pill Dispenser V3 multi-component system including ESP32 hardware, Raspberry Pi ML processing, and web dashboard APIs.

## Table of Contents

- [ESP32 Component APIs](#esp32-component-apis)
  - [ServoDriver API](#servodriver-api)
  - [IRSensor API](#irsensor-api)
  - [LCDDisplay API](#lcddisplay-api)
  - [RTClock API](#rtclock-api)
  - [FirebaseManager API](#firebasemanager-api)
  - [SIM800L API](#sim800l-api)
  - [Main Application API](#main-application-api)
- [Raspberry Pi APIs](#raspberry-pi-apis)
  - [ESP32CamInterface API](#esp32caminterface-api)
  - [EnsemblePillDetector API](#ensemblepilldetector-api)
  - [ESP32Communicator API](#esp32communicator-api)
  - [DatabaseManager API](#databasemanager-api)
- [Web Dashboard APIs](#web-dashboard-apis)
  - [Authentication API](#authentication-api)
  - [Dispenser Management API](#dispenser-management-api)
  - [Dispense Control API](#dispense-control-api)

## ESP32 Component APIs

## ServoDriver API

The ServoDriver class provides comprehensive control for the PCA9685 16-channel PWM servo driver, supporting both standard angle-controlled servos and continuous rotation servos for pill dispensing.

### Class Declaration

```cpp
class ServoDriver {
private:
    Adafruit_PWMServoDriver pwm;
    static const uint8_t PWM_FREQ = 50;
    static const uint8_t I2C_ADDRESS = 0x40;
    
public:
    ServoDriver();
    // Method declarations...
};
```

### Constructor

```cpp
ServoDriver::ServoDriver()
```

**Description**: Initializes a new ServoDriver instance with default I2C address (0x40).

**Parameters**: None

**Returns**: ServoDriver instance

### Initialization Methods

#### begin()

```cpp
bool begin()
```

**Description**: Initializes the PCA9685 servo driver, sets PWM frequency, and performs I2C device scanning.

**Parameters**: None

**Returns**: 
- `true` - Initialization successful
- `false` - Initialization failed

**Example**:
```cpp
ServoDriver servoDriver;
if (servoDriver.begin()) {
    Serial.println("Servo driver initialized successfully");
} else {
    Serial.println("Servo driver initialization failed");
}
```

#### scanI2CDevices()

```cpp
void scanI2CDevices()
```

**Description**: Scans the I2C bus for connected devices and reports their addresses.

**Parameters**: None

**Returns**: None (outputs to Serial)

**Example**:
```cpp
servoDriver.scanI2CDevices();
// Output: "I2C device found at address 0x40 (PCA9685 Servo Driver)"
```

### Standard Servo Control

#### setServoAngle()

```cpp
void setServoAngle(uint8_t servoNum, uint16_t angle)
```

**Description**: Sets a standard servo to a specific angle position.

**Parameters**:
- `servoNum` (uint8_t): Servo channel number (0-15)
- `angle` (uint16_t): Target angle in degrees (0-180)

**Returns**: None

**Example**:
```cpp
servoDriver.setServoAngle(0, 90);  // Set servo 0 to 90 degrees
servoDriver.setServoAngle(5, 180); // Set servo 5 to 180 degrees
```

#### setServoPulse()

```cpp
void setServoPulse(uint8_t servoNum, uint16_t pulse)
```

**Description**: Sets servo using raw PWM pulse width value.

**Parameters**:
- `servoNum` (uint8_t): Servo channel number (0-15)
- `pulse` (uint16_t): PWM pulse width (150-600 for standard servos)

**Returns**: None

**Example**:
```cpp
servoDriver.setServoPulse(0, 375); // Set servo 0 to center position
```

### Continuous Rotation Servo Control

#### setServoSpeed()

```cpp
void setServoSpeed(uint8_t channel, int speed)
```

**Description**: Controls continuous rotation servo speed and direction.

**Parameters**:
- `channel` (uint8_t): Servo channel number (0-15)
- `speed` (int): Speed value
  - `300-374`: Backward rotation (faster to slower)
  - `375`: Stop
  - `376-450`: Forward rotation (slower to faster)

**Returns**: None

**Example**:
```cpp
servoDriver.setServoSpeed(0, 400);  // Forward rotation
servoDriver.setServoSpeed(1, 350);  // Backward rotation
servoDriver.setServoSpeed(2, 375);  // Stop
```

#### stopServo()

```cpp
void stopServo(uint8_t channel)
```

**Description**: Completely stops a servo by setting PWM to 0.

**Parameters**:
- `channel` (uint8_t): Servo channel number (0-15)

**Returns**: None

**Example**:
```cpp
servoDriver.stopServo(0);  // Stop servo on channel 0
```

#### stopAllServos()

```cpp
void stopAllServos()
```

**Description**: Stops all servos on all 16 channels.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
servoDriver.stopAllServos();  // Emergency stop all servos
```

### Pill Dispensing Methods

#### dispensePill()

```cpp
void dispensePill(uint8_t channel, String pillSize = "medium")
```

**Description**: Dispenses a pill using timed continuous rotation servo operation.

**Parameters**:
- `channel` (uint8_t): Servo channel number (0-15)
- `pillSize` (String): Pill size ("small", "medium", "large", "custom")

**Timing**:
- Small: 800ms
- Medium: 1000ms
- Large: 1200ms
- Custom: 1050ms

**Returns**: None

**Example**:
```cpp
servoDriver.dispensePill(0, "small");   // Dispense small pill
servoDriver.dispensePill(1, "medium");  // Dispense medium pill
servoDriver.dispensePill(2, "large");   // Dispense large pill
```

#### dispensePillPair()

```cpp
void dispensePillPair(uint8_t channel1, uint8_t channel2, String pillSize = "medium")
```

**Description**: Simultaneously operates two servos to dispense pills.

**Parameters**:
- `channel1` (uint8_t): First servo channel (0-15)
- `channel2` (uint8_t): Second servo channel (0-15)
- `pillSize` (String): Pill size specification

**Returns**: None

**Example**:
```cpp
servoDriver.dispensePillPair(0, 1, "medium");  // Dispense using channels 0 and 1
```

#### operateServoTimed()

```cpp
void operateServoTimed(uint8_t channel, int direction, int duration)
```

**Description**: Operates a servo in a specific direction for a specified duration.

**Parameters**:
- `channel` (uint8_t): Servo channel number (0-15)
- `direction` (int): Direction (300-450, see setServoSpeed)
- `duration` (int): Operation time in milliseconds

**Returns**: None

**Example**:
```cpp
servoDriver.operateServoTimed(0, 400, 1500);  // Forward for 1.5 seconds
```

### Testing Methods

#### testServo()

```cpp
void testServo(uint8_t servoNum)
```

**Description**: Performs a comprehensive test of a single servo.

**Test Sequence**:
1. Move to 0 degrees
2. Move to 90 degrees
3. Move to 180 degrees
4. Return to 90 degrees

**Parameters**:
- `servoNum` (uint8_t): Servo channel to test (0-15)

**Returns**: None

**Example**:
```cpp
servoDriver.testServo(0);  // Test servo on channel 0
```

#### testAllServos()

```cpp
void testAllServos()
```

**Description**: Tests all 16 servo channels sequentially.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
servoDriver.testAllServos();  // Test all servos
```

#### testPillDispenser()

```cpp
void testPillDispenser(uint8_t channel)
```

**Description**: Tests pill dispensing functionality for a specific channel.

**Test Sequence**:
1. Dispense small pill
2. Dispense medium pill
3. Dispense large pill

**Parameters**:
- `channel` (uint8_t): Channel to test (0-15)

**Returns**: None

**Example**:
```cpp
servoDriver.testPillDispenser(0);  // Test pill dispensing on channel 0
```

#### testAllDispenserPairs()

```cpp
void testAllDispenserPairs()
```

**Description**: Tests all possible servo pairs (0&1, 2&3, 4&5, etc.) for synchronized dispensing.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
servoDriver.testAllDispenserPairs();  // Test all dispenser pairs
```

### Utility Methods

#### isConnected()

```cpp
bool isConnected()
```

**Description**: Checks if the PCA9685 is connected and responding on I2C bus.

**Parameters**: None

**Returns**:
- `true` - Device is connected and responding
- `false` - Device not found or not responding

**Example**:
```cpp
if (servoDriver.isConnected()) {
    Serial.println("Servo driver is connected");
}
```

#### resetAllServos()

```cpp
void resetAllServos()
```

**Description**: Resets all servos to 90-degree position (center position).

**Parameters**: None

**Returns**: None

**Example**:
```cpp
servoDriver.resetAllServos();  // Center all servos
```

#### getDurationForPillSize()

```cpp
int getDurationForPillSize(String pillSize)
```

**Description**: Returns the dispensing duration for a given pill size.

**Parameters**:
- `pillSize` (String): Pill size specification

**Returns**:
- Duration in milliseconds

**Example**:
```cpp
int duration = servoDriver.getDurationForPillSize("medium");  // Returns 1000
```

## IRSensor API

The IRSensor class manages three IR obstacle sensors for pill detection and verification throughout the dispensing process.

### Class Declaration

```cpp
class IRSensor {
private:
    uint8_t pin1, pin2, pin3;
    bool lastState1, lastState2, lastState3;
    unsigned long lastDebounceTime1, lastDebounceTime2, lastDebounceTime3;
    static const unsigned long DEBOUNCE_DELAY = 50;
    
public:
    IRSensor(uint8_t sensorPin1, uint8_t sensorPin2, uint8_t sensorPin3);
    // Method declarations...
};
```

### Constructor

```cpp
IRSensor::IRSensor(uint8_t sensorPin1, uint8_t sensorPin2, uint8_t sensorPin3)
```

**Description**: Creates an IRSensor instance with three sensor pins.

**Parameters**:
- `sensorPin1` (uint8_t): GPIO pin for sensor 1 (entrance)
- `sensorPin2` (uint8_t): GPIO pin for sensor 2 (middle)
- `sensorPin3` (uint8_t): GPIO pin for sensor 3 (exit)

**Example**:
```cpp
IRSensor irSensors(34, 35, 32);  // Create sensor array
```

### Initialization Methods

#### begin()

```cpp
void begin()
```

**Description**: Initializes all three IR sensors and configures GPIO pins.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
irSensors.begin();  // Initialize sensor array
```

### Sensor Reading Methods

#### isBlocked()

```cpp
bool isBlocked(uint8_t sensorNum)
```

**Description**: Checks if a specific sensor is currently blocked.

**Parameters**:
- `sensorNum` (uint8_t): Sensor number (1, 2, or 3)

**Returns**:
- `true` - Sensor is blocked (object detected)
- `false` - Sensor is clear

**Example**:
```cpp
if (irSensors.isBlocked(1)) {
    Serial.println("Pill detected at entrance");
}
```

#### hasStateChanged()

```cpp
bool hasStateChanged(uint8_t sensorNum)
```

**Description**: Checks if a sensor state has changed since last reading (with debouncing).

**Parameters**:
- `sensorNum` (uint8_t): Sensor number (1, 2, or 3)

**Returns**:
- `true` - State changed
- `false` - No state change

**Example**:
```cpp
if (irSensors.hasStateChanged(1)) {
    Serial.println("Sensor 1 state changed");
}
```

#### readAllSensors()

```cpp
void readAllSensors()
```

**Description**: Updates state for all three sensors and triggers callbacks if states changed.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
irSensors.readAllSensors();  // Update all sensor states
```

#### printSensorStates()

```cpp
void printSensorStates()
```

**Description**: Prints current state of all sensors to Serial.

**Parameters**: None

**Returns**: None

**Output Format**: "IRSensor States - S1: BLOCKED, S2: CLEAR, S3: CLEAR"

**Example**:
```cpp
irSensors.printSensorStates();  // Display sensor status
```

### Testing Methods

#### testSensor()

```cpp
void testSensor(uint8_t sensorNum)
```

**Description**: Tests a specific sensor for 10 seconds, monitoring state changes.

**Parameters**:
- `sensorNum` (uint8_t): Sensor number to test (1, 2, or 3)

**Returns**: None

**Example**:
```cpp
irSensors.testSensor(1);  // Test sensor 1 for 10 seconds
```

#### testAllSensors()

```cpp
void testAllSensors()
```

**Description**: Tests all sensors simultaneously for 15 seconds.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
irSensors.testAllSensors();  // Test all sensors
```

### Callback Methods

#### onSensor1Triggered()

```cpp
void onSensor1Triggered()
```

**Description**: Called when sensor 1 (entrance) detects an object.

**Parameters**: None

**Returns**: None

**Note**: Override this method to implement custom behavior.

#### onSensor2Triggered()

```cpp
void onSensor2Triggered()
```

**Description**: Called when sensor 2 (middle) detects an object.

**Parameters**: None

**Returns**: None

#### onSensor3Triggered()

```cpp
void onSensor3Triggered()
```

**Description**: Called when sensor 3 (exit) detects an object.

**Parameters**: None

**Returns**: None

## LCDDisplay API

The LCDDisplay class provides comprehensive control for a 20x4 character I2C LCD display.

### Class Declaration

```cpp
class LCDDisplay {
private:
    LiquidCrystal_I2C lcd;
    uint8_t i2cAddress;
    static const uint8_t COLS = 20;
    static const uint8_t ROWS = 4;
    
public:
    LCDDisplay(uint8_t address = 0x27);
    // Method declarations...
};
```

### Constructor

```cpp
LCDDisplay::LCDDisplay(uint8_t address = 0x27)
```

**Description**: Creates an LCDDisplay instance with specified I2C address.

**Parameters**:
- `address` (uint8_t): I2C address (default: 0x27)

**Example**:
```cpp
LCDDisplay lcd;        // Use default address 0x27
LCDDisplay lcd(0x3F);  // Use custom address 0x3F
```

### Initialization Methods

#### begin()

```cpp
bool begin()
```

**Description**: Initializes the LCD display and tests I2C communication.

**Parameters**: None

**Returns**:
- `true` - Initialization successful
- `false` - Initialization failed

**Example**:
```cpp
if (lcd.begin()) {
    Serial.println("LCD initialized successfully");
}
```

### Basic Display Methods

#### clear()

```cpp
void clear()
```

**Description**: Clears the entire display.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
lcd.clear();  // Clear display
```

#### setCursor()

```cpp
void setCursor(uint8_t col, uint8_t row)
```

**Description**: Sets cursor position for next text output.

**Parameters**:
- `col` (uint8_t): Column position (0-19)
- `row` (uint8_t): Row position (0-3)

**Returns**: None

**Example**:
```cpp
lcd.setCursor(0, 0);   // Top-left corner
lcd.setCursor(10, 2);  // Column 10, row 2
```

#### print()

```cpp
void print(String text)
void print(String text, uint8_t col, uint8_t row)
```

**Description**: Prints text at current cursor position or specified position.

**Parameters**:
- `text` (String): Text to display
- `col` (uint8_t): Column position (optional)
- `row` (uint8_t): Row position (optional)

**Returns**: None

**Example**:
```cpp
lcd.print("Hello World");        // Print at current position
lcd.print("Status: Ready", 0, 1); // Print at column 0, row 1
```

### Advanced Display Methods

#### printLine()

```cpp
void printLine(String text, uint8_t row)
```

**Description**: Prints text on a specific row, clearing the entire row first.

**Parameters**:
- `text` (String): Text to display (truncated if > 20 characters)
- `row` (uint8_t): Row number (0-3)

**Returns**: None

**Example**:
```cpp
lcd.printLine("System Status: OK", 0);  // Clear row 0 and print text
```

#### centerText()

```cpp
void centerText(String text, uint8_t row)
```

**Description**: Centers text on a specific row.

**Parameters**:
- `text` (String): Text to center
- `row` (uint8_t): Row number (0-3)

**Returns**: None

**Example**:
```cpp
lcd.centerText("PILL DISPENSER V3", 0);  // Center title on row 0
```

### Status Display Methods

#### displayWelcome()

```cpp
void displayWelcome()
```

**Description**: Displays welcome screen with system title.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
lcd.displayWelcome();  // Show welcome screen
```

#### displayMainScreen()

```cpp
void displayMainScreen()
```

**Description**: Displays main operational screen with status information.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
lcd.displayMainScreen();  // Show main screen
```

#### displayStatus()

```cpp
void displayStatus(String status)
```

**Description**: Updates status line on display.

**Parameters**:
- `status` (String): Status message

**Returns**: None

**Example**:
```cpp
lcd.displayStatus("Dispensing...");  // Show dispensing status
lcd.displayStatus("Ready");          // Show ready status
```

#### displayTime()

```cpp
void displayTime(String timeStr)
```

**Description**: Displays current time on the display.

**Parameters**:
- `timeStr` (String): Formatted time string

**Returns**: None

**Example**:
```cpp
lcd.displayTime("14:30:25");  // Display time
```

#### displayPillCount()

```cpp
void displayPillCount(int count)
```

**Description**: Displays current pill count.

**Parameters**:
- `count` (int): Number of pills dispensed

**Returns**: None

**Example**:
```cpp
lcd.displayPillCount(42);  // Display "Pills: 42"
```

#### displayError()

```cpp
void displayError(String error)
```

**Description**: Displays error message with centered "ERROR" header.

**Parameters**:
- `error` (String): Error message

**Returns**: None

**Example**:
```cpp
lcd.displayError("Sensor Failure");  // Display error message
```

#### displayTestMenu()

```cpp
void displayTestMenu()
```

**Description**: Displays test mode menu information.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
lcd.displayTestMenu();  // Show test mode menu
```

### Utility Methods

#### backlight()

```cpp
void backlight(bool on)
```

**Description**: Controls LCD backlight.

**Parameters**:
- `on` (bool): true to turn on, false to turn off

**Returns**: None

**Example**:
```cpp
lcd.backlight(true);   // Turn on backlight
lcd.backlight(false);  // Turn off backlight
```

#### isConnected()

```cpp
bool isConnected()
```

**Description**: Checks if LCD is connected and responding.

**Parameters**: None

**Returns**:
- `true` - LCD is connected
- `false` - LCD not responding

**Example**:
```cpp
if (lcd.isConnected()) {
    Serial.println("LCD is connected");
}
```

### Testing Methods

#### testDisplay()

```cpp
void testDisplay()
```

**Description**: Performs comprehensive LCD test including all display functions.

**Test Sequence**:
1. Basic text display
2. All rows test
3. Centered text test
4. Backlight test
5. Character fill test

**Parameters**: None

**Returns**: None

**Example**:
```cpp
lcd.testDisplay();  // Run complete LCD test
```

## RTClock API

The RTClock class provides real-time clock functionality using the DS1302 RTC module.

### Class Declaration

```cpp
class RTClock {
private:
    ThreeWire myWire;
    RtcDS1302<ThreeWire> rtc;
    uint8_t clkPin, datPin, rstPin;
    
public:
    RTClock(uint8_t clkPin, uint8_t datPin, uint8_t rstPin);
    // Method declarations...
};
```

### Constructor

```cpp
RTClock::RTClock(uint8_t clkPin, uint8_t datPin, uint8_t rstPin)
```

**Description**: Creates an RTClock instance with specified pin connections.

**Parameters**:
- `clkPin` (uint8_t): Clock pin (typically GPIO 18)
- `datPin` (uint8_t): Data pin (typically GPIO 19)
- `rstPin` (uint8_t): Reset pin (typically GPIO 5)

**Example**:
```cpp
RTClock rtclock(18, 19, 5);  // Create RTC instance
```

### Initialization Methods

#### begin()

```cpp
bool begin()
```

**Description**: Initializes the DS1302 RTC module and validates operation.

**Parameters**: None

**Returns**:
- `true` - Initialization successful
- `false` - Initialization failed

**Example**:
```cpp
if (rtclock.begin()) {
    Serial.println("RTC initialized successfully");
}
```

### Date/Time Setting Methods

#### setDateTime()

```cpp
void setDateTime(uint16_t year, uint8_t month, uint8_t day, 
                uint8_t hour, uint8_t minute, uint8_t second)
```

**Description**: Sets the current date and time.

**Parameters**:
- `year` (uint16_t): Year (e.g., 2025)
- `month` (uint8_t): Month (1-12)
- `day` (uint8_t): Day (1-31)
- `hour` (uint8_t): Hour (0-23)
- `minute` (uint8_t): Minute (0-59)
- `second` (uint8_t): Second (0-59)

**Returns**: None

**Example**:
```cpp
rtclock.setDateTime(2025, 11, 1, 14, 30, 0);  // Set to Nov 1, 2025 14:30:00
```

### Date/Time Reading Methods

#### getTimeString()

```cpp
String getTimeString()
```

**Description**: Returns formatted time string in HH:MM:SS format.

**Parameters**: None

**Returns**: Time string (e.g., "14:30:25")

**Example**:
```cpp
String currentTime = rtclock.getTimeString();
Serial.println("Current time: " + currentTime);
```

#### getDateString()

```cpp
String getDateString()
```

**Description**: Returns formatted date string in DD/MM/YYYY format.

**Parameters**: None

**Returns**: Date string (e.g., "01/11/2025")

**Example**:
```cpp
String currentDate = rtclock.getDateString();
Serial.println("Current date: " + currentDate);
```

#### getDateTimeString()

```cpp
String getDateTimeString()
```

**Description**: Returns formatted date and time string.

**Parameters**: None

**Returns**: DateTime string (e.g., "01/11/2025 14:30:25")

**Example**:
```cpp
String dateTime = rtclock.getDateTimeString();
Serial.println("Current: " + dateTime);
```

#### getDateTime()

```cpp
RtcDateTime getDateTime()
```

**Description**: Returns raw RtcDateTime object for advanced operations.

**Parameters**: None

**Returns**: RtcDateTime object

**Example**:
```cpp
RtcDateTime now = rtclock.getDateTime();
int currentHour = now.Hour();
```

### Individual Component Access

#### getHour()

```cpp
uint8_t getHour()
```

**Description**: Returns current hour (0-23).

**Parameters**: None

**Returns**: Hour value

**Example**:
```cpp
uint8_t hour = rtclock.getHour();
if (hour == 12) {
    Serial.println("It's noon!");
}
```

#### getMinute()

```cpp
uint8_t getMinute()
```

**Description**: Returns current minute (0-59).

**Parameters**: None

**Returns**: Minute value

#### getSecond()

```cpp
uint8_t getSecond()
```

**Description**: Returns current second (0-59).

**Parameters**: None

**Returns**: Second value

#### getDay()

```cpp
uint8_t getDay()
```

**Description**: Returns current day of month (1-31).

**Parameters**: None

**Returns**: Day value

#### getMonth()

```cpp
uint8_t getMonth()
```

**Description**: Returns current month (1-12).

**Parameters**: None

**Returns**: Month value

#### getYear()

```cpp
uint16_t getYear()
```

**Description**: Returns current year (e.g., 2025).

**Parameters**: None

**Returns**: Year value

### Scheduling Methods

#### isTimeMatch()

```cpp
bool isTimeMatch(uint8_t hour, uint8_t minute)
```

**Description**: Checks if current time matches specified hour and minute (for scheduling).

**Parameters**:
- `hour` (uint8_t): Target hour (0-23)
- `minute` (uint8_t): Target minute (0-59)

**Returns**:
- `true` - Time matches (and seconds == 0)
- `false` - Time doesn't match

**Example**:
```cpp
if (rtclock.isTimeMatch(8, 30)) {
    Serial.println("Time for morning medication!");
}
```

#### hasMinutePassed()

```cpp
bool hasMinutePassed()
```

**Description**: Checks if a new minute has started since last call.

**Parameters**: None

**Returns**:
- `true` - New minute has started
- `false` - Same minute as last call

**Example**:
```cpp
if (rtclock.hasMinutePassed()) {
    Serial.println("New minute started");
}
```

### Utility Methods

#### isValidTime()

```cpp
bool isValidTime()
```

**Description**: Checks if RTC has valid time (not corrupted).

**Parameters**: None

**Returns**:
- `true` - Time is valid
- `false` - Time is invalid/corrupted

**Example**:
```cpp
if (!rtclock.isValidTime()) {
    Serial.println("RTC time is invalid, please set time");
}
```

#### printDateTime()

```cpp
void printDateTime()
```

**Description**: Prints current date and time to Serial.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
rtclock.printDateTime();  // Output: "RTClock: Current DateTime: 01/11/2025 14:30:25"
```

### Testing Methods

#### testRTC()

```cpp
void testRTC()
```

**Description**: Performs comprehensive RTC testing.

**Test Sequence**:
1. Display current time
2. Time progression test (5 seconds)
3. Individual component test
4. Validity check

**Parameters**: None

**Returns**: None

**Example**:
```cpp
rtclock.testRTC();  // Run complete RTC test
```

## FirebaseManager API

The FirebaseManager class provides comprehensive Firebase Realtime Database connectivity with real-time streaming capabilities.

### Class Declaration

```cpp
class FirebaseManager {
private:
    FirebaseData fbdo;
    FirebaseData deviceStream;
    FirebaseAuth auth;
    FirebaseConfig config;
    bool isConnected;
    bool isAuthenticated;
    bool signupOk;
    String deviceId;
    
public:
    FirebaseManager();
    // Method declarations...
};
```

### Constructor

```cpp
FirebaseManager::FirebaseManager()
```

**Description**: Creates a FirebaseManager instance with auto-generated device ID.

**Parameters**: None

**Example**:
```cpp
FirebaseManager firebase;  // Create Firebase manager
```

### Initialization Methods

#### begin()

```cpp
bool begin(String apiKey, String databaseURL)
```

**Description**: Initializes Firebase with API key and database URL.

**Parameters**:
- `apiKey` (String): Firebase project API key
- `databaseURL` (String): Firebase Realtime Database URL

**Returns**:
- `true` - Initialization successful
- `false` - Initialization failed

**Example**:
```cpp
bool success = firebase.begin("your-api-key", "https://your-project.firebaseio.com/");
```

#### connectWiFi()

```cpp
bool connectWiFi(String ssid, String password)
```

**Description**: Connects to WiFi network with specified credentials.

**Parameters**:
- `ssid` (String): WiFi network name
- `password` (String): WiFi password

**Returns**:
- `true` - WiFi connection successful
- `false` - WiFi connection failed

**Example**:
```cpp
bool wifiConnected = firebase.connectWiFi("MyNetwork", "MyPassword");
```

#### initializeFirebase()

```cpp
bool initializeFirebase()
```

**Description**: Completes Firebase authentication and starts data streaming.

**Parameters**: None

**Returns**:
- `true` - Firebase ready for use
- `false` - Firebase initialization failed

### Data Upload Methods

#### sendPillDispenseLog()

```cpp
bool sendPillDispenseLog(int pillCount, String timestamp)
```

**Description**: Logs pill dispensing event to Firebase.

**Parameters**:
- `pillCount` (int): Number of pills dispensed
- `timestamp` (String): Time of dispensing

**Returns**:
- `true` - Log sent successfully
- `false` - Failed to send log

**Example**:
```cpp
bool logged = firebase.sendPillDispenseLog(1, "01/11/2025 14:30:25");
```

#### updateDeviceStatus()

```cpp
bool updateDeviceStatus(String status)
```

**Description**: Updates device status in Firebase.

**Parameters**:
- `status` (String): Current device status

**Returns**:
- `true` - Status updated successfully
- `false` - Failed to update status

**Example**:
```cpp
firebase.updateDeviceStatus("Ready");
firebase.updateDeviceStatus("Dispensing");
firebase.updateDeviceStatus("Error");
```

#### sendHeartbeat()

```cpp
bool sendHeartbeat()
```

**Description**: Sends periodic heartbeat to indicate device is online.

**Parameters**: None

**Returns**:
- `true` - Heartbeat sent successfully
- `false` - Failed to send heartbeat

**Note**: Automatically rate-limited to once per minute

**Example**:
```cpp
firebase.sendHeartbeat();  // Send if time interval elapsed
```

#### uploadSensorData()

```cpp
bool uploadSensorData(String sensorName, String value)
```

**Description**: Uploads sensor reading to Firebase.

**Parameters**:
- `sensorName` (String): Name/ID of sensor
- `value` (String): Sensor reading value

**Returns**:
- `true` - Data uploaded successfully
- `false` - Failed to upload data

**Example**:
```cpp
firebase.uploadSensorData("ir_sensor_1", "blocked");
firebase.uploadSensorData("temperature", "25.5");
```

#### sendPillReport()

```cpp
bool sendPillReport(int pillCount, String datetime, String description, int status)
```

**Description**: Sends comprehensive pill dispensing report with unique key.

**Parameters**:
- `pillCount` (int): Number of pills in report
- `datetime` (String): Date and time of event
- `description` (String): Description of event
- `status` (int): Status code (1=success, 0=error)

**Returns**:
- `true` - Report sent successfully
- `false` - Failed to send report

**Example**:
```cpp
firebase.sendPillReport(1, "01/11/2025 14:30:25", "Scheduled morning dose", 1);
```

### Data Download Methods

#### downloadSchedule()

```cpp
bool downloadSchedule()
```

**Description**: Downloads medication schedule from Firebase.

**Parameters**: None

**Returns**:
- `true` - Schedule downloaded successfully
- `false` - Failed to download schedule

**Example**:
```cpp
if (firebase.downloadSchedule()) {
    Serial.println("Schedule updated");
}
```

#### checkForCommands()

```cpp
bool checkForCommands()
```

**Description**: Checks for remote commands and clears them after reading.

**Parameters**: None

**Returns**:
- `true` - Command found and processed
- `false` - No commands available

**Example**:
```cpp
if (firebase.checkForCommands()) {
    Serial.println("Remote command received");
}
```

### Real-time Streaming

#### beginDataStream()

```cpp
bool beginDataStream()
```

**Description**: Starts real-time data streaming for remote commands.

**Parameters**: None

**Returns**:
- `true` - Streaming started successfully
- `false` - Failed to start streaming

**Example**:
```cpp
firebase.beginDataStream();  // Start real-time updates
```

#### handleStreamUpdates()

```cpp
void handleStreamUpdates()
```

**Description**: Processes pending real-time stream updates.

**Parameters**: None

**Returns**: None

**Note**: Call this in main loop to handle real-time updates

**Example**:
```cpp
void loop() {
    firebase.handleStreamUpdates();
    // Other loop code...
}
```

### Utility Methods

#### isFirebaseReady()

```cpp
bool isFirebaseReady()
```

**Description**: Checks if Firebase is connected and ready for operations.

**Parameters**: None

**Returns**:
- `true` - Firebase is ready
- `false` - Firebase not ready

**Example**:
```cpp
if (firebase.isFirebaseReady()) {
    firebase.sendPillDispenseLog(1, "now");
}
```

#### getDeviceId()

```cpp
String getDeviceId()
```

**Description**: Returns unique device identifier.

**Parameters**: None

**Returns**: Device ID string

**Example**:
```cpp
String deviceId = firebase.getDeviceId();
Serial.println("Device ID: " + deviceId);
```

#### setDeviceId()

```cpp
void setDeviceId(String id)
```

**Description**: Sets custom device identifier.

**Parameters**:
- `id` (String): Custom device ID

**Returns**: None

**Example**:
```cpp
firebase.setDeviceId("PILL_DISPENSER_001");
```

#### shouldSendData()

```cpp
bool shouldSendData()
```

**Description**: Checks if enough time has elapsed since last data transmission.

**Parameters**: None

**Returns**:
- `true` - OK to send data
- `false` - Too soon since last transmission

**Example**:
```cpp
if (firebase.shouldSendData()) {
    firebase.uploadSensorData("status", "active");
}
```

#### printConnectionStatus()

```cpp
void printConnectionStatus()
```

**Description**: Prints comprehensive connection status to Serial.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
firebase.printConnectionStatus();
// Output: WiFi status, Firebase status, device info, etc.
```

### Testing Methods

#### testConnection()

```cpp
bool testConnection()
```

**Description**: Tests basic Firebase connectivity by writing test data.

**Parameters**: None

**Returns**:
- `true` - Connection test passed
- `false` - Connection test failed

**Example**:
```cpp
if (firebase.testConnection()) {
    Serial.println("Firebase connection test passed");
}
```

#### testDataUpload()

```cpp
bool testDataUpload()
```

**Description**: Tests data upload functionality with sample data.

**Parameters**: None

**Returns**:
- `true` - Upload test passed
- `false` - Upload test failed

**Example**:
```cpp
firebase.testDataUpload();  // Test upload capability
```

#### testDataDownload()

```cpp
bool testDataDownload()
```

**Description**: Tests data download functionality.

**Parameters**: None

**Returns**:
- `true` - Download test passed
- `false` - Download test failed

**Example**:
```cpp
firebase.testDataDownload();  // Test download capability
```

## SIM800L API

The SIM800L class provides comprehensive GSM/GPRS communication capabilities for SMS and voice call functionality.

### Class Declaration

```cpp
class SIM800L {
private:
    SoftwareSerial sim800;
    uint8_t rxPin, txPin, rstPin;
    bool isModuleReady;
    String response;
    unsigned long lastCommand;
    static const unsigned long COMMAND_DELAY = 1000;
    
public:
    SIM800L(uint8_t rxPin, uint8_t txPin, uint8_t rstPin);
    // Method declarations...
};
```

### Constructor

```cpp
SIM800L::SIM800L(uint8_t rxPin, uint8_t txPin, uint8_t rstPin)
```

**Description**: Creates a SIM800L instance with specified pin connections.

**Parameters**:
- `rxPin` (uint8_t): ESP32 pin connected to SIM800L TX
- `txPin` (uint8_t): ESP32 pin connected to SIM800L RX  
- `rstPin` (uint8_t): ESP32 pin connected to SIM800L RST

**Example**:
```cpp
SIM800L sim800(16, 17, 4);  // RX=16, TX=17, RST=4
```

### Initialization Methods

#### begin()

```cpp
bool begin(long baudRate = 9600)
```

**Description**: Initializes SIM800L module and tests basic communication.

**Parameters**:
- `baudRate` (long): Serial communication speed (default: 9600)

**Returns**:
- `true` - Module initialized successfully
- `false` - Module initialization failed

**Example**:
```cpp
if (sim800.begin()) {
    Serial.println("SIM800L ready");
} else {
    Serial.println("SIM800L initialization failed");
}
```

#### reset()

```cpp
void reset()
```

**Description**: Performs hardware reset of SIM800L module.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
sim800.reset();  // Hardware reset module
delay(3000);     // Wait for startup
```

### Basic Communication

#### sendATCommand()

```cpp
bool sendATCommand(String command, String expectedResponse = "OK", unsigned long timeout = 5000)
```

**Description**: Sends AT command and waits for expected response.

**Parameters**:
- `command` (String): AT command to send
- `expectedResponse` (String): Expected response (default: "OK")
- `timeout` (unsigned long): Timeout in milliseconds (default: 5000)

**Returns**:
- `true` - Expected response received
- `false` - Unexpected response or timeout

**Example**:
```cpp
bool success = sim800.sendATCommand("AT+CSQ", "+CSQ:", 3000);  // Check signal
```

#### getResponse()

```cpp
String getResponse()
```

**Description**: Returns the last response received from module.

**Parameters**: None

**Returns**: Response string

**Example**:
```cpp
sim800.sendATCommand("AT+CSQ");
String response = sim800.getResponse();
Serial.println("Response: " + response);
```

#### clearBuffer()

```cpp
void clearBuffer()
```

**Description**: Clears the serial communication buffer.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
sim800.clearBuffer();  // Clear old data
```

### Network Operations

#### checkNetworkRegistration()

```cpp
bool checkNetworkRegistration()
```

**Description**: Checks if module is registered on cellular network.

**Parameters**: None

**Returns**:
- `true` - Network registered
- `false` - Not registered

**Example**:
```cpp
if (sim800.checkNetworkRegistration()) {
    Serial.println("Network connected");
} else {
    Serial.println("No network connection");
}
```

#### getSignalStrength()

```cpp
String getSignalStrength()
```

**Description**: Gets current cellular signal strength.

**Parameters**: None

**Returns**: Signal strength response string

**Example**:
```cpp
String signal = sim800.getSignalStrength();
Serial.println("Signal: " + signal);
```

#### getNetworkOperator()

```cpp
String getNetworkOperator()
```

**Description**: Gets current network operator information.

**Parameters**: None

**Returns**: Network operator response string

**Example**:
```cpp
String operator = sim800.getNetworkOperator();
Serial.println("Operator: " + operator);
```

#### isNetworkConnected()

```cpp
bool isNetworkConnected()
```

**Description**: Alias for checkNetworkRegistration().

**Parameters**: None

**Returns**:
- `true` - Network connected
- `false` - Not connected

### SMS Operations

#### sendSMS()

```cpp
bool sendSMS(String phoneNumber, String message)
```

**Description**: Sends SMS message to specified phone number.

**Parameters**:
- `phoneNumber` (String): Recipient phone number (with country code)
- `message` (String): Message text (max 160 characters)

**Returns**:
- `true` - SMS sent successfully
- `false` - Failed to send SMS

**Example**:
```cpp
bool sent = sim800.sendSMS("+1234567890", "Pill dispensed at 14:30");
if (sent) {
    Serial.println("SMS sent successfully");
}
```

#### readSMS()

```cpp
bool readSMS(int index)
```

**Description**: Reads SMS message at specified index.

**Parameters**:
- `index` (int): SMS storage index

**Returns**:
- `true` - SMS read successfully
- `false` - Failed to read SMS

**Example**:
```cpp
if (sim800.readSMS(1)) {
    String message = sim800.getResponse();
    Serial.println("SMS: " + message);
}
```

#### deleteSMS()

```cpp
bool deleteSMS(int index)
```

**Description**: Deletes SMS message at specified index.

**Parameters**:
- `index` (int): SMS storage index

**Returns**:
- `true` - SMS deleted successfully
- `false` - Failed to delete SMS

**Example**:
```cpp
sim800.deleteSMS(1);  // Delete SMS at index 1
```

#### getLastSMS()

```cpp
String getLastSMS()
```

**Description**: Returns the last SMS message content.

**Parameters**: None

**Returns**: SMS message content

### Voice Call Operations

#### makeCall()

```cpp
bool makeCall(String phoneNumber)
```

**Description**: Initiates voice call to specified number.

**Parameters**:
- `phoneNumber` (String): Phone number to call

**Returns**:
- `true` - Call initiated successfully
- `false` - Failed to initiate call

**Example**:
```cpp
bool calling = sim800.makeCall("+1234567890");
if (calling) {
    Serial.println("Calling...");
}
```

#### hangupCall()

```cpp
bool hangupCall()
```

**Description**: Ends current voice call.

**Parameters**: None

**Returns**:
- `true` - Call ended successfully
- `false` - Failed to end call

**Example**:
```cpp
sim800.hangupCall();  // End current call
```

#### answerCall()

```cpp
bool answerCall()
```

**Description**: Answers incoming voice call.

**Parameters**: None

**Returns**:
- `true` - Call answered successfully
- `false` - Failed to answer call

**Example**:
```cpp
sim800.answerCall();  // Answer incoming call
```

### Status Methods

#### isReady()

```cpp
bool isReady()
```

**Description**: Checks if module is ready for commands.

**Parameters**: None

**Returns**:
- `true` - Module ready
- `false` - Module not ready

**Example**:
```cpp
if (sim800.isReady()) {
    sim800.sendSMS("+1234567890", "Test message");
}
```

### Information Methods

#### printModuleInfo()

```cpp
void printModuleInfo()
```

**Description**: Prints comprehensive module information to Serial.

**Parameters**: None

**Returns**: None

**Output**: Module info, signal strength, network operator, registration status

**Example**:
```cpp
sim800.printModuleInfo();
// Output: Module info, signal strength, network status, etc.
```

### Testing Methods

#### testModule()

```cpp
void testModule()
```

**Description**: Performs comprehensive module testing.

**Test Sequence**:
1. Basic AT communication
2. Module information retrieval
3. Network registration check

**Parameters**: None

**Returns**: None

**Example**:
```cpp
sim800.testModule();  // Run complete module test
```

#### testSMS()

```cpp
void testSMS()
```

**Description**: Tests SMS functionality (simulation mode).

**Parameters**: None

**Returns**: None

**Example**:
```cpp
sim800.testSMS();  // Test SMS capabilities
```

#### testCall()

```cpp
void testCall()
```

**Description**: Tests voice call functionality (simulation mode).

**Parameters**: None

**Returns**: None

**Example**:
```cpp
sim800.testCall();  // Test call capabilities
```

#### testGPRS()

```cpp
void testGPRS()
```

**Description**: Tests GPRS data functionality.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
sim800.testGPRS();  // Test GPRS capabilities
```

---

This comprehensive API documentation provides detailed information for all classes and methods in the Pill Dispenser V3 system. Each method includes description, parameters, return values, and practical examples for implementation.