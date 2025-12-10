'use client';

import { useEffect, useState } from 'react';
import { onAuthStateChanged, User } from 'firebase/auth';
import { auth, db } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { ref, onValue, set, get } from 'firebase/database';
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
    // Read from ESP32 device path - scan all devices under pilldispenser/device/
    const allDevicesRef = ref(db, 'pilldispenser/device');

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

    const unsubscribeDevice = onValue(allDevicesRef, (snapshot) => {
      const devices = snapshot.val();
      
      if (devices) {
        // Get the first device (or you can let user select device later)
        const deviceIds = Object.keys(devices);
        if (deviceIds.length > 0) {
          const firstDeviceId = deviceIds[0];
          const deviceData = devices[firstDeviceId];
          
          // Extract battery data from sensors
          let batteryPercentage = 0;
          if (deviceData.sensors && deviceData.sensors.battery_percentage) {
            batteryPercentage = parseFloat(deviceData.sensors.battery_percentage.value) || 0;
          }
          
          // Extract online status from heartbeat or status
          let isOnline = false;
          if (deviceData.heartbeat && deviceData.heartbeat.device_status === 'online') {
            isOnline = true;
          } else if (deviceData.status && deviceData.status.ip_address) {
            // If we have status data with IP, consider it online
            isOnline = true;
          }
          
          setDeviceStatus({
            battery: Math.round(batteryPercentage),
            status: isOnline ? 'online' : 'offline'
          });
        }
      }
    });

    return () => {
      unsubscribeContainers();
      unsubscribeDevice();
    };
  }, [user]);

  const dispenseNow = async (dispenserId: number) => {
    if (!user || !db) return;

    try {
      // Send command to ESP32
      const devices = await get(ref(db, 'pilldispenser/device'));
      if (devices.exists()) {
        const deviceIds = Object.keys(devices.val());
        if (deviceIds.length > 0) {
          const firstDeviceId = deviceIds[0];
          const commandRef = ref(db, `pilldispenser/device/${firstDeviceId}/commands`);
          
          // Send dispense command
          await set(commandRef, `DISPENSE:${dispenserId}`);
          
          // Clear the command after a short delay to allow ESP32 to process it
          setTimeout(async () => {
            try {
              await set(commandRef, null);
            } catch (error) {
              console.error('Error clearing command:', error);
            }
          }, 1000);
          
          // Update local UI state
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
          await set(dispensersRef, updatedDispensers);
        }
      }
    } catch (error) {
      console.error('Error sending dispense command:', error);
    }
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
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-purple-50 to-pink-50 py-12">
      <div className="max-w-7xl mx-auto px-12 sm:px-16 lg:px-20">
        {/* Header */}
        <div className="mb-16 animate-slide-in-down">
          <h1 className="text-4xl md:text-5xl font-extrabold bg-gradient-to-r from-blue-600 via-purple-600 to-pink-600 bg-clip-text text-transparent mb-6">
            Welcome back, {user.email?.split('@')[0]}! 👋
          </h1>
          <p className="text-gray-600 text-lg font-medium">
            Monitor and manage your pill dispensers in real-time
          </p>
        </div>

        {/* Stats Overview */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-8 mb-16">
          <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-10 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100 animate-scale-in" style={{ animationDelay: '0.1s' }}>
            <div className="flex items-center justify-between px-2">
              <div className="pr-4">
                <p className="text-sm font-semibold text-gray-500 mb-3 uppercase tracking-wide">Device Status</p>
                <p className={`text-4xl font-extrabold ${
                  deviceStatus.status === 'online' 
                    ? 'bg-gradient-to-r from-green-600 to-emerald-600 bg-clip-text text-transparent' 
                    : 'text-gray-400'
                }`}>
                  {deviceStatus.status === 'online' ? 'Online' : 'Offline'}
                </p>
              </div>
              <div className={`${
                deviceStatus.status === 'online' 
                  ? 'bg-gradient-to-br from-green-400 to-emerald-500' 
                  : 'bg-gray-200'
              } p-6 rounded-2xl shadow-lg flex items-center justify-center ml-4`}>
                {deviceStatus.status === 'online' ? (
                  <Wifi className="h-12 w-12 text-white" />
                ) : (
                  <WifiOff className="h-12 w-12 text-gray-400" />
                )}
              </div>
            </div>
          </div>

          <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-10 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100 animate-scale-in" style={{ animationDelay: '0.2s' }}>
            <div className="flex items-center justify-between px-2">
              <div className="pr-4">
                <p className="text-sm font-semibold text-gray-500 mb-3 uppercase tracking-wide">Battery Level</p>
                <p className={`text-4xl font-extrabold ${
                  deviceStatus.battery >= 70 
                    ? 'bg-gradient-to-r from-green-600 to-emerald-600 bg-clip-text text-transparent' 
                    : deviceStatus.battery >= 30 
                      ? 'bg-gradient-to-r from-yellow-600 to-orange-600 bg-clip-text text-transparent' 
                      : 'bg-gradient-to-r from-red-600 to-pink-600 bg-clip-text text-transparent'
                }`}>
                  {deviceStatus.battery}%
                </p>
              </div>
              <div className={`${
                deviceStatus.battery >= 70 
                  ? 'bg-gradient-to-br from-green-400 to-emerald-500' 
                  : deviceStatus.battery >= 30 
                    ? 'bg-gradient-to-br from-yellow-400 to-orange-500' 
                    : 'bg-gradient-to-br from-red-400 to-pink-500'
              } p-6 rounded-2xl shadow-lg flex items-center justify-center ml-4`}>
                <Battery className="h-12 w-12 text-white" />
              </div>
            </div>
          </div>

          <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-10 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100 animate-scale-in" style={{ animationDelay: '0.3s' }}>
            <div className="flex items-center justify-between px-2">
              <div className="pr-4">
                <p className="text-sm font-semibold text-gray-500 mb-3 uppercase tracking-wide">Low Pills Alert</p>
                <p className="text-4xl font-extrabold bg-gradient-to-r from-orange-600 to-red-600 bg-clip-text text-transparent">
                  {dispensers.filter((d) => d.pillsRemaining < 10).length}
                </p>
              </div>
              <div className="bg-gradient-to-br from-orange-400 to-red-500 p-6 rounded-2xl shadow-lg flex items-center justify-center ml-4">
                <AlertCircle className="h-12 w-12 text-white" />
              </div>
            </div>
          </div>
        </div>

        {/* Dispensers Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-10">
          {dispensers.map((dispenser, index) => (
            <div
              key={dispenser.id}
              className="bg-white/90 backdrop-blur-sm rounded-2xl p-10 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-[1.02] border border-gray-100 animate-slide-in-up"
              style={{ animationDelay: `${0.1 * (index + 1)}s` }}
            >
              <div className="space-y-8 px-2">
              {/* Header */}
              <div className="flex items-center justify-between mb-8">
                <div className="flex items-center space-x-6">
                  <div className="bg-gradient-to-br from-blue-500 via-purple-500 to-purple-600 p-5 rounded-2xl shadow-lg flex items-center justify-center">
                    <Pill className="h-8 w-8 text-white" />
                  </div>
                  <div className="pr-4">
                    <h3 className="font-bold text-gray-900 text-xl mb-2">{dispenser.name}</h3>
                    <p className="text-xs text-gray-500 font-semibold">ID: {dispenser.id}</p>
                  </div>
                </div>
              </div>

              {/* Stats */}
              <div className="space-y-5 mb-8">
                <div className="flex items-center justify-between p-5 bg-gray-50 rounded-xl">
                  <div className="flex items-center space-x-5 text-sm text-gray-600 font-medium pr-4">
                    <Pill className={`h-5 w-5 ${
                      dispenser.pillsRemaining >= 20 
                        ? 'text-green-500' 
                        : dispenser.pillsRemaining >= 10 
                          ? 'text-yellow-500' 
                          : 'text-red-500'
                    }`} />
                    <span>Pills Remaining</span>
                  </div>
                  <span className={`font-bold text-lg ml-4 ${
                    dispenser.pillsRemaining >= 20 
                      ? 'text-green-600' 
                      : dispenser.pillsRemaining >= 10 
                        ? 'text-yellow-600' 
                        : 'text-red-600'
                  }`}>
                    {dispenser.pillsRemaining}
                  </span>
                </div>

                <div className="flex items-center justify-between p-5 bg-gray-50 rounded-xl">
                  <div className="flex items-center space-x-5 text-sm text-gray-600 font-medium pr-4">
                    <Clock className="h-5 w-5 text-blue-500" />
                    <span>Next Dose</span>
                  </div>
                  <span className="font-bold text-gray-900 text-lg ml-4">{dispenser.nextDose}</span>
                </div>
              </div>

              {/* Last Dispensed */}
              <div className="border-t-2 border-gray-100 pt-8 mb-8 px-2">
                <p className="text-xs text-gray-500 mb-3 font-semibold uppercase tracking-wide">Last Dispensed</p>
                <p className="text-sm font-bold text-gray-700">{dispenser.lastDispensed}</p>
              </div>

              {/* Action Button */}
              <div className="px-2">
                <button
                  onClick={() => dispenseNow(dispenser.id)}
                  disabled={deviceStatus.status === 'offline'}
                  className={`w-full py-5 px-10 rounded-xl font-bold text-white transition-all duration-300 shadow-lg ${
                    deviceStatus.status === 'offline'
                      ? 'bg-gray-300 cursor-not-allowed'
                      : 'bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 hover:shadow-xl hover:scale-[1.02]'
                  }`}
                >
                  {deviceStatus.status === 'offline' ? 'Device Offline' : 'Dispense Now'}
                </button>
              </div>
              </div>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
