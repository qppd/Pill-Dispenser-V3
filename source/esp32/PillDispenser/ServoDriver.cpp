#include "ServoDriver.h"

ServoDriver::ServoDriver() : pwm(I2C_ADDRESS) {
  // Constructor uses default I2C address
}

void ServoDriver::scanI2CDevices() {
  Serial.println("ServoDriver: Scanning I2C bus for devices...");
  int deviceCount = 0;
  
  for (int address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    int error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("ServoDriver: I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Identify common devices
      if (address == 0x27 || address == 0x3F) {
        Serial.println(" (LCD Display)");
      } else if (address == 0x40) {
        Serial.println(" (PCA9685 Servo Driver)");
      } else {
        Serial.println(" (Unknown device)");
      }
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("ServoDriver: No I2C devices found!");
  } else {
    Serial.print("ServoDriver: Found ");
    Serial.print(deviceCount);
    Serial.println(" I2C device(s)");
  }
}

bool ServoDriver::begin() {
  // Scan for I2C devices first
  scanI2CDevices();
  
  pwm.begin();
  pwm.setPWMFreq(PWM_FREQ);
  delay(10);
  
  // Test if device is connected
  Wire.beginTransmission(I2C_ADDRESS);
  uint8_t error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("ServoDriver: PCA9685 initialized successfully");
    Serial.println("ServoDriver: 360° Servo Mode for pill dispensing");
    
    // Deactivate all servo outputs initially
    stopAllServos();
    return true;
  } else {
    Serial.println("ServoDriver: Failed to initialize PCA9685");
    return false;
  }
}

// ========================================
// 360-DEGREE SERVO CONTROL FOR PILL DISPENSING
// ========================================

void ServoDriver::setServoSpeed(uint8_t channel, int speed) {
  if (channel > 15) {
    Serial.println("ServoDriver: Invalid channel number");
    return;
  }
  pwm.setPWM(channel, 0, speed);
}

void ServoDriver::stopServo(uint8_t channel) {
  if (channel > 15) {
    Serial.println("ServoDriver: Invalid channel number");
    return;
  }
  // Fully deactivate servo output
  pwm.setPWM(channel, 0, 0);
}

void ServoDriver::stopAllServos() {
  Serial.println("ServoDriver: Stopping all servos");
  for (uint8_t i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, 0);
  }
}

void ServoDriver::operateServoTimed(uint8_t channel, int direction, int duration) {
  if (channel > 15) {
    Serial.println("ServoDriver: Invalid channel number");
    return;
  }
  
  Serial.print("ServoDriver: Operating channel ");
  Serial.print(channel);
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println("ms");
  
  // Start servo rotation
  setServoSpeed(channel, direction);
  
  // Wait for specified duration
  delay(duration);
  
  // Stop servo
  stopServo(channel);
  
  Serial.print("ServoDriver: Channel ");
  Serial.print(channel);
  Serial.println(" stopped");
}

int ServoDriver::getDurationForPillSize(String pillSize) {
  pillSize.toLowerCase();
  
  if (pillSize == "small") {
    return DISPENSE_DURATION_SMALL;
  } else if (pillSize == "medium") {
    return DISPENSE_DURATION_MEDIUM;
  } else if (pillSize == "large") {
    return DISPENSE_DURATION_LARGE;
  } else if (pillSize == "custom") {
    return DISPENSE_DURATION_CUSTOM;
  } else {
    Serial.println("ServoDriver: Unknown pill size, using medium");
    return DISPENSE_DURATION_MEDIUM;
  }
}

void ServoDriver::dispensePill(uint8_t channel, String pillSize) {
  if (channel > 15) {
    Serial.println("ServoDriver: Invalid channel number");
    return;
  }
  
  int duration = getDurationForPillSize(pillSize);
  
  Serial.print("ServoDriver: Dispensing ");
  Serial.print(pillSize);
  Serial.print(" pill on channel ");
  Serial.print(channel);
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println("ms");
  
  // Dispense pill with forward rotation
  operateServoTimed(channel, SERVO_FORWARD, duration);
  
  // Small delay between operations
  delay(100);
}

