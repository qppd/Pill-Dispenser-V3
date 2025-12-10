#include "VoltageSensor.h"

// Initialize static constants
const float VoltageSensor::ADC_VOLTAGE_STEP = 0.00080566;   // 3.3V / 4095 ≈ 0.00080566V (ESP32 12-bit ADC with 11dB attenuation)
const float VoltageSensor::VOLTAGE_DIVIDER_RATIO = 5.0;    // 25V / 5V = 5.0

VoltageSensor::VoltageSensor(uint8_t pin) {
  analogPin = pin;
  lastRawVoltage = 0.0;
  lastActualVoltage = 0.0;
  lastBatteryPercentage = 0.0;
  lastAdcValue = 0;
  lastBatteryUpdate = 0;
}

void VoltageSensor::begin() {
  pinMode(analogPin, INPUT);
  
  // Configure ADC attenuation for full 0-3.3V range
  analogSetAttenuation(ADC_11db);  // 0-3.3V range
  
  Serial.println("VoltageSensor: Module initialized");
  Serial.print("VoltageSensor: Pin: GPIO ");
  Serial.println(analogPin);
  Serial.print("VoltageSensor: ADC Resolution: ");
  Serial.println(ADC_RESOLUTION);
  Serial.print("VoltageSensor: Voltage Step: ");
  Serial.print(ADC_VOLTAGE_STEP, 8);
  Serial.println(" V");
  Serial.print("VoltageSensor: Input Range: 0-25V (via voltage divider)");
  Serial.println();
  
  // Initial reading
  readActualVoltage();
  Serial.print("VoltageSensor: Initial reading: ");
  Serial.print(lastActualVoltage, 2);
  Serial.println(" V");
}

uint16_t VoltageSensor::readADC() {
  lastAdcValue = analogRead(analogPin);
  return lastAdcValue;
}

float VoltageSensor::readRawVoltage() {
  lastAdcValue = analogRead(analogPin);
  lastRawVoltage = lastAdcValue * ADC_VOLTAGE_STEP;
  return lastRawVoltage;
}

float VoltageSensor::readActualVoltage() {
  readRawVoltage();
  lastActualVoltage = lastRawVoltage * VOLTAGE_DIVIDER_RATIO;
  return lastActualVoltage;
}

float VoltageSensor::getLastRawVoltage() {
  return lastRawVoltage;
}

float VoltageSensor::getLastActualVoltage() {
  return lastActualVoltage;
}

uint16_t VoltageSensor::getLastADC() {
  return lastAdcValue;
}

float VoltageSensor::readAveragedVoltage(uint8_t samples) {
  if (samples == 0) samples = 1;
  
  float sum = 0.0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += readActualVoltage();
    delay(10);
  }
  
  float averaged = sum / samples;
  lastActualVoltage = averaged;
  
  return averaged;
}

bool VoltageSensor::isVoltageLow(float threshold) {
  return lastActualVoltage < threshold;
}

bool VoltageSensor::isVoltageHigh(float threshold) {
  return lastActualVoltage > threshold;
}

bool VoltageSensor::isConnected() {
  // Sensor is always "connected" if pin is readable
  // Check if we get a valid ADC reading
  uint16_t testValue = analogRead(analogPin);
  return (testValue <= ADC_RESOLUTION);
}

float VoltageSensor::calculateBatteryPercentage(float voltage) {
  // Linear interpolation between min and max voltage
  if (voltage <= BATTERY_MIN_VOLTAGE) {
    return 0.0;
  }
  if (voltage >= BATTERY_MAX_VOLTAGE) {
    return 100.0;
  }
  
  // Calculate percentage
  float percentage = ((voltage - BATTERY_MIN_VOLTAGE) / 
                     (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100.0;
  
  // Ensure within 0-100 range
  percentage = constrain(percentage, 0.0, 100.0);
  
  return percentage;
}

float VoltageSensor::readBatteryPercentage() {
  float voltage = readAveragedVoltage(5); // Average of 5 samples
  lastBatteryPercentage = calculateBatteryPercentage(voltage);
  lastBatteryUpdate = millis();
  return lastBatteryPercentage;
}

float VoltageSensor::getLastBatteryPercentage() {
  return lastBatteryPercentage;
}

bool VoltageSensor::shouldUpdateBattery() {
  return (millis() - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL);
}

String VoltageSensor::getBatteryStatus() {
  if (lastBatteryPercentage >= 80) {
    return "Full";
  } else if (lastBatteryPercentage >= 50) {
    return "Good";
  } else if (lastBatteryPercentage >= 20) {
    return "Low";
  } else {
    return "Critical";
  }
}

bool VoltageSensor::isBatteryLow(float threshold) {
  return lastBatteryPercentage < threshold;
}

void VoltageSensor::printDebug() {
  readActualVoltage();
  readBatteryPercentage();
  
  Serial.println("─────────────────────────────────────");
  Serial.println("[VOLTAGE SENSOR DEBUG]");
  Serial.print("ADC Value:       ");
  Serial.println(lastAdcValue);
  Serial.print("Raw Voltage:     ");
  Serial.print(lastRawVoltage, 2);
  Serial.println(" V");
  Serial.print("Actual Voltage:  ");
  Serial.print(lastActualVoltage, 2);
  Serial.println(" V");
  Serial.print("Battery %:       ");
  Serial.print(lastBatteryPercentage, 1);
  Serial.println(" %");
  Serial.print("Battery Status:  ");
  Serial.println(getBatteryStatus());
  Serial.println("─────────────────────────────────────");
}

void VoltageSensor::testSensor() {
  Serial.println("VoltageSensor: Starting sensor test");
  Serial.println("VoltageSensor: Monitoring voltage continuously...");
  Serial.println("VoltageSensor: Type 'exit' to stop test");
  Serial.println();
  
  unsigned long lastSample = 0;
  bool testRunning = true;
  
  while (testRunning) {
    // Check for exit command
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      input.toLowerCase();
      
      if (input == "exit") {
        Serial.println();
        Serial.println("VoltageSensor: Test stopped by user");
        testRunning = false;
        break;
      }
    }
    
    // Sample every 1 second
    if (millis() - lastSample >= 1000) {
      lastSample = millis();
      
      // Read voltage
      readActualVoltage();
      
      // Print formatted output
      Serial.println("─────────────────────────────────────");
      Serial.println("[VOLTAGE SENSOR]");
      Serial.print("Raw Voltage:     ");
      Serial.print(lastRawVoltage, 2);
      Serial.println(" V");
      Serial.print("Actual Voltage:  ");
      Serial.print(lastActualVoltage, 2);
      Serial.println(" V");
      Serial.print("ADC Value:       ");
      Serial.println(lastAdcValue);
      
      // Voltage level indicators
      if (lastActualVoltage < 3.0) {
        Serial.println("Status:          ⚠️  LOW VOLTAGE");
      } else if (lastActualVoltage > 24.0) {
        Serial.println("Status:          ⚠️  HIGH VOLTAGE");
      } else {
        Serial.println("Status:          ✅ NORMAL");
      }
      
      Serial.println("─────────────────────────────────────");
      Serial.println();
    }
    
    delay(100);
  }
  
  Serial.println("VoltageSensor: Test complete");
}
