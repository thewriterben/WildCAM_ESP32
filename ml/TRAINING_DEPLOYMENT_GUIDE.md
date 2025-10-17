# YOLO-tiny Model Training and Deployment Guide

## Overview

This guide provides step-by-step instructions for training and deploying a YOLO-tiny model for wildlife species classification on the ESP32 WildCAM platform.

## Current Status

✅ **Framework**: Complete and functional
- Species classifier interface implemented (`firmware/src/ai/vision/species_classifier.h`)
- YOLO-tiny detector framework ready (`firmware/ml_models/yolo_tiny/yolo_tiny_detector.cpp`)
- Model loading and inference infrastructure in place
- TensorFlow Lite integration configured

📦 **Test Model**: Available for integration testing
- Location: `firmware/models/test/wildlife_classifier_test_quantized.tflite`
- Size: 34 KB (quantized INT8)
- Species: 10 test classes (white-tailed deer, black bear, red fox, etc.)
- **⚠️ WARNING**: Test model trained on synthetic data - DO NOT use in production

🔄 **Production Model**: Not yet trained
- Estimated effort: 40-80 hours
- Requires real wildlife dataset collection and labeling

## Quick Start: Using the Test Model

For immediate integration testing:

```bash
# Generate test model (already done)
python3 backend/ml/generate_test_model.py

# Test model files created at:
# - firmware/models/test/wildlife_classifier_test_quantized.tflite
# - firmware/models/test/model_metadata.json
# - firmware/models/test/class_labels.json
```

The test model can be used to:
- Validate ML integration pipeline
- Test model loading and inference
- Benchmark inference performance
- Develop and debug species classification features

## Production Model Training Pipeline

### Phase 1: Data Collection (20 hours)

#### 1.1 Dataset Requirements
- **Minimum**: 1,000+ images per species
- **Recommended**: 5,000+ images per species
- **Total species**: 10-20 (start small, expand later)
- **Image format**: JPEG/PNG, 640x640 minimum resolution
- **Diversity required**:
  - Multiple environments (forest, plains, mountains)
  - Different lighting conditions (dawn, day, dusk, night)
  - Various weather conditions
  - Multiple angles and distances
  - Juvenile and adult specimens

#### 1.2 Recommended Datasets

**Public Wildlife Datasets:**
1. **iNaturalist** - https://www.inaturalist.org/
   - Millions of wildlife observations
   - Community-verified species identification
   - Geographic diversity

2. **Snapshot Serengeti** - https://www.zooniverse.org/projects/zooniverse/snapshot-serengeti
   - Camera trap images from Africa
   - Pre-labeled with species and behaviors
   - ~3 million images

3. **Caltech Camera Traps** - https://beerys.github.io/CaltechCameraTraps/
   - North American wildlife
   - High-quality camera trap data
   - Multiple species

4. **LILA BC (Labeled Information Library of Alexandria: Biology and Conservation)**
   - http://lila.science/
   - Multiple camera trap datasets
   - Standardized format

**Custom Collection:**
- Use existing WildCAM deployments to collect field data
- Ensure proper permissions for data usage
- Follow ethical wildlife photography guidelines

#### 1.3 Data Organization

```
dataset/
├── images/
│   ├── train/
│   │   ├── white_tailed_deer/
│   │   ├── black_bear/
│   │   ├── red_fox/
│   │   └── ...
│   ├── val/
│   │   └── (same structure)
│   └── test/
│       └── (same structure)
└── labels/
    ├── train/
    ├── val/
    └── test/
```

### Phase 2: Data Labeling (10-15 hours)

#### 2.1 Labeling Tools

**Recommended Tools:**
1. **Label Studio** - https://labelstud.io/
   - Open source
   - Supports bounding boxes
   - YOLO format export

2. **CVAT** - https://github.com/opencv/cvat
   - Collaborative labeling
   - Video support
   - Multiple export formats

3. **Roboflow** - https://roboflow.com/
   - Cloud-based
   - Auto-labeling features
   - Dataset augmentation

#### 2.2 Labeling Guidelines

For each image:
1. Draw bounding box around each animal
2. Assign species label
3. Mark unclear/partial specimens as "unknown"
4. Include metadata:
   - Time of day
   - Weather conditions (if available)
   - Animal behavior (optional)