void ServoDriver::dispensePillPair(uint8_t channel1, uint8_t channel2, String pillSize) {
  if (channel1 > 15 || channel2 > 15) {
    Serial.println("ServoDriver: Invalid channel number(s)");
    return;
  }
  
  int duration = getDurationForPillSize(pillSize);
  
  Serial.print("ServoDriver: Dispensing ");
  Serial.print(pillSize);
  Serial.print(" pills using channels ");
  Serial.print(channel1);
  Serial.print(" & ");
  Serial.print(channel2);
  Serial.print(" for ");
  Serial.print(duration);
  Serial.println("ms");
  
  // Start both servos simultaneously
  setServoSpeed(channel1, SERVO_FORWARD);
  setServoSpeed(channel2, SERVO_FORWARD);
  
  // Wait for specified duration
  delay(duration);
  
  // Stop both servos
  stopServo(channel1);
  stopServo(channel2);
  
  Serial.print("ServoDriver: Both channels ");
  Serial.print(channel1);
  Serial.print(" & ");
  Serial.print(channel2);
  Serial.println(" stopped");
  
  // Buffer between operations
  delay(150);
}

// Custom dispense function for full rotation control
void ServoDriver::dispenseWithRotation(uint8_t servoNum, uint16_t startAngle, uint16_t stopAngle, uint16_t speed) {
  if (servoNum > 15) {
    Serial.println("ServoDriver: Invalid servo number");
    return;
  }
  
  // For MG90S servos, limit angles to mechanical range (typically 0-120°)
  if (startAngle > MG90S_MAX_ANGLE) {
    Serial.print("ServoDriver: Limiting start angle to MG90S max (");
    Serial.print(MG90S_MAX_ANGLE);
    Serial.println("°)");
    startAngle = MG90S_MAX_ANGLE;
  }
  if (stopAngle > MG90S_MAX_ANGLE) {
    Serial.print("ServoDriver: Limiting stop angle to MG90S max (");
    Serial.print(MG90S_MAX_ANGLE);
    Serial.println("°)");
    stopAngle = MG90S_MAX_ANGLE;
  }
  
  Serial.print("ServoDriver: Dispensing with rotation - Servo ");
  Serial.print(servoNum);
  Serial.print(" from ");
  Serial.print(startAngle);
  Serial.print("° to ");
  Serial.print(stopAngle);
  Serial.print("° at speed ");
  Serial.println(speed);
  
  // Determine rotation direction
  int step = (startAngle < stopAngle) ? 1 : -1;
  int steps = abs(stopAngle - startAngle);
  
  // Move servo step by step for smooth rotation
  for (int angle = startAngle; steps >= 0; angle += step, steps--) {
    setServoAngle(servoNum, angle);
    delay(speed);  // Speed controls delay between steps (lower = faster)
  }
  
  // Ensure final position is reached
  setServoAngle(servoNum, stopAngle);
  
  Serial.print("ServoDriver: Rotation complete for servo ");
  Serial.println(servoNum);
}

// Servo calibration for MG90S mechanical limits
void ServoDriver::calibrateServo(uint8_t servoNum) {
  if (servoNum > 15) {
    Serial.println("ServoDriver: Invalid servo number for calibration");
    return;
  }
  
  Serial.print("ServoDriver: Calibrating MG90S servo ");
  Serial.println(servoNum);
  Serial.println("ServoDriver: Testing mechanical limits...");
  
  // Test minimum position
  Serial.println("ServoDriver: Moving to minimum position (0°)...");
  setServoAngle(servoNum, 0);
  delay(1000);
  
  // Test maximum position (MG90S typically stops around 120-150°)
  Serial.println("ServoDriver: Moving to maximum position (120°)...");
  setServoAngle(servoNum, MG90S_MAX_ANGLE);
  delay(1000);
  
  // Test center position
  Serial.println("ServoDriver: Moving to center position (60°)...");
  setServoAngle(servoNum, MG90S_MAX_ANGLE / 2);
  delay(1000);
  
  Serial.print("ServoDriver: Calibration complete for servo ");
  Serial.println(servoNum);
  Serial.println("ServoDriver: Note - MG90S servos have mechanical stops around 120°");
}

// ========================================
// TRADITIONAL SERVO CONTROL (COMPATIBILITY)
// ========================================

void ServoDriver::setServoAngle(uint8_t servoNum, uint16_t angle) {
  if (servoNum > 15 || angle > 180) {
    Serial.println("ServoDriver: Invalid servo number or angle");
    return;
  }
  
  // Map angle (0-180) to pulse width (SERVO_MIN-SERVO_MAX)
  uint16_t pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(servoNum, 0, pulse);
  
  Serial.print("ServoDriver: Servo ");
  Serial.print(servoNum);
  Serial.print(" set to ");
  Serial.print(angle);
  Serial.println(" degrees");
}

