# Deer Wildlife Classifier

## Overview
Specialized TensorFlow Lite model for deer species classification.

**⚠️ DEMO MODEL**: This model is trained on synthetic data for demonstration and testing.
For production deployment, train on real wildlife datasets using the training pipeline.

## Model Specifications
- **Category**: Deer Wildlife
- **Architecture**: Lightweight CNN optimized for ESP32
- **Input**: 224x224x3 RGB images (uint8)
- **Output**: 6 species classes
- **Quantization**: INT8
- **Model Size**: 29.2 KB
- **Target Platform**: ESP32-S3 (8MB PSRAM)

## Supported Species
- white_tailed_deer
- mule_deer
- elk
- moose
- caribou
- unknown

## Performance Targets
- **Inference Time**: < 600ms on ESP32-S3 @ 240MHz
- **Memory Usage**: ~512KB RAM
- **Target Accuracy**: > 95% (on real data)
- **Power Consumption**: < 180mA during inference

## Usage

### Load Model in ESP32
```cpp
#include "firmware/src/ai/vision/species_classifier.h"

SpeciesClassifier classifier;

void setup() {
    classifier.init();
    classifier.loadModel("/models/species_detection/deer_classifier.tflite");
}

void loop() {
    CameraFrame frame = camera.capture();
    ClassificationResult result = classifier.classify(frame);
    
    Serial.printf("Species: %s\n", result.species_name.c_str());
    Serial.printf("Confidence: %.2f\n", result.confidence);
}
```

### Integration with Detection Pipeline
```cpp
#include "firmware/ml_models/ai_detection_interface.h"

AIDetectionInterface ai;
ai.loadSpecializedModel("deer_classifier.tflite");

// Automatic classification after motion detection
auto results = ai.enhancedDetection(frame_data, frame_size, width, height);
```

## Training for Production

To train a production model with real data:

1. **Collect Dataset**
   - Gather 500-1000 images per species
   - Use diverse lighting and weather conditions
   - Include various angles and distances

2. **Label Data**
   - Use CVAT or Label Studio
   - Create bounding boxes
   - Verify label accuracy

3. **Train Model**
   ```bash
   python backend/ml/model_trainer.py \
       --dataset /path/to/deer_dataset \
       --output firmware/models/species_detection \
       --category deer \
       --epochs 100
   ```

4. **Validate**
   ```bash
   python backend/ml/model_evaluator.py \
       --model firmware/models/species_detection/deer_classifier.tflite \
       --test-data /path/to/test_set
   ```

5. **Deploy**
   - Flash to ESP32
   - Validate in field conditions
   - Monitor performance metrics

## Files
- `deer_classifier.tflite` - TFLite model
- `deer_metadata.json` - Model specifications
- `deer_labels.json` - Species labels
- `README.md` - This documentation

## Next Steps
- Train on real wildlife datasets
- Validate accuracy on test data
- Deploy and monitor in production
- Iterate based on field performance

## Support
- **Documentation**: See [ML_INTEGRATION_README.md](../../ML_INTEGRATION_README.md)
- **Training Guide**: [ml/TRAINING_DEPLOYMENT_GUIDE.md](../../ml/TRAINING_DEPLOYMENT_GUIDE.md)
- **Issues**: GitHub Issues with `machine-learning` label

Generated: 2025-10-29 02:35:42
