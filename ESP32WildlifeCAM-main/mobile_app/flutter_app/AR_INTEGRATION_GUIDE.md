# AR Integration Guide

## Quick Start

### Prerequisites

1. **Device Requirements**
   - iOS: iPhone 6S or later with iOS 11+
   - Android: ARCore supported device with Android 7.0+ (API level 24+)
   - Minimum 6GB RAM recommended
   - GPS capability
   - High-resolution camera

2. **Development Environment**
   - Flutter 3.10.0 or later
   - Xcode 12+ (for iOS)
   - Android Studio (for Android)
   - CocoaPods (for iOS dependencies)

### Installation Steps

1. **Install Dependencies**
   ```bash
   cd ESP32WildlifeCAM-main/mobile_app/flutter_app
   flutter pub get
   ```

2. **iOS Setup**
   ```bash
   cd ios
   pod install
   cd ..
   ```
   
   Add to `ios/Podfile`:
   ```ruby
   platform :ios, '11.0'
   ```
   
   Add to `ios/Runner/Info.plist`:
   ```xml
   <key>NSCameraUsageDescription</key>
   <string>AR wildlife identification requires camera access</string>
   <key>NSLocationWhenInUseUsageDescription</key>
   <string>AR navigation requires location access</string>
   <key>NSMicrophoneUsageDescription</key>
   <string>Voice annotations require microphone access</string>
   <key>UIRequiredDeviceCapabilities</key>
   <array>
       <string>arkit</string>
   </array>
   ```

3. **Android Setup**
   
   Add to `android/app/src/main/AndroidManifest.xml`:
   ```xml
   <uses-permission android:name="android.permission.CAMERA" />
   <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
   <uses-permission android:name="android.permission.RECORD_AUDIO" />
   
   <uses-feature android:name="android.hardware.camera.ar" android:required="true"/>
   
   <application>
       <meta-data android:name="com.google.ar.core" android:value="required" />
   </application>
   ```

### Running the App

```bash
# Run on connected device
flutter run

# Build release APK (Android)
flutter build apk --release

# Build iOS app
flutter build ios --release
```

## Architecture Overview

### Module Structure

```
lib/ar/
├── core/
│   └── ar_platform_service.dart      # Platform abstraction (ARKit/ARCore)
├── models/
│   ├── ar_species_model.dart         # Species data and 3D models
│   └── ar_detection.dart             # Detection and measurement models
├── services/
│   ├── ar_wildlife_identification_service.dart
│   ├── ar_field_guide_service.dart
│   ├── ar_navigation_service.dart
│   ├── ar_data_visualization_service.dart
│   └── ar_backend_integration.dart
├── screens/
│   └── ar_main_screen.dart           # Main AR interface
├── widgets/
│   ├── ar_overlay_widget.dart        # AR overlays
│   └── ar_controls_widget.dart       # Control interface
└── utils/
    ├── ar_audio_visualizer.dart      # Audio visualization
    └── ar_measurement_tool.dart      # Measurement utilities
```

### Data Flow

```
User → AR Screen → AR Platform Service → ARKit/ARCore
                ↓
        AR Services (Identification, Navigation, etc.)
                ↓
        Backend Integration ← → ESP32 Backend
                ↓
        AR Widgets (Overlay, Controls)
                ↓
        User (Visual Feedback)
```

## Service Integration

### 1. Wildlife Identification Service

```dart
import 'package:wildlife_camera_mobile/ar/services/ar_wildlife_identification_service.dart';

// Initialize service
final identificationService = ARWildlifeIdentificationService();
await identificationService.initialize();

// Listen for detections
identificationService.detectionStream.listen((detection) {
  print('Detected: ${detection.speciesName}');
  print('Confidence: ${detection.confidence}');
});

// Identify from camera frame
final detection = await identificationService.identifySpecies(
  imageData,
  {'imageUrl': 'path/to/image.jpg'},
);
```

### 2. Field Guide Service

```dart
import 'package:wildlife_camera_mobile/ar/services/ar_field_guide_service.dart';

final fieldGuideService = ARFieldGuideService();
await fieldGuideService.initialize();

// Get species model
final deerModel = fieldGuideService.getSpeciesModel('White-tailed Deer');

// Search species
final results = fieldGuideService.searchSpecies('deer');

// Get animations
final animations = fieldGuideService.getBehaviorAnimations('White-tailed Deer');
```

### 3. Navigation Service

```dart
import 'package:wildlife_camera_mobile/ar/services/ar_navigation_service.dart';
import 'package:wildlife_camera_mobile/ar/services/ar_navigation_service.dart';

final navigationService = ARNavigationService();
await navigationService.initialize();

// Add camera waypoint
navigationService.addCameraWaypoint(
  ARWaypoint(
    id: 'cam_001',
    name: 'North Trail Camera',
    latitude: 40.7128,
    longitude: -74.0060,
    type: 'camera',
  ),
);

// Get nearest waypoint
final nearest = navigationService.getNearestWaypoint();
print('Distance: ${nearest?.formattedDistance}');

// Get AR direction
final direction = navigationService.getARDirection(nearest!);
```

### 4. Data Visualization Service

```dart
import 'package:wildlife_camera_mobile/ar/services/ar_data_visualization_service.dart';

final visualizationService = ARDataVisualizationService();
await visualizationService.initialize();

// Get camera network status
final status = visualizationService.getNetworkStatus();
print('Online cameras: ${status.onlineCameras}/${status.totalCameras}');

// Generate heat map
final heatmap = visualizationService.generateHeatmap(
  startTime,
  endTime,
  species: 'White-tailed Deer',
);

// Get camera coverage
final coverage = visualizationService.getCameracoverage();
```

