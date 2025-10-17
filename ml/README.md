# ML Directory - YOLO-tiny Model Training and Deployment

This directory contains everything needed for wildlife species classification model training and deployment on ESP32.

## 📁 Directory Contents

### Documentation
- **[TRAINING_DEPLOYMENT_GUIDE.md](TRAINING_DEPLOYMENT_GUIDE.md)** - Complete 7-phase training pipeline (40-80 hours)
- **[QUICKSTART.md](QUICKSTART.md)** - 5-minute quick start guide
- **[IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md)** - Current status and deliverables

### Scripts
- **[test_model_demo.py](test_model_demo.py)** - Python demo for test model inference and benchmarking
- **[train_model.py](train_model.py)** - Basic training script (alternative to backend/ml/)

## 🚀 Quick Start

### 1. Test Model (Ready Now)

The test model is already generated and ready to use:

```bash
# Run benchmark
python3 ml/test_model_demo.py --benchmark

# View quick start guide
cat ml/QUICKSTART.md
```

**Location**: `firmware/models/test/wildlife_classifier_test_quantized.tflite` (34 KB)

**⚠️ WARNING**: Test model trained on synthetic data. For testing only!

### 2. Train Production Model

Follow the comprehensive training guide:

```bash
# Read complete guide
cat ml/TRAINING_DEPLOYMENT_GUIDE.md

# Quick summary:
# Phase 1: Data Collection (20 hours) - 1000+ images per species
# Phase 2: Data Labeling (10-15 hours)
# Phase 3: Model Training (10-20 hours)
# Phase 4: Model Optimization (5 hours)
# Phase 5: Model Evaluation (3 hours)
# Phase 6: Deployment (5 hours)
# Phase 7: Field Testing (Ongoing)

# Install dependencies
pip install ultralytics tensorflow opencv-python

# Train YOLOv8 model
python3 ml/train_model.py --model yolov8n --epochs 300 --data dataset/wildlife.yaml
```

## 📊 Test Model Specifications

- **Size**: 34 KB (INT8 quantized)
- **Input**: 224x224x3 RGB (uint8)
- **Output**: 10 species classes (uint8)
- **Inference**: ~5ms on CPU, ~500-800ms on ESP32-S3 (estimated)
- **Species**: deer, bear, fox, wolf, lion, raccoon, coyote, elk, turkey, unknown

## 🎯 Production Model Targets

- **Accuracy**: >85% (target: >90%)
- **Inference Time**: <200ms on ESP32-S3
- **Model Size**: <2MB (ideally <1MB)
- **False Positive Rate**: <10%
- **Species**: 10-20 initially, expand to 30+

## 📖 Documentation Links

### Getting Started
- [Quick Start Guide](QUICKSTART.md) - Get started in 5 minutes
- [Implementation Status](IMPLEMENTATION_STATUS.md) - Current deliverables
- [Integration Example](../examples/test_model_integration_example.cpp) - C++ example

### Training Pipeline
- [Training Guide](TRAINING_DEPLOYMENT_GUIDE.md) - Complete training documentation
- [Model Trainer](../backend/ml/model_trainer.py) - Training script
- [Model Evaluator](../backend/ml/model_evaluator.py) - Evaluation tools
- [Deployment Pipeline](../backend/ml/deployment_pipeline.py) - Deployment automation

### Model Information
- [Model Manifest](../firmware/models/model_manifest.json) - Model specifications
- [Test Model README](../firmware/models/test/README.md) - Test model docs
- [Model Metadata](../firmware/models/test/model_metadata.json) - Test model metadata

### Integration
- [ML Integration Guide](../ML_INTEGRATION_README.md) - Main integration guide
- [Species Classifier API](../firmware/src/ai/vision/species_classifier.h) - C++ API
- [YOLO Detector](../firmware/ml_models/yolo_tiny/yolo_tiny_detector.cpp) - YOLO implementation

## 🛠️ Scripts Usage

### test_model_demo.py

Demonstrate test model usage:

