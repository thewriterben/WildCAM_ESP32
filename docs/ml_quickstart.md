# ML Integration Quick Start Guide

## Overview

Get started with machine learning integration for ESP32 Wildlife Camera in 5 minutes.

## Prerequisites

```bash
# Install Python dependencies
cd backend
pip install -r requirements.txt

# Verify installation
python -c "import tensorflow as tf; print('TensorFlow', tf.__version__)"
```

## Option 1: Quick Test (No Training Required)

### Generate Test Model

For immediate testing without training:

```bash
# Generate a minimal test model
cd backend/ml
python generate_test_model.py

# This creates:
# - firmware/models/test/wildlife_classifier_test_quantized.tflite
# - firmware/models/test/model_metadata.json
# - firmware/models/test/class_labels.json
```

### Test Integration

```bash
# Run integration tests
cd scripts
python ml_integration_test.py

# Expected output:
# ✅ Test Model Generation
# ✅ Model File Validation
# ✅ Metadata Validation
# ✅ Model Size Check
# ✅ ESP32 Compatibility
```

### Deploy to ESP32

```bash
# Copy test model to firmware
cp firmware/models/test/wildlife_classifier_test_quantized.tflite \
   ESP32WildlifeCAM-main/models/deployment/

# Update metadata
cp firmware/models/test/model_metadata.json \
   firmware/models/
```

## Option 2: Train Your Own Model

### 1. Prepare Dataset

```bash
# Organize your dataset
mkdir -p /data/wildlife_dataset/{train,validation,test}

# Structure:
# /data/wildlife_dataset/
#   ├── train/
#   │   ├── deer/
#   │   ├── bear/
#   │   └── fox/
#   ├── validation/
#   └── test/
```

### 2. Train Model

```bash
cd backend/ml

# Quick training (30 epochs)
python -c "
from model_trainer import WildlifeModelTrainer

trainer = WildlifeModelTrainer(
    dataset_dir='/data/wildlife_dataset',
    output_dir='/data/models',
    config={
        'epochs': 30,
        'batch_size': 32,
        'quantization': True
    }
)

# Train and convert
trainer.train_model()
trainer.convert_to_tflite(quantize=True)
"
```

### 3. Deploy Model

```bash
# Copy trained model
cp /data/models/wildlife_classifier_quantized.tflite \
   firmware/models/wildlife_classifier_v2.tflite

# Copy metadata
cp /data/models/tflite_conversion_info.json \
   firmware/models/
```

## ESP32 Integration

### Load Model in Firmware

```cpp
#include "ai/tinyml/inference_engine.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize inference engine
    InferenceEngine engine;
    engine.init();
    
    // Load model
    if (engine.loadModel("/models/wildlife_classifier_v2.tflite",
                        ModelType::MODEL_SPECIES_CLASSIFIER)) {
        Serial.println("✅ Model loaded successfully");
    } else {
        Serial.println("❌ Failed to load model");
    }
}
```

### Run Inference

```cpp
void loop() {
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    
    // Run inference
    CameraFrame frame = {fb->buf, fb->width, fb->height, fb->format};
    AIResult result = engine.runInference(frame, 
                                         ModelType::MODEL_SPECIES_CLASSIFIER);
    
    // Process result
    if (result.confidence > 0.7) {
        Serial.printf("Detected: %s (%.1f%%)\n",
                     result.label.c_str(),
                     result.confidence * 100);
    }
    
    esp_camera_fb_return(fb);
    delay(5000);
}
```

## Complete Pipeline (Automated)

### One-Command Deployment

```bash
cd backend/ml

# Run complete pipeline
python deployment_pipeline.py

# This automatically:
# 1. Collects data (if configured)
# 2. Trains model
# 3. Evaluates performance
# 4. Creates deployment package
```

### Configuration

Create `pipeline_config.json`:

```json
{
  "data_collection": {
    "enabled": true,
    "species_list": ["deer", "bear", "fox", "wolf"],
    "samples_per_species": 500
  },
  "model_training": {
    "enabled": true,
    "epochs": 30,
    "batch_size": 32,
    "quantization": true
  },
  "model_evaluation": {
    "enabled": true
  },
  "deployment": {
    "enabled": true
  }
}
```

Run pipeline:

```bash
python deployment_pipeline.py --config pipeline_config.json
```

## Verification

### Check Model

```bash
# Verify model exists
ls -lh firmware/models/wildlife_classifier_v2.tflite

# Check metadata
cat firmware/models/model_metadata.json | jq '.model_info'
```

### Test Inference

```bash
# Quick Python test
python -c "
import tensorflow as tf
import numpy as np

# Load model
interpreter = tf.lite.Interpreter(
    model_path='firmware/models/wildlife_classifier_v2.tflite'
)
interpreter.allocate_tensors()

# Get input/output details
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

print('✅ Model loaded successfully')
print(f'Input shape: {input_details[0][\"shape\"]}')
print(f'Output shape: {output_details[0][\"shape\"]}')

# Test inference
test_input = np.random.randint(0, 255, input_details[0]['shape'], dtype=np.uint8)
interpreter.set_tensor(input_details[0]['index'], test_input)
interpreter.invoke()

print('✅ Inference successful')
"
```

## Performance Targets

| Metric | Target | Status |
|--------|--------|--------|
| Model Size | < 15 MB | ✓ Check after training |
| Inference Time | < 1000 ms | ✓ Measure on ESP32 |
| Accuracy | > 90% | ✓ Validate on test set |
| Memory Usage | < 2 MB | ✓ Monitor PSRAM |

## Next Steps

1. **Collect More Data**: Improve accuracy with diverse datasets
2. **Fine-tune Model**: Adjust hyperparameters for better performance
3. **Optimize Model**: Apply pruning and compression
4. **Deploy to Multiple Devices**: Scale across camera network
5. **Enable Continuous Learning**: Implement federated learning

## Common Commands

```bash
# Generate test model
python backend/ml/generate_test_model.py

# Train model
python backend/ml/model_trainer.py

# Run full pipeline
python backend/ml/deployment_pipeline.py

# Test integration
python scripts/ml_integration_test.py

# Deploy model
cp /path/to/model.tflite firmware/models/
```

## Troubleshooting

### Model won't load
```bash
# Check file exists
ls firmware/models/*.tflite

# Verify format
file firmware/models/*.tflite

# Check permissions
chmod 644 firmware/models/*.tflite
```

### Low accuracy
```bash
# Collect more training data
# Increase training epochs
# Adjust learning rate
# Try different base model (MobileNetV2 vs V3)
```

### Slow inference
```bash
# Verify INT8 quantization applied
# Enable PSRAM in ESP32 config
# Increase CPU frequency to 240MHz
# Reduce input image size if acceptable
```

## Resources

- **Full Guide**: [ml_integration_guide.md](ml_integration_guide.md)
- **ML Workflow**: [ML_WORKFLOW.md](ML_WORKFLOW.md)
- **AI Architecture**: [ai/ai_architecture.md](ai/ai_architecture.md)
- **API Reference**: Check source code documentation

## Support

Need help? 
- Open an issue: https://github.com/thewriterben/WildCAM_ESP32/issues
- Check documentation: https://github.com/thewriterben/WildCAM_ESP32/docs
- Review examples: `examples/` directory

---

**Ready to deploy?** Follow [ml_integration_guide.md](ml_integration_guide.md) for detailed instructions.
