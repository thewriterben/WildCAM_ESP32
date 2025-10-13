# AR Mobile Interface - Quick Start Guide

## 5-Minute Setup

### 1. Add Dependencies

**build.gradle (app level):**
```gradle
dependencies {
    // AR Core
    implementation 'com.google.ar:core:1.39.0'
    implementation 'com.google.ar.sceneform.ux:sceneform-ux:1.17.1'
    
    // Coroutines
    implementation 'org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3'
    
    // Networking
    implementation 'com.squareup.retrofit2:retrofit:2.9.0'
}
```

### 2. Add Permissions

**AndroidManifest.xml:**
```xml
<uses-permission android:name="android.permission.CAMERA" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
<uses-feature android:name="android.hardware.camera.ar" android:required="true" />
```

### 3. Initialize AR System

```kotlin
import com.wildlife.monitor.ar.*

class MainActivity : AppCompatActivity() {
    private lateinit var arApp: ARWildlifeApp
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        
        // Create AR app
        arApp = ARWildlifeApp(this)
        
        // Initialize
        lifecycleScope.launch {
            val success = arApp.initialize()
            if (success) {
                // AR ready!
                startARSession()
            }
        }
    }
}
```

### 4. Use AR Features

**Species Identification:**
```kotlin
// Set mode
arApp.setMode(ARMode.SPECIES_IDENTIFICATION)

// Configure
arApp.speciesIdentifier.apply {
    minConfidenceThreshold = 0.75f
    enableOfflineMode = true
}

// Detect in AR frame
override fun onARFrameUpdate(frame: Frame) {
    lifecycleScope.launch {
        val detections = arApp.speciesIdentifier.detectSpecies(
            frame,
            frame.camera,
            getCurrentEnvironment()
        )
        
        // Show detections
        detections.forEach { detection ->
            showAROverlay(detection)
        }
    }
}
```

**Camera Placement:**
```kotlin
// Set mode
arApp.setMode(ARMode.CAMERA_PLACEMENT)

// Get recommendations
lifecycleScope.launch {
    val recs = arApp.placementAssistant.generateRecommendations(
        area = currentArea,
        targetSpecies = listOf("white_tailed_deer"),
        existingCameras = emptyList()
    )
    
    // Visualize in AR
    recs.forEach { rec ->
        visualizePlacement(rec)
    }
}
```

**AR Navigation:**
```kotlin
// Set mode
arApp.setMode(ARMode.NAVIGATION)

// Add waypoint
val waypoint = ARWaypoint(
    id = "cam_001",
    name = "Camera A-01",
    location = LatLng(40.7128, -74.0060),
    elevation = 125.5f,
    type = WaypointType.CAMERA_LOCATION,
    description = "Main trail camera",
    icon = "camera",
    color = 0xFF00FF00.toInt(),
    cloudAnchor = null,
    createdBy = "user_123",
    createdDate = Date()
)

arApp.navigationManager.addWaypoint(waypoint)
```

## Common Tasks

### Load 3D Model
```kotlin
lifecycleScope.launch {
    val model = arApp.modelManager.loadModel("white_tailed_deer", lodLevel = 0)
    if (model != null) {
        renderModel(model)
    }
}
```

### Create Collaborative Session
```kotlin
arApp.setMode(ARMode.COLLABORATION)

lifecycleScope.launch {
    val session = arApp.collaborationManager.createSession(
        name = "Field Survey",
        hostUserId = currentUserId
    )
    
    // Session ready!
}
```

### Add Field Note
```kotlin
lifecycleScope.launch {
    val note = arApp.fieldAssistant.generateFieldNote(
        audioData = recordedAudio,
        location = currentLocation,
        weather = WeatherCondition.CLEAR
    )
    
    // Note auto-generated!
}
```

### Start Tutorial
```kotlin
arApp.setMode(ARMode.EDUCATION)

val tutorial = arApp.educationManager.startTutorial("tut_001")
// Follow tutorial steps
```

## Configuration

Edit `mobile/ar/ar_config.json`:

```json
{
  "ar_config": {
    "target_frame_rate": 30,
    "enable_depth": true
  },
  "species_identification": {
    "min_confidence_threshold": 0.75,
    "enable_offline_mode": true
  },
  "performance": {
    "adaptive_quality": true,
    "battery_optimization": true
  }
}
```

## Troubleshooting

**AR not initializing:**
```kotlin
// Check availability
val availability = ArCoreApk.getInstance().checkAvailability(context)
if (!availability.isSupported) {
    showError("AR not supported")
}
```

**Low performance:**
```kotlin
// Enable battery optimization
arApp.batteryOptimization = true
arApp.minFrameRate = 20  // Lower target FPS
```

**Models not loading:**
```kotlin
// Clear cache
arApp.modelManager.clearCache()

// Check storage
val freeSpace = context.getExternalFilesDir(null)?.freeSpace
```

## Next Steps

1. Read [Implementation Guide](../../docs/AR_IMPLEMENTATION_GUIDE.md) for details
2. See [API Endpoints](../../docs/AR_API_ENDPOINTS.md) for backend integration
3. Check [Technical Specs](../../docs/AR_TECHNICAL_SPECS.md) for requirements
4. Review [3D Pipeline](../../docs/AR_3D_ASSET_PIPELINE.md) for model creation

## Full Example

```kotlin
class ARActivity : AppCompatActivity() {
    private lateinit var arApp: ARWildlifeApp
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_ar)
        
        // Initialize AR
        arApp = ARWildlifeApp(this)
        
        lifecycleScope.launch {
            if (arApp.initialize()) {
                setupAR()
            }
        }
    }
    
    private fun setupAR() {
        // Set mode
        arApp.setMode(ARMode.SPECIES_IDENTIFICATION)
        
        // Configure
        arApp.speciesIdentifier.apply {
            minConfidenceThreshold = 0.75f
            enableSizeEstimation = true
            enableOfflineMode = true
        }
        
        // Load offline guide
        lifecycleScope.launch {
            arApp.speciesIdentifier.loadOfflineFieldGuide()
        }
        
        // Start AR
        startARSession()
    }
    
    private fun startARSession() {
        // Get AR session
        val session = arApp.getARSession() ?: return
        
        // Configure AR view
        arView.setupSession(session)
        
        // Handle AR frames
        arView.scene.addOnUpdateListener { frameTime ->
            val frame = arView.arFrame ?: return@addOnUpdateListener
            arApp.onARFrameUpdate(frame)
        }
    }
    
    override fun onDestroy() {
        super.onDestroy()
        arApp.cleanup()
    }
}
```

## Resources

- **Documentation**: `/docs/AR_*.md`
- **Examples**: `/examples/ar/`
- **Config**: `mobile/ar/ar_config.json`
- **API**: [API Endpoints](../../docs/AR_API_ENDPOINTS.md)

## Support

- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Documentation: See `/docs` directory
- Community: Join Discord server

---

**Ready to go?** Start with species identification mode - it's the most commonly used feature!
