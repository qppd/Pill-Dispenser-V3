# Hardware Setup Guide - Pill Dispenser V3

This document provides detailed instructions for assembling and configuring the hardware components of the Pill Dispenser V3 ESP32-based medication dispensing system.

## Prerequisites

### Tools Required
- Soldering iron and solder
- Wire strippers
- Multimeter
- Screwdrivers (Phillips and flathead)
- Heat shrink tubing
- Breadboard or prototyping board
- Computer for programming

### Safety Considerations
- Always disconnect power before making connections
- Use appropriate voltage levels (3.3V for ESP32 GPIO)
- Ensure proper grounding of all components
- Use surge protection for power supplies
- Handle components with ESD protection

## Component Overview

### ESP32 Main Controller Components

| Component | Specifications | Purpose |
|-----------|---------------|---------|
| ESP32 Development Board | ESP32-WROOM-32, 32 GPIO pins | Main microcontroller for dispensing |
| PCA9685 PWM Driver | 16-channel, 12-bit resolution | Servo motor control |
| I2C LCD Display | 20x4 characters, I2C interface | Status display |
| SIM800L GSM Module | Quad-band GSM/GPRS | SMS notifications |
| Voltage Sensor | Analog divider circuit | Battery monitoring |
| Servo Motors | 5x continuous rotation | Pill dispensing |

### Power Supply Requirements

| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| ESP32 Main | 3.3V/5V | 250mA | Via USB or external |
| PCA9685 | 3.3V/5V | 20mA | Logic supply / servo power |
| Servos | 5V | 1-2A each | External supply required |
| LCD | 5V | 50mA | Via I2C backpack |
| SIM800L | 3.7-4.2V | 2A peak | Dedicated supply during transmission |
| Voltage Sensor | Battery voltage | <1mA | ADC input |

## Wiring Diagram

### ESP32 Pin Assignments

```
ESP32 GPIO Pins:
├── I2C Bus
│   ├── GPIO 21 (SDA) → PCA9685 SDA, LCD SDA
│   └── GPIO 22 (SCL) → PCA9685 SCL, LCD SCL
├── Serial Communication
│   ├── GPIO 16 → SIM800L TX
│   ├── GPIO 17 → SIM800L RX
│   └── GPIO 4  → SIM800L RST
├── Analog Input
│   └── GPIO 34 → Voltage Sensor (ADC1_CH5)
└── Status LED
    └── GPIO 2  → Status LED (with resistor)
```

### Power Distribution

```
Power Distribution:
├── 5V Main Supply → 5V Rail
│   ├── PCA9685 V+ (servo power)
│   ├── LCD VCC
│   └── Servo Motors VCC (all 5)
├── 3.3V Supply → 3.3V Rail
│   ├── ESP32 VCC
│   └── PCA9685 VCC (logic)
├── Battery Supply → Voltage Sensor
│   └── Battery + → Voltage Divider → GPIO 34
└── SIM800L Supply → 3.7-4.2V Rail
    └── SIM800L VCC (dedicated regulator)
```

## Assembly Instructions

### Step 1: ESP32 Setup

1. **Prepare ESP32 Development Board**
   - Ensure ESP32 Dev Module is selected in Arduino IDE
   - Verify board has 4MB flash memory
   - Check USB connection for programming

2. **Initial Testing**
   - Connect ESP32 to computer via USB
   - Open Arduino IDE Serial Monitor
   - Verify basic communication at 115200 baud

### Step 2: I2C Bus Setup

1. **Connect PCA9685 PWM Driver**
   ```
   PCA9685 → ESP32
   SDA    → GPIO 21
   SCL    → GPIO 22
   VCC    → 3.3V (logic power)
   V+     → 5V (servo power)
   GND    → Ground
   ```

2. **Connect LCD Display**
   ```
   LCD → ESP32 (via I2C)
   SDA → GPIO 21 (shared)
   SCL → GPIO 22 (shared)
   VCC → 5V
   GND → Ground
   ```