### 5. Backend Integration

```dart
import 'package:wildlife_camera_mobile/ar/services/ar_backend_integration.dart';

final backend = ARBackendIntegration();
await backend.initialize('http://192.168.1.100');

// Submit detection
await backend.submitARDetection(detection);

// Get camera locations
final cameras = await backend.getCameraLocations();

// Get wildlife activity
final activity = await backend.getWildlifeActivity(
  startTime: DateTime.now().subtract(Duration(days: 7)),
  endTime: DateTime.now(),
  species: 'White-tailed Deer',
);

// Get environmental data
final environmental = await backend.getEnvironmentalData();
```

## Utilities

### AR Measurement Tool

```dart
import 'package:wildlife_camera_mobile/ar/utils/ar_measurement_tool.dart';
import 'package:vector_math/vector_math_64.dart';

// Calculate distance
final distance = ARMeasurementTool.calculateDistance(
  Vector3(0, 0, 0),
  Vector3(5, 0, 0),
);

// Estimate animal size
final sizeEstimate = ARMeasurementTool.estimateAnimalSize(
  topPoint,
  bottomPoint,
  frontPoint,
  backPoint,
  cameraDistance,
);

// Compare with known species
final matches = ARMeasurementTool.compareWithKnownSpecies(
  sizeEstimate,
  SpeciesDimensionsDatabase.database,
);
print('Best match: ${matches.bestMatch?.speciesName}');
```

### AR Audio Visualizer

```dart
import 'package:wildlife_camera_mobile/ar/utils/ar_audio_visualizer.dart';

final audioVisualizer = ARAudioVisualizer();

// Start visualization
audioVisualizer.startVisualization(
  'call_001',
  Vector3(0, 1, -5),
  AudioData(
    frequency: 1.5,
    amplitude: 0.8,
    duration: 3.0,
    waveformType: WaveformType.sine,
  ),
);

// Listen for updates
audioVisualizer.updateStream.listen((update) {
  final points = audioVisualizer.generateWaveformPoints(
    update.waveform,
    64, // resolution
  );
  // Render waveform with points
});

// Get wildlife calls
final deerCalls = WildlifeCallLibrary.getCallsForSpecies('White-tailed Deer');
```

## Backend API Endpoints

The AR features integrate with the following ESP32 backend endpoints:

### AR-Specific Endpoints

```
POST   /api/ar/detections          - Submit AR detection
POST   /api/ar/measurements        - Submit AR measurement
POST   /api/ar/sessions            - Upload AR session data
POST   /api/ar/annotations/voice   - Submit voice annotation

GET    /api/cameras/locations      - Get camera GPS locations
GET    /api/activity               - Get wildlife activity data
GET    /api/environmental          - Get environmental data
GET    /api/species/:name          - Get species information
```

### Example Backend Response

```json
{
  "cameras": [
    {
      "id": "cam_001",
      "name": "North Trail Camera",
      "latitude": 40.7128,
      "longitude": -74.0060,
      "altitude": 150.0,
      "status": "online",
      "battery_level": 85
    }
  ]
}
```

## Testing

### Unit Tests

```dart
// test/ar/services/ar_platform_service_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:wildlife_camera_mobile/ar/core/ar_platform_service.dart';

void main() {
  test('AR Platform Service initialization', () async {
    final service = ARPlatformService();
    final result = await service.initialize();
    expect(result, isA<bool>());
  });
}
```

### Integration Tests

```dart
// integration_test/ar_workflow_test.dart
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  
  testWidgets('AR workflow test', (WidgetTester tester) async {
    // Test AR initialization
    // Test species identification
    // Test navigation
    // Test data visualization
  });
}
```

## Troubleshooting

### Common Issues

1. **AR not supported**
   - Verify device has ARKit/ARCore support
   - Check iOS version (11+) or Android version (7.0+)
   - Ensure proper permissions granted

2. **Poor tracking**
   - Ensure adequate lighting
   - Avoid featureless surfaces
   - Move device slowly
   - Calibrate compass

3. **GPS accuracy issues**
   - Enable high accuracy mode
   - Use external GPS if needed
   - Wait for GPS lock before starting AR

4. **Performance issues**
   - Reduce 3D model quality
   - Lower frame rate
   - Disable environmental lighting
   - Clear cached data

### Debug Mode

Enable debug logging:

```dart
// In main.dart
void main() {
  debugPrint('AR Debug Mode Enabled');
  runApp(WildlifeCAMApp());
}
```

## Performance Optimization

### Battery Management

```dart
// Reduce AR processing when battery is low
if (batteryLevel < 20) {
  // Reduce frame rate
  // Disable advanced features
  // Switch to low-power mode
}
```

### Model Optimization

- Use Level-of-Detail (LOD) for 3D models
- Progressive loading for large models
- Texture compression
- Cache frequently used models

### Network Optimization

- Batch API requests
- Use delta updates
- Implement offline mode
- Background sync when connected

## Best Practices

1. **Always check AR support** before showing AR features
2. **Request permissions** gracefully with explanations
3. **Provide fallbacks** for unsupported devices
4. **Cache data** for offline functionality
5. **Optimize battery** usage for field deployment
6. **Test on real devices** - AR doesn't work in simulators
7. **Handle errors** gracefully with user-friendly messages

## Resources

- [ARKit Documentation](https://developer.apple.com/augmented-reality/)
- [ARCore Documentation](https://developers.google.com/ar)
- [Flutter AR Plugins](https://pub.dev/packages?q=ar)
- [WildCAM ESP32 API Documentation](../../../docs/API.md)

## Support

For issues or questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Documentation: See AR_FEATURES.md for feature details

## License

This AR implementation is part of the WildCAM ESP32 project.
