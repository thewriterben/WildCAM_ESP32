#!/usr/bin/env python3
"""
Model Conversion Script for WildCAM_ESP32
Converts trained Keras model to TensorFlow Lite with quantization

Author: Manus AI
Date: October 29, 2025
"""

import tensorflow as tf
import os
import sys
from pathlib import Path

# --- Configuration ---
SAVED_MODEL_PATH = 'wildlife_classifier.h5'
TFLITE_MODEL_PATH = 'wildlife_classifier.tflite'
DATASET_DIR = './training_data/'
IMAGE_SIZE = (128, 128)

def representative_dataset():
    """
    Generate representative dataset for quantization calibration.
    Uses a subset of training images.
    """
    print("Generating representative dataset for quantization...")
    
    train_dir = Path(DATASET_DIR) / 'train'
    
    if not train_dir.exists():
        print(f"ERROR: Training directory not found: {train_dir}")
        print("Make sure you've run prepare_training_data.py first")
        sys.exit(1)
    
    # Collect image paths
    image_paths = []
    for species_dir in train_dir.iterdir():
        if species_dir.is_dir():
            for img_path in species_dir.glob('*.jpg'):
                image_paths.append(img_path)
                if len(image_paths) >= 100:  # Use 100 images for calibration
                    break
            if len(image_paths) >= 100:
                break
    
    if len(image_paths) < 10:
        print(f"WARNING: Only found {len(image_paths)} images for calibration")
        print("Quantization may not be optimal")
    
    print(f"Using {len(image_paths)} images for quantization calibration")
    
    # Generate batches
    for img_path in image_paths:
        try:
            # Load and preprocess image
            img = tf.io.read_file(str(img_path))
            img = tf.image.decode_jpeg(img, channels=3)
            img = tf.image.resize(img, IMAGE_SIZE)
            img = tf.cast(img, tf.float32) / 255.0
            img = tf.expand_dims(img, 0)  # Add batch dimension
            
            yield [img]
        except Exception as e:
            print(f"Warning: Could not process {img_path}: {e}")
            continue

def convert_model():
    """
    Convert Keras model to TensorFlow Lite with full integer quantization
    """
    print("="*80)
    print("WildCAM_ESP32 Model Conversion")
    print("="*80)
    
    # Check if model exists
    if not os.path.exists(SAVED_MODEL_PATH):
        print(f"\nERROR: Model file not found: {SAVED_MODEL_PATH}")
        print("Make sure you've run train_model.py first")
        sys.exit(1)
    
    print(f"\nLoading trained Keras model from {SAVED_MODEL_PATH}...")
    try:
        model = tf.keras.models.load_model(SAVED_MODEL_PATH)
        print("✓ Model loaded successfully")
    except Exception as e:
        print(f"ERROR: Failed to load model: {e}")
        sys.exit(1)
    
    # Print model summary
    print("\nModel Summary:")
    model.summary()
    
    # Create converter
    print("\nConverting model to TensorFlow Lite with full integer quantization...")
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    
    # Enable optimizations
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    
    # Set representative dataset for quantization
    converter.representative_dataset = representative_dataset
    
    # Ensure full integer quantization
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    
    # Set input and output tensors to uint8
    converter.inference_input_type = tf.uint8
    converter.inference_output_type = tf.uint8
    
    # Convert
    try:
        tflite_model = converter.convert()
        print("✓ Conversion successful")
    except Exception as e:
        print(f"ERROR: Conversion failed: {e}")
        print("\nTrying without strict integer-only mode...")
        
        # Fallback: try without strict integer-only
        converter.target_spec.supported_ops = [
            tf.lite.OpsSet.TFLITE_BUILTINS_INT8,
            tf.lite.OpsSet.TFLITE_BUILTINS
        ]
        try:
            tflite_model = converter.convert()
            print("✓ Conversion successful (with fallback mode)")
        except Exception as e2:
            print(f"ERROR: Conversion failed even with fallback: {e2}")
            sys.exit(1)
    
    # Save the TFLite model
    with open(TFLITE_MODEL_PATH, 'wb') as f:
        f.write(tflite_model)
    
    # Print statistics
    original_size = os.path.getsize(SAVED_MODEL_PATH)
    quantized_size = os.path.getsize(TFLITE_MODEL_PATH)
    reduction = (1 - quantized_size / original_size) * 100
    
    print("\n" + "="*80)
    print("Conversion Complete!")
    print("="*80)
    print(f"Original model size:  {original_size / 1024:.2f} KB")
    print(f"Quantized model size: {quantized_size / 1024:.2f} KB")
    print(f"Size reduction:       {reduction:.1f}%")
    print(f"\nQuantized model saved as: {TFLITE_MODEL_PATH}")
    print("\nNext steps:")
    print("  1. Convert to C header: xxd -i wildlife_classifier.tflite > wildlife_model.h")
    print("  2. Copy wildlife_model.h to your ESP32 project's include/ folder")
    print("  3. Follow deployment_guide.md for integration")
    print("="*80)

if __name__ == '__main__':
    convert_model()

