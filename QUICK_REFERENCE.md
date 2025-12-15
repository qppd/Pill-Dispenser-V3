# Quick Reference: ESP32-Arduino Servo Control

## Wiring

```
ESP32 GPIO25 (RX) ← Arduino Pin 3 (TX)
ESP32 GPIO26 (TX) → Arduino Pin 2 (RX)
GND              ↔ GND (IMPORTANT!)
```

## Upload Order

1. **Arduino Uno First**
   - Open: `source/arduino/PillDispenserUno/PillDispenserUno.ino`
   - Upload to Arduino Uno
   - Verify "Arduino Ready" in Serial Monitor (9600 baud)

2. **ESP32 Second**
   - Open: `source/esp32/PillDispenser/PillDispenser.ino`
   - Upload to ESP32
   - Look for "Arduino Servo Controller: ✅ OK" (115200 baud)

## Command Reference

### From ESP32 Serial Monitor (115200 baud)

| Command | Action |
|---------|--------|
| `servo status` | Check Arduino connection |
| `servo test 0` | Test servo channel 0 |
| `servo sweep` | Test all servos (0-4) |
| `dispense 1` | Dispense from container 1 |
| `calibrate 0` | Calibrate servo 0 |
| `servo reset` | Reset all servos to 90° |
| `servo stop` | Stop all servos |
| `help` | Show all commands |

### Protocol Commands (Serial)

| Command | Format | Example |
|---------|--------|---------|
| Ping | `PING` | `PING` |
| Status | `STATUS` | `STATUS` |
| Set Angle | `SET_ANGLE:<ch>,<angle>` | `SET_ANGLE:0,90` |
| Dispense | `DISPENSE:<ch>,<size>` | `DISPENSE:2,medium` |
| Test | `TEST_SERVO:<ch>` | `TEST_SERVO:0` |
| Calibrate | `CALIBRATE:<ch>` | `CALIBRATE:0` |
| Reset All | `RESET_ALL` | `RESET_ALL` |
| Stop All | `STOP_ALL` | `STOP_ALL` |

## Code Usage

### ESP32 Code

```cpp
#include "ArduinoServoController.h"

ArduinoServoController servoController(PIN_UNO_RX, PIN_UNO_TX);

void setup() {
    servoController.begin();
}

void loop() {
    servoController.update();  // Important: Process async messages
    
    if (servoController.isConnected()) {
        servoController.setServoAngle(0, 90);
        servoController.dispensePill(2, "medium");
    }
}
```

### Arduino Code

Already implemented in `PillDispenserUno.ino` - just upload it!

## Troubleshooting Quick Fix

**Problem:** Arduino not responding

```cpp
// Check wiring
// Verify Arduino Serial Monitor shows "Arduino Ready"
// ESP32: Type "servo status" to test
```

**Problem:** Servos not moving

```cpp
// Check PCA9685 I2C connections (SDA/SCL)
// Verify power supply to PCA9685
// Check servo connections to PCA9685
```

**Problem:** Timeout errors

```cpp
// Ensure GND is connected between boards
// Check baud rate: 9600 on both sides
// Verify wiring: GPIO25↔Pin3, GPIO26↔Pin2
```

## Pin Configuration

### ESP32 Pins (PINS_CONFIG.h)

```cpp
#define PIN_UNO_RX 25   // ESP32 RX <- Arduino TX (Pin 3)
#define PIN_UNO_TX 26   // ESP32 TX -> Arduino RX (Pin 2)
```

### Arduino Pins

```cpp
SoftwareSerial ESP32Serial(2, 3); // RX=Pin2, TX=Pin3
```

## Response Format

**Success:**
- `OK:READY`
- `OK:SET_ANGLE:0,90`
- `OK:DISPENSED:2,medium`
- `PONG`

**Error:**
- `ERROR:Invalid channel`
- `ERROR:Unknown command`
- `TIMEOUT`

## Files Modified

### New Files
- ✅ `source/esp32/PillDispenser/ArduinoServoController.h`
- ✅ `source/esp32/PillDispenser/ArduinoServoController.cpp`
- ✅ `ESP32_ARDUINO_ARCHITECTURE.md`
- ✅ `QUICK_REFERENCE.md`

### Modified Files
- ✅ `source/arduino/PillDispenserUno/PillDispenserUno.ino` - Complete rewrite
- ✅ `source/esp32/PillDispenser/PillDispenser.ino` - Updated to use serial

### Removed (no longer needed)
- ❌ `ServoDriver.h` - Functionality moved to Arduino
- ❌ `ServoDriver.cpp` - Functionality moved to Arduino

## Testing Checklist

- [ ] Arduino uploads successfully
- [ ] Arduino Serial Monitor shows "Arduino Ready"
- [ ] ESP32 uploads successfully
- [ ] ESP32 shows "Arduino Servo Controller: ✅ OK"
- [ ] Command `servo status` works
- [ ] Command `servo test 0` moves servo
- [ ] Command `dispense 1` triggers dispensing
- [ ] Heartbeat messages appear every 5 seconds

## Support

See [ESP32_ARDUINO_ARCHITECTURE.md](ESP32_ARDUINO_ARCHITECTURE.md) for detailed documentation.

---

**Last Updated:** December 15, 2025
