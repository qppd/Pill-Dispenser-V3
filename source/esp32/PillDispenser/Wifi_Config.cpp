#include "Wifi_Config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

bool setupWiFiWithManager(TimeManager* timeManager, const char* apName) {
    Serial.println("\n=== WiFi Manager Setup ===");
    
    // Create WiFiManager instance
    WiFiManager wm;
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    
    // Uncomment to reset WiFi settings for testing
    // wm.resetSettings();
    
    // Set timeout for configuration portal (3 minutes)
    wm.setConfigPortalTimeout(180);
    
    // Create unique AP name using device MAC
    String uniqueAPName = String(apName) + "_" + String(ESP.getEfuseMac(), HEX);
    
    Serial.println("Attempting to connect to saved WiFi...");
    Serial.println("If no credentials or connection fails:");
    Serial.print("  - AP Name: ");
    Serial.println(uniqueAPName);
    Serial.println("  - AP Password: 12345678");
    Serial.println("  - Configuration Portal: http://192.168.4.1");
    
    // Try to connect with saved credentials or start AP
    // autoConnect will:
    // 1. Try to connect to saved WiFi
    // 2. If fails, start AP with captive portal
    // 3. Wait for user to configure WiFi
    // 4. Return true if connected, false if timeout
    bool res = wm.autoConnect(uniqueAPName.c_str(), "12345678");
    
    if (!res) {
        Serial.println("❌ Failed to connect to WiFi");
        Serial.println("Configuration portal timeout or user cancelled");
        return false;
    } else {
        Serial.println("✅ WiFi connected successfully!");
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        
        // Initialize NTP after successful WiFi connection
        if (timeManager != nullptr) {
            Serial.println("Initializing NTP time sync...");
            timeManager->begin("pool.ntp.org", 28800, 0); // GMT+8 Philippine Time
        }
        
        return true;
    }
}

void resetWiFiSettings() {
    Serial.println("\n=== Resetting WiFi Settings ===");
    
    WiFiManager wm;
    wm.resetSettings();
    
    Serial.println("✅ WiFi credentials cleared from storage");
    Serial.println("Device will restart and enter configuration mode");
}
