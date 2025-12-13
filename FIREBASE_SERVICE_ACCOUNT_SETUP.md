# Firebase Service Account Setup

## 1. Create Firebase Project
1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Click "Add project"
3. Enter project name: "pill-dispenser-v3"
4. Enable Realtime Database

## 2. Generate Service Account Credentials
1. Project Settings → Service Accounts
2. Generate new private key
3. Download JSON file (keep secure!)
4. Extract credentials for `FirebaseConfig.cpp`

## 3. Configure ESP32
Use the extracted credentials in `FirebaseConfig.cpp`:
- `client_email`
- `private_key`
- `project_id`
- `database_url`

## 4. Configure Web App
Add credentials to `.env.local`:
- API key
- Auth domain
- Database URL
- Project ID

## 5. Security Best Practices
- Never commit `FirebaseConfig.cpp` to repository
- Use `.gitignore` to exclude credential files
- Rotate service account keys periodically
- Use production database rules