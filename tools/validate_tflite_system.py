#!/usr/bin/env python3
"""
TensorFlow Lite Wildlife Classification System Validator

Validates that the complete TensorFlow Lite wildlife classification system
meets all requirements for ESP32-S3 deployment.
"""

import os
import json
import sys
import logging
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from datetime import datetime

import numpy as np

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class TFLiteSystemValidator:
    """
    Validates the complete TensorFlow Lite wildlife classification system
    for ESP32-S3 deployment readiness.
    """
    
    def __init__(self, project_root: str):
        """Initialize validator with project root directory"""
        self.project_root = Path(project_root)
        self.validation_results = {
            "validation_timestamp": datetime.now().isoformat(),
            "project_root": str(self.project_root),
            "tests": {},
            "overall_status": "UNKNOWN"
        }
        
        logger.info(f"TensorFlow Lite system validator initialized")
        logger.info(f"Project root: {self.project_root}")
    
    def validate_complete_system(self) -> Dict:
        """Run complete system validation"""
        logger.info("Starting complete TensorFlow Lite system validation")
        
        # Test categories
        test_categories = [
            ("Core Infrastructure", self.validate_core_infrastructure),
            ("Model Management", self.validate_model_management),
            ("Backend ML Pipeline", self.validate_backend_pipeline),
            ("ESP32 Integration", self.validate_esp32_integration),
            ("Documentation", self.validate_documentation),
            ("Configuration", self.validate_configuration)
        ]
        
        total_tests = 0
        passed_tests = 0
        
        for category_name, test_function in test_categories:
            logger.info(f"Validating {category_name}")
            
            try:
                category_results = test_function()
                self.validation_results["tests"][category_name] = category_results
                
                category_passed = category_results.get("passed", 0)
                category_total = category_results.get("total", 0)
                
                total_tests += category_total
                passed_tests += category_passed
                
                logger.info(f"{category_name}: {category_passed}/{category_total} tests passed")
                
            except Exception as e:
                logger.error(f"Error validating {category_name}: {e}")
                self.validation_results["tests"][category_name] = {
                    "error": str(e),
                    "passed": 0,
                    "total": 1
                }
                total_tests += 1
        
        # Calculate overall status
        pass_rate = passed_tests / total_tests if total_tests > 0 else 0
        
        if pass_rate >= 0.9:
            overall_status = "EXCELLENT"
        elif pass_rate >= 0.8:
            overall_status = "GOOD"
        elif pass_rate >= 0.7:
            overall_status = "ACCEPTABLE"
        else:
            overall_status = "NEEDS_IMPROVEMENT"
        
        self.validation_results.update({
            "total_tests": total_tests,
            "passed_tests": passed_tests,
            "pass_rate": pass_rate,
            "overall_status": overall_status
        })
        
        logger.info(f"Validation completed: {passed_tests}/{total_tests} tests passed ({pass_rate:.1%})")
        logger.info(f"Overall status: {overall_status}")
        
        return self.validation_results
    
    def validate_core_infrastructure(self) -> Dict:
        """Validate core TensorFlow Lite infrastructure"""
        results = {"category": "Core Infrastructure", "tests": []}
        
        # Test 1: TensorFlow Lite ESP32 implementation
        tensorflow_lite_files = [
            "firmware/src/ai/tensorflow_lite_esp32.h",
            "firmware/src/ai/tensorflow_lite_esp32.cpp"
        ]
        
        for file_path in tensorflow_lite_files:
            full_path = self.project_root / file_path
            test_result = {
                "name": f"TensorFlow Lite ESP32 file exists: {file_path}",
                "passed": full_path.exists(),
                "details": f"File path: {full_path}"
            }
            results["tests"].append(test_result)
        
        # Test 2: Model manager implementation
        model_manager_files = [
            "firmware/src/ai/model_manager.h",
            "firmware/src/ai/model_manager.cpp"
        ]
        
        for file_path in model_manager_files:
            full_path = self.project_root / file_path
            test_result = {
                "name": f"Model manager file exists: {file_path}",
                "passed": full_path.exists(),
                "details": f"File path: {full_path}"
            }
            results["tests"].append(test_result)
        
        # Test 3: Inference engine implementation
        inference_engine_files = [
            "firmware/src/ai/inference_engine.h", 
            "firmware/src/ai/inference_engine.cpp"
        ]
        
        for file_path in inference_engine_files:
            full_path = self.project_root / file_path
            test_result = {
                "name": f"Inference engine file exists: {file_path}",
                "passed": full_path.exists(),
                "details": f"File path: {full_path}"
            }
            results["tests"].append(test_result)
        
        # Test 4: Advanced features
        advanced_features = [
            "firmware/src/ai/preprocessing.h",
            "firmware/src/ai/postprocessing.h",
            "firmware/src/ai/multithreaded_inference.h",
            "firmware/src/ai/federated_learning.h"
        ]
        
        for file_path in advanced_features:
            full_path = self.project_root / file_path
            test_result = {
                "name": f"Advanced feature file exists: {file_path}",
                "passed": full_path.exists(),
                "details": f"File path: {full_path}"
            }
            results["tests"].append(test_result)
        
        # Calculate results
        results["passed"] = sum(1 for test in results["tests"] if test["passed"])
        results["total"] = len(results["tests"])
        
        return results
    
    def validate_model_management(self) -> Dict:
        """Validate model management system"""
        results = {"category": "Model Management", "tests": []}
        
        # Test 1: Model metadata file
        metadata_path = self.project_root / "firmware/models/model_metadata.json"
        test_result = {
            "name": "Model metadata file exists",
            "passed": metadata_path.exists(),
            "details": f"File path: {metadata_path}"
        }
        results["tests"].append(test_result)
        
        # Test 2: Validate metadata structure
        if metadata_path.exists():
            try:
                with open(metadata_path, 'r') as f:
                    metadata = json.load(f)
                
                required_fields = ["metadata_version", "models", "target_hardware"]
                has_required_fields = all(field in metadata for field in required_fields)
                
                test_result = {
                    "name": "Model metadata has required fields",
                    "passed": has_required_fields,
                    "details": f"Required fields: {required_fields}"
                }
                results["tests"].append(test_result)
                
                # Test 3: Model specifications
                if "models" in metadata:
                    models = metadata["models"]
                    valid_models = 0
                    
                    for model in models:
                        required_model_fields = ["id", "name", "version", "type", "file_size_bytes"]
                        if all(field in model for field in required_model_fields):
                            valid_models += 1
                    
                    test_result = {
                        "name": "Model specifications are valid",
                        "passed": valid_models == len(models),
                        "details": f"Valid models: {valid_models}/{len(models)}"
                    }
                    results["tests"].append(test_result)
                
            except Exception as e:
                test_result = {
                    "name": "Model metadata is valid JSON",
                    "passed": False,
                    "details": f"Error: {e}"
                }
                results["tests"].append(test_result)
        
        # Calculate results
        results["passed"] = sum(1 for test in results["tests"] if test["passed"])
        results["total"] = len(results["tests"])
        
        return results
    
    def validate_backend_pipeline(self) -> Dict:
        """Validate backend ML pipeline"""
        results = {"category": "Backend ML Pipeline", "tests": []}
        
        # Test 1: Dataset manager
        dataset_manager_path = self.project_root / "backend/ml/dataset_manager.py"
        test_result = {
            "name": "Dataset manager exists",
            "passed": dataset_manager_path.exists(),
            "details": f"File path: {dataset_manager_path}"
        }
        results["tests"].append(test_result)
        
        # Test 2: Model trainer
        model_trainer_path = self.project_root / "backend/ml/model_trainer.py"
        test_result = {
            "name": "Model trainer exists",
            "passed": model_trainer_path.exists(),
            "details": f"File path: {model_trainer_path}"
        }
        results["tests"].append(test_result)
        
        # Test 3: Model evaluator
        model_evaluator_path = self.project_root / "backend/ml/model_evaluator.py"
        test_result = {
            "name": "Model evaluator exists",
            "passed": model_evaluator_path.exists(),
            "details": f"File path: {model_evaluator_path}"
        }
        results["tests"].append(test_result)
        
        # Test 4: Deployment pipeline
        deployment_pipeline_path = self.project_root / "backend/ml/deployment_pipeline.py"
        test_result = {
            "name": "Deployment pipeline exists",
            "passed": deployment_pipeline_path.exists(),
            "details": f"File path: {deployment_pipeline_path}"
        }
        results["tests"].append(test_result)
        
        # Test 5: Python dependencies check
        requirements_path = self.project_root / "backend/requirements.txt"
        if requirements_path.exists():
            try:
                with open(requirements_path, 'r') as f:
                    requirements = f.read()
                
                required_packages = ["tensorflow", "numpy", "pillow", "requests"]
                has_required_packages = all(pkg in requirements.lower() for pkg in required_packages)
                
                test_result = {
                    "name": "Required Python packages specified",
                    "passed": has_required_packages,
                    "details": f"Required packages: {required_packages}"
                }
                results["tests"].append(test_result)
                
            except Exception as e:
                test_result = {
                    "name": "Requirements file is readable",
                    "passed": False,
                    "details": f"Error: {e}"
                }
                results["tests"].append(test_result)
        
        # Calculate results
        results["passed"] = sum(1 for test in results["tests"] if test["passed"])
        results["total"] = len(results["tests"])
        
        return results
    
    def validate_esp32_integration(self) -> Dict:
        """Validate ESP32 integration"""
        results = {"category": "ESP32 Integration", "tests": []}
        
        # Test 1: Wildlife classifier integration
        wildlife_classifier_path = self.project_root / "ESP32WildlifeCAM-main/src/ai/wildlife_classifier.cpp"
        if wildlife_classifier_path.exists():
            try:
                with open(wildlife_classifier_path, 'r') as f:
                    content = f.read()
                
                # Check for new TensorFlow Lite integration
                has_new_integration = "tensorflow_lite_esp32.h" in content
                
                test_result = {
                    "name": "Wildlife classifier uses new TensorFlow Lite integration",
                    "passed": has_new_integration,
                    "details": "Checks for tensorflow_lite_esp32.h include"
                }
                results["tests"].append(test_result)
                
                # Check for model manager integration
                has_model_manager = "model_manager.h" in content
                
                test_result = {
                    "name": "Wildlife classifier integrates model manager",
                    "passed": has_model_manager,
                    "details": "Checks for model_manager.h include"
                }
                results["tests"].append(test_result)
                
            except Exception as e:
                test_result = {
                    "name": "Wildlife classifier file is readable",
                    "passed": False,
                    "details": f"Error: {e}"
                }
                results["tests"].append(test_result)
        else:
            test_result = {
                "name": "Wildlife classifier file exists",
                "passed": False,
                "details": f"File not found: {wildlife_classifier_path}"
            }
            results["tests"].append(test_result)
        
        # Test 2: PlatformIO configuration
        platformio_path = self.project_root / "ESP32WildlifeCAM-main/platformio.ini"
        if platformio_path.exists():
            try:
                with open(platformio_path, 'r') as f:
                    content = f.read()
                
                # Check for TensorFlow Lite dependency
                has_tflite_dep = "TensorFlow Lite" in content
                
                test_result = {
                    "name": "PlatformIO configuration includes TensorFlow Lite",
                    "passed": has_tflite_dep,
                    "details": "Checks for TensorFlow Lite dependency"
                }
                results["tests"].append(test_result)
                
            except Exception as e:
                test_result = {
                    "name": "PlatformIO configuration is readable",
                    "passed": False,
                    "details": f"Error: {e}"
                }
                results["tests"].append(test_result)
        
        # Calculate results
        results["passed"] = sum(1 for test in results["tests"] if test["passed"])
        results["total"] = len(results["tests"])
        
        return results
    
    def validate_documentation(self) -> Dict:
        """Validate documentation"""
        results = {"category": "Documentation", "tests": []}
        
        # Test 1: README file
        readme_files = ["README.md", "README_COMPLETE_IMPLEMENTATION.md"]
        for readme_file in readme_files:
            readme_path = self.project_root / readme_file
            test_result = {
                "name": f"Documentation file exists: {readme_file}",
                "passed": readme_path.exists(),
                "details": f"File path: {readme_path}"
            }
            results["tests"].append(test_result)
        
        # Test 2: ML workflow documentation
        ml_workflow_path = self.project_root / "ESP32WildlifeCAM-main/docs/ML_WORKFLOW.md"
        test_result = {
            "name": "ML workflow documentation exists",
            "passed": ml_workflow_path.exists(),
            "details": f"File path: {ml_workflow_path}"
        }
        results["tests"].append(test_result)
        
        # Calculate results
        results["passed"] = sum(1 for test in results["tests"] if test["passed"])
        results["total"] = len(results["tests"])
        
        return results
    
    def validate_configuration(self) -> Dict:
        """Validate system configuration"""
        results = {"category": "Configuration", "tests": []}
        
        # Test 1: ESP32 configuration
        config_path = self.project_root / "ESP32WildlifeCAM-main/include/config.h"
        if config_path.exists():
            try:
                with open(config_path, 'r') as f:
                    content = f.read()
                
                # Check for camera configuration
                has_camera_config = "CAMERA_MODEL" in content
                test_result = {
                    "name": "Configuration includes camera settings",
                    "passed": has_camera_config,
                    "details": "Checks for CAMERA_MODEL definitions"
                }
                results["tests"].append(test_result)
                
                # Check for AI configuration
                has_ai_config = "SPECIES_CONFIDENCE_THRESHOLD" in content or "TFLITE" in content
                test_result = {
                    "name": "Configuration includes AI settings",
                    "passed": has_ai_config,
                    "details": "Checks for AI-related configurations"
                }
                results["tests"].append(test_result)
                
            except Exception as e:
                test_result = {
                    "name": "Configuration file is readable",
                    "passed": False,
                    "details": f"Error: {e}"
                }
                results["tests"].append(test_result)
        else:
            test_result = {
                "name": "Configuration file exists",
                "passed": False,
                "details": f"File not found: {config_path}"
            }
            results["tests"].append(test_result)
        
        # Calculate results
        results["passed"] = sum(1 for test in results["tests"] if test["passed"])
        results["total"] = len(results["tests"])
        
        return results
    
    def generate_report(self, output_path: str = None) -> str:
        """Generate validation report"""
        if output_path is None:
            output_path = self.project_root / "validation_report.json"
        else:
            output_path = Path(output_path)
        
        # Save detailed JSON report
        with open(output_path, 'w') as f:
            json.dump(self.validation_results, f, indent=2)
        
        # Generate summary report
        summary_path = output_path.with_suffix('.txt')
        with open(summary_path, 'w') as f:
            f.write("TensorFlow Lite Wildlife Classification System Validation Report\n")
            f.write("=" * 70 + "\n\n")
            
            f.write(f"Validation Date: {self.validation_results['validation_timestamp']}\n")
            f.write(f"Project Root: {self.validation_results['project_root']}\n")
            f.write(f"Overall Status: {self.validation_results['overall_status']}\n")
            f.write(f"Pass Rate: {self.validation_results['pass_rate']:.1%}\n")
            f.write(f"Tests Passed: {self.validation_results['passed_tests']}/{self.validation_results['total_tests']}\n\n")
            
            for category, results in self.validation_results["tests"].items():
                if "error" in results:
                    f.write(f"{category}: ERROR - {results['error']}\n")
                else:
                    f.write(f"{category}: {results['passed']}/{results['total']} tests passed\n")
                    
                    # List failed tests
                    if "tests" in results:
                        failed_tests = [test for test in results["tests"] if not test["passed"]]
                        if failed_tests:
                            f.write("  Failed tests:\n")
                            for test in failed_tests:
                                f.write(f"    - {test['name']}: {test['details']}\n")
                f.write("\n")
        
        logger.info(f"Validation report saved to: {output_path}")
        logger.info(f"Summary report saved to: {summary_path}")
        
        return str(output_path)


def main():
    """Main validation function"""
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        project_root = os.getcwd()
    
    logger.info("Starting TensorFlow Lite Wildlife Classification System validation")
    
    validator = TFLiteSystemValidator(project_root)
    results = validator.validate_complete_system()
    
    # Generate report
    report_path = validator.generate_report()
    
    # Print summary
    print("\n" + "=" * 70)
    print("VALIDATION SUMMARY")
    print("=" * 70)
    print(f"Overall Status: {results['overall_status']}")
    print(f"Pass Rate: {results['pass_rate']:.1%}")
    print(f"Tests Passed: {results['passed_tests']}/{results['total_tests']}")
    print(f"Report saved to: {report_path}")
    
    # Return appropriate exit code
    exit_code = 0 if results["pass_rate"] >= 0.7 else 1
    sys.exit(exit_code)


if __name__ == "__main__":
    main()