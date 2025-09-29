#!/usr/bin/env python3
"""
Deployment Pipeline for Wildlife Classification Models

Automates the complete pipeline from training to ESP32 deployment
with model optimization and validation.
"""

import os
import json
import logging
import shutil
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime

from dataset_manager import DatasetManager
from model_trainer import WildlifeModelTrainer
from model_evaluator import WildlifeModelEvaluator

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WildlifeDeploymentPipeline:
    """
    Complete deployment pipeline for wildlife classification models
    from data collection to ESP32-ready TensorFlow Lite models.
    """
    
    def __init__(self, config_path: str, output_dir: str):
        """
        Initialize deployment pipeline
        
        Args:
            config_path: Path to pipeline configuration file
            output_dir: Base output directory for all artifacts
        """
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Load configuration
        with open(config_path, 'r') as f:
            self.config = json.load(f)
        
        # Create subdirectories
        self.dataset_dir = self.output_dir / "dataset"
        self.models_dir = self.output_dir / "models"
        self.evaluation_dir = self.output_dir / "evaluation"
        self.deployment_dir = self.output_dir / "deployment"
        
        for dir_path in [self.dataset_dir, self.models_dir, 
                        self.evaluation_dir, self.deployment_dir]:
            dir_path.mkdir(exist_ok=True)
        
        # Initialize pipeline components
        self.dataset_manager = None
        self.model_trainer = None
        self.model_evaluator = None
        
        logger.info(f"Deployment pipeline initialized")
        logger.info(f"Output directory: {self.output_dir}")
    
    def run_complete_pipeline(self) -> Dict:
        """
        Run the complete deployment pipeline
        
        Returns:
            Pipeline execution results
        """
        logger.info("Starting complete deployment pipeline")
        
        pipeline_results = {
            "pipeline_started": datetime.now().isoformat(),
            "config": self.config,
            "stages": {}
        }
        
        try:
            # Stage 1: Data collection and preprocessing
            if self.config.get("data_collection", {}).get("enabled", True):
                logger.info("Stage 1: Data collection and preprocessing")
                data_results = self.collect_and_preprocess_data()
                pipeline_results["stages"]["data_collection"] = data_results
            
            # Stage 2: Model training
            if self.config.get("model_training", {}).get("enabled", True):
                logger.info("Stage 2: Model training")
                training_results = self.train_model()
                pipeline_results["stages"]["model_training"] = training_results
            
            # Stage 3: Model evaluation
            if self.config.get("model_evaluation", {}).get("enabled", True):
                logger.info("Stage 3: Model evaluation")
                evaluation_results = self.evaluate_model()
                pipeline_results["stages"]["model_evaluation"] = evaluation_results
            
            # Stage 4: Model optimization and deployment
            if self.config.get("deployment", {}).get("enabled", True):
                logger.info("Stage 4: Model optimization and deployment")
                deployment_results = self.optimize_and_deploy()
                pipeline_results["stages"]["deployment"] = deployment_results
            
            pipeline_results["pipeline_completed"] = datetime.now().isoformat()
            pipeline_results["success"] = True
            
        except Exception as e:
            logger.error(f"Pipeline failed: {e}")
            pipeline_results["error"] = str(e)
            pipeline_results["success"] = False
            pipeline_results["pipeline_failed"] = datetime.now().isoformat()
        
        # Save pipeline results
        with open(self.output_dir / "pipeline_results.json", 'w') as f:
            json.dump(pipeline_results, f, indent=2)
        
        logger.info(f"Pipeline completed: {'SUCCESS' if pipeline_results['success'] else 'FAILED'}")
        
        return pipeline_results
    
    def collect_and_preprocess_data(self) -> Dict:
        """Collect and preprocess training data"""
        logger.info("Collecting and preprocessing data")
        
        data_config = self.config.get("data_collection", {})
        
        # Initialize dataset manager
        self.dataset_manager = DatasetManager(
            base_dir=str(self.dataset_dir),
            species_config=data_config.get("species_config_path")
        )
        
        results = {
            "stage": "data_collection",
            "started": datetime.now().isoformat()
        }
        
        # Collect data from sources
        if data_config.get("collect_from_inaturalist", False):
            species_list = data_config.get("species_list", ["white_tailed_deer", "black_bear"])
            samples_per_species = data_config.get("samples_per_species", 1000)
            
            collection_stats = self.dataset_manager.collect_from_inaturalist(
                species_list, samples_per_species
            )
            results["inaturalist_collection"] = collection_stats
        
        # Preprocess dataset
        preprocessing_config = data_config.get("preprocessing", {})
        image_size = tuple(preprocessing_config.get("image_size", [224, 224]))
        augmentation = preprocessing_config.get("augmentation", True)
        
        preprocessing_stats = self.dataset_manager.preprocess_dataset(
            image_size=image_size,
            augmentation=augmentation
        )
        results["preprocessing"] = preprocessing_stats
        
        # Export for TensorFlow training
        tf_dataset_dir = self.dataset_dir / "tensorflow_format"
        self.dataset_manager.export_tensorflow_dataset(str(tf_dataset_dir))
        
        # Get dataset statistics
        dataset_stats = self.dataset_manager.get_dataset_statistics()
        results["dataset_statistics"] = dataset_stats
        
        results["completed"] = datetime.now().isoformat()
        results["tensorflow_dataset_path"] = str(tf_dataset_dir)
        
        logger.info(f"Data collection completed: {dataset_stats['total_samples']} total samples")
        
        return results
    
    def train_model(self) -> Dict:
        """Train the wildlife classification model"""
        logger.info("Training wildlife classification model")
        
        training_config = self.config.get("model_training", {})
        
        # Get dataset path
        tf_dataset_path = self.dataset_dir / "tensorflow_format"
        if not tf_dataset_path.exists():
            raise ValueError("TensorFlow dataset not found. Run data collection first.")
        
        # Initialize trainer
        self.model_trainer = WildlifeModelTrainer(
            dataset_dir=str(tf_dataset_path),
            output_dir=str(self.models_dir),
            config=training_config
        )
        
        # Train model
        training_results = self.model_trainer.train_model()
        
        # Convert to TensorFlow Lite
        quantize = training_config.get("quantization", True)
        tflite_path = self.model_trainer.convert_to_tflite(quantize=quantize)
        
        # Benchmark TFLite model
        benchmark_results = self.model_trainer.benchmark_tflite_model(tflite_path)
        
        results = {
            "stage": "model_training",
            "training_results": training_results,
            "tflite_conversion": {
                "tflite_model_path": tflite_path,
                "quantized": quantize
            },
            "tflite_benchmark": benchmark_results
        }
        
        logger.info(f"Model training completed. TFLite model: {tflite_path}")
        
        return results
    
    def evaluate_model(self) -> Dict:
        """Evaluate the trained model"""
        logger.info("Evaluating trained model")
        
        evaluation_config = self.config.get("model_evaluation", {})
        
        # Find TFLite model
        tflite_models = list(self.models_dir.glob("*.tflite"))
        if not tflite_models:
            raise ValueError("No TFLite model found for evaluation")
        
        tflite_model_path = tflite_models[0]  # Use first found model
        
        # Get dataset path
        tf_dataset_path = self.dataset_dir / "tensorflow_format"
        
        # Initialize evaluator
        self.model_evaluator = WildlifeModelEvaluator(
            model_path=str(tflite_model_path),
            dataset_dir=str(tf_dataset_path),
            output_dir=str(self.evaluation_dir)
        )
        
        # Generate comprehensive evaluation report
        evaluation_results = self.model_evaluator.generate_comprehensive_report()
        
        # Check if model meets deployment criteria
        deployment_ready = self._check_deployment_criteria(evaluation_results)
        evaluation_results["deployment_ready"] = deployment_ready
        
        logger.info(f"Model evaluation completed. Deployment ready: {deployment_ready}")
        
        return evaluation_results
    
    def _check_deployment_criteria(self, evaluation_results: Dict) -> bool:
        """Check if model meets deployment criteria"""
        criteria = self.config.get("deployment_criteria", {})
        
        min_accuracy = criteria.get("min_accuracy", 0.85)
        max_inference_time = criteria.get("max_inference_time_ms", 2000)
        max_model_size_mb = criteria.get("max_model_size_mb", 15)
        
        summary = evaluation_results["summary"]
        
        checks = [
            summary["accuracy"] >= min_accuracy,
            summary["mean_inference_time_ms"] <= max_inference_time,
            summary["model_size_mb"] <= max_model_size_mb,
            summary["esp32_compatible"]
        ]
        
        return all(checks)
    
    def optimize_and_deploy(self) -> Dict:
        """Optimize model and prepare for deployment"""
        logger.info("Optimizing model and preparing deployment")
        
        deployment_config = self.config.get("deployment", {})
        
        # Find best TFLite model
        tflite_models = list(self.models_dir.glob("*.tflite"))
        if not tflite_models:
            raise ValueError("No TFLite model found for deployment")
        
        best_model = tflite_models[0]  # In a real implementation, select based on metrics
        
        results = {
            "stage": "deployment",
            "started": datetime.now().isoformat(),
            "selected_model": str(best_model)
        }
        
        # Copy model to deployment directory
        deployment_model_path = self.deployment_dir / "wildlife_classifier_v2.tflite"
        shutil.copy2(best_model, deployment_model_path)
        
        # Generate model metadata
        metadata = self._generate_model_metadata(best_model)
        
        with open(self.deployment_dir / "model_metadata.json", 'w') as f:
            json.dump(metadata, f, indent=2)
        
        # Create deployment package
        deployment_package = self._create_deployment_package()
        
        results.update({
            "deployment_model_path": str(deployment_model_path),
            "model_metadata": metadata,
            "deployment_package": deployment_package,
            "completed": datetime.now().isoformat()
        })
        
        logger.info(f"Deployment preparation completed: {deployment_model_path}")
        
        return results
    
    def _generate_model_metadata(self, model_path: Path) -> Dict:
        """Generate metadata for the deployed model"""
        model_size = model_path.stat().st_size
        
        metadata = {
            "model_name": "wildlife_classifier_v2.tflite",
            "version": "2.0.0",
            "created_date": datetime.now().isoformat(),
            "model_type": "species_classification",
            "file_size_bytes": model_size,
            "target_platform": "ESP32-S3",
            "optimization": {
                "quantization": "INT8",
                "optimized_for_edge": True
            },
            "supported_species": list(self.config.get("data_collection", {}).get("species_list", [])),
            "deployment_info": {
                "inference_framework": "TensorFlow Lite Micro",
                "memory_requirements": "2MB PSRAM",
                "expected_inference_time_ms": 1000
            }
        }
        
        return metadata
    
    def _create_deployment_package(self) -> Dict:
        """Create complete deployment package"""
        # Create deployment archive structure
        package_dir = self.deployment_dir / "esp32_deployment_package"
        package_dir.mkdir(exist_ok=True)
        
        # Copy necessary files
        files_to_copy = [
            (self.deployment_dir / "wildlife_classifier_v2.tflite", "models/"),
            (self.deployment_dir / "model_metadata.json", "models/"),
            (self.evaluation_dir / "comprehensive_evaluation_report.json", "validation/")
        ]
        
        for src_file, dest_dir in files_to_copy:
            if src_file.exists():
                dest_path = package_dir / dest_dir
                dest_path.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src_file, dest_path / src_file.name)
        
        # Create deployment README
        readme_content = self._generate_deployment_readme()
        with open(package_dir / "README.md", 'w') as f:
            f.write(readme_content)
        
        # Create installation script
        install_script = self._generate_install_script()
        with open(package_dir / "install.sh", 'w') as f:
            f.write(install_script)
        
        return {
            "package_directory": str(package_dir),
            "files_included": [f.name for f in package_dir.rglob("*") if f.is_file()],
            "size_bytes": sum(f.stat().st_size for f in package_dir.rglob("*") if f.is_file())
        }
    
    def _generate_deployment_readme(self) -> str:
        """Generate README for deployment package"""
        return """# Wildlife Classification Model - ESP32 Deployment Package

## Overview
This package contains a production-ready TensorFlow Lite model for wildlife classification optimized for ESP32-S3 deployment.

## Contents
- `models/wildlife_classifier_v2.tflite` - Quantized TensorFlow Lite model
- `models/model_metadata.json` - Model metadata and specifications
- `validation/comprehensive_evaluation_report.json` - Model evaluation results
- `install.sh` - Installation script for ESP32 deployment

## Model Specifications
- Input: 224x224x3 RGB images
- Output: Species classification probabilities
- Optimization: INT8 quantization
- Target Platform: ESP32-S3 with 8MB PSRAM

## Installation
1. Copy the model file to your ESP32 filesystem
2. Update your firmware to use the new model
3. Run validation tests to ensure proper operation

## Support
For issues and updates, visit: https://github.com/thewriterben/WildCAM_ESP32
"""
    
    def _generate_install_script(self) -> str:
        """Generate installation script"""
        return """#!/bin/bash
# Wildlife Classification Model Installation Script
# For ESP32-S3 deployment

echo "Installing wildlife classification model..."

# Check if model file exists
if [ ! -f "models/wildlife_classifier_v2.tflite" ]; then
    echo "Error: Model file not found"
    exit 1
fi

# Copy model to ESP32 filesystem (modify path as needed)
# cp models/wildlife_classifier_v2.tflite /path/to/esp32/filesystem/models/

echo "Model installation completed"
echo "Remember to update your firmware configuration to use the new model"
"""


