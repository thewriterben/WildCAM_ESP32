# WildCAM ESP32 - Flutter Mobile App

A cross-platform mobile application for monitoring and controlling the WildCAM ESP32 wildlife monitoring system.

## Features

### Real-time Monitoring
- Live system status dashboard
- Wildlife detection alerts
- Environmental sensor readings
- Battery and connectivity status
- Camera live preview

### Wildlife Detection Management
- Browse detection gallery
- View detection details with confidence scores
- Environmental correlation data
- Image thumbnails and full-size viewing
- Share and export detections

### Camera Control
- Remote capture trigger
- Burst mode control
- Camera settings adjustment
- Live preview streaming
- Flash control

### Push Notifications
- Instant wildlife detection alerts
- Environmental alerts (temperature, air quality)
- System status notifications
- Low battery warnings
- Connection status updates

### Environmental Monitoring
- Temperature and humidity tracking
- Air quality index
- Light level monitoring
- Wildlife comfort index
- Historical data charts

## Setup Instructions

### Prerequisites

- Flutter SDK 3.10 or higher
- Dart SDK 3.0 or higher
- Android Studio / Xcode (for mobile deployment)
- Firebase account (for push notifications)

### Installation

1. **Install Flutter dependencies:**
```bash
cd ESP32WildlifeCAM-main/mobile_app/flutter_app
flutter pub get
```

2. **Configure Firebase:**

   a. Create a Firebase project at https://console.firebase.google.com
   
   b. Add Android app:
   - Download `google-services.json`
   - Place in `android/app/`
   
   c. Add iOS app:
   - Download `GoogleService-Info.plist`
   - Place in `ios/Runner/`

3. **Configure Device Connection:**

Edit `lib/config/api_config.dart`:
```dart
static String baseUrl = 'http://YOUR_DEVICE_IP';
static String deviceId = 'YOUR_DEVICE_ID';
```

### Running the App

#### Development Mode
```bash
# Connect device or start emulator
flutter run
```

#### Production Build

**Android:**
```bash
flutter build apk --release
# APK location: build/app/outputs/flutter-apk/app-release.apk
```

**iOS:**
```bash
flutter build ios --release
# Then use Xcode to sign and deploy
```

**Web:**
```bash
flutter build web --release
# Output: build/web/
```

## App Structure

```
lib/
├── main.dart                 # App entry point
├── config/
│   ├── api_config.dart      # API endpoints configuration
│   └── theme_config.dart    # App theme and styling
├── models/
│   ├── system_status.dart   # System status data model
│   ├── wildlife_detection.dart # Detection data model
│   └── environmental_data.dart # Environmental data model
├── screens/
│   ├── home_screen.dart     # Home dashboard
│   ├── dashboard_screen.dart # Real-time monitoring
│   ├── camera_screen.dart   # Camera control
│   ├── gallery_screen.dart  # Detection gallery
│   └── settings_screen.dart # App settings
├── services/
│   ├── api_service.dart     # REST API client
│   ├── websocket_service.dart # WebSocket client
│   └── notification_service.dart # Push notifications
└── widgets/
    └── (reusable UI components)
```

## API Integration

### REST API Endpoints

The app communicates with ESP32 via REST API:

```dart
GET  /api/mobile/status        # System status
POST /api/mobile/capture       # Capture image
GET  /api/mobile/preview       # Live preview
GET  /api/mobile/images        # Detection list
GET  /api/mobile/environmental # Environmental data
POST /api/mobile/settings      # Update settings
POST /api/mobile/burst         # Burst mode control
```

### WebSocket Connection

Real-time updates via WebSocket:
- Wildlife detection alerts
- System status changes
- Environmental readings
- Connection status

### Push Notifications

Firebase Cloud Messaging for:
- Wildlife detections
- Environmental alerts
- System warnings
- Battery notifications

## Configuration

### Network Settings

Configure your ESP32 device IP in Settings screen:
1. Open app
2. Go to Settings
3. Enter Device IP address
4. Enter Device ID
5. Test connection

### Notification Preferences

Customize notifications in Settings:
- Enable/disable wildlife alerts
- Enable/disable environmental alerts
- Enable/disable system notifications
- Set quiet hours
- Adjust alert thresholds

### Camera Settings

Adjust camera parameters:
- Image quality (60-100)
- Resolution (VGA to UXGA)
- Flash mode
- Burst mode settings
- Auto-capture settings

## Troubleshooting

### Connection Issues

**Cannot connect to device:**
- Verify device and phone on same network
- Check device IP address is correct
- Ensure device web server is running
- Check firewall settings

**WebSocket disconnecting:**
- Check network stability
- Verify WebSocket port (81) is accessible
- Increase reconnect timeout in settings

### Notification Issues

**Not receiving notifications:**
- Enable notifications in phone settings
- Check app notification permissions
- Verify Firebase configuration
- Test with manual notification trigger

**Delayed notifications:**
- Check network connection quality
- Verify WebSocket is connected
- Check phone battery optimization settings

### Performance Issues

**Slow image loading:**
- Enable bandwidth optimization in settings
- Reduce image quality setting
- Use thumbnails in gallery view
- Check network speed

**App crashes:**
- Clear app cache
- Reinstall app
- Check device compatibility
- Update to latest version

## Development

### Adding New Features

1. Create model class in `lib/models/`
2. Add API method in `lib/services/api_service.dart`
3. Create screen in `lib/screens/`
4. Add navigation route
5. Update UI components

### State Management

The app uses Riverpod for state management:
```dart
import 'package:flutter_riverpod/flutter_riverpod.dart';

// Create provider
final statusProvider = FutureProvider<SystemStatus>((ref) async {
  final api = ref.read(apiServiceProvider);
  return api.getSystemStatus();
});

// Use in widget
Consumer(
  builder: (context, ref, child) {
    final statusAsync = ref.watch(statusProvider);
    return statusAsync.when(
      data: (status) => Text(status.connectionType),
      loading: () => CircularProgressIndicator(),
      error: (error, stack) => Text('Error: $error'),
    );
  },
);
```

### Testing

```bash
# Run unit tests
flutter test

# Run integration tests
flutter test integration_test

# Run with coverage
flutter test --coverage
```

## Building for Production

### Android Release

1. Generate keystore:
```bash
keytool -genkey -v -keystore release-keystore.jks -keyalg RSA -keysize 2048 -validity 10000 -alias wildlife-cam
```

2. Configure `android/key.properties`:
```properties
storePassword=<password>
keyPassword=<password>
keyAlias=wildlife-cam
storeFile=<path-to-keystore>
```

3. Build:
```bash
flutter build apk --release
flutter build appbundle --release
```

### iOS Release

1. Open in Xcode:
```bash
open ios/Runner.xcworkspace
```

2. Configure signing in Xcode
3. Archive and upload to App Store

## License

MIT License - See LICENSE file for details

## Support

For issues and questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Discussions: https://github.com/thewriterben/WildCAM_ESP32/discussions

---

**Version**: 1.0.0
**Last Updated**: 2025-01-02
