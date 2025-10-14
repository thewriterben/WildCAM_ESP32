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
const CAPTURE_MARKER_LIMIT = 50;

const ICONS = {
  camera: '📷',
  activity: '🦌',
  alert: '⚠️',
  wifi: '📡',
  battery: '🔋',
  refresh: '🔄',
  trendingUp: '📈',
  trendingDown: '📉',
  steady: '➖',
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

function normalizeCamera(rawCamera) {
  if (!rawCamera) return null;

  return {
    id: rawCamera.id,
    name: rawCamera.name ?? rawCamera.device_id ?? `Camera ${rawCamera.id}`,
    deviceId: rawCamera.device_id ?? rawCamera.deviceId,
    locationName: rawCamera.location_name ?? rawCamera.locationName,
    latitude: coerceNumber(rawCamera.latitude ?? rawCamera.lat ?? rawCamera.location?.lat),
    longitude: coerceNumber(rawCamera.longitude ?? rawCamera.lng ?? rawCamera.location?.lng),
    altitude: coerceNumber(rawCamera.altitude ?? rawCamera.altitudeMeters),
    lastSeen: rawCamera.last_seen ?? rawCamera.lastSeen,
    batteryLevel: coerceNumber(rawCamera.battery_level ?? rawCamera.batteryLevel),
    solarVoltage: coerceNumber(rawCamera.solar_voltage ?? rawCamera.solarVoltage),
    temperature: coerceNumber(rawCamera.temperature),
    humidity: coerceNumber(rawCamera.humidity),
    firmwareVersion: rawCamera.firmware_version ?? rawCamera.firmwareVersion,
    hardwareVersion: rawCamera.hardware_version ?? rawCamera.hardwareVersion,
    status: rawCamera.status ?? rawCamera.cameraStatus ?? 'unknown',
    createdAt: rawCamera.created_at ?? rawCamera.createdAt,
    batteryHistory: rawCamera.batteryHistory,
    statusHistory: rawCamera.statusHistory,
  };
}

function normalizeDetection(rawDetection) {
  if (!rawDetection) return null;

  return {
    id: rawDetection.id,
    cameraId: rawDetection.camera_id ?? rawDetection.cameraId,
    species: rawDetection.detected_species ?? rawDetection.species ?? rawDetection.classification,
    classification: rawDetection.detected_species ?? rawDetection.classification,
    timestamp: rawDetection.timestamp,
    imageUrl: rawDetection.image_url ?? rawDetection.imageUrl,
    thumbnailUrl: rawDetection.thumbnail_url ?? rawDetection.thumbnailUrl,
    confidence: rawDetection.confidence_score ?? rawDetection.confidence,
    notes: rawDetection.verification_notes ?? rawDetection.notes,
    latitude: coerceNumber(rawDetection.latitude ?? rawDetection.lat ?? rawDetection.location?.lat),
    longitude: coerceNumber(rawDetection.longitude ?? rawDetection.lng ?? rawDetection.location?.lng),
    cameraName: rawDetection.camera_name ?? rawDetection.cameraName,
    metadata: rawDetection.metadata,
  };
}

function normalizeAlert(rawAlert) {
  if (!rawAlert) return null;

  return {
    id: rawAlert.id,
    cameraId: rawAlert.camera_id ?? rawAlert.cameraId,
    cameraName: rawAlert.camera_name ?? rawAlert.cameraName,
    detectionId: rawAlert.detection_id ?? rawAlert.detectionId,
    severity: rawAlert.severity ?? 'info',
    title: rawAlert.title,
    message: rawAlert.message,
    timestamp: rawAlert.created_at ?? rawAlert.timestamp,
    alertType: rawAlert.alert_type ?? rawAlert.alertType,
  };
}

function coerceNumber(value) {
  if (value == null) return null;
  const parsed = typeof value === 'string' ? Number.parseFloat(value) : value;
  return Number.isFinite(parsed) ? parsed : null;
}

function getCoordinates(entity) {
  if (!entity) return { lat: null, lng: null };
  const lat = coerceNumber(entity.latitude ?? entity.lat ?? entity.location?.lat);
  const lng = coerceNumber(entity.longitude ?? entity.lng ?? entity.location?.lng);
  return {
    lat: Number.isFinite(lat) ? lat : null,
    lng: Number.isFinite(lng) ? lng : null,
  };
}

function formatCoordinates(entity, fractionDigits = 4) {
  const { lat, lng } = getCoordinates(entity);
  if (lat == null || lng == null) {
    return '—';
  }
  return `${lat.toFixed(fractionDigits)}, ${lng.toFixed(fractionDigits)}`;
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
      const requests = [
        { key: 'cameras', request: axios.get('/api/cameras') },
        { key: 'captures', request: axios.get('/api/detections', { params: { per_page: 200 } }) },
        { key: 'alerts', request: axios.get('/api/alerts') },
      ];

      const results = await Promise.allSettled(requests.map((item) => item.request));

      const nextState = { ...defaultState };
      const warnings = [];
      const criticalFailures = new Set();

      results.forEach((result, index) => {
        const { key } = requests[index];
        if (result.status === 'fulfilled') {
          const payload = result.value.data ?? [];
          if (key === 'cameras') {
            const rawCameras = payload.cameras ?? payload;
            nextState.cameras = rawCameras.map(normalizeCamera).filter(Boolean);
          } else if (key === 'captures') {
            const rawDetections = payload.detections ?? payload;
            nextState.captures = rawDetections.map(normalizeDetection).filter(Boolean);
          } else if (key === 'alerts') {
            const rawAlerts = payload.alerts ?? payload;
            nextState.alerts = rawAlerts.map(normalizeAlert).filter(Boolean);
          }
        } else {
          const message = result.reason?.response?.data?.message ?? result.reason?.message ?? key;
          warnings.push(message);
          if (key === 'cameras' || key === 'captures') {
            criticalFailures.add(key);
          }
        }
      });

      nextState.insights = deriveInsights(nextState.cameras, nextState.captures);

      setData(nextState);

      if (criticalFailures.size > 0) {
        setError('Some critical data could not be loaded. Please try refreshing.');
      } else if (warnings.length > 0) {
        setError('Some panels may be missing data, but the dashboard is still usable.');
      }
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
            symbol=
              {trend.direction === 'down'
                ? ICONS.trendingDown
                : trend.direction === 'up'
                ? ICONS.trendingUp
                : ICONS.steady}
            label=
              {trend.direction === 'down'
                ? 'Trend down'
                : trend.direction === 'up'
                ? 'Trend up'
                : 'Trend steady'}
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
              {cameras.map((camera) => {
                const { lat, lng } = getCoordinates(camera);
                if (lat == null || lng == null) {
                  return null;
                }
                return (
                  <Marker
                    key={camera.id}
                    position={[lat, lng]}
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
                );
              })}
            </LayerGroup>
          </LayersControl.Overlay>
          <LayersControl.Overlay name="Recent Captures">
            <LayerGroup>
              {captures.slice(0, CAPTURE_MARKER_LIMIT).map((capture) => {
                const { lat, lng } = getCoordinates(capture);
                if (lat == null || lng == null) {
                  return null;
                }
                return (
                  <Marker
                    key={capture.id}
                    position={[lat, lng]}
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
                );
              })}
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
        <span>{formatCoordinates(camera)}</span>
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
              {formatCoordinates(capture)}
            </span>
          </div>
          {capture.notes && <p className="modal-notes">{capture.notes}</p>}
        </div>
      </div>
    </div>
  );
}

