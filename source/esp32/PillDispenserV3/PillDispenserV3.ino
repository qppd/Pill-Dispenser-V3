#include "WIFI_CONFIG.h"
#include "PCA9685_CONFIG.h"

void setup() {
 
    // put your setup code here, to run once:
    Serial.begin(115200);
    setupWiFiManager();
    initPCA9685();

}

void loop() {
    // put your main code here, to run repeatedly:   
}
