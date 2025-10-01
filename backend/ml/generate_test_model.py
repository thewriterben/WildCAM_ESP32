#!/usr/bin/env python3
"""
Generate Test Model for Wildlife Classification

Creates a minimal TensorFlow Lite model for testing the ML integration
without requiring a full training dataset.
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


class TestModelGenerator:
    """Generate lightweight test models for integration testing"""
    
    def __init__(self, output_dir: str):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Define test species (subset for testing)
        self.test_species = [
            'white_tailed_deer', 'black_bear', 'red_fox', 'gray_wolf',
            'mountain_lion', 'raccoon', 'coyote', 'elk', 
            'wild_turkey', 'unknown'
        ]
        self.num_classes = len(self.test_species)
    
    def create_minimal_model(self, input_size=(224, 224, 3)) -> tf.keras.Model:
        """Create a minimal CNN model for testing"""
        
        model = tf.keras.Sequential([
            tf.keras.layers.Input(shape=input_size),
            
            # Lightweight convolutional layers
            tf.keras.layers.Conv2D(16, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.MaxPooling2D((2, 2)),
            
            tf.keras.layers.Conv2D(32, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.MaxPooling2D((2, 2)),
            
            tf.keras.layers.Conv2D(64, (3, 3), activation='relu', padding='same'),
            tf.keras.layers.GlobalAveragePooling2D(),
            
            # Classification head
            tf.keras.layers.Dense(32, activation='relu'),
            tf.keras.layers.Dropout(0.2),
            tf.keras.layers.Dense(self.num_classes, activation='softmax', name='species_output')
        ])
        
        model.compile(
            optimizer='adam',
            loss='categorical_crossentropy',
            metrics=['accuracy']
        )
        
        return model
    
    def train_minimal_model(self, model: tf.keras.Model, epochs=5) -> dict:
        """Train model on synthetic data for testing"""
        
        logger.info("Generating synthetic training data...")
        
        # Generate synthetic training data
        num_samples = 100
        X_train = np.random.rand(num_samples, 224, 224, 3).astype(np.float32)
        y_train = tf.keras.utils.to_categorical(
            np.random.randint(0, self.num_classes, num_samples),
            num_classes=self.num_classes
        )
        
        # Generate validation data
        X_val = np.random.rand(20, 224, 224, 3).astype(np.float32)
        y_val = tf.keras.utils.to_categorical(
            np.random.randint(0, self.num_classes, 20),
            num_classes=self.num_classes
        )
        
        logger.info(f"Training minimal model for {epochs} epochs...")
        
        history = model.fit(
            X_train, y_train,
            validation_data=(X_val, y_val),
            epochs=epochs,
            batch_size=16,
            verbose=1
        )
        
        training_stats = {
            'final_loss': float(history.history['loss'][-1]),
            'final_accuracy': float(history.history['accuracy'][-1]),
            'final_val_loss': float(history.history['val_loss'][-1]),
            'final_val_accuracy': float(history.history['val_accuracy'][-1])
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
        filename = "wildlife_classifier_test_quantized.tflite" if quantize else "wildlife_classifier_test.tflite"
        tflite_path = self.output_dir / filename
        
        with open(tflite_path, 'wb') as f:
            f.write(tflite_model)
        
        model_size = len(tflite_model)
        logger.info(f"TFLite model saved: {tflite_path} ({model_size / 1024:.1f} KB)")
        
        return str(tflite_path), model_size
    
    def create_model_metadata(self, tflite_path: str, model_size: int, training_stats: dict):
        """Create metadata for the test model"""
        
        metadata = {
            "metadata_version": "2.0.0",
            "created_date": datetime.now().isoformat(),
            "model_info": {
                "name": "wildlife_classifier_test.tflite",
                "version": "2.0.0-test",
                "type": "species_classification",
                "description": "Minimal test model for ML integration testing",
                "file_size_bytes": model_size,
                "quantization": "INT8"
            },
            "input_specification": {
                "shape": [1, 224, 224, 3],
                "dtype": "uint8",
                "format": "RGB"
            },
            "output_specification": {
                "shape": [1, self.num_classes],
                "dtype": "uint8",
                "classes": self.test_species
            },
            "target_hardware": {
                "platform": "ESP32-S3",
                "min_psram_mb": 8,
                "min_flash_mb": 4
            },
            "performance": {
                "expected_inference_time_ms": 800,
                "memory_usage_kb": 512,
                "model_is_test": True
            },
            "training_info": {
                "framework": "TensorFlow Lite",
                "training_type": "synthetic_data",
                "note": "This is a test model trained on synthetic data for integration testing only",
                **training_stats
            },
            "supported_species": self.test_species
        }
        
        metadata_path = self.output_dir / "model_metadata.json"
        with open(metadata_path, 'w') as f:
            json.dump(metadata, f, indent=2)
        
        logger.info(f"Metadata saved: {metadata_path}")
        
        return metadata
    
    def create_class_labels(self):
        """Create class labels file"""
        
        labels_data = {
            "labels": self.test_species,
            "num_classes": self.num_classes,
            "label_to_index": {label: idx for idx, label in enumerate(self.test_species)},
            "index_to_label": {idx: label for idx, label in enumerate(self.test_species)}
        }
        
        labels_path = self.output_dir / "class_labels.json"
        with open(labels_path, 'w') as f:
            json.dump(labels_data, f, indent=2)
        
        logger.info(f"Class labels saved: {labels_path}")
    
    def generate_test_model(self) -> dict:
        """Generate complete test model with metadata"""
        
        logger.info("=== Starting Test Model Generation ===")
        
        # Create minimal model
        model = self.create_minimal_model()
        logger.info(f"Model created with {model.count_params()} parameters")
        
        # Train on synthetic data
        training_stats = self.train_minimal_model(model, epochs=3)
        
        # Save Keras model
        keras_path = self.output_dir / "wildlife_classifier_test.h5"
        model.save(keras_path)
        logger.info(f"Keras model saved: {keras_path}")
        
        # Convert to TFLite
        tflite_path, model_size = self.convert_to_tflite(model, quantize=True)
        
        # Create metadata
        metadata = self.create_model_metadata(tflite_path, model_size, training_stats)
        
        # Create class labels
        self.create_class_labels()
        
        # Create README
        self._create_readme()
        
        result = {
            "success": True,
            "keras_model": str(keras_path),
            "tflite_model": tflite_path,
            "model_size_kb": model_size / 1024,
            "metadata_file": str(self.output_dir / "model_metadata.json"),
            "training_stats": training_stats
        }
        
        logger.info("=== Test Model Generation Complete ===")
        logger.info(f"Model size: {result['model_size_kb']:.1f} KB")
        logger.info(f"Output directory: {self.output_dir}")
        
        return result
    
    def _create_readme(self):
        """Create README for test model"""
        
        readme_content = f"""# Wildlife Classification Test Model

