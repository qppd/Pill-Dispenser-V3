'use client';

import { useEffect, useState } from 'react';
import { onAuthStateChanged, User } from 'firebase/auth';
import { auth, db } from '@/lib/firebase';
import { useRouter } from 'next/navigation';
import { ref, onValue, set, push, remove } from 'firebase/database';
import { Clock, Plus, Trash2, Pill, User as UserIcon, CalendarDays } from 'lucide-react';

export const dynamic = 'force-dynamic';

interface Schedule {
  id?: string;
  dispenser_id: number;
  time: string;
  enabled: boolean;
  medication_name: string;
  patient_name: string;
  pill_size: 'small' | 'medium' | 'large';
  days: number[]; // 0-6 (Mon-Sun)
}

const DISPENSERS = [0, 1, 2, 3, 4];
const DAYS = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];

export default function NewSchedulePage() {
  const [user, setUser] = useState<User | null>(null);
  const [schedules, setSchedules] = useState<Record<string, Schedule>>({});
  const [loading, setLoading] = useState(true);
  const [selectedDispenser, setSelectedDispenser] = useState<number>(0);
  const router = useRouter();

  // Firebase device ID
  const deviceId = 'PILL_DISPENSER_DEFAULT';

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

    const schedulesRef = ref(db, `pilldispenser/device/schedules/d1SdACjSzbZBNzfhMOFhZixVEX82`);
    const unsubscribe = onValue(schedulesRef, (snapshot) => {
      const data = snapshot.val() || {};
      setSchedules(data);
      setLoading(false);
    });

    return () => unsubscribe();
  }, [user]);

  const addSchedule = () => {
    const newSchedule: Schedule = {
      dispenser_id: selectedDispenser,
      time: '08:00',
      enabled: true,
      medication_name: 'New Medication',
      patient_name: 'Patient Name',
      pill_size: 'medium',
      days: [0, 1, 2, 3, 4, 5, 6],
    };

    const schedulesRef = ref(db, `pilldispenser/device/schedules/d1SdACjSzbZBNzfhMOFhZixVEX82`);
    const newScheduleRef = push(schedulesRef);
    set(newScheduleRef, newSchedule);
  };

  const removeSchedule = (scheduleId: string) => {
    const scheduleRef = ref(db, `pilldispenser/device/schedules/d1SdACjSzbZBNzfhMOFhZixVEX82/${scheduleId}`);
    remove(scheduleRef);
  };

  const updateSchedule = (scheduleId: string, updates: Partial<Schedule>) => {
    const scheduleRef = ref(db, `pilldispenser/device/schedules/d1SdACjSzbZBNzfhMOFhZixVEX82/${scheduleId}`);
    const currentSchedule = schedules[scheduleId];
    set(scheduleRef, { ...currentSchedule, ...updates });
  };

  const toggleDay = (scheduleId: string, dayIndex: number) => {
    const schedule = schedules[scheduleId];
    const days = schedule.days || [0, 1, 2, 3, 4, 5, 6];
    const newDays = days.includes(dayIndex)
      ? days.filter((d) => d !== dayIndex)
      : [...days, dayIndex].sort();
    
    updateSchedule(scheduleId, { days: newDays });
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

  const dispenserSchedules = Object.entries(schedules).filter(
    ([_, schedule]) => schedule.dispenser_id === selectedDispenser
  );

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-purple-50 to-pink-50 px-2 sm:px-8 md:px-16 lg:px-32 xl:px-64">
      <div className="max-w-6xl mx-auto px-6 sm:px-12 lg:px-20 py-12">
        <div className="mb-10 animate-slide-in-down">
          <h1 className="text-4xl md:text-5xl font-extrabold bg-gradient-to-r from-blue-600 via-purple-600 to-pink-600 bg-clip-text text-transparent mb-3">
            Medication Schedules 📅
          </h1>
          <p className="text-gray-600 text-lg font-medium">
            Configure automatic dispensing schedules for each container
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
          <div className="lg:col-span-1">
            <div className="bg-white/90 backdrop-blur-sm rounded-2xl shadow-xl p-6 border border-gray-100 animate-slide-in-left">
              <h2 className="text-xl font-bold text-gray-900 mb-5">Containers</h2>
              <div className="space-y-3">
                {DISPENSERS.map((dispenserId) => {
                  const count = Object.values(schedules).filter(
                    (s) => s.dispenser_id === dispenserId && s.enabled
                  ).length;

                  return (
                    <button
                      key={dispenserId}
                      onClick={() => setSelectedDispenser(dispenserId)}
                      className={`w-full flex items-center justify-between p-4 rounded-xl transition-all duration-300 font-semibold shadow-md ${
                        selectedDispenser === dispenserId
                          ? 'bg-gradient-to-r from-blue-500 via-purple-500 to-purple-600 text-white shadow-lg scale-105'
                          : 'bg-gray-50 text-gray-700 hover:bg-gray-100 hover:shadow-lg hover:scale-[1.02]'
                      }`}
                    >
                      <div className="flex items-center space-x-3">
                        <Pill className="h-6 w-6" />
                        <span className="font-bold">Container {dispenserId}</span>
                      </div>
                      <span
                        className={`text-sm px-3 py-1.5 rounded-lg font-bold ${
                          selectedDispenser === dispenserId
                            ? 'bg-white/20 text-white'
                            : 'bg-blue-100 text-blue-600'
                        }`}
                      >
                        {count}
                      </span>
                    </button>
                  );
                })}
              </div>
            </div>
          </div>

          <div className="lg:col-span-3">
            <div className="bg-white/90 backdrop-blur-sm rounded-2xl shadow-xl p-6 border border-gray-100 animate-slide-in-right">
              <div className="flex items-center justify-between mb-6">
                <h2 className="text-xl font-bold text-gray-900">
                  Container {selectedDispenser} Schedules
                </h2>
                <button onClick={addSchedule} className="flex items-center space-x-2 px-5 py-3 bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl hover:scale-105">
                  <Plus className="h-5 w-5" />
                  <span>Add Schedule</span>
                </button>
              </div>

              {dispenserSchedules.length > 0 ? (
                <div className="space-y-5">
                  {dispenserSchedules.map(([scheduleId, schedule]) => (
                    <div
                      key={scheduleId}
                      className={`p-6 rounded-2xl border-2 transition-all duration-300 shadow-md hover:shadow-xl ${
                        schedule.enabled
                          ? 'border-green-300 bg-gradient-to-r from-green-50 to-emerald-50'
                          : 'border-gray-300 bg-gray-50'
                      }`}
                    >
                      <div className="space-y-5">
                        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                          <div>
                            <label className="flex items-center text-sm font-bold text-gray-700 mb-2">
                              <Clock className="h-4 w-4 mr-2" />
                              Time
                            </label>
                            <input
                              type="time"
                              value={schedule.time}
                              onChange={(e) => updateSchedule(scheduleId, { time: e.target.value })}
                              className="w-full px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent font-bold text-gray-900 transition-all duration-300"
                            />
                          </div>

                          <div>
                            <label className="flex items-center text-sm font-bold text-gray-700 mb-2">
                              <UserIcon className="h-4 w-4 mr-2" />
                              Patient Name
                            </label>
                            <input
                              type="text"
                              value={schedule.patient_name}
                              onChange={(e) =>
                                updateSchedule(scheduleId, { patient_name: e.target.value })
                              }
                              className="w-full px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent font-medium text-gray-900 transition-all duration-300"
                            />
                          </div>

                          <div>
                            <label className="flex items-center text-sm font-bold text-gray-700 mb-2">
                              <Pill className="h-4 w-4 mr-2" />
                              Medication
                            </label>
                            <input
                              type="text"
                              value={schedule.medication_name}
                              onChange={(e) =>
                                updateSchedule(scheduleId, { medication_name: e.target.value })
                              }
                              className="w-full px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent font-medium text-gray-900 transition-all duration-300"
                            />
                          </div>
                        </div>

                        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                          <div>
                            <label className="block text-sm font-bold text-gray-700 mb-2">
                              Pill Size
                            </label>
                            <select
                              value={schedule.pill_size}
                              onChange={(e) =>
                                updateSchedule(scheduleId, {
                                  pill_size: e.target.value as 'small' | 'medium' | 'large',
                                })
                              }
                              className="w-full px-4 py-3 border-2 border-gray-300 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent font-bold text-gray-900 transition-all duration-300"
                            >
                              <option value="small">Small</option>
                              <option value="medium">Medium</option>
                              <option value="large">Large</option>
                            </select>
                          </div>

                          <div>
                            <label className="flex items-center text-sm font-bold text-gray-700 mb-2">
                              <CalendarDays className="h-4 w-4 mr-2" />
                              Days of Week
                            </label>
                            <div className="flex space-x-1">
                              {DAYS.map((day, index) => (
                                <button
                                  key={day}
                                  onClick={() => toggleDay(scheduleId, index)}
                                  className={`flex-1 py-3 px-1 text-xs font-bold rounded-xl transition-all duration-300 shadow-sm ${
                                    (schedule.days || []).includes(index)
                                      ? 'bg-gradient-to-r from-blue-500 to-purple-600 text-white shadow-md scale-105'
                                      : 'bg-gray-200 text-gray-600 hover:bg-gray-300'
                                  }`}
                                >
                                  {day}
                                </button>
                              ))}
                            </div>
                          </div>
                        </div>

                        <div className="flex items-center justify-between pt-4 border-t-2 border-gray-200">
                          <label className="flex items-center space-x-3 cursor-pointer bg-white px-4 py-3 rounded-xl border-2 border-gray-200 hover:border-blue-300 transition-all duration-300">
                            <input
                              type="checkbox"
                              checked={schedule.enabled}
                              onChange={(e) =>
                                updateSchedule(scheduleId, { enabled: e.target.checked })
                              }
                              className="w-5 h-5 text-blue-600 border-gray-300 rounded focus:ring-blue-500"
                            />
                            <span className="font-bold text-gray-700">
                              {schedule.enabled ? '✓ Enabled' : 'Disabled'}
                            </span>
                          </label>

                          <button
                            onClick={() => removeSchedule(scheduleId)}
                            className="p-3 text-red-600 hover:text-white hover:bg-red-500 rounded-xl transition-all duration-300 border-2 border-red-200 hover:border-red-500 shadow-sm hover:shadow-md"
                          >
                            <Trash2 className="h-5 w-5" />
                          </button>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              ) : (
                <div className="text-center py-16">
                  <Clock className="h-20 w-20 text-gray-300 mx-auto mb-4" />
                  <h3 className="text-xl font-bold text-gray-900 mb-2">No Schedules Yet</h3>
                  <p className="text-gray-600 text-lg mb-6">
                    Add a schedule to start automatic medication dispensing
                  </p>
                  <button onClick={addSchedule} className="inline-flex items-center px-6 py-3 bg-gradient-to-r from-blue-600 to-purple-600 hover:from-blue-700 hover:to-purple-700 text-white font-bold rounded-xl transition-all duration-300 shadow-lg hover:shadow-xl hover:scale-105">
                    <Plus className="h-5 w-5 mr-2" />
                    Add First Schedule
                  </button>
                </div>
              )}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
