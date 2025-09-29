# ESP32 Wildlife Camera API Reference

This document provides comprehensive API reference for the ESP32 Wildlife Camera project, including the new AI-powered wildlife detection capabilities.

## AI and Machine Learning Classes

### TensorFlowLiteImplementation
*TensorFlow Lite Micro integration for wildlife detection.*

#### Initialization
```cpp
#include "ai/tensorflow_lite_implementation.h"

// Initialize TensorFlow Lite system
bool initializeTensorFlowLite();

// Clean up resources
void cleanupTensorFlowLite();

// Load models from directory
bool loadWildlifeModels(const char* modelsDirectory);
```

#### Methods
- `init()` - Initialize TensorFlow Lite Micro system
- `loadModel(WildlifeModelType type, const char* modelPath)` - Load specific model
- `runInference(WildlifeModelType type, const uint8_t* imageData, uint32_t width, uint32_t height)` - Run inference
- `detectSpecies(const CameraFrame& frame)` - Detect wildlife species
- `analyzeBehavior(const CameraFrame& frame)` - Analyze animal behavior
- `detectMotion(const CameraFrame& frame)` - AI-enhanced motion detection
- `detectHuman(const CameraFrame& frame)` - Human presence detection
- `validateModel(const char* modelPath)` - Validate model file
- `benchmarkModel(WildlifeModelType type, uint32_t iterations)` - Performance benchmarking

#### Model Types
```cpp
enum WildlifeModelType {
    MODEL_SPECIES_CLASSIFIER,  // Wildlife species identification
    MODEL_BEHAVIOR_ANALYZER,   // Animal behavior analysis
    MODEL_MOTION_DETECTOR,     // AI-enhanced motion detection
    MODEL_HUMAN_DETECTOR,      // Human detection
    MODEL_COUNT
};
```

#### Performance Monitoring
```cpp
// Enable performance monitoring
void enablePerformanceMonitoring(bool enable);

// Get performance metrics
uint32_t getAverageInferenceTime(WildlifeModelType type);
float getModelAccuracy(WildlifeModelType type);
size_t getMemoryUsage();
```

### WildlifeClassifier
*High-level wildlife species classification interface.*

#### Initialization
```cpp
#include "ai/wildlife_classifier.h"

WildlifeClassifier classifier;
bool success = classifier.initialize();
```

#### Methods
- `initialize()` - Initialize classifier system
- `classifyFrame(camera_fb_t* frame)` - Classify species in camera frame
- `classifyImage(const uint8_t* imageData, size_t imageSize, uint16_t width, uint16_t height)` - Classify species in image data
- `setConfidenceThreshold(float threshold)` - Set minimum confidence threshold
- `setEnabled(bool enable)` - Enable/disable classifier
- `getStatistics()` - Get classification statistics
- `resetStatistics()` - Reset performance statistics
- `cleanup()` - Clean up resources

#### Species Types
```cpp
enum class SpeciesType {
    UNKNOWN = 0,
    WHITE_TAILED_DEER = 1,
    BLACK_BEAR = 2,
    RED_FOX = 3,
    GRAY_WOLF = 4,
    MOUNTAIN_LION = 5,
    ELK = 6,
    MOOSE = 7,
    RACCOON = 8,
    COYOTE = 9,
    BOBCAT = 10,
    WILD_TURKEY = 11,
    BALD_EAGLE = 12,
    RED_TAILED_HAWK = 13,
    GREAT_BLUE_HERON = 14,
    HUMAN = 50
};
```

#### Static Utility Methods
```cpp
// Get human-readable species name
static String getSpeciesName(SpeciesType species);

// Get confidence level description
static String getConfidenceLevelDescription(ConfidenceLevel level);

// Check if species is potentially dangerous
static bool isDangerousSpecies(SpeciesType species);
```

## Core Classes

### CameraHandler
*Camera management and image capture functionality.*

#### Methods
- `init()` - Initialize camera system
- `captureImage()` - Capture a single image
- `getStatus()` - Get camera status

### MotionFilter
*Motion detection and filtering system.*

