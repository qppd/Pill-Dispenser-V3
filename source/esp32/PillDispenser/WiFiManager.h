#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "TimeManager.h"

void setupWiFi(const char* ssid, const char* password, TimeManager* timeManager) {
  Serial.println("\\n=== WiFi Setup ===");
  Serial.print("Connecting to: ");
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
    Serial.println("✅ WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    if (timeManager) {
      timeManager->begin("pool.ntp.org", 28800, 0); // GMT+8 Philippine Time
    }
  } else {
    Serial.println("\\n❌ WiFi failed");
  }
}

#endif
