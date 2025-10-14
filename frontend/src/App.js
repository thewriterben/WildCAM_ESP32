import React, { useState, useEffect, useMemo } from 'react';
import axios from 'axios';
import {
  MapContainer,
  TileLayer,
  Marker,
  Popup,
  LayersControl,
  LayerGroup,
} from 'react-leaflet';
import L from 'leaflet';
import 'leaflet/dist/leaflet.css';
import { Line } from 'react-chartjs-2';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
import './App.css';

import markerIcon2x from 'leaflet/dist/images/marker-icon-2x.png';
import markerIcon from 'leaflet/dist/images/marker-icon.png';
import markerShadow from 'leaflet/dist/images/marker-shadow.png';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

// Configure default Leaflet marker icons for bundlers
L.Icon.Default.mergeOptions({
  iconRetinaUrl: markerIcon2x,
  iconUrl: markerIcon,
  shadowUrl: markerShadow,
});

const defaultState = {
  cameras: [],
  captures: [],
  alerts: [],
  insights: [],
};

const DEFAULT_MAP_CENTER = [0, 0];
const DEFAULT_MAP_ZOOM = 2;

const ICONS = {
  camera: '📷',
  activity: '🦌',
  alert: '⚠️',
  wifi: '📡',
  battery: '🔋',
  refresh: '🔄',
  trendingUp: '📈',
  trendingDown: '📉',
  mapPin: '📍',
  clock: '🕒',
  shield: '🛡️',
};

function EmojiIcon({ symbol, label, className }) {
  return (
    <span
      className={`emoji-icon${className ? ` ${className}` : ''}`}
      role={label ? 'img' : 'presentation'}
      aria-label={label ?? undefined}
    >
      {symbol}
    </span>
  );
}

function WildlifeDashboard() {
  const [data, setData] = useState(defaultState);
  const [selectedCameraId, setSelectedCameraId] = useState(null);
  const [selectedCapture, setSelectedCapture] = useState(null);
  const [galleryFilter, setGalleryFilter] = useState('all');
  const [timeRange, setTimeRange] = useState('30d');
  const [isRefreshing, setIsRefreshing] = useState(false);
  const [error, setError] = useState(null);

  const selectedCamera = useMemo(
    () => data.cameras.find((camera) => camera.id === selectedCameraId) ?? null,
    [data.cameras, selectedCameraId]
  );

  useEffect(() => {
    loadAllData();
  }, []);

  useEffect(() => {
    if (selectedCameraId && !data.cameras.some((camera) => camera.id === selectedCameraId)) {
      setSelectedCameraId(null);
    }
  }, [data.cameras, selectedCameraId]);

  const loadAllData = async () => {
    setIsRefreshing(true);
    setError(null);
    try {
      const [camerasResponse, capturesResponse, alertsResponse, insightsResponse] =
        await Promise.all([
          axios.get('/api/cameras'),
          axios.get('/api/captures/recent'),
          axios.get('/api/alerts/active'),
          axios.get('/api/insights'),
        ]);

      setData({
        cameras: camerasResponse.data ?? [],
        captures: capturesResponse.data ?? [],
        alerts: alertsResponse.data ?? [],
        insights: insightsResponse.data ?? [],
      });
    } catch (requestError) {
      setError(
        requestError?.response?.data?.message ??
          requestError?.message ??
          'Unable to load WildCAM data.'
      );
    } finally {
      setIsRefreshing(false);
    }
  };

  const metrics = useMemo(() => computeMetrics(data, timeRange), [data, timeRange]);

  const filteredCaptures = useMemo(() => {
    const normalizedFilter = galleryFilter.toLowerCase();
    return data.captures.filter((capture) => {
      if (normalizedFilter === 'all') return true;
      return (capture?.classification ?? '').toLowerCase() === normalizedFilter;
    });
  }, [data.captures, galleryFilter]);

  return (
    <div className="dashboard">
      <Header onRefresh={loadAllData} refreshing={isRefreshing} />

      {error && <ErrorBanner message={error} />}

      <section className="stats-section">
        <QuickStats metrics={metrics} timeRange={timeRange} onTimeRangeChange={setTimeRange} />
        <SystemHealth cameras={data.cameras} alerts={data.alerts} />
      </section>

      <section className="map-and-details">
        <CameraMap
          cameras={data.cameras}
          captures={data.captures}
          selectedCameraId={selectedCameraId}
          onCameraSelect={setSelectedCameraId}
        />
        <CameraDetails
          camera={selectedCamera}
          onViewGallery={() => setGalleryFilter('all')}
          onOpenCapture={setSelectedCapture}
          captures={data.captures.filter((capture) => capture.cameraId === selectedCameraId)}
        />
      </section>

      <section className="analytics-section">
        <ActivityAnalytics metrics={metrics} />
        <InsightsPanel insights={data.insights} />
      </section>

      <section className="gallery-section">
        <CaptureGallery
          captures={filteredCaptures}
          filter={galleryFilter}
          onFilterChange={setGalleryFilter}
          onCaptureClick={setSelectedCapture}
        />
        <AlertFeed alerts={data.alerts} />
      </section>

      <CaptureModal capture={selectedCapture} onClose={() => setSelectedCapture(null)} />
    </div>
  );
}

