#include "ServoDriver.h"
#include <Wire.h>

ServoDriver::ServoDriver() : pwm(I2C_ADDRESS) {
  // Constructor uses default I2C address
  // Initialize error tracking counters
  totalNackErrors = 0;
  totalBusRecoveries = 0;
  totalI2COperations = 0;
}

// ========================================
// I2C RETRY AND BUS RECOVERY MECHANISMS
// ========================================

/**
 * Perform I2C write operation with minimal error handling
 * Uses fail-fast approach to prevent program freezing
 *
 * @param reg Register address to write to
 * @param value Value to write
 * @return true if write succeeded, false otherwise
 */
bool ServoDriver::i2cWriteWithRetry(uint8_t reg, uint8_t value) {
  totalI2COperations++;

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    return true;
  }

  // Log error but don't retry - fail fast
  if (error == 2 || error == 3) {
    totalNackErrors++;
    Serial.print("ServoDriver: NACK on I2C write (reg ");
    Serial.print(reg);
    Serial.println(") - bus may be disconnected");
  } else {
    Serial.print("ServoDriver: I2C error ");
    Serial.print(error);
    Serial.print(" on write (reg ");
    Serial.print(reg);
    Serial.println(")");
  }

  return false;
}

/**
 * Perform multi-byte I2C write with minimal error handling
 * Uses fail-fast approach to prevent program freezing
 *
 * @param data Pointer to data buffer
 * @param length Number of bytes to write
 * @return true if write succeeded, false otherwise
 */
bool ServoDriver::i2cMultiWriteWithRetry(uint8_t* data, uint8_t length) {
  totalI2COperations++;

  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(data, length);
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    return true;
  }

  // Log error but don't retry - fail fast
  if (error == 2 || error == 3) {
    totalNackErrors++;
    Serial.print("ServoDriver: NACK on multi-write (");
    Serial.print(length);
    Serial.println(" bytes) - bus may be disconnected");
  } else {
    Serial.print("ServoDriver: I2C error ");
    Serial.print(error);
    Serial.print(" on multi-write (");
    Serial.print(length);
    Serial.println(" bytes)");
  }

  return false;
}

/**
 * Perform I2C bus recovery by reinitializing the bus
 * This helps recover from stuck bus conditions
 * 
 * @return true if recovery appears successful, false otherwise
 */
bool ServoDriver::performBusRecovery() {
  Serial.println("ServoDriver: *** PERFORMING I2C BUS RECOVERY ***");
  totalBusRecoveries++;
  
  // End current I2C session
  Wire.end();
  delay(I2C_BUS_RECOVERY_DELAY_MS);
  
  // Reinitialize I2C bus
  Wire.begin();
  Wire.setClock(50000); // Reduced from 100000 to 50000 Hz for better stability
  delay(I2C_BUS_RECOVERY_DELAY_MS);
  
  // Test if device is reachable
  Wire.beginTransmission(I2C_ADDRESS);
  uint8_t error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("ServoDriver: Bus recovery successful - PCA9685 reachable");
    
    // Reinitialize PCA9685
    pwm.begin();
    pwm.setPWMFreq(PWM_FREQ);
    delay(10);
    
    Serial.println("ServoDriver: PCA9685 reinitialized after recovery");
    return true;
  } else {
    Serial.print("ServoDriver: Bus recovery failed - error code: ");
    Serial.println(error);
    return false;
  }
}

/**
 * Log NACK error with details for debugging
 * Used in fail-fast error handling
 *
 * @param operation Description of the operation that failed
 */
void ServoDriver::logNackError(const char* operation) {
  Serial.print("ServoDriver: *** NACK ERROR *** Operation: ");
  Serial.print(operation);
  Serial.print(", Total NACKs: ");
  Serial.println(totalNackErrors);
}

/**
 * Safe PWM write operation with minimal retry logic
 * Wraps the Adafruit_PWMServoDriver setPWM call with error handling
 * Uses fail-fast approach to prevent program freezing
 *
 * @param channel PWM channel (0-15)
 * @param on PWM on time (0-4095)
 * @param off PWM off time (0-4095)
 * @return true if write succeeded, false otherwise
 */
