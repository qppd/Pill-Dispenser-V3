'use client';

import { useEffect, useState } from 'react';
import { db } from '@/lib/firebase';
import { ref, onValue } from 'firebase/database';
import { FileText, Download, Search, Filter, Calendar } from 'lucide-react';

interface PillLog {
  timestamp: string;
  pill_count: number;
  device_id?: string;
  status?: string;
  uptime?: string;
}

interface PillReport {
  pill_count: number;
  datetime: string;
  description: string;
  status: number;
  device_id?: string;
}

export default function LogsPage() {
  const [logs, setLogs] = useState<PillLog[]>([]);
  const [reports, setReports] = useState<PillReport[]>([]);
  const [loading, setLoading] = useState(true);
  const [searchTerm, setSearchTerm] = useState('');
  const [filterType, setFilterType] = useState<'all' | 'logs' | 'reports'>('all');

  useEffect(() => {
    if (!db) return;

    const logsRef = ref(db, 'pilldispenser/device');
    const unsubscribeLogs = onValue(logsRef, (snapshot) => {
      const data = snapshot.val();
      let allLogs: PillLog[] = [];
      if (data) {
        Object.values(data).forEach((device: any) => {
          if (device.pill_logs) {
            Object.values(device.pill_logs).forEach((log: any) => {
              allLogs.push(log);
            });
          }
        });
      }
      setLogs(allLogs.sort((a, b) => new Date(b.timestamp).getTime() - new Date(a.timestamp).getTime()));
      setLoading(false);
    });

    const reportsRef = ref(db, 'pilldispenser/reports');
    const unsubscribeReports = onValue(reportsRef, (snapshot) => {
      const data = snapshot.val();
      let allReports: PillReport[] = [];
      if (data) {
        Object.values(data).forEach((report: any) => {
          allReports.push(report);
        });
      }
      setReports(allReports.sort((a, b) => new Date(b.datetime).getTime() - new Date(a.datetime).getTime()));
    });

    return () => {
      unsubscribeLogs();
      unsubscribeReports();
    };
  }, []);

  const filteredLogs = logs.filter((log) =>
    log.device_id?.toLowerCase().includes(searchTerm.toLowerCase()) ||
    log.timestamp?.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const filteredReports = reports.filter((report) =>
    report.device_id?.toLowerCase().includes(searchTerm.toLowerCase()) ||
    report.description?.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const exportLogs = () => {
    const csvContent = [
      ['Timestamp', 'Pill Count', 'Device ID', 'Status', 'Uptime'].join(','),
      ...logs.map((log) =>
        [log.timestamp, log.pill_count, log.device_id || '-', log.status || '-', log.uptime || '-'].join(',')
      ),
    ].join('\n');

    const blob = new Blob([csvContent], { type: 'text/csv' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `pill-logs-${new Date().toISOString()}.csv`;
    a.click();
  };

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-center animate-fade-in">
          <div className="spinner mx-auto mb-4"></div>
          <p className="text-gray-600">Loading logs...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-purple-50 to-pink-50 px-2 sm:px-8 md:px-16 lg:px-32 xl:px-64">
      <div className="max-w-6xl mx-auto px-6 sm:px-12 lg:px-20 py-12">
        <div className="mb-10 animate-slide-in-down">
          <div className="flex items-center justify-between flex-wrap gap-4">
            <div>
              <h1 className="text-4xl md:text-5xl font-extrabold bg-gradient-to-r from-blue-600 via-purple-600 to-pink-600 bg-clip-text text-transparent mb-3">Activity Logs 📋</h1>
              <p className="text-gray-600 text-lg font-medium">Track all dispensing activities and reports</p>
            </div>
            <button onClick={exportLogs} className="flex items-center space-x-2 px-6 py-3.5 bg-white hover:bg-gray-50 border-2 border-gray-200 rounded-xl font-bold text-gray-700 transition-all duration-300 shadow-lg hover:shadow-xl hover:scale-105">
              <Download className="h-5 w-5" />
              <span>Export CSV</span>
            </button>
          </div>
        </div>

        <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-6 shadow-xl border border-gray-100 mb-8 animate-scale-in">
          <div className="flex flex-col md:flex-row gap-4">
            <div className="flex-1 relative">
              <Search className="absolute left-4 top-1/2 transform -translate-y-1/2 h-5 w-5 text-gray-400" />
              <input
                type="text"
                placeholder="Search logs by device ID, timestamp..."
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                className="w-full pl-12 pr-4 py-3.5 border-2 border-gray-200 rounded-xl focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-all duration-300 bg-gray-50 focus:bg-white font-medium"
              />
            </div>
            <div className="flex items-center space-x-3 bg-gray-50 px-4 rounded-xl border-2 border-gray-200">
              <Filter className="h-5 w-5 text-gray-500" />
              <select
                value={filterType}
                onChange={(e) => setFilterType(e.target.value as any)}
                className="py-3.5 bg-transparent border-none focus:ring-0 focus:outline-none font-bold text-gray-700 cursor-pointer"
              >
                <option value="all">All Logs</option>
                <option value="logs">Dispense Logs</option>
                <option value="reports">Reports</option>
              </select>
            </div>
          </div>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-10">
          <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-6 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100 animate-scale-in" style={{ animationDelay: '0.1s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm font-semibold text-gray-500 mb-2 uppercase tracking-wide">Total Logs</p>
                <p className="text-4xl font-extrabold bg-gradient-to-r from-blue-600 to-purple-600 bg-clip-text text-transparent">{logs.length}</p>
              </div>
              <div className="bg-gradient-to-br from-blue-400 to-purple-500 p-4 rounded-2xl shadow-lg">
                <FileText className="h-10 w-10 text-white" />
              </div>
            </div>
          </div>

          <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-6 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100 animate-scale-in" style={{ animationDelay: '0.2s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm font-semibold text-gray-500 mb-2 uppercase tracking-wide">Total Reports</p>
                <p className="text-4xl font-extrabold bg-gradient-to-r from-purple-600 to-pink-600 bg-clip-text text-transparent">{reports.length}</p>
              </div>
              <div className="bg-gradient-to-br from-purple-400 to-pink-500 p-4 rounded-2xl shadow-lg">
                <FileText className="h-10 w-10 text-white" />
              </div>
            </div>
          </div>

          <div className="bg-white/90 backdrop-blur-sm rounded-2xl p-6 shadow-xl hover:shadow-2xl transition-all duration-300 hover:scale-105 border border-gray-100 animate-scale-in" style={{ animationDelay: '0.3s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm font-semibold text-gray-500 mb-2 uppercase tracking-wide">Today</p>
                <p className="text-4xl font-extrabold bg-gradient-to-r from-green-600 to-teal-600 bg-clip-text text-transparent">
                  {logs.filter((log) => new Date(log.timestamp).toDateString() === new Date().toDateString()).length}
                </p>
              </div>
              <div className="bg-gradient-to-br from-green-400 to-teal-500 p-4 rounded-2xl shadow-lg">
                <Calendar className="h-10 w-10 text-white" />
              </div>
            </div>
          </div>
        </div>

        {(filterType === 'all' || filterType === 'logs') && (
          <div className="bg-white/90 backdrop-blur-sm rounded-2xl shadow-xl border border-gray-100 mb-8 overflow-hidden animate-slide-in-up" style={{ animationDelay: '0.1s' }}>
            <div className="p-6 border-b-2 border-gray-100">
              <h2 className="text-2xl font-bold text-gray-900">Dispense Logs</h2>
            </div>
            {filteredLogs.length > 0 ? (
              <div className="overflow-x-auto">
                <table className="min-w-full">
                  <thead className="bg-gradient-to-r from-gray-50 to-gray-100">
                    <tr className="border-b-2 border-gray-200">
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Timestamp</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Pills</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Device ID</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Status</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Uptime</th>
                    </tr>
                  </thead>
                  <tbody className="bg-white divide-y divide-gray-100">
                    {filteredLogs.map((log, idx) => (
                      <tr key={idx} className="hover:bg-gradient-to-r hover:from-blue-50 hover:to-purple-50 transition-all duration-200">
                        <td className="px-6 py-4 text-sm font-medium text-gray-900">{log.timestamp}</td>
                        <td className="px-6 py-4 text-sm">
                          <span className="inline-flex items-center px-3 py-1.5 rounded-lg text-xs font-bold bg-gradient-to-r from-blue-100 to-blue-200 text-blue-700 border border-blue-300">{log.pill_count}</span>
                        </td>
                        <td className="px-6 py-4 text-sm font-medium text-gray-600">{log.device_id || '-'}</td>
                        <td className="px-6 py-4 text-sm">
                          <span className={`inline-flex items-center px-3 py-1.5 rounded-lg text-xs font-bold border ${
                            log.status === 'dispensed' 
                              ? 'bg-gradient-to-r from-green-100 to-emerald-200 text-green-700 border-green-300' 
                              : 'bg-gradient-to-r from-yellow-100 to-orange-200 text-yellow-700 border-yellow-300'
                          }`}>
                            {log.status || '-'}
                          </span>
                        </td>
                        <td className="px-6 py-4 text-sm font-medium text-gray-600">{log.uptime || '-'}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            ) : (
              <div className="p-12 text-center">
                <FileText className="h-16 w-16 text-gray-300 mx-auto mb-4" />
                <h3 className="text-xl font-bold text-gray-900 mb-2">No Logs Yet</h3>
                <p className="text-gray-600 text-lg">Dispense logs will appear here</p>
              </div>
            )}
          </div>
        )}

        {(filterType === 'all' || filterType === 'reports') && (
          <div className="bg-white/90 backdrop-blur-sm rounded-2xl shadow-xl border border-gray-100 overflow-hidden animate-slide-in-up" style={{ animationDelay: '0.2s' }}>
            <div className="p-6 border-b-2 border-gray-100">
              <h2 className="text-2xl font-bold text-gray-900">Pill Reports</h2>
            </div>
            {filteredReports.length > 0 ? (
              <div className="overflow-x-auto">
                <table className="min-w-full">
                  <thead className="bg-gradient-to-r from-gray-50 to-gray-100">
                    <tr className="border-b-2 border-gray-200">
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Datetime</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Pills</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Description</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Status</th>
                      <th className="px-6 py-4 text-left text-sm font-bold text-gray-700 uppercase tracking-wider">Device ID</th>
                    </tr>
                  </thead>
                  <tbody className="bg-white divide-y divide-gray-100">
                    {filteredReports.map((report, idx) => (
                      <tr key={idx} className="hover:bg-gradient-to-r hover:from-purple-50 hover:to-pink-50 transition-all duration-200">
                        <td className="px-6 py-4 text-sm font-medium text-gray-900">{report.datetime}</td>
                        <td className="px-6 py-4 text-sm">
                          <span className="inline-flex items-center px-3 py-1.5 rounded-lg text-xs font-bold bg-gradient-to-r from-blue-100 to-blue-200 text-blue-700 border border-blue-300">{report.pill_count}</span>
                        </td>
                        <td className="px-6 py-4 text-sm font-medium text-gray-900">{report.description}</td>
                        <td className="px-6 py-4 text-sm">
                          <span className={`inline-flex items-center px-3 py-1.5 rounded-lg text-xs font-bold border ${
                            report.status === 1 
                              ? 'bg-gradient-to-r from-green-100 to-emerald-200 text-green-700 border-green-300' 
                              : 'bg-gradient-to-r from-red-100 to-pink-200 text-red-700 border-red-300'
                          }`}>
                            {report.status === 1 ? 'Success' : 'Error'}
                          </span>
                        </td>
                        <td className="px-6 py-4 text-sm font-medium text-gray-600">{report.device_id || '-'}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            ) : (
              <div className="p-12 text-center">
                <FileText className="h-16 w-16 text-gray-300 mx-auto mb-4" />
                <h3 className="text-xl font-bold text-gray-900 mb-2">No Reports Yet</h3>
                <p className="text-gray-600 text-lg">Reports will appear here</p>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