**Quality standards:**
- Tight bounding boxes (minimal background)
- Consistent species identification
- Handle edge cases (distant animals, partial views)

#### 2.3 Label Format (YOLO format)

```
<class_id> <x_center> <y_center> <width> <height>
```

Example: `0 0.5 0.5 0.3 0.4` (normalized 0-1)

### Phase 3: Model Training (10-20 hours)

#### 3.1 Training Environment Setup

**Hardware Requirements:**
- GPU with 8GB+ VRAM (NVIDIA recommended)
- 16GB+ RAM
- 50GB+ free storage

**Software Setup:**
```bash
# Create virtual environment
python3 -m venv wildlife_ml_env
source wildlife_ml_env/bin/activate

# Install dependencies
pip install ultralytics torch torchvision
pip install tensorflow opencv-python pillow
pip install tensorflowjs tensorflowlite

# For Roboflow integration
pip install roboflow
```

#### 3.2 Train YOLO-tiny Model

**Option A: Using Ultralytics YOLOv8 (Recommended)**

```python
from ultralytics import YOLO

# Start with YOLOv8n (nano) - smallest model
model = YOLO('yolov8n.pt')

# Train on custom dataset
results = model.train(
    data='dataset/wildlife.yaml',
    epochs=300,
    imgsz=640,
    batch=16,
    device='0',  # GPU
    patience=50,
    save=True,
    project='wildlife_detection',
    name='yolo_wildlife_v1'
)

# Validate
metrics = model.val()
print(f"mAP@50: {metrics.box.map50}")
print(f"mAP@50-95: {metrics.box.map}")

# Export for ESP32
model.export(format='tflite', int8=True, imgsz=160)
```

**Option B: Using TensorFlow/Keras**

```python
# Use the existing model_trainer.py
from backend.ml.model_trainer import WildlifeModelTrainer

trainer = WildlifeModelTrainer()
trainer.build_model()
history = trainer.train('dataset/', epochs=100)
trainer.save_model('wildlife_classifier.h5')
```

#### 3.3 Training Configuration

**dataset/wildlife.yaml:**
```yaml
path: /path/to/dataset
train: images/train
val: images/val
test: images/test

nc: 10  # number of classes
names: ['white_tailed_deer', 'black_bear', 'red_fox', 'gray_wolf', 
        'mountain_lion', 'raccoon', 'coyote', 'elk', 'wild_turkey', 'unknown']
```

#### 3.4 Training Monitoring

Track metrics:
- Training loss
- Validation loss
- mAP (mean Average Precision)
- Precision and Recall per class
- Inference time

Use TensorBoard for visualization:
```bash
tensorboard --logdir=runs/wildlife_detection
```

### Phase 4: Model Optimization (5 hours)

#### 4.1 Quantization

Convert to INT8 quantization for ESP32:

```python
import tensorflow as tf

# Load trained model
model = tf.keras.models.load_model('wildlife_classifier.h5')

# Convert with INT8 quantization
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]

# Representative dataset for calibration
def representative_data_gen():
    for image in dataset.take(100):
        yield [image]

converter.representative_dataset = representative_data_gen
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.uint8
converter.inference_output_type = tf.uint8

tflite_model = converter.convert()

# Save
with open('wildlife_classifier_quantized.tflite', 'wb') as f:
    f.write(tflite_model)
```

#### 4.2 Model Pruning

Reduce model size:

```python
import tensorflow_model_optimization as tfmot

# Apply pruning
pruning_params = {
    'pruning_schedule': tfmot.sparsity.keras.PolynomialDecay(
        initial_sparsity=0.0,
        final_sparsity=0.5,
        begin_step=0,
        end_step=1000
    )
}

model_for_pruning = tfmot.sparsity.keras.prune_low_magnitude(
    model, **pruning_params
)

# Retrain
model_for_pruning.compile(
    optimizer='adam',
    loss='categorical_crossentropy',
    metrics=['accuracy']
)

# Callbacks
callbacks = [
    tfmot.sparsity.keras.UpdatePruningStep(),
    tfmot.sparsity.keras.PruningSummaries(log_dir='logs')
]

model_for_pruning.fit(X_train, y_train, callbacks=callbacks, epochs=10)

# Strip pruning wrappers
model_for_export = tfmot.sparsity.keras.strip_pruning(model_for_pruning)
```

