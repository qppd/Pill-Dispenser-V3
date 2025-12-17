#include "WiFiManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

void setupWiFi(const char* ssid, const char* password, TimeManager* timeManager) {
    Serial.println("\n=== WiFi Setup ===");
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    // Disconnect from any previous WiFi connections
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);

    // Small delay to ensure clean state
    delay(100);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        yield(); // Feed watchdog timer
        delay(500);
        Serial.print(".");

        // Print WiFi status for debugging
        if (attempts % 5 == 0) { // Every 5 attempts (2.5 seconds)
            wl_status_t status = WiFi.status();
            Serial.printf(" [Status: %d]", status);
        }

        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("WiFi connected successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        // Initialize NTP after successful WiFi connection
        if (timeManager != nullptr) {
            timeManager->begin("pool.ntp.org", 28800, 0); // GMT+8 Philippine Time (8 hours * 3600 seconds)
        }
    } else {
        Serial.println();
        Serial.println("Failed to connect to WiFi");
        Serial.printf("Final WiFi status: %d\n", WiFi.status());
        Serial.println("Possible issues:");
        Serial.println("- Check SSID and password");
        Serial.println("- Verify WiFi network is available");
        Serial.println("- Check WiFi antenna connection");
        Serial.println("- Try power cycling the device");

        // Try one more time with a fresh start
        Serial.println("Attempting one more connection...");
        WiFi.disconnect(true);
        delay(1000);
        WiFi.begin(ssid, password);

        int retryAttempts = 0;
        while (WiFi.status() != WL_CONNECTED && retryAttempts < 10) {
            yield();
            delay(1000);
            Serial.print("R");
            retryAttempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println();
            Serial.println("WiFi connected on retry!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            if (timeManager != nullptr) {
                timeManager->begin("pool.ntp.org", 28800, 0);
            }
        } else {
            Serial.println();
            Serial.println("Still failed to connect. Check your WiFi setup.");
        }
    }
}

bool checkWiFiCredentialsStored() {
    // Check if WiFi credentials are stored in NVS
    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    
    // Check if SSID is not empty
    if (strlen((const char*)conf.sta.ssid) > 0) {
        Serial.println("WiFi credentials found in NVS");
        return true;
    }
    
    Serial.println("No WiFi credentials found in NVS");
    return false;
}

void startWiFiManagerAP() {
    Serial.println("\n=== Starting WiFi Manager AP Mode ===");
    Serial.println("No WiFi credentials found or reset requested");
    Serial.println("Starting Access Point for configuration...");
    
    // Set ESP32 as Access Point
    WiFi.mode(WIFI_AP);
    
    // Create AP with device-specific name
    String apName = "PillDispenser_" + String(ESP.getEfuseMac(), HEX);
    const char* apPassword = "12345678"; // Default AP password
    
    // Start Access Point
    if (WiFi.softAP(apName.c_str(), apPassword)) {
        Serial.println("WiFi Manager AP started successfully!");
        Serial.print("AP Name: ");
        Serial.println(apName);
        Serial.print("AP Password: ");
        Serial.println(apPassword);
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());
        Serial.println("Connect to this AP to configure WiFi credentials");
        Serial.println("========================================");
    } else {
        Serial.println("Failed to start WiFi Manager AP!");
    }
}
