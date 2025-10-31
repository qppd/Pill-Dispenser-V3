#ifndef RTCLOCK_H
#define RTCLOCK_H

#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

class RTClock {
private:
  ThreeWire myWire;
  RtcDS1302<ThreeWire> rtc;
  uint8_t clkPin, datPin, rstPin;
  
public:
  RTClock(uint8_t clkPin, uint8_t datPin, uint8_t rstPin);
  bool begin();
  void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
  String getTimeString();
  String getDateString();
  String getDateTimeString();
  RtcDateTime getDateTime();
  bool isValidTime();
  void testRTC();
  void printDateTime();
  void syncFromSerial();
  uint8_t getHour();
  uint8_t getMinute();
  uint8_t getSecond();
  uint8_t getDay();
  uint8_t getMonth();
  uint16_t getYear();
  
  // Pill scheduling helpers
  bool isTimeMatch(uint8_t hour, uint8_t minute);
  bool hasMinutePassed();
  void setAlarm(uint8_t hour, uint8_t minute);
};

#endif