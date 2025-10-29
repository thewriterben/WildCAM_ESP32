# Wildlife Classifier v2.0 - Production Model

## Overview
Comprehensive TensorFlow Lite model for wildlife species classification with 36 species coverage.

**⚠️ DEMO MODEL**: This model is trained on synthetic data for demonstration and integration testing.
For production deployment, train on real wildlife datasets using the comprehensive training pipeline.

## Model Specifications
- **Model Name**: wildlife_classifier_v2
- **Version**: 2.0.0
- **Architecture**: Deep CNN optimized for ESP32-S3
- **Input**: 224x224x3 RGB images (uint8, 0-255)
- **Output**: 36 species classes (uint8 quantized)
- **Quantization**: Full INT8
- **Target Platform**: ESP32-S3 with 8MB PSRAM

## Supported Species (36 total)

### Deer and Cervids
- White-tailed Deer
- Mule Deer
- Elk
- Moose
- Caribou

### Large Predators
- Black Bear
- Grizzly Bear
- Mountain Lion
- Gray Wolf
- Coyote

### Medium Predators
- Bobcat
- Lynx
- Red Fox
- Gray Fox

### Medium Mammals
- Raccoon
- Opossum
- Skunk
- Porcupine
- Beaver

### Small Mammals
- Rabbit
- Squirrel
- Chipmunk
- Groundhog

### Birds
- Wild Turkey
- Grouse
- Pheasant
- Crow
- Raven
- Eagle
- Hawk
- Owl

### Other
- Domestic Dog
- Domestic Cat
- Livestock
- Human
- Unknown

## Performance Targets

| Metric | Target | Notes |
|--------|--------|-------|
| Inference Time | < 850ms | ESP32-S3 @ 240MHz |
| Memory Usage | 1.5 MB | Including model and buffers |
| Power Consumption | < 165mA | During inference |
| Accuracy | > 95% | On real wildlife data |
| Top-5 Accuracy | > 99% | On real wildlife data |

## Usage

### Basic Integration
```cpp
#include "firmware/src/ai/wildlife_classifier.h"

WildlifeClassifier classifier;

void setup() {
    WildlifeClassifier::ClassifierConfig config;
    config.confidenceThreshold = 0.7;
    config.enableEnvironmentalAdaptation = true;
    
    if (classifier.initialize(config)) {
        Serial.println("Wildlife classifier initialized");
    }
}

void loop() {
    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    
    // Classify
    auto result = classifier.classifyFrame(fb);
    
    if (result.isValid) {
        Serial.printf("Species: %s\n", result.speciesName.c_str());
        Serial.printf("Confidence: %.2f%%\n", result.confidence * 100);
        
        if (result.isDangerous) {
            Serial.println("⚠️  Dangerous species detected!");
        }
    }
    
    esp_camera_fb_return(fb);
    delay(1000);
}
```

### Advanced Usage with Behavior Analysis
```cpp
#include "firmware/src/ai/vision/species_classifier.h"

SpeciesClassifier classifier;

void detectAndAnalyze() {
    // Get camera frame
    CameraFrame frame = camera.capture();
    
    // Classify species
    SpeciesResult result = classifier.classifyWithBehavior(frame);
    
    // Process results
    Serial.printf("Species: %s\n", result.specificName.c_str());
    Serial.printf("Confidence: %.2f\n", result.confidence);
    Serial.printf("Behavior: %s\n", result.behaviorState.c_str());
    Serial.printf("Size estimate: %s\n", result.sizeCategory.c_str());
}
```

## Training for Production

To replace this demo model with a production model:

### 1. Dataset Collection
```bash
# Collect diverse wildlife images
# - 500-1000 images per species
# - Multiple lighting conditions
# - Various angles and distances
# - Different seasons/weather
```

### 2. Model Training
```bash
cd /home/runner/work/WildCAM_ESP32/WildCAM_ESP32
python3 backend/ml/model_trainer.py \
    --dataset /path/to/wildlife_dataset \
    --output firmware/models/production \
    --model-name wildlife_classifier_v2 \
    --epochs 100 \
    --batch-size 32 \
    --quantize
```

### 3. Model Evaluation
```bash
python3 backend/ml/model_evaluator.py \
    --model firmware/models/production/wildlife_classifier_v2.tflite \
    --test-data /path/to/test_set \
    --threshold 0.95
```

### 4. Deployment
```bash
# Copy to production directory
cp firmware/models/production/wildlife_classifier_v2_quantized.tflite \
   firmware/models/wildlife_classifier_v2.tflite

# Flash to ESP32
pio run -t upload -e esp32s3cam_ai
```

## Files in This Directory
- `wildlife_classifier_v2_quantized.tflite` - INT8 quantized TFLite model
- `wildlife_classifier_v2.h5` - Keras model (for reference)
- `wildlife_classifier_v2_metadata.json` - Model specifications
- `wildlife_classifier_v2_labels.json` - Species labels
- `README.md` - This documentation

## Integration with Existing Systems

This model is designed to integrate seamlessly with:
- **Detection Pipeline**: Automatic classification after motion detection
- **Mesh Networking**: Share detection results across camera network
- **Alert System**: Trigger alerts for dangerous species
- **Data Collection**: Store classifications with metadata
- **Cloud Sync**: Upload results to backend server

## Performance Optimization Tips

1. **Enable PSRAM**: Essential for 8MB models
2. **CPU Frequency**: Run at 240MHz for best performance
3. **Pre-processing**: Resize images before inference
4. **Confidence Threshold**: Tune based on field testing
5. **Power Management**: Use motion triggers to reduce power

## Model Limitations

⚠️ **Demo Model Limitations**:
- Trained on synthetic data only
- Low accuracy on real wildlife images
- Not suitable for production use
- Requires real dataset training for deployment

## Next Steps

1. Collect real wildlife dataset
2. Train production model (see ml/TRAINING_DEPLOYMENT_GUIDE.md)
3. Validate accuracy > 95%
4. Deploy to field units
5. Monitor and iterate

## Support

- **Training Guide**: [ml/TRAINING_DEPLOYMENT_GUIDE.md](../../ml/TRAINING_DEPLOYMENT_GUIDE.md)
- **Quick Start**: [ml/QUICKSTART.md](../../ml/QUICKSTART.md)
- **ML Integration**: [ML_INTEGRATION_README.md](../../ML_INTEGRATION_README.md)
- **Issues**: GitHub Issues with `machine-learning` label

Generated: 2025-10-29 02:42:47
Version: 2.0.0 (Demo)
