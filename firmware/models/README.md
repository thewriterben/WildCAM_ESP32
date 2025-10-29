# Wildlife Classification Models Library

## Overview

This directory contains the complete model library for the ESP32 WildCAM wildlife classification system. All models are TensorFlow Lite INT8 quantized for optimal performance on ESP32-S3 hardware.

**⚠️ IMPORTANT**: All models currently available are DEMO models trained on synthetic data for integration testing and development. For production deployment, train models on real wildlife datasets using the comprehensive training pipeline documented in `ml/TRAINING_DEPLOYMENT_GUIDE.md`.

## Model Library Structure

```
models/
├── production/              # Production-ready comprehensive model
│   ├── wildlife_classifier_v2_quantized.tflite (186 KB)
│   ├── wildlife_classifier_v2_metadata.json
│   ├── wildlife_classifier_v2_labels.json
│   └── README.md
│
├── species_detection/       # Specialized species-specific models
│   ├── deer/               # Deer and cervid classifier (30 KB)
│   ├── bird/               # Bird species classifier (36 KB)
│   ├── mammal/             # Small-medium mammal classifier (36 KB)
│   └── predator/           # Large predator classifier (36 KB)
│
├── test/                   # Integration testing model
│   ├── wildlife_classifier_test_quantized.tflite (34 KB)
│   └── ...
│
├── model_manifest.json     # Model registry and specifications
└── README.md              # This file
```

## Available Models

### 1. Production Model (wildlife_classifier_v2)

**Location**: `production/wildlife_classifier_v2_quantized.tflite`

**Comprehensive wildlife classifier with 36 species coverage**

- **Size**: 186 KB
- **Species**: 36 (deer, predators, birds, mammals, etc.)
- **Inference**: ~850ms on ESP32-S3 @ 240MHz
- **Use Case**: General-purpose wildlife monitoring

**Supported Species**:
- Deer/Cervids: white-tailed deer, mule deer, elk, moose, caribou
- Large Predators: black bear, grizzly bear, mountain lion, gray wolf, coyote
- Medium Predators: bobcat, lynx, red fox, gray fox
- Medium Mammals: raccoon, opossum, skunk, porcupine, beaver
- Small Mammals: rabbit, squirrel, chipmunk, groundhog
- Birds: wild turkey, grouse, pheasant, crow, raven, eagle, hawk, owl
- Other: domestic dog, domestic cat, livestock, human, unknown

**Documentation**: See `production/README.md`

### 2. Species-Specific Models

#### Deer Classifier
**Location**: `species_detection/deer/deer_classifier.tflite`

- **Size**: 30 KB
- **Species**: 6 (cervid family)
- **Inference**: ~600ms
- **Use Case**: Specialized deer and elk monitoring

**Species**: white-tailed deer, mule deer, elk, moose, caribou, unknown

#### Bird Classifier
**Location**: `species_detection/bird/bird_classifier.tflite`

- **Size**: 36 KB
- **Species**: 7 (common birds)
- **Inference**: ~600ms
- **Use Case**: Avian wildlife monitoring

**Species**: wild turkey, crow, eagle, hawk, owl, woodpecker, unknown

#### Mammal Classifier
**Location**: `species_detection/mammal/mammal_classifier.tflite`

- **Size**: 36 KB
- **Species**: 7 (small-medium mammals)
- **Inference**: ~600ms
- **Use Case**: Small animal monitoring

**Species**: raccoon, squirrel, rabbit, opossum, skunk, beaver, unknown

#### Predator Classifier
**Location**: `species_detection/predator/predator_classifier.tflite`

- **Size**: 36 KB
- **Species**: 7 (large carnivores)
- **Inference**: ~600ms
- **Use Case**: Safety monitoring, predator alerts

**Species**: black bear, mountain lion, gray wolf, coyote, bobcat, lynx, unknown

### 3. Test Model (wildlife_classifier_test)

**Location**: `test/wildlife_classifier_test_quantized.tflite`

- **Size**: 34 KB
- **Species**: 10 (subset for testing)
- **Use Case**: Integration testing only

## Quick Start

### Loading the Production Model

```cpp
#include "firmware/src/ai/wildlife_classifier.h"

WildlifeClassifier classifier;

void setup() {
    WildlifeClassifier::ClassifierConfig config;
    config.confidenceThreshold = 0.7;
    
    if (classifier.initialize(config)) {
        Serial.println("✅ Classifier ready");
    }
}

void loop() {
    camera_fb_t* fb = esp_camera_fb_get();
    auto result = classifier.classifyFrame(fb);
    
    if (result.isValid) {
        Serial.printf("Species: %s (%.1f%%)\n", 
                     result.speciesName.c_str(), 
                     result.confidence * 100);
    }
    
    esp_camera_fb_return(fb);
}
```

### Loading Specialized Models

```cpp
#include "firmware/src/ai/vision/species_classifier.h"

SpeciesClassifier classifier;

void setup() {
    // Load specialized predator model for safety monitoring
    classifier.init();
    classifier.loadModel("/models/species_detection/predator/predator_classifier.tflite");
}

void checkForPredators() {
    CameraFrame frame = camera.capture();
    ClassificationResult result = classifier.classify(frame);
    
    if (result.confidence > 0.8) {
        Serial.printf("⚠️  Predator detected: %s\n", result.species_name.c_str());
        triggerAlert();
    }
}
```

## Model Selection Guide

| Use Case | Recommended Model | Priority | Notes |
|----------|------------------|----------|-------|
| General monitoring | wildlife_classifier_v2 | High | Comprehensive coverage |
| Deer/hunting season | deer_classifier | Medium | Specialized accuracy |
| Bird watching | bird_classifier | Medium | Avian focus |
| Small animals | mammal_classifier | Low | Rodents, small mammals |
| Safety/alerts | predator_classifier | Critical | Dangerous species |
| Integration testing | test_classifier | N/A | Testing only |

