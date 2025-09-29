# ESP32 Wildlife Camera Mobile App

A comprehensive mobile application suite for remote control and monitoring of the ESP32 Wildlife Camera system. This implementation includes both Progressive Web App (PWA) capabilities and a native Flutter mobile application.

## 🚀 Features

### Progressive Web App (PWA)
- **Offline Functionality**: Service worker with comprehensive caching strategy
- **Mobile Optimizations**: Touch gestures, pull-to-refresh, swipe navigation
- **Install Prompts**: Native app-like installation on mobile devices
- **Background Sync**: Offline actions synced when connection is restored
- **Push Notifications**: Wildlife detection alerts and system notifications

### Flutter Mobile App
- **Cross-Platform**: iOS, Android, and Web support
- **Real-Time Communication**: WebSocket integration for live updates
- **Comprehensive UI**: Dashboard, camera control, gallery, and settings
- **Push Notifications**: Multi-channel notification system
- **Theme Support**: Light/dark modes with system preference detection

### Backend Integration
- **Mobile API Endpoints**: Bandwidth-optimized APIs for mobile devices
- **WebSocket Communication**: Real-time bidirectional communication
- **Power Management**: Battery optimization and low-power mode support
- **Image Processing**: Mobile-optimized image thumbnails and previews

## 📱 Mobile App Components

### Screens
- **Dashboard**: Real-time system monitoring with live charts and status cards
- **Camera**: Live preview, capture controls, and camera settings
- **Gallery**: Image management with grid/list view and filtering
- **Settings**: Configuration, notifications, and connection management

### Services
- **API Service**: RESTful communication with ESP32 backend
- **WebSocket Service**: Real-time data streaming and control
- **Notification Service**: Push notifications for wildlife detections
- **Storage Service**: Local data caching and preferences

### Widgets
- **Status Cards**: System status indicators with color coding
- **Battery Indicator**: Custom battery level visualization
- **Wildlife Detection Cards**: Detection history with confidence levels
- **Camera Controls**: Touch-optimized capture and settings controls

## 🔧 Backend Mobile API

### Mobile-Optimized Endpoints
```
GET  /api/mobile/status        - System status with reduced data
POST /api/mobile/capture       - Photo capture with mobile modes
GET  /api/mobile/preview       - Live camera preview
GET  /api/mobile/images        - Paginated image list
GET  /api/mobile/thumbnail     - Optimized thumbnails
GET  /api/mobile/settings      - Camera and app settings
POST /api/mobile/burst         - Burst mode control
```

### WebSocket Communication
- Real-time wildlife detection alerts
- System status updates
- Battery and power notifications
- Connection status monitoring
- Camera preview streaming

## 🛠 Installation & Setup

### PWA Installation
1. Open the web dashboard in a mobile browser
2. Tap "Add to Home Screen" when prompted
3. The PWA will install as a native-like app

### Flutter App Development
```bash
# Install Flutter dependencies
cd mobile_app/flutter_app
flutter pub get

# Run on connected device
flutter run

# Build for production
flutter build apk        # Android
flutter build ios        # iOS
flutter build web        # Web
```

### Backend Integration
1. Include mobile API headers in enhanced web server:
```cpp
#include "mobile_app/firmware/src/mobile/mobile_api.h"
#include "mobile_app/firmware/src/mobile/mobile_websocket.h"
```

2. Initialize mobile services in setup():
```cpp
g_mobileAPI = new MobileAPI();
g_mobileWebSocket = new MobileWebSocketHandler();
```

## 📋 Configuration

### Mobile API Configuration
```cpp
MobileAPIConfig config;
config.enableBandwidthOptimization = true;
config.maxImageSize = 1024 * 1024;  // 1MB
config.thumbnailSize = 150;         // 150x150px
config.updateInterval = 3000;       // 3 seconds
config.enableBurstMode = true;
config.maxBurstCount = 10;
```

### PWA Manifest
- App name and icons
- Display modes and orientation
- Background/theme colors
- Shortcuts and file handlers
- Share targets and protocol handlers

### Flutter Dependencies
- **UI Framework**: Flutter 3.0+ with Material Design 3
- **State Management**: Riverpod for reactive state management
- **Networking**: Dio for HTTP requests, WebSocket for real-time communication
- **Notifications**: Flutter Local Notifications + Awesome Notifications
- **Storage**: Hive for local storage, SharedPreferences for settings
- **Charts**: FL Chart for system performance visualization

