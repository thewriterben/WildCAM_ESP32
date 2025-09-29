#!/usr/bin/env python3
"""
Model Trainer for Wildlife Classification

Implements transfer learning and model training for TensorFlow Lite
wildlife classification optimized for ESP32 deployment.
"""

import os
import json
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from datetime import datetime

import tensorflow as tf
import numpy as np
from tensorflow.keras import layers, models, optimizers, callbacks
from tensorflow.keras.applications import MobileNetV3Large, MobileNetV2
from tensorflow.keras.preprocessing.image import ImageDataGenerator

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeModelTrainer:
    """
    Trains TensorFlow Lite models for wildlife classification using
    transfer learning optimized for ESP32-S3 deployment constraints.
    """
    
    def __init__(self, dataset_dir: str, output_dir: str, config: Dict = None):
        """
        Initialize model trainer
        
        Args:
            dataset_dir: Directory containing training dataset
            output_dir: Directory for output models and artifacts
            config: Training configuration dictionary
        """
        self.dataset_dir = Path(dataset_dir)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Default training configuration
        self.config = {
            "image_size": [224, 224],
            "batch_size": 32,
            "epochs": 50,
            "learning_rate": 0.001,
            "base_model": "MobileNetV3Large",
            "fine_tune_layers": 20,
            "dropout_rate": 0.2,
            "l2_regularization": 0.0001,
            "early_stopping_patience": 10,
            "reduce_lr_patience": 5,
            "quantization": True,
            "model_optimization": True
        }
        
        if config:
            self.config.update(config)
        
        # Load species labels
        self.labels = self._load_labels()
        self.num_classes = len(self.labels)
        
        logger.info(f"Trainer initialized with {self.num_classes} classes")
    
    def _load_labels(self) -> List[str]:
        """Load species labels from dataset"""
        labels_file = self.dataset_dir / "labels.txt"
        if labels_file.exists():
            with open(labels_file, 'r') as f:
                return [line.strip() for line in f.readlines()]
        
        # Fallback: get labels from train directory structure
        train_dir = self.dataset_dir / "train"
        if train_dir.exists():
            return sorted([d.name for d in train_dir.iterdir() if d.is_dir()])
        
        raise ValueError("Could not find species labels")
    
    def create_data_generators(self) -> Tuple[tf.keras.utils.Sequence, tf.keras.utils.Sequence, tf.keras.utils.Sequence]:
        """Create data generators for training, validation, and testing"""
        
        # Training data generator with augmentation
        train_datagen = ImageDataGenerator(
            rescale=1.0/255.0,
            rotation_range=20,
            width_shift_range=0.2,
            height_shift_range=0.2,
            horizontal_flip=True,
            zoom_range=0.2,
            shear_range=0.2,
            fill_mode='nearest'
        )
        
        # Validation and test generators (no augmentation)
        val_test_datagen = ImageDataGenerator(rescale=1.0/255.0)
        
        train_generator = train_datagen.flow_from_directory(
            self.dataset_dir / "train",
            target_size=self.config["image_size"],
            batch_size=self.config["batch_size"],
            class_mode='categorical',
            shuffle=True
        )
        
        validation_generator = val_test_datagen.flow_from_directory(
            self.dataset_dir / "validation",
            target_size=self.config["image_size"],
            batch_size=self.config["batch_size"],
            class_mode='categorical',
            shuffle=False
        )
        
        test_generator = val_test_datagen.flow_from_directory(
            self.dataset_dir / "test",
            target_size=self.config["image_size"],
            batch_size=self.config["batch_size"],
            class_mode='categorical',
            shuffle=False
        )
        
        return train_generator, validation_generator, test_generator
    
    def create_base_model(self) -> tf.keras.Model:
        """Create base model for transfer learning"""
        
        input_shape = (*self.config["image_size"], 3)
        
        if self.config["base_model"] == "MobileNetV3Large":
            base_model = MobileNetV3Large(
                weights='imagenet',
                include_top=False,
                input_shape=input_shape
            )
        elif self.config["base_model"] == "MobileNetV2":
            base_model = MobileNetV2(
                weights='imagenet',
                include_top=False,
                input_shape=input_shape
            )
        else:
            raise ValueError(f"Unsupported base model: {self.config['base_model']}")
        
        # Freeze base model layers initially
        base_model.trainable = False
        
        return base_model
    
    def build_model(self) -> tf.keras.Model:
        """Build complete model with custom classification head"""
        
        base_model = self.create_base_model()
        
        # Add custom classification head
        model = models.Sequential([
            base_model,
            layers.GlobalAveragePooling2D(),
            layers.Dropout(self.config["dropout_rate"]),
            layers.Dense(
                128, 
                activation='relu',
                kernel_regularizer=tf.keras.regularizers.l2(self.config["l2_regularization"])
            ),
            layers.Dropout(self.config["dropout_rate"]),
            layers.Dense(
                self.num_classes, 
                activation='softmax',
                name='species_predictions'
            )
        ])
        
        return model
    
    def compile_model(self, model: tf.keras.Model) -> tf.keras.Model:
        """Compile model with optimizer and loss function"""
        
        optimizer = optimizers.Adam(learning_rate=self.config["learning_rate"])
        
        model.compile(
            optimizer=optimizer,
            loss='categorical_crossentropy',
            metrics=['accuracy', 'top_k_categorical_accuracy']
        )
        
        return model
    
    def create_callbacks(self) -> List[tf.keras.callbacks.Callback]:
        """Create training callbacks"""
        
        callbacks_list = [
            callbacks.EarlyStopping(
                monitor='val_loss',
                patience=self.config["early_stopping_patience"],
                restore_best_weights=True,
                verbose=1
            ),
            callbacks.ReduceLROnPlateau(
                monitor='val_loss',
                factor=0.5,
                patience=self.config["reduce_lr_patience"],
                min_lr=1e-7,
                verbose=1
            ),
            callbacks.ModelCheckpoint(
                str(self.output_dir / "best_model.h5"),
                monitor='val_accuracy',
                save_best_only=True,
                save_weights_only=False,
                verbose=1
            ),
            callbacks.CSVLogger(
                str(self.output_dir / "training_log.csv")
            )
        ]
        
        return callbacks_list
    
    def train_model(self) -> Dict:
        """
        Train wildlife classification model
        
        Returns:
            Training statistics and metrics
        """
        logger.info("Starting model training")
        
        # Create data generators
        train_gen, val_gen, test_gen = self.create_data_generators()
        
        # Build and compile model
        model = self.build_model()
        model = self.compile_model(model)
        
        # Print model summary
        model.summary()
        
        # Create callbacks
        callback_list = self.create_callbacks()
        
        # Initial training (frozen base model)
        logger.info("Phase 1: Training with frozen base model")
        history_1 = model.fit(
            train_gen,
            epochs=self.config["epochs"] // 2,
            validation_data=val_gen,
            callbacks=callback_list,
            verbose=1
        )
        
        # Fine-tuning phase
        logger.info("Phase 2: Fine-tuning with unfrozen layers")
        
        # Unfreeze top layers of base model
        base_model = model.layers[0]
        base_model.trainable = True
        
        # Fine-tune from this layer onwards
        fine_tune_at = len(base_model.layers) - self.config["fine_tune_layers"]
        
        # Freeze all layers before fine_tune_at
        for layer in base_model.layers[:fine_tune_at]:
            layer.trainable = False
        
        # Use lower learning rate for fine-tuning
        model.compile(
            optimizer=optimizers.Adam(learning_rate=self.config["learning_rate"] / 10),
            loss='categorical_crossentropy',
            metrics=['accuracy', 'top_k_categorical_accuracy']
        )
        
        # Continue training
        history_2 = model.fit(
            train_gen,
            epochs=self.config["epochs"],
            initial_epoch=len(history_1.history['loss']),
            validation_data=val_gen,
            callbacks=callback_list,
            verbose=1
        )
        
        # Combine training histories
        combined_history = {}
        for key in history_1.history.keys():
            combined_history[key] = history_1.history[key] + history_2.history[key]
        
        # Evaluate on test set
        test_metrics = model.evaluate(test_gen, verbose=1)
        test_results = dict(zip(model.metrics_names, test_metrics))
        
        # Save final model
        model.save(self.output_dir / "wildlife_classifier.h5")
        
        # Create training summary
        training_stats = {
            "training_completed": datetime.now().isoformat(),
            "config": self.config,
            "num_classes": self.num_classes,
            "test_metrics": test_results,
            "training_samples": train_gen.samples,
            "validation_samples": val_gen.samples,
            "test_samples": test_gen.samples,
            "epochs_trained": len(combined_history['loss']),
            "best_val_accuracy": max(combined_history['val_accuracy']),
            "final_train_accuracy": combined_history['accuracy'][-1],
            "final_val_accuracy": combined_history['val_accuracy'][-1]
        }
        
        # Save training statistics
        with open(self.output_dir / "training_stats.json", 'w') as f:
            json.dump(training_stats, f, indent=2)
        
        # Save class labels
        with open(self.output_dir / "class_labels.json", 'w') as f:
            json.dump({
                "labels": self.labels,
                "label_to_index": {label: idx for idx, label in enumerate(self.labels)}
            }, f, indent=2)
        
        logger.info(f"Training completed. Test accuracy: {test_results['accuracy']:.4f}")
        
        return training_stats
    
    def convert_to_tflite(self, model_path: str = None, quantize: bool = True) -> str:
        """
        Convert trained model to TensorFlow Lite format
        
        Args:
            model_path: Path to saved model (defaults to best model)
            quantize: Whether to apply INT8 quantization
            
        Returns:
            Path to converted TFLite model
        """
        if model_path is None:
            model_path = str(self.output_dir / "best_model.h5")
        
        logger.info(f"Converting model to TensorFlow Lite: {model_path}")
        
        # Load the trained model
        model = tf.keras.models.load_model(model_path)
        
        # Create TFLite converter
        converter = tf.lite.TFLiteConverter.from_keras_model(model)
        
        if quantize:
            logger.info("Applying INT8 quantization")
            
            # Apply quantization
            converter.optimizations = [tf.lite.Optimize.DEFAULT]
            
            # Create representative dataset for quantization
            def representative_data_gen():
                # Use validation data for representative dataset
                val_gen = ImageDataGenerator(rescale=1.0/255.0).flow_from_directory(
                    self.dataset_dir / "validation",
                    target_size=self.config["image_size"],
                    batch_size=1,
                    class_mode=None,
                    shuffle=False
                )
                
                for i in range(100):  # Use 100 samples for calibration
                    yield [next(val_gen)]
            
            converter.representative_dataset = representative_data_gen
            converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
            converter.inference_input_type = tf.uint8
            converter.inference_output_type = tf.uint8
        
        # Convert model
        tflite_model = converter.convert()
        
        # Save TFLite model
        tflite_filename = "wildlife_classifier_quantized.tflite" if quantize else "wildlife_classifier.tflite"
        tflite_path = self.output_dir / tflite_filename
        
        with open(tflite_path, 'wb') as f:
            f.write(tflite_model)
        
        # Get model size
        model_size = len(tflite_model)
        
        logger.info(f"TFLite model saved: {tflite_path} ({model_size / 1024:.1f} KB)")
        
        # Save conversion metadata
        conversion_info = {
            "original_model": str(model_path),
            "tflite_model": str(tflite_path),
            "quantized": quantize,
            "model_size_bytes": model_size,
            "conversion_date": datetime.now().isoformat(),
            "target_platform": "ESP32-S3"
        }
        
        with open(self.output_dir / "tflite_conversion_info.json", 'w') as f:
            json.dump(conversion_info, f, indent=2)
        
        return str(tflite_path)
    
    def benchmark_tflite_model(self, tflite_path: str) -> Dict:
        """
        Benchmark TFLite model performance
        
        Args:
            tflite_path: Path to TFLite model
            
        Returns:
            Benchmark results
        """
        logger.info(f"Benchmarking TFLite model: {tflite_path}")
        
        # Load TFLite model
        interpreter = tf.lite.Interpreter(model_path=tflite_path)
        interpreter.allocate_tensors()
        
        # Get input and output details
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        
        # Create test data generator
        test_gen = ImageDataGenerator(rescale=1.0/255.0).flow_from_directory(
            self.dataset_dir / "test",
            target_size=self.config["image_size"],
            batch_size=1,
            class_mode='categorical',
            shuffle=False
        )
        
        # Run benchmark
        correct_predictions = 0
        total_predictions = 0
        inference_times = []
        
        for i in range(min(100, test_gen.samples)):  # Test on subset
            batch_x, batch_y = next(test_gen)
            
            # Convert input to appropriate type
            if input_details[0]['dtype'] == np.uint8:
                input_data = (batch_x[0] * 255).astype(np.uint8)
            else:
                input_data = batch_x[0].astype(np.float32)
            
            # Run inference
            start_time = datetime.now()
            interpreter.set_tensor(input_details[0]['index'], np.expand_dims(input_data, axis=0))
            interpreter.invoke()
            end_time = datetime.now()
            
            # Get prediction
            output_data = interpreter.get_tensor(output_details[0]['index'])
            prediction = np.argmax(output_data)
            true_label = np.argmax(batch_y[0])
            
            if prediction == true_label:
                correct_predictions += 1
                
            total_predictions += 1
            inference_times.append((end_time - start_time).total_seconds() * 1000)
        
        # Calculate metrics
        accuracy = correct_predictions / total_predictions
        avg_inference_time = np.mean(inference_times)
        
        benchmark_results = {
            "accuracy": accuracy,
            "total_predictions": total_predictions,
            "correct_predictions": correct_predictions,
            "average_inference_time_ms": avg_inference_time,
            "min_inference_time_ms": np.min(inference_times),
            "max_inference_time_ms": np.max(inference_times),
            "model_size_bytes": os.path.getsize(tflite_path),
            "input_shape": input_details[0]['shape'].tolist(),
            "output_shape": output_details[0]['shape'].tolist(),
            "input_dtype": str(input_details[0]['dtype']),
            "output_dtype": str(output_details[0]['dtype'])
        }
        
        logger.info(f"Benchmark results - Accuracy: {accuracy:.4f}, Avg inference time: {avg_inference_time:.2f}ms")
        
        # Save benchmark results
        with open(self.output_dir / "tflite_benchmark.json", 'w') as f:
            json.dump(benchmark_results, f, indent=2)
        
        return benchmark_results


if __name__ == "__main__":
    # Example usage
    config = {
        "image_size": [224, 224],
        "batch_size": 32,
        "epochs": 30,
        "learning_rate": 0.001,
        "base_model": "MobileNetV3Large"
    }
    
    trainer = WildlifeModelTrainer(
        dataset_dir="/data/tensorflow_dataset",
        output_dir="/data/trained_models",
        config=config
    )
    
    # Train model
    training_stats = trainer.train_model()
    
    # Convert to TFLite
    tflite_path = trainer.convert_to_tflite(quantize=True)
    
    # Benchmark TFLite model
    benchmark_results = trainer.benchmark_tflite_model(tflite_path)
    
    print("Training completed!")
    print(f"Best validation accuracy: {training_stats['best_val_accuracy']:.4f}")
    print(f"TFLite model size: {benchmark_results['model_size_bytes'] / 1024:.1f} KB")
    print(f"TFLite inference time: {benchmark_results['average_inference_time_ms']:.2f} ms")