function computeMetrics({ cameras, captures }, timeRange) {
  const currentPeriod = getCurrentPeriodMetrics({ cameras, captures }, timeRange);
  const previousPeriod = getPreviousPeriodMetrics({ cameras, captures }, timeRange);
  const activityByDay = aggregateCapturesByDay(captures, timeRange);

  return {
    activeCameras: currentPeriod.cameraCount,
    totalCaptures: currentPeriod.totalCaptures,
    onlineRate: currentPeriod.onlineRate,
    averageBattery: currentPeriod.averageBattery,
    cameraGrowth: calculateTrend(currentPeriod.cameraCount, previousPeriod.cameraCount),
    captureTrend: calculateTrend(currentPeriod.totalCaptures, previousPeriod.totalCaptures),
    onlineTrend: calculateTrend(currentPeriod.onlineRate, previousPeriod.onlineRate),
    batteryTrend: calculateTrend(currentPeriod.averageBattery, previousPeriod.averageBattery),
    activityChart: buildActivityChart(activityByDay),
  };
}

function getCurrentPeriodMetrics({ cameras, captures }, timeRange) {
  const days = Number.parseInt(timeRange, 10) || 30;
  const now = new Date();
  const periodStart = new Date(now);
  periodStart.setDate(periodStart.getDate() - days);

  const cameraCount = cameras.length;
  const onlineCameras = cameras.filter((camera) => camera.status === 'online');
  const periodCaptures = captures.filter((capture) => {
    const timestamp = new Date(capture.timestamp);
    return !Number.isNaN(timestamp.getTime()) && timestamp >= periodStart && timestamp <= now;
  });

  const totalCaptures = periodCaptures.length;
  const averageBattery = Math.round(
    cameras.reduce((sum, camera) => sum + (camera.batteryLevel ?? 0), 0) / Math.max(cameraCount, 1)
  );
  const onlineRate = Math.round((onlineCameras.length / Math.max(cameraCount, 1)) * 100);

  return {
    cameraCount,
    totalCaptures,
    onlineRate,
    averageBattery,
  };
}

