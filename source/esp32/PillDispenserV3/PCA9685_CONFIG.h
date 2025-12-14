/*************************************************** 
  PCA9685 Servo Driver Configuration
  Adapted from Adafruit 16-channel PWM & Servo driver example
  
  This configuration supports 5 servo channels (ch0-ch4) for the
  pill dispenser servos.
  
  Original example by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
  
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815
  
  These drivers use I2C to communicate, 2 pins are required to  
  interface.
 ****************************************************/

#ifndef PCA9685_CONFIG_H
#define PCA9685_CONFIG_H

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// PCA9685 I2C address (default is 0x40)
#define PCA9685_ADDRESS 0x40

// Servo pulse width configuration
// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you have!
#define SERVOMIN  150   // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600   // This is the 'maximum' pulse length count (out of 4096)
#define USMIN     600   // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX     2400  // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50   // Analog servos run at ~50 Hz updates

// Dispenser servo channels (only ch0-ch4 are used)
#define SERVO_CH0 0
#define SERVO_CH1 1
#define SERVO_CH2 2
#define SERVO_CH3 3
#define SERVO_CH4 4
#define NUM_DISPENSER_SERVOS 5

// Create PCA9685 driver instance
// Called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(PCA9685_ADDRESS);
// You can also call it with a different address if needed:
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

/**
 * Initialize the PCA9685 servo driver
 */
void initPCA9685() {
  Serial.println("Initializing PCA9685 Servo Driver...");
  
  pwm.begin();
  
  /*
   * In theory the internal oscillator (clock) is 25MHz but it really isn't
   * that precise. You can 'calibrate' this by tweaking this number until
   * you get the PWM update frequency you're expecting!
   * The int.osc. for the PCA9685 chip is a range between about 23-27MHz and
   * is used for calculating things like writeMicroseconds()
   * Analog servos run at ~50 Hz updates, It is importaint to use an
   * oscilloscope in setting the int.osc frequency for the I2C PCA9685 chip.
   * 1) Attach the oscilloscope to one of the PWM signal pins and ground on
   *    the I2C PCA9685 chip you are setting the value for.
   * 2) Adjust setOscillatorFrequency() until the PWM update frequency is the
   *    expected value (50Hz for most ESCs)
   * Setting the value here is specific to each individual I2C PCA9685 chip and
   * affects the calculations for the PWM update frequency. 
   * Failure to correctly set the int.osc value will cause unexpected PWM results
   */
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);
  
  Serial.println("PCA9685 initialized successfully!");
}

/**
 * Set servo pulse length in seconds
 * e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. 
 * Note: It's not precise!
 * 
 * @param n Servo channel (0-4 for dispenser servos)
 * @param pulse Pulse length in seconds
 */
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= SERVO_FREQ;   // Analog servos run at ~50 Hz updates
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000000;  // convert input seconds to us
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}

/**
 * Test all dispenser servos (channels 0-4)
 * Sweeps each servo from min to max position
 */
void testDispenserServos() {
  Serial.println("Testing dispenser servos (ch0-ch4)...");
  
  for (uint8_t servonum = 0; servonum < NUM_DISPENSER_SERVOS; servonum++) {
    Serial.print("Testing servo channel: ");
    Serial.println(servonum);
    
    // Drive servo using setPWM()
    for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
      pwm.setPWM(servonum, 0, pulselen);
    }
    delay(500);
    
    for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
      pwm.setPWM(servonum, 0, pulselen);
    }
    delay(500);
    
    // Drive servo using writeMicroseconds()
    // Not precise due to calculation rounding, but mimics Arduino Servo library
    for (uint16_t microsec = USMIN; microsec < USMAX; microsec++) {
      pwm.writeMicroseconds(servonum, microsec);
    }
    delay(500);
    
    for (uint16_t microsec = USMAX; microsec > USMIN; microsec--) {
      pwm.writeMicroseconds(servonum, microsec);
    }
    delay(500);
  }
  
  Serial.println("Servo test complete!");
}

/**
 * Set a specific servo to a position using PWM value
 * 
 * @param channel Servo channel (0-4)
 * @param pulseLength Pulse length (SERVOMIN to SERVOMAX)
 */
void setServoPosition(uint8_t channel, uint16_t pulseLength) {
  if (channel >= NUM_DISPENSER_SERVOS) {
    Serial.println("Error: Invalid servo channel!");
    return;
  }
  
  // Constrain pulse length to safe range
  pulseLength = constrain(pulseLength, SERVOMIN, SERVOMAX);
  pwm.setPWM(channel, 0, pulseLength);
}

