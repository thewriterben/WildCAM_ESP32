# WildCAM Mobile App Framework

## Overview

The WildCAM Mobile App Framework provides comprehensive companion applications for iOS and Android platforms, enabling real-time monitoring, push notifications, and remote configuration of WildCAM ESP32 wildlife camera systems.

## Features

### 📱 Cross-Platform Support
- **iOS**: Native Swift/SwiftUI application with ARKit support
- **Android**: Native Kotlin application with ARCore support
- **React Native**: Cross-platform JavaScript framework for shared functionality

### 🔔 Real-Time Notifications
- Wildlife detection alerts with species identification
- Device status notifications (online/offline)
- Low battery warnings
- Customizable notification preferences
- Quiet hours support
- Per-species alert filtering

### ⚙️ Remote Configuration
- Push configuration changes to cameras remotely
- Multiple configuration presets (Research, Monitoring, Battery-saver, Survey)
- Real-time settings synchronization
- Configuration import/export

### 📊 Analytics & Monitoring
- Live camera feeds
- Detection history and gallery
- Species analytics and activity patterns
- Device health monitoring
- Map view with camera locations

### 🦌 AR Features (Advanced)
- Species identification in AR
- 3D wildlife models
- Camera placement assistant
- Field navigation tools
- Collaborative AR sessions

## Project Structure

```
mobile/
├── ios/                          # iOS Native App
│   ├── WildCAM/
│   │   └── WildCAMApp.swift     # Main app entry point
│   ├── Models/
│   │   └── Models.swift         # Data models
│   ├── Services/
│   │   ├── APIService.swift     # API client
│   │   ├── NotificationService.swift
│   │   ├── ConfigurationService.swift
│   │   └── WebSocketService.swift
│   ├── Views/                    # SwiftUI views
│   └── Config/                   # App configuration
├── ar/                           # AR Module (Kotlin)
│   ├── ARWildlifeApp.kt
│   └── ...
└── WildlifeMonitorApp.kt        # Android Compose app

frontend/mobile/
├── App.js                        # React Native entry point
├── package.json
└── src/
    ├── screens/                  # Screen components
    ├── services/
    │   ├── APIService.js        # API client
    │   ├── NotificationService.js
    │   └── ConfigurationService.js
    ├── components/               # UI components
    ├── config/                   # Configuration
    └── hooks/                    # Custom React hooks
```

## Quick Start

### iOS Setup

1. **Prerequisites**
   - Xcode 15.0+
   - iOS 16.0+ deployment target
   - CocoaPods or Swift Package Manager

2. **Installation**
   ```bash
   cd mobile/ios
   pod install
   open WildCAM.xcworkspace
   ```

3. **Configuration**
   - Update `Info.plist` with your API URLs:
     ```xml
     <key>API_BASE_URL</key>
     <string>https://api.wildlife-monitor.com</string>
     <key>WEBSOCKET_URL</key>
     <string>wss://api.wildlife-monitor.com/ws</string>
     ```

4. **Build and Run**
   - Select your target device
   - Press Cmd+R to build and run

### Android Setup

1. **Prerequisites**
   - Android Studio Hedgehog+
   - Android SDK 24+ (Android 7.0)
   - Kotlin 1.9+

2. **Installation**
   ```bash
   cd mobile
   ./gradlew build
   ```

3. **Configuration**
   - Update `local.properties` or `gradle.properties`:
     ```properties
     API_BASE_URL=https://api.wildlife-monitor.com
     WEBSOCKET_URL=wss://api.wildlife-monitor.com/ws
     ```

4. **Build and Run**
   - Connect your device or start emulator
   - Run from Android Studio

### React Native Setup

1. **Prerequisites**
   - Node.js 18+
   - npm or yarn
   - Expo CLI (optional)

2. **Installation**
   ```bash
   cd frontend/mobile
   npm install
   ```

3. **Run Development Server**
   ```bash
   # iOS
   npm run ios
   
   # Android
   npm run android
   
   # Expo
   npm start
   ```

## API Integration

### Authentication

The mobile apps support JWT-based authentication:

```swift
// iOS
APIService.shared.setAuthToken("your-jwt-token")

// React Native
APIService.setAuthToken("your-jwt-token")
```

### Push Notifications

#### iOS (Swift)

```swift
// Request permission
NotificationService.shared.requestAuthorization()

// Show detection notification
NotificationService.shared.showWildlifeDetectionNotification(detection: detection)

// Update preferences
NotificationService.shared.wildlifeAlertsEnabled = true
NotificationService.shared.lowBatteryEnabled = true
```

