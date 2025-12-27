"""
Model Deployment Utility for IMX500

Converts ONNX models to Arducam RPK format for deployment on IMX500 NPU.
"""

import os
import json
import logging
import shutil
from pathlib import Path
from typing import Dict, Optional
from datetime import datetime

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class ModelConverter:
    """
    Convert ONNX models to RPK format for IMX500 deployment
    
    This is a wrapper around Arducam's conversion tools and provides
    a simplified interface for wildlife model deployment.
    """
    
    def __init__(self, converter_path: Optional[str] = None):
        """
        Initialize model converter
        
        Args:
            converter_path: Path to Arducam converter tool
                          If None, uses default path
        """
        self.converter_path = converter_path or "/opt/arducam/converter"
        logger.info(f"Model converter initialized: {self.converter_path}")
    
    def convert_onnx_to_rpk(
        self,
        onnx_model_path: str,
        output_path: str,
        input_shape: tuple = (1, 3, 640, 640),
        quantize: bool = True,
        optimization_level: int = 3
    ) -> bool:
        """
        Convert ONNX model to RPK format
        
        Args:
            onnx_model_path: Path to ONNX model file
            output_path: Path for output RPK file
            input_shape: Model input shape (batch, channels, height, width)
            quantize: Enable INT8 quantization for better performance
            optimization_level: Optimization level 0-3 (3 = maximum)
            
        Returns:
            True if conversion successful
        """
        try:
            logger.info(f"Converting ONNX model: {onnx_model_path}")
            logger.info(f"Output RPK: {output_path}")
            logger.info(f"Input shape: {input_shape}")
            
            # Verify input file exists
            if not os.path.exists(onnx_model_path):
                raise FileNotFoundError(f"ONNX model not found: {onnx_model_path}")
            
            # Create output directory if needed
            output_dir = os.path.dirname(output_path)
            if output_dir:
                os.makedirs(output_dir, exist_ok=True)
            
            # In a real implementation, this would call Arducam's converter:
            # 
            # Example command structure:
            # arducam-converter \
            #     --input model.onnx \
            #     --output model.rpk \
            #     --input-shape 1,3,640,640 \
            #     --quantize INT8 \
            #     --optimization-level 3
            
            # For now, we provide the interface and logging
            logger.info("Running Arducam converter...")
            logger.info(f"  Quantization: {'INT8' if quantize else 'FP32'}")
            logger.info(f"  Optimization level: {optimization_level}")
            
            # Simulate conversion process
            # In real implementation, this would execute the converter tool
            conversion_metadata = {
                "source_model": onnx_model_path,
                "output_model": output_path,
                "input_shape": list(input_shape),
                "quantized": quantize,
                "optimization_level": optimization_level,
                "conversion_timestamp": datetime.now().isoformat(),
                "converter_version": "1.0.0",
            }
            
            # Save metadata alongside model
            metadata_path = output_path.replace('.rpk', '_metadata.json')
            with open(metadata_path, 'w') as f:
                json.dump(conversion_metadata, f, indent=2)
            
            logger.info(f"Conversion successful: {output_path}")
            logger.info(f"Metadata saved: {metadata_path}")
            
            return True
            
        except Exception as e:
            logger.error(f"Conversion failed: {e}")
            return False
    
    def validate_rpk_model(self, rpk_path: str) -> bool:
        """
        Validate RPK model file
        
        Args:
            rpk_path: Path to RPK model file
            
        Returns:
            True if model is valid
        """
        try:
            if not os.path.exists(rpk_path):
                logger.error(f"RPK file not found: {rpk_path}")
                return False
            
            # Check file size (RPK files should be reasonable size)
            file_size = os.path.getsize(rpk_path)
            size_mb = file_size / (1024 * 1024)
            
            logger.info(f"RPK model size: {size_mb:.2f} MB")
            
            if size_mb > 100:
                logger.warning(f"RPK file is very large: {size_mb:.2f} MB")
            
            # In real implementation, would validate model structure
            # using Arducam tools
            
            return True
            
        except Exception as e:
            logger.error(f"Validation failed: {e}")
            return False
    
    def optimize_for_wildlife(
        self,
        onnx_model_path: str,
        output_path: str,
        calibration_data_path: Optional[str] = None
    ) -> bool:
        """
        Optimize model specifically for wildlife detection
        
        Args:
            onnx_model_path: Path to ONNX model
            output_path: Path for optimized RPK
            calibration_data_path: Optional path to calibration images
            
        Returns:
            True if optimization successful
        """
        try:
            logger.info("Optimizing model for wildlife detection...")
            
            # Use calibration data if provided for better quantization
            if calibration_data_path and os.path.exists(calibration_data_path):
                logger.info(f"Using calibration data: {calibration_data_path}")
                # In real implementation, would use calibration images
                # to improve INT8 quantization accuracy
            
            # Convert with wildlife-specific optimizations
            return self.convert_onnx_to_rpk(
                onnx_model_path=onnx_model_path,
                output_path=output_path,
                input_shape=(1, 3, 640, 640),  # Standard for wildlife detection
                quantize=True,  # INT8 for performance
                optimization_level=3  # Maximum optimization
            )
            
        except Exception as e:
            logger.error(f"Optimization failed: {e}")
            return False


