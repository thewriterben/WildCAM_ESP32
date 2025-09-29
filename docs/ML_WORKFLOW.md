# AI Wildlife Detection - Machine Learning Workflow Guide

## Overview

This document provides a comprehensive guide for deploying, training, and managing machine learning models for the ESP32 Wildlife Camera system.

## Table of Contents

1. [Model Architecture](#model-architecture)
2. [Model Deployment](#model-deployment)
3. [Training Pipeline](#training-pipeline)
4. [Performance Optimization](#performance-optimization)
5. [Species Classification](#species-classification)
6. [Dangerous Species Detection](#dangerous-species-detection)
7. [Memory Management](#memory-management)
8. [Troubleshooting](#troubleshooting)

## Model Architecture

### Supported Model Types

The system supports four main model types:

1. **Species Classifier** (`MODEL_SPECIES_CLASSIFIER`)
   - Input: 224x224x3 RGB images
   - Output: 50 species classes + confidence scores
   - Arena Size: 512KB
   - Purpose: Identify wildlife species

2. **Behavior Analyzer** (`MODEL_BEHAVIOR_ANALYZER`)
   - Input: 3x160x160x3 temporal sequence
   - Output: 12 behavior classes
   - Arena Size: 384KB
   - Purpose: Analyze animal behavior patterns

3. **Motion Detector** (`MODEL_MOTION_DETECTOR`)
   - Input: 96x96x1 grayscale
   - Output: 5 motion classes
   - Arena Size: 128KB
   - Purpose: Efficient motion detection

4. **Human Detector** (`MODEL_HUMAN_DETECTOR`)
   - Input: 224x224x3 RGB images
   - Output: Human detection + confidence
   - Arena Size: 256KB
   - Purpose: Human presence detection

### Model Requirements

- **Format**: TensorFlow Lite (.tflite)
- **Quantization**: INT8 preferred for ESP32 efficiency
- **Memory**: PSRAM required for models >100KB
- **Optimization**: Pruned and compressed models recommended

## Model Deployment

### File Structure

```
/models/
├── species_classifier_v1.0.0.tflite
├── behavior_analyzer_v1.0.0.tflite
├── motion_detector_v1.0.0.tflite
├── human_detector_v1.0.0.tflite
└── model_manifest.json
```

### Deployment Locations

Models are searched in the following order:
1. `/models/` (primary location)
2. `/sd/models/` (SD card storage)
3. `/data/models/` (alternative location)

### Initialization Code

```cpp
#include "ai/tensorflow_lite_implementation.h"

// Initialize TensorFlow Lite system
if (!initializeTensorFlowLite()) {
    Serial.println("ERROR: Failed to initialize TensorFlow Lite");
    return false;
}

// Load models from directory
if (!loadWildlifeModels("/models")) {
    Serial.println("WARNING: No models loaded, using fallback mode");
}
```

### Model Validation

The system automatically validates models during loading:

```cpp
// Validate a model file before loading
if (g_tensorflowImplementation->validateModel("/models/species_classifier_v1.0.0.tflite")) {
    Serial.println("Model validation passed");
} else {
    Serial.println("Model validation failed");
}
```

## Training Pipeline

### Data Requirements

#### Species Classification
- **Training Set**: 50,000+ labeled wildlife images
- **Validation Set**: 10,000+ images (separate from training)
- **Test Set**: 5,000+ images for final evaluation
- **Classes**: 50 wildlife species + background/unknown

#### Behavior Analysis
- **Training Set**: 30,000+ temporal sequences (3 frames each)
- **Behaviors**: Feeding, drinking, resting, moving, alert, grooming, mating, aggressive, territorial, social, hunting, playing
- **Sequence Length**: 3 frames at 1-second intervals

### Training Framework

The models are trained using TensorFlow and converted to TensorFlow Lite:

```python
import tensorflow as tf

# Load pre-trained model
base_model = tf.keras.applications.MobileNetV2(
    input_shape=(224, 224, 3),
    include_top=False,
    weights='imagenet'
)

# Add classification head
model = tf.keras.Sequential([
    base_model,
    tf.keras.layers.GlobalAveragePooling2D(),
    tf.keras.layers.Dense(128, activation='relu'),
    tf.keras.layers.Dropout(0.2),
    tf.keras.layers.Dense(50, activation='softmax')  # 50 species
])

# Compile and train
model.compile(
    optimizer='adam',
    loss='categorical_crossentropy',
    metrics=['accuracy']
)

# Convert to TensorFlow Lite
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_types = [tf.int8]
tflite_model = converter.convert()
```

### Edge Impulse Integration

The system supports Edge Impulse for rapid model development:

1. Create project at [edgeimpulse.com](https://edgeimpulse.com)
2. Upload wildlife images with labels
3. Train model with appropriate settings
4. Export as TensorFlow Lite
5. Deploy to ESP32

## Performance Optimization

### Memory Management

The system uses advanced memory optimization:

```cpp
// Memory-optimized allocation
model.tensorArena = (uint8_t*)MemoryOptimizer::allocateAligned(model.arenaSize, 32);

// Monitor fragmentation
size_t fragmentationLevel = MemoryOptimizer::getFragmentationLevel();
if (fragmentationLevel > 30) {
    MemoryOptimizer::defragmentHeap();
}
```

### Inference Optimization

- **Model Loading**: Models loaded on-demand to save memory
- **Preprocessing**: Optimized image resizing and normalization
- **Quantization**: INT8 models for 4x speed improvement
- **PSRAM**: Large models stored in external PSRAM

### Performance Monitoring

```cpp
// Enable performance monitoring
g_tensorflowImplementation->enablePerformanceMonitoring(true);

// Get performance metrics
uint32_t avgTime = g_tensorflowImplementation->getAverageInferenceTime(MODEL_SPECIES_CLASSIFIER);
Serial.printf("Average inference time: %d ms\n", avgTime);
```

### Benchmarking

```cpp
// Benchmark model performance
bool success = g_tensorflowImplementation->benchmarkModel(MODEL_SPECIES_CLASSIFIER, 10);
if (success) {
    Serial.println("Benchmark completed successfully");
}
```

## Species Classification

### Supported Species

The system recognizes 50+ wildlife species:

**Mammals:**
- White-tailed Deer, Black Bear, Red Fox, Gray Wolf
- Mountain Lion, Elk, Moose, Raccoon, Coyote, Bobcat
- And 25+ additional species

**Birds:**
- Wild Turkey, Bald Eagle, Red-tailed Hawk
- Great Blue Heron, Canada Goose, Mallard Duck
- And 15+ additional species

**Other:**
- Snake, Turtle, Lizard, Frog, Salamander
- Human (for monitoring purposes)

### Usage Example

```cpp
#include "ai/wildlife_classifier.h"

WildlifeClassifier classifier;
classifier.initialize();

// Classify camera frame
camera_fb_t* frame = esp_camera_fb_get();
WildlifeClassifier::ClassificationResult result = classifier.classifyFrame(frame);

if (result.isValid) {
    Serial.printf("Detected: %s (%.1f%% confidence)\n", 
                 result.speciesName.c_str(), 
                 result.confidence * 100);
    
    // Check if dangerous species
    if (WildlifeClassifier::isDangerousSpecies(result.species)) {
        Serial.println("WARNING: Dangerous species detected!");
        // Trigger alert system
    }
}

esp_camera_fb_return(frame);
```

## Dangerous Species Detection

The system automatically identifies potentially dangerous wildlife:

### Dangerous Species List

- **Black Bear** - Large predator, requires caution
- **Gray Wolf** - Pack predator, territorial behavior
- **Mountain Lion** - Solitary predator, stealth hunter  
- **Coyote** - Opportunistic predator, adaptable
- **Bobcat** - Small predator, rarely dangerous to humans

### Alert System Integration

```cpp
// Automatic dangerous species detection
if (result.isValid && WildlifeClassifier::isDangerousSpecies(result.species)) {
    // Generate conservation alert
    ConservationAlert alert;
    alert.severity = ALERT_HIGH;
    alert.species_involved = result.speciesName;
    alert.confidence_level = result.confidence;
    alert.recommended_action = "Maintain safe distance, monitor behavior";
    
    // Send alert to stakeholders
    conservation_system.sendAlert(alert);
}
```

## Memory Management

### PSRAM Utilization

The system efficiently uses ESP32-S3 PSRAM:

```cpp
// Check PSRAM availability
if (psramFound()) {
    Serial.printf("PSRAM found: %d bytes\n", ESP.getPsramSize());
    
    // Large models use PSRAM
    if (model.arenaSize > 100 * 1024) {
        model.tensorArena = (uint8_t*)ps_malloc(model.arenaSize);
    }
}
```

### Memory Optimization Best Practices

1. **Load models on-demand** - Only load required models
2. **Use model unloading** - Free memory when models not needed
3. **Monitor fragmentation** - Regular defragmentation
4. **Optimize preprocessing** - In-place image processing when possible

## Troubleshooting

### Common Issues

#### Model Loading Failures

**Symptom**: "ERROR: Failed to load classification model"

**Solutions**:
1. Check model file exists in `/models/` directory
2. Verify model file is valid TensorFlow Lite format
3. Ensure sufficient memory (PSRAM) available
4. Check SD card mounting if using SD storage

#### Inference Failures

**Symptom**: "ERROR: Model inference failed"

**Solutions**:
1. Verify input image dimensions match model requirements
2. Check tensor arena size is sufficient
3. Monitor memory fragmentation levels
4. Validate model quantization settings

#### Performance Issues

**Symptom**: Slow inference times (>2 seconds)

**Solutions**:
1. Use INT8 quantized models
2. Optimize input image preprocessing
3. Enable PSRAM for large models
4. Consider model pruning/compression

### Debug Information

Enable detailed debugging:

```cpp
// Enable TensorFlow Lite debugging
#define TFLITE_MICRO_ENABLED 1
#define DEBUG_TENSORFLOW_LITE 1

// Monitor performance
g_tensorflowImplementation->enablePerformanceMonitoring(true);
```

### Performance Targets

| Metric | Target | Acceptable |
|--------|--------|------------|
| Species Classification | <1000ms | <2000ms |
| Behavior Analysis | <800ms | <1500ms |
| Motion Detection | <200ms | <500ms |
| Memory Usage | <2MB | <4MB |
| Accuracy | >95% | >90% |

## Future Enhancements

### Planned Features

1. **Federated Learning** - Collaborative model improvement
2. **Multi-model Ensembles** - Improved accuracy through model combination
3. **Real-time Training** - On-device model adaptation
4. **Advanced Preprocessing** - HDR and low-light optimization
5. **Model Compression** - Further size reduction techniques

### Model Updates

The system supports over-the-air model updates:

```cpp
// Check for model updates
if (modelUpdateAvailable()) {
    downloadAndInstallModelUpdate();
    reloadModels();
}
```

## Conclusion

This ML workflow provides a robust foundation for AI-powered wildlife monitoring. The system balances accuracy, performance, and memory efficiency while providing comprehensive species detection and dangerous wildlife alerting capabilities.

For additional support or model training assistance, refer to the project documentation or contact the development team.