function Header({ onRefresh, refreshing }) {
  return (
    <header className="header">
      <div>
        <h1>WildCAM Command Center</h1>
        <p>Unified monitoring and analytics for the ESP32 wildlife camera network.</p>
      </div>
      <button className="refresh-button" onClick={onRefresh} disabled={refreshing} type="button">
        <EmojiIcon symbol={ICONS.refresh} className={refreshing ? 'spinning' : ''} label="Refresh" />
        {refreshing ? 'Refreshing' : 'Refresh Data'}
      </button>
    </header>
  );
}

function ErrorBanner({ message }) {
  return (
    <div className="error-banner">
      <EmojiIcon symbol={ICONS.alert} label="Alert" className="alert-icon" />
      <span>{message}</span>
    </div>
  );
}

function QuickStats({ metrics, timeRange, onTimeRangeChange }) {
  return (
    <div className="panel stats-panel">
      <div className="panel-header">
        <h2>Network Overview</h2>
        <select value={timeRange} onChange={(event) => onTimeRangeChange(event.target.value)}>
          <option value="7d">Last 7 days</option>
          <option value="30d">Last 30 days</option>
          <option value="90d">Last 90 days</option>
        </select>
      </div>
      <div className="stats-grid">
        <StatCard icon="camera" label="Active Cameras" value={metrics.activeCameras} trend={metrics.cameraGrowth} />
        <StatCard
          icon="activity"
          label="Total Captures"
          value={metrics.totalCaptures}
          trend={metrics.captureTrend}
        />
        <StatCard
          icon="wifi"
          label="Online Rate"
          value={`${metrics.onlineRate}%`}
          trend={metrics.onlineTrend}
        />
        <StatCard
          icon="battery"
          label="Avg Battery"
          value={`${metrics.averageBattery}%`}
          trend={metrics.batteryTrend}
        />
      </div>
    </div>
  );
}

function StatCard({ icon, label, value, trend }) {
  return (
    <div className="stat-card">
      <div className="stat-icon">
        <EmojiIcon symbol={ICONS[icon]} label={label} className="stat-icon-symbol" />
      </div>
      <div>
        <p className="stat-label">{label}</p>
        <p className="stat-value">{value}</p>
      </div>
      {trend && (
        <div className={`stat-trend ${trend.direction}`}>
          <EmojiIcon
            symbol={trend.direction === 'down' ? ICONS.trendingDown : ICONS.trendingUp}
            label={trend.direction === 'down' ? 'Trend down' : 'Trend up'}
            className="trend-icon"
          />
          <span>{trend.percentage}%</span>
        </div>
      )}
    </div>
  );
}

function SystemHealth({ cameras, alerts }) {
  const offlineCameras = cameras.filter((camera) => camera.status === 'offline');
  const lowBatteryCameras = cameras.filter((camera) => camera.batteryLevel < 25);

  return (
    <div className="panel health-panel">
      <div className="panel-header">
        <h2>System Health</h2>
      </div>
      <div className="health-grid">
        <HealthCard
          icon="wifi"
          label="Offline Cameras"
          count={offlineCameras.length}
          status={offlineCameras.length > 0 ? 'warning' : 'success'}
        />
        <HealthCard
          icon="battery"
          label="Low Battery"
          count={lowBatteryCameras.length}
          status={lowBatteryCameras.length > 0 ? 'warning' : 'success'}
        />
        <HealthCard
          icon="alert"
          label="Active Alerts"
          count={alerts.length}
          status={alerts.length > 0 ? 'warning' : 'success'}
        />
      </div>
    </div>
  );
}