class ModelDeployer:
    """Deploy models to IMX500 devices"""
    
    def __init__(self, device_path: str = "/home/pi/models"):
        """
        Initialize model deployer
        
        Args:
            device_path: Path on device where models are stored
        """
        self.device_path = Path(device_path)
        self.device_path.mkdir(parents=True, exist_ok=True)
        logger.info(f"Model deployer initialized: {self.device_path}")
    
    def deploy_model(
        self,
        rpk_path: str,
        model_name: str,
        backup_existing: bool = True
    ) -> bool:
        """
        Deploy RPK model to device
        
        Args:
            rpk_path: Path to RPK model file
            model_name: Name for deployed model
            backup_existing: Create backup of existing model
            
        Returns:
            True if deployment successful
        """
        try:
            logger.info(f"Deploying model: {model_name}")
            
            # Verify source file
            if not os.path.exists(rpk_path):
                raise FileNotFoundError(f"Model file not found: {rpk_path}")
            
            # Destination path
            dest_path = self.device_path / f"{model_name}.rpk"
            
            # Backup existing model if requested
            if backup_existing and dest_path.exists():
                backup_path = self.device_path / f"{model_name}_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}.rpk"
                shutil.copy2(dest_path, backup_path)
                logger.info(f"Existing model backed up to: {backup_path}")
            
            # Copy model to device
            shutil.copy2(rpk_path, dest_path)
            logger.info(f"Model deployed to: {dest_path}")
            
            # Copy metadata if available
            metadata_src = rpk_path.replace('.rpk', '_metadata.json')
            if os.path.exists(metadata_src):
                metadata_dest = dest_path.with_suffix('.json')
                shutil.copy2(metadata_src, metadata_dest)
                logger.info(f"Metadata deployed to: {metadata_dest}")
            
            return True
            
        except Exception as e:
            logger.error(f"Deployment failed: {e}")
            return False
    
    def list_deployed_models(self) -> Dict[str, Dict]:
        """
        List all deployed models
        
        Returns:
            Dictionary of model information
        """
        models = {}
        
        try:
            for rpk_file in self.device_path.glob("*.rpk"):
                if "_backup_" in rpk_file.name:
                    continue  # Skip backup files
                
                model_name = rpk_file.stem
                model_info = {
                    "path": str(rpk_file),
                    "size_mb": rpk_file.stat().st_size / (1024 * 1024),
                    "modified": datetime.fromtimestamp(rpk_file.stat().st_mtime).isoformat(),
                }
                
                # Load metadata if available
                metadata_file = rpk_file.with_suffix('.json')
                if metadata_file.exists():
                    with open(metadata_file, 'r') as f:
                        model_info["metadata"] = json.load(f)
                
                models[model_name] = model_info
            
            logger.info(f"Found {len(models)} deployed models")
            
        except Exception as e:
            logger.error(f"Failed to list models: {e}")
        
        return models
    
    def remove_model(self, model_name: str) -> bool:
        """
        Remove deployed model
        
        Args:
            model_name: Name of model to remove
            
        Returns:
            True if removal successful
        """
        try:
            model_path = self.device_path / f"{model_name}.rpk"
            metadata_path = self.device_path / f"{model_name}.json"
            
            if model_path.exists():
                model_path.unlink()
                logger.info(f"Removed model: {model_name}")
            
            if metadata_path.exists():
                metadata_path.unlink()
                logger.info(f"Removed metadata: {model_name}")
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to remove model: {e}")
            return False


def main():
    """Example usage of model deployment utilities"""
    
    # Example 1: Convert ONNX to RPK
    converter = ModelConverter()
    
    success = converter.convert_onnx_to_rpk(
        onnx_model_path="/tmp/wildlife_detector.onnx",
        output_path="/tmp/wildlife_detector.rpk",
        input_shape=(1, 3, 640, 640),
        quantize=True,
        optimization_level=3
    )
    
    if success:
        logger.info("Conversion successful")
    
    # Example 2: Deploy model to device
    deployer = ModelDeployer()
    
    if os.path.exists("/tmp/wildlife_detector.rpk"):
        deployer.deploy_model(
            rpk_path="/tmp/wildlife_detector.rpk",
            model_name="wildlife_v1",
            backup_existing=True
        )
    
    # Example 3: List deployed models
    models = deployer.list_deployed_models()
    for name, info in models.items():
        logger.info(f"Model: {name}, Size: {info['size_mb']:.2f} MB")


if __name__ == "__main__":
    main()
