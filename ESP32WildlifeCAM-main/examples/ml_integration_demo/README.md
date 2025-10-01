# ML Integration Demo

## Overview

This example demonstrates the machine learning integration workflow for the ESP32 Wildlife Camera. It shows how the system captures images, runs AI inference, and processes classification results.

## What This Demo Shows

1. **Workflow Visualization** - Complete ML pipeline from capture to classification
2. **Architecture Overview** - System component diagram
3. **API Examples** - Code snippets showing how to use the ML integration
4. **Camera Integration** - Real camera capture demonstration

## Hardware Required

- ESP32-CAM (AI-Thinker) or ESP32-S3-CAM
- Camera module (OV2640 or compatible)
- USB-to-Serial adapter (for AI-Thinker)

## Installation

### Using PlatformIO

```bash
# Clone repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/ESP32WildlifeCAM-main/examples/ml_integration_demo

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor
```

### Using Arduino IDE

1. Copy this folder to your Arduino sketches directory
2. Open `ml_integration_demo.ino` in Arduino IDE
3. Select your ESP32 board (Tools → Board → ESP32 Arduino)
4. Select the correct port (Tools → Port)
5. Upload the sketch

## Usage

1. **Upload the sketch** to your ESP32-CAM
2. **Open Serial Monitor** at 115200 baud
3. **Send commands** to see different demonstrations:
   - `w` - Show complete ML workflow
   - `a` - Show system architecture
   - `x` - Show API code examples
   - `h` - Show help

## What You'll See

### Workflow Demo (Press 'w')

```
=== ML Integration Workflow Demo ===

1. Camera Capture
   ↓
   ✓ Image captured: 640x480 (45678 bytes)

2. Preprocessing
   - Resize to 224x224
   - Normalize pixel values
   - Convert to RGB format
   ↓

3. AI Inference (Simulated)
   - Load TFLite model
   - Run inference on preprocessed image
   - Get classification probabilities
   ✓ Inference completed in ~850ms
   ↓

4. Classification Results
   Species: White-tailed Deer
   Confidence: 92.3%
   Dangerous: No
   Estimated size: 68.5 kg
   ↓

5. Post-Processing
   - Log detection
   - Update statistics
   - Save metadata
   - Trigger alerts if needed

✓ Complete workflow demonstrated
```

### Architecture Demo (Press 'a')

Shows the complete system architecture diagram with all components.

### API Examples (Press 'x')

Displays code snippets showing how to:
- Initialize the classifier
- Capture and classify images
- Process results
- Handle dangerous species

## Notes

⚠️ **This is a demonstration sketch** that shows the ML integration workflow. It does not include the actual AI inference (which requires TensorFlow Lite models and additional libraries).

For **full ML functionality**, you need:

1. **Train a model** using the training pipeline in `backend/ml/`
2. **Deploy the model** to `firmware/models/`
3. **Use the complete firmware** in `src/main.cpp` which includes all ML libraries

## Full ML Integration Steps

### 1. Generate Test Model

```bash
cd backend/ml
python generate_test_model.py
```

### 2. Deploy Model

```bash
cp firmware/models/test/wildlife_classifier_test_quantized.tflite \
   ESP32WildlifeCAM-main/models/deployment/
```

### 3. Build Full Firmware

```bash
cd ESP32WildlifeCAM-main
pio run --target upload
```

## Documentation

For complete ML integration documentation, see:

- **[ML Integration Guide](../../docs/ml_integration_guide.md)** - Complete training and deployment guide
- **[Quick Start](../../docs/ml_quickstart.md)** - 5-minute setup guide
- **[ML Workflow](../../docs/ML_WORKFLOW.md)** - Detailed workflow documentation
- **[ML Integration README](../../ML_INTEGRATION_README.md)** - Implementation overview

## API Reference

### WildlifeClassifier

```cpp
// Initialize
WildlifeClassifier classifier;
WildlifeClassifierConfig config;
config.confidenceThreshold = 0.7;
config.enableEnvironmentalAdaptation = true;
classifier.init(config);

// Classify frame
camera_fb_t* fb = esp_camera_fb_get();
ClassificationResult result = classifier.classifyFrame(fb);

// Process results
if (result.isValid) {
    Serial.printf("Species: %s\n", result.speciesName.c_str());
    Serial.printf("Confidence: %.1f%%\n", result.confidence * 100);
    
    if (WildlifeClassifier::isDangerousSpecies(result.species)) {
        triggerAlert(result);
    }
}

esp_camera_fb_return(fb);
```

### InferenceEngine

```cpp
// Initialize engine
InferenceEngine engine;
engine.init();

// Load model
engine.loadModel("/models/wildlife_classifier_v2.tflite",
                 ModelType::SPECIES_CLASSIFIER);

// Run inference
CameraFrame frame = {fb->buf, fb->width, fb->height, fb->format};
AIResult result = engine.runInference(frame, 
                                     ModelType::SPECIES_CLASSIFIER);

// Get performance metrics
AIMetrics metrics = engine.getPerformanceMetrics();
Serial.printf("Inference time: %d ms\n", metrics.avgInferenceTime);
```

## Troubleshooting

### Camera Fails to Initialize

- Check camera connections
- Verify pin configuration matches your board
- Ensure sufficient power supply (5V 2A recommended)

### Serial Monitor Shows Garbled Text

- Set baud rate to 115200
- Try pressing the reset button on ESP32

### Want to Try Real ML?

Follow the complete guide at: [docs/ml_integration_guide.md](../../docs/ml_integration_guide.md)

## Support

- **Issues**: https://github.com/thewriterben/WildCAM_ESP32/issues
- **Docs**: https://github.com/thewriterben/WildCAM_ESP32/docs

## License

MIT License - See LICENSE file for details
