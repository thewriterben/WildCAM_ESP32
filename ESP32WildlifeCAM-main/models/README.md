# AI Model Storage and Management

This directory contains the AI models and related files for the ESP32 Wildlife Camera system.

## Directory Structure

```
models/
├── deployment/           # Production models ready for deployment
│   ├── species_classifier_v1.0.0.tflite
│   ├── behavior_analyzer_v1.0.0.tflite
│   ├── motion_detector_v1.0.0.tflite
│   └── model_manifest.json
├── training/            # Training datasets and scripts
│   ├── species_dataset/
│   │   ├── mammals/
│   │   ├── birds/
│   │   └── others/
│   ├── behavior_dataset/
│   └── training_scripts/
└── evaluation/          # Model evaluation and benchmarks
    ├── benchmarks/
    └── accuracy_reports/
```

## Model Types

### 1. Species Classifier
- **File**: `species_classifier_v1.0.0.tflite`
- **Size**: ~500KB
- **Input**: 224x224 RGB image
- **Output**: 50 species classes + confidence scores
- **Accuracy**: 92% on validation set
- **Optimizations**: INT8 quantization, pruned weights

### 2. Behavior Analyzer
- **File**: `behavior_analyzer_v1.0.0.tflite`
- **Size**: ~300KB
- **Input**: 160x160 RGB image sequence (3 frames)
- **Output**: 12 behavior classes + confidence scores
- **Accuracy**: 87% on validation set
- **Optimizations**: Temporal convolutions, quantized

### 3. Motion Detector
- **File**: `motion_detector_v1.0.0.tflite`
- **Size**: ~100KB
- **Input**: 96x96 grayscale difference image
- **Output**: Motion probability + bounding box
- **Accuracy**: 95% motion detection
- **Optimizations**: Ultra-light architecture, binary operations

## Model Deployment

### Loading Models
Models are automatically loaded on system startup from the `/models/deployment/` directory. The system checks for:

1. Model file existence
2. Version compatibility
3. Checksum validation
4. Memory requirements

### Model Updates
The system supports over-the-air model updates through:

1. **WiFi Downloads**: Direct download from model repository
2. **Edge Impulse**: Integration with Edge Impulse platform
3. **LoRa Mesh**: Model distribution through mesh network
4. **SD Card**: Manual model updates via SD card

### Memory Management
Models are loaded into PSRAM when available, with fallback to internal RAM:

- **Species Classifier**: Requires 512KB arena
- **Behavior Analyzer**: Requires 384KB arena
- **Motion Detector**: Requires 128KB arena

## Performance Targets

### Inference Speed
- **Species Classification**: < 1.5 seconds
- **Behavior Analysis**: < 1.0 seconds
- **Motion Detection**: < 0.3 seconds

### Power Consumption
- **High Performance Mode**: 250mA average
- **Balanced Mode**: 180mA average
- **Power Saving Mode**: 120mA average

### Accuracy Targets
- **Species Identification**: > 90% accuracy
- **Behavior Classification**: > 85% accuracy
- **Motion Detection**: > 95% accuracy

## Model Training Guidelines

### Data Collection
1. **Diverse Environments**: Various lighting, weather, seasons
2. **Multiple Angles**: Different camera positions and distances
3. **Species Variety**: Regional wildlife representation
4. **Behavior Contexts**: Natural and human-influenced behaviors

### Training Best Practices
1. **Data Augmentation**: Rotation, scaling, color variation
2. **Balanced Datasets**: Equal representation across classes
3. **Validation Strategy**: Time-based splits for temporal robustness
4. **Edge Optimization**: Target ESP32 hardware constraints

### Model Optimization
1. **Quantization**: INT8 post-training quantization
2. **Pruning**: Remove redundant weights (up to 50%)
3. **Knowledge Distillation**: Teacher-student model compression
4. **Architecture Search**: Hardware-aware neural architecture search

## Integration with Edge Impulse

### Project Setup
1. Create Edge Impulse project for wildlife monitoring
2. Configure data sources (camera, sensors)
3. Set up processing blocks for image/audio preprocessing
4. Design learning blocks for classification/detection

### Deployment Workflow
1. **Data Collection**: Automated from deployed cameras
2. **Model Training**: Cloud-based training with edge optimization
3. **Model Testing**: Validation on representative test sets
4. **Deployment**: Automatic deployment to ESP32 devices
5. **Monitoring**: Performance tracking and model improvement

### Continuous Learning
1. **Data Feedback**: Collect prediction confidence scores
2. **Model Retraining**: Periodic retraining with new data
3. **A/B Testing**: Compare model versions in production
4. **Federated Learning**: Distributed learning across camera network

## Troubleshooting

### Common Issues
1. **Out of Memory**: Reduce model complexity or arena size
2. **Slow Inference**: Check PSRAM availability and CPU frequency
3. **Poor Accuracy**: Verify model integrity and input preprocessing
4. **Load Failures**: Check file paths and permissions

### Debug Tools
1. **Model Validator**: Verify model format and compatibility
2. **Memory Profiler**: Monitor memory usage during inference
3. **Performance Benchmarks**: Measure inference speed and accuracy
4. **Diagnostic Logs**: Detailed logging for troubleshooting

## Future Enhancements

### Model Improvements
1. **Multi-Scale Detection**: Handle various animal sizes
2. **Temporal Modeling**: Improved behavior analysis with LSTM
3. **Few-Shot Learning**: Adapt to new species with minimal data
4. **Uncertainty Estimation**: Better confidence calibration

### Platform Integration
1. **Cloud Sync**: Seamless model synchronization
2. **Mobile Apps**: Remote monitoring and configuration
3. **Research APIs**: Integration with wildlife research platforms
4. **Conservation Alerts**: Real-time threat and poaching detection