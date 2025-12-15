/*****************************************************
  Pill Dispenser V3 - Arduino Uno Servo Controller
  
  This Arduino Uno handles all servo control via PCA9685
  Receives commands from ESP32 via Serial (Pins 2/3)
  
  Serial Communication:
    Arduino Pin 2 (RX) <- ESP32 GPIO26 (TX)
    Arduino Pin 3 (TX) -> ESP32 GPIO25 (RX)
    Baud Rate: 9600
  
  Command Protocol:
    PING - Test connection
    STATUS - Get system status
    SET_ANGLE:<channel>,<angle> - Set servo angle (0-180)
    DISPENSE:<channel>,<size> - Dispense pill (size: small/medium/large)
    DISPENSE_PAIR:<ch1>,<ch2>,<size> - Dispense from two channels
    TEST_SERVO:<channel> - Test single servo
    CALIBRATE:<channel> - Calibrate servo range
    RESET_ALL - Reset all servos to 90 degrees
    STOP_ALL - Stop all servos
    
  Author: Pill Dispenser V3 Team
  Date: December 2025
 *****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <SoftwareSerial.h>

// ===== SERIAL COMMUNICATION =====
SoftwareSerial ESP32Serial(2, 3); // RX, TX

// ===== PCA9685 SERVO DRIVER =====
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// ===== SERVO CONFIGURATION =====
#define SERVO_FREQ 50      // Analog servos run at ~50 Hz
#define I2C_ADDRESS 0x40   // Default PCA9685 address

// Traditional servo pulse widths (for MG90S compatibility)
#define SERVO_MIN 102      // 500μs (0 degrees)
#define SERVO_MAX 512      // 2500μs (180 degrees)

// Pill dispensing timing (milliseconds)
#define DISPENSE_DURATION_SMALL  800
#define DISPENSE_DURATION_MEDIUM 1000
#define DISPENSE_DURATION_LARGE  1200

// ===== FUNCTION PROTOTYPES =====
void setServoAngle(uint8_t channel, uint16_t angle);
void dispensePill(uint8_t channel, String pillSize);
void dispensePillPair(uint8_t ch1, uint8_t ch2, String pillSize);
void testServo(uint8_t channel);
void calibrateServo(uint8_t channel);
void resetAllServos();
void stopAllServos();
void processCommand(String command);
int getDurationForPillSize(String pillSize);

// ===== SETUP =====
void setup() {
  // Initialize USB Serial for debugging
  Serial.begin(9600);
  Serial.println("=====================================");
  Serial.println("  Pill Dispenser V3 - Arduino Uno");
  Serial.println("  Servo Controller");
  Serial.println("=====================================");
  
  // Initialize ESP32 Serial communication
  ESP32Serial.begin(9600);
  delay(100);
  ESP32Serial.println("READY");
  Serial.println("ESP32 Serial initialized");
  
  // Initialize PCA9685
  pwm.begin();
  pwm.setOscillatorFrequency(27000000); // Calibrate for your PCA9685
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);
  
  // Initialize all servos to neutral position
  stopAllServos();
  
  Serial.println("PCA9685 initialized - All servos stopped");
  Serial.println("System ready - Waiting for commands...");
  Serial.println("=====================================\n");
  
  // Send ready signal to ESP32
  ESP32Serial.println("INIT:OK");
}

// ===== MAIN LOOP =====
void loop() {
  // Check for commands from ESP32
  if (ESP32Serial.available()) {
    String command = ESP32Serial.readStringUntil('\n');
    command.trim();
    
    if (command.length() > 0) {
      Serial.print("Received: ");
      Serial.println(command);
      processCommand(command);
    }
  }
  
  // Send heartbeat every 5 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 5000) {
    ESP32Serial.println("HEARTBEAT");
    lastHeartbeat = millis();
  }
}

// ===== COMMAND PROCESSING =====
void processCommand(String command) {
  // PING command
  if (command == "PING") {
    ESP32Serial.println("PONG");
    Serial.println("Response: PONG");
  }
  
  // STATUS command
  else if (command == "STATUS") {
    ESP32Serial.println("OK:READY");
    Serial.println("Response: OK:READY");
  }
  
  // SET_ANGLE command: SET_ANGLE:<channel>,<angle>
  else if (command.startsWith("SET_ANGLE:")) {
    int commaIndex = command.indexOf(',');
    if (commaIndex > 0) {
      uint8_t channel = command.substring(10, commaIndex).toInt();
      uint16_t angle = command.substring(commaIndex + 1).toInt();
      
      if (channel <= 15 && angle <= 180) {
        setServoAngle(channel, angle);
        ESP32Serial.println("OK:SET_ANGLE:" + String(channel) + "," + String(angle));
      } else {
        ESP32Serial.println("ERROR:Invalid parameters");
      }
    } else {
      ESP32Serial.println("ERROR:Invalid format");
    }
  }
  
  // DISPENSE command: DISPENSE:<channel>,<size>
  else if (command.startsWith("DISPENSE:")) {
    int commaIndex = command.indexOf(',');
    if (commaIndex > 0) {
      uint8_t channel = command.substring(9, commaIndex).toInt();
      String pillSize = command.substring(commaIndex + 1);
      pillSize.trim();
      
      if (channel <= 15) {
        dispensePill(channel, pillSize);
        ESP32Serial.println("OK:DISPENSED:" + String(channel) + "," + pillSize);
      } else {
        ESP32Serial.println("ERROR:Invalid channel");
      }
    } else {
      ESP32Serial.println("ERROR:Invalid format");
    }
  }
  
  // DISPENSE_PAIR command: DISPENSE_PAIR:<ch1>,<ch2>,<size>
  else if (command.startsWith("DISPENSE_PAIR:")) {
    int firstComma = command.indexOf(',');
    int secondComma = command.indexOf(',', firstComma + 1);
    
    if (firstComma > 0 && secondComma > 0) {
      uint8_t ch1 = command.substring(14, firstComma).toInt();
      uint8_t ch2 = command.substring(firstComma + 1, secondComma).toInt();
      String pillSize = command.substring(secondComma + 1);
      pillSize.trim();
      
      if (ch1 <= 15 && ch2 <= 15) {
        dispensePillPair(ch1, ch2, pillSize);
        ESP32Serial.println("OK:DISPENSED_PAIR:" + String(ch1) + "," + String(ch2) + "," + pillSize);
      } else {
        ESP32Serial.println("ERROR:Invalid channels");
      }
    } else {
      ESP32Serial.println("ERROR:Invalid format");
    }
  }
  
  // TEST_SERVO command: TEST_SERVO:<channel>
  else if (command.startsWith("TEST_SERVO:")) {
    uint8_t channel = command.substring(11).toInt();
    if (channel <= 15) {
      testServo(channel);
      ESP32Serial.println("OK:TEST_COMPLETE:" + String(channel));
    } else {
      ESP32Serial.println("ERROR:Invalid channel");
    }
  }
  
  // CALIBRATE command: CALIBRATE:<channel>
  else if (command.startsWith("CALIBRATE:")) {
    uint8_t channel = command.substring(10).toInt();
    if (channel <= 15) {
      calibrateServo(channel);
      ESP32Serial.println("OK:CALIBRATE_COMPLETE:" + String(channel));
    } else {
      ESP32Serial.println("ERROR:Invalid channel");
    }
  }
  
  // RESET_ALL command
  else if (command == "RESET_ALL") {
    resetAllServos();
    ESP32Serial.println("OK:RESET_ALL_COMPLETE");
  }
  
  // STOP_ALL command
  else if (command == "STOP_ALL") {
    stopAllServos();
    ESP32Serial.println("OK:STOP_ALL_COMPLETE");
  }
  
  // Unknown command
  else {
    ESP32Serial.println("ERROR:Unknown command: " + command);
    Serial.println("Unknown command: " + command);
  }
}

// ===== SERVO CONTROL FUNCTIONS =====

void setServoAngle(uint8_t channel, uint16_t angle) {
  if (channel > 15 || angle > 180) {
    Serial.println("Error: Invalid channel or angle");
    return;
  }
  
  // Map angle (0-180) to pulse width (SERVO_MIN-SERVO_MAX)
  uint16_t pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(channel, 0, pulse);
  
  Serial.print("Servo ");
  Serial.print(channel);
  Serial.print(" -> ");
  Serial.print(angle);
  Serial.println("°");
}

int getDurationForPillSize(String pillSize) {
  pillSize.toLowerCase();
  
  if (pillSize == "small") {
    return DISPENSE_DURATION_SMALL;
  } else if (pillSize == "medium") {
    return DISPENSE_DURATION_MEDIUM;
  } else if (pillSize == "large") {
    return DISPENSE_DURATION_LARGE;
  } else {
    Serial.println("Unknown pill size, using medium");
    return DISPENSE_DURATION_MEDIUM;
  }
}

void dispensePill(uint8_t channel, String pillSize) {
  Serial.print("Dispensing ");
  Serial.print(pillSize);
  Serial.print(" pill from channel ");
  Serial.println(channel);
  
  // Move servo to 180 degrees
  setServoAngle(channel, 180);
  delay(100);
  
  // Wait for pill to dispense
  delay(2000);
  
  // Move servo back to 0 degrees
  setServoAngle(channel, 0);
  delay(100);
  
  Serial.println("Dispensing complete");
}

void dispensePillPair(uint8_t ch1, uint8_t ch2, String pillSize) {
  Serial.print("Dispensing ");
  Serial.print(pillSize);
  Serial.print(" pills from channels ");
  Serial.print(ch1);
  Serial.print(" & ");
  Serial.println(ch2);
  
  // Move both servos to 180 degrees
  setServoAngle(ch1, 180);
  setServoAngle(ch2, 180);
  delay(100);
  
  // Wait for pills to dispense
  delay(2000);
  
  // Move both servos back to 0 degrees
  setServoAngle(ch1, 0);
  setServoAngle(ch2, 0);
  delay(150);
  
  Serial.println("Pair dispensing complete");
}

void testServo(uint8_t channel) {
  Serial.print("Testing servo ");
  Serial.println(channel);
  
  // Test sequence: 0 -> 90 -> 180 -> 90
  setServoAngle(channel, 0);
  delay(1000);
  setServoAngle(channel, 90);
  delay(1000);
  setServoAngle(channel, 180);
  delay(1000);
  setServoAngle(channel, 90);
  delay(500);
  
  Serial.println("Test complete");
}

void calibrateServo(uint8_t channel) {
  Serial.print("Calibrating servo ");
  Serial.println(channel);
  
  // Test minimum position (0°)
  Serial.println("Moving to 0°...");
  setServoAngle(channel, 0);
  delay(1500);
  
  // Test maximum position (180°)
  Serial.println("Moving to 180°...");
  setServoAngle(channel, 180);
  delay(1500);
  
  // Test center position (90°)
  Serial.println("Moving to 90°...");
  setServoAngle(channel, 90);
  delay(1000);
  
  // Test quarter positions
  Serial.println("Testing 45°...");
  setServoAngle(channel, 45);
  delay(1000);
  
  Serial.println("Testing 135°...");
  setServoAngle(channel, 135);
  delay(1000);
  
  // Return to center
  setServoAngle(channel, 90);
  delay(500);
  
  Serial.println("Calibration complete");
}

void resetAllServos() {
  Serial.println("Resetting all servos to 90°");
  for (uint8_t i = 0; i < 16; i++) {
    setServoAngle(i, 90);
    delay(50);
  }
  Serial.println("Reset complete");
}

void stopAllServos() {
  Serial.println("Stopping all servos");
  for (uint8_t i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, 0);
  }
  Serial.println("All servos stopped");
}
