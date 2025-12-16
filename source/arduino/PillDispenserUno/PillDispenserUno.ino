/*****************************************************
  Pill Dispenser V3 - Arduino Uno Servo Controller
  
  This Arduino Uno handles all servo control via PCA9685
  Receives commands from ESP32 via Serial (Pins 2/3)
  
  Serial Communication:
    Arduino Pin 2 (RX) <- ESP32 GPIO26 (TX)
    Arduino Pin 3 (TX) -> ESP32 GPIO25 (RX)
    Baud Rate: 9600
  
  Command Protocol (Ultra-Short for Serial Reliability):
    PING - Test connection
    STATUS - Get system status
    SA<ch>,<angle> - Set servo angle (SA0,90)
    DP<ch> - Dispense pill (DP0-DP4)
    DP2<ch1>,<ch2> - Dispense pair (DP20,1)
    TS<ch> - Test servo (TS0)
    CA<ch> - Calibrate (CA0)
    RS - Reset all servos to 90°
    ST - Stop all servos
    RL - Release position (CH5: 90→0, CH6: 0→90)
    MH - Move to home (CH5: 0→90, CH6: 90→0)
    
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
void smoothSetServoAngle(uint8_t channel, uint16_t targetAngle, uint8_t speed);
void processSerialMonitorCommand(String command);
void startServoMovement(int ch5Start, int ch5Target, int ch6Start, int ch6Target);
void updateServoMovement();
void moveServosToRelease();
void moveServosToHome();

// ===== SETUP =====
void setup() {
  // Initialize USB Serial for debugging
  Serial.begin(115200);
  Serial.println(F("====================================="));
  Serial.println(F("  Pill Dispenser V3 - Arduino Uno"));
  Serial.println(F("  Servo Controller"));
  Serial.println(F("====================================="));
  
  // Initialize ESP32 Serial communication
  ESP32Serial.begin(115200);
  delay(100);
  ESP32Serial.println(F("READY"));
  Serial.println(F("ESP32 Serial initialized"));
  
  // Initialize PCA9685
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
  delay(10);
  
  // Initialize all servos to neutral position
  stopAllServos();
  
  // Set pill dispenser channels (0-4) to closed position (180°)
  for (uint8_t ch = 0; ch <= 4; ch++) {
    setServoAngle(ch, 180);
    delay(50);
  }
  
  // Set CH5 and CH6 to their starting positions
  setServoAngle(5, 90);
  // setServoAngle(6, 0); // CH6 deactivated for calibration
  delay(100);
  
  Serial.println(F("PCA9685 initialized"));
  Serial.println(F("System ready"));
  
  // Send ready signal to ESP32
  ESP32Serial.println(F("INIT:OK"));
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
      Serial.print("[ESP32] Received: ");
      Serial.println(command);
      processCommand(command);
    }
  }
  
  // Check for commands from Serial Monitor (for testing)
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase(); // Make it case-insensitive
    
    if (command.length() > 0) {
      Serial.print("[MONITOR] Command: ");
      Serial.println(command);
      processSerialMonitorCommand(command);
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
    ESP32Serial.println(F("PONG"));
    Serial.println(F("PONG"));
  }
  
  // STATUS command
  else if (command == "STATUS") {
    ESP32Serial.println(F("OK:READY"));
    Serial.println(F("OK:READY"));
  }
  
  // SET_ANGLE command: SET_ANGLE:<channel>,<angle>
  else if (command.startsWith("SET_ANGLE:")) {
    int commaIndex = command.indexOf(',');
    if (commaIndex > 0) {
      uint8_t channel = command.substring(10, commaIndex).toInt();
      uint16_t angle = command.substring(commaIndex + 1).toInt();
      
      if (channel <= 15 && angle <= 180) {
        setServoAngle(channel, angle);
        ESP32Serial.print(F("OK:SET_ANGLE:"));
        ESP32Serial.print(channel);
        ESP32Serial.print(',');
        ESP32Serial.println(angle);
      } else {
        ESP32Serial.println(F("ERROR:Invalid"));
      }
    } else {
      ESP32Serial.println(F("ERROR:Format"));
    }
  }
  
  // DP2 command: DP2 (Dispense channel 2, same as DP0, DP1, DP3, DP4)
  else if (command == "DP2") {
    uint8_t channel = 2;
    Serial.print(F("DP"));
    Serial.println(channel);
    testServo(channel);  // Use working test logic
    ESP32Serial.print(F("OK:DP"));
    ESP32Serial.println(channel);
  }
  
  // DP command: DP<channel> (Dispense using dispensePill logic) - DP0-DP4
  else if (command.startsWith("DP") && command.length() >= 3 && command.length() <= 4) {
    uint8_t channel = command.substring(2).toInt();
    
    if (channel <= 4) {  // Only channels 0-4 for dispensing
      Serial.print(F("DP"));
      Serial.println(channel);
      dispensePill(channel);  // Use dispensePill logic like serial monitor
      ESP32Serial.print(F("OK:DP"));
      ESP32Serial.println(channel);
    } else {
      ESP32Serial.println(F("ERROR:Invalid"));
    }
  }
  
  // TS command: TS<channel> (Test Servo)
  else if (command.startsWith("TS")) {
    uint8_t channel = command.substring(2).toInt();
    if (channel <= 15) {
      testServo(channel);
      ESP32Serial.print(F("OK:TS"));
      ESP32Serial.println(channel);
    } else {
      ESP32Serial.println(F("ERROR:Invalid"));
    }
  }
  
  // CA command: CA<channel> (Calibrate)
  else if (command.startsWith("CA")) {
    uint8_t channel = command.substring(2).toInt();
    if (channel <= 15) {
      calibrateServo(channel);
      ESP32Serial.print(F("OK:CA"));
      ESP32Serial.println(channel);
    } else {
      ESP32Serial.println(F("ERROR:Invalid"));
    }
  }
  
  // RS command (Reset All)
  else if (command == "RS") {
    resetAllServos();
    ESP32Serial.println(F("OK:RS"));
  }
  
  // ST command (Stop All)
  else if (command == "ST") {
    stopAllServos();
    ESP32Serial.println(F("OK:ST"));
  }
  
  // RL command for release (CH5: 90→0, CH6: 0→90)
  else if (command == "RL") {
    moveServosToRelease();
    ESP32Serial.println(F("OK:RL_STARTED"));
  }
  
  // MH command for move to home (CH5: 0→90, CH6: 90→0)
  else if (command == "MH") {
    moveServosToHome();
    ESP32Serial.println(F("OK:MH_STARTED"));
  }
  
  // Unknown command
  else {
    ESP32Serial.println(F("ERROR:Unknown"));
  }
}

// ===== SERVO CONTROL FUNCTIONS =====

void setServoAngle(uint8_t channel, uint16_t angle) {
  // Clamp angle
  if (angle > 180) angle = 180;
  if (angle < 0) angle = 0;
  // Invert CH6 direction
  if (channel == 6) {
    angle = 180 - angle;
  }
  if (channel > 15) return;
  uint16_t pulse = map(angle, 0, 180, SERVO_MIN, SERVO_MAX);
  pwm.setPWM(channel, 0, pulse);
  Serial.print(F("CH"));
  Serial.print(channel);
  Serial.print(':');
  Serial.println(angle);
}

// Smooth servo movement with incremental steps
void smoothSetServoAngle(uint8_t channel, uint16_t targetAngle, uint8_t speed) {
  if (channel > 15) return;
  if (targetAngle > 180) targetAngle = 180;
  
  // Get current angle by reading back from PWM (approximate)
  // Since we can't read back, we'll assume starting from neutral or use a variable
  // For simplicity, we'll move from 0 to targetAngle or vice versa
  static uint16_t lastAngles[16] = {90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90};
  
  uint16_t currentAngle = lastAngles[channel];
  
  // Determine direction
  int step = (currentAngle < targetAngle) ? 1 : -1;
  int steps = abs(targetAngle - currentAngle);
  
  // Move smoothly from current to target
  for (int i = 0; i <= steps; i++) {
    uint16_t angle = currentAngle + (step * i);
    setServoAngle(channel, angle);
    delay(speed);  // Lower value = faster movement
  }
  
  // Update last position
  lastAngles[channel] = targetAngle;
}

void dispensePill(uint8_t channel) {
  Serial.print(F("Dispense CH"));
  Serial.println(channel);
  
  // For channels 0-4: Start at 180°, move to 0°, wait 2 seconds, return to 180°
  if (channel <= 4) {
    // Smooth movement from 180° to 0°
    smoothSetServoAngle(channel, 0, 20);
    delay(2000);  // Wait exactly 2 seconds at 0°
    // Smooth movement back to 180°
    smoothSetServoAngle(channel, 180, 20);
  } else {
    // For other channels, use original logic
    smoothSetServoAngle(channel, 180, 20);
    delay(2000);
    smoothSetServoAngle(channel, 0, 20);
  }
  delay(100);
  
  Serial.println(F("Done"));
}

void dispensePillPair(uint8_t ch1, uint8_t ch2) {
  Serial.print(F("Pair "));
  Serial.print(ch1);
  Serial.print('&');
  Serial.println(ch2);
  
  // Smooth movement for both servos to 180 degrees
  smoothSetServoAngle(ch1, 180, 10);
  smoothSetServoAngle(ch2, 180, 10);
  delay(2100);
  // Smooth movement back to 0 degrees
  smoothSetServoAngle(ch1, 0, 10);
  smoothSetServoAngle(ch2, 0, 10);
  delay(150);
  
  Serial.println(F("Done"));
}

void testServo(uint8_t channel) {
  Serial.print(F("Test CH"));
  Serial.println(channel);
  
  // Smooth test sequence: 0 -> 90 -> 180 -> 90
  smoothSetServoAngle(channel, 0, 15);
  delay(1000);
  smoothSetServoAngle(channel, 90, 15);
  delay(1000);
  smoothSetServoAngle(channel, 180, 15);
  delay(1000);
  smoothSetServoAngle(channel, 90, 15);
  delay(500);
}

void calibrateServo(uint8_t channel) {
  Serial.print(F("Cal CH"));
  Serial.println(channel);
  
  // Smooth calibration sequence
  smoothSetServoAngle(channel, 0, 20);
  delay(1500);
  smoothSetServoAngle(channel, 180, 20);
  delay(1500);
  smoothSetServoAngle(channel, 90, 20);
  delay(1000);
  smoothSetServoAngle(channel, 45, 20);
  delay(1000);
  smoothSetServoAngle(channel, 135, 20);
  delay(1000);
  smoothSetServoAngle(channel, 90, 20);
  delay(500);
}

void resetAllServos() {
  Serial.println(F("Reset"));
  for (uint8_t i = 0; i < 16; i++) {
    setServoAngle(i, 90);
    delay(50);
  }
}

void stopAllServos() {
  Serial.println(F("Stop"));
  for (uint8_t i = 0; i < 16; i++) {
    pwm.setPWM(i, 0, 0);
  }
}

// ===== CH5/CH6 NON-BLOCKING MOVEMENT FUNCTIONS =====

void startServoMovement(int ch5Start, int ch5Target, int ch6Start, int ch6Target) {
  ch5StartAngle = ch5Start;
  ch5TargetAngle = ch5Target;
  // ch6StartAngle = ch6Start; // CH6 deactivated
  // ch6TargetAngle = ch6Target; // CH6 deactivated
  servoMoveStartTime = millis();
  servosMoving = true;
  
  Serial.println(F("Moving"));
}

void updateServoMovement() {
  if (!servosMoving) {
    return;
  }
  
  unsigned long elapsed = millis() - servoMoveStartTime;
  
  if (elapsed >= SERVO_MOVE_DURATION) {
    setServoAngle(5, ch5TargetAngle);
    // setServoAngle(6, ch6TargetAngle); // CH6 deactivated
    servosMoving = false;
    Serial.println(F("Complete"));
    return;
  }
  // Calculate current position using linear interpolation
  float progress = (float)elapsed / (float)SERVO_MOVE_DURATION;
  int ch5CurrentAngle = ch5StartAngle + (int)((ch5TargetAngle - ch5StartAngle) * progress);
  // int ch6CurrentAngle = ch6StartAngle + (int)((ch6TargetAngle - ch6StartAngle) * progress); // CH6 deactivated
  setServoAngle(5, ch5CurrentAngle);
  // setServoAngle(6, ch6CurrentAngle); // CH6 deactivated
  delay(20); // Small delay for smooth movement
}

void moveServosToRelease() {
  Serial.println(F("Release"));
  startServoMovement(100, 45, 0, 0);  // CH5: 100→45, CH6 deactivated
}

void moveServosToHome() {
  Serial.println(F("Home"));
  startServoMovement(45, 100, 0, 0);  // CH5: 45→100, CH6 deactivated
}

// ===== SERIAL MONITOR COMMAND PROCESSING =====
void processSerialMonitorCommand(String command) {
  // test <channel> - Test specific servo
  if (command.startsWith("test ")) {
    int channel = command.substring(5).toInt();
    if (channel >= 0 && channel <= 15) {
      Serial.print(F("Testing servo "));
      Serial.println(channel);
      testServo(channel);
      Serial.println(F("Test complete"));
    } else {
      Serial.println(F("Invalid channel (0-15)"));
    }
  }
  
  // dispense <channel> - Dispense from specific channel
  else if (command.startsWith("dispense ")) {
    int channel = command.substring(9).toInt();
    if (channel >= 0 && channel <= 15) {
      Serial.print(F("Dispensing ch"));
      Serial.println(channel);
      dispensePill(channel);
      Serial.println(F("Complete"));
    } else {
      Serial.println(F("Invalid channel"));
    }
  }
  
  // set <channel> <angle> - Set servo to specific angle
  else if (command.startsWith("set ")) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    
    if (secondSpace > 0) {
      int channel = command.substring(firstSpace + 1, secondSpace).toInt();
      int angle = command.substring(secondSpace + 1).toInt();
      
      if (channel >= 0 && channel <= 15 && angle >= 0 && angle <= 180) {
        setServoAngle(channel, angle);
        Serial.println(F("OK"));
      } else {
        Serial.println(F("Invalid"));
      }
    } else {
      Serial.println(F("Usage: set <ch> <angle>"));
    }
  }
  
  // calibrate <channel> - Calibrate specific servo
  else if (command.startsWith("calibrate ")) {
    int channel = command.substring(10).toInt();
    if (channel >= 0 && channel <= 15) {
      calibrateServo(channel);
      Serial.println(F("Done"));
    } else {
      Serial.println(F("Invalid"));
    }
  }
  
  // reset - Reset all servos to 90°
  else if (command == "reset") {
    resetAllServos();
    Serial.println(F("Reset OK"));
  }
  
  // stop - Stop all servos
  else if (command == "stop") {
    stopAllServos();
    Serial.println(F("Stop OK"));
  }
  
  // release - Move to release position
  else if (command == "release") {
    moveServosToRelease();
    Serial.println(F("Release"));
  }
  
  // home - Move to home position
  else if (command == "home") {
    moveServosToHome();
    Serial.println(F("Home"));
  }
  
  // status - Show system status
  else if (command == "status") {
    Serial.println(F("\nSTATUS:"));
    Serial.print(F("Moving: "));
    Serial.println(servosMoving ? F("YES") : F("NO"));
  }
  
  // help - Show available commands
  else if (command == "help") {
    Serial.println(F("\nCOMMANDS:"));
    Serial.println(F("test <0-15>"));
    Serial.println(F("dispense <0-15>"));
    Serial.println(F("set <0-15> <0-180>"));
    Serial.println(F("calibrate <0-15>"));
    Serial.println(F("reset/stop/release/home"));
    Serial.println(F("status/help\n"));
  }
  
  // Unknown command
  else {
    Serial.println(F("Unknown cmd"));
  }
}