3. **I2C Pull-up Resistors**
   - Add 4.7kΩ resistors between SDA/SCL and 3.3V
   - Required for reliable I2C communication

4. **Address Configuration**
   - PCA9685: Default address 0x40
   - LCD: Usually 0x27 or 0x3F (check with scanner)

### Step 3: Servo Motor Connections

1. **Connect Servo Motors to PCA9685**
   ```
   Servo 0 → PCA9685 Channel 0 (Dispenser 1)
   Servo 1 → PCA9685 Channel 1 (Dispenser 2)
   Servo 2 → PCA9685 Channel 2 (Dispenser 3)
   Servo 3 → PCA9685 Channel 3 (Dispenser 4)
   Servo 4 → PCA9685 Channel 4 (Dispenser 5)
   ```

2. **Servo Power Distribution**
   - All servo VCC wires → 5V rail
   - All servo GND wires → Ground rail
   - Signal wires → PCA9685 PWM channels 0-4

3. **External Power Supply**
   - Use dedicated 5V 2-3A supply for servos
   - Do not power servos from ESP32
   - Ensure common ground with ESP32

### Step 4: SIM800L GSM Module Setup

1. **Power Supply Requirements**
   - SIM800L requires 3.7-4.2V (typically 4V)
   - Peak current up to 2A during transmission
   - Use dedicated regulator, not shared with ESP32

2. **Serial Connections**
   ```
   SIM800L → ESP32
   TX     → GPIO 16
   RX     → GPIO 17
   RST    → GPIO 4
   GND    → Ground
   ```

3. **SIM Card Installation**
   - Insert activated SIM card
   - Ensure SIM has SMS capability
   - Disable PIN lock if present
   - Check for sufficient balance/credits

4. **Antenna Connection**
   - Connect GSM antenna to SIM800L antenna connector
   - Ensure antenna is properly seated
   - External antenna required for reliable signal

5. **Power Filtering**
   - Add 100µF electrolytic capacitor near power pins
   - Add 10µF ceramic capacitor for noise filtering
   - Use thick wires for power connections

### Step 5: Voltage Sensor Setup

1. **Voltage Divider Circuit**
   ```
   Battery +
   │
   ├─[10kΩ]─┬─[10kΩ]── GND
   │        │
   └────────┼─ ESP32 GPIO 34 (ADC)
   ```

2. **Calibration**
   - Ratio: 5:1 (10kΩ + 10kΩ) / 10kΩ = 2:1, but ESP32 ADC reference makes effective ratio 5:1
   - Input range: 9.0V - 12.6V (3S Li-ion battery)
   - ADC readings: ~372 (9V) to ~516 (12.6V)

3. **Battery Connection**
   - Connect battery positive to voltage divider input
   - Connect battery negative to ground
   - Ensure secure connections

### Step 6: Status LED Setup

1. **LED Connection**
   ```
   ESP32 GPIO 2 → 220Ω Resistor → LED Anode
   LED Cathode → Ground
   ```

2. **LED States**
   - OFF: System off or error
   - ON: System running normally
   - Blinking: Initialization or activity

## Testing Procedures

### I2C Bus Testing

1. **Upload I2C Scanner Sketch**
   ```cpp
   #include <Wire.h>
   
   void setup() {
     Wire.begin();
     Serial.begin(115200);
   }
   
   void loop() {
     Serial.println("Scanning I2C bus...");
     for (byte address = 1; address < 127; address++) {
       Wire.beginTransmission(address);
       if (Wire.endTransmission() == 0) {
         Serial.print("Device found at 0x");
         Serial.println(address, HEX);
       }
     }
     delay(5000);
   }
   ```

2. **Expected Results**
   - PCA9685 at address 0x40
   - LCD at address 0x27 or 0x3F

### Servo Testing

1. **Individual Servo Test**
   ```cpp
   // Test each servo channel
   servoDriver.setServoSpeed(0, 400); // Forward
   delay(1000);
   servoDriver.setServoSpeed(0, 375); // Stop
   ```

