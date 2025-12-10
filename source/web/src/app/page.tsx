'use client';

import { useEffect, useState } from 'react';
import { onAuthStateChanged, signOut, User } from 'firebase/auth';
import { auth } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { Pill, Clock, Wifi, CheckCircle2 } from 'lucide-react';

export const dynamic = 'force-dynamic';

export default function Home() {
  const [user, setUser] = useState<User | null>(null);
  const [loading, setLoading] = useState(true);
  const router = useRouter();

  useEffect(() => {
    if (!auth) {
      setLoading(false);
      return;
    }
    const unsubscribe = onAuthStateChanged(auth, (user) => {
      setUser(user);
      setLoading(false);
    });

    return () => unsubscribe();
  }, []);

  const handleLogout = async () => {
    if (!auth) return;
    await signOut(auth);
    router.push('/login');
  };

  useEffect(() => {
    if (user) {
      router.push('/dashboard');
    }
  }, [user, router]);

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-xl">Loading...</div>
      </div>
    );
  }

  if (!auth) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-purple-50 to-pink-100">
        <div className="container mx-auto px-4 py-16">
          <div className="max-w-2xl mx-auto text-center">
            <h1 className="text-4xl font-bold text-gray-900 mb-4">
              Pill Dispenser Dashboard
            </h1>
            <p className="text-lg text-gray-600 mb-8">
              Manage your pill dispenser remotely and efficiently.
            </p>
            <div className="bg-white rounded-xl shadow-lg p-8">
              <h2 className="text-2xl font-semibold text-gray-900 mb-4">
                Setup Required
              </h2>
              <p className="text-gray-600 mb-6">
                Please set up your Firebase configuration in .env.local to use the app.
              </p>
            </div>
          </div>
        </div>
      </div>
    );
  }

  if (user) {
    return null; // Will redirect via useEffect
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-purple-50 to-pink-50">
      <div className="container mx-auto px-6 sm:px-8 lg:px-12 py-24">
        <div className="max-w-6xl mx-auto text-center">
          {/* Hero Section */}
          <div className="mb-16 animate-slide-in-down">
            <div className="inline-flex items-center justify-center mb-8">
              <div className="bg-gradient-to-br from-blue-500 via-purple-600 to-pink-600 p-5 rounded-3xl shadow-2xl animate-pulse">
                <Pill className="h-20 w-20 text-white" />
              </div>
            </div>
            <h1 className="text-5xl md:text-6xl lg:text-7xl font-extrabold bg-gradient-to-r from-blue-600 via-purple-600 to-pink-600 bg-clip-text text-transparent mb-8 leading-tight">
              Smart Pill Dispenser
            </h1>
            <p className="text-xl md:text-2xl text-gray-700 mb-4 max-w-3xl mx-auto font-medium leading-relaxed">
              Manage your medication remotely with precision, reliability, and ease.
            </p>
          </div>

          {/* Features Cards */}
          <div className="grid md:grid-cols-3 gap-8 mb-16">
            <div className="bg-white/80 backdrop-blur-sm rounded-2xl shadow-xl p-8 hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100">
              <div className="bg-blue-100 w-16 h-16 rounded-2xl flex items-center justify-center mb-6 mx-auto">
                <Clock className="h-8 w-8 text-blue-600" />
              </div>
              <h3 className="text-xl font-bold text-gray-900 mb-3">Automated Scheduling</h3>
              <p className="text-gray-600 text-base leading-relaxed">Set custom dispensing times for each medication</p>
            </div>
            <div className="bg-white/80 backdrop-blur-sm rounded-2xl shadow-xl p-8 hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100">
              <div className="bg-purple-100 w-16 h-16 rounded-2xl flex items-center justify-center mb-6 mx-auto">
                <Wifi className="h-8 w-8 text-purple-600" />
              </div>
              <h3 className="text-xl font-bold text-gray-900 mb-3">Real-time Monitoring</h3>
              <p className="text-gray-600 text-base leading-relaxed">Track device status and medication levels</p>
            </div>
            <div className="bg-white/80 backdrop-blur-sm rounded-2xl shadow-xl p-8 hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100">
              <div className="bg-pink-100 w-16 h-16 rounded-2xl flex items-center justify-center mb-6 mx-auto">
                <CheckCircle2 className="h-8 w-8 text-pink-600" />
              </div>
              <h3 className="text-xl font-bold text-gray-900 mb-3">Secure & Reliable</h3>
              <p className="text-gray-600 text-base leading-relaxed">Cloud-based security with Firebase</p>
            </div>
          </div>

          {/* CTA Card */}
          <div className="bg-white/90 backdrop-blur-lg rounded-3xl shadow-2xl p-12 border border-gray-200 animate-scale-in">
            <h2 className="text-4xl font-bold text-gray-900 mb-6">
              Get Started Today
            </h2>
            <p className="text-gray-600 mb-10 text-lg leading-relaxed max-w-2xl mx-auto">
              Sign in to access your personalized pill dispenser dashboard.
            </p>
            <div className="flex flex-col sm:flex-row gap-5 justify-center max-w-lg mx-auto">
              <a
                href="/login"
                className="inline-flex items-center justify-center bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-bold py-5 px-10 rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl hover:scale-105"
              >
                Sign In
              </a>
              <a
                href="/register"
                className="inline-flex items-center justify-center bg-gradient-to-r from-emerald-500 to-teal-600 hover:from-emerald-600 hover:to-teal-700 text-white font-bold py-5 px-10 rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl hover:scale-105"
              >
                Create Account
              </a>
            </div>
          </div>
        </div>
      </div>
    </div>
  );

  return (
    <div className="min-h-screen bg-gradient-to-br from-purple-50 to-pink-100">
      <div className="container mx-auto px-4 py-16">
        <div className="max-w-2xl mx-auto text-center">
          <h1 className="text-4xl font-bold text-gray-900 mb-4">
            Pill Dispenser Dashboard
          </h1>
          <p className="text-lg text-gray-600 mb-8">
            Manage your pill dispenser remotely and efficiently.
          </p>

          <div className="bg-white rounded-xl shadow-lg p-8">
            <h2 className="text-2xl font-semibold text-gray-900 mb-4">
              Get Started
            </h2>
            <p className="text-gray-600 mb-6">
              Sign in to access your pill dispenser dashboard.
            </p>
            <div className="space-x-4">
              <a
                href="/login"
                className="bg-indigo-600 hover:bg-indigo-700 text-white font-medium py-2 px-4 rounded-md transition-colors inline-block"
              >
                Sign In
              </a>
              <a
                href="/register"
                className="bg-emerald-600 hover:bg-emerald-700 text-white font-medium py-2 px-4 rounded-md transition-colors inline-block"
              >
                Sign Up
              </a>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