## 🔔 Notification System

### Notification Channels
- **Wildlife Detections**: High priority with custom sounds
- **System Alerts**: Medium priority for errors and warnings
- **Battery Alerts**: High priority for critical battery levels

### Notification Types
- Wildlife detection with species and confidence
- Low battery warnings (20%, 10%, 5%)
- System errors and connection issues
- Motion detection alerts
- Storage space warnings

## 🎨 UI/UX Features

### Mobile Optimizations
- **Touch Targets**: Minimum 44px for accessibility
- **Gestures**: Pull-to-refresh, swipe navigation, long-press menus
- **Responsive Design**: Adaptive layouts for different screen sizes
- **Performance**: Hardware acceleration and smooth animations

### Accessibility
- Screen reader support
- High contrast mode compatibility
- Reduced motion support
- Keyboard navigation

## 🔒 Security Features

### Data Protection
- Local storage encryption for sensitive data
- Secure WebSocket connections (WSS)
- Input validation and sanitization
- XSS protection headers

### Network Security
- WiFi network validation
- Connection timeout handling
- Retry mechanisms with exponential backoff
- Error handling and graceful degradation

## 📊 Performance Metrics

### Bandwidth Optimization
- Adaptive image quality based on connection
- Compressed JSON responses
- Selective data updates
- Mobile-specific caching strategies

### Battery Optimization
- Background task management
- Efficient WebSocket connection handling
- Adaptive update intervals
- Low-power mode integration

## 🔄 Integration Points

### Existing ESP32 Systems
- **PowerManager**: Battery status and optimization
- **StorageManager**: SD card space monitoring
- **WildlifeClassifier**: AI detection results
- **MotionDetector**: Real-time motion alerts
- **EnhancedWebServer**: Unified API endpoint management

### Data Flow
1. ESP32 collects sensor data and camera images
2. Mobile API processes and optimizes data for mobile consumption
3. WebSocket broadcasts real-time updates to connected mobile clients
4. Flutter app receives updates and triggers appropriate UI changes
5. Push notifications alert users of important events

## 🚀 Future Enhancements

### Planned Features
- Video streaming support
- Cloud backup integration
- Machine learning model updates
- Advanced analytics dashboard
- Multi-camera support
- Geographic mapping integration

### Technical Improvements
- WebRTC integration for video streaming
- GraphQL API for more efficient data fetching
- Offline ML inference capabilities
- Advanced image processing features
- IoT device management dashboard

## 📄 File Structure

```
mobile_app/
├── pwa/                          # Progressive Web App
│   ├── service-worker.js         # Offline functionality
│   ├── mobile-optimizations.js   # Touch controls & gestures
│   └── mobile-styles.css         # Mobile-responsive styles
├── flutter_app/                  # Native mobile app
│   ├── lib/
│   │   ├── main.dart             # App entry point
│   │   ├── screens/              # UI screens
│   │   ├── services/             # Backend communication
│   │   ├── widgets/              # Reusable UI components
│   │   └── models/               # Data models
│   └── pubspec.yaml              # Dependencies
└── firmware/src/mobile/          # Backend mobile API
    ├── mobile_api.h/cpp          # Mobile-optimized endpoints
    └── mobile_websocket.h/cpp    # Real-time communication
```

## 🔗 API Documentation

### Mobile Status Endpoint
```json
GET /api/mobile/status

Response:
{
  "battery": 85,
  "charging": false,
  "camera_ready": true,
  "wifi_connected": true,
  "wifi_signal": -45,
  "free_storage": 2048,
  "image_count": 127,
  "monitoring": true,
  "last_species": "deer",
  "last_confidence": 0.92,
  "last_detection": 1640995200000,
  "alerts": {
    "low_battery": false,
    "low_storage": false,
    "error": false
  }
}
```

### Mobile Capture Endpoint
```json
POST /api/mobile/capture

Request:
{
  "mode": "single|burst|timer|preview",
  "quality": 85,
  "burst_count": 5,
  "timer_delay": 5
}

Response:
{
  "success": true,
  "message": "Capture initiated",
  "image_id": "IMG_20231201_143022.jpg"
}
```

This mobile app implementation provides a complete, production-ready solution for remote wildlife camera control with modern mobile app features and seamless ESP32 integration.