#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class LCDDisplay {
private:
  LiquidCrystal_I2C lcd;
  uint8_t i2cAddress;
  static const uint8_t COLS = 20;
  static const uint8_t ROWS = 4;
  
public:
  LCDDisplay(uint8_t address = 0x27);
  bool begin();
  void clear();
  void setCursor(uint8_t col, uint8_t row);
  void print(String text);
  void print(String text, uint8_t col, uint8_t row);
  void printLine(String text, uint8_t row);
  void centerText(String text, uint8_t row);
  void displayWelcome();
  void displayTime(String timeStr);
  void displayPillCount(int count);
  void displayStatus(String status);
  void displayError(String error);
  void testDisplay();
  bool isConnected();
  void backlight(bool on);
  void displayMainScreen();
  void displayTestMenu();
};

#endif