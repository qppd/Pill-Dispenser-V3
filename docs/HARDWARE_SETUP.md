# Hardware Setup Guide

This document provides detailed instructions for assembling and configuring the hardware components of the Pill Dispenser V3 multi-component system including ESP32, Raspberry Pi, and ESP32-CAM.

## Prerequisites

### Tools Required
- Soldering iron and solder
- Wire strippers
- Multimeter
- Screwdrivers (Phillips and flathead)
- Heat shrink tubing
- Breadboard or prototyping board
- MicroSD card writer (for Raspberry Pi)

### Safety Considerations
- Always disconnect power before making connections
- Use appropriate voltage levels (3.3V for ESP32 GPIO)
- Ensure proper grounding of all components
- Use surge protection for power supplies
- Handle Raspberry Pi with ESD protection

## Component Overview

### ESP32 Main Controller Components

| Component | Specifications | Purpose |
|-----------|---------------|---------|
| ESP32 Development Board | ESP32-WROOM-32, 32 GPIO pins | Main microcontroller for dispensing |
| PCA9685 PWM Driver | 16-channel, 12-bit resolution | Servo motor control |
| IR Obstacle Sensors | Digital output, 3-5V operation | Pill detection |
| I2C LCD Display | 20x4 characters, I2C interface | Status display |
| DS1302 RTC Module | Real-time clock with backup battery | Timing functions |
| SIM800L GSM Module | Quad-band GSM/GPRS | Communication |
| Servo Motors | Standard or continuous rotation | Pill dispensing |

### Raspberry Pi ML Processing Components

| Component | Specifications | Purpose |
|-----------|---------------|---------|
| Raspberry Pi 4 | 4GB RAM, Broadcom BCM2711 | Machine learning processing |
| MicroSD Card | 32GB Class 10, UHS-I | OS and storage |
| Power Supply | 5V 3A USB-C | Raspberry Pi power |
| Heat Sinks | Aluminum, 15x15x10mm | Thermal management |
| Camera Cable | 15-pin ribbon cable | ESP32-CAM connection |

### ESP32-CAM Image Capture Components

| Component | Specifications | Purpose |
|-----------|---------------|---------|
| ESP32-CAM Module | AI-Thinker, OV2640 camera | Image capture for pill detection |
| External Antenna | 2.4GHz WiFi antenna | Improved connectivity |
| Power Supply | 3.3V-5V regulated | Camera module power |
| Flash LED | Built-in GPIO 4 | Illumination control |

### Power Supply Requirements

| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| ESP32 Main | 3.3V/5V | 250mA | Via USB or external |
| PCA9685 | 5V | 20mA | Logic supply |
| Servos | 5V | 1-2A each | External supply required |
| LCD | 5V | 50mA | Via I2C backpack |
| DS1302 | 3.3V | 1mA | Plus CR2032 backup |
| SIM800L | 3.7-4.2V | 2A peak | During transmission |
| IR Sensors | 3.3-5V | 20mA each | Per sensor |
| Raspberry Pi | 5V | 3A | Via USB-C |
| ESP32-CAM | 3.3V-5V | 500mA | With camera active |

## Wiring Diagram

### ESP32 Pin Assignments

```
ESP32 GPIO Pins:
├── I2C Bus
│   ├── GPIO 21 (SDA) → PCA9685 SDA, LCD SDA
│   └── GPIO 22 (SCL) → PCA9685 SCL, LCD SCL
├── Digital Inputs
│   ├── GPIO 34 → IR Sensor 1 OUT
│   ├── GPIO 35 → IR Sensor 2 OUT
│   └── GPIO 32 → IR Sensor 3 OUT
├── DS1302 RTC
│   ├── GPIO 18 → DS1302 CLK
│   ├── GPIO 19 → DS1302 DAT
│   └── GPIO 5  → DS1302 RST
├── SIM800L GSM
│   ├── GPIO 16 → SIM800L TX
│   ├── GPIO 17 → SIM800L RX
│   └── GPIO 4  → SIM800L RST
└── Status LED
    └── GPIO 2  → Status LED (with resistor)
```

### Power Distribution

```
Power Distribution:
5V Main Supply → 5V Rail
├── PCA9685 VCC
├── Servo Motors (up to 16x)
├── LCD Backpack VCC
└── Buck Converter → 3.3V Rail
    ├── ESP32 VIN (if not USB powered)
    ├── IR Sensors VCC (3x)
    ├── DS1302 VCC
    └── SIM800L VCC (with filtering)

Ground Common:
All component GND pins connected to common ground rail
```

## Step-by-Step Assembly