#### Methods
- `init()` - Initialize motion detection
- `checkMotion()` - Check for motion events
- `setThreshold()` - Configure motion sensitivity

### SolarManager
*Solar power management and battery monitoring.*

#### Methods
- `init()` - Initialize power management
- `getBatteryLevel()` - Get current battery level
- `getSolarVoltage()` - Get solar panel voltage

### LoraMesh
*LoRa mesh networking functionality.*

#### Methods
- `init()` - Initialize LoRa network
- `sendMessage()` - Send data over LoRa
- `receiveMessage()` - Receive LoRa messages

## Data Structures

### InferenceResult
*TensorFlow Lite inference result structure.*

```cpp
struct InferenceResult {
    float confidence;           // Confidence score (0.0-1.0)
    String className;           // Detected class name
    uint32_t classIndex;        // Class index
    uint32_t inferenceTime;     // Inference time in milliseconds
    bool valid;                 // Result validity flag
};
```

### ClassificationResult
*Wildlife classification result structure.*

```cpp
struct ClassificationResult {
    SpeciesType species;                    // Detected species
    float confidence;                       // Confidence score (0.0-1.0)
    ConfidenceLevel confidenceLevel;        // Confidence category
    uint32_t inferenceTime;                 // Processing time (ms)
    bool isValid;                          // Detection validity
    String speciesName;                    // Human-readable species name
    uint8_t animalCount;                   // Number of animals detected
    float boundingBoxes[10][4];            // Bounding boxes [x, y, w, h]
};
```

### CameraMetadata
```cpp
struct CameraMetadata {
    unsigned long timestamp;
    int batteryLevel;
    bool motionDetected;
    float temperature;
    float humidity;
    int lightLevel;
    int imageWidth;
    int imageHeight;
    float compressionRatio;
    
    // AI Enhancement Fields
    bool aiProcessed;
    String detectedSpecies;
    float speciesConfidence;
    bool dangerousSpeciesAlert;
};
```

### ModelInfo
*TensorFlow Lite model information structure.*

```cpp
struct ModelInfo {
    WildlifeModelType type;     // Model type identifier
    String name;                // Human-readable model name
    String filename;            // Model filename
    size_t size;               // Model size in bytes
    uint32_t version;          // Model version
    bool loaded;               // Load status
    float accuracy;            // Model accuracy
    uint32_t inputWidth;       // Input image width
    uint32_t inputHeight;      // Input image height
    uint32_t channels;         // Input channels (1 or 3)
};
```

### Configuration Options
*Configuration parameters and their descriptions.*

```cpp
// AI Configuration
#define SPECIES_CONFIDENCE_THRESHOLD 0.7f
#define TFLITE_ARENA_SIZE (512 * 1024)
#define MAX_SPECIES_COUNT 50
#define AI_INFERENCE_TIMEOUT_MS 5000

// Model Paths
#define MODELS_PRIMARY_PATH "/models/"
#define MODELS_SD_PATH "/sd/models/"
#define MODELS_ALT_PATH "/data/models/"
```

## Examples

### Basic AI Wildlife Detection
```cpp
#include "ai/wildlife_classifier.h"
#include "camera_handler.h"

WildlifeClassifier classifier;

void setup() {
    Serial.begin(115200);
    
    // Initialize camera
    CameraHandler::init();
    
    // Initialize AI classifier
    if (!classifier.initialize()) {
        Serial.println("ERROR: Failed to initialize wildlife classifier");
        return;
    }
    
    Serial.println("Wildlife AI system ready");
}

void loop() {
    if (motionDetected) {
        // Capture image
        camera_fb_t* frame = esp_camera_fb_get();
        
        // Classify wildlife
        WildlifeClassifier::ClassificationResult result = classifier.classifyFrame(frame);
        
        if (result.isValid) {
            Serial.printf("Detected: %s (%.1f%% confidence)\n", 
                         result.speciesName.c_str(), 
                         result.confidence * 100);
            
            // Check for dangerous species
            if (WildlifeClassifier::isDangerousSpecies(result.species)) {
                Serial.println("⚠️ DANGEROUS SPECIES ALERT!");
                // Trigger alert system
                sendDangerousSpeciesAlert(result);
            }
        }
        
        esp_camera_fb_return(frame);
    }
    
    delay(1000);
}
```

