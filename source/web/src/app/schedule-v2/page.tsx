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

    const schedulesRef = ref(db, `pilldispenser/devices/${deviceId}/schedules`);
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

    const schedulesRef = ref(db, `pilldispenser/devices/${deviceId}/schedules`);
    const newScheduleRef = push(schedulesRef);
    set(newScheduleRef, newSchedule);
  };

  const removeSchedule = (scheduleId: string) => {
    const scheduleRef = ref(db, `pilldispenser/devices/${deviceId}/schedules/${scheduleId}`);
    remove(scheduleRef);
  };

  const updateSchedule = (scheduleId: string, updates: Partial<Schedule>) => {
    const scheduleRef = ref(db, `pilldispenser/devices/${deviceId}/schedules/${scheduleId}`);
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
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-white to-purple-50">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8 animate-slide-in-down">
          <h1 className="text-3xl font-bold text-gray-900 mb-2">
            Medication Schedules 📅
          </h1>
          <p className="text-gray-600">
            Configure automatic dispensing schedules for each container
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-4 gap-6">
          <div className="lg:col-span-1">
            <div className="card animate-slide-in-left">
              <h2 className="text-lg font-semibold text-gray-900 mb-4">Containers</h2>
              <div className="space-y-2">
                {DISPENSERS.map((dispenserId) => {
                  const count = Object.values(schedules).filter(
                    (s) => s.dispenser_id === dispenserId && s.enabled
                  ).length;

                  return (
                    <button
                      key={dispenserId}
                      onClick={() => setSelectedDispenser(dispenserId)}
                      className={`w-full flex items-center justify-between p-3 rounded-lg transition-all duration-200 ${
                        selectedDispenser === dispenserId
                          ? 'bg-gradient-to-r from-blue-500 to-purple-600 text-white shadow-lg'
                          : 'bg-gray-50 text-gray-700 hover:bg-gray-100'
                      }`}
                    >
                      <div className="flex items-center space-x-3">
                        <Pill className="h-5 w-5" />
                        <span className="font-medium">Container {dispenserId}</span>
                      </div>
                      <span
                        className={`text-sm px-2 py-1 rounded ${
                          selectedDispenser === dispenserId
                            ? 'bg-white/20 text-white'
                            : 'bg-gray-200 text-gray-600'
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
            <div className="card animate-slide-in-right">
              <div className="flex items-center justify-between mb-6">
                <h2 className="text-lg font-semibold text-gray-900">
                  Container {selectedDispenser} Schedules
                </h2>
                <button onClick={addSchedule} className="btn btn-primary flex items-center space-x-2">
                  <Plus className="h-4 w-4" />
                  <span>Add Schedule</span>
                </button>
              </div>

              {dispenserSchedules.length > 0 ? (
                <div className="space-y-4">
                  {dispenserSchedules.map(([scheduleId, schedule]) => (
                    <div
                      key={scheduleId}
                      className={`p-5 rounded-lg border-2 transition-all duration-200 ${
                        schedule.enabled
                          ? 'border-green-200 bg-green-50'
                          : 'border-gray-200 bg-gray-50'
                      }`}
                    >
                      <div className="space-y-4">
                        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                          <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                              <Clock className="inline h-4 w-4 mr-1" />
                              Time
                            </label>
                            <input
                              type="time"
                              value={schedule.time}
                              onChange={(e) => updateSchedule(scheduleId, { time: e.target.value })}
                              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                            />
                          </div>

                          <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                              <UserIcon className="inline h-4 w-4 mr-1" />
                              Patient Name
                            </label>
                            <input
                              type="text"
                              value={schedule.patient_name}
                              onChange={(e) =>
                                updateSchedule(scheduleId, { patient_name: e.target.value })
                              }
                              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                            />
                          </div>

                          <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                              <Pill className="inline h-4 w-4 mr-1" />
                              Medication
                            </label>
                            <input
                              type="text"
                              value={schedule.medication_name}
                              onChange={(e) =>
                                updateSchedule(scheduleId, { medication_name: e.target.value })
                              }
                              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                            />
                          </div>
                        </div>

                        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                          <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                              Pill Size
                            </label>
                            <select
                              value={schedule.pill_size}
                              onChange={(e) =>
                                updateSchedule(scheduleId, {
                                  pill_size: e.target.value as 'small' | 'medium' | 'large',
                                })
                              }
                              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                            >
                              <option value="small">Small</option>
                              <option value="medium">Medium</option>
                              <option value="large">Large</option>
                            </select>
                          </div>

                          <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                              <CalendarDays className="inline h-4 w-4 mr-1" />
                              Days of Week
                            </label>
                            <div className="flex space-x-1">
                              {DAYS.map((day, index) => (
                                <button
                                  key={day}
                                  onClick={() => toggleDay(scheduleId, index)}
                                  className={`flex-1 py-2 px-1 text-xs font-medium rounded transition-colors ${
                                    (schedule.days || []).includes(index)
                                      ? 'bg-blue-500 text-white'
                                      : 'bg-gray-200 text-gray-600 hover:bg-gray-300'
                                  }`}
                                >
                                  {day}
                                </button>
                              ))}
                            </div>
                          </div>
                        </div>

                        <div className="flex items-center justify-between pt-3 border-t border-gray-200">
                          <label className="flex items-center space-x-2 cursor-pointer">
                            <input
                              type="checkbox"
                              checked={schedule.enabled}
                              onChange={(e) =>
                                updateSchedule(scheduleId, { enabled: e.target.checked })
                              }
                              className="w-5 h-5 text-blue-600 border-gray-300 rounded focus:ring-blue-500"
                            />
                            <span className="font-medium text-gray-700">
                              {schedule.enabled ? 'Enabled' : 'Disabled'}
                            </span>
                          </label>

                          <button
                            onClick={() => removeSchedule(scheduleId)}
                            className="p-2 text-red-600 hover:bg-red-50 rounded-lg transition-colors"
                          >
                            <Trash2 className="h-5 w-5" />
                          </button>
                        </div>
                      </div>
                    </div>
                  ))}
                </div>
              ) : (
                <div className="empty-state py-12">
                  <Clock className="empty-state-icon mx-auto" />
                  <h3 className="text-lg font-semibold text-gray-900 mb-2">No Schedules Yet</h3>
                  <p className="text-gray-600 mb-4">
                    Add a schedule to start automatic medication dispensing
                  </p>
                  <button onClick={addSchedule} className="btn btn-primary">
                    <Plus className="h-4 w-4 mr-2" />
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
