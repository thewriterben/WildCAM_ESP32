# Advanced AI Species Detection Integration Guide

## Overview

This guide provides step-by-step instructions for integrating the Advanced AI Species Detection system into your ESP32 Wildlife Camera project. The system provides high-accuracy identification of 150+ wildlife species with edge computing optimization.

## Quick Start

### 1. Hardware Setup

**Minimum Requirements:**
- ESP32-S3 development board (recommended) or ESP32-WROVER
- 4MB+ PSRAM
- OV2640 or OV5640 camera module
- 8MB+ Flash memory

**Recommended Setup:**
- ESP32-S3-DevKitC-1 (8MB PSRAM)
- OV5640 camera (5MP)
- MicroSD card (for logging)
- LiPo battery (2000mAh+)

### 2. Software Dependencies

Add to your `platformio.ini`:

```ini
[env:esp32s3cam]
platform = espressif32@6.4.0
board = esp32s3dev
framework = arduino

build_flags = 
    -DBOARD_HAS_PSRAM
    -DCONFIG_SPIRAM_CACHE_WORKAROUND
    -DARDUINO_USB_MODE=1
    -DARDUINO_USB_CDC_ON_BOOT=1
    -std=gnu++11

lib_deps = 
    tensorflow/TensorFlow Lite for Microcontrollers@^2.4.0
```

### 3. Include Headers

```cpp
#include "firmware/src/ai/advanced_species_detection.h"
#include "firmware/ml_models/ai_detection_interface.h"
```

### 4. Basic Integration

```cpp
// Create detector instance
AdvancedSpeciesDetector detector;

// Configure
AdvancedDetectionConfig config;
config.modelPath = "wildlife_research_v3.tflite";
config.confidenceThreshold = 0.80f;
config.enableEdgeOptimization = true;

// Initialize
if (!detector.initialize(config)) {
    Serial.println("Failed to initialize detector");
    return;
}

// Capture and detect
CameraFrame frame = captureFrame();
AdvancedSpeciesResult result = detector.detectSpecies(frame);

if (result.isValidDetection) {
    Serial.printf("Detected: %s (%.1f%%)\n",
                 advancedSpeciesToString(result.primarySpecies),
                 result.primaryConfidence * 100.0f);
}
```

## Detailed Integration Steps

### Step 1: Project Setup

1. **Clone the repository** (if not already done):
   ```bash
   git clone https://github.com/thewriterben/WildCAM_ESP32.git
   cd WildCAM_ESP32
   ```

2. **Add necessary files to your project**:
   - `firmware/src/ai/advanced_species_detection.h`
   - `firmware/src/ai/advanced_species_detection.cpp`
   - `firmware/src/ai/ai_common.h`
   - `firmware/src/ai/ai_common.cpp`

3. **Update your build configuration** in `platformio.ini`

### Step 2: Initialize Hardware

```cpp
void setup() {
    Serial.begin(115200);
    
    // Set CPU to maximum frequency
    setCpuFrequencyMhz(240);
    
    // Initialize camera
    if (!initCamera()) {
        Serial.println("Camera init failed");
        return;
    }
    
    // Check PSRAM
    if (!psramFound()) {
        Serial.println("WARNING: PSRAM not available");
        return;
    }
    
    Serial.printf("PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
}
```

### Step 3: Configure Advanced Detection

```cpp
AdvancedDetectionConfig config;

// Model configuration
config.modelPath = "wildlife_research_v3.tflite";
config.useHierarchicalClassification = true;
config.enableSubspeciesDetection = true;

// Performance tuning
config.confidenceThreshold = 0.80f;      // 80% minimum confidence
config.maxInferenceTime_ms = 5000;       // 5 second timeout
config.enableEdgeOptimization = true;
config.enableQuantization = true;

// Hardware settings
config.requiresPSRAM = true;
config.minCPUFreq_mhz = 240;
config.requiresESP32S3 = true;

// Feature flags
config.enableSizeEstimation = true;
config.enableAgeEstimation = true;
config.enableBehaviorContext = false;    // Optional
config.enableTemporalTracking = false;   // Optional

// Regional optimization
config.region = "north_america";
config.enableRegionalPrioritization = true;
```

### Step 4: Initialize Detector

```cpp
AdvancedSpeciesDetector detector;

if (!detector.initialize(config)) {
    Serial.println("Failed to initialize detector");
    // Handle error - check PSRAM, model file, etc.
    return;
}

// Verify initialization
if (detector.runSelfTest()) {
    Serial.println("Self-test passed");
} else {
    Serial.println("Self-test failed");
}

// Get model information
ModelInfo info = detector.getModelInfo();
Serial.printf("Model: %s v%s (%.1f%% accuracy)\n",
             info.name.c_str(), 
             info.version.c_str(),
             info.accuracy * 100.0f);
```

### Step 5: Capture and Detect

