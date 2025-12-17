#ifndef PINS_CONFIG_H
#define PINS_CONFIG_H

// ===== PIN CONFIGURATION FOR PILL DISPENSER V3 =====
// Centralized pin mapping for all components

// ===== I2C PINS (DEFAULT FOR ESP32) =====
#define PIN_SDA 21
#define PIN_SCL 22

// ===== SIM800L GSM MODULE PINS =====
#define PIN_SIM800_RX 16
#define PIN_SIM800_TX 17
#define PIN_SIM800_RST 4

// ===== VOLTAGE SENSOR PIN =====
#define PIN_VOLTAGE_SENSOR 34  // ADC1_CH5 - Supports analog reading

// ===== STATUS LED PIN =====
#define PIN_STATUS_LED 2

// ===== BUZZER PIN =====
#define PIN_BUZZER 27

// ===== ARDUINO UNO SERIAL COMMUNICATION PINS =====
#define PIN_UNO_RX 26   // ESP32 RX <- Arduino TX
#define PIN_UNO_TX 33   // ESP32 TX -> Arduino RX

#endif
