#include "LCDDisplay.h"

LCDDisplay::LCDDisplay(uint8_t address) : lcd(address, COLS, ROWS) {
  i2cAddress = address;
}

bool LCDDisplay::begin() {
  lcd.init();
  lcd.backlight();
  
  // Test if device is connected
  Wire.beginTransmission(i2cAddress);
  uint8_t error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println("LCDDisplay: I2C LCD initialized successfully");
    displayWelcome();
    return true;
  } else {
    Serial.println("LCDDisplay: Failed to initialize I2C LCD");
    return false;
  }
}

void LCDDisplay::clear() {
  lcd.clear();
}

void LCDDisplay::setCursor(uint8_t col, uint8_t row) {
  if (col < COLS && row < ROWS) {
    lcd.setCursor(col, row);
  }
}

void LCDDisplay::print(String text) {
  lcd.print(text);
}

void LCDDisplay::print(String text, uint8_t col, uint8_t row) {
  setCursor(col, row);
  lcd.print(text);
}

void LCDDisplay::printLine(String text, uint8_t row) {
  if (row >= ROWS) return;
  
  // Clear the entire row
  setCursor(0, row);
  for (int i = 0; i < COLS; i++) {
    lcd.print(" ");
  }
  
  // Print the text at the beginning of the row
  setCursor(0, row);
  lcd.print(text.substring(0, COLS)); // Truncate if too long
}

void LCDDisplay::centerText(String text, uint8_t row) {
  if (row >= ROWS) return;
  
  uint8_t startCol = (COLS - text.length()) / 2;
  if (startCol < 0) startCol = 0;
  
  printLine("", row); // Clear the row
  print(text, startCol, row);
}

bool LCDDisplay::isConnected() {
  Wire.beginTransmission(i2cAddress);
  return (Wire.endTransmission() == 0);
}

void LCDDisplay::backlight(bool on) {
  if (on) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}

void LCDDisplay::displayWelcome() {
  clear();
  centerText("PILL DISPENSER V3", 0);
  centerText("Initializing...", 1);
  print("System Starting", 0, 3);
  Serial.println("LCDDisplay: Welcome screen displayed");
}

void LCDDisplay::displayMainScreen() {
  clear();
  centerText("PILL DISPENSER V3", 0);
  print("Status: Ready", 0, 1);
  print("Next: --:--", 0, 2);
  print("Count: 0", 0, 3);
}

void LCDDisplay::displayTime(String timeStr) {
  print("Time: " + timeStr, 0, 2);
}

void LCDDisplay::displayPillCount(int count) {
  String countStr = "Pills: " + String(count);
  print(countStr + "    ", 0, 3); // Add spaces to clear previous longer text
}

void LCDDisplay::displayStatus(String status) {
  String statusStr = "Status: " + status;
  printLine(statusStr, 1);
}

void LCDDisplay::displayError(String error) {
  clear();
  centerText("ERROR", 0);
  centerText(error, 1);
  print("Check connections", 0, 3);
  Serial.print("LCDDisplay: Error displayed - ");
  Serial.println(error);
}

void LCDDisplay::displayTestMenu() {
  clear();
  print("=== TEST MODE ===", 0, 0);
  print("Send commands via", 0, 1);
  print("Serial Monitor", 0, 2);
  print("Type 'help' for list", 0, 3);
}

void LCDDisplay::testDisplay() {
  Serial.println("LCDDisplay: Starting display test");
  
  // Test 1: Clear and basic text
  clear();
  print("Test 1: Basic Text", 0, 0);
  delay(2000);
  
  // Test 2: All rows
  clear();
  for (int i = 0; i < ROWS; i++) {
    print("Row " + String(i) + ": Hello World", 0, i);
  }
  delay(3000);
  
  // Test 3: Centered text
  clear();
  for (int i = 0; i < ROWS; i++) {
    centerText("Center " + String(i), i);
  }
  delay(3000);
  
  // Test 4: Backlight test
  print("Backlight OFF", 0, 0);
  backlight(false);
  delay(2000);
  backlight(true);
  clear();
  print("Backlight ON", 0, 0);
  delay(2000);
  
  // Test 5: Character fill test
  clear();
  print("Character Fill Test", 0, 0);
  for (int row = 1; row < ROWS; row++) {
    for (int col = 0; col < COLS; col++) {
      setCursor(col, row);
      print(String((char)('A' + (row * COLS + col) % 26)));
      delay(50);
    }
  }
  delay(2000);
  
  displayMainScreen();
  Serial.println("LCDDisplay: Test complete");
}