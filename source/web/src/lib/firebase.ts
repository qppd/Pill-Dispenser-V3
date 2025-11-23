import { initializeApp } from 'firebase/app';
import { getAuth } from 'firebase/auth';
import { getDatabase } from 'firebase/database';

const firebaseConfig = {
  apiKey: process.env.NEXT_PUBLIC_FIREBASE_API_KEY,
  authDomain: process.env.NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN,
  databaseURL: process.env.NEXT_PUBLIC_FIREBASE_DATABASE_URL,
  projectId: process.env.NEXT_PUBLIC_FIREBASE_PROJECT_ID,
  storageBucket: process.env.NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: process.env.NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID,
  appId: process.env.NEXT_PUBLIC_FIREBASE_APP_ID,
};

// Only initialize if API key is set and not placeholder
let app: any = null;
let authInstance: any = null;
let dbInstance: any = null;

if (firebaseConfig.apiKey && firebaseConfig.apiKey !== 'your_api_key') {
  app = initializeApp(firebaseConfig);
  authInstance = getAuth(app);
  dbInstance = getDatabase(app);
}

export const auth = authInstance;
export const db = dbInstance;