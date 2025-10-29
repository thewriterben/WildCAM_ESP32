#!/usr/bin/env python3
"""
Generate Production Wildlife Classifier Model

Creates the main wildlife_classifier_v2.tflite model referenced in the model manifest.
This is a comprehensive model covering all major wildlife species.
"""

import os
import json
import logging
from pathlib import Path
from datetime import datetime
import tensorflow as tf
import numpy as np

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class ProductionModelGenerator:
    """Generate production wildlife classification model"""
    
    def __init__(self, output_dir: str):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Comprehensive species list for production model
        self.species = [
            # Deer and Cervids
            'white_tailed_deer', 'mule_deer', 'elk', 'moose', 'caribou',
            
            # Large Predators
            'black_bear', 'grizzly_bear', 'mountain_lion', 'gray_wolf', 'coyote',
            
            # Medium Predators
            'bobcat', 'lynx', 'red_fox', 'gray_fox',
            
            # Medium Mammals
            'raccoon', 'opossum', 'skunk', 'porcupine', 'beaver',
            
            # Small Mammals
            'rabbit', 'squirrel', 'chipmunk', 'groundhog',
            
            # Birds
            'wild_turkey', 'grouse', 'pheasant', 'crow', 'raven', 'eagle',
            'hawk', 'owl',
            
            # Other
            'domestic_dog', 'domestic_cat', 'livestock', 'human', 'unknown'
        ]
        self.num_classes = len(self.species)
    
    def create_production_model(self, input_size=(224, 224, 3)) -> tf.keras.Model:
        """Create a production-ready CNN model"""
        
        model = tf.keras.Sequential([
            tf.keras.layers.Input(shape=input_size),
            
            # Block 1
            tf.keras.layers.Conv2D(32, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.BatchNormalization(),
            tf.keras.layers.Conv2D(32, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.MaxPooling2D((2, 2)),
            tf.keras.layers.Dropout(0.2),
            
            # Block 2
            tf.keras.layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.BatchNormalization(),
            tf.keras.layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.MaxPooling2D((2, 2)),
            tf.keras.layers.Dropout(0.2),
            
            # Block 3
            tf.keras.layers.Conv2D(128, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.BatchNormalization(),
            tf.keras.layers.GlobalAveragePooling2D(),
            
            # Classification head
            tf.keras.layers.Dense(128, activation='relu'),
            tf.keras.layers.Dropout(0.3),
            tf.keras.layers.Dense(64, activation='relu'),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(self.num_classes, activation='softmax', name='species_output')
        ])
        
        model.compile(
            optimizer=tf.keras.optimizers.Adam(learning_rate=0.001),
            loss='categorical_crossentropy',
            metrics=['accuracy', 'top_k_categorical_accuracy']
        )
        
        return model
    
    def train_model(self, model: tf.keras.Model, epochs=10) -> dict:
        """Train model on synthetic data"""
        
        logger.info(f"Generating synthetic training data for {self.num_classes} classes...")
        
        # Generate synthetic training data
        num_samples = 500
        X_train = np.random.rand(num_samples, 224, 224, 3).astype(np.float32)
        y_train = tf.keras.utils.to_categorical(
            np.random.randint(0, self.num_classes, num_samples),
            num_classes=self.num_classes
        )
        
        # Generate validation data
        X_val = np.random.rand(100, 224, 224, 3).astype(np.float32)
        y_val = tf.keras.utils.to_categorical(
            np.random.randint(0, self.num_classes, 100),
            num_classes=self.num_classes
        )
        
        logger.info(f"Training production model for {epochs} epochs...")
        
        # Add callbacks
        callbacks = [
            tf.keras.callbacks.EarlyStopping(
                monitor='val_loss',
                patience=3,
                restore_best_weights=True
            ),
            tf.keras.callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=2,
                min_lr=0.0001
            )
        ]
        
        history = model.fit(
            X_train, y_train,
            validation_data=(X_val, y_val),
            epochs=epochs,
            batch_size=32,
            callbacks=callbacks,
            verbose=1
        )
        
        training_stats = {
            'final_loss': float(history.history['loss'][-1]),
            'final_accuracy': float(history.history['accuracy'][-1]),
            'final_val_loss': float(history.history['val_loss'][-1]),
            'final_val_accuracy': float(history.history['val_accuracy'][-1]),
            'epochs_trained': len(history.history['loss'])
        }
        
        return training_stats
    
    def convert_to_tflite(self, model: tf.keras.Model, quantize=True) -> tuple:
        """Convert model to TensorFlow Lite format"""
        
        logger.info("Converting model to TensorFlow Lite...")
        
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        
        if quantize:
            logger.info("Applying INT8 quantization...")
            
            # Apply quantization
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            
            # Representative dataset for quantization
            def representative_data_gen():
                for _ in range(100):
                    yield [np.random.rand(1, 224, 224, 3).astype(np.float32)]
            
            converter.representative_dataset = representative_data_gen
            converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
            converter.inference_input_type = tf.uint8
            converter.inference_output_type = tf.uint8
        
        tflite_model = converter.convert()
        
        # Save TFLite model
        filename = "wildlife_classifier_v2_quantized.tflite"
        tflite_path = self.output_dir / filename
        
        with open(tflite_path, 'wb') as f:
            f.write(tflite_model)
        
        model_size = len(tflite_model)
        logger.info(f"TFLite model saved: {tflite_path} ({model_size / 1024:.1f} KB)")
        
        return str(tflite_path), model_size
    
    def create_model_metadata(self, tflite_path: str, model_size: int, training_stats: dict):
        """Create metadata for the production model"""
        
        metadata = {
            "metadata_version": "2.0.0",
            "created_date": datetime.now().isoformat(),
            "model_info": {
                "name": "wildlife_classifier_v2.tflite",
                "version": "2.0.0",
                "type": "species_classification",
                "description": "Production wildlife species classifier with comprehensive species coverage",
                "file_size_bytes": model_size,
                "quantization": "INT8"
            },
            "input_specification": {
                "shape": [1, 224, 224, 3],
                "dtype": "uint8",
                "format": "RGB",
                "normalization": "0-255"
            },
            "output_specification": {
                "shape": [1, self.num_classes],
                "dtype": "uint8",
                "classes": self.species,
                "format": "quantized_softmax"
            },
            "target_hardware": {
                "platform": "ESP32-S3",
                "min_psram_mb": 8,
                "min_flash_mb": 4,
                "min_cpu_freq_mhz": 160
            },
            "performance": {
                "expected_inference_time_ms": 850,
                "memory_usage_mb": 1.5,
                "power_consumption_ma": 165,
                "target_accuracy": 0.95,
                "target_top5_accuracy": 0.99,
                "model_is_demo": True
            },
            "training_info": {
                "framework": "TensorFlow Lite",
                "training_type": "synthetic_data",
                "note": "Demo model for integration. Train on real wildlife datasets for production deployment.",
                **training_stats
            },
            "supported_species": self.species,
            "species_categories": {
                "cervids": ["white_tailed_deer", "mule_deer", "elk", "moose", "caribou"],
                "large_predators": ["black_bear", "grizzly_bear", "mountain_lion", "gray_wolf", "coyote"],
                "medium_predators": ["bobcat", "lynx", "red_fox", "gray_fox"],
                "medium_mammals": ["raccoon", "opossum", "skunk", "porcupine", "beaver"],
                "small_mammals": ["rabbit", "squirrel", "chipmunk", "groundhog"],
                "birds": ["wild_turkey", "grouse", "pheasant", "crow", "raven", "eagle", "hawk", "owl"],
                "other": ["domestic_dog", "domestic_cat", "livestock", "human", "unknown"]
            }
        }
        
        metadata_path = self.output_dir / "wildlife_classifier_v2_metadata.json"
        with open(metadata_path, 'w') as f:
            json.dump(metadata, f, indent=2)
        
        logger.info(f"Metadata saved: {metadata_path}")
        
        return metadata
    
    def create_class_labels(self):
        """Create class labels file"""
        
        labels_data = {
            "model_name": "wildlife_classifier_v2",
            "labels": self.species,
            "num_classes": self.num_classes,
            "label_to_index": {label: idx for idx, label in enumerate(self.species)},
            "index_to_label": {str(idx): label for idx, label in enumerate(self.species)}
        }
        
        labels_path = self.output_dir / "wildlife_classifier_v2_labels.json"
        with open(labels_path, 'w') as f:
            json.dump(labels_data, f, indent=2)
        
        logger.info(f"Class labels saved: {labels_path}")
    
    def create_readme(self):
        """Create README for production model"""
        
        readme_content = f"""# Wildlife Classifier v2.0 - Production Model

## Overview
Comprehensive TensorFlow Lite model for wildlife species classification with {self.num_classes} species coverage.

**⚠️ DEMO MODEL**: This model is trained on synthetic data for demonstration and integration testing.
For production deployment, train on real wildlife datasets using the comprehensive training pipeline.

## Model Specifications
- **Model Name**: wildlife_classifier_v2
- **Version**: 2.0.0
- **Architecture**: Deep CNN optimized for ESP32-S3
- **Input**: 224x224x3 RGB images (uint8, 0-255)
- **Output**: {self.num_classes} species classes (uint8 quantized)
- **Quantization**: Full INT8
- **Target Platform**: ESP32-S3 with 8MB PSRAM

## Supported Species ({self.num_classes} total)

### Deer and Cervids
- White-tailed Deer
- Mule Deer
- Elk
- Moose
- Caribou

### Large Predators
- Black Bear
- Grizzly Bear
- Mountain Lion
- Gray Wolf
- Coyote

### Medium Predators
- Bobcat
- Lynx
- Red Fox
- Gray Fox

### Medium Mammals
- Raccoon
- Opossum
- Skunk
- Porcupine
- Beaver

### Small Mammals
- Rabbit
- Squirrel
- Chipmunk
- Groundhog

### Birds
- Wild Turkey
- Grouse
- Pheasant
- Crow
- Raven
- Eagle
- Hawk
- Owl

### Other
- Domestic Dog
- Domestic Cat
- Livestock
- Human
- Unknown

## Performance Targets

| Metric | Target | Notes |
|--------|--------|-------|
| Inference Time | < 850ms | ESP32-S3 @ 240MHz |
| Memory Usage | 1.5 MB | Including model and buffers |
| Power Consumption | < 165mA | During inference |
| Accuracy | > 95% | On real wildlife data |
| Top-5 Accuracy | > 99% | On real wildlife data |

## Usage

### Basic Integration
```cpp
#include "firmware/src/ai/wildlife_classifier.h"

WildlifeClassifier classifier;

void setup() {{
    WildlifeClassifier::ClassifierConfig config;
    config.confidenceThreshold = 0.7;
    config.enableEnvironmentalAdaptation = true;
    
    if (classifier.initialize(config)) {{
        Serial.println("Wildlife classifier initialized");
    }}
}}

void loop() {{
    // Capture frame
    camera_fb_t* fb = esp_camera_fb_get();
    
    // Classify
    auto result = classifier.classifyFrame(fb);
    
    if (result.isValid) {{
        Serial.printf("Species: %s\\n", result.speciesName.c_str());
        Serial.printf("Confidence: %.2f%%\\n", result.confidence * 100);
        
        if (result.isDangerous) {{
            Serial.println("⚠️  Dangerous species detected!");
        }}
    }}
    
    esp_camera_fb_return(fb);
    delay(1000);
}}
```

### Advanced Usage with Behavior Analysis
```cpp
#include "firmware/src/ai/vision/species_classifier.h"

SpeciesClassifier classifier;

void detectAndAnalyze() {{
    // Get camera frame
    CameraFrame frame = camera.capture();
    
    // Classify species
    SpeciesResult result = classifier.classifyWithBehavior(frame);
    
    // Process results
    Serial.printf("Species: %s\\n", result.specificName.c_str());
    Serial.printf("Confidence: %.2f\\n", result.confidence);
    Serial.printf("Behavior: %s\\n", result.behaviorState.c_str());
    Serial.printf("Size estimate: %s\\n", result.sizeCategory.c_str());
}}
```

## Training for Production

To replace this demo model with a production model:

### 1. Dataset Collection
```bash
# Collect diverse wildlife images
# - 500-1000 images per species
# - Multiple lighting conditions
# - Various angles and distances
# - Different seasons/weather
```

### 2. Model Training
```bash
cd /home/runner/work/WildCAM_ESP32/WildCAM_ESP32
python3 backend/ml/model_trainer.py \\
    --dataset /path/to/wildlife_dataset \\
    --output firmware/models/production \\
    --model-name wildlife_classifier_v2 \\
    --epochs 100 \\
    --batch-size 32 \\
    --quantize
```

### 3. Model Evaluation
```bash
python3 backend/ml/model_evaluator.py \\
    --model firmware/models/production/wildlife_classifier_v2.tflite \\
    --test-data /path/to/test_set \\
    --threshold 0.95
```

### 4. Deployment
```bash
# Copy to production directory
cp firmware/models/production/wildlife_classifier_v2_quantized.tflite \\
   firmware/models/wildlife_classifier_v2.tflite

# Flash to ESP32
pio run -t upload -e esp32s3cam_ai
```

## Files in This Directory
- `wildlife_classifier_v2_quantized.tflite` - INT8 quantized TFLite model
- `wildlife_classifier_v2.h5` - Keras model (for reference)
- `wildlife_classifier_v2_metadata.json` - Model specifications
- `wildlife_classifier_v2_labels.json` - Species labels
- `README.md` - This documentation

## Integration with Existing Systems

This model is designed to integrate seamlessly with:
- **Detection Pipeline**: Automatic classification after motion detection
- **Mesh Networking**: Share detection results across camera network
- **Alert System**: Trigger alerts for dangerous species
- **Data Collection**: Store classifications with metadata
- **Cloud Sync**: Upload results to backend server

## Performance Optimization Tips

1. **Enable PSRAM**: Essential for 8MB models
2. **CPU Frequency**: Run at 240MHz for best performance
3. **Pre-processing**: Resize images before inference
4. **Confidence Threshold**: Tune based on field testing
5. **Power Management**: Use motion triggers to reduce power

## Model Limitations

⚠️ **Demo Model Limitations**:
- Trained on synthetic data only
- Low accuracy on real wildlife images
- Not suitable for production use
- Requires real dataset training for deployment

## Next Steps

1. Collect real wildlife dataset
2. Train production model (see ml/TRAINING_DEPLOYMENT_GUIDE.md)
3. Validate accuracy > 95%
4. Deploy to field units
5. Monitor and iterate

## Support

- **Training Guide**: [ml/TRAINING_DEPLOYMENT_GUIDE.md](../../ml/TRAINING_DEPLOYMENT_GUIDE.md)
- **Quick Start**: [ml/QUICKSTART.md](../../ml/QUICKSTART.md)
- **ML Integration**: [ML_INTEGRATION_README.md](../../ML_INTEGRATION_README.md)
- **Issues**: GitHub Issues with `machine-learning` label

Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
Version: 2.0.0 (Demo)
"""
        
        readme_path = self.output_dir / "README.md"
        with open(readme_path, 'w') as f:
            f.write(readme_content)
        
        logger.info(f"README saved: {readme_path}")
    
    def generate_production_model(self) -> dict:
        """Generate complete production model with metadata"""
        
        logger.info("="*60)
        logger.info("PRODUCTION WILDLIFE CLASSIFIER GENERATION")
        logger.info("="*60)
        
        # Create model
        model = self.create_production_model()
        logger.info(f"Model created with {model.count_params()} parameters")
        
        # Train on synthetic data
        training_stats = self.train_model(model, epochs=10)
        
        # Save Keras model
        keras_path = self.output_dir / "wildlife_classifier_v2.h5"
        model.save(keras_path)
        logger.info(f"Keras model saved: {keras_path}")
        
        # Convert to TFLite
        tflite_path, model_size = self.convert_to_tflite(model, quantize=True)
        
        # Create metadata
        metadata = self.create_model_metadata(tflite_path, model_size, training_stats)
        
        # Create class labels
        self.create_class_labels()
        
        # Create README
        self.create_readme()
        
        result = {
            "success": True,
            "keras_model": str(keras_path),
            "tflite_model": tflite_path,
            "model_size_kb": model_size / 1024,
            "model_size_mb": model_size / (1024 * 1024),
            "num_species": self.num_classes,
            "metadata_file": str(self.output_dir / "wildlife_classifier_v2_metadata.json"),
            "training_stats": training_stats
        }
        
        logger.info("="*60)
        logger.info("PRODUCTION MODEL GENERATION COMPLETE")
        logger.info("="*60)
        logger.info(f"Model size: {result['model_size_mb']:.2f} MB ({result['model_size_kb']:.1f} KB)")
        logger.info(f"Species coverage: {result['num_species']} species")
        logger.info(f"Output directory: {self.output_dir}")
        
        return result


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Generate production wildlife classifier model")
    parser.add_argument(
        "--output-dir",
        type=str,
        default="/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware/models/production",
        help="Output directory for production model"
    )
    
    args = parser.parse_args()
    
    # Generate production model
    generator = ProductionModelGenerator(output_dir=args.output_dir)
    result = generator.generate_production_model()
    
    if result['success']:
        print("\n✅ Production model generation successful!")
        print(f"📁 Output directory: {args.output_dir}")
        print(f"📦 Model size: {result['model_size_mb']:.2f} MB")
        print(f"🎯 Species: {result['num_species']}")
        print(f"📄 Model file: {result['tflite_model']}")
    else:
        print("\n❌ Production model generation failed")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main())