### Advanced TensorFlow Lite Usage
```cpp
#include "ai/tensorflow_lite_implementation.h"

void setup() {
    // Initialize TensorFlow Lite system
    if (!initializeTensorFlowLite()) {
        Serial.println("ERROR: TensorFlow Lite initialization failed");
        return;
    }
    
    // Load models
    if (!loadWildlifeModels("/models")) {
        Serial.println("WARNING: No models loaded, using fallback mode");
    }
    
    // Validate models
    bool speciesModelValid = g_tensorflowImplementation->validateModel(
        "/models/species_classifier_v1.0.0.tflite");
    
    if (speciesModelValid) {
        Serial.println("Species classifier model validated successfully");
    }
    
    // Enable performance monitoring
    g_tensorflowImplementation->enablePerformanceMonitoring(true);
    
    // Benchmark performance
    g_tensorflowImplementation->benchmarkModel(MODEL_SPECIES_CLASSIFIER, 10);
}

void loop() {
    // Multi-model inference example
    camera_fb_t* frame = esp_camera_fb_get();
    
    CameraFrame cameraFrame;
    cameraFrame.data = frame->buf;
    cameraFrame.width = frame->width;
    cameraFrame.height = frame->height;
    
    // Run different types of inference
    InferenceResult speciesResult = g_tensorflowImplementation->detectSpecies(cameraFrame);
    InferenceResult motionResult = g_tensorflowImplementation->detectMotion(cameraFrame);
    InferenceResult humanResult = g_tensorflowImplementation->detectHuman(cameraFrame);
    
    // Process results
    if (speciesResult.valid) {
        Serial.printf("Species: %s (%.2f)\n", speciesResult.className.c_str(), speciesResult.confidence);
    }
    
    if (motionResult.valid && motionResult.confidence > 0.8) {
        Serial.println("High-confidence motion detected");
    }
    
    if (humanResult.valid && humanResult.confidence > 0.9) {
        Serial.println("Human presence detected");
    }
    
    esp_camera_fb_return(frame);
    delay(5000);
}
```

### Memory-Optimized AI Usage
```cpp
#include "ai/tensorflow_lite_implementation.h"
#include "optimizations/memory_optimizer.h"

void setup() {
    // Initialize memory optimizer
    MemoryOptimizer::init();
    
    // Initialize TensorFlow Lite with memory optimization
    if (!initializeTensorFlowLite()) {
        Serial.println("ERROR: Failed to initialize AI system");
        return;
    }
    
    // Monitor memory usage
    size_t memoryUsage = g_tensorflowImplementation->getMemoryUsage();
    Serial.printf("AI system memory usage: %d bytes\n", memoryUsage);
    
    // Check fragmentation
    size_t fragmentation = MemoryOptimizer::getFragmentationLevel();
    Serial.printf("Memory fragmentation: %d%%\n", fragmentation);
}

void classifyWithMemoryMonitoring(camera_fb_t* frame) {
    // Check memory before inference
    size_t freeHeap = ESP.getFreeHeap();
    size_t freePsram = ESP.getFreePsram();
    
    Serial.printf("Before inference - Heap: %d, PSRAM: %d\n", freeHeap, freePsram);
    
    // Run classification
    WildlifeClassifier::ClassificationResult result = classifier.classifyFrame(frame);
    
    // Check memory after inference  
    size_t freeHeapAfter = ESP.getFreeHeap();
    size_t freePsramAfter = ESP.getFreePsram();
    
    Serial.printf("After inference - Heap: %d, PSRAM: %d\n", freeHeapAfter, freePsramAfter);
    
    // Defragment if needed
    if (MemoryOptimizer::getFragmentationLevel() > 40) {
        MemoryOptimizer::defragmentHeap();
        Serial.println("Memory defragmentation performed");
    }
}
```

