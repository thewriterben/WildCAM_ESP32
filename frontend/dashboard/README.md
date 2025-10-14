# 🦌 WildCAM Advanced Dashboard - Power User Interface

**Professional-grade real-time wildlife monitoring dashboard for ESP32 WildCAM v2.0**

[![Version](https://img.shields.io/badge/version-3.0.0-green.svg)](https://github.com/thewriterben/WildCAM_ESP32)
[![React](https://img.shields.io/badge/React-18.2.0-blue.svg)](https://reactjs.org/)
[![Material-UI](https://img.shields.io/badge/Material--UI-5.8.6-blueviolet.svg)](https://mui.com/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## 🌟 Overview

The WildCAM Advanced Dashboard is a comprehensive, full-featured web interface designed for power users who need deep insights into their wildlife monitoring systems. Built with React and Material-UI, it provides real-time monitoring, advanced analytics, device management, and customizable visualizations.

### Key Features

- 🔴 **Real-Time Monitoring** - Live detection feeds with WebSocket updates
- 📊 **Advanced Analytics** - Species distribution, activity patterns, and trends
- 🎛️ **Device Management** - Multi-camera network control and configuration
- 📈 **Interactive Visualizations** - Customizable charts using Recharts and D3.js
- 🔐 **Role-Based Access Control** - Granular permissions and user management
- 🌓 **Theme Customization** - Dark/light modes with custom color schemes
- 📱 **Responsive Design** - Optimized for desktop, tablet, and mobile
- 🔌 **Extensible Architecture** - Modular components for easy customization

## 📋 Table of Contents

- [Quick Start](#-quick-start)
- [Installation](#-installation)
- [Architecture](#-architecture)
- [Features](#-features)
- [Configuration](#-configuration)
- [Customization](#-customization)
- [API Integration](#-api-integration)
- [Deployment](#-deployment)
- [Troubleshooting](#-troubleshooting)
- [Advanced Usage](#-advanced-usage)

## 🚀 Quick Start

### Prerequisites

- Node.js 16+ and npm/yarn
- Backend API server running (see [backend README](../../backend/README.md))
- Modern web browser (Chrome, Firefox, Safari, Edge)

### Installation

```bash
# Navigate to dashboard directory
cd frontend/dashboard

# Install dependencies
npm install

# Configure environment
cp .env.example .env
# Edit .env with your API endpoint

# Start development server
npm start
```

The dashboard will be available at `http://localhost:3000`

### First-Time Setup

1. **Configure API Endpoint**: Edit `.env` and set `REACT_APP_API_URL` to your backend URL
2. **Login**: Use your credentials to access the dashboard
3. **Add Cameras**: Register your ESP32 devices in the Device Management section
4. **Start Monitoring**: View live detections and analytics

## 🏗️ Architecture

### Component Structure

```
frontend/dashboard/
├── src/
│   ├── App.js                      # Main application with routing
│   ├── components/                 # React components
│   │   ├── DashboardHome.js       # Overview and key metrics
│   │   ├── LiveDetections.js      # Real-time detection feed
│   │   ├── SpeciesAnalytics.js    # Advanced analytics & charts
│   │   ├── CameraManagement.js    # Device control & config
│   │   ├── CollaborationPanel.js  # Multi-user features
│   │   ├── RBACExample.js         # Permission examples
│   │   └── collaboration/         # Collaboration components
│   ├── services/                   # API clients
│   │   ├── api.js                 # REST API client
│   │   ├── WebSocketService.js    # Real-time updates
│   │   └── collaborationApi.js    # Collaboration API
│   ├── hooks/                      # Custom React hooks
│   │   └── useRBAC.js             # Role-based access control
│   └── public/                     # Static assets
│       └── manifest.json           # PWA configuration
├── package.json                    # Dependencies
├── Dockerfile                      # Development container
└── Dockerfile.prod                 # Production container
```

### Technology Stack

| Technology | Purpose | Version |
|-----------|---------|---------|
| React | UI Framework | 18.2.0 |
| Material-UI | Component Library | 5.8.6 |
| React Router | Navigation | 6.3.0 |
| Recharts | Data Visualization | 2.5.0 |
| D3.js | Advanced Charts | 7.6.1 |
| Socket.io | Real-Time Updates | 4.5.0 |
| Axios | HTTP Client | 0.27.2 |
| Leaflet | Maps | 1.8.0 |
| Moment.js | Date/Time | 2.29.4 |

## 📊 Features

### 1. Dashboard Home

**Overview of your entire wildlife monitoring system**

- **System Statistics**: Total cameras, active devices, detection count, species identified
- **Recent Detections**: Latest wildlife captures with thumbnails and metadata
- **Species Distribution**: Pie chart showing species frequency
- **Activity Patterns**: Bar chart of detection activity by hour
- **Camera Status**: Real-time health indicators for all devices
- **Battery & Storage**: Monitor power levels and storage capacity

**Key Metrics Displayed:**
- Total cameras and active device count
- Total detections (today, week, month)
- Unique species identified
- Average battery level across network
- Storage usage statistics
- System uptime and reliability

### 2. Live Detections

**Real-time wildlife detection feed with filtering and search**

- **Live Feed**: WebSocket-powered real-time updates
- **Image Gallery**: High-quality thumbnails with lightbox preview
- **Filtering**: By species, camera, date range, confidence score
- **Search**: Full-text search across detection metadata
- **Pagination**: Efficient loading of large datasets
- **Details View**: Click any detection for detailed information
- **Export**: Download detection data in JSON format

**Features:**
- Auto-refresh on new detections
- Confidence score badges
- Timestamp and location data
- Camera identification
- Detection metadata (weather, temperature, etc.)

### 3. Species Analytics

**Advanced analytics and insights into wildlife behavior**

- **Species Distribution Chart**: Interactive pie chart with species percentages
- **Activity Heatmap**: Hourly activity patterns by species
- **Confidence Distribution**: Detection accuracy metrics
- **Temporal Analysis**: Daily, weekly, and monthly trends
- **Comparative View**: Compare species activity across time periods
- **Statistical Summary**: Mean, median, and mode calculations

**Visualization Types:**
- Pie charts for distribution
- Bar charts for frequency
- Line charts for trends
- Heatmaps for patterns
- Area charts for cumulative data

### 4. Camera Management

**Centralized control and configuration of your camera network**

- **Device List**: All registered cameras with status
- **Remote Configuration**: Adjust settings without physical access
- **Firmware Updates**: OTA (Over-The-Air) update management
- **Health Monitoring**: Battery, storage, connectivity status
- **Command Queue**: Schedule commands and actions
- **Batch Operations**: Configure multiple cameras simultaneously

**Configurable Settings:**
- Motion sensitivity (1-100)
- Photo quality (low/medium/high)
- Detection threshold (0.1-1.0)
- Capture interval (seconds)
- Night vision mode
- Power save options

### 5. Collaboration Features

**Multi-user access and team coordination**

- **User Management**: Role-based access control
- **Shared Annotations**: Collaborative wildlife identification
- **Comments & Notes**: Team communication on detections
- **Activity Feed**: Track team member actions
- **Permission Levels**: Admin, Researcher, Viewer roles

## ⚙️ Configuration

### Environment Variables

Create a `.env` file in the dashboard directory:

```bash
# API Configuration
REACT_APP_API_URL=http://localhost:5000
REACT_APP_WS_URL=ws://localhost:5000

# Feature Flags
REACT_APP_ENABLE_COLLABORATION=true
REACT_APP_ENABLE_MAPS=true
REACT_APP_ENABLE_EXPORT=true

# UI Configuration
REACT_APP_THEME_MODE=dark
REACT_APP_ITEMS_PER_PAGE=20
REACT_APP_AUTO_REFRESH_INTERVAL=30000

# Map Configuration (if using maps)
REACT_APP_MAP_TILE_URL=https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png
REACT_APP_DEFAULT_LAT=40.7128
REACT_APP_DEFAULT_LNG=-74.0060
REACT_APP_DEFAULT_ZOOM=10
```

### Theme Customization

Edit `src/App.js` to customize the Material-UI theme:

```javascript
const theme = createTheme({
  palette: {
    mode: 'dark', // 'light' or 'dark'
    primary: {
      main: '#2e7d32', // Custom primary color
    },
    secondary: {
      main: '#1976d2',
    },
    background: {
      default: '#0a1929',
      paper: '#132f4c',
    },
  },
  typography: {
    fontFamily: '"Roboto", "Helvetica", "Arial", sans-serif',
    h1: {
      fontSize: '2.5rem',
      fontWeight: 600,
    },
    // ... customize other typography
  },
  components: {
    MuiButton: {
      styleOverrides: {
        root: {
          borderRadius: 8,
          textTransform: 'none',
        },
      },
    },
    // ... customize other components
  },
});
```

### Dashboard Layout

Configure the dashboard layout and navigation in `src/App.js`:

```javascript
const menuItems = [
  { id: 'dashboard', label: 'Dashboard', icon: <Dashboard /> },
  { id: 'detections', label: 'Live Detections', icon: <PhotoCamera /> },
  { id: 'analytics', label: 'Species Analytics', icon: <Analytics /> },
  { id: 'devices', label: 'Device Management', icon: <DeviceHub /> },
  { id: 'map', label: 'Map View', icon: <Map /> },
  { id: 'timeline', label: 'Activity Timeline', icon: <Timeline /> },
  { id: 'settings', label: 'Settings', icon: <Settings /> },
];
```

## 🎨 Customization

### Adding Custom Components

Create a new component in `src/components/`:

```javascript
// src/components/CustomAnalytics.js
import React, { useState, useEffect } from 'react';
import { Card, CardContent, Typography } from '@mui/material';
import apiService from '../services/api';

function CustomAnalytics() {
  const [data, setData] = useState(null);

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    const result = await apiService.getCustomData();
    setData(result);
  };

  return (
    <Card>
      <CardContent>
        <Typography variant="h5">Custom Analytics</Typography>
        {/* Your custom content */}
      </CardContent>
    </Card>
  );
}

export default CustomAnalytics;
```

### Custom Hooks

Create custom hooks in `src/hooks/`:

```javascript
// src/hooks/useDetections.js
import { useState, useEffect } from 'react';
import apiService from '../services/api';

function useDetections(filters = {}) {
  const [detections, setDetections] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    loadDetections();
  }, [filters]);

  const loadDetections = async () => {
    try {
      setLoading(true);
      const result = await apiService.getDetections(filters);
      setDetections(result.detections || []);
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  return { detections, loading, error, refresh: loadDetections };
}

export default useDetections;
```

### Adding New Routes

Update `src/App.js` to add new pages:

```javascript
import CustomPage from './components/CustomPage';

// In the renderSelectedPage function:
case 'custom':
  return <CustomPage />;
```

## 🔌 API Integration

### REST API Client

The dashboard uses `src/services/api.js` for all API communication:

```javascript
import apiService from './services/api';

// Authentication
await apiService.login(username, password);
await apiService.logout();

// Cameras
const cameras = await apiService.getCameras();
await apiService.registerCamera(cameraData);

// Detections
const detections = await apiService.getDetections({ per_page: 50 });
await apiService.uploadDetection(formData);

// Analytics
const speciesData = await apiService.getSpeciesAnalytics(30); // last 30 days
const activityData = await apiService.getActivityPatterns(7); // last 7 days
```

### WebSocket Integration

Real-time updates use `src/services/WebSocketService.js`:

```javascript
import WebSocketService from './services/WebSocketService';

// Subscribe to detections
WebSocketService.subscribeToDetections((detection) => {
  console.log('New detection:', detection);
});

// Subscribe to device updates
WebSocketService.subscribeToDeviceUpdates((update) => {
  console.log('Device update:', update);
});

// Unsubscribe when component unmounts
WebSocketService.off('new_detection', handler);
```

### API Endpoints

The dashboard expects the following backend endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/auth/login` | POST | User authentication |
| `/api/auth/logout` | POST | User logout |
| `/api/cameras` | GET | List all cameras |
| `/api/cameras` | POST | Register new camera |
| `/api/detections` | GET | List detections |
| `/api/detections` | POST | Upload detection |
| `/api/analytics/species` | GET | Species analytics |
| `/api/analytics/activity` | GET | Activity patterns |
| `/api/alerts` | GET | System alerts |

See [API_REFERENCE.md](../../docs/API_REFERENCE.md) for complete API documentation.

## 🐳 Deployment

### Docker Development

```bash
# Build development image
docker build -t wildcam-dashboard:dev -f Dockerfile .

# Run container
docker run -p 3000:3000 \
  -e REACT_APP_API_URL=http://api.example.com \
  wildcam-dashboard:dev
```

### Docker Production

```bash
# Build production image
docker build -t wildcam-dashboard:prod -f Dockerfile.prod .

# Run container
docker run -p 80:80 \
  -e REACT_APP_API_URL=https://api.example.com \
  wildcam-dashboard:prod
```

### Docker Compose

Use with the complete stack (see root `docker-compose.yml`):

```bash
# Start all services
docker-compose up -d

# View logs
docker-compose logs -f dashboard

# Stop services
docker-compose down
```

### Production Build

```bash
# Create optimized production build
npm run build

# Serve static files with nginx
# Build output is in the 'build' directory
```

### Nginx Configuration

```nginx
server {
    listen 80;
    server_name dashboard.wildcam.example.com;
    root /usr/share/nginx/html;
    index index.html;

    # Gzip compression
    gzip on;
    gzip_types text/plain text/css application/json application/javascript;

    location / {
        try_files $uri $uri/ /index.html;
    }

    # Cache static assets
    location /static/ {
        expires 1y;
        add_header Cache-Control "public, immutable";
    }

    # API proxy
    location /api/ {
        proxy_pass http://backend:5000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }

    # WebSocket proxy
    location /ws/ {
        proxy_pass http://backend:5000;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
    }
}
```

## 🔍 Troubleshooting

### Common Issues

#### Connection Failed

**Problem**: Dashboard cannot connect to backend API

**Solutions**:
1. Verify `REACT_APP_API_URL` in `.env`
2. Check backend is running: `curl http://localhost:5000/api/health`
3. Check CORS settings in backend
4. Verify network connectivity

#### WebSocket Not Connecting

**Problem**: Real-time updates not working

**Solutions**:
1. Verify `REACT_APP_WS_URL` in `.env`
2. Check WebSocket endpoint: `ws://localhost:5000/ws`
3. Verify backend WebSocket server is running
4. Check browser console for WebSocket errors

#### Charts Not Rendering

**Problem**: Analytics charts are blank or not displaying

**Solutions**:
1. Check API responses in browser Network tab
2. Verify data format matches expected structure
3. Check browser console for React errors
4. Try clearing browser cache

#### Authentication Issues

**Problem**: Cannot login or session expires quickly

**Solutions**:
1. Check credentials are correct
2. Verify JWT token expiration in backend
3. Check browser localStorage for token
4. Try clearing localStorage and re-login

#### Performance Issues

**Problem**: Dashboard is slow or unresponsive

**Solutions**:
1. Reduce `REACT_APP_AUTO_REFRESH_INTERVAL`
2. Decrease `REACT_APP_ITEMS_PER_PAGE`
3. Disable unnecessary features in `.env`
4. Check browser memory usage
5. Try disabling browser extensions

### Debug Mode

Enable debug logging in browser console:

```javascript
// In browser console
localStorage.setItem('debug', 'wildcam:*');

// Reload page to see debug logs
```

### Performance Profiling

Use React DevTools Profiler:

1. Install React DevTools browser extension
2. Open Profiler tab
3. Start recording
4. Interact with dashboard
5. Stop recording and analyze results

## 🎓 Advanced Usage

### Power User Features

#### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl/Cmd + K` | Open command palette |
| `D` | Go to Dashboard |
| `L` | Go to Live Detections |
| `A` | Go to Analytics |
| `M` | Go to Map View |
| `R` | Refresh current page |
| `T` | Toggle theme |
| `/` | Focus search |

#### Custom Filters

Create advanced filters for detections:

```javascript
const filters = {
  species: ['deer', 'bear'],
  confidence_min: 0.8,
  date_from: '2024-01-01',
  date_to: '2024-12-31',
  camera_ids: ['cam-001', 'cam-002'],
  time_of_day: 'night', // 'day', 'night', 'dawn', 'dusk'
};

const detections = await apiService.getDetections(filters);
```

#### Batch Operations

Perform bulk actions on cameras:

```javascript
import apiService from './services/api';

// Update multiple cameras
const cameraIds = ['cam-001', 'cam-002', 'cam-003'];
const config = { motion_sensitivity: 75, night_vision: true };

for (const id of cameraIds) {
  await apiService.updateCamera(id, config);
}
```

#### Data Export

Export analytics data:

```javascript
// Export detections
const detections = await apiService.getDetections({ per_page: 1000 });
const csv = convertToCSV(detections);
downloadFile(csv, 'detections.csv');

// Export species analytics
const analytics = await apiService.getSpeciesAnalytics(90);
const json = JSON.stringify(analytics, null, 2);
downloadFile(json, 'analytics.json');
```

### Custom Dashboards

Create personalized dashboard layouts:

```javascript
import GridLayout from 'react-grid-layout';

const layout = [
  { i: 'stats', x: 0, y: 0, w: 6, h: 2 },
  { i: 'chart', x: 6, y: 0, w: 6, h: 4 },
  { i: 'feed', x: 0, y: 2, w: 6, h: 6 },
];

function CustomDashboard() {
  return (
    <GridLayout layout={layout} cols={12} rowHeight={30}>
      <div key="stats"><StatsPanel /></div>
      <div key="chart"><AnalyticsChart /></div>
      <div key="feed"><DetectionFeed /></div>
    </GridLayout>
  );
}
```

### Integration Examples

#### External Alert System

Integrate with external monitoring:

```javascript
WebSocketService.subscribeToDetections((detection) => {
  if (detection.species === 'bear' && detection.confidence > 0.9) {
    // Send alert to external system
    fetch('https://alert-system.example.com/api/alert', {
      method: 'POST',
      body: JSON.stringify({
        type: 'wildlife_alert',
        species: detection.species,
        location: detection.camera_location,
        timestamp: detection.timestamp,
      }),
    });
  }
});
```

#### Custom Analytics Engine

Process detections with custom logic:

```javascript
import { groupBy, countBy, meanBy } from 'lodash';

async function customAnalytics(days = 30) {
  const detections = await apiService.getDetections({
    date_from: moment().subtract(days, 'days').format('YYYY-MM-DD'),
  });

  const bySpecies = groupBy(detections, 'species');
  const byCameraAndSpecies = {};

  Object.keys(bySpecies).forEach(species => {
    byCameraAndSpecies[species] = groupBy(bySpecies[species], 'camera_id');
  });

  const avgConfidence = meanBy(detections, 'confidence');
  const detectionsByHour = countBy(detections, d => 
    new Date(d.timestamp).getHours()
  );

  return {
    speciesDistribution: bySpecies,
    cameraSpeciesMatrix: byCameraAndSpecies,
    averageConfidence: avgConfidence,
    hourlyActivity: detectionsByHour,
  };
}
```

## 📚 Additional Resources

### Documentation

- [API Reference](../../docs/API_REFERENCE.md) - Complete API documentation
- [Architecture Guide](../../docs/ARCHITECTURE.md) - System architecture
- [Deployment Guide](../../docs/DEPLOYMENT_PRODUCTION.md) - Production deployment
- [Analytics Dashboard](../../ESP32WildlifeCAM-main/docs/ANALYTICS_DASHBOARD.md) - Analytics features

### Related Projects

- [Backend API](../../backend/) - Python Flask backend
- [Mobile App](../../frontend/mobile/) - React Native companion app
- [Firmware](../../ESP32WildlifeCAM-main/firmware/) - ESP32 camera firmware

### Community

- [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- [Contributing Guide](../../CONTRIBUTING.md)

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../../LICENSE) file for details.

## 🤝 Contributing

Contributions are welcome! Please read our [Contributing Guide](../../CONTRIBUTING.md) for details on our code of conduct and the process for submitting pull requests.

## 📧 Support

For support and questions:
- Open an issue on [GitHub](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Check existing documentation
- Review troubleshooting section

---

**Built with ❤️ for wildlife conservation**
