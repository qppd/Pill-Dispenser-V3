# API Documentation - Pill Dispenser V3

This document provides comprehensive API documentation for the Pill Dispenser V3 ESP32-based medication dispensing system.

## Table of Contents

- [ESP32 Component APIs](#esp32-component-apis)
  - [FirebaseConfig API](#firebaseconfig-api)
  - [FirebaseManager API](#firebasemanager-api)
  - [LCDDisplay API](#lcddisplay-api)
  - [NotificationManager API](#notificationmanager-api)
  - [ScheduleManager API](#schedulemanager-api)
  - [ServoDriver API](#servodriver-api)
  - [SIM800L API](#sim800l-api)
  - [TimeManager API](#timemanager-api)
  - [VoltageSensor API](#voltagesensor-api)
  - [WiFiManager API](#wifimanager-api)
  - [Main Application API](#main-application-api)
- [Web Dashboard APIs](#web-dashboard-apis)
  - [Authentication API](#authentication-api)
  - [Dispenser Management API](#dispenser-management-api)
  - [Schedule Management API](#schedule-management-api)

## ESP32 Component APIs

## FirebaseConfig API

The FirebaseConfig class provides centralized configuration management for Firebase connection settings.

### Class Declaration

```cpp
class PillDispenserConfig {
public:
    static const char* getApiKey();
    static const char* getDatabaseURL();
    static const char* getProjectId();
    static const char* getUserEmail();
    static const char* getUserPassword();
};
```

### Methods

#### getApiKey()

```cpp
static const char* getApiKey()
```

**Description**: Returns the Firebase API key for authentication.

**Parameters**: None

**Returns**: Firebase API key as string

#### getDatabaseURL()

```cpp
static const char* getDatabaseURL()
```

**Description**: Returns the Firebase Realtime Database URL.

**Parameters**: None

**Returns**: Database URL as string

#### getProjectId()

```cpp
static const char* getProjectId()
```

**Description**: Returns the Firebase project ID.

**Parameters**: None

**Returns**: Project ID as string

#### getUserEmail()

```cpp
static const char* getUserEmail()
```

**Description**: Returns the service account email for authentication.

**Parameters**: None

**Returns**: Email address as string

#### getUserPassword()

```cpp
static const char* getUserPassword()
```

**Description**: Returns the service account password/key.

**Parameters**: None

**Returns**: Password/key as string

## FirebaseManager API

The FirebaseManager class handles all Firebase Realtime Database operations including authentication, data synchronization, and real-time streaming.

### Class Declaration

```cpp
class FirebaseManager {
private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    String deviceId;
    bool initialized;

public:
    FirebaseManager();
    // Method declarations...
};
```

### Constructor

```cpp
FirebaseManager::FirebaseManager()
```

**Description**: Initializes a new FirebaseManager instance.

**Parameters**: None

**Returns**: FirebaseManager instance

### Initialization Methods

#### begin()

```cpp
bool begin(const String& deviceId)
```

**Description**: Initializes Firebase connection with service account authentication.

**Parameters**:
- `deviceId` (String): Unique device identifier

**Returns**:
- `true` - Initialization successful
- `false` - Initialization failed

**Example**:
```cpp
FirebaseManager firebase;
if (firebase.begin("PILL_DISPENSER_001")) {
    Serial.println("Firebase initialized successfully");
}
```

#### isInitialized()

```cpp
bool isInitialized()
```

**Description**: Checks if Firebase is properly initialized.

**Parameters**: None

**Returns**:
- `true` - Firebase is initialized
- `false` - Firebase is not initialized

### Data Synchronization Methods

#### syncSchedulesFromFirebase()

```cpp
bool syncSchedulesFromFirebase()
```

**Description**: Downloads all schedules from Firebase and updates the local ScheduleManager.

**Parameters**: None

**Returns**:
- `true` - Sync successful
- `false` - Sync failed

**Example**:
```cpp
if (firebase.syncSchedulesFromFirebase()) {
    Serial.println("Schedules synced successfully");
}
```

#### sendHeartbeat()

```cpp
void sendHeartbeat()
```

**Description**: Sends device status heartbeat to Firebase including battery level, WiFi signal, and online status.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
firebase.sendHeartbeat();  // Called every 30 seconds
```

#### logDispenseEvent()

```cpp
void logDispenseEvent(int dispenserId, const String& medicationName, const String& patientName, int pillCount, const String& trigger)
```

**Description**: Logs a dispense event to Firebase with timestamp and details.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)
- `medicationName` (String): Name of medication dispensed
- `patientName` (String): Patient name
- `pillCount` (int): Number of pills dispensed
- `trigger` (String): Trigger type ("schedule", "manual", "remote")

**Returns**: None

**Example**:
```cpp
firebase.logDispenseEvent(0, "Aspirin", "John Doe", 1, "schedule");
```

### Real-time Streaming Methods

#### beginStream()

```cpp
bool beginStream(const String& path)
```

**Description**: Starts real-time streaming for a specific Firebase path.

**Parameters**:
- `path` (String): Firebase path to stream

**Returns**:
- `true` - Stream started successfully
- `false` - Stream failed to start

#### handleStreamCallback()

```cpp
void handleStreamCallback(MultiPathStreamData stream)
```

**Description**: Processes real-time stream data updates from Firebase.

**Parameters**:
- `stream` (MultiPathStreamData): Stream data from Firebase

**Returns**: None

### Utility Methods

#### getDevicePath()

```cpp
String getDevicePath(const String& subPath = "")
```

**Description**: Generates the full Firebase path for device-specific data.

**Parameters**:
- `subPath` (String): Optional sub-path to append

**Returns**: Complete Firebase path as string

**Example**:
```cpp
String batteryPath = firebase.getDevicePath("battery");
// Returns: "/pilldispenser/devices/PILL_DISPENSER_001/battery"
```

#### uploadBatteryData()

```cpp
void uploadBatteryData(float voltage, float percentage, const String& status)
```

**Description**: Uploads battery monitoring data to Firebase.

**Parameters**:
- `voltage` (float): Battery voltage in volts
- `percentage` (float): Battery percentage (0-100)
- `status` (String): Battery status ("Full", "Good", "Low", "Critical")

**Returns**: None

## LCDDisplay API

The LCDDisplay class provides control for the 20x4 I2C LCD display used for status information and user feedback.

### Class Declaration

```cpp
class LCDDisplay {
private:
    LiquidCrystal_I2C lcd;
    static const uint8_t LCD_ADDRESS = 0x27;
    static const uint8_t LCD_COLS = 20;
    static const uint8_t LCD_ROWS = 4;

public:
    LCDDisplay();
    // Method declarations...
};
```

### Constructor

```cpp
LCDDisplay::LCDDisplay()
```

**Description**: Initializes LCD display with default I2C address (0x27).

**Parameters**: None

**Returns**: LCDDisplay instance

### Initialization Methods

#### begin()

```cpp
bool begin()
```

**Description**: Initializes the I2C LCD display and performs backlight test.

**Parameters**: None

**Returns**:
- `true` - LCD initialized successfully
- `false` - LCD initialization failed

**Example**:
```cpp
LCDDisplay lcd;
if (lcd.begin()) {
    Serial.println("LCD initialized successfully");
}
```

### Display Control Methods

#### clear()

```cpp
void clear()
```

**Description**: Clears the LCD display and returns cursor to home position.

**Parameters**: None

**Returns**: None

#### print()

```cpp
void print(const String& text)
```

**Description**: Prints text to the LCD at current cursor position.

**Parameters**:
- `text` (String): Text to display

**Returns**: None

**Example**:
```cpp
lcd.clear();
lcd.print("Pill Dispenser V3");
```

#### setCursor()

```cpp
void setCursor(uint8_t col, uint8_t row)
```

**Description**: Sets the cursor position for subsequent text output.

**Parameters**:
- `col` (uint8_t): Column position (0-19)
- `row` (uint8_t): Row position (0-3)

**Returns**: None

**Example**:
```cpp
lcd.setCursor(0, 1);  // Move to start of second row
lcd.print("Status: Ready");
```

#### showStatus()

```cpp
void showStatus(const String& status, const String& detail = "")
```

**Description**: Displays system status information on the LCD.

**Parameters**:
- `status` (String): Main status message
- `detail` (String): Optional detail message

**Returns**: None

**Example**:
```cpp
lcd.showStatus("Initializing", "WiFi connecting...");
```

#### showDispensing()

```cpp
void showDispensing(int dispenserId, const String& medication, const String& patient)
```

**Description**: Shows dispensing information during pill dispensing.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)
- `medication` (String): Medication name
- `patient` (String): Patient name

**Returns**: None

## NotificationManager API

The NotificationManager class handles SMS notifications for medication reminders and system alerts.

### Class Declaration

```cpp
class NotificationManager {
private:
    SIM800L& sim800;
    std::vector<PhoneNumber> phoneNumbers;
    static const int MAX_PHONE_NUMBERS = 5;

public:
    NotificationManager(SIM800L& sim);
    // Method declarations...
};
```

### Constructor

```cpp
NotificationManager::NotificationManager(SIM800L& sim)
```

**Description**: Initializes notification manager with SIM800L reference.

**Parameters**:
- `sim` (SIM800L&): Reference to SIM800L instance

**Returns**: NotificationManager instance

### Phone Number Management

#### addPhoneNumber()

```cpp
bool addPhoneNumber(const String& number, const String& name)
```

**Description**: Adds a phone number for SMS notifications.

**Parameters**:
- `number` (String): Phone number in international format (+1234567890)
- `name` (String): Contact name identifier

**Returns**:
- `true` - Phone number added successfully
- `false` - Failed to add (max limit reached or invalid)

**Example**:
```cpp
notificationManager.addPhoneNumber("+1234567890", "Caregiver");
```

#### removePhoneNumber()

```cpp
bool removePhoneNumber(const String& number)
```

**Description**: Removes a phone number from the notification list.

**Parameters**:
- `number` (String): Phone number to remove

**Returns**:
- `true` - Phone number removed successfully
- `false` - Phone number not found

### Notification Methods

#### sendReminder()

```cpp
bool sendReminder(int dispenserId, const String& medication, const String& patient, int minutesUntil)
```

**Description**: Sends a medication reminder SMS before scheduled dispensing.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)
- `medication` (String): Medication name
- `patient` (String): Patient name
- `minutesUntil` (int): Minutes until dispensing

**Returns**:
- `true` - SMS sent successfully
- `false` - SMS failed to send

#### sendDispenseNotification()

```cpp
bool sendDispenseNotification(int dispenserId, const String& medication, const String& patient)
```

**Description**: Sends notification after successful pill dispensing.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)
- `medication` (String): Medication name
- `patient` (String): Patient name

**Returns**:
- `true` - SMS sent successfully
- `false` - SMS failed to send

#### sendLowBatteryAlert()

```cpp
bool sendLowBatteryAlert(float percentage)
```

**Description**: Sends low battery warning SMS.

**Parameters**:
- `percentage` (float): Current battery percentage

**Returns**:
- `true` - SMS sent successfully
- `false` - SMS failed to send

#### sendSystemAlert()

```cpp
bool sendSystemAlert(const String& message)
```

**Description**: Sends system error or status alert SMS.

**Parameters**:
- `message` (String): Alert message

**Returns**:
- `true` - SMS sent successfully
- `false` - SMS failed to send

## ScheduleManager API

The ScheduleManager class manages medication dispensing schedules using the TimeAlarms library.

### Class Declaration

```cpp
class ScheduleManager {
private:
    FirebaseManager* firebase;
    std::vector<Schedule> schedules;
    static const int MAX_SCHEDULES = 15;
    AlarmID_t alarmIds[MAX_SCHEDULES];

public:
    ScheduleManager();
    // Method declarations...
};
```

### Constructor

```cpp
ScheduleManager::ScheduleManager()
```

**Description**: Initializes schedule manager with empty schedule list.

**Parameters**: None

**Returns**: ScheduleManager instance

### Initialization Methods

#### begin()

```cpp
void begin(FirebaseManager* fb)
```

**Description**: Initializes schedule manager with Firebase reference.

**Parameters**:
- `fb` (FirebaseManager*): Pointer to FirebaseManager instance

**Returns**: None

### Schedule Management Methods

#### addSchedule()

```cpp
bool addSchedule(int dispenserId, int hour, int minute, bool enabled, const String& medicationName, const String& patientName, const String& pillSize, const std::vector<int>& days)
```

**Description**: Adds a new medication schedule.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)
- `hour` (int): Hour (0-23)
- `minute` (int): Minute (0-59)
- `enabled` (bool): Schedule enabled status
- `medicationName` (String): Medication name
- `patientName` (String): Patient name
- `pillSize` (String): Pill size ("small", "medium", "large")
- `days` (std::vector<int>): Days of week (0=Sun, 1=Mon, ..., 6=Sat)

**Returns**:
- `true` - Schedule added successfully
- `false` - Failed to add schedule

**Example**:
```cpp
std::vector<int> days = {1, 2, 3, 4, 5};  // Monday to Friday
scheduleManager.addSchedule(0, 8, 0, true, "Aspirin", "John Doe", "medium", days);
```

#### removeSchedule()

```cpp
bool removeSchedule(int scheduleId)
```

**Description**: Removes a schedule by ID.

**Parameters**:
- `scheduleId` (int): Schedule identifier

**Returns**:
- `true` - Schedule removed successfully
- `false` - Schedule not found

#### updateSchedule()

```cpp
bool updateSchedule(int scheduleId, int dispenserId, int hour, int minute, bool enabled, const String& medicationName, const String& patientName, const String& pillSize, const std::vector<int>& days)
```

**Description**: Updates an existing schedule.

**Parameters**:
- `scheduleId` (int): Schedule identifier
- `dispenserId` (int): Dispenser number (0-4)
- `hour` (int): Hour (0-23)
- `minute` (int): Minute (0-59)
- `enabled` (bool): Schedule enabled status
- `medicationName` (String): Medication name
- `patientName` (String): Patient name
- `pillSize` (String): Pill size ("small", "medium", "large")
- `days` (std::vector<int>): Days of week

**Returns**:
- `true` - Schedule updated successfully
- `false` - Schedule not found

### Schedule Control Methods

#### enableSchedule()

```cpp
bool enableSchedule(int scheduleId)
```

**Description**: Enables a schedule for execution.

**Parameters**:
- `scheduleId` (int): Schedule identifier

**Returns**:
- `true` - Schedule enabled successfully
- `false` - Schedule not found

#### disableSchedule()

```cpp
bool disableSchedule(int scheduleId)
```

**Description**: Disables a schedule from execution.

**Parameters**:
- `scheduleId` (int): Schedule identifier

**Returns**:
- `true` - Schedule disabled successfully
- `false` - Schedule not found

### Utility Methods

#### getScheduleCount()

```cpp
int getScheduleCount()
```

**Description**: Returns the total number of active schedules.

**Parameters**: None

**Returns**: Number of schedules (int)

#### printSchedules()

```cpp
void printSchedules()
```

**Description**: Prints all schedules to serial monitor for debugging.

**Parameters**: None

**Returns**: None

#### update()

```cpp
void update()
```

**Description**: Updates the TimeAlarms system. Must be called in main loop.

**Parameters**: None

**Returns**: None

**Example**:
```cpp
void loop() {
    scheduleManager.update();  // Check for alarms
    // Other loop code...
}
```

## ServoDriver API

The ServoDriver class provides control for the PCA9685 16-channel PWM servo driver for pill dispensing.

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

**Description**: Initializes servo driver with default I2C address (0x40).

**Parameters**: None

**Returns**: ServoDriver instance

### Initialization Methods

#### begin()

```cpp
bool begin()
```

**Description**: Initializes PCA9685 servo driver and sets PWM frequency.

**Parameters**: None

**Returns**:
- `true` - Initialization successful
- `false` - Initialization failed

**Example**:
```cpp
ServoDriver servoDriver;
if (servoDriver.begin()) {
    Serial.println("Servo driver ready");
}
```

### Dispensing Methods

#### dispensePill()

```cpp
void dispensePill(int dispenserId, const String& pillSize)
```

**Description**: Dispenses a pill from specified dispenser with size-based timing.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)
- `pillSize` (String): Pill size ("small", "medium", "large")

**Returns**: None

**Example**:
```cpp
servoDriver.dispensePill(0, "medium");  // Dispense from dispenser 0
```

#### testDispenser()

```cpp
void testDispenser(int dispenserId)
```

**Description**: Tests a dispenser by performing a short rotation.

**Parameters**:
- `dispenserId` (int): Dispenser number (0-4)

**Returns**: None

**Example**:
```cpp
servoDriver.testDispenser(2);  // Test dispenser 2
```

#### testAllDispensers()

```cpp
void testAllDispensers()
```

**Description**: Tests all 5 dispensers sequentially.

**Parameters**: None

**Returns**: None

### Servo Control Methods

#### setServoSpeed()

```cpp
void setServoSpeed(uint8_t channel, int speed)
```

**Description**: Sets continuous rotation servo speed.

**Parameters**:
- `channel` (uint8_t): Servo channel (0-15)
- `speed` (int): Speed value (300-450, 375=stop)

**Returns**: None

#### stopServo()

```cpp
void stopServo(uint8_t channel)
```

**Description**: Stops a servo by setting PWM to 0.

**Parameters**:
- `channel` (uint8_t): Servo channel (0-15)

**Returns**: None

#### stopAllServos()

```cpp
void stopAllServos()
```

**Description**: Stops all servos on all channels.

**Parameters**: None

**Returns**: None

## SIM800L API

The SIM800L class provides GSM/GPRS functionality for SMS notifications.

### Class Declaration

```cpp
class SIM800L {
private:
    HardwareSerial& serial;
    static const uint32_t BAUD_RATE = 9600;
    bool initialized;

public:
    SIM800L(HardwareSerial& s);
    // Method declarations...
};
```

### Constructor

```cpp
SIM800L::SIM800L(HardwareSerial& s)
```

**Description**: Initializes SIM800L with hardware serial reference.

**Parameters**:
- `s` (HardwareSerial&): Hardware serial port reference

**Returns**: SIM800L instance

### Initialization Methods

#### begin()

```cpp
bool begin()
```

**Description**: Initializes SIM800L module and checks communication.

**Parameters**: None

**Returns**:
- `true` - SIM800L ready
- `false` - Initialization failed

**Example**:
```cpp
SIM800L sim(Serial2);
if (sim.begin()) {
    Serial.println("SIM800L ready for SMS");
}
```

### SMS Methods

#### sendSMS()

```cpp
bool sendSMS(const String& number, const String& message)
```

**Description**: Sends an SMS message to specified number.

**Parameters**:
- `number` (String): Phone number in international format
- `message` (String): SMS message content

**Returns**:
- `true` - SMS sent successfully
- `false` - SMS failed to send

**Example**:
```cpp
sim.sendSMS("+1234567890", "Medication reminder: Take aspirin now");
```

#### readSMS()

```cpp
String readSMS(int index)
```

**Description**: Reads SMS message at specified index.

**Parameters**:
- `index` (int): SMS index number

**Returns**: SMS message content as string

### Utility Methods

#### checkSignal()

```cpp
int checkSignal()
```

**Description**: Checks GSM signal strength.

**Parameters**: None

**Returns**: Signal strength (0-31, higher is better)

#### getNetworkStatus()

```cpp
String getNetworkStatus()
```

**Description**: Gets current network registration status.

**Parameters**: None

**Returns**: Network status as string

## TimeManager API

The TimeManager class handles NTP time synchronization and provides time-related utilities.

### Class Declaration

```cpp
class TimeManager {
private:
    const char* ntpServer;
    long gmtOffset;
    int daylightOffset;
    bool initialized;

public:
    TimeManager();
    // Method declarations...
};
```

### Constructor

```cpp
TimeManager::TimeManager()
```

**Description**: Initializes time manager with default NTP server.

**Parameters**: None

**Returns**: TimeManager instance

### Initialization Methods

#### begin()

```cpp
bool begin(const char* server = "pool.ntp.org", long gmt = 0, int daylight = 0)
```

**Description**: Initializes NTP time synchronization.

**Parameters**:
- `server` (const char*): NTP server address
- `gmt` (long): GMT offset in seconds
- `daylight` (int): Daylight saving offset in seconds

**Returns**:
- `true` - NTP sync successful
- `false` - NTP sync failed

**Example**:
```cpp
TimeManager timeManager;
if (timeManager.begin("time.google.com", 0, 0)) {
    Serial.println("Time synchronized");
}
```

### Time Methods

#### getCurrentTime()

```cpp
String getCurrentTime()
```

**Description**: Gets current time as formatted string.

**Parameters**: None

**Returns**: Time string in "HH:MM:SS" format

#### getCurrentDateTime()

```cpp
String getCurrentDateTime()
```

**Description**: Gets current date and time as formatted string.

**Parameters**: None

**Returns**: DateTime string in "YYYY-MM-DD HH:MM:SS" format

#### syncTime()

```cpp
bool syncTime()
```

**Description**: Manually synchronizes time with NTP server.

**Parameters**: None

**Returns**:
- `true` - Sync successful
- `false` - Sync failed

## VoltageSensor API

The VoltageSensor class monitors battery voltage and calculates battery percentage.

### Class Declaration

```cpp
class VoltageSensor {
private:
    static const uint8_t ADC_PIN = 34;
    static const float VOLTAGE_DIVIDER_RATIO = 5.0;
    static const float MIN_VOLTAGE = 9.0;
    static const float MAX_VOLTAGE = 12.6;

public:
    VoltageSensor();
    // Method declarations...
};
```

### Constructor

```cpp
VoltageSensor::VoltageSensor()
```

**Description**: Initializes voltage sensor with default ADC pin.

**Parameters**: None

**Returns**: VoltageSensor instance

### Measurement Methods

#### readVoltage()

```cpp
float readVoltage()
```

**Description**: Reads battery voltage from ADC pin.

**Parameters**: None

**Returns**: Voltage in volts (float)

**Example**:
```cpp
VoltageSensor battery;
float voltage = battery.readVoltage();
// Returns: 11.8 (for 11.8V battery)
```

#### getBatteryPercentage()

```cpp
float getBatteryPercentage()
```

**Description**: Calculates battery percentage from voltage reading.

**Parameters**: None

**Returns**: Battery percentage (0-100)

**Example**:
```cpp
float percentage = battery.getBatteryPercentage();
// Returns: 85.0 (for healthy battery)
```

#### getBatteryStatus()

```cpp
String getBatteryStatus()
```

**Description**: Gets battery status based on voltage level.

**Parameters**: None

**Returns**: Status string ("Full", "Good", "Low", "Critical")

**Example**:
```cpp
String status = battery.getBatteryStatus();
// Returns: "Good" (for normal battery)
```

## WiFiManager API

The WiFiManager class handles WiFi connection and management.

### Class Declaration

```cpp
class WiFiManager {
private:
    String ssid;
    String password;
    bool connected;

public:
    WiFiManager();
    // Method declarations...
};
```

### Constructor

```cpp
WiFiManager::WiFiManager()
```

**Description**: Initializes WiFi manager.

**Parameters**: None

**Returns**: WiFiManager instance

### Connection Methods

#### connect()

```cpp
bool connect(const String& ssid, const String& password)
```

**Description**: Connects to WiFi network.

**Parameters**:
- `ssid` (String): WiFi network name
- `password` (String): WiFi password

**Returns**:
- `true` - Connection successful
- `false` - Connection failed

**Example**:
```cpp
WiFiManager wifi;
if (wifi.connect("MyWiFi", "password123")) {
    Serial.println("WiFi connected");
}
```

#### disconnect()

```cpp
void disconnect()
```

**Description**: Disconnects from current WiFi network.

**Parameters**: None

**Returns**: None

#### isConnected()

```cpp
bool isConnected()
```

**Description**: Checks WiFi connection status.

**Parameters**: None

**Returns**:
- `true` - Connected to WiFi
- `false` - Not connected

#### getSignalStrength()

```cpp
int getSignalStrength()
```

**Description**: Gets WiFi signal strength (RSSI).

**Parameters**: None

**Returns**: RSSI value (negative dBm, higher is better)

#### getIPAddress()

```cpp
String getIPAddress()
```

**Description**: Gets current IP address.

**Parameters**: None

**Returns**: IP address as string

## Main Application API

The main PillDispenser.ino application coordinates all system components.

### Global Variables

```cpp
// Core components
ServoDriver servoDriver;
LCDDisplay lcd;
SIM800L sim800(Serial2);
NotificationManager notificationManager(sim800);
ScheduleManager scheduleManager;
TimeManager timeManager;
VoltageSensor voltageSensor;
WiFiManager wifiManager;
FirebaseManager firebase;

// Configuration
const String WIFI_SSID = "YOUR_WIFI_SSID";
const String WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const String DEVICE_ID = "PILL_DISPENSER_V3";
```

### Setup Function

```cpp
void setup()
```

**Description**: Initializes all system components in production mode.

**Components Initialized**:
1. Serial communication
2. LCD display
3. WiFi connection
4. Time synchronization (NTP)
5. Firebase connection
6. Servo driver
7. GSM module
8. Battery monitoring
9. Notification system
10. Schedule management

### Loop Function

```cpp
void loop()
```

**Description**: Main system loop handling real-time operations.

**Operations**:
- Firebase data streaming
- Schedule updates
- Battery monitoring
- Serial command processing

### Serial Commands

The system supports various serial commands for testing and debugging:

#### Status Commands
- `status` - Show full system status
- `battery` - Show battery information
- `schedules` - List all schedules

#### Testing Commands
- `test dispenser N` - Test dispenser N (0-4)
- `test sms` - Send test SMS
- `test firebase` - Test Firebase connection
- `test time` - Test NTP sync

#### Control Commands
- `dispense N size` - Manual dispense (N=0-4, size=small/medium/large)
- `help` - Show all available commands

### Production vs Development Mode

The system operates in two modes:

#### Production Mode (Default)
- Schedule enforcement active
- Development commands locked
- Automatic dispensing enabled
- SMS notifications active

#### Development Mode
- All commands available
- Manual override allowed
- Debug output enabled
- Test functions accessible

## Web Dashboard APIs

## Authentication API

The web dashboard uses Firebase Authentication for user management.

### Authentication Methods

#### signInWithEmailAndPassword()

```typescript
async function signInWithEmailAndPassword(email: string, password: string)
```

**Description**: Signs in user with email and password.

**Parameters**:
- `email` (string): User email
- `password` (string): User password

**Returns**: Promise<UserCredential>

#### signOut()

```typescript
async function signOut()
```

**Description**: Signs out current user.

**Parameters**: None

**Returns**: Promise<void>

#### onAuthStateChanged()

```typescript
function onAuthStateChanged(callback: (user: User | null) => void)
```

**Description**: Listens for authentication state changes.

**Parameters**:
- `callback` (function): Callback function called on auth state change

**Returns**: Unsubscribe function

## Dispenser Management API

### Manual Dispense

#### dispensePill()

```typescript
async function dispensePill(dispenserId: number)
```

**Description**: Triggers manual pill dispensing from web dashboard.

**Parameters**:
- `dispenserId` (number): Dispenser number (0-4)

**Returns**: Promise<void>

### Real-time Status

#### subscribeToDeviceStatus()

```typescript
function subscribeToDeviceStatus(callback: (status: DeviceStatus) => void)
```

**Description**: Subscribes to real-time device status updates.

**Parameters**:
- `callback` (function): Callback for status updates

**Returns**: Unsubscribe function

## Schedule Management API

### Schedule CRUD Operations

#### addSchedule()

```typescript
async function addSchedule(dispenserId: number, schedule: ScheduleData)
```

**Description**: Adds a new medication schedule.

**Parameters**:
- `dispenserId` (number): Dispenser number (0-4)
- `schedule` (ScheduleData): Schedule configuration

**Returns**: Promise<void>

#### updateSchedule()

```typescript
async function updateSchedule(scheduleId: string, schedule: ScheduleData)
```

**Description**: Updates an existing schedule.

**Parameters**:
- `scheduleId` (string): Schedule identifier
- `schedule` (ScheduleData): Updated schedule data

**Returns**: Promise<void>

#### deleteSchedule()

```typescript
async function deleteSchedule(scheduleId: string)
```

**Description**: Deletes a schedule.

**Parameters**:
- `scheduleId` (string): Schedule identifier

**Returns**: Promise<void>

### Schedule Data Types

```typescript
interface ScheduleData {
  time: string;          // "HH:MM" format
  enabled: boolean;
  medicationName: string;
  patientName: string;
  pillSize: 'small' | 'medium' | 'large';
  days: number[];        // [0,1,2,3,4,5,6] for Sun-Sat
}

interface DeviceStatus {
  online: boolean;
  lastHeartbeat: number;
  battery: {
    voltage: number;
    percentage: number;
    status: string;
  };
  wifi: {
    rssi: number;
    ipAddress: string;
  };
}
```

---

**Last Updated**: December 2025  
**API Version**: 3.0.0
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