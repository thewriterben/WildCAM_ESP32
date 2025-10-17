# YOLO-tiny Model Quick Start Guide

## TL;DR - Get Started in 5 Minutes

```bash
# Generate test model for integration testing
cd /home/runner/work/WildCAM_ESP32/WildCAM_ESP32
python3 backend/ml/generate_test_model.py

# Model files created at:
# firmware/models/test/wildlife_classifier_test_quantized.tflite (34 KB)
# firmware/models/test/model_metadata.json
# firmware/models/test/class_labels.json
```

**✅ Test model ready for integration testing!**

**⚠️ IMPORTANT**: This is a test model trained on synthetic data. DO NOT use in production.

## What You Get

### Test Model Specifications
- **Size**: 34 KB (fits easily in ESP32 flash)
- **Format**: TensorFlow Lite INT8 quantized
- **Input**: 224x224x3 RGB images (uint8)
- **Output**: 10 species classes
- **Inference time**: ~500ms (estimated on ESP32-S3)

### Supported Species (Test Model)
1. White-tailed Deer
2. Black Bear
3. Red Fox
4. Gray Wolf
5. Mountain Lion
6. Raccoon
7. Coyote
8. Elk
9. Wild Turkey
10. Unknown

## Using the Test Model

### 1. Load Model in ESP32 Code

```cpp
#include "firmware/src/ai/vision/species_classifier.h"

WildlifeClassifier classifier;

void setup() {
    // Initialize classifier
    if (classifier.init()) {
        Serial.println("Classifier initialized");
        
        // Load test model
        bool loaded = classifier.loadSpeciesModel(
            "/models/test/wildlife_classifier_test_quantized.tflite"
        );
        
        if (loaded) {
            Serial.println("Test model loaded successfully");
        } else {
            Serial.println("Failed to load test model");
        }
    }
}

void loop() {
    // Capture frame from camera
    CameraFrame frame = camera.capture();
    
    // Classify
    SpeciesResult result = classifier.classifyImage(frame);
    
    // Display results
    Serial.printf("Species: %s\n", result.specificName.c_str());
    Serial.printf("Confidence: %.2f\n", result.confidence);
}
```

### 2. Testing Without Hardware

Use Python to test the model:

```python
import tensorflow as tf
import numpy as np
from PIL import Image

# Load model
interpreter = tf.lite.Interpreter(
    model_path='firmware/models/test/wildlife_classifier_test_quantized.tflite'
)
interpreter.allocate_tensors()

# Get input/output details
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Load and preprocess test image
image = Image.open('test_deer.jpg')
image = image.resize((224, 224))
image_array = np.array(image, dtype=np.uint8)
image_array = np.expand_dims(image_array, axis=0)

# Run inference
interpreter.set_tensor(input_details[0]['index'], image_array)
interpreter.invoke()
output = interpreter.get_tensor(output_details[0]['index'])

# Get predicted class
species = ['white_tailed_deer', 'black_bear', 'red_fox', 'gray_wolf',
           'mountain_lion', 'raccoon', 'coyote', 'elk', 'wild_turkey', 'unknown']
predicted_class = np.argmax(output)
confidence = output[0][predicted_class] / 255.0  # De-quantize

print(f"Predicted: {species[predicted_class]}")
print(f"Confidence: {confidence:.2f}")
```

## What's Next?

### For Integration Testing
The test model is perfect for:
- ✅ Validating model loading code
- ✅ Testing inference pipeline
- ✅ Benchmarking performance
- ✅ Debugging classification logic
- ✅ UI/UX development

### For Production Use
You need a real trained model:

**Option 1: Quick Training (Recommended for Getting Started)**
1. Collect 1,000+ images per species (5-10 species)
2. Use transfer learning with MobileNet
3. Train for 50-100 epochs
4. Estimated time: 5-10 hours

**Option 2: Full Pipeline (Best Performance)**
1. Follow [TRAINING_DEPLOYMENT_GUIDE.md](TRAINING_DEPLOYMENT_GUIDE.md)
2. Collect 5,000+ images per species
3. Train custom YOLO model
4. Estimated time: 40-80 hours

**Option 3: Use Pre-trained Models**
1. Download wildlife detection models from Model Zoo
2. Fine-tune for your specific species
3. Convert to TensorFlow Lite
4. Estimated time: 2-5 hours

## Performance Expectations

### Test Model
- Accuracy: ~10% (random - synthetic data)
- Purpose: Integration testing only
- Status: ❌ NOT for production use

