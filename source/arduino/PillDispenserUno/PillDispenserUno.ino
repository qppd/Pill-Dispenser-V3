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
    DISPENSE:<channel> - Dispense pill from channel
    DISPENSE_PAIR:<ch1>,<ch2> - Dispense from two channels
    TEST_SERVO:<channel> - Test single servo
    CALIBRATE:<channel> - Calibrate servo range
    RESET_ALL - Reset all servos to 90 degrees
    STOP_ALL - Stop all servos
    MOVE_TO_RELEASE - Move CH5/CH6 to release position (CH5: 90→0, CH6: 0→90)
    MOVE_TO_HOME - Move CH5/CH6 to home position (CH5: 0→90, CH6: 90→0)
    
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

// ===== CH5/CH6 NON-BLOCKING SERVO CONTROL =====
bool servosMoving = false;
unsigned long servoMoveStartTime = 0;
int ch5StartAngle = 90;
int ch5TargetAngle = 90;
int ch6StartAngle = 0;
int ch6TargetAngle = 0;
const int SERVO_MOVE_DURATION = 1000; // 1 second smooth movement

// ===== FUNCTION PROTOTYPES =====
void setServoAngle(uint8_t channel, uint16_t angle);
void dispensePill(uint8_t channel);
void dispensePillPair(uint8_t ch1, uint8_t ch2);
void testServo(uint8_t channel);
void calibrateServo(uint8_t channel);
void resetAllServos();
void stopAllServos();
void processCommand(String command);
void startServoMovement(int ch5Start, int ch5Target, int ch6Start, int ch6Target);
void updateServoMovement();
void moveServosToRelease();
void moveServosToHome();

// ===== SETUP =====
void setup() {
  // Initialize USB Serial for debugging
  Serial.begin(115200);
  Serial.println("=====================================");
  Serial.println("  Pill Dispenser V3 - Arduino Uno");
  Serial.println("  Servo Controller");
  Serial.println("=====================================");
  
  // Initialize ESP32 Serial communication
  ESP32Serial.begin(115200);
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
  
  // Set CH5 and CH6 to their starting positions
  setServoAngle(5, 90);  // CH5 starts at 90°
  setServoAngle(6, 0);   // CH6 starts at 0°
  delay(100);
  
  Serial.println("PCA9685 initialized - All servos stopped");
  Serial.println("CH5 initialized to 90°, CH6 initialized to 0°");
  Serial.println("System ready - Waiting for commands...");
  Serial.println("=====================================\n");
  
  // Send ready signal to ESP32
  ESP32Serial.println("INIT:OK");
}

// ===== MAIN LOOP =====
void loop() {
  // Update non-blocking servo movement for CH5/CH6
  updateServoMovement();
  
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
  // static unsigned long lastHeartbeat = 0;
  // if (millis() - lastHeartbeat > 5000) {
  //   ESP32Serial.println("HEARTBEAT");
  //   lastHeartbeat = millis();
  // }
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
  
  // DISPENSE command: DISPENSE:<channel>
  else if (command.startsWith("DISPENSE:")) {
    uint8_t channel = command.substring(9).toInt();
    
    if (channel <= 15) {
      dispensePill(channel);
      ESP32Serial.println("OK:DISPENSED:" + String(channel));
    } else {
      ESP32Serial.println("ERROR:Invalid channel");
    }
  }
  
  // DISPENSE_PAIR command: DISPENSE_PAIR:<ch1>,<ch2>
  else if (command.startsWith("DISPENSE_PAIR:")) {
    int commaIndex = command.indexOf(',');
    
    if (commaIndex > 0) {
      uint8_t ch1 = command.substring(14, commaIndex).toInt();
      uint8_t ch2 = command.substring(commaIndex + 1).toInt();
      
      if (ch1 <= 15 && ch2 <= 15) {
        dispensePillPair(ch1, ch2);
        ESP32Serial.println("OK:DISPENSED_PAIR:" + String(ch1) + "," + String(ch2));
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
  
  // MOVE_TO_RELEASE command - CH5: 90→ 0, CH6: 0→ 90
  else if (command == "MOVE_TO_RELEASE") {
    moveServosToRelease();
    ESP32Serial.println("OK:MOVE_TO_RELEASE_STARTED");
  }
  
  // MOVE_TO_HOME command - CH5: 0→ 90, CH6: 90→ 0
  else if (command == "MOVE_TO_HOME") {
    moveServosToHome();
    ESP32Serial.println("OK:MOVE_TO_HOME_STARTED");
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

void dispensePill(uint8_t channel) {
  Serial.print("Dispensing pill from channel ");
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

void dispensePillPair(uint8_t ch1, uint8_t ch2) {
  Serial.print("Dispensing pills from channels ");
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

// ===== CH5/CH6 NON-BLOCKING MOVEMENT FUNCTIONS =====

void startServoMovement(int ch5Start, int ch5Target, int ch6Start, int ch6Target) {
  ch5StartAngle = ch5Start;
  ch5TargetAngle = ch5Target;
  ch6StartAngle = ch6Start;
  ch6TargetAngle = ch6Target;
  servoMoveStartTime = millis();
  servosMoving = true;
  
  Serial.print("Starting servo movement: CH5 ");
  Serial.print(ch5Start);
  Serial.print("° → ");
  Serial.print(ch5Target);
  Serial.print("°, CH6 ");
  Serial.print(ch6Start);
  Serial.print("° → ");
  Serial.print(ch6Target);
  Serial.println("°");
}

void updateServoMovement() {
  if (!servosMoving) {
    return;
  }
  
  unsigned long elapsed = millis() - servoMoveStartTime;
  
  if (elapsed >= SERVO_MOVE_DURATION) {
    // Movement complete - set to target positions
    setServoAngle(5, ch5TargetAngle);
    setServoAngle(6, ch6TargetAngle);
    servosMoving = false;
    Serial.println("Servo movement complete");
    return;
  }
  
  // Calculate current position using linear interpolation
  float progress = (float)elapsed / (float)SERVO_MOVE_DURATION;
  
  int ch5CurrentAngle = ch5StartAngle + (int)((ch5TargetAngle - ch5StartAngle) * progress);
  int ch6CurrentAngle = ch6StartAngle + (int)((ch6TargetAngle - ch6StartAngle) * progress);
  
  setServoAngle(5, ch5CurrentAngle);
  setServoAngle(6, ch6CurrentAngle);
  
  delay(20); // Small delay for smooth movement
}

void moveServosToRelease() {
  Serial.println("Moving to RELEASE position");
  // CH5: 90° → 0°
  // CH6: 0° → 90°
  startServoMovement(90, 0, 0, 90);
}

void moveServosToHome() {
  Serial.println("Moving to HOME position");
  // CH5: 0° → 90°
  // CH6: 90° → 0°
  startServoMovement(0, 90, 90, 0);
}