## Error Codes

### AI System Error Codes

| Code | Name | Description |
|------|------|-------------|
| `AI_SUCCESS` | Success | Operation completed successfully |
| `AI_ERROR_INIT_FAILED` | Initialization Failed | TensorFlow Lite initialization failed |
| `AI_ERROR_MODEL_NOT_FOUND` | Model Not Found | Model file not found in specified paths |
| `AI_ERROR_MODEL_INVALID` | Invalid Model | Model file is corrupted or invalid |
| `AI_ERROR_INSUFFICIENT_MEMORY` | Insufficient Memory | Not enough memory for model loading |
| `AI_ERROR_INFERENCE_FAILED` | Inference Failed | Model inference execution failed |
| `AI_ERROR_TIMEOUT` | Timeout | Inference exceeded maximum time limit |
| `AI_ERROR_INVALID_INPUT` | Invalid Input | Input data format or size is invalid |

### Memory Management Error Codes

| Code | Name | Description |
|------|------|-------------|
| `MEM_SUCCESS` | Success | Memory operation successful |
| `MEM_ERROR_ALLOCATION_FAILED` | Allocation Failed | Memory allocation failed |
| `MEM_ERROR_HIGH_FRAGMENTATION` | High Fragmentation | Memory fragmentation above threshold |
| `MEM_ERROR_PSRAM_UNAVAILABLE` | PSRAM Unavailable | PSRAM not found or accessible |

## Performance Benchmarks

### Target Performance Metrics

| Operation | Target Time | Acceptable Time | Memory Usage |
|-----------|-------------|-----------------|--------------|
| Species Classification | <1000ms | <2000ms | <512KB |
| Behavior Analysis | <800ms | <1500ms | <384KB |
| Motion Detection | <200ms | <500ms | <128KB |
| Human Detection | <600ms | <1200ms | <256KB |
| Model Loading | <5000ms | <10000ms | <4MB |

### Accuracy Targets

| Model Type | Target Accuracy | Minimum Accuracy |
|------------|----------------|------------------|
| Species Classifier | >95% | >90% |
| Behavior Analyzer | >90% | >85% |
| Motion Detector | >98% | >95% |
| Human Detector | >97% | >93% |

---

*For detailed ML workflow documentation, see [ML_WORKFLOW.md](../docs/ML_WORKFLOW.md)*

*For deployment scenarios, see [docs/deployment/scenarios/](../docs/deployment/scenarios/)*

### CameraHandler
*Camera management and image capture functionality.*

#### Methods
- `init()` - Initialize camera system
- `captureImage()` - Capture a single image
- `getStatus()` - Get camera status

### MotionFilter
*Motion detection and filtering system.*

#### Methods
- `init()` - Initialize motion detection
- `checkMotion()` - Check for motion events
- `setThreshold()` - Configure motion sensitivity

### SolarManager
*Solar power management and battery monitoring.*

#### Methods
- `init()` - Initialize power management
- `getBatteryLevel()` - Get current battery level
- `getSolarVoltage()` - Get solar panel voltage

### LoraMesh
*LoRa mesh networking functionality.*

#### Methods
- `init()` - Initialize LoRa network
- `sendMessage()` - Send data over LoRa
- `receiveMessage()` - Receive LoRa messages

## Data Structures

### CameraMetadata
```cpp
struct CameraMetadata {
    unsigned long timestamp;
    int batteryLevel;
    bool motionDetected;
    float temperature;
    float humidity;
    int lightLevel;
    int imageWidth;
    int imageHeight;
    float compressionRatio;
};
```

### Configuration Options
*Configuration parameters and their descriptions.*

## Examples

### Basic Usage
```cpp
#include "camera_handler.h"

void setup() {
    CameraHandler::init();
}

void loop() {
    if (motionDetected) {
        CameraHandler::captureImage();
    }
}
```

## Error Codes

*Error codes and their meanings will be documented here.*

---

*This API reference is currently a placeholder and will be expanded with complete documentation.*