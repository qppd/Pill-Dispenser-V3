#ifndef FIREBASE_CONFIG_H
#define FIREBASE_CONFIG_H

#include <Arduino.h>

class PillDispenserConfig {
public:
    static const char* getFirebaseHost();
    static const char* getFirebaseAuth();
    static const char* getDatabaseURL();
    static const char* getApiKey();
    static const char* getProjectId();
};

#endif