#### 4.3 Target Specifications

**ESP32-S3 Constraints:**
- Model size: <2MB (preferably <1MB)
- Inference time: <200ms target
- Memory usage: <4MB RAM
- Input size: 160x160 or 224x224 RGB

**Optimization Goals:**
- Accuracy: >85% (target: >90%)
- False positive rate: <10%
- Latency: <200ms per inference
- Power consumption: <200mA during inference

### Phase 5: Model Evaluation (3 hours)

#### 5.1 Performance Metrics

Evaluate on test set:

```python
from backend.ml.model_evaluator import ModelEvaluator

evaluator = ModelEvaluator()
metrics = evaluator.evaluate_model(
    model_path='wildlife_classifier_quantized.tflite',
    test_dataset='dataset/test'
)

print(f"Overall Accuracy: {metrics['accuracy']:.3f}")
print(f"Precision: {metrics['precision']:.3f}")
print(f"Recall: {metrics['recall']:.3f}")
print(f"F1 Score: {metrics['f1_score']:.3f}")

# Per-class metrics
for species, acc in metrics['per_class_accuracy'].items():
    print(f"{species}: {acc:.3f}")
```

#### 5.2 Edge Case Testing

Test challenging scenarios:
- Low light conditions
- Partially obscured animals
- Multiple animals in frame
- Motion blur
- Weather conditions (rain, fog, snow)
- Distant animals

#### 5.3 Inference Benchmarking

```python
import time
import numpy as np

# Benchmark on ESP32 simulator or actual hardware
def benchmark_inference(model, num_runs=100):
    times = []
    for _ in range(num_runs):
        dummy_input = np.random.randint(0, 255, (1, 224, 224, 3), dtype=np.uint8)
        
        start = time.time()
        predictions = model.predict(dummy_input)
        end = time.time()
        
        times.append((end - start) * 1000)  # Convert to ms
    
    print(f"Average inference time: {np.mean(times):.2f}ms")
    print(f"Min: {np.min(times):.2f}ms, Max: {np.max(times):.2f}ms")
    print(f"95th percentile: {np.percentile(times, 95):.2f}ms")
```

### Phase 6: Deployment (5 hours)

#### 6.1 Model File Preparation

```bash
# Create deployment package
mkdir -p firmware/models/production
cp wildlife_classifier_quantized.tflite firmware/models/production/
cp class_labels.json firmware/models/production/
cp model_metadata.json firmware/models/production/
```

#### 6.2 Update Model Metadata

Edit `firmware/models/model_metadata.json`:

```json
{
  "models": [
    {
      "id": "wildlife_classifier_v1_prod",
      "name": "wildlife_classifier_v1.tflite",
      "version": "1.0.0",
      "type": "species_classification",
      "file_size_bytes": 1048576,
      "accuracy_metrics": {
        "top1_accuracy": 0.89,
        "top5_accuracy": 0.96
      },
      "performance": {
        "inference_time_ms": 180,
        "memory_usage_kb": 1024
      }
    }
  ]
}
```

#### 6.3 Flash to ESP32

**Using PlatformIO:**
```bash
cd firmware
# Upload filesystem with model
pio run --target uploadfs

# Upload firmware
pio run --target upload
```

**Using Arduino IDE:**
1. Tools -> ESP32 Sketch Data Upload
2. Upload model files to SPIFFS/LittleFS
3. Upload sketch

#### 6.4 Verify Deployment

Connect to serial monitor and verify:
```
Model loaded: wildlife_classifier_v1.tflite
Model size: 1.0 MB
Input shape: [1, 224, 224, 3]
Output classes: 10
First inference: 185ms
Average inference: 178ms
```

### Phase 7: Field Testing (Ongoing)

#### 7.1 Initial Field Deployment

Deploy 3-5 test units:
- Different locations/habitats
- Monitor performance over 1-2 weeks
- Collect inference logs
- Review false positives/negatives

#### 7.2 Performance Monitoring

