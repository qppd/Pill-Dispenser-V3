# Firebase Service Account Setup Guide

## 🔐 What Changed?

The Pill Dispenser V3 system now uses **Service Account Authentication** instead of anonymous authentication, providing:
- ✅ Better security and reliability
- ✅ Production-ready authentication
- ✅ No user email/password needed
- ✅ Automatic token management
- ✅ Retry logic with exponential backoff
- ✅ Network diagnostics for troubleshooting

This implementation is based on the proven **Smart Fan** project pattern.

---

## 📋 Prerequisites

- Firebase project created
- Firebase Realtime Database enabled
- Admin access to Firebase Console

---

## 🔑 Step 1: Generate Service Account Credentials

### 1. Open Firebase Console
Go to: [https://console.firebase.google.com/](https://console.firebase.google.com/)

### 2. Select Your Project
Click on `pilldispenser-5c037` (or your project name)

### 3. Navigate to Service Accounts
1. Click **⚙️ Project Settings** (gear icon in left sidebar)
2. Click **Service Accounts** tab
3. You'll see your service account email like:
   ```
   firebase-adminsdk-xxxxx@pilldispenser-5c037.iam.gserviceaccount.com
   ```

### 4. Generate Private Key
1. Click **Generate New Private Key** button
2. Click **Generate Key** in the confirmation dialog
3. A JSON file will download (e.g., `pilldispenser-5c037-firebase-adminsdk-xxxxx.json`)

### 5. Keep This File Secure! 🔒
⚠️ **IMPORTANT**: This file contains sensitive credentials. Never commit it to public repositories!

---

## 📝 Step 2: Extract Credentials from JSON

Open the downloaded JSON file. You'll need these values:

```json
{
  "type": "service_account",
  "project_id": "pilldispenser-5c037",
  "private_key_id": "...",
  "private_key": "-----BEGIN PRIVATE KEY-----\nMIIE...\n-----END PRIVATE KEY-----\n",
  "client_email": "firebase-adminsdk-xxxxx@pilldispenser-5c037.iam.gserviceaccount.com",
  "client_id": "...",
  "auth_uri": "https://accounts.google.com/o/oauth2/auth",
  "token_uri": "https://oauth2.googleapis.com/token",
  ...
}
```

You need:
- ✅ `project_id`
- ✅ `private_key` (entire value including `-----BEGIN/END-----`)
- ✅ `client_email`

---

## 🔧 Step 3: Update FirebaseConfig.cpp

Open: `source/esp32/PillDispenser/FirebaseConfig.cpp`

### Update Client Email (Line ~38):
```cpp
const char* PillDispenserConfig::getClientEmail() {
    // REPLACE THIS with your client_email from JSON file
    return "firebase-adminsdk-xxxxx@pilldispenser-5c037.iam.gserviceaccount.com";
}
```

**Replace with YOUR client_email:**
```cpp
const char* PillDispenserConfig::getClientEmail() {
    return "firebase-adminsdk-abc12@pilldispenser-5c037.iam.gserviceaccount.com";
}
```

### Update Private Key (Line ~43):
```cpp
const char* PillDispenserConfig::getPrivateKey() {
    // IMPORTANT: Keep the \n characters - they represent newlines!
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_PRIVATE_KEY_HERE\n"
           "-----END PRIVATE KEY-----\n";
}
```

**Replace with YOUR private_key from JSON:**

The private key in the JSON looks like this:
```json
"private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQE...(many lines)...YOUR_KEY_HERE\n-----END PRIVATE KEY-----\n"
```

Copy the **entire value** between the quotes, including the `\n` characters.

**Example (shortened for readability):**
```cpp
const char* PillDispenserConfig::getPrivateKey() {
    return "-----BEGIN PRIVATE KEY-----\n"
           "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCknMHs00WuXVul\n"
           "LhmWGdgj1PqI2VzS1VM7DT09QJ56cD7ubyMxDADsGG1gonws61Z6LXij9JQlbjhh\n"
           "LHsGM2Wdxwd/X112mb1CHQJqzQIkeJiYe1VsCkzmFI7mol6U1AOz/9tr4E9u6q++\n"
           // ... many more lines ...
           "JlXWUwGe38dJXzCAum2MCt4=\n"
           "-----END PRIVATE KEY-----\n";
}
```

💡 **Tips:**
- Keep each line of the key on a separate line in your code
- End each line with `\n"`
- Start the next line with `"`
- The `\n` characters are REQUIRED - they represent line breaks in the key

---

## ✅ Step 4: Verify Configuration

Your `FirebaseConfig.cpp` should now have:

```cpp
const char* PillDispenserConfig::getProjectId() {
    return "pilldispenser-5c037";  // ✅ Your project ID
}

const char* PillDispenserConfig::getClientEmail() {
    return "firebase-adminsdk-xxxxx@pilldispenser-5c037.iam.gserviceaccount.com";  // ✅ Your client email
}

const char* PillDispenserConfig::getPrivateKey() {
    return "-----BEGIN PRIVATE KEY-----\n"
           "YOUR_ACTUAL_PRIVATE_KEY_LINES_HERE\n"
           "-----END PRIVATE KEY-----\n";  // ✅ Your private key
}
```

---

## 🚀 Step 5: Upload and Test

### 1. Upload Firmware
```bash
# In Arduino IDE:
1. Open PillDispenser.ino
2. Select Board: ESP32 Dev Module
3. Click Upload
```

### 2. Check Serial Monitor

You should see:
```
=== Network Diagnostics ===
WiFi Status: Connected
IP Address: 192.168.1.XXX
DNS Test: SUCCESS - pool.ntp.org resolved to X.X.X.X
NTP Time: SUCCESS - 2025-12-10 15:30:00
Free Heap: XXXXX bytes
=== End Diagnostics ===

FirebaseManager: Setting up Firebase with service account authentication...
Firebase Client v4.x.x

Initializing Firebase with retry logic...
.....
FirebaseManager: ✅ Firebase initialized successfully!
```

### 3. If Initialization Fails

You'll see retry attempts:
```
FirebaseManager: Retry 1/5 failed. Waiting 2000 ms before retry...
FirebaseManager: Retry 2/5 failed. Waiting 4000 ms before retry...
```

**Common Issues:**
- ❌ **Wrong private key format**: Make sure you have `\n` characters preserved
- ❌ **Missing lines in key**: Copy the ENTIRE private key including BEGIN/END
- ❌ **Wrong client email**: Check spelling and project ID
- ❌ **WiFi not connected**: Check network diagnostics output
- ❌ **Time not synced**: NTP must work for SSL/TLS authentication

---

## 🆕 New Features Added

### 1. Enhanced TimeManager Functions
```cpp
// New functions with fallback mechanisms:
time_t getTimestampWithFallback();      // Returns NTP time or fallback
String getFormattedDateTimeWithFallback();  // Human-readable datetime
String getCurrentLogPrefix();            // For logging: "[2025-12-10 15:30:00] "
bool isNTPSynced();                     // Check if NTP time is valid
```

### 2. Network Diagnostics
```cpp
// Automatically called during initialization:
printNetworkDiagnostics();

// Shows:
// - WiFi connection status and signal strength
// - DNS resolution test
// - NTP synchronization status
// - Available heap memory
```

### 3. Retry Logic
- Automatic retry on connection failure
- Exponential backoff (2s, 4s, 6s, 8s, 10s)
- Up to 5 retry attempts
- Clear status messages

### 4. Better Error Messages
- ✅ Success indicators with emojis
- ❌ Clear failure messages
- 🔄 Retry progress tracking
- ⚠️ Warning messages for issues

---

## 🧪 Testing Commands

After successful initialization, test using serial commands:

```
status              # Check all systems
firebase test       # Test Firebase connection
time sync          # Force NTP sync
diagnostics        # Show network diagnostics
```

---

## 📱 Firebase Database Rules

Update your Firebase Realtime Database rules for service account access:

```json
{
  "rules": {
    "pilldispenser": {
      ".read": "auth != null",
      ".write": "auth != null",
      "devices": {
        "$deviceId": {
          ".read": "auth != null",
          ".write": "auth != null"
        }
      }
    }
  }
}
```

Or for development (less secure):
```json
{
  "rules": {
    "pilldispenser": {
      ".read": true,
      ".write": true
    }
  }
}
```

---

## 🔒 Security Best Practices

1. **Never commit credentials to Git:**
   ```bash
   # Add to .gitignore:
   source/esp32/PillDispenser/FirebaseConfig.cpp
   *-firebase-adminsdk-*.json
   ```

2. **Rotate keys periodically:**
   - Generate new private key every 90 days
   - Delete old keys from Firebase Console

3. **Use environment-specific credentials:**
   - Development environment: One service account
   - Production environment: Different service account

4. **Monitor usage:**
   - Check Firebase Console for unusual activity
   - Set up billing alerts

---

## 📞 Troubleshooting

### Issue: "SSL handshake failed"
**Solution:** 
- Check if NTP time is synchronized
- Verify internet connection stability
- Try increasing `config.timeout.sslHandshake` to 60 seconds

### Issue: "Authentication failed"
**Solution:**
- Verify client email matches Firebase Console
- Check private key is complete (including BEGIN/END lines)
- Ensure no extra spaces or characters in key

### Issue: "Firebase not ready after retries"
**Solution:**
- Check network diagnostics output
- Verify DNS resolution works
- Test internet connectivity
- Check Firebase Database URL is correct

### Issue: "Memory allocation failed"
**Solution:**
- Reduce buffer sizes in `initializeFirebase()`
- Close unused connections
- Restart ESP32

---

## 📚 Reference: Smart Fan Implementation

This implementation is based on the working Smart Fan project:
- Located: `FINISHED PROJECT 2025/SMART FAN/`
- Files: `FirebaseConfig.cpp`, `NTPConfig.cpp`
- Proven stable for ESP8266/ESP32

Key differences for Pill Dispenser:
- ✅ ESP32 instead of ESP8266 (more memory available)
- ✅ Different Firebase database schema
- ✅ Additional features (scheduling, SMS)

---

## ✅ Checklist

Before deploying to production:

- [ ] Service account private key configured
- [ ] Client email configured
- [ ] Project ID matches your Firebase project
- [ ] WiFi credentials set
- [ ] NTP synchronization working
- [ ] Network diagnostics show all green
- [ ] Firebase initialization successful (no retries)
- [ ] Data streaming connected
- [ ] Heartbeat sent successfully
- [ ] FirebaseConfig.cpp added to .gitignore

---

## 🎉 Success!

Once you see:
```
FirebaseManager: ✅ Firebase initialized successfully!
FirebaseManager: Device stream initialized successfully!
```

Your system is ready for production use with secure, reliable Firebase connectivity!

---

**Next Steps:**
- Follow `QUICK_START.md` to create your first schedule
- Review `PRODUCTION_README.md` for full system documentation
- Test all features using serial commands

**Questions?** Check the troubleshooting section or review the Smart Fan implementation for reference.
