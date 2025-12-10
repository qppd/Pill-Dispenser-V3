'use client';

import { useState } from 'react';
import { createUserWithEmailAndPassword } from 'firebase/auth';
import { auth } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { Eye, EyeOff, Mail, Lock, User } from 'lucide-react';

export const dynamic = 'force-dynamic';

export default function Register() {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [confirmPassword, setConfirmPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [showConfirmPassword, setShowConfirmPassword] = useState(false);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const router = useRouter();

  if (!auth) {
    return (
      <div className="min-h-screen flex items-center justify-center bg-gradient-to-br from-green-50 to-emerald-100">
        <div className="max-w-md w-full space-y-8 p-8 bg-white rounded-xl shadow-lg text-center">
          <h2 className="text-3xl font-bold text-gray-900">Setup Required</h2>
          <p className="mt-2 text-sm text-gray-600">
            Please set up your Firebase configuration in .env.local to use authentication.
          </p>
        </div>
      </div>
    );
  }

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();
    if (password !== confirmPassword) {
      setError('Passwords do not match');
      return;
    }
    setLoading(true);
    setError('');
    try {
      await createUserWithEmailAndPassword(auth, email, password);
      router.push('/dashboard');
    } catch (err: any) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center bg-gradient-to-br from-emerald-50 via-teal-50 to-green-100 px-2 sm:px-8 md:px-16 lg:px-32 xl:px-64">
      <div className="max-w-md w-full space-y-8 p-12 bg-white/95 backdrop-blur-lg rounded-2xl shadow-2xl border border-gray-100 card">
        {/* Header */}
        <div className="text-center animate-slide-in-down">
          <div className="inline-flex items-center justify-center mb-4">
            <div className="bg-gradient-to-br from-emerald-500 to-teal-600 p-4 rounded-2xl shadow-lg flex items-center justify-center">
              <User className="h-10 w-10 text-white" />
            </div>
          </div>
          <h2 className="text-4xl font-extrabold bg-gradient-to-r from-emerald-600 to-teal-600 bg-clip-text text-transparent">Create Account</h2>
          <p className="mt-3 text-base text-gray-600 font-medium">Join Pill Dispenser today</p>
        </div>
        
        <form className="mt-8 space-y-6 animate-scale-in" suppressHydrationWarning onSubmit={handleRegister}>
          {error && (
            <div className="bg-red-50 border-l-4 border-red-500 p-4 rounded-lg animate-slide-in-down">
              <p className="text-red-700 text-sm font-medium">{error}</p>
            </div>
          )}
          
          <div className="space-y-7">
            <div className="relative group mt-2 mb-6">
              <label className="block text-sm font-semibold text-gray-700 mb-3">Email Address</label>
              <Mail className="absolute left-4 top-12 h-6 w-6 text-gray-400 group-focus-within:text-emerald-500 transition-colors" />
              <input
                type="email"
                required
                className="w-full pl-14 pr-4 py-4 border-2 border-gray-200 rounded-xl focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:border-transparent transition-all duration-300 bg-gray-50 focus:bg-white font-medium mt-1"
                placeholder="you@example.com"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
              />
            </div>
            <div className="relative group mt-2 mb-6">
              <label className="block text-sm font-semibold text-gray-700 mb-3">Password</label>
              <Lock className="absolute left-4 top-12 h-6 w-6 text-gray-400 group-focus-within:text-emerald-500 transition-colors" />
              <input
                type={showPassword ? 'text' : 'password'}
                required
                className="w-full pl-14 pr-14 py-4 border-2 border-gray-200 rounded-xl focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:border-transparent transition-all duration-300 bg-gray-50 focus:bg-white font-medium mt-1"
                placeholder="Create a password"
                value={password}
                onChange={(e) => setPassword(e.target.value)}
              />
              <button
                type="button"
                className="absolute right-4 top-12 text-gray-400 hover:text-gray-600 transition-colors"
                onClick={() => setShowPassword(!showPassword)}
              >
                {showPassword ? <EyeOff className="h-6 w-6" /> : <Eye className="h-6 w-6" />}
              </button>
            </div>
            <div className="relative group mt-2 mb-6">
              <label className="block text-sm font-semibold text-gray-700 mb-3">Confirm Password</label>
              <Lock className="absolute left-4 top-12 h-6 w-6 text-gray-400 group-focus-within:text-emerald-500 transition-colors" />
              <input
                type={showConfirmPassword ? 'text' : 'password'}
                required
                className="w-full pl-14 pr-14 py-4 border-2 border-gray-200 rounded-xl focus:outline-none focus:ring-2 focus:ring-emerald-500 focus:border-transparent transition-all duration-300 bg-gray-50 focus:bg-white font-medium mt-1"
                placeholder="Confirm your password"
                value={confirmPassword}
                onChange={(e) => setConfirmPassword(e.target.value)}
              />
              <button
                type="button"
                className="absolute right-4 top-12 text-gray-400 hover:text-gray-600 transition-colors"
                onClick={() => setShowConfirmPassword(!showConfirmPassword)}
              >
                {showConfirmPassword ? <EyeOff className="h-6 w-6" /> : <Eye className="h-6 w-6" />}
              </button>
            </div>
          </div>
          
          <button
            type="submit"
            disabled={loading}
            className="w-full flex justify-center items-center py-4 px-4 mt-4 mb-2 border border-transparent rounded-xl shadow-lg text-base font-bold text-white bg-gradient-to-r from-emerald-600 to-teal-600 hover:from-emerald-700 hover:to-teal-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-emerald-500 disabled:opacity-50 disabled:cursor-not-allowed transition-all duration-300 hover:shadow-xl hover:scale-[1.02]"
          >
            {loading ? (
              <>
                <div className="spinner w-5 h-5 border-2 border-white border-t-transparent mr-3"></div>
                Creating Account...
              </>
            ) : (
              'Create Account'
            )}
          </button>
        </form>
        
        <div className="text-center pt-6 border-t-2 border-gray-100">
          <p className="text-base text-gray-600">
            Already have an account?{' '}
            <a href="/login" className="font-bold text-emerald-600 hover:text-teal-600 transition-colors">
              Sign in here
            </a>
          </p>
        </div>
      </div>
    </div>
  );
}