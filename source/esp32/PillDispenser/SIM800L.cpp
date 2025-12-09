
#include "SIM800L.h"

SIM800L::SIM800L(uint8_t rxPin, uint8_t txPin, uint8_t rstPin, HardwareSerial& serialPort)
  : sim800(&serialPort), rxPin(rxPin), txPin(txPin), rstPin(rstPin) {
  isModuleReady = false;
  lastCommand = 0;
}

bool SIM800L::begin(long baudRate) {
  pinMode(rstPin, OUTPUT);
  digitalWrite(rstPin, HIGH);

  sim800->begin(baudRate, SERIAL_8N1, rxPin, txPin);
  delay(1000);

  Serial.println("SIM800L: Initializing module...");

  // Reset module
  reset();
  delay(3000);

  // Test basic communication
  if (sendATCommand("AT", "OK", 3000)) {
    Serial.println("SIM800L: Module responding to AT commands");

    // Disable echo
    sendATCommand("ATE0", "OK", 3000);

    // Check SIM card
    if (sendATCommand("AT+CPIN?", "READY", 5000)) {
      Serial.println("SIM800L: SIM card is ready");
      isModuleReady = true;
      return true;
    } else {
      Serial.println("SIM800L: SIM card not ready or missing");
      return false;
    }
  } else {
    Serial.println("SIM800L: Module not responding");
    return false;
  }
}

void SIM800L::reset() {
  Serial.println("SIM800L: Resetting module...");
  digitalWrite(rstPin, LOW);
  delay(200);
  digitalWrite(rstPin, HIGH);
  delay(3000);
}

bool SIM800L::isReady() {
  return isModuleReady && sendATCommand("AT", "OK", 1000);
}

bool SIM800L::sendATCommand(String command, String expectedResponse, unsigned long timeout) {
  // Ensure minimum delay between commands
  while (millis() - lastCommand < COMMAND_DELAY) {
    delay(10);
  }
  
  clearBuffer();
  
  Serial.print("SIM800L: Sending: ");
  Serial.println(command);
  
  sim800->println(command);
  lastCommand = millis();
  
  waitForResponse(timeout);
  
  bool success = response.indexOf(expectedResponse) >= 0;
  
  if (success) {
    Serial.println("SIM800L: Command successful");
  } else {
    Serial.print("SIM800L: Command failed. Expected: ");
    Serial.print(expectedResponse);
    Serial.print(", Got: ");
    Serial.println(response);
  }
  
  return success;
}

void SIM800L::waitForResponse(unsigned long timeout) {
  response = "";
  unsigned long startTime = millis();
  
  while (millis() - startTime < timeout) {
    if (sim800->available()) {
      char c = sim800->read();
      response += c;
      
      // Stop reading if we get a complete response
      if (response.indexOf("OK") >= 0 || response.indexOf("ERROR") >= 0) {
        break;
      }
    }
    delay(10);
  }
}

String SIM800L::getResponse() {
  return response;
}

void SIM800L::clearBuffer() {
  while (sim800->available()) {
    sim800->read();
  }
  response = "";
}

void SIM800L::printResponse() {
  Serial.print("SIM800L Response: ");
  Serial.println(response);
}

bool SIM800L::checkNetworkRegistration() {
  if (sendATCommand("AT+CREG?", "+CREG: 0,1", 10000) || 
      sendATCommand("AT+CREG?", "+CREG: 0,5", 10000)) {
    Serial.println("SIM800L: Network registered");
    return true;
  } else {
    Serial.println("SIM800L: Network not registered");
    return false;
  }
}

String SIM800L::getSignalStrength() {
  if (sendATCommand("AT+CSQ", "+CSQ:", 3000)) {
    return response;
  }
  return "Unknown";
}

String SIM800L::getNetworkOperator() {
  if (sendATCommand("AT+COPS?", "+COPS:", 5000)) {
    return response;
  }
  return "Unknown";
}

bool SIM800L::isNetworkConnected() {
  return checkNetworkRegistration();
}

