# Pill Dispenser Web App

A modern web application for managing pill dispensers, built with Next.js, Firebase Authentication, and Firebase Realtime Database.

## Features

- User authentication (Login/Register)
- Real-time dispenser monitoring and control
- Daily pill schedule management
- Battery and connectivity status
- API endpoints for hardware integration
- Responsive design with Tailwind CSS
- Deployed on Vercel

## Tech Stack

- **Frontend**: Next.js with TypeScript and Tailwind CSS
- **Authentication**: Firebase Auth
- **Database**: Firebase Realtime Database
- **Hosting**: Vercel

## Getting Started

### Prerequisites

- Node.js 18+
- Firebase project

### Installation

1. Clone the repository and navigate to the web directory:
   ```bash
   cd web
   ```

2. Install dependencies:
   ```bash
   npm install
   ```

3. Set up Firebase:
   - Create a Firebase project at [Firebase Console](https://console.firebase.google.com/)
   - Enable Authentication and Realtime Database
   - Copy your Firebase config

4. Create a `.env.local` file in the root directory and add your Firebase config:
   ```
   NEXT_PUBLIC_FIREBASE_API_KEY=your_api_key_here
   NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=pilldispenser-5c037.firebaseapp.com
   NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://pilldispenser-5c037-default-rtdb.firebaseio.com
   NEXT_PUBLIC_FIREBASE_PROJECT_ID=pilldispenser-5c037
   NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=pilldispenser-5c037.firebasestorage.app
   NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=your_messaging_sender_id_here
   NEXT_PUBLIC_FIREBASE_APP_ID=your_app_id_here
   ```

5. Run the development server:
   ```bash
   npm run dev
   ```

6. Open [http://localhost:3000](http://localhost:3000) in your browser.

## Project Structure

- `src/app/` - Next.js app router pages
  - `login/` - User login page
  - `register/` - User registration page
  - `dashboard/` - Main dispenser control dashboard
  - `api/` - API routes for hardware integration
- `src/lib/` - Firebase configuration and utilities
- `src/components/` - Reusable React components

## API Endpoints

### GET /api/dispenser?userId={userId}
Get dispenser data for a user.

### POST /api/dispenser
Update dispenser status from hardware.
```json
{
  "userId": "firebase-user-id",
  "status": "online|offline",
  "battery": 85,
  "pillsRemaining": 25,
  "lastDispensed": "timestamp"
}
```

### POST /api/dispense
Trigger pill dispensing.
```json
{
  "userId": "firebase-user-id",
  "pills": 1
}
```

## Usage Flow

1. **Registration/Login**: Users create accounts and authenticate
2. **Dashboard Access**: Authenticated users are redirected to the dashboard
3. **Monitor Status**: View real-time dispenser status, battery, and pill count
4. **Schedule Management**: Set daily pill dispensing times
5. **Manual Control**: Dispense pills manually from the web interface
6. **Hardware Integration**: ESP32/RPi devices update status via API endpoints

## Deployment

Deploy to Vercel:

1. Push your code to GitHub
2. Connect your repo to Vercel
3. Add environment variables in Vercel dashboard
4. Deploy!

## Hardware Integration

The web app provides REST API endpoints that your ESP32 and Raspberry Pi can use to:

- Update dispenser status and sensor data
- Receive dispensing commands
- Sync schedules and configurations

Use the Firebase service account JSON (not the web config) in your hardware code for server-side Firebase access.

## Learn More

- [Next.js Documentation](https://nextjs.org/docs)
- [Firebase Documentation](https://firebase.google.com/docs)
- [Tailwind CSS](https://tailwindcss.com/docs)
