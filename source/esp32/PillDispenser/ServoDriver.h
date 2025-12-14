#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PWMServoDriver.h>

class ServoDriver {
private:
  Adafruit_PWMServoDriver pwm;
  static const uint8_t PWM_FREQ = 50;    // Analog servos run at ~50 Hz
  static const uint8_t I2C_ADDRESS = 0x40; // Default PCA9685 address
  
  // I2C Retry and Bus Recovery Configuration (Optimized for responsiveness)
  static const uint8_t I2C_MAX_RETRIES = 2;        // Reduced from 5 to 2 for faster failure
  static const uint8_t I2C_RETRY_DELAY_MS = 5;     // Reduced from 10 to 5ms for responsiveness
  static const uint8_t I2C_BUS_RECOVERY_DELAY_MS = 20; // Reduced from 50 to 20ms
  
  // Error tracking
  uint32_t totalNackErrors;           // Total NACK errors encountered
  uint32_t totalBusRecoveries;        // Total bus recovery operations performed
  uint32_t totalI2COperations;        // Total I2C write operations attempted
  
  // Private helper functions for I2C operations with retry
  bool i2cWriteWithRetry(uint8_t reg, uint8_t value);
  bool i2cMultiWriteWithRetry(uint8_t* data, uint8_t length);
  bool safePWMWrite(uint8_t channel, uint16_t on, uint16_t off);
  bool performBusRecovery();
  void logNackError(const char* operation, uint8_t attempt);
  
  // 360-degree servo control constants for pill dispensing
  static const int SERVO_STOP = 375;      // PWM value for stopping (1.5ms pulse)
  static const int SERVO_FORWARD = 450;   // PWM value for forward rotation
  static const int SERVO_BACKWARD = 300;  // PWM value for backward rotation
  
  // Pill dispensing timing (in milliseconds)
  static const int DISPENSE_DURATION_SMALL = 800;   // Small pills
  static const int DISPENSE_DURATION_MEDIUM = 1000; // Medium pills  
  static const int DISPENSE_DURATION_LARGE = 1200;  // Large pills
  static const int DISPENSE_DURATION_CUSTOM = 1050; // Custom duration
  
  // Traditional servo constants (for MG90S TowerPro compatibility)
  // Servo range: 500-2500μs for full 0-180° travel
  // PCA9685: 50Hz, 12-bit (4096 steps), 1 tick = 4.88μs
  static const uint16_t SERVO_MIN = 102;  // 500μs (102 * 4.88μs ≈ 498μs)
  static const uint16_t SERVO_MAX = 512;  // 2500μs (512 * 4.88μs ≈ 2500μs)
  
  // MG90S mechanical limits (configurable - test with your servos)
  static const uint16_t MG90S_MIN_ANGLE = 0;
  static const uint16_t MG90S_MAX_ANGLE = 180;  // Default to spec - calibrate for your servos
  
public:
  ServoDriver();
  bool begin();
  void scanI2CDevices(); // I2C device scanner for debugging
  
  // 360-degree servo control (for pill dispensing)
  void setServoSpeed(uint8_t channel, int speed);
  void stopServo(uint8_t channel);
  void operateServoTimed(uint8_t channel, int direction, int duration);
  void dispensePill(uint8_t channel, String pillSize = "medium");
  void dispensePillPair(uint8_t channel1, uint8_t channel2, String pillSize = "medium");
  
  // Custom dispense function for full rotation control
  void dispenseWithRotation(uint8_t servoNum, uint16_t startAngle, uint16_t stopAngle, uint16_t speed);
  void calibrateServo(uint8_t servoNum);
  
  // Traditional servo control (for compatibility)
  void setServoAngle(uint8_t servoNum, uint16_t angle);
  void setServoPulse(uint8_t servoNum, uint16_t pulse);
  
  // Testing functions
  void testAllServos();
  void testServo(uint8_t servoNum);
  void testPillDispenser(uint8_t channel);
  void testAllDispenserPairs();
  void testDispenserRotation(uint8_t dispenserNum);
  
  // Utility functions
  bool isConnected();
  void resetAllServos();
  void stopAllServos();
  int getDurationForPillSize(String pillSize);
  
  // I2C Error Diagnostics
  void printI2CStatistics();          // Print error statistics
  void resetI2CStatistics();          // Reset error counters
  uint32_t getNackErrorCount();       // Get total NACK errors
  uint32_t getBusRecoveryCount();     // Get total bus recoveries
  float getI2CSuccessRate();          // Get success rate percentage
};

#endif