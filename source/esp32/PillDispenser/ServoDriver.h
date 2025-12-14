#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

class ServoDriver {
private:
  Adafruit_PWMServoDriver pwm;
  static const uint8_t PWM_FREQ = 50;
  static const uint8_t I2C_ADDRESS = 0x40;
  static const uint16_t SERVO_MIN = 102;   // 500μs
  static const uint16_t SERVO_MAX = 512;   // 2500μs
  
public:
  ServoDriver() : pwm(I2C_ADDRESS) {}
  
  bool begin() {
    Wire.setClock(50000);
    pwm.begin();
    delay(5);
    pwm.setPWMFreq(PWM_FREQ);
    delay(10);
    
    Wire.beginTransmission(I2C_ADDRESS);
    uint8_t error = Wire.endTransmission();
    delay(2);
    
    if (error == 0) {
      Serial.println("ServoDriver: PCA9685 initialized");
      stopAllServos();
      return true;
    }
    Serial.println("ServoDriver: Failed to initialize PCA9685");
    return false;
  }
  
  void setServoAngle(uint8_t servoNum, uint16_t angle) {
    if (angle > 180) angle = 180;
    uint16_t pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(servoNum, 0, pulse);
  }
  
  void stopServo(uint8_t channel) {
    pwm.setPWM(channel, 0, 0);
  }
  
  void stopAllServos() {
    for (uint8_t i = 0; i < 16; i++) {
      pwm.setPWM(i, 0, 0);
    }
  }
};

#endif