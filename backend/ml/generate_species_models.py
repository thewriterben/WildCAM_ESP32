#!/usr/bin/env python3
"""
Generate Species-Specific Classifier Models

Creates specialized TensorFlow Lite models for different wildlife categories:
- Deer classifier
- Bird classifier  
- Mammal classifier
- Predator classifier
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


class SpeciesModelGenerator:
    """Generate specialized wildlife classification models"""
    
    SPECIES_CATEGORIES = {
        'deer': {
            'species': ['white_tailed_deer', 'mule_deer', 'elk', 'moose', 'caribou', 'unknown'],
            'description': 'Specialized classifier for deer and cervid species',
            'model_size_target_mb': 0.8
        },
        'bird': {
            'species': ['wild_turkey', 'crow', 'eagle', 'hawk', 'owl', 'woodpecker', 'unknown'],
            'description': 'Classifier for common bird species',
            'model_size_target_mb': 0.6
        },
        'mammal': {
            'species': ['raccoon', 'squirrel', 'rabbit', 'opossum', 'skunk', 'beaver', 'unknown'],
            'description': 'Small to medium mammal classifier',
            'model_size_target_mb': 0.7
        },
        'predator': {
            'species': ['black_bear', 'mountain_lion', 'gray_wolf', 'coyote', 'bobcat', 'lynx', 'unknown'],
            'description': 'Large carnivore and predator detection',
            'model_size_target_mb': 0.9
        }
    }
    
    def __init__(self, output_base_dir: str):
        self.output_base_dir = Path(output_base_dir)
        self.output_base_dir.mkdir(parents=True, exist_ok=True)
    
    def create_specialized_model(self, category: str, species_list: list, 
                                 input_size=(224, 224, 3)) -> tf.keras.Model:
        """Create a specialized CNN model for a species category"""
        
        num_classes = len(species_list)
        
        # Adjust model complexity based on number of classes
        if num_classes <= 6:
            filters = [16, 32, 48]
            dense_units = 24
        else:
            filters = [16, 32, 64]
            dense_units = 32
        
        model = tf.keras.Sequential([
            tf.keras.layers.Input(shape=input_size),
            
            # First conv block
            tf.keras.layers.Conv2D(filters[0], (3, 3), activation='relu', padding='same'),
            tf.keras.layers.BatchNormalization(),
            tf.keras.layers.MaxPooling2D((2, 2)),
            
            # Second conv block
            tf.keras.layers.Conv2D(filters[1], (3, 3), activation='relu', padding='same'),
            tf.keras.layers.BatchNormalization(),
            tf.keras.layers.MaxPooling2D((2, 2)),
            
            # Third conv block
            tf.keras.layers.Conv2D(filters[2], (3, 3), activation='relu', padding='same'),
            tf.keras.layers.GlobalAveragePooling2D(),
            
            # Classification head
            tf.keras.layers.Dense(dense_units, activation='relu'),
            tf.keras.layers.Dropout(0.3),
            tf.keras.layers.Dense(num_classes, activation='softmax', name=f'{category}_output')
        ])
        
        model.compile(
            optimizer='adam',
            loss='categorical_crossentropy',
            metrics=['accuracy']
        )
        
        return model
    
    def train_minimal_model(self, model: tf.keras.Model, num_classes: int, epochs=5) -> dict:
        """Train model on synthetic data"""
        
        logger.info(f"Generating synthetic training data for {num_classes} classes...")
        
        # Generate synthetic training data
        num_samples = 150
        X_train = np.random.rand(num_samples, 224, 224, 3).astype(np.float32)
        y_train = tf.keras.utils.to_categorical(
            np.random.randint(0, num_classes, num_samples),
            num_classes=num_classes
        )
        
        # Generate validation data
        X_val = np.random.rand(30, 224, 224, 3).astype(np.float32)
        y_val = tf.keras.utils.to_categorical(
            np.random.randint(0, num_classes, 30),
            num_classes=num_classes
        )
        
        logger.info(f"Training model for {epochs} epochs...")
        
        history = model.fit(
            X_train, y_train,
            validation_data=(X_val, y_val),
            epochs=epochs,
            batch_size=16,
            verbose=0
        )
        
        training_stats = {
            'final_loss': float(history.history['loss'][-1]),
            'final_accuracy': float(history.history['accuracy'][-1]),
            'final_val_loss': float(history.history['val_loss'][-1]),
            'final_val_accuracy': float(history.history['val_accuracy'][-1])
        }
        
        return training_stats
    
    def convert_to_tflite(self, model: tf.keras.Model, output_path: Path) -> int:
        """Convert model to TensorFlow Lite with INT8 quantization"""
        
        logger.info("Converting to TensorFlow Lite with INT8 quantization...")
        
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        
        # Apply INT8 quantization
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
        with open(output_path, 'wb') as f:
            f.write(tflite_model)
        
        model_size = len(tflite_model)
        logger.info(f"TFLite model saved: {output_path} ({model_size / 1024:.1f} KB)")
        
        return model_size
    
    def create_model_metadata(self, category: str, species_list: list, 
                             model_size: int, training_stats: dict, 
                             output_dir: Path):
        """Create metadata for the model"""
        
        metadata = {
            "metadata_version": "2.0.0",
            "created_date": datetime.now().isoformat(),
            "model_info": {
                "name": f"{category}_classifier.tflite",
                "version": "2.0.0",
                "type": "species_classification",
                "category": category,
                "description": self.SPECIES_CATEGORIES[category]['description'],
                "file_size_bytes": model_size,
                "quantization": "INT8"
            },
            "input_specification": {
                "shape": [1, 224, 224, 3],
                "dtype": "uint8",
                "format": "RGB"
            },
            "output_specification": {
                "shape": [1, len(species_list)],
                "dtype": "uint8",
                "classes": species_list
            },
            "target_hardware": {
                "platform": "ESP32-S3",
                "min_psram_mb": 8,
                "min_flash_mb": 4
            },
            "performance": {
                "expected_inference_time_ms": 600,
                "memory_usage_kb": 512,
                "target_accuracy": 0.95,
                "model_is_demo": True
            },
            "training_info": {
                "framework": "TensorFlow Lite",
                "training_type": "synthetic_data",
                "note": "Demo model trained on synthetic data. Train on real wildlife datasets for production use.",
                **training_stats
            },
            "supported_species": species_list
        }
        
        metadata_path = output_dir / f"{category}_metadata.json"
        with open(metadata_path, 'w') as f:
            json.dump(metadata, f, indent=2)
        
        logger.info(f"Metadata saved: {metadata_path}")
        
        return metadata
    
    def create_class_labels(self, category: str, species_list: list, output_dir: Path):
        """Create class labels file"""
        
        labels_data = {
            "category": category,
            "labels": species_list,
            "num_classes": len(species_list),
            "label_to_index": {label: idx for idx, label in enumerate(species_list)},
            "index_to_label": {str(idx): label for idx, label in enumerate(species_list)}
        }
        
        labels_path = output_dir / f"{category}_labels.json"
        with open(labels_path, 'w') as f:
            json.dump(labels_data, f, indent=2)
        
        logger.info(f"Class labels saved: {labels_path}")
    
    def create_readme(self, category: str, species_list: list, 
                     model_size_kb: float, output_dir: Path):
        """Create README for the model"""
        
        readme_content = f"""# {category.title()} Wildlife Classifier