## Overview
This is a minimal test model for validating ML integration in the ESP32 Wildlife Camera system.

**⚠️ WARNING**: This model is trained on synthetic data and is for testing purposes only.
Do NOT use in production. Train a proper model with real wildlife datasets.

## Model Specifications
- **Architecture**: Lightweight CNN
- **Input**: 224x224x3 RGB images (uint8)
- **Output**: {self.num_classes} species classes (uint8)
- **Quantization**: INT8
- **Model Size**: ~{len(list((self.output_dir).glob('*.tflite')))} KB

## Supported Species (Test)
{chr(10).join('- ' + species for species in self.test_species)}

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

Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
"""
        
        readme_path = self.output_dir / "README.md"
        with open(readme_path, 'w') as f:
            f.write(readme_content)
        
        logger.info(f"README saved: {readme_path}")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Generate test model for ML integration")
    parser.add_argument(
        "--output-dir",
        type=str,
        default="/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware/models/test",
        help="Output directory for test model"
    )
    
    args = parser.parse_args()
    
    # Generate test model
    generator = TestModelGenerator(output_dir=args.output_dir)
    result = generator.generate_test_model()
    
    if result['success']:
        print("\n✅ Test model generation successful!")
        print(f"📁 Output directory: {args.output_dir}")
        print(f"📦 Model size: {result['model_size_kb']:.1f} KB")
        print(f"🎯 Model file: {result['tflite_model']}")
    else:
        print("\n❌ Test model generation failed")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main())