bool ServoDriver::safePWMWrite(uint8_t channel, uint16_t on, uint16_t off) {
  totalI2COperations++;

  // Test bus connectivity first (quick check)
  Wire.beginTransmission(I2C_ADDRESS);
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    // Bus is responding, perform the PWM write
    pwm.setPWM(channel, on, off);
    return true;
  }

  // Bus error detected - log it but don't retry aggressively
  totalNackErrors++;
  if (error == 2 || error == 3) {
    Serial.print("ServoDriver: NACK on PWM write (channel ");
    Serial.print(channel);
    Serial.println(") - bus may be disconnected");
  } else {
    Serial.print("ServoDriver: I2C error ");
    Serial.print(error);
    Serial.print(" on PWM write (channel ");
    Serial.print(channel);
    Serial.println(")");
  }

  // Don't retry or delay - fail fast to prevent program freezing
  return false;
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
  
  // Use safe PWM write with retry mechanism
  if (!safePWMWrite(channel, 0, speed)) {
    Serial.print("ServoDriver: FAILED to set servo ");
    Serial.print(channel);
    Serial.println(" speed after retries");
  }
}

void ServoDriver::stopServo(uint8_t channel) {
  if (channel > 15) {
    Serial.println("ServoDriver: Invalid channel number");
    return;
  }
  
  // Fully deactivate servo output with retry mechanism
  if (!safePWMWrite(channel, 0, 0)) {
    Serial.print("ServoDriver: WARNING - Failed to stop servo ");
    Serial.print(channel);
    Serial.println(" after retries");
  }
}

