'use client';

import { useEffect, useState } from 'react';
import { onAuthStateChanged, User } from 'firebase/auth';
import { auth, db } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { ref, onValue, set } from 'firebase/database';
import { Clock, Plus, Trash2, Save, Pill } from 'lucide-react';

export const dynamic = 'force-dynamic';

interface Schedule {
  time: string;
  enabled: boolean;
}

interface DispenserSchedule {
  id: number;
  name: string;
  schedules: Schedule[];
}

export default function SchedulePage() {
  const [user, setUser] = useState<User | null>(null);
  const [dispensers, setDispensers] = useState<DispenserSchedule[]>([]);
  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [selectedDispenser, setSelectedDispenser] = useState<number>(1);
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

    const schedulesRef = ref(db, `schedules/${user.uid}`);
    const unsubscribe = onValue(schedulesRef, (snapshot) => {
      const data = snapshot.val();
      if (data && data.dispensers) {
        setDispensers(data.dispensers);
      } else {
        // Initialize schedules for 5 dispensers
        const defaultSchedules: DispenserSchedule[] = Array.from({ length: 5 }, (_, i) => ({
          id: i + 1,
          name: `Container ${i + 1}`,
          schedules: [
            { time: '08:00', enabled: true },
            { time: '14:00', enabled: true },
            { time: '20:00', enabled: true },
          ],
        }));
        set(schedulesRef, { dispensers: defaultSchedules });
        setDispensers(defaultSchedules);
      }
      setLoading(false);
    });

    return () => unsubscribe();
  }, [user]);

  const addSchedule = (dispenserId: number) => {
    const updatedDispensers = dispensers.map((d) => {
      if (d.id === dispenserId && d.schedules.length < 3) {
        return {
          ...d,
          schedules: [...d.schedules, { time: '12:00', enabled: true }],
        };
      }
      return d;
    });
    setDispensers(updatedDispensers);
  };

  const removeSchedule = (dispenserId: number, index: number) => {
    const updatedDispensers = dispensers.map((d) => {
      if (d.id === dispenserId) {
        return {
          ...d,
          schedules: d.schedules.filter((_, i) => i !== index),
        };
      }
      return d;
    });
    setDispensers(updatedDispensers);
  };

  const updateScheduleTime = (dispenserId: number, index: number, time: string) => {
    const updatedDispensers = dispensers.map((d) => {
      if (d.id === dispenserId) {
        const newSchedules = [...d.schedules];
        newSchedules[index] = { ...newSchedules[index], time };
        return { ...d, schedules: newSchedules };
      }
      return d;
    });
    setDispensers(updatedDispensers);
  };

  const toggleSchedule = (dispenserId: number, index: number) => {
    const updatedDispensers = dispensers.map((d) => {
      if (d.id === dispenserId) {
        const newSchedules = [...d.schedules];
        newSchedules[index] = { ...newSchedules[index], enabled: !newSchedules[index].enabled };
        return { ...d, schedules: newSchedules };
      }
      return d;
    });
    setDispensers(updatedDispensers);
  };

  const saveSchedules = async () => {
    if (!user || !db) return;

    setSaving(true);
    try {
      // Save to user's schedule path
      const schedulesRef = ref(db, `schedules/${user.uid}`);
      await set(schedulesRef, { dispensers });

      // Also save to device path for ESP32 to read
      // Format: pilldispenser/device/schedules/default_user (matches ESP32 USER_ID)
      const deviceScheduleRef = ref(db, `pilldispenser/device/schedules/default_user`);
      
      // Convert schedules to ESP32-friendly format
      const esp32Schedules: any = {};
      dispensers.forEach((dispenser) => {
        dispenser.schedules.forEach((schedule, index) => {
          if (schedule.enabled) {
            const scheduleId = `${dispenser.id}_${index}`;
            const [hours, minutes] = schedule.time.split(':');
            esp32Schedules[scheduleId] = {
              dispenserId: dispenser.id - 1, // ESP32 uses 0-based index
              hour: parseInt(hours),
              minute: parseInt(minutes),
              enabled: schedule.enabled,
              medicationName: `Container ${dispenser.id} Medication`,
              patientName: user.email || 'Patient',
              pillSize: 'medium',
            };
          }
        });
      });

      await set(deviceScheduleRef, esp32Schedules);
      
      setSaving(false);
      alert('Schedules saved successfully!');
    } catch (error) {
      console.error('Error saving schedules:', error);
      setSaving(false);
      alert('Failed to save schedules. Please try again.');
    }
  };

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-center animate-fade-in">
          <div className="spinner mx-auto mb-4"></div>
          <p className="text-gray-600">Loading schedules...</p>
        </div>
      </div>
    );
  }

  if (!user) {
    return null;
  }

  const currentDispenser = dispensers.find((d) => d.id === selectedDispenser);

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-purple-50 to-pink-50 px-2 sm:px-8 md:px-16 lg:px-32 xl:px-64">
      <div className="max-w-4xl mx-auto px-6 sm:px-12 lg:px-20 py-12">
        {/* Header */}
        <div className="mb-10 animate-slide-in-down">
          <h1 className="text-4xl md:text-5xl font-extrabold bg-gradient-to-r from-blue-600 via-purple-600 to-pink-600 bg-clip-text text-transparent mb-3">
            Medication Schedule 📅
          </h1>
          <p className="text-gray-600 text-lg font-medium">
            Set up to 3 daily schedules for each container
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* Dispenser Selection */}
          <div className="lg:col-span-1">
            <div className="bg-white/90 backdrop-blur-sm rounded-2xl shadow-xl p-6 border border-gray-100 animate-slide-in-left">
              <h2 className="text-xl font-bold text-gray-900 mb-5">Containers</h2>
              <div className="space-y-3">
                {dispensers.map((dispenser) => (
                  <button
                    key={dispenser.id}
                    onClick={() => setSelectedDispenser(dispenser.id)}
                    className={`w-full flex items-center justify-between p-4 rounded-xl transition-all duration-300 font-semibold shadow-md ${
                      selectedDispenser === dispenser.id
                        ? 'bg-gradient-to-r from-blue-500 via-purple-500 to-purple-600 text-white shadow-lg scale-105'
                        : 'bg-gray-50 text-gray-700 hover:bg-gray-100 hover:shadow-lg hover:scale-[1.02]'
                    }`}
                  >
                    <div className="flex items-center space-x-3">
                      <Pill className="h-6 w-6" />
                      <span className="font-bold">{dispenser.name}</span>
                    </div>
                    <span className={`text-sm px-3 py-1 rounded-lg font-bold ${
                      selectedDispenser === dispenser.id 
                        ? 'bg-white/20 text-white' 
                        : 'bg-blue-100 text-blue-600'
                    }`}>
                      {dispenser.schedules.filter((s) => s.enabled).length} active
                    </span>
                  </button>
                ))}
              </div>
            </div>
          </div>

          {/* Schedule Editor */}
          <div className="lg:col-span-2">
            <div className="bg-white/90 backdrop-blur-sm rounded-2xl shadow-xl p-6 border border-gray-100 animate-slide-in-right">
              <div className="flex items-center justify-between mb-6">
                <h2 className="text-xl font-bold text-gray-900">
                  {currentDispenser?.name} Schedule
                </h2>
                <button
                  onClick={() => addSchedule(selectedDispenser)}
                  disabled={!currentDispenser || currentDispenser.schedules.length >= 3}
                  className="flex items-center space-x-2 px-5 py-3 bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl disabled:opacity-50 disabled:cursor-not-allowed hover:scale-105"
                >
                  <Plus className="h-5 w-5" />
                  <span>Add Schedule</span>
                </button>
              </div>

              {/* Schedules List */}
              {currentDispenser && currentDispenser.schedules.length > 0 ? (
                <div className="space-y-5 mb-6">
                  {currentDispenser.schedules.map((schedule, index) => (
                    <div
                      key={index}
                      className={`p-5 rounded-2xl border-2 transition-all duration-300 shadow-md hover:shadow-xl ${
                        schedule.enabled
                          ? 'border-green-300 bg-gradient-to-r from-green-50 to-emerald-50'
                          : 'border-gray-300 bg-gray-50'
                      }`}
                    >
                      <div className="flex items-center justify-between">
                        <div className="flex items-center space-x-4 flex-1">
                          <div className="flex items-center space-x-3">
                            <Clock className={`h-6 w-6 ${
                              schedule.enabled ? 'text-green-600' : 'text-gray-400'
                            }`} />
                            <input
                              type="time"
                              value={schedule.time}
                              onChange={(e) =>
                                updateScheduleTime(selectedDispenser, index, e.target.value)
                              }
                              className="px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent font-bold text-gray-900 transition-all duration-300"
                            />
                          </div>

                          <label className="flex items-center space-x-3 cursor-pointer bg-white px-4 py-3 rounded-xl border-2 border-gray-200 hover:border-blue-300 transition-all duration-300">
                            <input
                              type="checkbox"
                              checked={schedule.enabled}
                              onChange={() => toggleSchedule(selectedDispenser, index)}
                              className="w-5 h-5 text-blue-600 border-gray-300 rounded focus:ring-blue-500"
                            />
                            <span className={`font-bold ${
                              schedule.enabled ? 'text-green-600' : 'text-gray-500'
                            }`}>
                              {schedule.enabled ? 'Enabled' : 'Disabled'}
                            </span>
                          </label>
                        </div>

                        <button
                          onClick={() => removeSchedule(selectedDispenser, index)}
                          className="p-3 text-red-600 hover:text-white hover:bg-red-500 rounded-xl transition-all duration-300 border-2 border-red-200 hover:border-red-500 shadow-sm hover:shadow-md"
                        >
                          <Trash2 className="h-5 w-5" />
                        </button>
                      </div>
                    </div>
                  ))}
                </div>
              ) : (
                <div className="text-center py-16">
                  <Clock className="h-20 w-20 text-gray-300 mx-auto mb-4" />
                  <h3 className="text-xl font-bold text-gray-900 mb-2">No Schedules Yet</h3>
                  <p className="text-gray-600 text-lg mb-6">Add a schedule to get started</p>
                  <button
                    onClick={() => addSchedule(selectedDispenser)}
                    className="inline-flex items-center px-6 py-3 bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl hover:scale-105"
                  >
                    <Plus className="h-5 w-5 mr-2" />
                    Add First Schedule
                  </button>
                </div>
              )}

              {/* Save Button */}
              <div className="flex items-center justify-end space-x-3 pt-6 border-t-2 border-gray-100">
                <button
                  onClick={saveSchedules}
                  disabled={saving}
                  className="flex items-center space-x-2 px-6 py-3.5 bg-gradient-to-r from-green-600 to-emerald-600 hover:from-green-700 hover:to-emerald-700 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl disabled:opacity-50 disabled:cursor-not-allowed hover:scale-105"
                >
                  {saving ? (
                    <>
                      <div className="spinner w-5 h-5 border-2 border-white border-t-transparent"></div>
                      <span>Saving...</span>
                    </>
                  ) : (
                    <>
                      <Save className="h-5 w-5" />
                      <span>Save Changes</span>
                    </>
                  )}
                </button>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