```cpp
void loop() {
    // Capture camera frame
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    // Create frame structure
    CameraFrame frame;
    frame.data = fb->buf;
    frame.length = fb->len;
    frame.width = fb->width;
    frame.height = fb->height;
    frame.format = fb->format;
    frame.timestamp = millis();
    
    // Run detection
    AdvancedSpeciesResult result = detector.detectSpecies(frame);
    
    // Process results
    if (result.isValidDetection) {
        processDetection(result);
    }
    
    // Return frame buffer
    esp_camera_fb_return(fb);
    
    delay(5000);  // Wait before next detection
}
```

### Step 6: Process Detection Results

```cpp
void processDetection(const AdvancedSpeciesResult& result) {
    // Basic information
    Serial.printf("Species: %s\n", 
                 advancedSpeciesToString(result.primarySpecies));
    Serial.printf("Confidence: %.1f%%\n", 
                 result.primaryConfidence * 100.0f);
    
    // Taxonomic classification
    Serial.printf("Scientific name: %s %s\n",
                 result.taxonomy.genus.c_str(),
                 result.taxonomy.species.c_str());
    Serial.printf("Family: %s\n", result.taxonomy.family.c_str());
    
    // Physical characteristics
    if (result.sizeEstimate_cm > 0) {
        Serial.printf("Estimated size: %.1f cm\n", result.sizeEstimate_cm);
    }
    Serial.printf("Age: %s\n", result.isJuvenile ? "Juvenile" : "Adult");
    
    // Conservation alert
    if (isEndangeredSpecies(result.primarySpecies)) {
        Serial.println("⚠️ ENDANGERED SPECIES DETECTED");
        triggerConservationAlert(result);
    }
    
    // Save to SD card or cloud
    saveDetectionData(result);
}
```

## Integration with AI Detection Interface

For seamless integration with the existing AI framework:

```cpp
#include "firmware/ml_models/ai_detection_interface.h"

WildlifeDetection::AIDetectionInterface aiInterface;

// Configure
WildlifeDetection::AIDetectionInterface::AdvancedConfig config;
config.enable_advanced_species_detection = true;
config.ai_confidence_threshold = 0.80f;
config.ai_processing_timeout_ms = 6000;

aiInterface.configureAdvancedAI(config);
aiInterface.initializeAdvancedAI();

// Check capabilities
if (aiInterface.hasAdvancedSpeciesDetection()) {
    Serial.println("Advanced 150+ species detection enabled");
    
    // List available models
    std::vector<const char*> models = aiInterface.getAvailableModels();
    for (const char* model : models) {
        Serial.printf("Available: %s\n", model);
    }
}
```

## Advanced Features

### Power Optimization

Optimize detection based on battery level:

```cpp
float batteryLevel = readBatteryLevel();  // 0.0 to 1.0

detector.optimizeForPower(batteryLevel);

// Or set specific mode
if (batteryLevel < 0.3f) {
    detector.setInferenceMode("fast");
} else if (batteryLevel < 0.7f) {
    detector.setInferenceMode("balanced");
} else {
    detector.setInferenceMode("accurate");
}
```

### Regional Adaptation

Optimize for your geographic region:

```cpp
detector.setRegion("rocky_mountains");

// Or load regional model
detector.loadRegionalModel("pacific_northwest");

// Prioritize specific species
std::vector<AdvancedSpeciesType> priority = {
    AdvancedSpeciesType::GRIZZLY_BEAR,
    AdvancedSpeciesType::GRAY_WOLF,
    AdvancedSpeciesType::MOUNTAIN_LION
};
detector.setPrioritizedSpecies(priority);
```

### Multiple Species Detection

Detect multiple animals in a single frame:

```cpp
std::vector<AdvancedSpeciesResult> results = 
    detector.detectMultipleSpecies(frame);

for (const auto& result : results) {
    if (result.isValidDetection) {
        Serial.printf("Found: %s at [%d, %d]\n",
                     advancedSpeciesToString(result.primarySpecies),
                     result.boundingBox[0],
                     result.boundingBox[1]);
    }
}
```

### Size and Age Estimation

Enable additional analysis:

```cpp
detector.enableSizeEstimation(true);
detector.enableAgeEstimation(true);

// Set reference for size calibration
detector.setReferenceObjectSize(50.0f);  // 50cm reference object

AdvancedSpeciesResult result = detector.detectSpecies(frame);

if (result.sizeEstimate_cm > 0) {
    float typicalSize = getTypicalSpeciesSize(result.primarySpecies);
    float sizeRatio = result.sizeEstimate_cm / typicalSize;
    
    if (sizeRatio < 0.7f) {
        Serial.println("Smaller than typical - possibly juvenile");
    } else if (sizeRatio > 1.3f) {
        Serial.println("Larger than typical - possibly mature male");
    }
}
```

### Hierarchical Taxonomy

Query taxonomic information:

