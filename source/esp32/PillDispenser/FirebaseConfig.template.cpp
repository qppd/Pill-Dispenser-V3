#include "FirebaseConfig.h"

// Firebase Configuration Template
// ⚠️  SECURITY WARNING: This file contains sensitive credentials
// ⚠️  NEVER commit this file with real credentials to any repository
// ⚠️  Use environment variables or secure key management in production
//
// SETUP INSTRUCTIONS:
// 1. Copy this file to FirebaseConfig.cpp
// 2. Replace all placeholder values with your actual Firebase credentials
// 3. Follow FIREBASE_SERVICE_ACCOUNT_SETUP.md for detailed instructions
// 4. Ensure FirebaseConfig.cpp is in .gitignore before committing

const char* PillDispenserConfig::getFirebaseAuth() {
    // ⚠️  REPLACE WITH YOUR SERVICE ACCOUNT EMAIL
    return "your-service-account@your-project.iam.gserviceaccount.com";
}

const char* PillDispenserConfig::getFirebasePassword() {
    // ⚠️  REPLACE WITH YOUR SERVICE ACCOUNT KEY/SECRET
    return "your-service-account-key";
}

const char* PillDispenserConfig::getDatabaseURL() {
    // ⚠️  REPLACE WITH YOUR FIREBASE DATABASE URL
    return "https://your-project-id-default-rtdb.firebaseio.com";
}

const char* PillDispenserConfig::getApiKey() {
    // ⚠️  REPLACE WITH YOUR FIREBASE API KEY
    return "AIzaSyXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
}

const char* PillDispenserConfig::getProjectId() {
    // ⚠️  REPLACE WITH YOUR FIREBASE PROJECT ID
    return "your-project-id";
}

const char* PillDispenserConfig::getClientEmail() {
    // ⚠️  REPLACE WITH YOUR SERVICE ACCOUNT EMAIL
    return "firebase-adminsdk-xxxxx@your-project.iam.gserviceaccount.com";
}

const char* PillDispenserConfig::getPrivateKey() {
    // ⚠️  REPLACE WITH YOUR SERVICE ACCOUNT PRIVATE KEY
    // The private key should include the full -----BEGIN/END PRIVATE KEY----- block
    // Follow FIREBASE_SERVICE_ACCOUNT_SETUP.md for detailed setup
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_PRIVATE_KEY_HERE\n"
           "-----END PRIVATE KEY-----\n";
}