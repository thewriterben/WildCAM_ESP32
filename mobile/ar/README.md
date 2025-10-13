# AR Wildlife Monitoring Module

## Overview

The AR (Augmented Reality) module provides an immersive mobile interface for wildlife monitoring, transforming field research capabilities with real-time 3D visualizations, intelligent species identification, and collaborative tools.

## Features

### 🦌 AR Species Identification
- Real-time wildlife detection using device camera
- 3D AR overlays with species information and conservation status
- Offline field guide with 500+ species
- Audio integration for species calls
- Size and age estimation using AR depth perception
- Persistent AR tags for individual animal tracking

### 🎨 3D Wildlife Models
- Photorealistic models of 200+ wildlife species
- Animated behavior demonstrations (feeding, mating, territorial displays)
- LOD (Level of Detail) system for performance optimization
- Seasonal coat variations and gender-specific models
- Interactive anatomy explorer
- Extinct species reconstructions

### 🗺️ AR Navigation
- GPS-integrated AR waypoints for camera locations
- Real-time AR compass with magnetic declination correction
- 3D terrain visualization with elevation contours
- Historical sighting heatmaps overlaid on terrain
- Virtual fence boundaries for restricted areas
- Weather overlay with real-time conditions

### 📸 Smart Camera Placement
- AR visualization of camera field-of-view
- Optimal height and angle recommendations
- Solar panel orientation guidance
- Network connectivity strength visualization
- Environmental hazard detection
- Camouflage assessment using AI
- Multi-camera coverage analysis

### 👥 Collaborative AR
- Multi-user shared AR sessions
- Real-time annotation sharing
- AR measurement tools (distance, area, height)
- Shared bookmarks for important locations
- Live video streaming with AR overlays
- Cross-platform compatibility (ARCore/ARKit)

### 🤖 AI Field Assistant
- Automatic animal detection in viewfinder
- Behavior prediction and capture timing suggestions
- Auto-generated field notes with voice-to-text
- Research protocol guidance with AR instructions
- Equipment status monitoring
- Environmental data collection

### 🎓 Education Platform
- Interactive AR tutorials
- Gamified species identification challenges
- Virtual wildlife encounters
- AR-based equipment training
- Field certification system
- Citizen science integration

## Module Structure

```
mobile/ar/
├── ARSpeciesIdentification.kt    # Species detection and field guide
├── AR3DModels.kt                  # 3D model management and rendering
├── ARNavigation.kt                # GPS waypoints and terrain
├── ARCameraPlacement.kt           # Camera placement assistant
├── ARCollaboration.kt             # Multi-user collaboration
├── AIFieldAssistant.kt            # AI-powered field tools
├── AREducation.kt                 # Training and education
├── ARWildlifeApp.kt               # Main AR application manager
└── README.md                      # This file
```

## Quick Start

### 1. Initialize AR System

```kotlin
import com.wildlife.monitor.ar.*

// Create AR app instance
val arApp = ARWildlifeApp(context)

// Initialize AR system
lifecycleScope.launch {
    val success = arApp.initialize()
    if (success) {
        // AR is ready
    }
}
```

### 2. Enable Species Identification

```kotlin
// Set AR mode to species identification
arApp.setMode(ARMode.SPECIES_IDENTIFICATION)

// Configure species identifier
arApp.speciesIdentifier.apply {
    minConfidenceThreshold = 0.75f
    enableSizeEstimation = true
    enableAgeEstimation = true
    enableOfflineMode = true
}

// Load offline field guide
lifecycleScope.launch {
    arApp.speciesIdentifier.loadOfflineFieldGuide()
}
```

### 3. Detect Species in AR Frame

```kotlin
// In AR frame update callback
override fun onARFrameUpdate(frame: Frame) {
    arApp.onARFrameUpdate(frame)
    
    // Get detections
    lifecycleScope.launch {
        val detections = arApp.speciesIdentifier.detectSpecies(
            frame,
            frame.camera,
            getCurrentEnvironmentalContext()
        )
        
        // Display AR overlays
        detections.forEach { detection ->
            displaySpeciesOverlay(detection)
        }
    }
}
```

### 4. Use Camera Placement Assistant

```kotlin
// Set mode to camera placement
arApp.setMode(ARMode.CAMERA_PLACEMENT)

// Generate placement recommendations
lifecycleScope.launch {
    val recommendations = arApp.placementAssistant.generateRecommendations(
        area = currentArea,
        targetSpecies = listOf("white_tailed_deer", "black_bear"),
        existingCameras = currentCameras
    )
    
    // Visualize recommendations in AR
    recommendations.forEach { rec ->
        visualizeRecommendation(rec)
    }
}
```

### 5. Create Collaborative Session

```kotlin
// Set mode to collaboration
arApp.setMode(ARMode.COLLABORATION)

// Create session
lifecycleScope.launch {
    val session = arApp.collaborationManager.createSession(
        name = "Field Survey 2024",
        hostUserId = currentUserId
    )
    
    // Add annotation
    val annotation = ARAnnotation(
        annotationId = "anno_1",
        createdBy = currentUserId,
        createdDate = Date(),
        position = Position3D(x, y, z),
        type = AnnotationType.TEXT_NOTE,
        content = AnnotationContent(text = "Important sighting"),
        visibility = AnnotationVisibility.TEAM_ONLY
    )
    
    arApp.collaborationManager.addAnnotation(annotation)
}
```

