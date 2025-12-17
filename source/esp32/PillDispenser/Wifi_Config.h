#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "TimeManager.h"

bool setupWiFiWithManager(TimeManager* timeManager, const char* apName = "PillDispenser_AP");
void resetWiFiSettings();

#endif // WIFI_MANAGER_H