## Performance Comparison

| Model | Size | Species | Inference (ms) | Memory (KB) | Power (mA) |
|-------|------|---------|----------------|-------------|------------|
| wildlife_classifier_v2 | 186 KB | 36 | 850 | 1500 | 165 |
| deer_classifier | 30 KB | 6 | 600 | 512 | 140 |
| bird_classifier | 36 KB | 7 | 600 | 512 | 140 |
| mammal_classifier | 36 KB | 7 | 600 | 512 | 140 |
| predator_classifier | 36 KB | 7 | 600 | 512 | 140 |
| test_classifier | 34 KB | 10 | 500 | 512 | 140 |

## Hardware Requirements

### Minimum Requirements
- **Platform**: ESP32-S3
- **PSRAM**: 8 MB
- **Flash**: 4 MB
- **CPU**: 160 MHz (240 MHz recommended)

### Recommended Configuration
- **Platform**: ESP32-S3-CAM
- **PSRAM**: 8 MB
- **Flash**: 8+ MB
- **CPU**: 240 MHz
- **Camera**: OV2640 or better

## Deployment Strategies

### Strategy 1: Single Comprehensive Model
```json
{
  "primary_model": "wildlife_classifier_v2",
  "enable_fallback": false,
  "confidence_threshold": 0.7
}
```
**Best for**: General wildlife monitoring, research

### Strategy 2: Cascaded Specialized Models
```json
{
  "stage1": "motion_detector",
  "stage2": "wildlife_classifier_v2",
  "stage3_predator": "predator_classifier",
  "stage3_deer": "deer_classifier"
}
```
**Best for**: High-accuracy needs, specific species focus

### Strategy 3: Power-Optimized
```json
{
  "primary_model": "predator_classifier",
  "enable_motion_trigger": true,
  "deep_sleep_enabled": true
}
```
**Best for**: Remote deployment, battery-powered

## Training Production Models

All current models are DEMO models. To train production models:

### 1. Collect Dataset
- 500-1000 images per species
- Diverse conditions (lighting, weather, angles)
- High quality (1080p minimum)
- Properly labeled

### 2. Train Model
```bash
cd /home/runner/work/WildCAM_ESP32/WildCAM_ESP32

# Train comprehensive model
python3 backend/ml/model_trainer.py \
    --dataset /path/to/wildlife_dataset \
    --output firmware/models/production \
    --model-name wildlife_classifier_v2 \
    --epochs 100

# Or train specialized model
python3 backend/ml/model_trainer.py \
    --dataset /path/to/deer_dataset \
    --output firmware/models/species_detection/deer \
    --model-name deer_classifier \
    --epochs 80
```

### 3. Evaluate
```bash
python3 backend/ml/model_evaluator.py \
    --model firmware/models/production/wildlife_classifier_v2.tflite \
    --test-data /path/to/test_set \
    --min-accuracy 0.95
```

### 4. Deploy
- Copy trained model to appropriate directory
- Update model_manifest.json
- Test on hardware
- Monitor field performance

## Model Management

### Updating Models

1. **Train new model** following training guide
2. **Validate accuracy** > 95% on test set
3. **Test on hardware** verify inference time and memory
4. **Update manifest** add entry to model_manifest.json
5. **Deploy** OTA update or manual flash

### Version Control

Models follow semantic versioning:
- **Major**: Breaking changes in architecture or API
- **Minor**: New species added, improved accuracy
- **Patch**: Bug fixes, optimization

### Model Registry

All models are registered in `model_manifest.json` with:
- Model ID and version
- File paths and metadata
- Performance specifications
- Hardware requirements
- Deployment configuration

## Troubleshooting

### Model Won't Load
```bash
# Check file exists
ls -l firmware/models/production/*.tflite

# Verify format
file firmware/models/production/*.tflite

# Check PSRAM
pio run -t monitor  # Look for PSRAM initialization
```

### Low Accuracy
- **Cause**: Demo model on real data
- **Solution**: Train on real wildlife dataset
- **Timeline**: 40-80 hours (see TRAINING_DEPLOYMENT_GUIDE.md)

### Slow Inference
- **Check**: CPU frequency (should be 240 MHz)
- **Check**: PSRAM enabled
- **Check**: Input image resolution (should be 224x224)
- **Try**: Specialized smaller model instead

### Out of Memory
- **Use**: Smaller specialized model
- **Enable**: PSRAM
- **Reduce**: Input resolution
- **Check**: Memory leaks in application

## Integration Examples

See `examples/` directory for:
- `test_model_integration_example.cpp` - Basic integration
- `advanced_classification_example.cpp` - Multi-model cascade
- `power_optimized_example.cpp` - Battery-powered deployment

## Documentation

- **Training Guide**: `ml/TRAINING_DEPLOYMENT_GUIDE.md`
- **Quick Start**: `ml/QUICKSTART.md`
- **ML Integration**: `ML_INTEGRATION_README.md`
- **Model Manifest**: `model_manifest.json`
- **API Reference**: `firmware/src/ai/wildlife_classifier.h`

## Support

- **Issues**: GitHub Issues with `machine-learning` label
- **Discussions**: GitHub Discussions
- **Examples**: `examples/` directory
- **Documentation**: `docs/` and `ml/` directories

## License

Models are part of the ESP32 WildCAM project and follow the same license terms.

---

**Last Updated**: 2025-10-29  
**Model Library Version**: 2.0.0  
**Status**: Demo models available, production training required
