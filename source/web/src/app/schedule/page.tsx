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
    const schedulesRef = ref(db, `schedules/${user.uid}`);
    await set(schedulesRef, { dispensers });
    setSaving(false);

    // Show success toast (you can add a toast library later)
    alert('Schedules saved successfully!');
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
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-white to-purple-50">
      <div className="max-w-5xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {/* Header */}
        <div className="mb-8 animate-slide-in-down">
          <h1 className="text-3xl font-bold text-gray-900 mb-2">
            Medication Schedule 📅
          </h1>
          <p className="text-gray-600">
            Set up to 3 daily schedules for each container
          </p>
        </div>

        <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
          {/* Dispenser Selection */}
          <div className="lg:col-span-1">
            <div className="card animate-slide-in-left">
              <h2 className="text-lg font-semibold text-gray-900 mb-4">Containers</h2>
              <div className="space-y-2">
                {dispensers.map((dispenser) => (
                  <button
                    key={dispenser.id}
                    onClick={() => setSelectedDispenser(dispenser.id)}
                    className={`w-full flex items-center justify-between p-3 rounded-lg transition-all duration-200 ${
                      selectedDispenser === dispenser.id
                        ? 'bg-gradient-to-r from-blue-500 to-purple-600 text-white shadow-lg'
                        : 'bg-gray-50 text-gray-700 hover:bg-gray-100'
                    }`}
                  >
                    <div className="flex items-center space-x-3">
                      <Pill className="h-5 w-5" />
                      <span className="font-medium">{dispenser.name}</span>
                    </div>
                    <span className={`text-sm ${
                      selectedDispenser === dispenser.id ? 'text-white' : 'text-gray-500'
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
            <div className="card animate-slide-in-right">
              <div className="flex items-center justify-between mb-6">
                <h2 className="text-lg font-semibold text-gray-900">
                  {currentDispenser?.name} Schedule
                </h2>
                <button
                  onClick={() => addSchedule(selectedDispenser)}
                  disabled={!currentDispenser || currentDispenser.schedules.length >= 3}
                  className="btn btn-primary flex items-center space-x-2 disabled:opacity-50 disabled:cursor-not-allowed"
                >
                  <Plus className="h-4 w-4" />
                  <span>Add Schedule</span>
                </button>
              </div>

              {/* Schedules List */}
              {currentDispenser && currentDispenser.schedules.length > 0 ? (
                <div className="space-y-4 mb-6">
                  {currentDispenser.schedules.map((schedule, index) => (
                    <div
                      key={index}
                      className={`p-4 rounded-lg border-2 transition-all duration-200 ${
                        schedule.enabled
                          ? 'border-green-200 bg-green-50'
                          : 'border-gray-200 bg-gray-50'
                      }`}
                    >
                      <div className="flex items-center justify-between">
                        <div className="flex items-center space-x-4 flex-1">
                          <div className="flex items-center space-x-2">
                            <Clock className={`h-5 w-5 ${
                              schedule.enabled ? 'text-green-600' : 'text-gray-400'
                            }`} />
                            <input
                              type="time"
                              value={schedule.time}
                              onChange={(e) =>
                                updateScheduleTime(selectedDispenser, index, e.target.value)
                              }
                              className="px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
                            />
                          </div>

                          <label className="flex items-center space-x-2 cursor-pointer">
                            <input
                              type="checkbox"
                              checked={schedule.enabled}
                              onChange={() => toggleSchedule(selectedDispenser, index)}
                              className="w-5 h-5 text-blue-600 border-gray-300 rounded focus:ring-blue-500"
                            />
                            <span className={`font-medium ${
                              schedule.enabled ? 'text-gray-900' : 'text-gray-500'
                            }`}>
                              {schedule.enabled ? 'Enabled' : 'Disabled'}
                            </span>
                          </label>
                        </div>

                        <button
                          onClick={() => removeSchedule(selectedDispenser, index)}
                          className="p-2 text-red-600 hover:bg-red-50 rounded-lg transition-colors"
                        >
                          <Trash2 className="h-5 w-5" />
                        </button>
                      </div>
                    </div>
                  ))}
                </div>
              ) : (
                <div className="empty-state py-12">
                  <Clock className="empty-state-icon mx-auto" />
                  <h3 className="text-lg font-semibold text-gray-900 mb-2">No Schedules Yet</h3>
                  <p className="text-gray-600 mb-4">Add a schedule to get started</p>
                  <button
                    onClick={() => addSchedule(selectedDispenser)}
                    className="btn btn-primary"
                  >
                    <Plus className="h-4 w-4 mr-2" />
                    Add First Schedule
                  </button>
                </div>
              )}

              {/* Save Button */}
              <div className="flex items-center justify-end space-x-3 pt-4 border-t border-gray-200">
                <button
                  onClick={saveSchedules}
                  disabled={saving}
                  className="btn btn-success flex items-center space-x-2"
                >
                  {saving ? (
                    <>
                      <div className="spinner w-4 h-4 border-2"></div>
                      <span>Saving...</span>
                    </>
                  ) : (
                    <>
                      <Save className="h-4 w-4" />
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
