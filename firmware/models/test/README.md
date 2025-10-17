# Wildlife Classification Test Model

## Overview
This is a minimal test model for validating ML integration in the ESP32 Wildlife Camera system.

**⚠️ WARNING**: This model is trained on synthetic data and is for testing purposes only.
Do NOT use in production. Train a proper model with real wildlife datasets.

## Model Specifications
- **Architecture**: Lightweight CNN
- **Input**: 224x224x3 RGB images (uint8)
- **Output**: 10 species classes (uint8)
- **Quantization**: INT8
- **Model Size**: ~1 KB

## Supported Species (Test)
- white_tailed_deer
- black_bear
- red_fox
- gray_wolf
- mountain_lion
- raccoon
- coyote
- elk
- wild_turkey
- unknown

## Usage
1. Deploy the `.tflite` model to your ESP32 filesystem
2. Load it using the InferenceEngine
3. Run inference on camera frames
4. Validate the integration pipeline

## Next Steps
For production deployment:
1. Collect real wildlife datasets
2. Train model using `model_trainer.py`
3. Validate on test data
4. Deploy production model

## Files
- `wildlife_classifier_test_quantized.tflite` - TFLite model
- `model_metadata.json` - Model specifications
- `class_labels.json` - Species labels
- `README.md` - This file

Generated: 2025-10-16 23:52:11
