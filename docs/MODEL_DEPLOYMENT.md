# TensorFlow Lite Model Deployment Guide

This guide explains how to deploy, update, and troubleshoot TensorFlow Lite models for AI-based wildlife classification on the WildCAM ESP32 system.

## Table of Contents

- [Overview](#overview)
- [Model Requirements](#model-requirements)
- [Deployment Instructions](#deployment-instructions)
- [Model Specifications](#model-specifications)
- [Updating Models](#updating-models)
- [Troubleshooting](#troubleshooting)
- [Performance Optimization](#performance-optimization)

## Overview

WildCAM ESP32 supports AI-based wildlife species classification using TensorFlow Lite INT8 quantized models. The system automatically loads models from the SD card at startup and performs real-time inference on captured images.

### Current Model

- **Filename**: `best_int8.tflite`
- **Type**: INT8 quantized TensorFlow Lite model
- **Purpose**: Wildlife species classification
- **Input Size**: 224x224 RGB images
- **Output**: Species classification with confidence scores

## Model Requirements

### Hardware Requirements

- **ESP32-CAM** with minimum 4MB PSRAM
- **SD Card** (Class 10 recommended, 16GB+)
- **Available Heap**: At least 4MB for tensor arena

### Model Specifications

Your TensorFlow Lite model must meet the following requirements:

1. **Quantization**: INT8 quantized for optimal ESP32 performance
2. **Input Shape**: `[1, 224, 224, 3]` (batch, height, width, channels)
3. **Input Type**: `int8` or `uint8`
4. **Output Type**: `int8` or `float32`
5. **File Size**: < 5MB recommended for ESP32 compatibility
6. **Format**: `.tflite` (TensorFlow Lite FlatBuffer)

### Supported Model Types

- **Image Classification**: Species identification (current)
- **Object Detection**: Animal detection with bounding boxes (future)
- **Behavioral Analysis**: Activity recognition (future)

## Deployment Instructions

### Step 1: Prepare the SD Card

1. **Format SD Card**:
   ```
   Format: FAT32
   Allocation unit size: 4096 bytes (default)
   ```

2. **Create Directory Structure**:
   ```
   /
   ├── models/
   │   └── best_int8.tflite
   ├── WILDLIFE/
   │   ├── IMAGES/
   │   └── METADATA/
   ```

### Step 2: Copy Model to SD Card

1. **Locate the Model File**:
   - Find `best_int8.tflite` in the `models/` directory of this repository
   - Model size: ~3.1 MB

2. **Copy to SD Card**:
   ```bash
   # On Linux/Mac
   cp models/best_int8.tflite /media/sdcard/models/
   
   # On Windows
   copy models\best_int8.tflite E:\models\
   ```

3. **Verify File Integrity**:
   - Check file size matches (3,168,457 bytes)
   - Ensure no corruption during transfer
   - Test read access on SD card

### Step 3: Configure System

Edit `include/config.h` to enable AI classification:

```cpp
// Enable AI-based species classification
#define AI_CLASSIFICATION_ENABLED true

// Specify model filename
#define TFLITE_MODEL_FILENAME "best_int8.tflite"

// Set confidence threshold (0.0 to 1.0)
#define SPECIES_CONFIDENCE_THRESHOLD 0.6f

// Adjust arena size if needed (default: 4MB)
#define TFLITE_ARENA_SIZE (4 * 1024 * 1024)
```

### Step 4: Upload Firmware

1. Build and upload firmware to ESP32-CAM
2. Monitor serial output for model loading status:
   ```
   [INIT] Initializing wildlife classifier...
   Loading wildlife classification model...
   Model loaded from: /models/best_int8.tflite
   Wildlife classifier initialized successfully
   Arena size: 4194304 bytes
   Confidence threshold: 0.60
   ```

### Step 5: Verify Operation

1. **Trigger a Capture**: Use motion sensor or manual capture
2. **Check Serial Output**: Look for classification results
   ```
   Species classified: White-tailed Deer (confidence: 0.85)
   Inference time: 1234 ms
   ```
3. **Review Web Interface**: Check dashboard for AI classification data

## Model Specifications

### Input Preprocessing

The system automatically preprocesses images:

1. **JPEG Decode**: Decode captured JPEG to RGB
2. **Resize**: Scale to 224x224 using nearest-neighbor interpolation
3. **Normalize**: Convert pixel values to INT8 range
4. **Layout**: NHWC format (batch, height, width, channels)

### Output Postprocessing

Model outputs are processed as follows:

1. **Dequantization**: Convert INT8 to confidence scores (0.0-1.0)
2. **Softmax**: Apply softmax to get class probabilities (if not included in model)
3. **Threshold Filtering**: Filter results below confidence threshold
4. **Species Mapping**: Map class indices to species names

### Supported Species Classes

The model supports up to 21 species classes:

| Class ID | Species Name | Notes |
|----------|--------------|-------|
| 0 | Unknown | Fallback for unrecognized animals |
| 1 | White-tailed Deer | Most common |
| 2 | Black Bear | Potentially dangerous |
| 3 | Red Fox | Common predator |
| 4 | Gray Wolf | Endangered in some regions |
| 5 | Mountain Lion | Rare, dangerous |
| 6-20 | Various Species | See `wildlife_classifier.h` for full list |

## Updating Models

### Replacing Existing Model

1. **Power Off ESP32-CAM**
2. **Remove SD Card**
3. **Delete Old Model**:
   ```bash
   rm /media/sdcard/models/best_int8.tflite
   ```
4. **Copy New Model**:
   ```bash
   cp new_model.tflite /media/sdcard/models/best_int8.tflite
   ```
5. **Reinsert SD Card and Power On**

### Training Your Own Model

To train a custom model for your wildlife monitoring needs:

1. **Collect Training Data**:
   - Use WildCAM to capture wildlife images
   - Export images from SD card `/WILDLIFE/IMAGES/`
   - Minimum 1000 images per species recommended

2. **Label Dataset**:
   - Use labelImg, CVAT, or similar tools
   - Create balanced dataset across species
   - Split: 70% training, 20% validation, 10% test

3. **Train Model**:
   ```bash
   # Example using TensorFlow
   python train_model.py --dataset wildlife_data/ \
                         --model mobilenet_v2 \
                         --epochs 100 \
                         --input_size 224
   ```

4. **Convert to TensorFlow Lite**:
   ```python
   import tensorflow as tf
   
   # Load trained model
   model = tf.keras.models.load_model('wildlife_model.h5')
   
   # Convert to TensorFlow Lite with INT8 quantization
   converter = tf.lite.TFLiteConverter.from_keras_model(model)
   converter.optimizations = [tf.lite.Optimize.DEFAULT]
   converter.target_spec.supported_types = [tf.int8]
   
   # Representative dataset for quantization
   def representative_dataset():
       for image in representative_images:
           yield [image]
   
   converter.representative_dataset = representative_dataset
   tflite_model = converter.convert()
   
   # Save model
   with open('custom_wildlife_model.tflite', 'wb') as f:
       f.write(tflite_model)
   ```

5. **Validate Model**:
   ```bash
   # Check model size
   ls -lh custom_wildlife_model.tflite
   
   # Verify model structure
   python -m tensorflow.lite.tools.visualize custom_wildlife_model.tflite
   ```

6. **Deploy to WildCAM** (follow deployment instructions above)

## Troubleshooting

### Model Loading Issues

#### Error: "Failed to initialize TensorFlow Lite"

**Possible Causes**:
- Insufficient heap memory
- PSRAM not enabled

**Solutions**:
1. Check PSRAM is enabled in `platformio.ini`:
   ```ini
   board_build.arduino.memory_type = qio_qspi
   ```
2. Reduce arena size in `config.h`:
   ```cpp
   #define TFLITE_ARENA_SIZE (2 * 1024 * 1024)  // Reduce to 2MB
   ```

#### Error: "No model file found"

**Possible Causes**:
- Model file not on SD card
- Incorrect directory path
- SD card not mounted

**Solutions**:
1. Verify SD card directory structure:
   ```
   /models/best_int8.tflite
   ```
2. Check SD card mount status via serial monitor
3. Try alternative paths (model loader checks multiple locations)

#### Error: "Failed to allocate model buffer"

**Possible Causes**:
- Model file too large
- Memory fragmentation

**Solutions**:
1. Use smaller model (< 3MB recommended)
2. Reboot ESP32-CAM to clear fragmentation
3. Reduce image quality to free memory

### Inference Issues

#### Slow Inference (> 5 seconds)

**Possible Causes**:
- Model too complex
- Insufficient CPU speed

**Solutions**:
1. Use INT8 quantized model (not float32)
2. Reduce model complexity (fewer layers)
3. Enable CPU frequency boost:
   ```cpp
   setCpuFrequencyMhz(240);  // Max 240 MHz
   ```

#### Low Accuracy (< 60%)

**Possible Causes**:
- Model not suited for your wildlife
- Poor lighting conditions
- Camera quality issues

**Solutions**:
1. Retrain model with local wildlife dataset
2. Adjust confidence threshold:
   ```cpp
   #define SPECIES_CONFIDENCE_THRESHOLD 0.4f  // Lower threshold
   ```
3. Improve lighting (add flash/IR illuminator)

#### High False Positive Rate

**Possible Causes**:
- Threshold too low
- Model overfitting

**Solutions**:
1. Increase confidence threshold:
   ```cpp
   #define SPECIES_CONFIDENCE_THRESHOLD 0.75f  // Higher threshold
   ```
2. Enable two-factor detection (PIR + AI confirmation)
3. Retrain with more diverse dataset

### Memory Issues

#### Error: "Out of memory during inference"

**Solutions**:
1. Increase tensor arena size:
   ```cpp
   #define TFLITE_ARENA_SIZE (6 * 1024 * 1024)  // Increase to 6MB
   ```
2. Reduce image capture quality:
   ```cpp
   #define DEFAULT_FRAME_SIZE FRAMESIZE_SVGA  // Use smaller resolution
   ```
3. Disable other features temporarily to free memory

## Performance Optimization

### Memory Optimization

- Use **INT8 quantization** (8x smaller than float32)
- Enable **PSRAM** for tensor arena allocation
- Reduce **image resolution** if possible (SVGA vs UXGA)
- Minimize **model size** (< 3MB ideal)

### Inference Speed Optimization

- Use **MobileNet** or **EfficientNet-Lite** architectures
- Reduce **input resolution** to 160x160 or 128x128
- Enable **CPU overclocking** to 240 MHz
- Consider **hardware acceleration** (ESP32-S3 with AI extensions)

### Power Optimization

- Run inference **only on motion events**
- Use **deep sleep** between captures
- Reduce **inference frequency** (every 2nd or 3rd capture)
- Lower **confidence threshold** to reduce repeated captures

### Accuracy Optimization

- Use **domain-specific training data** (local wildlife)
- Implement **ensemble models** (multiple models voting)
- Add **temporal filtering** (confirm species across multiple frames)
- Use **metadata** (time of day, location) to improve predictions

## Additional Resources

### Documentation

- [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
- [Model Optimization Guide](https://www.tensorflow.org/lite/performance/model_optimization)
- [ESP32 Memory Management](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/memory-types.html)

### Tools

- **TensorFlow Model Converter**: Convert Keras to TFLite
- **Netron**: Visualize model architecture
- **TFLite Benchmark Tool**: Test model performance

### Example Models

- **MobileNet v2 INT8**: Efficient classification model
- **EfficientNet-Lite**: State-of-the-art accuracy/speed trade-off
- **SqueezeNet INT8**: Ultra-compact model for ESP32

## Support

For model deployment issues:

1. Check [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
2. Review [Troubleshooting](#troubleshooting) section above
3. Post in [Discussions](https://github.com/thewriterben/WildCAM_ESP32/discussions)
4. Include serial monitor output and model specifications

---

**Last Updated**: December 2024  
**Model Version**: best_int8.tflite (v1.0)  
**Compatible Firmware**: v3.1.0+
