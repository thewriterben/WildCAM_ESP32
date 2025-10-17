#!/usr/bin/env python3
"""
Test Model Demonstration Script

This script demonstrates how to use the generated test model for wildlife
species classification. It shows model loading, inference, and result interpretation.

Usage:
    python3 test_model_demo.py
    python3 test_model_demo.py --benchmark
    python3 test_model_demo.py --image path/to/image.jpg

Requirements:
    pip install tensorflow numpy pillow
"""

import argparse
import time
import json
from pathlib import Path
import numpy as np

try:
    import tensorflow as tf
except ImportError:
    print("❌ TensorFlow not installed. Run: pip install tensorflow")
    exit(1)

try:
    from PIL import Image
except ImportError:
    print("❌ Pillow not installed. Run: pip install pillow")
    exit(1)


class WildlifeClassifierDemo:
    """Demo class for test model inference"""
    
    def __init__(self, model_path: str):
        self.model_path = Path(model_path)
        self.model_dir = self.model_path.parent
        self.interpreter = None
        self.input_details = None
        self.output_details = None
        self.labels = []
        self.metadata = {}
        
    def load_model(self):
        """Load the TFLite model and related files"""
        print(f"Loading model from: {self.model_path}")
        
        # Load TFLite model
        self.interpreter = tf.lite.Interpreter(model_path=str(self.model_path))
        self.interpreter.allocate_tensors()
        
        # Get input/output details
        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()
        
        # Load class labels
        labels_path = self.model_dir / "class_labels.json"
        with open(labels_path, 'r') as f:
            labels_data = json.load(f)
            self.labels = labels_data['labels']
        
        # Load metadata
        metadata_path = self.model_dir / "model_metadata.json"
        with open(metadata_path, 'r') as f:
            self.metadata = json.load(f)
        
        print("✅ Model loaded successfully")
        self._print_model_info()
    
    def _print_model_info(self):
        """Print model information"""
        print("\nModel Information:")
        print(f"  Version: {self.metadata['model_info']['version']}")
        print(f"  Size: {self.metadata['model_info']['file_size_bytes'] / 1024:.1f} KB")
        print(f"  Input shape: {self.input_details[0]['shape']}")
        print(f"  Output shape: {self.output_details[0]['shape']}")
        print(f"  Number of classes: {len(self.labels)}")
        print(f"  Platform: {self.metadata['target_hardware']['platform']}")
        print(f"\nSupported Species:")
        for i, label in enumerate(self.labels, 1):
            print(f"  {i}. {label.replace('_', ' ').title()}")
    
    def preprocess_image(self, image_path: str):
        """Preprocess image for model input"""
        # Load image
        img = Image.open(image_path)
        
        # Resize to model input size
        input_shape = self.input_details[0]['shape']
        target_size = (input_shape[2], input_shape[1])  # width, height
        img = img.resize(target_size, Image.BILINEAR)
        
        # Convert to RGB if needed
        if img.mode != 'RGB':
            img = img.convert('RGB')
        
        # Convert to numpy array
        img_array = np.array(img, dtype=np.uint8)
        
        # Add batch dimension
        img_array = np.expand_dims(img_array, axis=0)
        
        return img_array
    
    def classify(self, image_data: np.ndarray):
        """Run inference on preprocessed image"""
        # Set input tensor
        self.interpreter.set_tensor(self.input_details[0]['index'], image_data)
        
        # Run inference
        start_time = time.time()
        self.interpreter.invoke()
        inference_time = (time.time() - start_time) * 1000  # Convert to ms
        
        # Get output
        output = self.interpreter.get_tensor(self.output_details[0]['index'])
        
        # De-quantize output (uint8 to float)
        output_scale = self.output_details[0]['quantization'][0]
        output_zero_point = self.output_details[0]['quantization'][1]
        output_float = (output.astype(np.float32) - output_zero_point) * output_scale
        
        # Get predictions
        predicted_class = np.argmax(output_float)
        confidence = output_float[0][predicted_class]
        
        return {
            'class_id': int(predicted_class),
            'species': self.labels[predicted_class],
            'confidence': float(confidence),
            'inference_time_ms': inference_time,
            'all_scores': output_float[0].tolist()
        }
    
    def classify_image(self, image_path: str):
        """Complete classification pipeline for an image"""
        print(f"\n{'='*60}")
        print(f"Classifying: {image_path}")
        print('='*60)
        
        # Preprocess
        print("Preprocessing image...")
        image_data = self.preprocess_image(image_path)
        
        # Classify
        print("Running inference...")
        result = self.classify(image_data)
        
        # Display results
        self._print_result(result)
        
        return result
    
    def _print_result(self, result):
        """Print classification result"""
        print("\nClassification Result:")
        print(f"  Species: {result['species'].replace('_', ' ').title()}")
        print(f"  Confidence: {result['confidence']:.4f}")
        print(f"  Class ID: {result['class_id']}")
        print(f"  Inference Time: {result['inference_time_ms']:.2f} ms")
        
        # Top 3 predictions
        scores = result['all_scores']
        top_3_idx = np.argsort(scores)[-3:][::-1]
        
        print("\nTop 3 Predictions:")
        for i, idx in enumerate(top_3_idx, 1):
            print(f"  {i}. {self.labels[idx].replace('_', ' ').title()}: "
                  f"{scores[idx]:.4f}")
        
        # Warning about test model
        print("\n⚠️  WARNING: This is a test model trained on synthetic data.")
        print("   Results are not meaningful. For production, train a real model.")
    
    def benchmark(self, num_iterations: int = 100):
        """Benchmark inference performance"""
        print(f"\n{'='*60}")
        print(f"Running Benchmark ({num_iterations} iterations)")
        print('='*60)
        
        # Create random input
        input_shape = self.input_details[0]['shape']
        dummy_input = np.random.randint(0, 255, input_shape, dtype=np.uint8)
        
        times = []
        print("\nRunning inference...")
        
        for i in range(num_iterations):
            result = self.classify(dummy_input)
            times.append(result['inference_time_ms'])
            
            if (i + 1) % 10 == 0:
                print(f"  Progress: {i+1}/{num_iterations}")
        
        # Calculate statistics
        times = np.array(times)
        
        print("\nBenchmark Results:")
        print(f"  Average: {np.mean(times):.2f} ms")
        print(f"  Min: {np.min(times):.2f} ms")
        print(f"  Max: {np.max(times):.2f} ms")
        print(f"  Std Dev: {np.std(times):.2f} ms")
        print(f"  Median: {np.median(times):.2f} ms")
        print(f"  95th percentile: {np.percentile(times, 95):.2f} ms")
        
        # Compare to target
        target_time = self.metadata['performance']['expected_inference_time_ms']
        print(f"\n  Target inference time: {target_time} ms")
        
        avg_time = np.mean(times)
        if avg_time <= target_time:
            print(f"  ✅ Performance target met! ({avg_time:.2f}ms <= {target_time}ms)")
        else:
            print(f"  ⚠️  Above target ({avg_time:.2f}ms > {target_time}ms)")
            print(f"     Note: ESP32 will be slower than this CPU benchmark")


