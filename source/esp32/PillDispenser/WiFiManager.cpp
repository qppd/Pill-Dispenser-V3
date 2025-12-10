#include "WiFiManager.h"

void setupWiFi(const char* ssid, const char* password, TimeManager* timeManager) {
    Serial.println("\n=== WiFi Setup ===");
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        // Initialize NTP after successful WiFi connection
        if (timeManager != nullptr) {
            timeManager->begin("pool.ntp.org", 0, 0); // GMT+0, adjust as needed
        }
    } else {
        Serial.println();
        Serial.println("Failed to connect to WiFi");
        Serial.println("Check credentials and try again");
    }
}
