#ifndef SIM800L_H
#define SIM800L_H

#include <Arduino.h>
#include <HardwareSerial.h>

class SIM800L {
private:
  HardwareSerial* sim800;
  uint8_t rxPin, txPin, rstPin;
  bool isModuleReady;
  String response;
  unsigned long lastCommand;
  static const unsigned long COMMAND_DELAY = 1000;
  
  void waitForResponse(unsigned long timeout) {
    response = "";
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
      if (sim800->available()) {
        response += (char)sim800->read();
        if (response.indexOf("OK") >= 0 || response.indexOf("ERROR") >= 0) break;
      }
      delay(10);
    }
  }
  
  void clearBuffer() {
    while (sim800->available()) sim800->read();
    response = "";
  }
  
public:
  SIM800L(uint8_t rxPin, uint8_t txPin, uint8_t rstPin, HardwareSerial& serialPort = Serial2)
    : sim800(&serialPort), rxPin(rxPin), txPin(txPin), rstPin(rstPin), 
      isModuleReady(false), lastCommand(0) {}
  
  bool begin(long baudRate = 9600) {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    sim800->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    delay(1000);
    Serial.println("SIM800L: Initializing...");
    reset();
    delay(3000);
    
    if (sendATCommand("AT", "OK", 3000)) {
      Serial.println("SIM800L: Module responding");
      sendATCommand("ATE0", "OK", 3000);
      if (sendATCommand("AT+CPIN?", "READY", 5000)) {
        Serial.println("SIM800L: SIM card ready");
        isModuleReady = true;
        return true;
      }
      Serial.println("SIM800L: SIM card not ready");
      return false;
    }
    Serial.println("SIM800L: Module not responding");
    return false;
  }
  
  void reset() {
    Serial.println("SIM800L: Resetting...");
    digitalWrite(rstPin, LOW);
    delay(200);
    digitalWrite(rstPin, HIGH);
    delay(3000);
  }
  
  bool isReady() { return isModuleReady && sendATCommand("AT", "OK", 1000); }
  
  bool sendATCommand(String command, String expectedResponse = "OK", unsigned long timeout = 5000) {
    while (millis() - lastCommand < COMMAND_DELAY) delay(10);
    clearBuffer();
    Serial.println("SIM800L: " + command);
    sim800->println(command);
    lastCommand = millis();
    waitForResponse(timeout);
    return response.indexOf(expectedResponse) >= 0;
  }
  
  bool sendSMS(String phoneNumber, String message) {
    if (!isReady()) {
      Serial.println("SIM800L: Not ready for SMS");
      return false;
    }
    
    Serial.println("SIM800L: Sending SMS to " + phoneNumber);
    if (!sendATCommand("AT+CMGF=1", "OK", 3000)) return false;
    
    sim800->println("AT+CMGS=\"" + phoneNumber + "\"");
    delay(1000);
    sim800->print(message);
    delay(500);
    sim800->write(26);
    waitForResponse(10000);
    
    if (response.indexOf("+CMGS:") >= 0) {
      Serial.println("✅ SMS sent");
      return true;
    }
    Serial.println("❌ SMS failed");
    return false;
  }
};

#endif