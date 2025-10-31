#include "RTClock.h"

RTClock::RTClock(uint8_t clkPin, uint8_t datPin, uint8_t rstPin) 
  : myWire(datPin, clkPin, rstPin), rtc(myWire) {
  this->clkPin = clkPin;
  this->datPin = datPin;
  this->rstPin = rstPin;
}

bool RTClock::begin() {
  rtc.Begin();
  
  if (!rtc.IsDateTimeValid()) {
    Serial.println("RTClock: RTC lost confidence in the DateTime!");
    Serial.println("RTClock: Setting default time...");
    // Set a default time if RTC lost power
    setDateTime(2025, 1, 1, 12, 0, 0);
  }
  
  if (!rtc.GetIsRunning()) {
    Serial.println("RTClock: RTC was not actively running, starting now");
    rtc.SetIsRunning(true);
  }
  
  Serial.println("RTClock: DS1302 initialized successfully");
  Serial.print("RTClock: Current time: ");
  Serial.println(getDateTimeString());
  
  return true;
}

void RTClock::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  RtcDateTime compiled = RtcDateTime(year, month, day, hour, minute, second);
  rtc.SetDateTime(compiled);
  
  Serial.print("RTClock: Time set to: ");
  Serial.println(getDateTimeString());
}

String RTClock::getTimeString() {
  RtcDateTime now = rtc.GetDateTime();
  
  String timeStr = "";
  if (now.Hour() < 10) timeStr += "0";
  timeStr += String(now.Hour());
  timeStr += ":";
  if (now.Minute() < 10) timeStr += "0";
  timeStr += String(now.Minute());
  timeStr += ":";
  if (now.Second() < 10) timeStr += "0";
  timeStr += String(now.Second());
  
  return timeStr;
}

String RTClock::getDateString() {
  RtcDateTime now = rtc.GetDateTime();
  
  String dateStr = "";
  if (now.Day() < 10) dateStr += "0";
  dateStr += String(now.Day());
  dateStr += "/";
  if (now.Month() < 10) dateStr += "0";
  dateStr += String(now.Month());
  dateStr += "/";
  dateStr += String(now.Year());
  
  return dateStr;
}

String RTClock::getDateTimeString() {
  return getDateString() + " " + getTimeString();
}

RtcDateTime RTClock::getDateTime() {
  return rtc.GetDateTime();
}

bool RTClock::isValidTime() {
  return rtc.IsDateTimeValid();
}

uint8_t RTClock::getHour() {
  return rtc.GetDateTime().Hour();
}

uint8_t RTClock::getMinute() {
  return rtc.GetDateTime().Minute();
}

uint8_t RTClock::getSecond() {
  return rtc.GetDateTime().Second();
}

uint8_t RTClock::getDay() {
  return rtc.GetDateTime().Day();
}

uint8_t RTClock::getMonth() {
  return rtc.GetDateTime().Month();
}

uint16_t RTClock::getYear() {
  return rtc.GetDateTime().Year();
}

bool RTClock::isTimeMatch(uint8_t hour, uint8_t minute) {
  RtcDateTime now = rtc.GetDateTime();
  return (now.Hour() == hour && now.Minute() == minute && now.Second() == 0);
}

bool RTClock::hasMinutePassed() {
  static uint8_t lastMinute = 255; // Invalid minute to trigger first time
  uint8_t currentMinute = getMinute();
  
  if (currentMinute != lastMinute) {
    lastMinute = currentMinute;
    return true;
  }
  return false;
}

void RTClock::printDateTime() {
  Serial.print("RTClock: Current DateTime: ");
  Serial.println(getDateTimeString());
}

void RTClock::syncFromSerial() {
  Serial.println("RTClock: Enter time in format: YYYY MM DD HH MM SS");
  Serial.println("RTClock: Example: 2025 11 01 14 30 00");
  
  // Wait for input (this is for testing purposes)
  // In production, you'd implement proper serial parsing
  Serial.println("RTClock: Time sync from serial not implemented in this demo");
}

void RTClock::testRTC() {
  Serial.println("RTClock: Starting RTC test");
  
  // Test 1: Display current time
  Serial.println("RTClock: Test 1 - Current time");
  printDateTime();
  delay(1000);
  
  // Test 2: Time progression
  Serial.println("RTClock: Test 2 - Time progression (5 seconds)");
  for (int i = 0; i < 5; i++) {
    Serial.print("RTClock: ");
    Serial.println(getTimeString());
    delay(1000);
  }
  
  // Test 3: Date/Time components
  Serial.println("RTClock: Test 3 - Date/Time components");
  Serial.print("RTClock: Year: "); Serial.println(getYear());
  Serial.print("RTClock: Month: "); Serial.println(getMonth());
  Serial.print("RTClock: Day: "); Serial.println(getDay());
  Serial.print("RTClock: Hour: "); Serial.println(getHour());
  Serial.print("RTClock: Minute: "); Serial.println(getMinute());
  Serial.print("RTClock: Second: "); Serial.println(getSecond());
  
  // Test 4: Validity check
  Serial.println("RTClock: Test 4 - Validity check");
  Serial.print("RTClock: Is time valid? ");
  Serial.println(isValidTime() ? "YES" : "NO");
  Serial.print("RTClock: Is RTC running? ");
  Serial.println(rtc.GetIsRunning() ? "YES" : "NO");
  
  Serial.println("RTClock: Test complete");
}