def create_default_config() -> Dict:
    """Create default pipeline configuration"""
    return {
        "data_collection": {
            "enabled": True,
            "collect_from_inaturalist": True,
            "species_list": [
                "white_tailed_deer", "black_bear", "red_fox", "gray_wolf"
            ],
            "samples_per_species": 500,
            "preprocessing": {
                "image_size": [224, 224],
                "augmentation": True
            }
        },
        "model_training": {
            "enabled": True,
            "image_size": [224, 224],
            "batch_size": 32,
            "epochs": 30,
            "learning_rate": 0.001,
            "base_model": "MobileNetV3Large",
            "quantization": True
        },
        "model_evaluation": {
            "enabled": True,
            "evaluation_samples": 1000,
            "benchmark_iterations": 500
        },
        "deployment": {
            "enabled": True,
            "create_package": True
        },
        "deployment_criteria": {
            "min_accuracy": 0.85,
            "max_inference_time_ms": 2000,
            "max_model_size_mb": 15
        }
    }


if __name__ == "__main__":
    # Create default configuration
    config = create_default_config()
    config_path = "/tmp/pipeline_config.json"
    
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=2)
    
    # Run pipeline
    pipeline = WildlifeDeploymentPipeline(
        config_path=config_path,
        output_dir="/data/wildlife_deployment"
    )
    
    results = pipeline.run_complete_pipeline()
    
    print("Deployment pipeline completed!")
    print(f"Success: {results['success']}")
    if results['success']:
        print("ESP32-ready model available in deployment directory")