# Machine Learning Integration Guide

## Overview

This guide provides comprehensive instructions for integrating machine learning models into the ESP32 Wildlife Camera system for on-device species identification and automated wildlife detection.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Training Pipeline](#training-pipeline)
3. [Model Deployment](#model-deployment)
4. [ESP32 Integration](#esp32-integration)
5. [Testing and Validation](#testing-and-validation)
6. [Troubleshooting](#troubleshooting)

## Architecture Overview

### System Components

```
┌─────────────────────────────────────────────────────────────┐
│                    ML Integration Pipeline                   │
├─────────────────────────────────────────────────────────────┤
│  1. Data Collection                                          │
│     └─ Dataset Manager (backend/ml/dataset_manager.py)      │
├─────────────────────────────────────────────────────────────┤
│  2. Model Training                                           │
│     └─ Model Trainer (backend/ml/model_trainer.py)          │
├─────────────────────────────────────────────────────────────┤
│  3. Model Evaluation                                         │
│     └─ Model Evaluator (backend/ml/model_evaluator.py)      │
├─────────────────────────────────────────────────────────────┤
│  4. Deployment Pipeline                                      │
│     └─ Deployment Pipeline (backend/ml/deployment_pipeline.py)│
├─────────────────────────────────────────────────────────────┤
│  5. ESP32 Integration                                        │
│     ├─ Inference Engine (firmware/src/ai/tinyml/)           │
│     ├─ Wildlife Classifier (firmware/src/ai/)               │
│     └─ Camera Integration (ESP32WildlifeCAM-main/src/)      │
└─────────────────────────────────────────────────────────────┘
```

## Training Pipeline

### Prerequisites

1. **Python Environment**:
   ```bash
   cd backend
   pip install -r requirements.txt
   ```

2. **Wildlife Dataset**:
   - Collect images of wildlife species
   - Organize in class-specific folders
   - Minimum 500 images per species recommended

### Step 1: Data Collection

The `dataset_manager.py` handles data collection and preprocessing:

```python
from backend.ml.dataset_manager import DatasetManager

# Initialize dataset manager
manager = DatasetManager(
    base_dir="/data/wildlife_dataset",
    species_config="config/species_config.json"
)

# Collect data from iNaturalist (optional)
manager.collect_from_inaturalist(
    species_list=["white_tailed_deer", "black_bear", "red_fox"],
    samples_per_species=1000
)

# Preprocess dataset
manager.preprocess_dataset(
    image_size=(224, 224),
    augmentation=True
)

# Export for TensorFlow
manager.export_tensorflow_dataset("/data/tf_dataset")
```

### Step 2: Model Training

The `model_trainer.py` implements transfer learning with MobileNet:

```python
from backend.ml.model_trainer import WildlifeModelTrainer

# Configure training
config = {
    "image_size": [224, 224],
    "batch_size": 32,
    "epochs": 30,
    "learning_rate": 0.001,
    "base_model": "MobileNetV3Large",
    "quantization": True
}

# Initialize trainer
trainer = WildlifeModelTrainer(
    dataset_dir="/data/tf_dataset",
    output_dir="/data/models",
    config=config
)

# Train model
training_stats = trainer.train_model()

# Convert to TensorFlow Lite with INT8 quantization
tflite_path = trainer.convert_to_tflite(quantize=True)

# Benchmark performance
benchmark = trainer.benchmark_tflite_model(tflite_path)
```

### Step 3: Model Evaluation

The `model_evaluator.py` provides comprehensive evaluation:

```python
from backend.ml.model_evaluator import WildlifeModelEvaluator

# Initialize evaluator
evaluator = WildlifeModelEvaluator(
    model_path="/data/models/wildlife_classifier_quantized.tflite",
    dataset_dir="/data/tf_dataset",
    output_dir="/data/evaluation"
)

# Generate comprehensive report
report = evaluator.generate_comprehensive_report()

# Check deployment readiness
if report['deployment_ready']:
    print("✅ Model ready for deployment!")
else:
    print("❌ Model needs improvement")
```

### Step 4: Complete Pipeline

Use `deployment_pipeline.py` for end-to-end automation:

```python
from backend.ml.deployment_pipeline import WildlifeDeploymentPipeline

# Create configuration
config = {
    "data_collection": {
        "enabled": True,
        "species_list": ["deer", "bear", "fox", "wolf"],
        "samples_per_species": 500
    },
    "model_training": {
        "enabled": True,
        "epochs": 30,
        "quantization": True
    },
    "model_evaluation": {
        "enabled": True
    },
    "deployment": {
        "enabled": True
    }
}

# Run complete pipeline
pipeline = WildlifeDeploymentPipeline(
    config_path="config.json",
    output_dir="/data/deployment"
)

results = pipeline.run_complete_pipeline()
```

## Model Deployment

### Deployment Structure

```
firmware/models/
├── wildlife_classifier_v2.tflite    # Main classification model
├── model_metadata.json              # Model specifications
├── class_labels.json                # Species labels
└── README.md                        # Deployment instructions
```

### Model Specifications

Required specifications for ESP32 deployment:

| Specification | Value |
|--------------|-------|
| **Format** | TensorFlow Lite (.tflite) |
| **Quantization** | INT8 (recommended) |
| **Input Shape** | [1, 224, 224, 3] |
| **Input Type** | uint8 (0-255) |
| **Output Shape** | [1, num_classes] |
| **Output Type** | uint8 or float32 |
| **Max Size** | < 15 MB |
| **Target Platform** | ESP32-S3 with 8MB PSRAM |

### Deployment Process

1. **Copy Model to Firmware**:
   ```bash
   cp /data/models/wildlife_classifier_quantized.tflite \
      firmware/models/wildlife_classifier_v2.tflite
   ```

2. **Update Metadata**:
   ```bash
   cp /data/models/model_metadata.json \
      firmware/models/model_metadata.json
   ```

3. **Update Class Labels**:
   ```bash
   cp /data/models/class_labels.json \
      firmware/models/class_labels.json
   ```

## ESP32 Integration

### Inference Engine

The `InferenceEngine` class handles TFLite model execution:

```cpp
#include "ai/tinyml/inference_engine.h"

// Initialize inference engine
InferenceEngine inferenceEngine;
inferenceEngine.init();

// Load model
const char* modelPath = "/models/wildlife_classifier_v2.tflite";
if (!inferenceEngine.loadModel(modelPath, ModelType::MODEL_SPECIES_CLASSIFIER)) {
    Serial.println("Failed to load model");
    return;
}

// Run inference
CameraFrame frame = captureFrame();
AIResult result = inferenceEngine.runInference(frame, ModelType::MODEL_SPECIES_CLASSIFIER);

if (result.confidence > 0.7) {
    Serial.printf("Detected: %s (%.1f%% confidence)\n",
                 result.label.c_str(),
                 result.confidence * 100);
}
```

### Wildlife Classifier Integration

The `WildlifeClassifier` provides high-level classification:

```cpp
#include "ai/wildlife_classifier.h"

// Initialize classifier
WildlifeClassifier classifier;
WildlifeClassifierConfig config;
config.confidenceThreshold = 0.7;
config.enableEnvironmentalAdaptation = true;

classifier.init(config);

// Classify image
camera_fb_t* fb = esp_camera_fb_get();
std::vector<WildlifeDetection> detections = classifier.classifyImage(
    {fb->buf, fb->width, fb->height, fb->format}
);

// Process detections
for (const auto& detection : detections) {
    Serial.printf("Species: %s, Confidence: %.2f\n",
                 getSpeciesName(detection.species).c_str(),
                 detection.confidence);
    
    // Check for dangerous species
    if (isDangerousSpecies(detection.species)) {
        triggerAlert(detection);
    }
}

esp_camera_fb_return(fb);
```

### Camera Integration

Integration with camera capture workflow:

```cpp
// In main.cpp handleMotionEvent()
void handleMotionEvent() {
    // Capture image
    CameraManager::CaptureResult result = cameraManager.captureImage();
    
    if (result.success && wildlifeClassifier.isEnabled()) {
        // Run AI classification
        WildlifeClassifier::ClassificationResult aiResult = 
            wildlifeClassifier.classifyFrame(result.frameBuffer);
        
        if (aiResult.isValid) {
            // Log detection
            logWildlifeDetection(aiResult);
            
            // Save with metadata
            saveImageWithMetadata(result.filename, aiResult);
            
            // Update statistics
            updateSpeciesStatistics(aiResult);
        }
        
        cameraManager.returnFrameBuffer(result.frameBuffer);
    }
}
```

## Testing and Validation

### Test Model Generation

For testing without a trained model, use the test model generator:

```bash
cd backend/ml
python generate_test_model.py --output-dir ../../firmware/models/test
```

This creates:
- Minimal test model (< 1 MB)
- Model metadata
- Class labels
- Test documentation

### Integration Testing

Run the ML integration test suite:

```bash
cd scripts
python ml_integration_test.py
```

Tests include:
- ✓ Model generation
- ✓ File structure validation
- ✓ Metadata validation
- ✓ Model size constraints
- ✓ ESP32 compatibility

### Performance Benchmarks

Expected performance on ESP32-S3:

| Metric | Target | Acceptable |
|--------|--------|------------|
| **Inference Time** | < 800 ms | < 1500 ms |
| **Memory Usage** | < 2 MB | < 4 MB |
| **Accuracy** | > 95% | > 90% |
| **Power Consumption** | < 180 mA | < 250 mA |

### Validation Checklist

- [ ] Model loads successfully on ESP32
- [ ] Inference completes within time constraints
- [ ] Memory usage within PSRAM limits
- [ ] Classification accuracy meets requirements
- [ ] Integration with camera workflow works
- [ ] Power consumption acceptable
- [ ] Error handling robust

## Troubleshooting

### Common Issues

#### 1. Model Loading Fails

**Symptom**: "Failed to load model" error

**Solutions**:
- Verify model file exists at specified path
- Check model format is TensorFlow Lite (.tflite)
- Ensure sufficient PSRAM available (8MB minimum)
- Validate model file is not corrupted

#### 2. Inference Fails

**Symptom**: "Inference failed" error

**Solutions**:
- Check input image dimensions match model requirements (224x224x3)
- Verify input data type (uint8 for quantized models)
- Ensure tensor arena size sufficient
- Monitor memory fragmentation

#### 3. Slow Inference

**Symptom**: Inference takes > 2 seconds

**Solutions**:
- Use INT8 quantized model
- Enable PSRAM for model storage
- Increase CPU frequency (240 MHz)
- Optimize preprocessing pipeline
- Consider model pruning/compression

#### 4. Low Accuracy

**Symptom**: Poor species detection accuracy

**Solutions**:
- Retrain with more diverse dataset
- Increase training epochs
- Fine-tune confidence thresholds
- Enable environmental adaptation
- Verify image preprocessing correct

#### 5. Memory Issues

**Symptom**: Out of memory errors

**Solutions**:
- Use smaller model architecture
- Enable model quantization
- Implement memory pooling
- Unload unused models
- Monitor heap fragmentation

### Debug Mode

Enable debug logging for detailed diagnostics:

```cpp
// Enable TensorFlow Lite debug output
#define TFLITE_MICRO_ENABLED 1
#define DEBUG_TENSORFLOW_LITE 1

// Enable performance monitoring
inferenceEngine.enablePerformanceMonitoring(true);

// Get detailed metrics
AIMetrics metrics = inferenceEngine.getPerformanceMetrics();
Serial.printf("Avg inference time: %d ms\n", metrics.avgInferenceTime);
Serial.printf("Memory usage: %d KB\n", metrics.memoryUsage);
Serial.printf("Success rate: %.1f%%\n", metrics.successRate * 100);
```

## Best Practices

### Model Training

1. **Dataset Quality**:
   - Use high-quality, diverse images
   - Balance dataset across species
   - Include various lighting conditions
   - Represent different seasons and weather

2. **Transfer Learning**:
   - Start with pre-trained MobileNet
   - Fine-tune top layers
   - Use appropriate learning rate schedule
   - Monitor validation metrics

3. **Optimization**:
   - Always use INT8 quantization for ESP32
   - Validate accuracy after quantization
   - Profile inference performance
   - Optimize model size vs. accuracy tradeoff

### ESP32 Deployment

1. **Memory Management**:
   - Use PSRAM for model storage
   - Implement proper memory cleanup
   - Monitor heap fragmentation
   - Use memory pools for frequent allocations

2. **Power Optimization**:
   - Adjust inference frequency based on battery
   - Use low-power modes when possible
   - Implement adaptive inference intervals
   - Profile power consumption

3. **Error Handling**:
   - Implement robust error recovery
   - Log failures for debugging
   - Graceful degradation on errors
   - Validate all inputs

## Additional Resources

- [TensorFlow Lite Documentation](https://www.tensorflow.org/lite)
- [ESP32-S3 Technical Reference](https://www.espressif.com/en/products/socs/esp32-s3)
- [Edge Impulse Platform](https://www.edgeimpulse.com/)
- [ML Workflow Guide](ML_WORKFLOW.md)
- [AI Architecture Documentation](ai/ai_architecture.md)

## Support

For issues and questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Documentation: https://github.com/thewriterben/WildCAM_ESP32/docs
- Community: Discussion forum (link TBD)

## Version History

- **v2.0.0** (2024): Complete ML integration implementation
- **v1.5.0** (2024): Initial ML integration framework
- **v1.0.0** (2023): Basic wildlife camera functionality
