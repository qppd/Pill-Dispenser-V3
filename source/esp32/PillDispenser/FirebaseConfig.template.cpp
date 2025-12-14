#include "FirebaseConfig.h"

// Firebase Configuration Template
// Copy this file to FirebaseConfig.cpp and replace the placeholder values with your actual Firebase credentials
// Get these from your Firebase project settings and service account key

const char* PillDispenserConfig::getFirebaseAuth() {
    return "your_firebase_auth_token";  // Replace with actual auth token
}

const char* PillDispenserConfig::getFirebasePassword() {
    return "your_firebase_password";    // Replace with actual password
}

const char* PillDispenserConfig::getDatabaseURL() {
    return "https://your-project-id-default-rtdb.firebaseio.com/";  // Replace with your Firebase Realtime Database URL
}

const char* PillDispenserConfig::getApiKey() {
    return "your_api_key";  // Replace with your Firebase Web API Key
}

const char* PillDispenserConfig::getProjectId() {
    return "your-project-id";  // Replace with your Firebase project ID
}

const char* PillDispenserConfig::getClientEmail() {
    return "firebase-adminsdk-xxxxx@your-project-id.iam.gserviceaccount.com";  // Replace with service account email
}

const char* PillDispenserConfig::getPrivateKey() {
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_PRIVATE_KEY_HERE\n"
           "-----END PRIVATE KEY-----\n";  // Replace with your service account private key
}