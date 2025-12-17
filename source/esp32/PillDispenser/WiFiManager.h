#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "TimeManager.h"

void setupWiFi(const char* ssid, const char* password, TimeManager* timeManager);
bool checkWiFiCredentialsStored();
void startWiFiManagerAP();

#endif // WIFI_MANAGER_H
