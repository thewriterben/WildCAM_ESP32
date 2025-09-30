#!/usr/bin/env python3
"""
ML Integration Test Script

Tests the complete machine learning integration from model generation
to deployment readiness for ESP32 wildlife camera.
"""

import os
import sys
import json
import logging
from pathlib import Path

# Add backend to path
backend_path = Path(__file__).parent.parent / "backend"
sys.path.insert(0, str(backend_path))

from ml.generate_test_model import TestModelGenerator

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class MLIntegrationTest:
    """Test ML integration end-to-end"""
    
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.test_output_dir = self.project_root / "firmware" / "models" / "test"
        self.test_results = {
            "tests_passed": [],
            "tests_failed": [],
            "warnings": []
        }
    
    def run_all_tests(self) -> bool:
        """Run all integration tests"""
        
        logger.info("="*60)
        logger.info("ML INTEGRATION TEST SUITE")
        logger.info("="*60)
        
        tests = [
            ("Test Model Generation", self.test_model_generation),
            ("Model File Validation", self.test_model_files),
            ("Metadata Validation", self.test_metadata),
            ("Model Size Check", self.test_model_size),
            ("ESP32 Compatibility", self.test_esp32_compatibility),
        ]
        
        for test_name, test_func in tests:
            logger.info(f"\n▶ Running: {test_name}")
            try:
                if test_func():
                    logger.info(f"✅ PASS: {test_name}")
                    self.test_results["tests_passed"].append(test_name)
                else:
                    logger.error(f"❌ FAIL: {test_name}")
                    self.test_results["tests_failed"].append(test_name)
            except Exception as e:
                logger.error(f"❌ ERROR in {test_name}: {e}")
                self.test_results["tests_failed"].append(test_name)
        
        # Print summary
        self._print_summary()
        
        return len(self.test_results["tests_failed"]) == 0
    
    def test_model_generation(self) -> bool:
        """Test 1: Generate test model"""
        
        try:
            generator = TestModelGenerator(output_dir=str(self.test_output_dir))
            result = generator.generate_test_model()
            
            if not result['success']:
                logger.error("Model generation failed")
                return False
            
            logger.info(f"  Model size: {result['model_size_kb']:.1f} KB")
            logger.info(f"  Training accuracy: {result['training_stats']['final_accuracy']:.3f}")
            
            return True
            
        except Exception as e:
            logger.error(f"Model generation error: {e}")
            return False
    
    def test_model_files(self) -> bool:
        """Test 2: Validate model files exist"""
        
        required_files = [
            "wildlife_classifier_test_quantized.tflite",
            "model_metadata.json",
            "class_labels.json",
            "README.md"
        ]
        
        for filename in required_files:
            file_path = self.test_output_dir / filename
            if not file_path.exists():
                logger.error(f"  Missing file: {filename}")
                return False
            logger.info(f"  ✓ Found: {filename}")
        
        return True
    
    def test_metadata(self) -> bool:
        """Test 3: Validate metadata structure"""
        
        metadata_path = self.test_output_dir / "model_metadata.json"
        
        try:
            with open(metadata_path, 'r') as f:
                metadata = json.load(f)
            
            # Check required fields
            required_fields = [
                "model_info",
                "input_specification",
                "output_specification",
                "target_hardware",
                "supported_species"
            ]
            
            for field in required_fields:
                if field not in metadata:
                    logger.error(f"  Missing metadata field: {field}")
                    return False
                logger.info(f"  ✓ Field present: {field}")
            
            # Validate target hardware
            hw = metadata.get("target_hardware", {})
            if hw.get("platform") != "ESP32-S3":
                self.test_results["warnings"].append(
                    "Target platform should be ESP32-S3"
                )
            
            # Validate species list
            species = metadata.get("supported_species", [])
            if len(species) < 1:
                logger.error("  No species defined in metadata")
                return False
            logger.info(f"  ✓ Species count: {len(species)}")
            
            return True
            
        except json.JSONDecodeError as e:
            logger.error(f"  Invalid JSON: {e}")
            return False
        except Exception as e:
            logger.error(f"  Metadata validation error: {e}")
            return False
    
    def test_model_size(self) -> bool:
        """Test 4: Check model size constraints"""
        
        tflite_path = self.test_output_dir / "wildlife_classifier_test_quantized.tflite"
        
        if not tflite_path.exists():
            logger.error("  TFLite model not found")
            return False
        
        model_size_mb = tflite_path.stat().st_size / (1024 * 1024)
        logger.info(f"  Model size: {model_size_mb:.2f} MB")
        
        # Check against ESP32 constraints
        max_size_mb = 15  # ESP32-S3 typically has 16MB flash
        if model_size_mb > max_size_mb:
            logger.error(f"  Model too large for ESP32: {model_size_mb:.2f} MB > {max_size_mb} MB")
            return False
        
        if model_size_mb < 0.1:
            logger.info(f"  ✓ Compact model suitable for edge deployment")
        elif model_size_mb < 5:
            logger.info(f"  ✓ Model size acceptable for ESP32-S3")
        else:
            self.test_results["warnings"].append(
                f"Model is large ({model_size_mb:.1f} MB). Consider optimization."
            )
        
        return True
    
    def test_esp32_compatibility(self) -> bool:
        """Test 5: Check ESP32 compatibility"""
        
        try:
            import tensorflow as tf
            
            tflite_path = self.test_output_dir / "wildlife_classifier_test_quantized.tflite"
            
            # Load TFLite model
            interpreter = tf.lite.Interpreter(model_path=str(tflite_path))
            interpreter.allocate_tensors()
            
            # Get input and output details
            input_details = interpreter.get_input_details()
            output_details = interpreter.get_output_details()
            
            logger.info(f"  Input shape: {input_details[0]['shape']}")
            logger.info(f"  Input dtype: {input_details[0]['dtype']}")
            logger.info(f"  Output shape: {output_details[0]['shape']}")
            logger.info(f"  Output dtype: {output_details[0]['dtype']}")
            
            # Check for INT8 quantization
            if input_details[0]['dtype'] == 'uint8' or input_details[0]['dtype'] == 'int8':
                logger.info("  ✓ Model is quantized (INT8)")
            else:
                self.test_results["warnings"].append(
                    "Model is not INT8 quantized. Consider quantization for better ESP32 performance."
                )
            
            # Test inference
            import numpy as np
            input_shape = input_details[0]['shape']
            test_input = np.random.randint(0, 255, input_shape, dtype=np.uint8)
            
            interpreter.set_tensor(input_details[0]['index'], test_input)
            interpreter.invoke()
            output = interpreter.get_tensor(output_details[0]['index'])
            
            logger.info(f"  ✓ Test inference successful, output shape: {output.shape}")
            
            return True
            
        except ImportError:
            logger.warning("  TensorFlow not available, skipping runtime tests")
            self.test_results["warnings"].append(
                "TensorFlow not available for runtime validation"
            )
            return True
        except Exception as e:
            logger.error(f"  ESP32 compatibility check failed: {e}")
            return False
    
    def _print_summary(self):
        """Print test summary"""
        
        logger.info("\n" + "="*60)
        logger.info("TEST SUMMARY")
        logger.info("="*60)
        
        total = len(self.test_results["tests_passed"]) + len(self.test_results["tests_failed"])
        passed = len(self.test_results["tests_passed"])
        
        logger.info(f"Total Tests: {total}")
        logger.info(f"Passed: {passed}")
        logger.info(f"Failed: {len(self.test_results['tests_failed'])}")
        
        if self.test_results["tests_failed"]:
            logger.info("\nFailed Tests:")
            for test in self.test_results["tests_failed"]:
                logger.info(f"  ❌ {test}")
        
        if self.test_results["warnings"]:
            logger.info("\nWarnings:")
            for warning in self.test_results["warnings"]:
                logger.info(f"  ⚠️  {warning}")
        
        if passed == total and not self.test_results["tests_failed"]:
            logger.info("\n✅ ALL TESTS PASSED!")
        else:
            logger.info("\n❌ SOME TESTS FAILED")
        
        logger.info("="*60)


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Test ML integration")
    parser.add_argument(
        "--project-root",
        type=str,
        default="/home/runner/work/WildCAM_ESP32/WildCAM_ESP32",
        help="Project root directory"
    )
    
    args = parser.parse_args()
    
    # Run tests
    tester = MLIntegrationTest(project_root=args.project_root)
    success = tester.run_all_tests()
    
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