```bash
# Run benchmark (default: 100 iterations)
python3 ml/test_model_demo.py --benchmark

# Run longer benchmark
python3 ml/test_model_demo.py --benchmark --iterations 500

# Classify an image (when available)
python3 ml/test_model_demo.py --image path/to/wildlife_photo.jpg

# Use custom model path
python3 ml/test_model_demo.py --model path/to/model.tflite --benchmark
```

### train_model.py

Basic training script (see TRAINING_DEPLOYMENT_GUIDE.md for full details):

```python
from ml.train_model import WildlifeModelTrainer

# Create trainer
trainer = WildlifeModelTrainer()

# Build model
trainer.build_model()

# Train
history = trainer.train('dataset/', epochs=100)

# Convert to TFLite
from ml.train_model import TinyMLModel
converter = TinyMLModel()
converter.convert_to_tflite('best_model.h5')
```

## 🔧 Requirements

### Python Dependencies

```bash
# For test model and demo
pip install tensorflow numpy pillow

# For training (full pipeline)
pip install ultralytics torch torchvision
pip install tensorflow opencv-python scikit-learn
pip install matplotlib seaborn

# Optional: For Roboflow integration
pip install roboflow
```

### Hardware Requirements

#### Training
- **GPU**: 8GB+ VRAM (NVIDIA recommended)
- **RAM**: 16GB+
- **Storage**: 50GB+ for datasets

#### Deployment
- **Platform**: ESP32-S3
- **PSRAM**: 8MB recommended (2MB minimum)
- **Flash**: 4MB minimum
- **CPU**: 160-240MHz

## 📚 Recommended Datasets

### Public Wildlife Datasets

1. **iNaturalist** - https://www.inaturalist.org/
   - Millions of wildlife observations
   - Community-verified species

2. **Snapshot Serengeti** - https://www.zooniverse.org/projects/zooniverse/snapshot-serengeti
   - ~3 million camera trap images
   - Pre-labeled with species

3. **Caltech Camera Traps** - https://beerys.github.io/CaltechCameraTraps/
   - North American wildlife
   - High-quality data

4. **LILA BC** - http://lila.science/
   - Multiple camera trap datasets
   - Standardized format

## 🎓 Learning Resources

### Tutorials
- [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
- [YOLOv8 Documentation](https://docs.ultralytics.com/)
- [ESP32 ML Examples](https://github.com/espressif/tflite-micro-esp-examples)

### Research Papers
- YOLOv5/v8 papers
- MobileNet architectures
- Wildlife detection research

## 🤝 Contributing

To contribute models or improvements:

1. Train and evaluate your model
2. Document performance metrics
3. Test on ESP32 hardware
4. Submit pull request with:
   - Model file (.tflite)
   - Training code/notebook
   - Evaluation results
   - Documentation

## 📝 Notes

### Test Model Limitations
- Trained on synthetic data (~10% accuracy)
- Only for integration testing
- DO NOT use in production
- Train real model for field deployment

### Production Model Requirements
- Real wildlife dataset (1000+ images per species)
- Proper labeling and validation
- >85% accuracy on test set
- Field testing required

### Performance Optimization
- Use INT8 quantization
- Target 160x160 or 224x224 input
- Enable PSRAM on ESP32
- Set CPU to 240MHz during inference
- Consider model pruning

## 🐛 Troubleshooting

### Common Issues

**Model won't load**
- Check file path
- Verify TensorFlow Lite installed
- Ensure model is quantized correctly

**Slow inference**
- Reduce input size
- Use quantized model
- Enable XNNPACK delegate

**Low accuracy**
- Collect more diverse data
- Use data augmentation
- Try transfer learning
- Increase training epochs

**Memory errors**
- Use PSRAM
- Reduce batch size
- Smaller model architecture
- Free memory between inferences

## 📞 Support

- [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues) - Bug reports
- [GitHub Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions) - Questions
- Tag with `machine-learning` for ML-specific help

## 📄 License

MIT License - See LICENSE file for details

---

**Last Updated**: 2025-10-16  
**Version**: 1.0.0  
**Maintainer**: WildCAM Development Team
