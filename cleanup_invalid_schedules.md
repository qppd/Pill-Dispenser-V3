# Cleanup Invalid Schedules

## Problem
You currently have multiple invalid schedules in Firebase for Dispenser 0 with:
- Empty patient names
- Empty medication names
- Default time (00:00)
- More than 3 schedules per dispenser

## Solution Steps

### Option 1: Web App Cleanup (Recommended)
1. Open your web application at the schedule-v2 page
2. Navigate to each container (0-4)
3. Delete all schedules that show:
   - Empty patient name
   - Empty medication name  
   - Time of 00:00
   - Default values like "New Medication" or "Patient Name"
4. Keep only valid, configured schedules (max 3 per container)

### Option 2: Firebase Console Cleanup
1. Go to https://console.firebase.google.com
2. Select your project
3. Navigate to **Realtime Database**
4. Find the path: `pilldispenser/device/PILL_DISPENSER_841aa6080814/schedules`
5. Manually delete invalid schedule entries
6. Keep only valid schedules with proper:
   - Patient name (not empty or "Patient Name")
   - Medication name (not empty or "New Medication")  
   - Valid time (not 00:00 unless intentional)
   - Valid dispenser ID (0-4)

### Option 3: Delete All and Start Fresh
If you want to start completely fresh:
1. Go to Firebase Console → Realtime Database
2. Navigate to: `pilldispenser/device/PILL_DISPENSER_841aa6080814/schedules`
3. Click the ❌ to delete the entire `schedules` node
4. Go to your web app and create new schedules properly

## After Cleanup

After cleaning up invalid schedules, your ESP32 will:
- ✅ Only load valid schedules (max 3 per dispenser)
- ✅ Skip schedules with empty patient/medication info
- ✅ Skip schedules with default values
- ✅ Show proper schedule count per dispenser
- ✅ Display accurate schedule information on LCD

## Expected Result
With 5 dispensers and max 3 schedules each:
- **Maximum total schedules: 15**
- Container 0: 0-3 schedules
- Container 1: 0-3 schedules  
- Container 2: 0-3 schedules
- Container 3: 0-3 schedules
- Container 4: 0-3 schedules

## What Changed

### Web App ([schedule-v2/page.tsx](source/pdv3/src/app/schedule-v2/page.tsx))
- ✅ Enforces max 3 schedules per container
- ✅ Shows "X of 3 schedules used" counter
- ✅ Disables "Add Schedule" button when limit reached
- ✅ Shows alert if trying to exceed limit

### ESP32 ([FirebaseManager.cpp](source/esp32/PillDispenser/FirebaseManager.cpp))
- ✅ Validates dispenser ID (must be 0-4)
- ✅ Skips schedules with 00:00 and empty medication
- ✅ Skips schedules with empty/default patient or medication names
- ✅ Enforces max 3 schedules per dispenser during sync
- ✅ Shows detailed sync summary with skip reasons

## Next Steps
1. Clean up invalid schedules using one of the options above
2. Create proper schedules through the web app
3. Verify on ESP32 serial monitor that only valid schedules are loaded
4. Check that each container has ≤3 schedules
