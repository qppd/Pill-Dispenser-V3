# ESP32-Arduino Servo Control Architecture

## Overview

The Pill Dispenser V3 system has been refactored to separate servo control responsibilities between two microcontrollers:

- **ESP32**: System logic, scheduling, sensors, GSM, WiFi, Firebase
- **Arduino Uno**: Dedicated servo controller via PCA9685

## Architecture

### Communication Protocol

**Hardware Connection:**
- ESP32 GPIO25 (RX) ← Arduino Pin 3 (TX)
- ESP32 GPIO26 (TX) → Arduino Pin 2 (RX)
- Baud Rate: 9600
- **Don't forget to connect GND between both boards!**

**Software Implementation:**
- ESP32 uses UART1 (Serial1) via `HardwareSerial`
- Arduino uses SoftwareSerial on pins 2 & 3
- Text-based command protocol with newline delimiters
- Response format: `OK:` (success) or `ERROR:` (failure)

### Command Set

#### Basic Commands
| Command | Description | Response |
|---------|-------------|----------|
| `PING` | Test connection | `PONG` |
| `STATUS` | Get system status | `OK:READY` |

#### Servo Control Commands
| Command | Format | Example | Description |
|---------|--------|---------|-------------|
| `SET_ANGLE` | `SET_ANGLE:<channel>,<angle>` | `SET_ANGLE:0,90` | Set servo to angle (0-180°) |
| `DISPENSE` | `DISPENSE:<channel>,<size>` | `DISPENSE:2,medium` | Dispense pill (size: small/medium/large) |
| `DISPENSE_PAIR` | `DISPENSE_PAIR:<ch1>,<ch2>,<size>` | `DISPENSE_PAIR:0,1,large` | Dispense from two channels |
| `TEST_SERVO` | `TEST_SERVO:<channel>` | `TEST_SERVO:3` | Test servo through full range |
| `CALIBRATE` | `CALIBRATE:<channel>` | `CALIBRATE:0` | Calibrate servo range |
| `RESET_ALL` | `RESET_ALL` | `RESET_ALL` | Reset all servos to 90° |
| `STOP_ALL` | `STOP_ALL` | `STOP_ALL` | Stop all servos |

#### Async Messages (from Arduino to ESP32)
- `HEARTBEAT` - Sent every 5 seconds to indicate Arduino is alive
- `READY` - Sent on startup when Arduino is initialized
- `INIT:OK` - Sent after PCA9685 initialization

## File Structure

### Arduino Uno Files
```
source/arduino/PillDispenserUno/
└── PillDispenserUno.ino    - Main servo controller with command processing
```

### ESP32 Files
```
source/esp32/PillDispenser/
├── ArduinoServoController.h    - Header for serial servo control
├── ArduinoServoController.cpp  - Implementation of serial communication
├── PillDispenser.ino           - Main ESP32 code (updated to use serial)
└── PINS_CONFIG.h               - Pin definitions
```

### Removed Files
- `ServoDriver.h` - No longer needed (functionality moved to Arduino)
- `ServoDriver.cpp` - No longer needed

## ESP32 Implementation

### ArduinoServoController Class

The `ArduinoServoController` class provides a clean interface that mimics the original `ServoDriver` API:

```cpp
ArduinoServoController servoController(PIN_UNO_RX, PIN_UNO_TX);

// Initialize
servoController.begin();

// Check connection
if (servoController.isConnected()) {
    // Control servos
    servoController.setServoAngle(0, 90);
    servoController.dispensePill(2, "medium");
    servoController.dispensePillPair(0, 1, "large");
}

// Update to process async messages
servoController.update();
```

### Key Methods

#### Initialization
```cpp
bool begin()
```
- Initializes UART1 communication
- Waits for Arduino READY signal (up to 5 seconds)
- Returns true if connection established

