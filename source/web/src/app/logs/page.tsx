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
    <div className="min-h-screen bg-gradient-to-br from-blue-50 via-white to-purple-50">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8 animate-slide-in-down">
          <div className="flex items-center justify-between flex-wrap gap-4">
            <div>
              <h1 className="text-3xl font-bold text-gray-900 mb-2">Activity Logs 📋</h1>
              <p className="text-gray-600">Track all dispensing activities and reports</p>
            </div>
            <button onClick={exportLogs} className="btn btn-outline flex items-center space-x-2">
              <Download className="h-4 w-4" />
              <span>Export CSV</span>
            </button>
          </div>
        </div>

        <div className="card mb-6 animate-scale-in">
          <div className="flex flex-col md:flex-row gap-4">
            <div className="flex-1 relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-5 w-5 text-gray-400" />
              <input
                type="text"
                placeholder="Search logs..."
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                className="w-full pl-10 pr-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              />
            </div>
            <div className="flex items-center space-x-2">
              <Filter className="h-5 w-5 text-gray-400" />
              <select
                value={filterType}
                onChange={(e) => setFilterType(e.target.value as any)}
                className="px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-transparent"
              >
                <option value="all">All Logs</option>
                <option value="logs">Dispense Logs</option>
                <option value="reports">Reports</option>
              </select>
            </div>
          </div>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
          <div className="card animate-scale-in" style={{ animationDelay: '0.1s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm text-gray-600 mb-1">Total Logs</p>
                <p className="text-3xl font-bold text-gray-900">{logs.length}</p>
              </div>
              <div className="bg-blue-100 p-3 rounded-xl">
                <FileText className="h-8 w-8 text-blue-600" />
              </div>
            </div>
          </div>

          <div className="card animate-scale-in" style={{ animationDelay: '0.2s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm text-gray-600 mb-1">Total Reports</p>
                <p className="text-3xl font-bold text-gray-900">{reports.length}</p>
              </div>
              <div className="bg-purple-100 p-3 rounded-xl">
                <FileText className="h-8 w-8 text-purple-600" />
              </div>
            </div>
          </div>

          <div className="card animate-scale-in" style={{ animationDelay: '0.3s' }}>
            <div className="flex items-center justify-between">
              <div>
                <p className="text-sm text-gray-600 mb-1">Today</p>
                <p className="text-3xl font-bold text-gray-900">
                  {logs.filter((log) => new Date(log.timestamp).toDateString() === new Date().toDateString()).length}
                </p>
              </div>
              <div className="bg-green-100 p-3 rounded-xl">
                <Calendar className="h-8 w-8 text-green-600" />
              </div>
            </div>
          </div>
        </div>

        {(filterType === 'all' || filterType === 'logs') && (
          <div className="card mb-8 animate-slide-in-up" style={{ animationDelay: '0.1s' }}>
            <h2 className="text-xl font-semibold text-gray-900 mb-4">Dispense Logs</h2>
            {filteredLogs.length > 0 ? (
              <div className="overflow-x-auto">
                <table className="min-w-full">
                  <thead>
                    <tr className="border-b border-gray-200">
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Timestamp</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Pills</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Device ID</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Status</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Uptime</th>
                    </tr>
                  </thead>
                  <tbody>
                    {filteredLogs.map((log, idx) => (
                      <tr key={idx} className="border-b border-gray-100 hover:bg-gray-50 transition-colors">
                        <td className="px-4 py-3 text-sm text-gray-900">{log.timestamp}</td>
                        <td className="px-4 py-3 text-sm">
                          <span className="badge badge-info">{log.pill_count}</span>
                        </td>
                        <td className="px-4 py-3 text-sm text-gray-600">{log.device_id || '-'}</td>
                        <td className="px-4 py-3 text-sm">
                          <span className={`badge ${log.status === 'dispensed' ? 'badge-success' : 'badge-warning'}`}>
                            {log.status || '-'}
                          </span>
                        </td>
                        <td className="px-4 py-3 text-sm text-gray-600">{log.uptime || '-'}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            ) : (
              <div className="empty-state">
                <FileText className="empty-state-icon" />
                <h3 className="text-lg font-semibold text-gray-900 mb-2">No Logs Yet</h3>
                <p className="text-gray-600">Dispense logs will appear here</p>
              </div>
            )}
          </div>
        )}

        {(filterType === 'all' || filterType === 'reports') && (
          <div className="card animate-slide-in-up" style={{ animationDelay: '0.2s' }}>
            <h2 className="text-xl font-semibold text-gray-900 mb-4">Pill Reports</h2>
            {filteredReports.length > 0 ? (
              <div className="overflow-x-auto">
                <table className="min-w-full">
                  <thead>
                    <tr className="border-b border-gray-200">
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Datetime</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Pills</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Description</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Status</th>
                      <th className="px-4 py-3 text-left text-sm font-semibold text-gray-700">Device ID</th>
                    </tr>
                  </thead>
                  <tbody>
                    {filteredReports.map((report, idx) => (
                      <tr key={idx} className="border-b border-gray-100 hover:bg-gray-50 transition-colors">
                        <td className="px-4 py-3 text-sm text-gray-900">{report.datetime}</td>
                        <td className="px-4 py-3 text-sm">
                          <span className="badge badge-info">{report.pill_count}</span>
                        </td>
                        <td className="px-4 py-3 text-sm text-gray-900">{report.description}</td>
                        <td className="px-4 py-3 text-sm">
                          <span className={`badge ${report.status === 1 ? 'badge-success' : 'badge-error'}`}>
                            {report.status === 1 ? 'Success' : 'Error'}
                          </span>
                        </td>
                        <td className="px-4 py-3 text-sm text-gray-600">{report.device_id || '-'}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            ) : (
              <div className="empty-state">
                <FileText className="empty-state-icon" />
                <h3 className="text-lg font-semibold text-gray-900 mb-2">No Reports Yet</h3>
                <p className="text-gray-600">Reports will appear here</p>
              </div>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