2. **Calibration**
   - Adjust speed values for consistent rotation
   - Verify pill dispensing mechanism
   - Test all 5 dispensers

### SIM800L Testing

1. **Basic Communication**
   - Open Serial Monitor at 9600 baud
   - Send AT command
   - Expect OK response

2. **Signal Strength**
   - Send AT+CSQ command
   - Check signal quality (0-31, higher better)

3. **SMS Test**
   - Send test SMS to verify functionality
   - Check delivery and format

### Voltage Sensor Testing

1. **ADC Reading Test**
   ```cpp
   int rawValue = analogRead(34);
   float voltage = rawValue * (3.3 / 4095.0) * 5.0; // Adjust multiplier
   Serial.println(voltage);
   ```

2. **Calibration**
   - Measure actual battery voltage
   - Adjust voltage divider multiplier
   - Verify percentage calculation

## Troubleshooting

### Common Issues

**I2C Communication Problems**
- Check pull-up resistors (4.7kΩ)
- Verify wiring connections
- Check I2C addresses
- Test with I2C scanner

**Servo Motor Issues**
- Verify external power supply (5V 2A+)
- Check PCA9685 power connections
- Test individual servo channels
- Verify PWM frequency (50Hz)

**SIM800L Problems**
- Check power supply (3.7-4.2V, 2A peak)
- Verify serial connections and baud rate
- Test with AT commands
- Check SIM card and antenna

**Voltage Reading Issues**
- Verify voltage divider circuit
- Check ADC pin connection (GPIO34)
- Calibrate voltage multiplier
- Test with known voltage source

**ESP32 Connection Problems**
- Check USB driver installation
- Verify board selection in Arduino IDE
- Test with blink sketch
- Check power supply stability

## Final Assembly

### Enclosure Considerations

1. **Component Placement**
   - ESP32 accessible for programming
   - LCD visible for status display
   - SIM800L antenna accessible
   - Servo motors securely mounted

2. **Cable Management**
   - Use cable ties for organization
   - Secure connections to prevent vibration
   - Label wires for maintenance

3. **Power Distribution**
   - Central power distribution point
   - Fused protection for servo power
   - Easy access to power switches

### Environmental Considerations

1. **Temperature**
   - Operating range: 0-40°C
   - Avoid direct sunlight
   - Ensure ventilation for heat dissipation

2. **Humidity**
   - Avoid condensation
   - Use desiccant packs if needed
   - Protect electronics from moisture

3. **Vibration**
   - Secure all components
   - Use vibration-damping mounts
   - Test in operating environment

## Maintenance Procedures

### Regular Maintenance

**Weekly Checks**
- Verify all connections secure
- Check servo motor operation
- Test LCD display functionality
- Verify battery voltage readings

**Monthly Maintenance**
- Clean dust from components
- Check antenna connections
- Verify SIM card functionality
- Update firmware if available

### Component Replacement

**Servo Motor Replacement**
1. Disconnect power
2. Remove old servo
3. Connect new servo to same channel
4. Test operation
5. Calibrate if necessary

**SIM Card Replacement**
1. Power off system
2. Remove old SIM card
3. Insert new SIM card
4. Configure phone number in software
5. Test SMS functionality

**Battery Replacement**
1. Disconnect old battery
2. Connect new battery
3. Verify voltage readings
4. Update battery calibration if needed

## Safety Guidelines

### Electrical Safety
- Always disconnect power before working
- Use insulated tools
- Avoid short circuits
- Test voltage levels before connecting

### Component Handling
- Handle ESP32 by edges only
- Avoid static discharge
- Use proper soldering techniques
- Verify connections before power-on

### System Operation
- Monitor system status regularly
- Respond to error conditions promptly
- Keep backup power available
- Maintain system logs for troubleshooting

---

**Last Updated**: December 2025
**Hardware Version**: 3.0.0

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