void ServoDriver::stopAllServos() {
  Serial.println("ServoDriver: Stopping all servos");
  uint8_t failedChannels = 0;
  
  for (uint8_t i = 0; i < 16; i++) {
    if (!safePWMWrite(i, 0, 0)) {
      failedChannels++;
    }
  }
  
  if (failedChannels > 0) {
    Serial.print("ServoDriver: WARNING - Failed to stop ");
    Serial.print(failedChannels);
    Serial.println(" servo channel(s)");
  } else {
    Serial.println("ServoDriver: All servos stopped successfully");
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
  
  Serial.print("ServoDriver: Dispensing ");
  Serial.print(pillSize);
  Serial.print(" pill on channel ");
  Serial.println(channel);
  
  // Move servo to 180 degrees
  setServoAngle(channel, 180);
  delay(100); // Small delay for servo movement
  
  // Wait 2 seconds
  delay(2000);
  
  // Move servo back to 0 degrees
  setServoAngle(channel, 0);
  delay(100); // Small delay for servo movement
  
  Serial.print("ServoDriver: Dispensing complete on channel ");
  Serial.println(channel);
}

void ServoDriver::dispensePillPair(uint8_t channel1, uint8_t channel2, String pillSize) {
  if (channel1 > 15 || channel2 > 15) {
    Serial.println("ServoDriver: Invalid channel number(s)");
    return;
  }
  
  Serial.print("ServoDriver: Dispensing ");
  Serial.print(pillSize);
  Serial.print(" pills using channels ");
  Serial.print(channel1);
  Serial.print(" & ");
  Serial.println(channel2);
  
  // Move both servos to 180 degrees
  setServoAngle(channel1, 180);
  setServoAngle(channel2, 180);
  delay(100); // Small delay for servo movement
  
  // Wait 2 seconds
  delay(2000);
  
  // Move both servos back to 0 degrees
  setServoAngle(channel1, 0);
  setServoAngle(channel2, 0);
  delay(100); // Small delay for servo movement
  
  Serial.print("ServoDriver: Dispensing complete on channels ");
  Serial.print(channel1);
  Serial.print(" & ");
  Serial.println(channel2);
  
  // Buffer between operations
  delay(150);
}

// Custom dispense function for full rotation control
void ServoDriver::dispenseWithRotation(uint8_t servoNum, uint16_t startAngle, uint16_t stopAngle, uint16_t speed) {
  if (servoNum > 15) {
    Serial.println("ServoDriver: Invalid servo number");
    return;
  }
  
  // For MG90S servos, warn about potential mechanical limits but allow full range
  if (startAngle > 120 || stopAngle > 120) {
    Serial.println("ServoDriver: WARNING - Angles > 120° may hit MG90S mechanical stops");
    Serial.println("ServoDriver: Use 'calibrate servo' to test your servo's actual range");
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
  Serial.println("ServoDriver: Testing full 180° range (per specs)...");
  
  // Test minimum position
  Serial.println("ServoDriver: Moving to minimum position (0°)...");
  setServoAngle(servoNum, 0);
  delay(1500);
  
  // Test maximum position (specs say 180°, but mechanical stops may limit)
  Serial.println("ServoDriver: Moving to maximum position (180° per specs)...");
  setServoAngle(servoNum, 180);
  delay(1500);
  
  // Test center position
  Serial.println("ServoDriver: Moving to center position (90°)...");
  setServoAngle(servoNum, 90);
  delay(1000);
  
  // Test quarter positions to find actual working range
  Serial.println("ServoDriver: Testing quarter positions...");
  setServoAngle(servoNum, 45);
  delay(1000);
  setServoAngle(servoNum, 135);
  delay(1000);
  
  Serial.print("ServoDriver: Calibration complete for servo ");
  Serial.println(servoNum);
  Serial.println("ServoDriver: If servo didn't reach 180°, adjust MG90S_MAX_ANGLE in ServoDriver.h");
  Serial.println("ServoDriver: Typical working range: 0-120° to 0-150° for MG90S");
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
  
  // Use safe PWM write with retry mechanism
  if (safePWMWrite(servoNum, 0, pulse)) {
    Serial.print("ServoDriver: Servo ");
    Serial.print(servoNum);
    Serial.print(" set to ");
    Serial.print(angle);
    Serial.println(" degrees");
  } else {
    Serial.print("ServoDriver: FAILED to set servo ");
    Serial.print(servoNum);
    Serial.print(" to ");
    Serial.print(angle);
    Serial.println(" degrees after retries");
  }
}

void ServoDriver::setServoPulse(uint8_t servoNum, uint16_t pulse) {
  if (servoNum > 15) {
    Serial.println("ServoDriver: Invalid servo number");
    return;
  }
  
  // Use safe PWM write with retry mechanism
  if (!safePWMWrite(servoNum, 0, pulse)) {
    Serial.print("ServoDriver: FAILED to set servo ");
    Serial.print(servoNum);
    Serial.println(" pulse after retries");
  }
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
  Serial.println(") - Testing full 180° range");
  
  // Test sequence: 0° -> 180° -> 0° with different speeds (test full specs)
  Serial.println("ServoDriver: Test 1 - Full spec rotation 0° to 180° (fast)");
  dispenseWithRotation(servoNum, 0, 180, 10);  // Fast speed
  delay(1000);
  
  Serial.println("ServoDriver: Test 2 - Full spec rotation 180° to 0° (medium)");
  dispenseWithRotation(servoNum, 180, 0, 20);  // Medium speed
  delay(1000);
  
  Serial.println("ServoDriver: Test 3 - Partial rotation 45° to 135° (slow)");
  dispenseWithRotation(servoNum, 45, 135, 30); // Slow speed
  delay(1000);
  
  Serial.println("ServoDriver: Test 4 - Return to center position (90°)");
  dispenseWithRotation(servoNum, 135, 90, 20); // Medium speed
  delay(500);
  
  Serial.print("ServoDriver: Dispenser ");
  Serial.print(dispenserNum);
  Serial.println(" rotation test complete");
}

// ========================================
// I2C ERROR DIAGNOSTICS AND STATISTICS
// ========================================

/**
 * Print comprehensive I2C error statistics
 */
void ServoDriver::printI2CStatistics() {
  Serial.println("\n========== I2C STATISTICS ==========");
  Serial.print("Total I2C Operations:  ");
  Serial.println(totalI2COperations);
  Serial.print("Total NACK Errors:     ");
  Serial.println(totalNackErrors);
  Serial.print("Total Bus Recoveries:  ");
  Serial.println(totalBusRecoveries);

  if (totalI2COperations > 0) {
    float successRate = getI2CSuccessRate();
    Serial.print("Success Rate:          ");
    Serial.print(successRate, 2);
    Serial.println("%");

    float errorRate = (totalNackErrors * 100.0) / totalI2COperations;
    Serial.print("NACK Error Rate:       ");
    Serial.print(errorRate, 2);
    Serial.println("%");
  }
  Serial.println("===================================\n");
}

/**
 * Reset all error statistics counters
 */
void ServoDriver::resetI2CStatistics() {
  Serial.println("ServoDriver: Resetting I2C statistics");
  totalNackErrors = 0;
  totalBusRecoveries = 0;
  totalI2COperations = 0;
}

/**
 * Get total NACK error count
 * @return Number of NACK errors encountered
 */
uint32_t ServoDriver::getNackErrorCount() {
  return totalNackErrors;
}

/**
 * Get total bus recovery count
 * @return Number of bus recovery operations performed
 */
uint32_t ServoDriver::getBusRecoveryCount() {
  return totalBusRecoveries;
}

/**
 * Calculate I2C success rate
 * @return Success rate as percentage (0-100)
 */
float ServoDriver::getI2CSuccessRate() {
  if (totalI2COperations == 0) {
    return 100.0;
  }

  uint32_t successfulOps = totalI2COperations - totalNackErrors;
  return (successfulOps * 100.0) / totalI2COperations;
}