void ServoDriver::setServoPulse(uint8_t servoNum, uint16_t pulse) {
  if (servoNum > 15) {
    Serial.println("ServoDriver: Invalid servo number");
    return;
  }
  
  pwm.setPWM(servoNum, 0, pulse);
}

bool ServoDriver::isConnected() {
  Wire.beginTransmission(I2C_ADDRESS);
  return (Wire.endTransmission() == 0);
}

void ServoDriver::resetAllServos() {
  Serial.println("ServoDriver: Resetting all servos to 90 degrees");
  for (uint8_t i = 0; i < 16; i++) {
    setServoAngle(i, 90);
    delay(50);
  }
}

// ========================================
// TESTING FUNCTIONS
// ========================================

void ServoDriver::testServo(uint8_t servoNum) {
  if (servoNum > 15) {
    Serial.println("ServoDriver: Invalid servo number for testing");
    return;
  }
  
  Serial.print("ServoDriver: Testing servo ");
  Serial.println(servoNum);
  
  // Test sequence: 0 -> 90 -> 180 -> 90
  setServoAngle(servoNum, 0);
  delay(1000);
  setServoAngle(servoNum, 90);
  delay(1000);
  setServoAngle(servoNum, 180);
  delay(1000);
  setServoAngle(servoNum, 90);
  delay(500);
  
  Serial.println("ServoDriver: Test complete");
}

void ServoDriver::testAllServos() {
  Serial.println("ServoDriver: Testing all servos sequentially");
  
  for (uint8_t i = 0; i < 16; i++) {
    Serial.print("Testing servo ");
    Serial.println(i);
    testServo(i);
    delay(500);
  }
  
  Serial.println("ServoDriver: All servo tests complete");
}

void ServoDriver::testPillDispenser(uint8_t channel) {
  if (channel > 15) {
    Serial.println("ServoDriver: Invalid channel for pill dispenser test");
    return;
  }
  
  Serial.print("ServoDriver: Testing pill dispenser on channel ");
  Serial.println(channel);
  
  // Test different pill sizes
  dispensePill(channel, "small");
  delay(1000);
  dispensePill(channel, "medium");
  delay(1000);
  dispensePill(channel, "large");
  delay(1000);
  
  Serial.println("ServoDriver: Pill dispenser test complete");
}

void ServoDriver::testAllDispenserPairs() {
  Serial.println("ServoDriver: Testing all dispenser pairs");
  
  // Test pairs (0&1, 2&3, 4&5, 6&7, etc.)
  for (uint8_t i = 0; i < 16; i += 2) {
    if (i + 1 < 16) {
      Serial.print("ServoDriver: Testing pair ");
      Serial.print(i);
      Serial.print(" & ");
      Serial.println(i + 1);
      
      dispensePillPair(i, i + 1, "medium");
      delay(2000);
    }
  }
  
  Serial.println("ServoDriver: All dispenser pair tests complete");
}

void ServoDriver::testDispenserRotation(uint8_t dispenserNum) {
  if (dispenserNum >= 5) {
    Serial.println("ServoDriver: Invalid dispenser number (0-4)");
    return;
  }
  
  // Define dispenser servo mappings (0-4 map to servos 0-4)
  uint8_t servoNum = dispenserNum;
  
  Serial.print("ServoDriver: Testing dispenser ");
  Serial.print(dispenserNum);
  Serial.print(" (servo ");
  Serial.print(servoNum);
  Serial.println(") - MG90S limits: 0-120°");
  
  // Test sequence: 0° -> 120° -> 0° with different speeds (MG90S compatible)
  Serial.println("ServoDriver: Test 1 - Full rotation 0° to 120° (fast)");
  dispenseWithRotation(servoNum, 0, MG90S_MAX_ANGLE, 10);  // Fast speed
  delay(1000);
  
  Serial.println("ServoDriver: Test 2 - Full rotation 120° to 0° (medium)");
  dispenseWithRotation(servoNum, MG90S_MAX_ANGLE, 0, 20);  // Medium speed
  delay(1000);
  
  Serial.println("ServoDriver: Test 3 - Partial rotation 30° to 90° (slow)");
  dispenseWithRotation(servoNum, 30, 90, 30); // Slow speed
  delay(1000);
  
  Serial.println("ServoDriver: Test 4 - Return to center position (60°)");
  dispenseWithRotation(servoNum, 90, MG90S_MAX_ANGLE / 2, 20); // Medium speed
  delay(500);
  
  Serial.print("ServoDriver: Dispenser ");
  Serial.print(dispenserNum);
  Serial.println(" rotation test complete");
}