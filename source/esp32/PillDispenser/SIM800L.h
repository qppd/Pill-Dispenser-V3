#ifndef SIM800L_H
#define SIM800L_H

#include <Arduino.h>

#include <HardwareSerial.h>


class SIM800L {
private:
  HardwareSerial* sim800;
  uint8_t rxPin, txPin, rstPin;
  bool isModuleReady;
  bool isNetworkRegistered;
  String response;
  unsigned long lastCommand;
  unsigned long lastNetworkCheck;
  unsigned long lastReconnectAttempt;
  static const unsigned long COMMAND_DELAY = 1000;
  static const unsigned long NETWORK_CHECK_INTERVAL = 60000; // Check every 60 seconds
  static const unsigned long RECONNECT_INTERVAL = 30000; // Retry every 30 seconds

public:
  SIM800L(uint8_t rxPin, uint8_t txPin, uint8_t rstPin, HardwareSerial& serialPort = Serial2);
  bool begin(long baudRate = 9600);
  bool isReady();
  void reset();

  // Basic AT commands
  bool sendATCommand(String command, String expectedResponse = "OK", unsigned long timeout = 5000);
  String getResponse();
  void clearBuffer();

  // Network operations
  bool checkNetworkRegistration();
  String getSignalStrength();
  String getNetworkOperator();
  bool isNetworkConnected();
  void update(); // Call in main loop for background reconnection
  bool attemptNetworkReconnect();

  // SMS operations
  bool sendSMS(String phoneNumber, String message);
  bool readSMS(int index);
  bool deleteSMS(int index);
  String getLastSMS();

  // Call operations
  bool makeCall(String phoneNumber);
  bool hangupCall();
  bool answerCall();

  // GPRS/Internet operations
  bool enableGPRS(String apn, String username = "", String password = "");
  bool disableGPRS();
  bool sendHTTPRequest(String url, String data = "");

  // Testing functions
  void testModule();
  void testSMS();
  void testCall();
  void testGPRS();
  void printModuleInfo();

  // Utility functions
  void waitForResponse(unsigned long timeout = 5000);
  bool waitForOK(unsigned long timeout = 5000);
  void printResponse();
};

#endif