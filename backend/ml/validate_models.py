#!/usr/bin/env python3
"""
Validate Species Classifier Models

Tests all generated models to ensure they:
1. Load successfully
2. Have correct input/output shapes
3. Can perform inference
4. Match metadata specifications
"""

import os
import sys
import json
import logging
from pathlib import Path
import tensorflow as tf
import numpy as np

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class ModelValidator:
    """Validate TensorFlow Lite models"""
    
    def __init__(self, models_dir: str):
        self.models_dir = Path(models_dir)
        self.results = []
    
    def validate_model(self, model_path: Path, metadata_path: Path = None) -> dict:
        """Validate a single model"""
        
        result = {
            "model": str(model_path.name),
            "path": str(model_path),
            "success": False,
            "checks": {}
        }
        
        try:
            # Check file exists
            if not model_path.exists():
                result["error"] = "Model file not found"
                return result
            
            result["checks"]["file_exists"] = True
            result["file_size_kb"] = model_path.stat().st_size / 1024
            
            # Load model
            interpreter = tf.lite.Interpreter(model_path=str(model_path))
            interpreter.allocate_tensors()
            result["checks"]["loads"] = True
            
            # Get input details
            input_details = interpreter.get_input_details()
            output_details = interpreter.get_output_details()
            
            result["input_shape"] = input_details[0]["shape"].tolist()
            result["input_dtype"] = str(input_details[0]["dtype"])
            result["output_shape"] = output_details[0]["shape"].tolist()
            result["output_dtype"] = str(output_details[0]["dtype"])
            
            result["checks"]["correct_input_shape"] = (
                input_details[0]["shape"].tolist() == [1, 224, 224, 3]
            )
            
            # Test inference
            input_shape = input_details[0]["shape"]
            input_data = np.random.randint(0, 255, input_shape, dtype=np.uint8)
            
            interpreter.set_tensor(input_details[0]["index"], input_data)
            interpreter.invoke()
            output_data = interpreter.get_tensor(output_details[0]["index"])
            
            result["checks"]["inference"] = True
            result["output_example"] = output_data[0][:5].tolist()  # First 5 values
            
            # Validate metadata if provided
            if metadata_path and metadata_path.exists():
                with open(metadata_path, 'r') as f:
                    metadata = json.load(f)
                
                result["checks"]["metadata_exists"] = True
                result["metadata_version"] = metadata.get("metadata_version")
                result["num_species"] = len(metadata.get("supported_species", []))
                
                # Check metadata matches model
                meta_input_shape = metadata.get("input_specification", {}).get("shape")
                meta_output_shape = metadata.get("output_specification", {}).get("shape")
                
                result["checks"]["metadata_input_matches"] = (
                    meta_input_shape == input_details[0]["shape"].tolist()
                )
                result["checks"]["metadata_output_matches"] = (
                    meta_output_shape == output_details[0]["shape"].tolist()
                )
            
            # All checks passed
            all_passed = all(result["checks"].values())
            result["success"] = all_passed
            
        except Exception as e:
            result["error"] = str(e)
            logger.error(f"Error validating {model_path.name}: {e}")
        
        return result
    
    def validate_all_models(self) -> dict:
        """Validate all models in the library"""
        
        logger.info("="*60)
        logger.info("MODEL VALIDATION")
        logger.info("="*60)
        
        # Find all TFLite models
        model_files = list(self.models_dir.rglob("*.tflite"))
        logger.info(f"Found {len(model_files)} models")
        
        for model_path in model_files:
            logger.info(f"\nValidating: {model_path.relative_to(self.models_dir)}")
            
            # Look for metadata file
            metadata_path = model_path.parent / model_path.name.replace(".tflite", "_metadata.json")
            if not metadata_path.exists():
                # Try alternative naming
                metadata_path = model_path.parent / "model_metadata.json"
            
            result = self.validate_model(model_path, metadata_path)
            self.results.append(result)
            
            # Log results
            if result["success"]:
                logger.info(f"  ✅ PASSED - {result['file_size_kb']:.1f} KB")
                logger.info(f"     Input: {result['input_shape']}")
                logger.info(f"     Output: {result['output_shape']}")
                if "num_species" in result:
                    logger.info(f"     Species: {result['num_species']}")
            else:
                logger.error(f"  ❌ FAILED")
                if "error" in result:
                    logger.error(f"     Error: {result['error']}")
                failed_checks = [k for k, v in result["checks"].items() if not v]
                if failed_checks:
                    logger.error(f"     Failed checks: {', '.join(failed_checks)}")
        
        # Summary
        logger.info("\n" + "="*60)
        logger.info("VALIDATION SUMMARY")
        logger.info("="*60)
        
        total = len(self.results)
        passed = sum(1 for r in self.results if r["success"])
        failed = total - passed
        total_size = sum(r.get("file_size_kb", 0) for r in self.results)
        
        logger.info(f"Total models: {total}")
        logger.info(f"Passed: {passed}")
        logger.info(f"Failed: {failed}")
        logger.info(f"Total size: {total_size:.1f} KB ({total_size/1024:.2f} MB)")
        
        if failed > 0:
            logger.warning("\n⚠️  Some models failed validation")
            for r in self.results:
                if not r["success"]:
                    logger.warning(f"  - {r['model']}")
        else:
            logger.info("\n✅ All models passed validation!")
        
        return {
            "total": total,
            "passed": passed,
            "failed": failed,
            "total_size_kb": total_size,
            "results": self.results
        }
    
    def save_report(self, output_path: str):
        """Save validation report to JSON"""
        
        from datetime import datetime
        
        report = {
            "validation_date": datetime.now().isoformat(),
            "models_directory": str(self.models_dir),
            "summary": {
                "total_models": len(self.results),
                "passed": sum(1 for r in self.results if r["success"]),
                "failed": sum(1 for r in self.results if not r["success"]),
                "total_size_kb": sum(r.get("file_size_kb", 0) for r in self.results)
            },
            "results": self.results
        }
        
        with open(output_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        logger.info(f"\n📄 Validation report saved: {output_path}")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Validate species classifier models")
    parser.add_argument(
        "--models-dir",
        type=str,
        default="/home/runner/work/WildCAM_ESP32/WildCAM_ESP32/firmware/models",
        help="Directory containing models"
    )
    parser.add_argument(
        "--report",
        type=str,
        default=None,
        help="Save validation report to JSON file"
    )
    
    args = parser.parse_args()
    
    # Validate models
    validator = ModelValidator(models_dir=args.models_dir)
    summary = validator.validate_all_models()
    
    # Save report if requested
    if args.report:
        validator.save_report(args.report)
    
    # Exit with appropriate code
    if summary["failed"] > 0:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