## Overview
Specialized TensorFlow Lite model for {category} species classification.

**⚠️ DEMO MODEL**: This model is trained on synthetic data for demonstration and testing.
For production deployment, train on real wildlife datasets using the training pipeline.

## Model Specifications
- **Category**: {category.title()} Wildlife
- **Architecture**: Lightweight CNN optimized for ESP32
- **Input**: 224x224x3 RGB images (uint8)
- **Output**: {len(species_list)} species classes
- **Quantization**: INT8
- **Model Size**: {model_size_kb:.1f} KB
- **Target Platform**: ESP32-S3 (8MB PSRAM)

## Supported Species
{chr(10).join('- ' + species for species in species_list)}

## Performance Targets
- **Inference Time**: < 600ms on ESP32-S3 @ 240MHz
- **Memory Usage**: ~512KB RAM
- **Target Accuracy**: > 95% (on real data)
- **Power Consumption**: < 180mA during inference

## Usage

### Load Model in ESP32
```cpp
#include "firmware/src/ai/vision/species_classifier.h"

SpeciesClassifier classifier;

void setup() {{
    classifier.init();
    classifier.loadModel("/models/species_detection/{category}_classifier.tflite");
}}

void loop() {{
    CameraFrame frame = camera.capture();
    ClassificationResult result = classifier.classify(frame);
    
    Serial.printf("Species: %s\\n", result.species_name.c_str());
    Serial.printf("Confidence: %.2f\\n", result.confidence);
}}
```

### Integration with Detection Pipeline
```cpp
#include "firmware/ml_models/ai_detection_interface.h"

AIDetectionInterface ai;
ai.loadSpecializedModel("{category}_classifier.tflite");

// Automatic classification after motion detection
auto results = ai.enhancedDetection(frame_data, frame_size, width, height);
```

## Training for Production

To train a production model with real data:

1. **Collect Dataset**
   - Gather 500-1000 images per species
   - Use diverse lighting and weather conditions
   - Include various angles and distances

2. **Label Data**
   - Use CVAT or Label Studio
   - Create bounding boxes
   - Verify label accuracy

3. **Train Model**
   ```bash
   python backend/ml/model_trainer.py \\
       --dataset /path/to/{category}_dataset \\
       --output firmware/models/species_detection \\
       --category {category} \\
       --epochs 100
   ```

4. **Validate**
   ```bash
   python backend/ml/model_evaluator.py \\
       --model firmware/models/species_detection/{category}_classifier.tflite \\
       --test-data /path/to/test_set
   ```

