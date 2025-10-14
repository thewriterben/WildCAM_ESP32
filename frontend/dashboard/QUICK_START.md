# ⚡ WildCAM Dashboard - Quick Start Guide

**Get up and running with the advanced user dashboard in minutes**

## 🎯 5-Minute Setup

### Step 1: Install Dependencies

```bash
cd frontend/dashboard
npm install
```

### Step 2: Configure Environment

```bash
# Copy example configuration
cp .env.example .env

# Edit with your API endpoint
echo "REACT_APP_API_URL=http://localhost:5000" > .env
echo "REACT_APP_WS_URL=ws://localhost:5000" >> .env
```

### Step 3: Start Dashboard

```bash
npm start
```

Dashboard opens at `http://localhost:3000` 🎉

### Step 4: Login

Use your backend credentials:
- Username: `admin` (default)
- Password: `wildlife` (default)

### Step 5: Add Your First Camera

1. Navigate to **Device Management**
2. Click **Register Camera**
3. Enter camera details:
   - **Camera ID**: `CAM-001`
   - **Name**: `Front Trail Camera`
   - **Location**: `North Ridge`
4. Click **Save**

You're ready to monitor wildlife! 🦌

## 🚀 Docker Quick Start

### One-Command Deploy

```bash
docker-compose up -d
```

Access dashboard at `http://localhost:3000`

### Manual Docker

```bash
# Build
docker build -t wildcam-dashboard .

# Run
docker run -p 3000:3000 \
  -e REACT_APP_API_URL=http://backend:5000 \
  wildcam-dashboard
```

## 🔑 Key Features Overview

### Dashboard Home
- **Real-time metrics**: Active cameras, detections, species count
- **Recent detections**: Latest wildlife captures
- **Species distribution**: Pie chart of detected animals
- **Activity patterns**: Hourly detection activity

**Quick Actions**:
- `R` - Refresh dashboard
- `D` - Return to home

### Live Detections
- **Live feed**: Real-time detection updates
- **Filters**: Species, camera, date range
- **Search**: Find specific detections
- **Details**: Click any image for full info

**Quick Actions**:
- `L` - Go to detections
- `/` - Focus search
- `↑` `↓` - Navigate detections

### Species Analytics
- **Distribution charts**: Pie and bar charts
- **Time analysis**: Hourly, daily, weekly patterns
- **Confidence scores**: AI detection accuracy
- **Export data**: Download as JSON/CSV

**Quick Actions**:
- `A` - Go to analytics
- `E` - Export data

### Camera Management
- **Device list**: All registered cameras
- **Remote config**: Adjust settings remotely
- **Health status**: Battery, storage, connectivity
- **Bulk actions**: Update multiple cameras

**Quick Actions**:
- `C` - Go to cameras
- `N` - Add new camera

## 🎨 Customization Basics

### Change Theme

```javascript
// In browser console
localStorage.setItem('theme', 'light'); // or 'dark'
location.reload();
```

### Adjust Auto-Refresh

Edit `.env`:
```bash
REACT_APP_AUTO_REFRESH_INTERVAL=15000  # 15 seconds
```

### Items Per Page

Edit `.env`:
```bash
REACT_APP_ITEMS_PER_PAGE=50  # Show 50 items
```

## 🔧 Common Tasks

### Upload Detection Manually

```bash
curl -X POST http://localhost:5000/api/detections \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -F "image=@wildlife.jpg" \
  -F "camera_id=CAM-001" \
  -F "species=deer"
```

### Configure Camera Settings

```bash
curl -X PUT http://localhost:5000/api/cameras/CAM-001/config \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "motion_sensitivity": 75,
    "photo_quality": "high",
    "night_vision": true
  }'
```

### Export Analytics Data

```javascript
// In browser console
const analytics = await fetch('/api/analytics/species?days=30')
  .then(r => r.json());

const blob = new Blob([JSON.stringify(analytics, null, 2)], 
  { type: 'application/json' });
  
const url = URL.createObjectURL(blob);
const a = document.createElement('a');
a.href = url;
a.download = 'analytics.json';
a.click();
```

