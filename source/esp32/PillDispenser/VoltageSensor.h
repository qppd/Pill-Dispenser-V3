#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

#include <Arduino.h>

class VoltageSensor {
private:
  uint8_t analogPin;
  static const uint16_t ADC_RESOLUTION = 4095;  // 12-bit ADC
  static const float ADC_VOLTAGE_STEP;          // 0.00489V per increment
  static const float VOLTAGE_DIVIDER_RATIO;     // 25.0 / 5.0 = 5.0
  static const uint8_t SAMPLE_COUNT = 10;       // Number of samples for averaging
  
  float lastRawVoltage;
  float lastActualVoltage;
  uint16_t lastAdcValue;
  
public:
  VoltageSensor(uint8_t pin);
  void begin();
  float readRawVoltage();
  float readActualVoltage();
  uint16_t readADC();
  float getLastRawVoltage();
  float getLastActualVoltage();
  uint16_t getLastADC();
  void printDebug();
  void testSensor();
  bool isConnected();
  
  // Advanced features
  float readAveragedVoltage(uint8_t samples = SAMPLE_COUNT);
  bool isVoltageLow(float threshold = 3.0);
  bool isVoltageHigh(float threshold = 24.0);
};

#endif