### Step 1: Prepare the ESP32 Board

1. **Verify ESP32 Board**:
   - Check for 38-pin configuration
   - Verify onboard USB-to-serial converter
   - Test with simple blink sketch

2. **Header Pin Installation**:
   ```
   Solder header pins for:
   - GPIO pins (both sides)
   - Power pins (3.3V, 5V, GND)
   - I2C pins (21, 22)
   ```

### Step 2: Set Up Power Supply

1. **5V Power Supply**:
   ```
   Specifications:
   - Voltage: 5V ±5%
   - Current: Minimum 5A (for multiple servos)
   - Ripple: <100mV
   - Protection: Short circuit, overcurrent
   ```

2. **Power Distribution Board**:
   ```
   Create power rails:
   - 5V positive rail
   - 3.3V positive rail (via buck converter)
   - Common ground rail
   ```

3. **Filtering and Protection**:
   ```
   Add components:
   - 1000µF capacitor on 5V rail
   - 100µF capacitor on 3.3V rail
   - Fuse protection (5A for 5V rail)
   ```

### Step 3: Install I2C Components

#### PCA9685 Servo Driver

1. **Module Preparation**:
   ```
   Check jumpers:
   - Address jumpers (A0-A5) for 0x40 address
   - Power jumper configuration
   ```

2. **Connections**:
   ```
   PCA9685 → ESP32:
   VCC → 5V
   GND → GND
   SDA → GPIO 21
   SCL → GPIO 22
   V+ → 5V (servo power)
   ```

3. **Servo Connections**:
   ```
   Connect servos to channels 0-15:
   - Red wire → V+ (5V)
   - Brown/Black wire → GND
   - Orange/Yellow wire → PWM channel
   ```

#### I2C LCD Display

1. **I2C Backpack Configuration**:
   ```
   Default address: 0x27
   - Check address with I2C scanner
   - Adjust contrast potentiometer if needed
   ```

2. **Connections**:
   ```
   LCD I2C → ESP32:
   VCC → 5V
   GND → GND
   SDA → GPIO 21
   SCL → GPIO 22
   ```

### Step 4: Install Sensor Components

#### IR Obstacle Sensors

1. **Sensor Preparation**:
   ```
   For each sensor:
   - Verify operation voltage (3.3V or 5V)
   - Test detection range and sensitivity
   ```

2. **Mounting Considerations**:
   ```
   Position sensors to detect pills:
   - Sensor 1: Entrance detection
   - Sensor 2: Middle/transition detection  
   - Sensor 3: Exit/completion detection
   ```

3. **Connections**:
   ```
   Each IR Sensor → ESP32:
   VCC → 3.3V
   GND → GND
   OUT → GPIO (34, 35, 32 respectively)
   ```

#### DS1302 RTC Module

1. **Battery Installation**:
   ```
   Install CR2032 battery:
   - Check polarity (+ side up)
   - Verify 3V output
   ```

2. **Connections**:
   ```
   DS1302 → ESP32:
   VCC → 3.3V
   GND → GND
   CLK → GPIO 18
   DAT → GPIO 19
   RST → GPIO 5
   ```

### Step 5: Install Communication Module

#### SIM800L GSM Module

1. **Power Supply Preparation**:
   ```
   SIM800L requires:
   - Stable 3.7-4.2V supply
   - Peak current: 2A during transmission
   - Add large capacitor (1000µF) near module
   ```

2. **Antenna Installation**:
   ```
   Connect GSM antenna:
   - Use provided antenna or external antenna
   - Ensure good connection to antenna connector
   - Position for optimal signal reception
   ```

3. **SIM Card Installation**:
   ```
   Install SIM card:
   - Power off before insertion
   - Check card orientation
   - Verify network carrier compatibility
   ```

4. **Connections**:
   ```
   SIM800L → ESP32:
   VCC → 3.9V (regulated from 5V)
   GND → GND
   TXD → GPIO 16
   RXD → GPIO 17
   RST → GPIO 4
   ```

### Step 6: Final Assembly

1. **Cable Management**:
   ```
   Organize wiring:
   - Group by voltage level
   - Use cable ties or sleeves
   - Maintain separation between power and signal
   ```

2. **Ground Plane**:
   ```
   Ensure solid grounding:
   - Star ground configuration
   - Heavy gauge wire for ground connections
   - Single point ground reference
   ```

3. **Enclosure Preparation**:
   ```
   Mechanical considerations:
   - Ventilation for heat dissipation
   - Access for status LED
   - Cable entry/exit points
   - Mounting provisions for components
   ```

## Testing and Verification

### Power System Test

