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
  
  // Battery calibration (18650 Li-ion 2S configuration: 6.0V - 8.4V)
  static constexpr float BATTERY_MIN_VOLTAGE = 6.0;   // 3.0V per cell x 2 = 6.0V (empty)
  static constexpr float BATTERY_MAX_VOLTAGE = 8.4;  // 4.2V per cell x 2 = 8.4V (full)
  
  float lastRawVoltage;
  float lastActualVoltage;
  float lastBatteryPercentage;
  uint16_t lastAdcValue;
  unsigned long lastBatteryUpdate;
  static const unsigned long BATTERY_UPDATE_INTERVAL = 60000;  // 1 minute
  
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
  
  // Battery percentage functions
  float calculateBatteryPercentage(float voltage);
  float readBatteryPercentage();
  float getLastBatteryPercentage();
  bool shouldUpdateBattery();
  String getBatteryStatus();  // Returns: "Full", "Good", "Low", "Critical"
  
  // Advanced features
  float readAveragedVoltage(uint8_t samples = SAMPLE_COUNT);
  bool isVoltageLow(float threshold = 6.5);  // Updated for 2S battery
  bool isVoltageHigh(float threshold = 8.4); // Updated for 2S battery
  bool isBatteryLow(float threshold = 20.0); // Battery % threshold
};

#endif
