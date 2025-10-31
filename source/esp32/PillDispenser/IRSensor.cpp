#include "IRSensor.h"

IRSensor::IRSensor(uint8_t sensorPin1, uint8_t sensorPin2, uint8_t sensorPin3) {
  pin1 = sensorPin1;
  pin2 = sensorPin2;
  pin3 = sensorPin3;
  lastState1 = lastState2 = lastState3 = false;
  lastDebounceTime1 = lastDebounceTime2 = lastDebounceTime3 = 0;
}

void IRSensor::begin() {
  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
  
  Serial.println("IRSensor: 3 IR sensors initialized");
  Serial.print("IRSensor: Pins - Sensor1: ");
  Serial.print(pin1);
  Serial.print(", Sensor2: ");
  Serial.print(pin2);
  Serial.print(", Sensor3: ");
  Serial.println(pin3);
  
  // Initial reading
  readAllSensors();
}

bool IRSensor::isBlocked(uint8_t sensorNum) {
  switch (sensorNum) {
    case 1:
      return digitalRead(pin1) == LOW; // Assuming LOW means blocked
    case 2:
      return digitalRead(pin2) == LOW;
    case 3:
      return digitalRead(pin3) == LOW;
    default:
      Serial.println("IRSensor: Invalid sensor number");
      return false;
  }
}

bool IRSensor::hasStateChanged(uint8_t sensorNum) {
  bool currentState = isBlocked(sensorNum);
  bool hasChanged = false;
  unsigned long currentTime = millis();
  
  switch (sensorNum) {
    case 1:
      if (currentState != lastState1 && (currentTime - lastDebounceTime1) > DEBOUNCE_DELAY) {
        lastState1 = currentState;
        lastDebounceTime1 = currentTime;
        hasChanged = true;
        if (currentState) onSensor1Triggered();
      }
      break;
    case 2:
      if (currentState != lastState2 && (currentTime - lastDebounceTime2) > DEBOUNCE_DELAY) {
        lastState2 = currentState;
        lastDebounceTime2 = currentTime;
        hasChanged = true;
        if (currentState) onSensor2Triggered();
      }
      break;
    case 3:
      if (currentState != lastState3 && (currentTime - lastDebounceTime3) > DEBOUNCE_DELAY) {
        lastState3 = currentState;
        lastDebounceTime3 = currentTime;
        hasChanged = true;
        if (currentState) onSensor3Triggered();
      }
      break;
  }
  
  return hasChanged;
}

void IRSensor::readAllSensors() {
  for (uint8_t i = 1; i <= 3; i++) {
    hasStateChanged(i);
  }
}

void IRSensor::printSensorStates() {
  Serial.print("IRSensor States - S1: ");
  Serial.print(isBlocked(1) ? "BLOCKED" : "CLEAR");
  Serial.print(", S2: ");
  Serial.print(isBlocked(2) ? "BLOCKED" : "CLEAR");
  Serial.print(", S3: ");
  Serial.println(isBlocked(3) ? "BLOCKED" : "CLEAR");
}

void IRSensor::testSensor(uint8_t sensorNum) {
  if (sensorNum < 1 || sensorNum > 3) {
    Serial.println("IRSensor: Invalid sensor number for testing");
    return;
  }
  
  Serial.print("IRSensor: Testing sensor ");
  Serial.println(sensorNum);
  Serial.println("IRSensor: Block and unblock the sensor to see changes...");
  
  unsigned long startTime = millis();
  bool lastTestState = isBlocked(sensorNum);
  
  while (millis() - startTime < 10000) { // Test for 10 seconds
    bool currentTestState = isBlocked(sensorNum);
    if (currentTestState != lastTestState) {
      Serial.print("IRSensor ");
      Serial.print(sensorNum);
      Serial.print(": ");
      Serial.println(currentTestState ? "BLOCKED" : "CLEAR");
      lastTestState = currentTestState;
    }
    delay(100);
  }
  
  Serial.println("IRSensor: Test complete");
}

void IRSensor::testAllSensors() {
  Serial.println("IRSensor: Testing all sensors for 15 seconds...");
  Serial.println("IRSensor: Block and unblock sensors to see changes");
  
  unsigned long startTime = millis();
  
  while (millis() - startTime < 15000) { // Test for 15 seconds
    readAllSensors();
    delay(100);
  }
  
  Serial.println("IRSensor: All sensor tests complete");
}

void IRSensor::onSensor1Triggered() {
  Serial.println("IRSensor: Sensor 1 TRIGGERED (Pill dispenser entrance)");
}

void IRSensor::onSensor2Triggered() {
  Serial.println("IRSensor: Sensor 2 TRIGGERED (Pill dispenser middle)");
}

void IRSensor::onSensor3Triggered() {
  Serial.println("IRSensor: Sensor 3 TRIGGERED (Pill dispenser exit)");
}