function HealthCard({ icon, label, count, status }) {
  return (
    <div className={`health-card ${status}`}>
      <div className="health-icon">
        <EmojiIcon symbol={ICONS[icon]} label={label} className="health-icon-symbol" />
      </div>
      <div>
        <p className="health-label">{label}</p>
        <p className="health-count">{count}</p>
      </div>
    </div>
  );
}

function CameraMap({ cameras, captures, selectedCameraId, onCameraSelect }) {
  return (
    <div className="panel map-panel">
      <div className="panel-header">
        <h2>Camera Map</h2>
      </div>
      <MapContainer center={DEFAULT_MAP_CENTER} zoom={DEFAULT_MAP_ZOOM} className="map">
        <TileLayer url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png" />
        <LayersControl position="topright">
          <LayersControl.Overlay name="Cameras" checked>
            <LayerGroup>
              {cameras.map((camera) => (
                <Marker
                  key={camera.id}
                  position={[camera.latitude, camera.longitude]}
                  eventHandlers={{
                    click: () => onCameraSelect(camera.id),
                  }}
                >
                  <Popup>
                    <strong>{camera.name}</strong>
                    <div className="popup-details">
                      <span>
                        <EmojiIcon symbol={ICONS.wifi} label="Status" className="popup-icon" /> {camera.status}
                      </span>
                      <span>
                        <EmojiIcon symbol={ICONS.battery} label="Battery" className="popup-icon" />{' '}
                        {camera.batteryLevel}%
                      </span>
                    </div>
                  </Popup>
                </Marker>
              ))}
            </LayerGroup>
          </LayersControl.Overlay>
          <LayersControl.Overlay name="Recent Captures">
            <LayerGroup>
              {captures.slice(0, 50).map((capture) => (
                <Marker
                  key={capture.id}
                  position={[capture.latitude ?? capture.lat, capture.longitude ?? capture.lng]}
                  eventHandlers={{
                    click: () => onCameraSelect(capture.cameraId),
                  }}
                >
                  <Popup>
                    <strong>{capture.species ?? capture.classification}</strong>
                    <div className="popup-details">
                      <span>
                        <EmojiIcon symbol={ICONS.clock} label="Captured at" className="popup-icon" />
                        {formatDateTime(capture.timestamp)}
                      </span>
                      <span>
                        <EmojiIcon symbol={ICONS.camera} label="Camera" className="popup-icon" />{' '}
                        {capture.cameraName ?? capture.cameraId}
                      </span>
                    </div>
                  </Popup>
                </Marker>
              ))}
            </LayerGroup>
          </LayersControl.Overlay>
        </LayersControl>
      </MapContainer>
    </div>
  );
}

function CameraDetails({ camera, onViewGallery, onOpenCapture, captures }) {
  if (!camera) {
    return (
      <div className="panel details-panel empty">
        <EmojiIcon symbol={ICONS.mapPin} label="Select a camera" className="icon-empty-state" />
        <p>Select a camera from the map to view details.</p>
      </div>
    );
  }

  return (
    <div className="panel details-panel">
      <div className="panel-header">
        <h2>{camera.name}</h2>
        <StatusPill status={camera.status} />
      </div>

      <div className="details-grid">
        <DetailItem icon="wifi" label="Connectivity" value={camera.status} />
        <DetailItem icon="battery" label="Battery" value={`${camera.batteryLevel}%`} />
        <DetailItem icon="shield" label="Firmware" value={camera.firmwareVersion} />
        <DetailItem icon="clock" label="Last Sync" value={formatDateTime(camera.lastSeen)} />
      </div>

      <div className="location-info">
        <EmojiIcon symbol={ICONS.mapPin} label="Location" className="location-icon" />
        <span>
          {camera.latitude?.toFixed(4)}, {camera.longitude?.toFixed(4)}
        </span>
      </div>

      <div className="recent-captures">
        <div className="recent-header">
          <h3>Recent Captures</h3>
          <button type="button" onClick={onViewGallery} className="link-button">
            View gallery
          </button>
        </div>
        <ul>
          {captures.slice(0, 5).map((capture) => (
            <li key={capture.id} onClick={() => onOpenCapture(capture)}>
              <img src={capture.thumbnailUrl ?? capture.imageUrl} alt={capture.species ?? 'Capture'} />
              <div>
                <p>{capture.species ?? capture.classification ?? 'Unknown'}</p>
                <span>{formatDateTime(capture.timestamp)}</span>
              </div>
            </li>
          ))}
          {captures.length === 0 && <p className="empty-text">No captures yet.</p>}
        </ul>
      </div>
    </div>
  );
}

