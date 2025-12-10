'use client';

import { useState } from 'react';
import { signInWithEmailAndPassword } from 'firebase/auth';
import { auth } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { Eye, EyeOff, Mail, Lock } from 'lucide-react';

export const dynamic = 'force-dynamic';

export default function Login() {
  const [email, setEmail] = useState('');
  const [password, setPassword] = useState('');
  const [showPassword, setShowPassword] = useState(false);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const router = useRouter();

  if (!auth) {
    return (
      <div className="min-h-screen flex items-center justify-center bg-gradient-to-br from-blue-50 to-indigo-100">
        <div className="max-w-md w-full space-y-8 p-8 bg-white rounded-xl shadow-lg text-center">
          <h2 className="text-3xl font-bold text-gray-900">Setup Required</h2>
          <p className="mt-2 text-sm text-gray-600">
            Please set up your Firebase configuration in .env.local to use authentication.
          </p>
        </div>
      </div>
    );
  }

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);
    setError('');
    try {
      await signInWithEmailAndPassword(auth, email, password);
      router.push('/dashboard');
    } catch (err: any) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen flex items-center justify-center bg-gradient-to-br from-blue-50 via-indigo-50 to-purple-100 px-2 sm:px-8 md:px-16 lg:px-32 xl:px-64">
      <div className="max-w-md w-full space-y-8 p-12 bg-white/95 backdrop-blur-lg rounded-2xl shadow-2xl border border-gray-100 card">
        {/* Header */}
        <div className="text-center animate-slide-in-down">
          <div className="inline-flex items-center justify-center mb-4">
            <div className="bg-gradient-to-br from-blue-500 to-purple-600 p-4 rounded-2xl shadow-lg flex items-center justify-center">
              <Lock className="h-10 w-10 text-white" />
            </div>
          </div>
          <h2 className="text-4xl font-extrabold bg-gradient-to-r from-blue-600 to-purple-600 bg-clip-text text-transparent">Welcome Back</h2>
          <p className="mt-3 text-base text-gray-600 font-medium">Sign in to your Pill Dispenser account</p>
        </div>
        
        <form className="mt-8 space-y-6 animate-scale-in" suppressHydrationWarning onSubmit={handleLogin}>
          {error && (
            <div className="bg-red-50 border-l-4 border-red-500 p-4 rounded-lg animate-slide-in-down">
              <p className="text-red-700 text-sm font-medium">{error}</p>
            </div>
          )}
          
          <div className="space-y-7">
            <div className="relative group mt-2 mb-6">
              <label className="block text-sm font-semibold text-gray-700 mb-3">Email Address</label>
              <Mail className="absolute left-4 top-12 h-6 w-6 text-gray-400 group-focus-within:text-blue-500 transition-colors" />
              <input
                type="email"
                required
                className="w-full pl-14 pr-4 py-4 border-2 border-gray-200 rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-all duration-300 bg-gray-50 focus:bg-white font-medium mt-1"
                placeholder="you@example.com"
                value={email}
                onChange={(e) => setEmail(e.target.value)}
              />
            </div>
            <div className="relative group mt-2 mb-6">
              <label className="block text-sm font-semibold text-gray-700 mb-3">Password</label>
              <Lock className="absolute left-4 top-12 h-6 w-6 text-gray-400 group-focus-within:text-blue-500 transition-colors" />
              <input
                type={showPassword ? 'text' : 'password'}
                required
                className="w-full pl-14 pr-14 py-4 border-2 border-gray-200 rounded-xl focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-all duration-300 bg-gray-50 focus:bg-white font-medium mt-1"
                placeholder="Enter your password"
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
          </div>
          
          <button
            type="submit"
            disabled={loading}
            className="w-full flex justify-center items-center py-4 px-4 mt-4 mb-2 border border-transparent rounded-xl shadow-lg text-base font-bold text-white bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed transition-all duration-300 hover:shadow-xl hover:scale-[1.02]"
          >
            {loading ? (
              <>
                <div className="spinner w-5 h-5 border-2 border-white border-t-transparent mr-3"></div>
                Signing In...
              </>
            ) : (
              'Sign In'
            )}
          </button>
        </form>
        
        <div className="text-center pt-6 border-t-2 border-gray-100">
          <p className="text-base text-gray-600">
            Don't have an account?{' '}
            <a href="/register" className="font-bold text-blue-600 hover:text-purple-600 transition-colors">
              Create one now
            </a>
          </p>
        </div>
      </div>
    </div>
  );
}