'use client';

import { useEffect, useState } from 'react';
import { onAuthStateChanged, User } from 'firebase/auth';
import { auth, db } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { ref, onValue, set } from 'firebase/database';
import { Pill, Battery, Wifi, WifiOff, AlertCircle, CheckCircle2, Clock } from 'lucide-react';

export const dynamic = 'force-dynamic';

interface DispenserData {
  id: number;
  name: string;
  pillsRemaining: number;
  lastDispensed: string;
  nextDose: string;
}

interface DeviceStatus {
  battery: number;
  status: 'online' | 'offline';
}

export default function Dashboard() {
  const [user, setUser] = useState<User | null>(null);
  const [dispensers, setDispensers] = useState<DispenserData[]>([]);
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus>({ battery: 85, status: 'offline' });
  const [loading, setLoading] = useState(true);
  const router = useRouter();

  useEffect(() => {
    if (!auth) {
      setLoading(false);
      return;
    }

    const unsubscribeAuth = onAuthStateChanged(auth, (user) => {
      if (!user) {
        router.push('/login');
        return;
      }
      setUser(user);
    });

    return () => unsubscribeAuth();
  }, [router]);

  useEffect(() => {
    if (!user || !db) return;

    const dispensersRef = ref(db, `dispensers/${user.uid}/containers`);
    const deviceRef = ref(db, `dispensers/${user.uid}/device`);

    const unsubscribeContainers = onValue(dispensersRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        setDispensers(data);
      } else {
        // Initialize 5 dispensers
        const defaultDispensers: DispenserData[] = Array.from({ length: 5 }, (_, i) => ({
          id: i + 1,
          name: `Container ${i + 1}`,
          pillsRemaining: 30,
          lastDispensed: 'Never',
          nextDose: '08:00',
        }));
        set(dispensersRef, defaultDispensers);
        setDispensers(defaultDispensers);
      }
      setLoading(false);
    });

    const unsubscribeDevice = onValue(deviceRef, (snapshot) => {
      const data = snapshot.val();
      if (data) {
        setDeviceStatus(data);
      } else {
        const defaultDevice: DeviceStatus = { battery: 85, status: 'offline' };
        set(deviceRef, defaultDevice);
        setDeviceStatus(defaultDevice);
      }
    });

    return () => {
      unsubscribeContainers();
      unsubscribeDevice();
    };
  }, [user]);

  const dispenseNow = (dispenserId: number) => {
    if (!user || !db) return;

    const now = new Date().toLocaleString();
    const updatedDispensers = dispensers.map((d) =>
      d.id === dispenserId
        ? {
            ...d,
            lastDispensed: now,
            pillsRemaining: Math.max(0, d.pillsRemaining - 1),
          }
        : d
    );

    const dispensersRef = ref(db, `dispensers/${user.uid}/containers`);
    set(dispensersRef, updatedDispensers);
  };

  const getBatteryColor = (battery: number) => {
    if (battery >= 70) return 'text-green-500';
    if (battery >= 30) return 'text-yellow-500';
    return 'text-red-500';
  };

  const getPillsColor = (pills: number) => {
    if (pills >= 20) return 'text-green-500';
    if (pills >= 10) return 'text-yellow-500';
    return 'text-red-500';
  };

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-center animate-fade-in">
          <div className="spinner mx-auto mb-4"></div>
          <p className="text-gray-600">Loading dashboard...</p>
        </div>
      </div>
    );
  }

  if (!user) {
    return null;
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-white to-purple-50">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
      <div className="max-w-7xl mx-auto">
        {/* Header */}
        <div className="mb-8 animate-slide-in-down">
          <h1 className="text-3xl font-bold text-gray-900 mb-2">
            Welcome back, {user.email?.split('@')[0]}! 👋
          </h1>
          <p className="text-gray-600">
            Monitor and manage your pill dispensers
          </p>
        </div>

        {/* Stats Overview */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
          <div className="card animate-scale-in" style={{ animationDelay: '0.1s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm text-gray-600 mb-1">Device Status</p>
                <p className={`text-3xl font-bold ${deviceStatus.status === 'online' ? 'text-green-600' : 'text-gray-400'}`}>
                  {deviceStatus.status === 'online' ? 'Online' : 'Offline'}
                </p>
              </div>
              <div className={`${deviceStatus.status === 'online' ? 'bg-green-100' : 'bg-gray-100'} p-3 rounded-xl`}>
                {deviceStatus.status === 'online' ? (
                  <Wifi className="h-8 w-8 text-green-600" />
                ) : (
                  <WifiOff className="h-8 w-8 text-gray-400" />
                )}
              </div>
            </div>
          </div>

          <div className="card animate-scale-in" style={{ animationDelay: '0.2s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm text-gray-600 mb-1">Battery Level</p>
                <p className={`text-3xl font-bold ${getBatteryColor(deviceStatus.battery)}`}>
                  {deviceStatus.battery}%
                </p>
              </div>
              <div className={`${deviceStatus.battery >= 70 ? 'bg-green-100' : deviceStatus.battery >= 30 ? 'bg-yellow-100' : 'bg-red-100'} p-3 rounded-xl`}>
                <Battery className={`h-8 w-8 ${getBatteryColor(deviceStatus.battery)}`} />
              </div>
            </div>
          </div>

          <div className="card animate-scale-in" style={{ animationDelay: '0.3s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm text-gray-600 mb-1">Low Pills</p>
                <p className="text-3xl font-bold text-orange-600">
                  {dispensers.filter((d) => d.pillsRemaining < 10).length}
                </p>
              </div>
              <div className="bg-orange-100 p-3 rounded-xl">
                <AlertCircle className="h-8 w-8 text-orange-600" />
              </div>
            </div>
          </div>
        </div>

        {/* Dispensers Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {dispensers.map((dispenser, index) => (
            <div
              key={dispenser.id}
              className="card animate-slide-in-up"
              style={{ animationDelay: `${0.1 * (index + 1)}s` }}
            >
              {/* Header */}
              <div className="flex items-center justify-between mb-4">
                <div className="flex items-center space-x-3">
                  <div className="bg-gradient-to-br from-blue-500 to-purple-600 p-2 rounded-lg">
                    <Pill className="h-5 w-5 text-white" />
                  </div>
                  <div>
                    <h3 className="font-semibold text-gray-900">{dispenser.name}</h3>
                    <p className="text-xs text-gray-500">ID: {dispenser.id}</p>
                  </div>
                </div>
              </div>

              {/* Stats */}
              <div className="space-y-3 mb-4">
                <div className="flex items-center justify-between">
                  <div className="flex items-center space-x-2 text-sm text-gray-600">
                    <Pill className={`h-4 w-4 ${getPillsColor(dispenser.pillsRemaining)}`} />
                    <span>Pills Remaining</span>
                  </div>
                  <span className={`font-semibold ${getPillsColor(dispenser.pillsRemaining)}`}>
                    {dispenser.pillsRemaining}
                  </span>
                </div>

                <div className="flex items-center justify-between">
                  <div className="flex items-center space-x-2 text-sm text-gray-600">
                    <Clock className="h-4 w-4" />
                    <span>Next Dose</span>
                  </div>
                  <span className="font-semibold text-gray-900">{dispenser.nextDose}</span>
                </div>
              </div>

              {/* Last Dispensed */}
              <div className="border-t border-gray-100 pt-3 mb-4">
                <p className="text-xs text-gray-500 mb-1">Last Dispensed</p>
                <p className="text-sm font-medium text-gray-700">{dispenser.lastDispensed}</p>
              </div>

              {/* Action Button */}
              <button
                onClick={() => dispenseNow(dispenser.id)}
                className="btn btn-primary w-full"
                disabled={deviceStatus.status === 'offline'}
              >
                Dispense Now
              </button>
            </div>
          ))}
        </div>
      </div>
      </div>
    </div>
  );
}
