#ifndef IR_SENSOR_H
#define IR_SENSOR_H

#include <Arduino.h>

class IRSensor {
private:
  uint8_t pin1, pin2, pin3;
  bool lastState1, lastState2, lastState3;
  unsigned long lastDebounceTime1, lastDebounceTime2, lastDebounceTime3;
  static const unsigned long DEBOUNCE_DELAY = 50;
  
public:
  IRSensor(uint8_t sensorPin1, uint8_t sensorPin2, uint8_t sensorPin3);
  void begin();
  bool isBlocked(uint8_t sensorNum);
  bool hasStateChanged(uint8_t sensorNum);
  void readAllSensors();
  void testSensor(uint8_t sensorNum);
  void testAllSensors();
  void printSensorStates();
  
  // Callbacks for when sensors are triggered
  void onSensor1Triggered();
  void onSensor2Triggered();
  void onSensor3Triggered();
};

#endif