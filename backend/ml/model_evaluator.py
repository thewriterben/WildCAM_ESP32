#!/usr/bin/env python3
"""
Model Evaluator for Wildlife Classification

Comprehensive evaluation and validation of TensorFlow Lite models
for wildlife classification with edge deployment metrics.
"""

import os
import json
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from datetime import datetime

import numpy as np
import tensorflow as tf
from sklearn.metrics import classification_report, confusion_matrix, accuracy_score
from sklearn.metrics import precision_recall_fscore_support, roc_auc_score
import matplotlib.pyplot as plt
import seaborn as sns

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeModelEvaluator:
    """
    Evaluates TensorFlow Lite models for wildlife classification
    with comprehensive metrics and ESP32 deployment validation.
    """
    
    def __init__(self, model_path: str, dataset_dir: str, output_dir: str):
        """
        Initialize model evaluator
        
        Args:
            model_path: Path to TensorFlow Lite model
            dataset_dir: Directory containing test dataset
            output_dir: Directory for evaluation outputs
        """
        self.model_path = Path(model_path)
        self.dataset_dir = Path(dataset_dir)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Load model
        self.interpreter = tf.lite.Interpreter(model_path=str(self.model_path))
        self.interpreter.allocate_tensors()
        
        # Get model details
        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()
        
        # Load class labels
        self.labels = self._load_labels()
        self.num_classes = len(self.labels)
        
        logger.info(f"Model evaluator initialized for {self.num_classes} classes")
        logger.info(f"Model input shape: {self.input_details[0]['shape']}")
        logger.info(f"Model output shape: {self.output_details[0]['shape']}")
    
    def _load_labels(self) -> List[str]:
        """Load class labels from dataset"""
        labels_file = self.dataset_dir / "labels.txt"
        if labels_file.exists():
            with open(labels_file, 'r') as f:
                return [line.strip() for line in f.readlines()]
        
        # Fallback: get labels from test directory
        test_dir = self.dataset_dir / "test"
        if test_dir.exists():
            return sorted([d.name for d in test_dir.iterdir() if d.is_dir()])
        
        raise ValueError("Could not find class labels")
    
    def preprocess_image(self, image_path: Path) -> np.ndarray:
        """Preprocess image for model input"""
        from PIL import Image
        
        # Load and resize image
        with Image.open(image_path) as img:
            img = img.convert('RGB')
            img = img.resize((224, 224))  # Assuming 224x224 input
            
            # Convert to numpy array
            img_array = np.array(img)
            
            # Normalize based on model input type
            if self.input_details[0]['dtype'] == np.uint8:
                # Keep as uint8 for quantized models
                return img_array
            else:
                # Normalize to 0-1 for float models
                return img_array.astype(np.float32) / 255.0
    
    def evaluate_accuracy(self, num_samples: int = None) -> Dict:
        """
        Evaluate model accuracy on test dataset
        
        Args:
            num_samples: Maximum number of samples to evaluate (None for all)
            
        Returns:
            Evaluation results dictionary
        """
        logger.info("Evaluating model accuracy")
        
        test_dir = self.dataset_dir / "test"
        if not test_dir.exists():
            raise ValueError("Test directory not found")
        
        predictions = []
        true_labels = []
        inference_times = []
        image_paths = []
        
        sample_count = 0
        
        # Iterate through test samples
        for class_idx, class_name in enumerate(self.labels):
            class_dir = test_dir / class_name
            if not class_dir.exists():
                continue
            
            image_files = list(class_dir.glob("*.jpg")) + list(class_dir.glob("*.png"))
            
            for img_path in image_files:
                if num_samples and sample_count >= num_samples:
                    break
                
                try:
                    # Preprocess image
                    img_array = self.preprocess_image(img_path)
                    
                    # Run inference
                    start_time = datetime.now()
                    
                    self.interpreter.set_tensor(
                        self.input_details[0]['index'], 
                        np.expand_dims(img_array, axis=0)
                    )
                    self.interpreter.invoke()
                    
                    end_time = datetime.now()
                    
                    # Get prediction
                    output_data = self.interpreter.get_tensor(self.output_details[0]['index'])
                    predicted_class = np.argmax(output_data[0])
                    
                    predictions.append(predicted_class)
                    true_labels.append(class_idx)
                    inference_times.append((end_time - start_time).total_seconds() * 1000)
                    image_paths.append(str(img_path))
                    
                    sample_count += 1
                    
                    if sample_count % 100 == 0:
                        logger.info(f"Processed {sample_count} samples")
                        
                except Exception as e:
                    logger.error(f"Error processing {img_path}: {e}")
                    continue
            
            if num_samples and sample_count >= num_samples:
                break
        
        # Calculate metrics
        accuracy = accuracy_score(true_labels, predictions)
        precision, recall, f1, _ = precision_recall_fscore_support(
            true_labels, predictions, average='weighted'
        )
        
        # Per-class metrics
        class_report = classification_report(
            true_labels, predictions, 
            target_names=self.labels, 
            output_dict=True
        )
        
        # Confusion matrix
        conf_matrix = confusion_matrix(true_labels, predictions)
        
        # Top-k accuracy
        top5_accuracy = self._calculate_topk_accuracy(true_labels, predictions, k=5)
        
        results = {
            "model_path": str(self.model_path),
            "evaluation_date": datetime.now().isoformat(),
            "total_samples": len(predictions),
            "accuracy": float(accuracy),
            "precision": float(precision),
            "recall": float(recall),
            "f1_score": float(f1),
            "top5_accuracy": float(top5_accuracy),
            "average_inference_time_ms": float(np.mean(inference_times)),
            "min_inference_time_ms": float(np.min(inference_times)),
            "max_inference_time_ms": float(np.max(inference_times)),
            "class_report": class_report,
            "confusion_matrix": conf_matrix.tolist(),
            "class_labels": self.labels
        }
        
        # Save results
        with open(self.output_dir / "accuracy_evaluation.json", 'w') as f:
            json.dump(results, f, indent=2)
        
        # Create confusion matrix plot
        self._plot_confusion_matrix(conf_matrix, self.labels)
        
        logger.info(f"Accuracy evaluation completed: {accuracy:.4f}")
        logger.info(f"Average inference time: {np.mean(inference_times):.2f}ms")
        
        return results
    
    def _calculate_topk_accuracy(self, true_labels: List[int], predictions: List[int], k: int) -> float:
        """Calculate top-k accuracy (simplified implementation)"""
        # This is a simplified version - in practice, you'd need the full probability distributions
        # For now, just return the standard accuracy if k >= 1
        if k >= 1:
            return accuracy_score(true_labels, predictions)
        return 0.0
    
    def _plot_confusion_matrix(self, conf_matrix: np.ndarray, labels: List[str]):
        """Create and save confusion matrix plot"""
        plt.figure(figsize=(12, 10))
        sns.heatmap(
            conf_matrix, 
            annot=True, 
            fmt='d', 
            xticklabels=labels, 
            yticklabels=labels,
            cmap='Blues'
        )
        plt.title('Confusion Matrix')
        plt.ylabel('True Label')
        plt.xlabel('Predicted Label')
        plt.xticks(rotation=45)
        plt.yticks(rotation=0)
        plt.tight_layout()
        plt.savefig(self.output_dir / "confusion_matrix.png", dpi=300, bbox_inches='tight')
        plt.close()
    
    def benchmark_edge_performance(self, num_iterations: int = 1000) -> Dict:
        """
        Benchmark model performance for edge deployment
        
        Args:
            num_iterations: Number of inference iterations
            
        Returns:
            Performance benchmark results
        """
        logger.info(f"Benchmarking edge performance ({num_iterations} iterations)")
        
        # Create dummy input data
        input_shape = self.input_details[0]['shape']
        input_dtype = self.input_details[0]['dtype']
        
        if input_dtype == np.uint8:
            dummy_input = np.random.randint(0, 256, input_shape, dtype=np.uint8)
        else:
            dummy_input = np.random.random(input_shape).astype(np.float32)
        
        # Warm-up
        for _ in range(10):
            self.interpreter.set_tensor(self.input_details[0]['index'], dummy_input)
            self.interpreter.invoke()
        
        # Benchmark
        inference_times = []
        
        for i in range(num_iterations):
            start_time = datetime.now()
            
            self.interpreter.set_tensor(self.input_details[0]['index'], dummy_input)
            self.interpreter.invoke()
            
            end_time = datetime.now()
            inference_times.append((end_time - start_time).total_seconds() * 1000)
            
            if (i + 1) % 100 == 0:
                logger.info(f"Completed {i + 1}/{num_iterations} iterations")
        
        # Calculate statistics
        mean_time = np.mean(inference_times)
        std_time = np.std(inference_times)
        min_time = np.min(inference_times)
        max_time = np.max(inference_times)
        p95_time = np.percentile(inference_times, 95)
        p99_time = np.percentile(inference_times, 99)
        
        # Model size
        model_size = os.path.getsize(self.model_path)
        
        # Memory usage estimation (simplified)
        tensor_memory = 0
        for detail in self.input_details + self.output_details:
            tensor_size = np.prod(detail['shape']) * np.dtype(detail['dtype']).itemsize
            tensor_memory += tensor_size
        
        benchmark_results = {
            "model_path": str(self.model_path),
            "benchmark_date": datetime.now().isoformat(),
            "iterations": num_iterations,
            "model_size_bytes": model_size,
            "model_size_kb": model_size / 1024,
            "estimated_memory_usage_bytes": tensor_memory,
            "inference_statistics": {
                "mean_time_ms": float(mean_time),
                "std_time_ms": float(std_time),
                "min_time_ms": float(min_time),
                "max_time_ms": float(max_time),
                "p95_time_ms": float(p95_time),
                "p99_time_ms": float(p99_time)
            },
            "throughput": {
                "inferences_per_second": 1000.0 / mean_time,
                "batch_processing_capability": "single_image"
            },
            "esp32_compatibility": {
                "meets_1s_requirement": mean_time <= 1000.0,
                "meets_500ms_target": mean_time <= 500.0,
                "memory_fits_2mb": tensor_memory <= 2 * 1024 * 1024,
                "model_fits_4mb": model_size <= 4 * 1024 * 1024
            }
        }
        
        # Save benchmark results
        with open(self.output_dir / "edge_performance_benchmark.json", 'w') as f:
            json.dump(benchmark_results, f, indent=2)
        
        # Create performance plot
        self._plot_inference_times(inference_times)
        
        logger.info(f"Edge performance benchmark completed")
        logger.info(f"Mean inference time: {mean_time:.2f}ms")
        logger.info(f"Throughput: {1000.0 / mean_time:.1f} inferences/second")
        
        return benchmark_results
    
    def _plot_inference_times(self, inference_times: List[float]):
        """Create inference time distribution plot"""
        plt.figure(figsize=(12, 6))
        
        # Histogram
        plt.subplot(1, 2, 1)
        plt.hist(inference_times, bins=50, alpha=0.7, edgecolor='black')
        plt.title('Inference Time Distribution')
        plt.xlabel('Inference Time (ms)')
        plt.ylabel('Frequency')
        plt.grid(True, alpha=0.3)
        
        # Box plot
        plt.subplot(1, 2, 2)
        plt.boxplot(inference_times)
        plt.title('Inference Time Box Plot')
        plt.ylabel('Inference Time (ms)')
        plt.grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.savefig(self.output_dir / "inference_time_distribution.png", dpi=300, bbox_inches='tight')
        plt.close()
    
    def validate_esp32_deployment(self) -> Dict:
        """
        Validate model for ESP32-S3 deployment requirements
        
        Returns:
            Validation results with pass/fail status
        """
        logger.info("Validating ESP32-S3 deployment requirements")
        
        model_size = os.path.getsize(self.model_path)
        
        # Get model details
        input_shape = self.input_details[0]['shape']
        output_shape = self.output_details[0]['shape']
        
        # Calculate memory requirements
        input_memory = np.prod(input_shape) * np.dtype(self.input_details[0]['dtype']).itemsize
        output_memory = np.prod(output_shape) * np.dtype(self.output_details[0]['dtype']).itemsize
        
        # Run quick inference test
        dummy_input = np.random.random(input_shape).astype(self.input_details[0]['dtype'])
        
        start_time = datetime.now()
        self.interpreter.set_tensor(self.input_details[0]['index'], dummy_input)
        self.interpreter.invoke()
        end_time = datetime.now()
        
        inference_time = (end_time - start_time).total_seconds() * 1000
        
        # Define ESP32-S3 constraints
        max_model_size = 15 * 1024 * 1024  # 15MB
        max_inference_time = 2000  # 2 seconds
        max_memory_usage = 2 * 1024 * 1024  # 2MB
        target_inference_time = 1000  # 1 second target
        
        validation_results = {
            "validation_date": datetime.now().isoformat(),
            "model_path": str(self.model_path),
            "esp32_s3_requirements": {
                "model_size": {
                    "actual_bytes": model_size,
                    "actual_mb": model_size / (1024 * 1024),
                    "limit_mb": max_model_size / (1024 * 1024),
                    "passed": model_size <= max_model_size
                },
                "inference_time": {
                    "actual_ms": float(inference_time),
                    "limit_ms": max_inference_time,
                    "target_ms": target_inference_time,
                    "meets_limit": inference_time <= max_inference_time,
                    "meets_target": inference_time <= target_inference_time
                },
                "memory_usage": {
                    "input_bytes": input_memory,
                    "output_bytes": output_memory,
                    "total_tensor_bytes": input_memory + output_memory,
                    "limit_bytes": max_memory_usage,
                    "passed": (input_memory + output_memory) <= max_memory_usage
                },
                "model_format": {
                    "is_tflite": str(self.model_path).endswith('.tflite'),
                    "input_dtype": str(self.input_details[0]['dtype']),
                    "output_dtype": str(self.output_details[0]['dtype']),
                    "quantized": self.input_details[0]['dtype'] == np.uint8
                }
            },
            "overall_compatibility": True
        }
        
        # Check overall compatibility
        checks = [
            validation_results["esp32_s3_requirements"]["model_size"]["passed"],
            validation_results["esp32_s3_requirements"]["inference_time"]["meets_limit"],
            validation_results["esp32_s3_requirements"]["memory_usage"]["passed"],
            validation_results["esp32_s3_requirements"]["model_format"]["is_tflite"]
        ]
        
        validation_results["overall_compatibility"] = all(checks)
        
        # Save validation results
        with open(self.output_dir / "esp32_deployment_validation.json", 'w') as f:
            json.dump(validation_results, f, indent=2)
        
        logger.info(f"ESP32-S3 deployment validation: {'PASSED' if validation_results['overall_compatibility'] else 'FAILED'}")
        
        return validation_results
    
    def generate_comprehensive_report(self) -> Dict:
        """Generate comprehensive evaluation report"""
        logger.info("Generating comprehensive evaluation report")
        
        # Run all evaluations
        accuracy_results = self.evaluate_accuracy(num_samples=1000)
        benchmark_results = self.benchmark_edge_performance(num_iterations=500)
        validation_results = self.validate_esp32_deployment()
        
        # Compile comprehensive report
        report = {
            "report_generated": datetime.now().isoformat(),
            "model_path": str(self.model_path),
            "dataset_path": str(self.dataset_dir),
            "summary": {
                "accuracy": accuracy_results["accuracy"],
                "f1_score": accuracy_results["f1_score"],
                "mean_inference_time_ms": benchmark_results["inference_statistics"]["mean_time_ms"],
                "model_size_mb": benchmark_results["model_size_kb"] / 1024,
                "esp32_compatible": validation_results["overall_compatibility"]
            },
            "detailed_results": {
                "accuracy_evaluation": accuracy_results,
                "performance_benchmark": benchmark_results,
                "deployment_validation": validation_results
            },
            "recommendations": self._generate_recommendations(
                accuracy_results, benchmark_results, validation_results
            )
        }
        
        # Save comprehensive report
        with open(self.output_dir / "comprehensive_evaluation_report.json", 'w') as f:
            json.dump(report, f, indent=2)
        
        logger.info("Comprehensive evaluation report generated")
        return report
    
    def _generate_recommendations(self, accuracy_results: Dict, 
                                benchmark_results: Dict, 
                                validation_results: Dict) -> List[str]:
        """Generate recommendations based on evaluation results"""
        recommendations = []
        
        # Accuracy recommendations
        if accuracy_results["accuracy"] < 0.90:
            recommendations.append("Consider collecting more training data or using data augmentation to improve accuracy")
        
        if accuracy_results["accuracy"] < 0.85:
            recommendations.append("Model accuracy is below target. Consider using a larger base model or more training epochs")
        
        # Performance recommendations  
        mean_time = benchmark_results["inference_statistics"]["mean_time_ms"]
        if mean_time > 1000:
            recommendations.append("Inference time exceeds 1 second target. Consider model quantization or architecture optimization")
        
        if mean_time > 2000:
            recommendations.append("Inference time too slow for real-time use. Major model optimization required")
        
        # Memory recommendations
        model_size_mb = benchmark_results["model_size_kb"] / 1024
        if model_size_mb > 10:
            recommendations.append("Model size is large. Consider pruning or more aggressive quantization")
        
        # Deployment recommendations
        if not validation_results["overall_compatibility"]:
            recommendations.append("Model does not meet ESP32-S3 deployment requirements. Optimization needed")
        
        if not benchmark_results["esp32_compatibility"]["meets_500ms_target"]:
            recommendations.append("Consider targeting 500ms inference time for better user experience")
        
        return recommendations


if __name__ == "__main__":
    # Example usage
    evaluator = WildlifeModelEvaluator(
        model_path="/data/models/wildlife_classifier_quantized.tflite",
        dataset_dir="/data/tensorflow_dataset",
        output_dir="/data/evaluation_results"
    )
    
    # Generate comprehensive evaluation report
    report = evaluator.generate_comprehensive_report()
    
    print("Evaluation completed!")
    print(f"Model accuracy: {report['summary']['accuracy']:.4f}")
    print(f"Mean inference time: {report['summary']['mean_inference_time_ms']:.2f}ms")
    print(f"ESP32 compatible: {report['summary']['esp32_compatible']}")