#### Control Methods
```cpp
bool setServoAngle(uint8_t channel, uint16_t angle)
bool dispensePill(uint8_t channel, String pillSize)
bool dispensePillPair(uint8_t ch1, uint8_t ch2, String pillSize)
bool testServo(uint8_t channel)
bool calibrateServo(uint8_t channel)
bool resetAllServos()
bool stopAllServos()
```

All methods return `bool`:
- `true` = command successful
- `false` = communication error or invalid parameters

#### Monitoring
```cpp
void update()
```
- Call in main loop to process async messages
- Handles heartbeat messages
- Maintains connection status

## Arduino Implementation

### Main Features

1. **Command Processing Loop**
   - Continuously monitors serial for commands
   - Parses and validates commands
   - Executes servo operations
   - Sends responses

2. **Servo Control Functions**
   - Direct PCA9685 control via I2C
   - Angle mapping (0-180° to PWM values)
   - Pill dispensing sequences
   - Test and calibration routines

3. **Error Handling**
   - Parameter validation
   - Clear error messages
   - Graceful failure recovery

## Benefits of This Architecture

### 1. **Separation of Concerns**
- ESP32 focuses on high-level logic
- Arduino handles real-time servo control
- Clear interface boundary

### 2. **Reliability**
- Dedicated microcontroller for servo control
- No I2C conflicts between ESP32 modules
- Independent failure domains

### 3. **Flexibility**
- Easy to test Arduino servo control independently
- Can update servo logic without touching ESP32
- Simple protocol for future expansion

### 4. **Debugging**
- Arduino has USB serial for direct debugging
- ESP32 logs all serial communication
- Heartbeat monitoring for health checks

### 5. **Performance**
- Non-blocking serial communication
- Async message handling
- Timeout protection

## Migration from ServoDriver

### Code Changes Required

**Before (using ServoDriver):**
```cpp
#include "ServoDriver.h"

ServoDriver servoDriver;

void setup() {
    servoDriver.begin();
}

void loop() {
    if (servoDriver.isConnected()) {
        servoDriver.dispensePill(0, "medium");
    }
}
```

**After (using ArduinoServoController):**
```cpp
#include "ArduinoServoController.h"

ArduinoServoController servoController(PIN_UNO_RX, PIN_UNO_TX);

void setup() {
    servoController.begin();
}

void loop() {
    servoController.update(); // Process async messages
    
    if (servoController.isConnected()) {
        servoController.dispensePill(0, "medium");
    }
}
```

### API Compatibility

Most methods have identical signatures, making migration straightforward:

| ServoDriver Method | ArduinoServoController Method | Notes |
|-------------------|------------------------------|-------|
| `begin()` | `begin()` | ✅ Same |
| `isConnected()` | `isConnected()` | ✅ Same |
| `setServoAngle()` | `setServoAngle()` | ✅ Same |
| `dispensePill()` | `dispensePill()` | ✅ Same |
| `dispensePillPair()` | `dispensePillPair()` | ✅ Same |
| `testServo()` | `testServo()` | ✅ Same |
| `calibrateServo()` | `calibrateServo()` | ✅ Same |
| `resetAllServos()` | `resetAllServos()` | ✅ Same |
| `stopAllServos()` | `stopAllServos()` | ✅ Same |
| N/A | `update()` | ⚠️ New - call in loop() |
| N/A | `ping()` | ⚠️ New - test connectivity |
| N/A | `checkStatus()` | ⚠️ New - verify readiness |

**Key Difference:** Add `servoController.update()` to your main loop to process async messages.

## Testing

### Test Commands (via Serial Monitor at 115200 baud)

```
servo status        - Check Arduino connection
servo test 0        - Test servo 0
servo sweep         - Test all servos (0-4)
servo reset         - Reset all servos to 90°
servo stop          - Stop all servos
dispense 1          - Manually dispense from container 1
calibrate 0         - Calibrate servo 0
help                - Show all commands
```

### Verification Steps

