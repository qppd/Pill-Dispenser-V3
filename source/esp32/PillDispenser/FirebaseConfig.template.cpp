#include "FirebaseConfig.h"

// Firebase Configuration Template
// Copy this file to FirebaseConfig.cpp and fill in your actual credentials
// DO NOT COMMIT FirebaseConfig.cpp TO PUBLIC REPOSITORY

// Replace these with your actual Firebase credentials
const char* PillDispenserConfig::getFirebaseAuth() {
    return "your-email@example.com";  // Replace with your Firebase email
}

const char* PillDispenserConfig::getFirebasePassword() {
    return "your-password";  // Replace with your Firebase password
}

const char* PillDispenserConfig::getDatabaseURL() {
    return "https://your-project-id-default-rtdb.firebaseio.com";  // Replace with your database URL
}

const char* PillDispenserConfig::getApiKey() {
    return "your-api-key";  // Replace with your Firebase API key
}

const char* PillDispenserConfig::getProjectId() {
    return "your-project-id";  // Replace with your Firebase project ID
}

const char* PillDispenserConfig::getClientEmail() {
    return "firebase-adminsdk-xxxxx@your-project-id.iam.gserviceaccount.com";  // Replace with your service account email
}

const char* PillDispenserConfig::getPrivateKey() {
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_PRIVATE_KEY_HERE\n"  // Replace with your actual private key
           "-----END PRIVATE KEY-----\n";
}