Track metrics:
- Detection accuracy
- Battery life impact
- Inference time distribution
- Memory usage
- False positive/negative rates

#### 7.3 Iterative Improvement

Based on field results:
1. Collect misclassified examples
2. Add to training dataset
3. Retrain model
4. Re-evaluate and redeploy

## Using Pre-trained Models

Alternative to training from scratch:

### Option 1: Transfer Learning

Start with pre-trained ImageNet model:

```python
from tensorflow.keras.applications import MobileNetV3Small

base_model = MobileNetV3Small(
    input_shape=(224, 224, 3),
    include_top=False,
    weights='imagenet'
)

# Freeze base layers
base_model.trainable = False

# Add custom classification head
model = tf.keras.Sequential([
    base_model,
    tf.keras.layers.GlobalAveragePooling2D(),
    tf.keras.layers.Dense(128, activation='relu'),
    tf.keras.layers.Dropout(0.3),
    tf.keras.layers.Dense(num_species, activation='softmax')
])

# Train only classification head
model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])
model.fit(X_train, y_train, epochs=50)

# Fine-tune: unfreeze base and train with low learning rate
base_model.trainable = True
model.compile(optimizer=tf.keras.optimizers.Adam(1e-5), 
              loss='categorical_crossentropy', 
              metrics=['accuracy'])
model.fit(X_train, y_train, epochs=20)
```

### Option 2: Partner with ML Researchers

Consider collaborating with:
- University wildlife research labs
- Conservation organizations
- ML/AI research groups
- Open source model repositories

Benefits:
- Access to larger datasets
- Expertise in model architecture
- Shared computational resources
- Peer review and validation

## Troubleshooting

### Common Issues

**1. Model too large for ESP32**
- Reduce input size (160x160 instead of 224x224)
- Apply more aggressive quantization
- Use model pruning
- Consider simpler architecture

**2. Inference too slow**
- Reduce model complexity
- Lower input resolution
- Use PSRAM for model storage
- Optimize TensorFlow Lite operations

**3. Low accuracy**
- Collect more training data
- Improve data diversity
- Use data augmentation
- Adjust class balance
- Try transfer learning

**4. High false positive rate**
- Add "empty" class with non-animal images
- Increase confidence threshold
- Use ensemble models
- Implement temporal filtering (multiple frames)

**5. Memory issues**
- Use quantized models (INT8)
- Stream model from flash instead of loading to RAM
- Reduce input buffer size
- Free memory between inferences

## Resources

### Documentation
- [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
- [Ultralytics YOLOv8](https://docs.ultralytics.com/)
- [ESP32 TensorFlow Lite](https://github.com/espressif/tflite-micro-esp-examples)

### Datasets
- iNaturalist: https://www.inaturalist.org/
- Snapshot Serengeti: https://www.zooniverse.org/projects/zooniverse/snapshot-serengeti
- LILA BC: http://lila.science/

### Tools
- Label Studio: https://labelstud.io/
- Roboflow: https://roboflow.com/
- TensorBoard: https://www.tensorflow.org/tensorboard

### Community
- ESP32 Forum: https://esp32.com/
- TensorFlow Lite Community: https://www.tensorflow.org/lite/community
- Conservation Technology Network: https://www.wildlabs.net/

## Next Steps

1. **Immediate** (Use test model):
   - Test ML integration pipeline
   - Validate inference performance
   - Develop species classification UI

2. **Short-term** (1-3 months):
   - Collect initial dataset (1000+ images)
   - Train basic 5-10 species model
   - Deploy to test units

3. **Medium-term** (3-6 months):
   - Expand to 15-20 species
   - Implement behavior detection
   - Deploy to production

4. **Long-term** (6-12 months):
   - Achieve >90% accuracy
   - Support 30+ species
   - Implement federated learning
   - Regional model optimization

## Contributing

To contribute models or improvements:
1. Fork the repository
2. Train/optimize your model
3. Document performance metrics
4. Submit pull request with:
   - Model file (.tflite)
   - Training code/notebook
   - Evaluation results
   - Documentation

## License

Models and training code are released under MIT License. See LICENSE file for details.

---

**Last Updated**: 2025-10-16
**Version**: 1.0.0
**Maintainer**: WildCAM Development Team
