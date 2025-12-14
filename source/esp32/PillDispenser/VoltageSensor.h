#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

#include <Arduino.h>

class VoltageSensor {
private:
  uint8_t analogPin;
  static const uint16_t ADC_RESOLUTION = 4095;
  static constexpr float BATTERY_MIN_VOLTAGE = 6.0;
  static constexpr float BATTERY_MAX_VOLTAGE = 8.4;
  float lastBatteryPercentage;
  unsigned long lastBatteryUpdate;
  
public:
  VoltageSensor(uint8_t pin) : analogPin(pin), lastBatteryPercentage(0), lastBatteryUpdate(0) {}
  
  void begin() {
    pinMode(analogPin, INPUT);
    Serial.println("VoltageSensor: Initialized on pin " + String(analogPin));
  }
  
  float readActualVoltage() {
    uint16_t adcValue = analogRead(analogPin);
    float voltage = (adcValue / (float)ADC_RESOLUTION) * 3.3 * 5.0;
    return voltage;
  }
  
  float calculateBatteryPercentage(float voltage) {
    if (voltage >= BATTERY_MAX_VOLTAGE) return 100.0;
    if (voltage <= BATTERY_MIN_VOLTAGE) return 0.0;
    return ((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0;
  }
  
  float readBatteryPercentage() {
    float voltage = readActualVoltage();
    lastBatteryPercentage = calculateBatteryPercentage(voltage);
    lastBatteryUpdate = millis();
    return lastBatteryPercentage;
  }
  
  float getLastBatteryPercentage() { return lastBatteryPercentage; }
};

#endif