5. **Deploy**
   - Flash to ESP32
   - Validate in field conditions
   - Monitor performance metrics

## Files
- `{category}_classifier.tflite` - TFLite model
- `{category}_metadata.json` - Model specifications
- `{category}_labels.json` - Species labels
- `README.md` - This documentation

## Next Steps
- Train on real wildlife datasets
- Validate accuracy on test data
- Deploy and monitor in production
- Iterate based on field performance

## Support
- **Documentation**: See [ML_INTEGRATION_README.md](../../ML_INTEGRATION_README.md)
- **Training Guide**: [ml/TRAINING_DEPLOYMENT_GUIDE.md](../../ml/TRAINING_DEPLOYMENT_GUIDE.md)
- **Issues**: GitHub Issues with `machine-learning` label

Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
"""
        
        readme_path = output_dir / "README.md"
        with open(readme_path, 'w') as f:
            f.write(readme_content)
        
        logger.info(f"README saved: {readme_path}")
    
    def generate_category_model(self, category: str) -> dict:
        """Generate a complete model for a species category"""
        
        logger.info(f"\n{'='*60}")
        logger.info(f"Generating {category.upper()} classifier model")
        logger.info(f"{'='*60}")
        
        # Get species list for this category
        category_info = self.SPECIES_CATEGORIES[category]
        species_list = category_info['species']
        
        # Create output directory
        output_dir = self.output_base_dir / category
        output_dir.mkdir(parents=True, exist_ok=True)
        
        # Create model
        model = self.create_specialized_model(category, species_list)
        logger.info(f"Model created with {model.count_params()} parameters")
        
        # Train on synthetic data
        training_stats = self.train_minimal_model(model, len(species_list), epochs=5)
        
        # Save Keras model
        keras_path = output_dir / f"{category}_classifier.h5"
        model.save(keras_path)
        logger.info(f"Keras model saved: {keras_path}")
        
        # Convert to TFLite
        tflite_path = output_dir / f"{category}_classifier.tflite"
        model_size = self.convert_to_tflite(model, tflite_path)
        
        # Create metadata
        self.create_model_metadata(category, species_list, model_size, 
                                   training_stats, output_dir)
        
        # Create class labels
        self.create_class_labels(category, species_list, output_dir)
        
        # Create README
        self.create_readme(category, species_list, model_size / 1024, output_dir)
        
        result = {
            "category": category,
            "success": True,
            "keras_model": str(keras_path),
            "tflite_model": str(tflite_path),
            "model_size_kb": model_size / 1024,
            "num_species": len(species_list),
            "training_stats": training_stats
        }
        
        logger.info(f"✅ {category.title()} model complete: {result['model_size_kb']:.1f} KB")
        
        return result
    
    def generate_all_models(self) -> dict:
        """Generate all specialized species models"""
        
        logger.info("\n" + "="*60)
        logger.info("SPECIES-SPECIFIC MODEL GENERATION")
        logger.info("="*60)
        
        results = {}
        
        for category in self.SPECIES_CATEGORIES.keys():
            try:
                result = self.generate_category_model(category)
                results[category] = result
            except Exception as e:
                logger.error(f"Failed to generate {category} model: {e}")
                results[category] = {
                    "category": category,
                    "success": False,
                    "error": str(e)
                }
        
        # Summary
        logger.info("\n" + "="*60)
        logger.info("GENERATION SUMMARY")
        logger.info("="*60)
        
        successful = sum(1 for r in results.values() if r.get('success'))
        total_size_kb = sum(r.get('model_size_kb', 0) for r in results.values() if r.get('success'))
        
        logger.info(f"Models generated: {successful}/{len(self.SPECIES_CATEGORIES)}")
        logger.info(f"Total size: {total_size_kb:.1f} KB")
        logger.info(f"Output directory: {self.output_base_dir}")
        
        for category, result in results.items():
            if result.get('success'):
                logger.info(f"  ✅ {category}: {result['model_size_kb']:.1f} KB")
            else:
                logger.info(f"  ❌ {category}: Failed")
        
        return results


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Generate species-specific classifier models")
    parser.add_argument(
        "--output-dir",
        type=str,
        default="/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware/models/species_detection",
        help="Output base directory for models"
    )
    parser.add_argument(
        "--category",
        type=str,
        choices=['deer', 'bird', 'mammal', 'predator', 'all'],
        default='all',
        help="Which category to generate (default: all)"
    )
    
    args = parser.parse_args()
    
    generator = SpeciesModelGenerator(output_base_dir=args.output_dir)
    
    if args.category == 'all':
        results = generator.generate_all_models()
    else:
        results = {args.category: generator.generate_category_model(args.category)}
    
    # Check results
    all_successful = all(r.get('success', False) for r in results.values())
    
    if all_successful:
        print("\n✅ All models generated successfully!")
        print(f"📁 Output directory: {args.output_dir}")
        return 0
    else:
        print("\n⚠️ Some models failed to generate")
        return 1


if __name__ == "__main__":
    exit(main())
