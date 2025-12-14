#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-W#warnings"
#include <LiquidCrystal_I2C.h>
#pragma GCC diagnostic pop

class LCDDisplay {
private:
  LiquidCrystal_I2C lcd;
  uint8_t i2cAddress;
  static const uint8_t COLS = 20;
  static const uint8_t ROWS = 4;
  
public:
  LCDDisplay(uint8_t address = 0x27) : lcd(address, COLS, ROWS), i2cAddress(address) {}
  
  bool begin() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    Wire.beginTransmission(i2cAddress);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.println("LCD: Initialized");
      return true;
    }
    Serial.println("LCD: Failed to initialize");
    return false;
  }
  
  void clear() { lcd.clear(); }
  void setCursor(uint8_t col, uint8_t row) { lcd.setCursor(col, row); }
  void print(String text) { lcd.print(text); }
  
  void print(String text, uint8_t col, uint8_t row) {
    lcd.setCursor(col, row);
    lcd.print(text);
  }
  
  void displayTime(String timeStr) {
    lcd.setCursor(0, 0);
    lcd.print("Time: " + timeStr + "      ");
  }
  
  void displayMainScreen() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pill Dispenser V3");
    lcd.setCursor(0, 1);
    lcd.print("Ready...");
  }
  
  void displayDispenseInfo(int containerNum, String medication) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dispensing...");
    lcd.setCursor(0, 1);
    lcd.print("Container: " + String(containerNum));
    lcd.setCursor(0, 2);
    lcd.print(medication);
  }
};

#endif