### Production Model (Target)
- Accuracy: >85% (target: >90%)
- False positive rate: <10%
- Inference time: <200ms
- Status: ✅ Ready for field deployment

## Benchmarking the Test Model

```python
import time
import tensorflow as tf
import numpy as np

# Load model
interpreter = tf.lite.Interpreter(
    model_path='firmware/models/test/wildlife_classifier_test_quantized.tflite'
)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

# Benchmark
num_runs = 100
times = []

for i in range(num_runs):
    # Random input
    dummy_input = np.random.randint(0, 255, (1, 224, 224, 3), dtype=np.uint8)
    
    # Time inference
    start = time.time()
    interpreter.set_tensor(input_details[0]['index'], dummy_input)
    interpreter.invoke()
    output = interpreter.get_tensor(output_details[0]['index'])
    end = time.time()
    
    times.append((end - start) * 1000)

print(f"Average inference time: {np.mean(times):.2f}ms")
print(f"Min: {np.min(times):.2f}ms")
print(f"Max: {np.max(times):.2f}ms")
print(f"Std dev: {np.std(times):.2f}ms")
```

Expected results on different platforms:
- **Desktop CPU**: 5-15ms
- **Raspberry Pi 4**: 20-50ms
- **ESP32-S3**: 500-800ms (estimated)

## Troubleshooting

### Model Won't Load
```
Error: Failed to allocate tensors
```
**Solution**: Ensure ESP32 has enough PSRAM (8MB recommended)

### Out of Memory
```
Error: Failed to allocate memory for tensor arena
```
**Solution**: Reduce tensor arena size or use PSRAM

### Slow Inference
```
Warning: Inference time >1000ms
```
**Solution**: 
- Use INT8 quantized model (already done)
- Reduce input size to 160x160
- Increase CPU frequency to 240MHz

### Low Confidence Scores
This is expected with the test model (trained on synthetic data). For production use, train a real model.

## Important Files

```
firmware/models/test/
├── wildlife_classifier_test_quantized.tflite  # The model file
├── model_metadata.json                        # Model specifications
├── class_labels.json                          # Species labels
└── README.md                                  # Model documentation

backend/ml/
├── generate_test_model.py                     # Test model generator
├── model_trainer.py                           # Production training
├── model_evaluator.py                         # Model evaluation
└── deployment_pipeline.py                     # Deployment automation

ml/
├── TRAINING_DEPLOYMENT_GUIDE.md               # Full training guide
├── QUICKSTART.md                              # This file
└── train_model.py                             # Simple training script
```

## Additional Resources

### Documentation
- [Full Training Guide](TRAINING_DEPLOYMENT_GUIDE.md) - Complete 40-80 hour pipeline
- [Model Manifest](../firmware/models/model_manifest.json) - Available models
- [Species Classifier API](../firmware/src/ai/vision/species_classifier.h) - Code reference

### Tools
- Test Model Generator: `backend/ml/generate_test_model.py`
- Model Trainer: `backend/ml/model_trainer.py`
- Model Evaluator: `backend/ml/model_evaluator.py`

### Community
- Report issues: [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- Discussions: [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)
- ML Help: Tag with `machine-learning` label

## FAQ

**Q: Can I use the test model in production?**
A: No. It's trained on synthetic data and has ~10% accuracy. Only use for integration testing.

**Q: How long does training take?**
A: 
- Quick training (transfer learning): 5-10 hours
- Full pipeline (custom YOLO): 40-80 hours
- Using pre-trained models: 2-5 hours

**Q: What accuracy can I expect?**
A: With proper training:
- Basic model: 80-85% accuracy
- Well-trained model: 85-90% accuracy
- Highly optimized: 90-95% accuracy

**Q: How much data do I need?**
A:
- Minimum: 1,000 images per species
- Recommended: 5,000 images per species
- Optimal: 10,000+ images per species

**Q: Can I use existing datasets?**
A: Yes! See [TRAINING_DEPLOYMENT_GUIDE.md](TRAINING_DEPLOYMENT_GUIDE.md) for list of public wildlife datasets.

**Q: How do I improve accuracy?**
A:
1. Collect more diverse training data
2. Use data augmentation
3. Try transfer learning
4. Ensemble multiple models
5. Collect field data for retraining

---

**Need Help?**
- Check [TRAINING_DEPLOYMENT_GUIDE.md](TRAINING_DEPLOYMENT_GUIDE.md) for detailed instructions
- Open an issue on GitHub
- Join our community discussions

**Last Updated**: 2025-10-16
**Version**: 1.0.0