/**
 * Set a specific servo to a position using microseconds
 * 
 * @param channel Servo channel (0-4)
 * @param microseconds Microsecond value (USMIN to USMAX)
 */
void setServoMicroseconds(uint8_t channel, uint16_t microseconds) {
  if (channel >= NUM_DISPENSER_SERVOS) {
    Serial.println("Error: Invalid servo channel!");
    return;
  }
  
  // Constrain microseconds to safe range
  microseconds = constrain(microseconds, USMIN, USMAX);
  pwm.writeMicroseconds(channel, microseconds);
}

// Non-blocking servo testing variables
static bool isTestingServos = false;
static uint8_t currentTestServo = 0;
static uint16_t currentTestAngle = 0;
static bool sweepingForward = true;
static unsigned long lastServoUpdate = 0;
static const unsigned long SERVO_UPDATE_INTERVAL = 15; // ms between angle updates

/**
 * Convert angle (0-180) to pulse length
 * 
 * @param angle Angle in degrees (0-180)
 * @return Pulse length for PWM
 */
uint16_t angleToPulse(uint16_t angle) {
  // Map angle 0-180 to SERVOMIN-SERVOMAX
  return map(angle, 0, 180, SERVOMIN, SERVOMAX);
}

/**
 * Start sequential servo testing (0-180° and back, one servo at a time)
 * Call this function to begin the test
 */
void startServoTest() {
  if (isTestingServos) {
    Serial.println("Servo test already running!");
    return;
  }
  
  Serial.println("Starting sequential servo test (0-180° and back)...");
  isTestingServos = true;
  currentTestServo = 0;
  currentTestAngle = 0;
  sweepingForward = true;
  lastServoUpdate = millis();
  
  Serial.print("Testing servo CH");
  Serial.println(currentTestServo);
}

/**
 * Stop the servo testing
 */
void stopServoTest() {
  isTestingServos = false;
  Serial.println("Servo test stopped.");
}

/**
 * Update servo testing (call this in loop() for non-blocking operation)
 * Tests servos sequentially: CH0 -> CH1 -> CH2 -> CH3 -> CH4 -> repeat
 */
void updateServoTest() {
  if (!isTestingServos) return;
  
  unsigned long currentTime = millis();
  if (currentTime - lastServoUpdate < SERVO_UPDATE_INTERVAL) return;
  
  lastServoUpdate = currentTime;
  
  // Set current servo to current angle
  uint16_t pulse = angleToPulse(currentTestAngle);
  pwm.setPWM(currentTestServo, 0, pulse);
  
  // Update angle
  if (sweepingForward) {
    currentTestAngle++;
    if (currentTestAngle >= 180) {
      sweepingForward = false;
      Serial.println("Reached 180°, sweeping back...");
    }
  } else {
    currentTestAngle--;
    if (currentTestAngle <= 0) {
      sweepingForward = true;
      Serial.print("Completed servo CH");
      Serial.println(currentTestServo);
      
      // Move to next servo
      currentTestServo++;
      if (currentTestServo >= NUM_DISPENSER_SERVOS) {
        currentTestServo = 0;
        Serial.println("All servos tested. Starting over...");
      }
      
      Serial.print("Testing servo CH");
      Serial.println(currentTestServo);
    }
  }
}

/**
 * Handle serial commands for servo testing
 * Call this in loop() to process serial input
 */
void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.equalsIgnoreCase("test_all_pill_dispenser")) {
      startServoTest();
    } else if (command.equalsIgnoreCase("stop_test")) {
      stopServoTest();
    } else if (command.startsWith("servo ")) {
      // Parse commands like "servo 0 90" to set servo 0 to 90 degrees
      int space1 = command.indexOf(' ');
      int space2 = command.indexOf(' ', space1 + 1);
      
      if (space1 > 0 && space2 > space1) {
        uint8_t servoNum = command.substring(space1 + 1, space2).toInt();
        uint16_t angle = command.substring(space2 + 1).toInt();
        
        if (servoNum < NUM_DISPENSER_SERVOS && angle <= 180) {
          uint16_t pulse = angleToPulse(angle);
          pwm.setPWM(servoNum, 0, pulse);
          Serial.print("Set servo CH");
          Serial.print(servoNum);
          Serial.print(" to ");
          Serial.print(angle);
          Serial.println("°");
        } else {
          Serial.println("Invalid servo number or angle (0-180°)");
        }
      }
    }
  }
}

#endif // PCA9685_CONFIG_H