bool SIM800L::sendSMS(String phoneNumber, String message) {
  if (!isReady()) {
    Serial.println("SIM800L: Module not ready for SMS");
    return false;
  }
  
  Serial.print("SIM800L: Sending SMS to ");
  Serial.println(phoneNumber);
  
  // Set SMS text mode
  if (!sendATCommand("AT+CMGF=1", "OK", 3000)) {
    return false;
  }
  
  // Set recipient
  String recipient = "AT+CMGS=\"" + phoneNumber + "\"";
  sim800->println(recipient);
  delay(1000);
  
  // Send message
  sim800->print(message);
  delay(500);
  sim800->write(26); // Ctrl+Z to send
  
  waitForResponse(10000);
  
  if (response.indexOf("+CMGS:") >= 0) {
    Serial.println("SIM800L: SMS sent successfully");
    return true;
  } else {
    Serial.println("SIM800L: SMS sending failed");
    return false;
  }
}

bool SIM800L::makeCall(String phoneNumber) {
  if (!isReady()) {
    Serial.println("SIM800L: Module not ready for call");
    return false;
  }
  
  String dialCommand = "ATD" + phoneNumber + ";";
  return sendATCommand(dialCommand, "OK", 5000);
}

bool SIM800L::hangupCall() {
  return sendATCommand("ATH", "OK", 3000);
}

bool SIM800L::answerCall() {
  return sendATCommand("ATA", "OK", 3000);
}

void SIM800L::printModuleInfo() {
  Serial.println("=== SIM800L Module Info ===");
  
  // Module information
  sendATCommand("ATI", "OK", 3000);
  Serial.print("Module Info: ");
  printResponse();
  
  // Signal strength
  Serial.print("Signal Strength: ");
  Serial.println(getSignalStrength());
  
  // Network operator
  Serial.print("Network Operator: ");
  Serial.println(getNetworkOperator());
  
  // Network registration
  Serial.print("Network Status: ");
  Serial.println(checkNetworkRegistration() ? "Registered" : "Not Registered");
  
  Serial.println("==========================");
}

void SIM800L::testModule() {
  Serial.println("SIM800L: Starting module test");
  
  // Test 1: Basic AT communication
  Serial.println("SIM800L: Test 1 - Basic AT communication");
  if (sendATCommand("AT", "OK", 3000)) {
    Serial.println("SIM800L: Basic communication OK");
  } else {
    Serial.println("SIM800L: Basic communication FAILED");
    return;
  }
  
  // Test 2: Module information
  Serial.println("SIM800L: Test 2 - Module information");
  printModuleInfo();
  
  // Test 3: Network registration
  Serial.println("SIM800L: Test 3 - Network registration");
  if (checkNetworkRegistration()) {
    Serial.println("SIM800L: Network registration OK");
  } else {
    Serial.println("SIM800L: Network registration FAILED");
  }
  
  Serial.println("SIM800L: Module test complete");
}

void SIM800L::testSMS() {
  Serial.println("SIM800L: Testing SMS functionality");
  Serial.println("SIM800L: Note - This is a simulation, no actual SMS sent");
  
  // Test SMS mode setting
  if (sendATCommand("AT+CMGF=1", "OK", 3000)) {
    Serial.println("SIM800L: SMS text mode set successfully");
  } else {
    Serial.println("SIM800L: Failed to set SMS text mode");
  }
  
  // Test SMS storage settings
  sendATCommand("AT+CPMS?", "OK", 3000);
  Serial.println("SIM800L: SMS storage info retrieved");
  
  Serial.println("SIM800L: SMS test complete");
}

void SIM800L::testCall() {
  Serial.println("SIM800L: Testing call functionality");
  Serial.println("SIM800L: Note - This is a simulation, no actual call made");
  
  // Test call capabilities
  sendATCommand("AT+COLP=1", "OK", 3000);
  Serial.println("SIM800L: Call line identification enabled");
  
  sendATCommand("AT+CLIP=1", "OK", 3000);
  Serial.println("SIM800L: Caller ID enabled");
  
  Serial.println("SIM800L: Call test complete");
}

void SIM800L::testGPRS() {
  Serial.println("SIM800L: Testing GPRS functionality");
  Serial.println("SIM800L: Note - This requires APN configuration");
  
  // Check GPRS attachment
  sendATCommand("AT+CGATT?", "+CGATT:", 5000);
  Serial.println("SIM800L: GPRS attachment status checked");
  
  // Check PDP context
  sendATCommand("AT+CGDCONT?", "OK", 3000);
  Serial.println("SIM800L: PDP context checked");
  
  Serial.println("SIM800L: GPRS test complete");
}