#### React Native

```javascript
import NotificationService from './services/NotificationService';

// Initialize
NotificationService.initialize({
  onNotification: (notification) => {
    console.log('Notification received:', notification);
  },
  onRegistration: (token) => {
    console.log('Device registered:', token);
  }
});

// Show local notification
NotificationService.showDetectionNotification({
  species: 'White-tailed Deer',
  confidence: 0.95,
  camera_id: 'CAM_001'
});
```

### Remote Configuration

#### iOS (Swift)

```swift
// Get configuration
let config = ConfigurationService.shared

// Set values
config.captureInterval = 300
config.motionSensitivity = 70

// Apply preset
config.applyPreset(ConfigurationService.presets[0])

// Push to device
Task {
    try await config.pushConfigurationToDevice(deviceId: "CAM_001")
}
```

#### React Native

```javascript
import ConfigurationService from './services/ConfigurationService';

// Initialize
await ConfigurationService.initialize();

// Get/Set values
const interval = ConfigurationService.getValue('captureInterval');
await ConfigurationService.setConfigValue('captureInterval', 600);

// Apply preset
await ConfigurationService.applyPreset('longTermMonitoring');

// Push to device
await ConfigurationService.pushConfigToDevice('CAM_001');
```

### WebSocket Real-Time Updates

#### iOS (Swift)

```swift
// Connect to WebSocket
WebSocketService.shared.connect()

// Subscribe to detections
WebSocketService.shared.detectionSubject
    .sink { detection in
        print("New detection: \(detection)")
    }
    .store(in: &cancellables)

// Subscribe to specific device
WebSocketService.shared.subscribeToDevice(deviceId: "CAM_001")
```

#### React Native

```javascript
import io from 'socket.io-client';

const socket = io('wss://api.wildlife-monitor.com', {
  transports: ['websocket']
});

socket.on('new_detection', (detection) => {
  console.log('New detection:', detection);
  NotificationService.showDetectionNotification(detection);
});

socket.on('device_status_update', (status) => {
  console.log('Device status:', status);
});
```

## Configuration Presets

| Preset | Capture Interval | Motion Sensitivity | JPEG Quality | Frame Size |
|--------|-----------------|-------------------|--------------|------------|
| High Quality Research | 300s | 70% | 8 | UXGA |
| Long-term Monitoring | 600s | 50% | 15 | SXGA |
| Maximum Battery Life | 1800s | 40% | 20 | SVGA |
| Quick Survey | 60s | 80% | 12 | XGA |

## Backend API Endpoints

### Push Notifications

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/v1/push/register` | Register device for push notifications |
| DELETE | `/api/v1/push/unregister` | Unregister device |
| GET/POST | `/api/v1/push/preferences` | Get/Update notification preferences |
| POST | `/api/v1/push/send` | Send push notification (internal) |
| GET | `/api/v1/push/stats` | Get push notification statistics |

### Configuration

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/v1/config` | Get remote configuration |
| POST | `/api/v1/config` | Update remote configuration |
| POST | `/api/v1/devices/{id}/config` | Push config to specific device |

## Notification Types

| Type | Description | Default Enabled |
|------|-------------|-----------------|
| `detection` | Wildlife detection alerts | Yes |
| `device_status` | Camera online/offline alerts | Yes |
| `low_battery` | Low battery warnings | Yes |
| `system_alert` | System notifications | Yes |

## Testing

### iOS Unit Tests
```bash
cd mobile/ios
xcodebuild test -workspace WildCAM.xcworkspace -scheme WildCAM
```

### React Native Tests
```bash
cd frontend/mobile
npm test
```

## Troubleshooting

### Push Notifications Not Working

1. **iOS**: Check that push notification capabilities are enabled in Xcode
2. **Android**: Verify Firebase configuration in `google-services.json`
3. **Backend**: Check device registration with `/api/v1/push/stats`

### WebSocket Connection Issues

1. Check network connectivity
2. Verify WebSocket URL in configuration
3. Check server logs for connection errors

### Configuration Not Syncing

1. Verify API connectivity
2. Check authentication token validity
3. Ensure device is online

## Security Considerations

- All API calls use HTTPS
- JWT tokens for authentication
- Push notification tokens are device-specific
- Sensitive data is not stored in notification payloads
- Configuration changes are validated before applying

## Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines on contributing to the mobile app framework.

## License

This project is licensed under the MIT License - see [LICENSE](../LICENSE) for details.

## Support

- GitHub Issues: [WildCAM_ESP32 Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Documentation: See `/docs` directory
- Community: Join our Discord server
