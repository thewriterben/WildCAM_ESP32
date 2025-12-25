"""
ONNX to RPK Model Converter
Wrapper for Arducam imx500-model-converter tool
"""

import os
import subprocess
import logging
from pathlib import Path
from typing import Optional, Dict

logger = logging.getLogger(__name__)


class ModelConverter:
    """
    Converts ONNX models to IMX500 RPK format
    Uses Arducam's imx500-model-converter tool
    """
    
    def __init__(self, converter_path: Optional[str] = None):
        """
        Initialize model converter
        
        Args:
            converter_path: Path to imx500-converter executable
                          If None, assumes it's in PATH
        """
        self.converter_path = converter_path or "imx500-converter"
        
    def check_converter_available(self) -> bool:
        """
        Check if the converter tool is available
        
        Returns:
            True if converter is available, False otherwise
        """
        try:
            result = subprocess.run(
                [self.converter_path, "--version"],
                capture_output=True,
                timeout=5
            )
            return result.returncode == 0
        except Exception as e:
            logger.warning(f"Converter not found: {e}")
            return False
    
    def convert_onnx_to_rpk(
        self,
        onnx_path: str,
        output_path: Optional[str] = None,
        quantization: str = "int8",
        input_shape: tuple = (1, 3, 640, 640),
        calibration_data: Optional[str] = None,
        optimization_level: int = 2
    ) -> bool:
        """
        Convert ONNX model to IMX500 RPK format
        
        Args:
            onnx_path: Path to input ONNX model file
            output_path: Path for output RPK file (default: same as input with .rpk)
            quantization: Quantization type ("int8", "fp16", "mixed")
            input_shape: Model input shape (batch, channels, height, width)
            calibration_data: Path to calibration dataset for quantization
            optimization_level: Optimization level 0-3 (higher = more aggressive)
            
        Returns:
            True if conversion successful, False otherwise
        """
        # Validate input file exists
        if not os.path.exists(onnx_path):
            logger.error(f"ONNX file not found: {onnx_path}")
            return False
        
        # Determine output path
        if output_path is None:
            output_path = str(Path(onnx_path).with_suffix('.rpk'))
        
        # Check converter availability
        if not self.check_converter_available():
            logger.error("IMX500 converter tool not available")
            logger.info("Install from: https://docs.arducam.com/AI-Camera-Solutions/")
            return False
        
        # Build conversion command
        cmd = [
            self.converter_path,
            "--input", onnx_path,
            "--output", output_path,
            "--quantization", quantization,
            "--input-shape", f"{input_shape[0]},{input_shape[1]},{input_shape[2]},{input_shape[3]}",
            "--optimization-level", str(optimization_level),
        ]
        
        # Add calibration data if provided
        if calibration_data and os.path.exists(calibration_data):
            cmd.extend(["--calibration-data", calibration_data])
        
        logger.info(f"Converting {onnx_path} to {output_path}")
        logger.debug(f"Command: {' '.join(cmd)}")
        
        try:
            # Run conversion
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            
            if result.returncode == 0:
                logger.info(f"Conversion successful: {output_path}")
                logger.debug(f"Converter output: {result.stdout}")
                return True
            else:
                logger.error(f"Conversion failed: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            logger.error("Conversion timed out after 5 minutes")
            return False
        except Exception as e:
            logger.error(f"Conversion error: {e}")
            return False
    
    def validate_rpk(self, rpk_path: str) -> Dict:
        """
        Validate an RPK model file
        
        Args:
            rpk_path: Path to RPK file
            
        Returns:
            Dictionary with validation results
        """
        if not os.path.exists(rpk_path):
            return {
                'valid': False,
                'error': 'File not found'
            }
        
        file_size = os.path.getsize(rpk_path)
        
        # Basic validation - RPK files should be reasonable size
        if file_size < 1024:  # Less than 1KB is suspicious
            return {
                'valid': False,
                'error': 'File too small',
                'size_bytes': file_size
            }
        
        if file_size > 100 * 1024 * 1024:  # More than 100MB is suspicious
            return {
                'valid': False,
                'error': 'File too large',
                'size_bytes': file_size
            }
        
        return {
            'valid': True,
            'size_bytes': file_size,
            'size_mb': file_size / (1024 * 1024)
        }


def convert_onnx_to_rpk(
    onnx_path: str,
    output_path: Optional[str] = None,
    **kwargs
) -> bool:
    """
    Convenience function to convert ONNX to RPK
    
    Args:
        onnx_path: Path to ONNX model
        output_path: Output RPK path (optional)
        **kwargs: Additional arguments for conversion
        
    Returns:
        True if successful, False otherwise
    """
    converter = ModelConverter()
    return converter.convert_onnx_to_rpk(onnx_path, output_path, **kwargs)
