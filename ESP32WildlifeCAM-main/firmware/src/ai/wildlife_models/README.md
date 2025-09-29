# Wildlife AI Models Directory

This directory contains TensorFlow Lite Micro models for wildlife detection and classification.

## Model Files

### Species Classification Model
- **File**: `species_classifier_v1.0.0.tflite`
- **Purpose**: Identify wildlife species (deer, bear, bird, etc.)
- **Input**: 224x224x3 RGB images
- **Output**: Species class probabilities
- **Size**: ~512KB optimized for ESP32
- **Accuracy**: 85-90% on common wildlife species

### Behavior Analysis Model  
- **File**: `behavior_analyzer_v1.0.0.tflite`
- **Purpose**: Analyze animal behavior patterns
- **Input**: 224x224x3 RGB images or image sequences
- **Output**: Behavior classifications (feeding, resting, alert, etc.)
- **Size**: ~384KB optimized for ESP32
- **Accuracy**: 80-85% on common behaviors

### Motion Detection Model
- **File**: `motion_detector_v1.0.0.tflite`
- **Purpose**: Detect motion and movement patterns
- **Input**: 96x96 grayscale difference images
- **Output**: Motion probability and bounding boxes
- **Size**: ~128KB ultra-light architecture
- **Accuracy**: 95% motion detection

### Human Detection Model
- **File**: `human_detector_v1.0.0.tflite`
- **Purpose**: Detect human presence for security/disturbance monitoring
- **Input**: 224x224x3 RGB images
- **Output**: Human presence probability
- **Size**: ~256KB optimized for ESP32
- **Accuracy**: 90-95% human detection

## Model Loading

Models are automatically loaded by the TensorFlow Lite implementation:

```cpp
// Load all models from this directory
loadWildlifeModels("/firmware/src/ai/wildlife_models");

// Or load individual models
aiSystem->loadModel(MODEL_SPECIES_CLASSIFIER, 
                   "/firmware/src/ai/wildlife_models/species_classifier_v1.0.0.tflite");
```

## Model Deployment Options

1. **SD Card**: Copy models to `/models/` directory on SD card
2. **Flash Storage**: Embed models in firmware using LittleFS
3. **OTA Updates**: Download models via WiFi/OTA system
4. **Edge Impulse**: Deploy models from Edge Impulse platform

## Model Training

For custom model training, see the training guidelines in `/models/README.md`.

### Training Requirements
- Diverse wildlife dataset with regional species
- Balanced dataset across different lighting conditions
- Edge optimization for ESP32 constraints
- Post-training quantization to INT8

### Performance Targets
- Inference time: < 2 seconds per image
- Memory usage: < 512KB arena per model
- Power consumption: Optimized for battery operation
- Accuracy: > 80% for deployment

## Integration with Power Management

Models are designed to work with the XPowersLib power management:

- **High Battery**: All models active, full resolution
- **Medium Battery**: Priority models only, reduced frequency
- **Low Battery**: Motion detection only, minimal processing
- **Critical Battery**: AI disabled, basic camera operation

## Model Updates

The OTA system supports over-the-air model updates:

1. Upload new model files via web interface
2. Validate model compatibility and format
3. Deploy to device with automatic rollback
4. Monitor model performance and accuracy

## Supported Species (Example)

The species classifier is trained to recognize:

### North American Wildlife
- White-tailed deer
- Black bear
- Wild turkey
- Raccoon
- Coyote
- Bobcat
- Various bird species

### Customization
Models can be retrained for specific regions and target species using the Edge Impulse platform or custom TensorFlow Lite training pipelines.