def main():
    parser = argparse.ArgumentParser(
        description='Wildlife Species Classification Test Model Demo'
    )
    parser.add_argument(
        '--model',
        type=str,
        default='firmware/models/test/wildlife_classifier_test_quantized.tflite',
        help='Path to TFLite model file'
    )
    parser.add_argument(
        '--image',
        type=str,
        help='Path to image file to classify'
    )
    parser.add_argument(
        '--benchmark',
        action='store_true',
        help='Run performance benchmark'
    )
    parser.add_argument(
        '--iterations',
        type=int,
        default=100,
        help='Number of benchmark iterations (default: 100)'
    )
    
    args = parser.parse_args()
    
    # Print header
    print("\n" + "="*60)
    print("Wildlife Species Classification - Test Model Demo")
    print("="*60)
    
    # Initialize demo
    demo = WildlifeClassifierDemo(args.model)
    
    try:
        # Load model
        demo.load_model()
        
        # Run requested operation
        if args.benchmark:
            demo.benchmark(args.iterations)
        elif args.image:
            demo.classify_image(args.image)
        else:
            print("\n⚠️  No operation specified.")
            print("\nUsage examples:")
            print("  # Classify an image")
            print("  python3 test_model_demo.py --image path/to/image.jpg")
            print("\n  # Run benchmark")
            print("  python3 test_model_demo.py --benchmark")
            print("\n  # Run longer benchmark")
            print("  python3 test_model_demo.py --benchmark --iterations 500")
        
        print("\n" + "="*60)
        print("Demo complete!")
        print("="*60 + "\n")
        
    except FileNotFoundError as e:
        print(f"\n❌ Error: File not found - {e}")
        print("   Ensure the model files exist in the correct location.")
        exit(1)
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()
        exit(1)


if __name__ == '__main__':
    main()