function DetailItem({ icon, label, value }) {
  return (
    <div className="detail-item">
      <div className="detail-icon">
        <EmojiIcon symbol={ICONS[icon]} label={label} className="detail-icon-symbol" />
      </div>
      <div>
        <p className="detail-label">{label}</p>
        <p className="detail-value">{value ?? '—'}</p>
      </div>
    </div>
  );
}

function StatusPill({ status }) {
  return <span className={`status-pill ${status}`}>{status}</span>;
}

function ActivityAnalytics({ metrics }) {
  return (
    <div className="panel analytics-panel">
      <div className="panel-header">
        <h2>Activity Trends</h2>
      </div>
      <Line data={metrics.activityChart.data} options={metrics.activityChart.options} height={240} />
    </div>
  );
}

function InsightsPanel({ insights }) {
  return (
    <div className="panel insights-panel">
      <div className="panel-header">
        <h2>AI Insights</h2>
      </div>
      <ul>
        {insights.map((insight) => (
          <li key={insight.id}>
            <div className="insight-header">
              <span className="insight-type">{insight.type}</span>
              <span className="insight-time">{formatDateTime(insight.timestamp)}</span>
            </div>
            <p>{insight.message}</p>
          </li>
        ))}
        {insights.length === 0 && <p className="empty-text">No insights available.</p>}
      </ul>
    </div>
  );
}

function CaptureGallery({ captures, filter, onFilterChange, onCaptureClick }) {
  const classifications = useMemo(() => {
    const dictionary = new Map();
    captures.forEach((capture) => {
      if (!capture?.classification) return;
      const normalized = capture.classification.toLowerCase();
      if (!dictionary.has(normalized)) {
        dictionary.set(normalized, capture.classification);
      }
    });

    return [
      { value: 'all', label: 'All classifications' },
      ...Array.from(dictionary.entries()).map(([value, label]) => ({ value, label })),
    ];
  }, [captures]);

  return (
    <div className="panel gallery-panel">
      <div className="panel-header">
        <h2>Capture Gallery</h2>
        <select value={filter} onChange={(event) => onFilterChange(event.target.value)}>
          {classifications.map((option) => (
            <option key={option.value} value={option.value}>
              {option.label}
            </option>
          ))}
        </select>
      </div>
      <div className="capture-grid">
        {captures.map((capture) => (
          <button
            type="button"
            key={capture.id}
            className="capture-card"
            onClick={() => onCaptureClick(capture)}
          >
            <img src={capture.thumbnailUrl ?? capture.imageUrl} alt={capture.species ?? 'Capture'} />
            <div className="capture-meta">
              <p>{capture.species ?? capture.classification ?? 'Unknown'}</p>
              <span>{formatDateTime(capture.timestamp)}</span>
            </div>
          </button>
        ))}
        {captures.length === 0 && <p className="empty-text">No captures found.</p>}
      </div>
    </div>
  );
}

function AlertFeed({ alerts }) {
  return (
    <div className="panel alerts-panel">
      <div className="panel-header">
        <h2>Live Alerts</h2>
      </div>
      <ul>
        {alerts.map((alert) => (
          <li key={alert.id}>
            <div className={`alert-badge ${alert.severity}`}>{alert.severity}</div>
            <div>
              <p className="alert-title">{alert.title}</p>
              <span className="alert-meta">
                <EmojiIcon symbol={ICONS.camera} label="Camera" className="alert-icon" />{' '}
                {alert.cameraName ?? alert.cameraId}
              </span>
              <span className="alert-meta">
                <EmojiIcon symbol={ICONS.clock} label="Timestamp" className="alert-icon" />
                {formatDateTime(alert.timestamp)}
              </span>
            </div>
          </li>
        ))}
        {alerts.length === 0 && <p className="empty-text">All systems normal.</p>}
      </ul>
    </div>
  );
}