## 🎓 Power User Tips

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `D` | Dashboard Home |
| `L` | Live Detections |
| `A` | Analytics |
| `C` | Camera Management |
| `M` | Map View |
| `S` | Settings |
| `R` | Refresh Current Page |
| `T` | Toggle Theme |
| `/` | Focus Search |
| `Ctrl/Cmd + K` | Command Palette |
| `?` | Show Help |

### URL Parameters

Access specific views directly:

```
# View specific camera
http://localhost:3000/?camera=CAM-001

# Filter by species
http://localhost:3000/detections?species=deer

# Date range
http://localhost:3000/analytics?from=2024-01-01&to=2024-12-31

# Auto-refresh off
http://localhost:3000/?refresh=false
```

### Browser Console Commands

```javascript
// Get dashboard stats
wildcam.getStats();

// Force refresh
wildcam.refresh();

// Export detections
wildcam.exportDetections({ days: 7 });

// Get camera status
wildcam.getCameraStatus('CAM-001');

// Enable debug mode
wildcam.setDebug(true);
```

## 📊 Sample Workflows

### Morning Check Routine

1. Open Dashboard (`D`)
2. Check overnight detections count
3. Review battery levels of all cameras
4. Check for any alerts
5. View Species Analytics (`A`) for patterns

### Weekly Analysis

1. Go to Analytics (`A`)
2. Set date range to last 7 days
3. Review species distribution
4. Check activity patterns
5. Export data (`E`) for records
6. Generate summary report

### Camera Maintenance

1. Go to Camera Management (`C`)
2. Sort by battery level (lowest first)
3. Identify cameras needing maintenance
4. Review storage usage
5. Schedule maintenance visits
6. Update camera configurations

### Incident Investigation

1. Go to Live Detections (`L`)
2. Use filters:
   - Species: All
   - Date: Incident timeframe
   - Camera: Specific location
3. Review detection sequence
4. Check detection confidence
5. Export relevant images
6. Document findings

## 🐛 Quick Troubleshooting

### Cannot Connect to API

```bash
# Check API is running
curl http://localhost:5000/api/health

# Verify .env configuration
cat .env | grep API_URL

# Check firewall
sudo ufw status
```

### WebSocket Not Connecting

```bash
# Check WebSocket endpoint
wscat -c ws://localhost:5000/ws

# Verify in browser console
ws = new WebSocket('ws://localhost:5000/ws');
ws.onopen = () => console.log('Connected!');
```

### Dashboard Not Updating

1. Check browser console for errors (`F12`)
2. Verify auto-refresh is enabled
3. Check WebSocket connection status
4. Clear browser cache (`Ctrl+Shift+Del`)
5. Hard refresh (`Ctrl+Shift+R`)

### Login Issues

```bash
# Test authentication
curl -X POST http://localhost:5000/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"wildlife"}'

# Check token storage
# In browser console:
localStorage.getItem('access_token');
```

## 📚 Next Steps

### Learn More

- 📖 [Complete Documentation](./README.md)
- 🎨 [Design Guide](./DESIGN_GUIDE.md)
- ⚙️ [Configuration Guide](./CONFIGURATION_GUIDE.md)
- 🚀 [Deployment Guide](./DEPLOYMENT_GUIDE.md)

### Advanced Features

- **Custom Dashboards**: Create personalized layouts
- **API Integration**: Build custom tools
- **Automation**: Set up automated workflows
- **Alerts**: Configure custom notifications
- **Reports**: Generate automated reports

### Get Help

- 🐛 [Report Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- 💬 [Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- 📧 [Contact Support](mailto:support@wildcam.example.com)

## 🎉 You're Ready!

You now have everything you need to effectively monitor wildlife with the WildCAM advanced dashboard.

**Happy Wildlife Monitoring!** 🦌📷

---

**Quick Start Version**: 3.0.0  
**Last Updated**: 2025-10-14  
**Maintained by**: WildCAM ESP32 Team
