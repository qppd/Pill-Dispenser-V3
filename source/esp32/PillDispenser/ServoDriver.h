#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

class ServoDriver {
private:
  Adafruit_PWMServoDriver pwm;
  static const uint8_t PWM_FREQ = 50;    // Analog servos run at ~50 Hz
  static const uint8_t I2C_ADDRESS = 0x40; // Default PCA9685 address
  
  // 360-degree servo control constants for pill dispensing
  static const int SERVO_STOP = 375;      // PWM value for stopping (1.5ms pulse)
  static const int SERVO_FORWARD = 450;   // PWM value for forward rotation
  static const int SERVO_BACKWARD = 300;  // PWM value for backward rotation
  
  // Pill dispensing timing (in milliseconds)
  static const int DISPENSE_DURATION_SMALL = 800;   // Small pills
  static const int DISPENSE_DURATION_MEDIUM = 1000; // Medium pills  
  static const int DISPENSE_DURATION_LARGE = 1200;  // Large pills
  static const int DISPENSE_DURATION_CUSTOM = 1050; // Custom duration
  
  // Traditional servo constants (for compatibility)
  static const uint16_t SERVO_MIN = 150;  // Min pulse length out of 4096
  static const uint16_t SERVO_MAX = 600;  // Max pulse length out of 4096
  
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
  
  // Traditional servo control (for compatibility)
  void setServoAngle(uint8_t servoNum, uint16_t angle);
  void setServoPulse(uint8_t servoNum, uint16_t pulse);
  
  // Testing functions
  void testAllServos();
  void testServo(uint8_t servoNum);
  void testPillDispenser(uint8_t channel);
  void testAllDispenserPairs();
  
  // Utility functions
  bool isConnected();
  void resetAllServos();
  void stopAllServos();
  int getDurationForPillSize(String pillSize);
};

#endif