function CaptureModal({ capture, onClose }) {
  if (!capture) return null;

  return (
    <div className="modal-backdrop" role="dialog" aria-modal="true">
      <div className="modal">
        <button type="button" className="close-button" onClick={onClose}>
          ×
        </button>
        <img src={capture.imageUrl ?? capture.thumbnailUrl} alt={capture.species ?? 'Wildlife capture'} />
        <div className="modal-content">
          <h3>{capture.species ?? capture.classification ?? 'Unknown species'}</h3>
          <div className="modal-meta">
            <span>
              <EmojiIcon symbol={ICONS.camera} label="Camera" className="modal-icon" />
              {capture.cameraName ?? capture.cameraId}
            </span>
            <span>
              <EmojiIcon symbol={ICONS.clock} label="Captured at" className="modal-icon" />
              {formatDateTime(capture.timestamp)}
            </span>
            <span>
              <EmojiIcon symbol={ICONS.mapPin} label="Location" className="modal-icon" />
              {capture.latitude?.toFixed?.(4) ?? capture.lat?.toFixed?.(4) ?? '—'},
              {capture.longitude?.toFixed?.(4) ?? capture.lng?.toFixed?.(4) ?? '—'}
            </span>
          </div>
          {capture.notes && <p className="modal-notes">{capture.notes}</p>}
        </div>
      </div>
    </div>
  );
}

function computeMetrics({ cameras, captures }, timeRange) {
  const cameraCount = cameras.length;
  const onlineCameras = cameras.filter((camera) => camera.status === 'online');
  const totalCaptures = captures.length;
  const averageBattery = Math.round(
    cameras.reduce((sum, camera) => sum + (camera.batteryLevel ?? 0), 0) / Math.max(cameraCount, 1)
  );

  const activityByDay = aggregateCapturesByDay(captures, timeRange);

  return {
    activeCameras: cameraCount,
    totalCaptures,
    onlineRate: Math.round((onlineCameras.length / Math.max(cameraCount, 1)) * 100),
    averageBattery,
    cameraGrowth: { direction: 'up', percentage: 4.2 },
    captureTrend: { direction: 'up', percentage: 12.4 },
    onlineTrend: { direction: 'up', percentage: 3.1 },
    batteryTrend: { direction: 'down', percentage: 1.2 },
    activityChart: buildActivityChart(activityByDay),
  };
}

function aggregateCapturesByDay(captures, timeRange) {
  const days = parseInt(timeRange, 10);
  const now = new Date();
  const startDate = new Date(now);
  startDate.setDate(startDate.getDate() - days);

  const counts = {};
  captures.forEach((capture) => {
    const timestamp = new Date(capture.timestamp);
    if (Number.isNaN(timestamp.getTime())) {
      return;
    }
    const dateKey = timestamp.toISOString().slice(0, 10);
    if (!counts[dateKey]) {
      counts[dateKey] = 0;
    }
    counts[dateKey] += 1;
  });

  const labels = [];
  const dataPoints = [];
  for (let date = new Date(startDate); date <= now; date.setDate(date.getDate() + 1)) {
    const key = date.toISOString().slice(0, 10);
    labels.push(key);
    dataPoints.push(counts[key] ?? 0);
  }

  return { labels, dataPoints };
}

function buildActivityChart({ labels, dataPoints }) {
  return {
    data: {
      labels,
      datasets: [
        {
          label: 'Captures',
          data: dataPoints,
          borderColor: '#3f51b5',
          backgroundColor: 'rgba(63, 81, 181, 0.12)',
          tension: 0.3,
          pointRadius: 2,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: {
          display: false,
        },
      },
      scales: {
        x: {
          ticks: {
            maxTicksLimit: 10,
          },
        },
        y: {
          beginAtZero: true,
        },
      },
    },
  };
}

function formatDateTime(dateLike) {
  if (!dateLike) return '—';
  const date = new Date(dateLike);
  if (Number.isNaN(date.getTime())) return '—';
  return `${date.toLocaleDateString()} ${date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}`;
}

export default WildlifeDashboard;