1. **Upload Arduino Code**
   - Upload `PillDispenserUno.ino` to Arduino Uno
   - Open Serial Monitor (9600 baud)
   - Verify "Arduino Ready" message

2. **Upload ESP32 Code**
   - Upload `PillDispenser.ino` to ESP32
   - Open Serial Monitor (115200 baud)
   - Look for "Arduino Servo Controller: ✅ OK"

3. **Test Communication**
   - Type `servo status` in ESP32 Serial Monitor
   - Should see "✅ Arduino Uno connected and responding"

4. **Test Servo Control**
   - Type `servo test 0` to test first servo
   - Type `dispense 1` to test pill dispensing

## Troubleshooting

### Arduino Not Responding

**Symptoms:**
- ESP32 shows "Failed to connect to Arduino"
- Timeout errors

**Solutions:**
1. Check wiring:
   - ESP32 GPIO25 → Arduino Pin 3
   - ESP32 GPIO26 → Arduino Pin 2
   - GND → GND
2. Verify Arduino is powered and code uploaded
3. Check Arduino Serial Monitor for "Arduino Ready"
4. Try manual PING from ESP32: Type `servo status`

### Servos Not Moving

**Symptoms:**
- Commands succeed but servos don't move
- Arduino reports "OK" but no physical movement

**Solutions:**
1. Check PCA9685 connections (SDA/SCL)
2. Verify PCA9685 power supply (5V, adequate current)
3. Check servo connections to PCA9685
4. Test directly on Arduino: Upload test sketch
5. Verify I2C address (default: 0x40)

### Communication Errors

**Symptoms:**
- Intermittent timeouts
- Garbled responses

**Solutions:**
1. Ensure baud rate matches (9600)
2. Check for loose connections
3. Keep wires short and away from noise sources
4. Add pull-up resistors if needed (4.7kΩ)

### Heartbeat Not Received

**Symptoms:**
- Connection drops after initial success
- No heartbeat messages

**Solutions:**
1. Arduino may be stuck in servo operation
2. Check Arduino Serial Monitor for errors
3. Restart both microcontrollers
4. Verify Arduino loop() is not blocking

## Performance Considerations

### Timeouts

Default timeout: 2000ms (2 seconds)

Adjust for specific operations:
- Quick commands: 1000ms (ping, status)
- Servo operations: 2000ms (setServoAngle)
- Dispensing: 5000ms (includes 2s wait time)
- Calibration: 8000ms (multiple movements)

### Non-Blocking Operation

The `update()` method is non-blocking and should be called frequently:

```cpp
void loop() {
    servoController.update();  // Process async messages
    
    // Other non-blocking code here
    
    Alarm.delay(100);
}
```

### Message Buffer

- Arduino sends heartbeat every 5 seconds
- ESP32 processes messages in `update()`
- No buffer overflow - messages are consumed immediately

## Future Enhancements

### Potential Additions

1. **Extended Command Set**
   - `SET_SPEED:<channel>,<speed>` - Control servo speed
   - `GET_POSITION:<channel>` - Read current position
   - `BATCH:<cmd1>;<cmd2>;...` - Execute multiple commands

2. **Enhanced Monitoring**
   - Current sensing for servo load detection
   - Position feedback for verification
   - Error counters and diagnostics

3. **Configuration**
   - Dynamic servo limits per channel
   - Configurable pulse widths
   - Per-channel calibration data

4. **Advanced Features**
   - Smooth movement profiles
   - Coordinated multi-servo moves
   - Emergency stop broadcast

## Conclusion

This architecture provides a robust, maintainable solution for servo control in the Pill Dispenser V3 system. The separation of responsibilities between ESP32 and Arduino Uno ensures reliable operation while maintaining code clarity and ease of debugging.

For questions or issues, refer to the troubleshooting section or check the source code comments.

---

**Last Updated:** December 15, 2025  
**Version:** 3.0  
**Author:** Pill Dispenser V3 Team