```cpp
// Get full taxonomy for a species
TaxonomicInfo taxonomy = detector.getTaxonomy(
    AdvancedSpeciesType::WHITE_TAILED_DEER
);

Serial.printf("%s (%s %s)\n",
             taxonomy.commonName.c_str(),
             taxonomy.genus.c_str(),
             taxonomy.species.c_str());

// Get all species in a family
std::vector<AdvancedSpeciesType> cervids = 
    detector.getSpeciesByFamily("Cervidae");

Serial.printf("Found %d species in Cervidae family\n", cervids.size());

// Get all species in an order
std::vector<AdvancedSpeciesType> carnivores = 
    detector.getSpeciesByOrder("Carnivora");
```

### Performance Monitoring

Track system performance:

```cpp
AIMetrics metrics = detector.getPerformanceMetrics();

Serial.printf("Total detections: %u\n", metrics.totalInferences);
Serial.printf("Successful: %u\n", metrics.successfulInferences);
Serial.printf("Accuracy: %.1f%%\n", 
             detector.getOverallAccuracy() * 100.0f);
Serial.printf("Avg inference time: %.1f ms\n", metrics.inferenceTime);
Serial.printf("Memory usage: %u KB\n", metrics.memoryUsage / 1024);

// Reset metrics
detector.resetMetrics();
```

## Testing

Run the comprehensive test suite:

```cpp
#include "tests/test_advanced_species_detection.cpp"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    runAllTests();  // Run all validation tests
}
```

## Example Applications

### 1. Wildlife Monitoring Station

See `examples/advanced_species_detection_example.cpp` for a complete implementation.

### 2. Conservation Alert System

```cpp
void checkConservationStatus(const AdvancedSpeciesResult& result) {
    if (isEndangeredSpecies(result.primarySpecies)) {
        String status = getConservationStatus(result.primarySpecies);
        
        // Log to SD card
        logEndangeredSighting(result);
        
        // Send alert via LoRa/WiFi
        sendConservationAlert(result);
        
        // Capture high-res image
        captureHighResolution();
    }
}
```

### 3. Multi-Camera Network

```cpp
// On each camera node
void broadcastDetection(const AdvancedSpeciesResult& result) {
    DetectionMessage msg;
    msg.nodeId = getNodeId();
    msg.species = result.primarySpecies;
    msg.confidence = result.primaryConfidence;
    msg.timestamp = result.timestamp;
    msg.location = getGPSLocation();
    
    // Broadcast to mesh network
    meshNetwork.broadcast(msg);
}
```

## Troubleshooting

### Common Issues

#### 1. "PSRAM required but not found"
**Solution**: Ensure you're using ESP32-S3 or ESP32-WROVER with PSRAM enabled in platformio.ini

#### 2. Inference timeout
**Solutions**:
- Increase `maxInferenceTime_ms` in config
- Reduce image resolution
- Enable edge optimization
- Set inference mode to "fast"

#### 3. Low detection accuracy
**Solutions**:
- Ensure good lighting conditions
- Use higher resolution images
- Set correct geographic region
- Adjust confidence threshold
- Verify model is loaded correctly

#### 4. Out of memory errors
**Solutions**:
- Ensure PSRAM is available and enabled
- Reduce frame buffer count
- Lower image resolution
- Close other tasks before inference

#### 5. High power consumption
**Solutions**:
- Enable power optimization
- Reduce inference frequency
- Use "fast" mode when battery is low
- Lower CPU frequency when not inferencing

### Debug Mode

Enable detailed logging:

```cpp
detector.enableDetailedLogging(true);

// Logs will show:
// - Preprocessing details
// - Inference timing
// - Confidence scores for all classes
// - Memory usage
// - Hardware status
```

### Memory Profiling

Monitor memory usage:

```cpp
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
Serial.printf("Largest free block: %d bytes\n", 
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
```

## Performance Optimization Tips

1. **Preload Model**: Call `detector.preloadModel()` during initialization
2. **Use PSRAM**: Always use PSRAM for tensor operations
3. **Optimize Frame Size**: Use 224x224 or 320x320 for best balance
4. **Batch Processing**: Process multiple frames together when possible
5. **Regional Models**: Use region-specific models when available
6. **Power Modes**: Adjust mode based on battery level
7. **Caching**: Cache taxonomy data for faster lookups

## Best Practices

1. **Initialization**: Always check return values from `initialize()`
2. **Error Handling**: Implement proper error handling for all operations
3. **Resource Management**: Return frame buffers promptly
4. **Logging**: Log detections to SD card for later analysis
5. **Updates**: Keep model files updated for best accuracy
6. **Testing**: Run self-tests after initialization
7. **Monitoring**: Track performance metrics regularly

## API Reference

See the full API documentation in:
- `firmware/src/ai/advanced_species_detection.h`
- `firmware/src/ai/ADVANCED_SPECIES_DETECTION.md`

## Support

For issues and questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Documentation: See `docs/` directory
- Examples: See `examples/` directory

## License

This integration guide is part of the WildCAM ESP32 project and follows the same license terms.