## Configuration

### AR Settings

```kotlin
// Configure AR session
val config = ARConfig(
    enableDepth = true,
    enableLightEstimation = true,
    enableCloudAnchors = true,
    maxModelsInMemory = 10,
    targetFrameRate = 30,
    enableOcclusion = true
)

// Apply to AR app
arApp.offlineMode = false
arApp.adaptiveQuality = true
arApp.batteryOptimization = true
arApp.minFrameRate = 30
```

### Model Management

```kotlin
// Configure model manager
arApp.modelManager.apply {
    maxCacheSize = 500 * 1024 * 1024 // 500MB
}

// Download models for offline use
lifecycleScope.launch {
    arApp.modelManager.downloadForOffline(listOf(
        "white_tailed_deer",
        "black_bear",
        "raccoon",
        "coyote"
    ))
}
```

## Requirements

### Device Requirements

- **Android**: ARCore-compatible device (Android 7.0+)
- **iOS**: ARKit-compatible device (iOS 11+)
- **RAM**: 4GB minimum, 6GB recommended
- **Storage**: 2GB free space for models and cache
- **Camera**: Required for AR functionality
- **GPS**: Required for navigation features
- **Internet**: Optional (offline mode available)

### Permissions

```xml
<uses-permission android:name="android.permission.CAMERA" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
<uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
<uses-permission android:name="android.permission.RECORD_AUDIO" />
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />

<uses-feature android:name="android.hardware.camera.ar" android:required="true" />
```

### Dependencies

```gradle
dependencies {
    // AR Core
    implementation 'com.google.ar:core:1.39.0'
    implementation 'com.google.ar.sceneform.ux:sceneform-ux:1.17.1'
    implementation 'com.google.ar.sceneform:core:1.17.1'
    
    // 3D rendering
    implementation 'com.google.android.filament:filament-android:1.38.0'
    
    // ML
    implementation 'org.tensorflow:tensorflow-lite:2.13.0'
    implementation 'org.tensorflow:tensorflow-lite-gpu:2.13.0'
    
    // Networking
    implementation 'com.squareup.retrofit2:retrofit:2.9.0'
    implementation 'com.squareup.okhttp3:okhttp:4.11.0'
    
    // Coroutines
    implementation 'org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3'
}
```

## Performance Tips

1. **Enable Adaptive Quality**: Automatically adjusts rendering quality based on performance
2. **Use LOD System**: Load appropriate model detail based on distance and device capabilities
3. **Limit Concurrent Models**: Keep maximum 5-10 models in memory simultaneously
4. **Batch Rendering**: Group similar objects for efficient rendering
5. **Enable Battery Optimization**: Reduce update rates when battery is low
6. **Preload Common Models**: Cache frequently used species models
7. **Use Offline Mode**: Download models in advance for remote locations

## Troubleshooting

### AR Not Initializing

```kotlin
// Check AR availability
val availability = ArCoreApk.getInstance().checkAvailability(context)
if (!availability.isSupported) {
    // Device doesn't support ARCore
    showErrorMessage("AR not supported on this device")
}
```

### Low FPS

```kotlin
// Enable adaptive quality
arApp.adaptiveQuality = true
arApp.minFrameRate = 30

// Or manually reduce quality
arApp.modelManager.setGlobalLODLevel(+1) // Lower detail
```

### Models Not Loading

```kotlin
// Check storage and memory
val availableMemory = Runtime.getRuntime().freeMemory()
if (availableMemory < 100 * 1024 * 1024) { // 100MB
    arApp.modelManager.clearCache()
}
```

### Tracking Loss

- Ensure good lighting conditions
- Move device slowly and steadily
- Point camera at areas with visual features
- Avoid pointing at blank walls or uniform surfaces

## Integration with Backend

The AR module integrates with the WildCAM backend API for:

- Species classification and identification
- Historical detection data for heatmaps
- Terrain and elevation data
- Weather information
- Cloud anchor storage
- Collaborative session management
- Tutorial and educational content

See `/docs/AR_IMPLEMENTATION_GUIDE.md` for detailed API endpoints.

## Examples

See `/examples/ar/` directory for:

- Basic AR setup
- Species identification demo
- Camera placement tutorial
- Collaborative session example
- Educational module demo

## Testing

Run AR tests:

```bash
# Unit tests
./gradlew testDebugUnitTest

# Instrumentation tests (requires AR-capable device)
./gradlew connectedAndroidTest
```

## Contributing

When adding new AR features:

1. Follow existing code structure and naming conventions
2. Add comprehensive documentation
3. Include unit tests for logic
4. Test on multiple device types
5. Consider offline functionality
6. Optimize for performance
7. Update this README

## Resources

- [AR Implementation Guide](../../docs/AR_IMPLEMENTATION_GUIDE.md)
- [WildCAM API Documentation](../../backend/README.md)
- [3D Model Guidelines](../../docs/3D_MODEL_GUIDELINES.md)
- [Performance Optimization](../../docs/AR_PERFORMANCE.md)

## License

This module is part of the WildCAM ESP32 project and is licensed under the MIT License.

## Support

For issues and questions:
- GitHub Issues: [WildCAM_ESP32 Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Documentation: See `/docs` directory
- Community: Join our Discord server
