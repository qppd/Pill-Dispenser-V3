#include "ArduinoServoController.h"

ArduinoServoController::ArduinoServoController(uint8_t rxPin, uint8_t txPin, unsigned long timeout) {
  this->rxPin = rxPin;
  this->txPin = txPin;
  this->responseTimeout = timeout;
  this->arduinoReady = false;
  this->serial = &Serial1; // Use UART1 on ESP32
}

bool ArduinoServoController::begin() {
  Serial.println("ArduinoServoController: Initializing serial communication...");
  
  // Initialize UART1 with specified pins
  serial->begin(115200, SERIAL_8N1, rxPin, txPin);
  delay(100);
  
  // Clear any garbage in buffer
  while (serial->available()) {
    serial->read();
  }
  
  // Wait for Arduino to send READY or INIT:OK signal
  Serial.println("ArduinoServoController: Waiting for Arduino...");
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < 5000) { // Wait up to 5 seconds
    if (serial->available()) {
      char c = serial->read();
      if (c == '\n') {
        response.trim();
        Serial.println("ArduinoServoController: Received: " + response);
        
        if (response == "READY" || response.startsWith("INIT:OK")) {
          arduinoReady = true;
          Serial.println("ArduinoServoController: Arduino is ready!");
          return true;
        }
        response = "";
      } else {
        response += c;
      }
    }
    delay(10);
  }
  
  // Try pinging if we didn't get READY signal
  if (ping()) {
    arduinoReady = true;
    Serial.println("ArduinoServoController: Arduino responded to PING");
    return true;
  }
  
  Serial.println("ArduinoServoController: Failed to connect to Arduino");
  return false;
}

bool ArduinoServoController::isConnected() {
  if (!arduinoReady) {
    return false;
  }
  
  // Periodically ping Arduino to verify connection
  static unsigned long lastPing = 0;
  if (millis() - lastPing > 30000) { // Ping every 30 seconds
    arduinoReady = ping();
    lastPing = millis();
  }
  
  return arduinoReady;
}

String ArduinoServoController::sendCommand(String command, unsigned long timeout) {
  // Clear receive buffer
  while (serial->available()) {
    serial->read();
  }
  
  // Send command
  serial->println(command);
  Serial.println("ArduinoServoController: Sent: " + command);
  
  // Wait for response
  unsigned long startTime = millis();
  String response = "";
  
  while (millis() - startTime < timeout) {
    if (serial->available()) {
      char c = serial->read();
      if (c == '\n') {
        response.trim();
        if (response.length() > 0 && !response.startsWith("HEARTBEAT")) {
          Serial.println("ArduinoServoController: Response: " + response);
          return response;
        }
        response = "";
      } else {
        response += c;
      }
    }
    delay(1);
  }
  
  Serial.println("ArduinoServoController: Timeout waiting for response");
  
  // Mark Arduino as not ready to trigger reconnection check
  arduinoReady = false;
  
  return "TIMEOUT";
}

bool ArduinoServoController::isSuccessResponse(String response) {
  return response.startsWith("OK:") || response == "PONG";
}

bool ArduinoServoController::ping() {
  String response = sendCommand("PING", 1000);
  return response == "PONG";
}

bool ArduinoServoController::checkStatus() {
  String response = sendCommand("STATUS", 1000);
  return response == "OK:READY";
}

bool ArduinoServoController::setServoAngle(uint8_t channel, uint16_t angle) {
  if (channel > 15 || angle > 180) {
    Serial.println("ArduinoServoController: Invalid channel or angle");
    return false;
  }
  
  String command = "SA" + String(channel) + "," + String(angle);
  String response = sendCommand(command, responseTimeout);
  return isSuccessResponse(response);
}

bool ArduinoServoController::dispensePill(uint8_t channel) {
  if (channel > 15) {
    Serial.println("ArduinoServoController: Invalid channel");
    return false;
  }
  
  // Check if Arduino is connected, attempt reconnection if not
  if (!arduinoReady) {
    Serial.println("ArduinoServoController: Arduino not ready, attempting reconnection...");
    if (!begin()) {
      Serial.println("ArduinoServoController: Reconnection failed");
      return false;
    }
  }
  
  String command = "DP" + String(channel);
  String response = sendCommand(command, responseTimeout + 3000); // Extra time for dispensing
  
  // If timeout, try one more time after reconnecting
  if (response == "TIMEOUT") {
    Serial.println("ArduinoServoController: Command timed out, attempting reconnection...");
    if (begin()) {
      Serial.println("ArduinoServoController: Reconnected, retrying command...");
      response = sendCommand(command, responseTimeout + 3000);
    }
  }
  
  return isSuccessResponse(response);
}

bool ArduinoServoController::dispensePillPair(uint8_t channel1, uint8_t channel2) {
  if (channel1 > 15 || channel2 > 15) {
    Serial.println("ArduinoServoController: Invalid channel(s)");
    return false;
  }
  
  String command = "DP2" + String(channel1) + "," + String(channel2);
  String response = sendCommand(command, responseTimeout + 3000); // Extra time for dispensing
  return isSuccessResponse(response);
}

bool ArduinoServoController::testServo(uint8_t channel) {
  if (channel > 15) {
    Serial.println("ArduinoServoController: Invalid channel");
    return false;
  }
  
  String command = "TS" + String(channel);
  String response = sendCommand(command, 5000); // Test takes ~3.5 seconds
  return isSuccessResponse(response);
}

bool ArduinoServoController::calibrateServo(uint8_t channel) {
  if (channel > 15) {
    Serial.println("ArduinoServoController: Invalid channel");
    return false;
  }
  
  String command = "CA" + String(channel);
  String response = sendCommand(command, 8000); // Calibration takes longer
  return isSuccessResponse(response);
}

bool ArduinoServoController::resetAllServos() {
  String response = sendCommand("RS", 5000);
  return isSuccessResponse(response);
}

bool ArduinoServoController::stopAllServos() {
  String response = sendCommand("ST", 2000);
  return isSuccessResponse(response);
}

bool ArduinoServoController::moveServosToRelease() {
  String response = sendCommand("RL", 3000);
  return isSuccessResponse(response);
}

bool ArduinoServoController::moveServosToHome() {
  String response = sendCommand("MH", 3000);
  return isSuccessResponse(response);
}

void ArduinoServoController::update() {
  // Read and log any async messages from Arduino (like heartbeat)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 100) {
    return; // Don't check too frequently
  }
  lastUpdate = millis();
  
  while (serial->available()) {
    String message = serial->readStringUntil('\n');
    message.trim();
    
    if (message.length() > 0) {
      if (message == "HEARTBEAT") {
        // Arduino is alive
        arduinoReady = true;
      } else {
        Serial.println("ArduinoServoController: Async message: " + message);
      }
    }
  }
}