function getPreviousPeriodMetrics({ cameras, captures }, timeRange) {
  const days = Number.parseInt(timeRange, 10) || 30;
  const now = new Date();
  const prevPeriodEnd = new Date(now);
  prevPeriodEnd.setDate(prevPeriodEnd.getDate() - days);
  const prevPeriodStart = new Date(prevPeriodEnd);
  prevPeriodStart.setDate(prevPeriodStart.getDate() - days);

  let previousCameraCount = cameras.length;
  if (cameras.some((camera) => camera.createdAt)) {
    previousCameraCount = cameras.filter((camera) => {
      if (!camera.createdAt) return true;
      const created = new Date(camera.createdAt);
      if (Number.isNaN(created.getTime())) return true;
      return created < prevPeriodEnd;
    }).length;
  }

  let previousOnlineCount = cameras.filter((camera) => camera.status === 'online').length;
  if (cameras.some((camera) => Array.isArray(camera.statusHistory))) {
    previousOnlineCount = cameras.filter((camera) => {
      const historyEntry = camera.statusHistory?.find((entry) => {
        const ts = new Date(entry.timestamp);
        return !Number.isNaN(ts.getTime()) && ts >= prevPeriodStart && ts < prevPeriodEnd;
      });
      return historyEntry?.status === 'online';
    }).length;
  }

  let previousBatteryAvg = Math.round(
    cameras.reduce((sum, camera) => sum + (camera.batteryLevel ?? 0), 0) / Math.max(previousCameraCount, 1)
  );
  if (cameras.some((camera) => Array.isArray(camera.batteryHistory))) {
    let sum = 0;
    let count = 0;
    cameras.forEach((camera) => {
      const historyEntry = camera.batteryHistory?.find((entry) => {
        const ts = new Date(entry.timestamp);
        return !Number.isNaN(ts.getTime()) && ts >= prevPeriodStart && ts < prevPeriodEnd;
      });
      if (historyEntry) {
        sum += historyEntry.level;
        count += 1;
      }
    });
    previousBatteryAvg = count > 0 ? Math.round(sum / count) : previousBatteryAvg;
  }

  const previousCaptures = captures.filter((capture) => {
    const timestamp = new Date(capture.timestamp);
    return !Number.isNaN(timestamp.getTime()) && timestamp >= prevPeriodStart && timestamp < prevPeriodEnd;
  }).length;

  return {
    cameraCount: previousCameraCount,
    totalCaptures: previousCaptures,
    onlineRate: Math.round((previousOnlineCount / Math.max(previousCameraCount, 1)) * 100),
    averageBattery: previousBatteryAvg,
  };
}

function calculateTrend(current, previous) {
  if (previous === 0) {
    return { direction: current > 0 ? 'up' : 'flat', percentage: current > 0 ? 100 : 0 };
  }
  const diff = current - previous;
  if (diff === 0) {
    return { direction: 'flat', percentage: 0 };
  }
  const percentage = Math.abs(diff) / previous * 100;
  return {
    direction: diff > 0 ? 'up' : 'down',
    percentage: Math.round(percentage * 10) / 10,
  };
}

function aggregateCapturesByDay(captures, timeRange) {
  const days = Number.parseInt(timeRange, 10) || 30;
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

function deriveInsights(cameras, captures) {
  if (!Array.isArray(cameras) || !Array.isArray(captures)) {
    return [];
  }

  const insights = [];
  const offlineCameras = cameras.filter((camera) => camera.status === 'offline');
  if (offlineCameras.length > 0) {
    insights.push({
      id: 'insight-offline',
      type: 'Connectivity',
      timestamp: new Date().toISOString(),
      message: `${offlineCameras.length} camera${offlineCameras.length === 1 ? '' : 's'} require attention due to offline status.`,
    });
  }

  const lowBattery = cameras.filter((camera) => (camera.batteryLevel ?? 100) < 25);
  if (lowBattery.length > 0) {
    insights.push({
      id: 'insight-battery',
      type: 'Maintenance',
      timestamp: new Date().toISOString(),
      message: `${lowBattery.length} camera${lowBattery.length === 1 ? '' : 's'} report battery below 25%. Schedule a recharge.`,
    });
  }

  if (captures.length > 0) {
    const latestCapture = captures
      .slice()
      .sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp))[0];
    if (latestCapture) {
      insights.push({
        id: `insight-capture-${latestCapture.id}`,
        type: 'Activity',
        timestamp: latestCapture.timestamp,
        message: `Most recent sighting: ${latestCapture.species ?? latestCapture.classification ?? 'Unknown species'} at camera ${
          latestCapture.cameraName ?? latestCapture.cameraId
        }.`,
      });
    }
  }

  return insights.slice(0, 5);
}

export default WildlifeDashboard;