1. **Voltage Verification**:
   ```bash
   Check with multimeter:
   - 5V rail: 4.75V - 5.25V
   - 3.3V rail: 3.15V - 3.45V
   - All grounds at 0V potential
   ```

2. **Current Draw Test**:
   ```bash
   Measure current consumption:
   - Idle state: <500mA
   - Single servo operation: +1A
   - GSM transmission: +2A peak
   ```

### I2C Bus Test

1. **Device Detection**:
   ```bash
   Upload I2C scanner sketch:
   Expected devices:
   - 0x27: LCD Display
   - 0x40: PCA9685 Servo Driver
   ```

2. **Communication Test**:
   ```bash
   Test each I2C device:
   - LCD: Display test pattern
   - PCA9685: Servo movement test
   ```

### Sensor Verification

1. **IR Sensor Test**:
   ```bash
   Test each sensor:
   - Normal state: HIGH (3.3V)
   - Blocked state: LOW (0V)
   - Response time: <50ms
   ```

2. **RTC Test**:
   ```bash
   Verify RTC operation:
   - Time keeping accuracy
   - Battery backup function
   - Register read/write
   ```

### Communication Test

1. **Serial Communication**:
   ```bash
   Test ESP32 serial:
   - Baud rate: 115200
   - Clean serial output
   - Command response
   ```

2. **GSM Module Test**:
   ```bash
   Test SIM800L:
   - AT command response
   - Network registration
   - Signal strength reading
   ```

## Troubleshooting

### Power Issues

**Problem**: System not powering on
```
Diagnosis:
1. Check power supply output voltage
2. Verify fuse integrity
3. Check for short circuits
4. Measure current draw

Solution:
- Replace blown fuses
- Repair short circuits
- Verify power supply capacity
```

**Problem**: Servo motors not responding
```
Diagnosis:
1. Check servo power supply (5V)
2. Verify current capacity
3. Test individual servo operation
4. Check PWM signal generation

Solution:
- Increase power supply current rating
- Check servo connections
- Test with external servo tester
```

### Communication Issues

**Problem**: I2C devices not detected
```
Diagnosis:
1. Check SDA/SCL connections
2. Verify pull-up resistors (usually on modules)
3. Test with I2C scanner
4. Check for address conflicts

Solution:
- Verify wiring integrity
- Add pull-up resistors if needed (4.7kΩ)
- Change device addresses if conflicting
```

**Problem**: Serial communication errors
```
Diagnosis:
1. Check baud rate settings
2. Verify TX/RX connections
3. Test with loopback
4. Check for interference

Solution:
- Match baud rates
- Swap TX/RX if needed
- Add ferrite cores for interference
```

### Sensor Issues

**Problem**: IR sensors giving false readings
```
Diagnosis:
1. Check sensor power supply
2. Verify mounting alignment
3. Test detection range
4. Check for ambient light interference

Solution:
- Adjust sensor position
- Add shielding from ambient light
- Calibrate detection threshold
```

**Problem**: RTC losing time
```
Diagnosis:
1. Check backup battery voltage
2. Verify crystal oscillator
3. Test temperature stability
4. Check for power interruptions

Solution:
- Replace CR2032 battery
- Verify crystal connections
- Add temperature compensation
```

## Maintenance

### Regular Checks

**Weekly**:
- Visual inspection of connections
- Power supply voltage verification
- Servo operation test
- Sensor calibration check

**Monthly**:
- Deep cleaning of components
- Connection tightness check
- Power consumption measurement
- Battery voltage check

**Quarterly**:
- Complete system test
- Firmware update check
- Mechanical wear inspection
- Performance optimization

### Replacement Schedule

| Component | Expected Life | Replacement Indicator |
|-----------|---------------|----------------------|
| CR2032 Battery | 2-3 years | RTC losing time |
| Servo Motors | 10,000+ cycles | Mechanical wear, noise |
| Power Capacitors | 5+ years | Voltage drop, ripple |
| IR Sensors | 50,000+ hours | Sensitivity degradation |

## Safety Guidelines

### Electrical Safety
- Always disconnect power before modifications
- Use proper ESD protection
- Verify voltage levels before connections
- Implement proper fusing and protection

### Mechanical Safety
- Secure all moving parts
- Provide emergency stop mechanisms
- Use appropriate enclosures
- Implement proper ventilation

### Operational Safety
- Monitor for overheating
- Implement fault detection
- Provide status indicators
- Document all modifications

---

This hardware setup guide provides comprehensive instructions for assembling a reliable and safe Pill Dispenser V3 system. Follow all steps carefully and perform thorough testing before deployment.