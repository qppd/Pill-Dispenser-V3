#include "FirebaseConfig.h"

// Firebase Configuration
// DO NOT COMMIT THIS FILE TO PUBLIC REPOSITORY


const char* PillDispenserConfig::getFirebaseHost() {
    return "pilldispenser-5c037-default-rtdb.firebaseio.com";
}


const char* PillDispenserConfig::getFirebaseAuth() {
    return "sajedhm@gmail.com";
}

const char* PillDispenserConfig::getFirebasePassword() {
    return "password123";
}

const char* PillDispenserConfig::getDatabaseURL() {
    return "https://pilldispenser-5c037-default-rtdb.firebaseio.com";
}

const char* PillDispenserConfig::getApiKey() {
    return "AIzaSyD4651wd6_Tyub8UfkIOFm-OSLbsDq-dkw";
}

const char* PillDispenserConfig::getProjectId() {
    return "pilldispenser-5c037";
}

const char* PillDispenserConfig::getClientEmail() {
    // Get this from Firebase Console -> Project Settings -> Service Accounts
    // Example format: firebase-adminsdk-xxxxx@your-project.iam.gserviceaccount.com
    return "firebase-adminsdk-xxxxx@pilldispenser-5c037.iam.gserviceaccount.com";
}

const char* PillDispenserConfig::getPrivateKey() {
    // IMPORTANT: Get this from Firebase Console -> Project Settings -> Service Accounts
    // Click "Generate New Private Key" and copy the private_key value here
    // Keep the \n characters as they are - they represent newlines in the key
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_PRIVATE_KEY_HERE\n"
           "-----END PRIVATE KEY-----\n";
}
