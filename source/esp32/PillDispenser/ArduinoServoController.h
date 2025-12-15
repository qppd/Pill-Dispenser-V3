#ifndef ARDUINO_SERVO_CONTROLLER_H
#define ARDUINO_SERVO_CONTROLLER_H

#include <Arduino.h>
#include <HardwareSerial.h>

/**
 * ArduinoServoController
 * 
 * This class provides an interface for controlling servos via serial
 * communication with an Arduino Uno.
 * 
 * The ESP32 sends commands to the Arduino, which physically controls
 * the servos via a PCA9685 module.
 * 
 * Communication Protocol:
 *   - Baud Rate: 9600
 *   - Commands are sent as text strings ending with '\n'
 *   - Responses start with OK: or ERROR:
 */
class ArduinoServoController {
private:
  HardwareSerial* serial;
  uint8_t rxPin;
  uint8_t txPin;
  unsigned long responseTimeout;
  bool arduinoReady;
  
  // Send command and wait for response
  String sendCommand(String command, unsigned long timeout = 2000);
  
  // Check if response indicates success
  bool isSuccessResponse(String response);
  
public:
  /**
   * Constructor
   * @param rxPin ESP32 RX pin (connects to Arduino TX)
   * @param txPin ESP32 TX pin (connects to Arduino RX)
   * @param timeout Response timeout in milliseconds (default: 2000ms)
   */
  ArduinoServoController(uint8_t rxPin, uint8_t txPin, unsigned long timeout = 2000);
  
  /**
   * Initialize serial communication with Arduino
   * @return true if Arduino responds, false otherwise
   */
  bool begin();
  
  /**
   * Check if Arduino is connected and responding
   * @return true if Arduino is ready
   */
  bool isConnected();
  
  /**
   * Set servo to specific angle
   * @param channel Servo channel (0-15)
   * @param angle Target angle in degrees (0-180)
   * @return true if successful
   */
  bool setServoAngle(uint8_t channel, uint16_t angle);
  
  /**
   * Dispense a pill from specified channel
   * @param channel Servo channel (0-15)
   * @param pillSize Size of pill: "small", "medium", "large" (default: "medium")
   * @return true if successful
   */
  bool dispensePill(uint8_t channel, String pillSize = "medium");
  
  /**
   * Dispense pills from two channels simultaneously
   * @param channel1 First servo channel (0-15)
   * @param channel2 Second servo channel (0-15)
   * @param pillSize Size of pill: "small", "medium", "large" (default: "medium")
   * @return true if successful
   */
  bool dispensePillPair(uint8_t channel1, uint8_t channel2, String pillSize = "medium");
  
  /**
   * Test a single servo through full range
   * @param channel Servo channel (0-15)
   * @return true if successful
   */
  bool testServo(uint8_t channel);
  
  /**
   * Calibrate servo and test its range
   * @param channel Servo channel (0-15)
   * @return true if successful
   */
  bool calibrateServo(uint8_t channel);
  
  /**
   * Reset all servos to 90 degree (neutral) position
   * @return true if successful
   */
  bool resetAllServos();
  
  /**
   * Stop all servos (deactivate PWM outputs)
   * @return true if successful
   */
  bool stopAllServos();
  
  /**
   * Send ping command to test connection
   * @return true if Arduino responds with PONG
   */
  bool ping();
  
  /**
   * Check Arduino status
   * @return true if Arduino is ready
   */
  bool checkStatus();
  
  /**
   * Move CH5 and CH6 servos to release position (non-blocking, simultaneous)
   * CH5: 90° → 0°
   * CH6: 0° → 90°
   * @return true if successful
   */
  bool moveServosToRelease();
  
  /**
   * Move CH5 and CH6 servos to home position (non-blocking, simultaneous)
   * CH5: 0° → 90°
   * CH6: 90° → 0°
   * @return true if successful
   */
  bool moveServosToHome();
  
  /**
   * Read any available messages from Arduino
   * Useful for monitoring heartbeat and async messages
   */
  void update();
};

#endif
