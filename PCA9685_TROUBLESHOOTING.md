# PCA9685 I2C Troubleshooting Guide

## Software I2C Option

If hardware I2C continues to fail, you can use software I2C (bit-banging) instead:

### Installation:
1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search for "**SoftWire**" by **Testato**
4. Install the library

### Configuration:
1. **Uncomment this line** in `PCA9685_CONFIG.h`:
   ```cpp
   #define USE_SOFTWARE_I2C
   ```
2. **Change I2C pins** if needed (default: GPIO 21 SDA, GPIO 22 SCL):
   ```cpp
   #define SOFT_I2C_SDA_PIN 21
   #define SOFT_I2C_SCL_PIN 22
   ```
3. **Upload and test** - software I2C is slower but more reliable on some boards

**Note:** Software I2C uses any GPIO pins but is slower (~50kHz max) and uses more CPU.

## Common I2C Issues and Solutions

### Error Messages You're Seeing:
- `I2C hardware timeout detected`
- `I2C transaction failed`
- `I2C hardware NACK detected`
- `I2C transaction unexpected nack detected`

### Step-by-Step Troubleshooting:

#### 1. **Check Power Supply**
- PCA9685 requires **5V power supply**
- ESP32 can provide 5V from VIN pin when powered by 5V
- **Current draw**: Each servo draws ~100-200mA, 5 servos = 500-1000mA total
- Use external 5V power supply if ESP32 can't provide enough current

#### 2. **Verify I2C Wiring**
```
ESP32    →  PCA9685
--------------------
3.3V     →  VCC     (if using 3.3V variant, otherwise use 5V)
GND      →  GND
GPIO 21  →  SDA
GPIO 22  →  SCL
```

#### 3. **Check I2C Pull-up Resistors**
- PCA9685 requires pull-up resistors on SDA/SCL lines
- Most breakout boards have them (4.7kΩ)
- If not, add 4.7kΩ resistors between SDA→3.3V and SCL→3.3V

#### 4. **Verify I2C Address**
- Default address: `0x40`
- Check if your board uses different address (A0-A5 pins)
- Use `scan_i2c` command to find connected devices

#### 5. **Test I2C Connection**
1. Upload code and open Serial Monitor
2. Send command: `scan_i2c`
3. Should see: `I2C device found at address 0x40`

#### 6. **Check Servo Connections**
- Servos connect to PWM outputs 0-4
- Ensure servo power is connected (red wire to 5V/6V)
- Servo ground to common ground

#### 7. **ESP32 I2C Pin Conflicts**
- GPIO 21 (SDA) and GPIO 22 (SCL) are default I2C pins
- If using different pins, update `I2C_SDA_PIN` and `I2C_SCL_PIN`

#### 8. **Serial Commands for Testing**
```
scan_i2c              - Scan for I2C devices
test_all_pill_dispenser - Test all servos sequentially
servo 0 90           - Set servo 0 to 90 degrees
stop_test            - Stop servo testing
```

#### 9. **Hardware Checklist**
- [ ] ESP32 powered correctly (5V recommended)
- [ ] PCA9685 powered with 5V
- [ ] I2C wires connected: SDA(21)→SDA, SCL(22)→SCL, GND→GND
- [ ] Pull-up resistors present (usually on PCA9685 board)
- [ ] Servo power connected (5-6V, adequate current)
- [ ] Servo signal wires to PCA9685 outputs 0-4
- [ ] All grounds connected together

#### 10. **If Still Not Working**
- Try different I2C address in code: `Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);`
- Check for short circuits on I2C lines
- Try different ESP32 board (some clones have issues)
- Use I2C logic analyzer to debug signals

### Expected Serial Output:
```
Initializing I2C bus...
I2C SDA Pin: 21
I2C SCL Pin: 22
I2C Frequency: 100 kHz
Scanning I2C bus...
I2C device found at address 0x40
Testing PCA9685 communication...
PCA9685 MODE1 register: 0x11
PCA9685 initialized successfully!
```</content>
<parameter name="filePath">C:\Users\sajed\Desktop\PROJECTS\PILL_DISPENSER_V3\PCA9685_TROUBLESHOOTING.md