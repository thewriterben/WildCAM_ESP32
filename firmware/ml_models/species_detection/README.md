# Species Detection Models

This directory contains TensorFlow Lite models for species-specific wildlife detection.

## Available Models

- `deer_classifier.tflite` - Deer species identification
- `bird_classifier.tflite` - Common bird species  
- `mammal_classifier.tflite` - Small to medium mammals
- `predator_classifier.tflite` - Large carnivore detection

## Model Performance Metrics

Each model is optimized for:
- Real-time inference on ESP32-S3 (8MB PSRAM)
- Battery-powered operation
- Low memory footprint (<512KB)
- High accuracy (>95% on test dataset)