"""
IMX500 Wildlife Detector
Main detector module for on-sensor AI inference
"""

import logging
import time
from dataclasses import dataclass, field
from datetime import datetime
from typing import Optional, Callable, Dict, List, Tuple
import threading

logger = logging.getLogger(__name__)


@dataclass
class IMX500Detection:
    """Detection result from IMX500 sensor"""
    species: str
    confidence: float
    bbox: Tuple[float, float, float, float]  # (x, y, width, height) normalized 0-1
    timestamp: datetime = field(default_factory=datetime.now)
    inference_time_ms: float = 0.0
    
    def to_dict(self) -> Dict:
        """Convert detection to dictionary"""
        return {
            'species': self.species,
            'confidence': self.confidence,
            'bbox': self.bbox,
            'timestamp': self.timestamp.isoformat(),
            'inference_time_ms': self.inference_time_ms
        }
    
    def to_lora_payload(self) -> bytes:
        """
        Convert detection to compact LoRa payload
        
        Returns:
            10-byte binary payload
        """
        from ..utils.payload_encoder import encode_lora_payload
        from .species_mapping import get_species_id
        
        species_id = get_species_id(self.species)
        return encode_lora_payload(
            species_id=species_id,
            confidence=self.confidence,
            bbox=self.bbox,
            timestamp=self.timestamp
        )


class IMX500WildlifeDetector:
    """
    Wildlife detector using Sony IMX500 on-sensor AI
    Supports real-time inference at 30fps with <10ms latency
    """
    
    def __init__(self, config: Optional[Dict] = None):
        """
        Initialize IMX500 detector
        
        Args:
            config: Configuration dictionary with model path, thresholds, etc.
        """
        self.config = config or {}
        self.model_path = self.config.get('model_path', '/usr/share/imx500-models/wildlife_detector.rpk')
        self.confidence_threshold = self.config.get('confidence_threshold', 0.5)
        self.inference_fps = self.config.get('inference_fps', 30)
        
        # Camera and inference state
        self.camera = None
        self.running = False
        self.inference_thread = None
        
        # Callbacks
        self.detection_callbacks: List[Callable] = []
        
        # Statistics
        self.stats = {
            'total_inferences': 0,
            'total_detections': 0,
            'total_inference_time_ms': 0.0,
            'start_time': None,
            'last_detection_time': None,
        }
        
        logger.info(f"IMX500WildlifeDetector initialized with model: {self.model_path}")
    
    def initialize(self) -> bool:
        """
        Initialize camera and load AI model onto IMX500 sensor
        
        Returns:
            True if successful, False otherwise
        """
        try:
            # Import picamera2 (only available on Raspberry Pi)
            try:
                from picamera2 import Picamera2
                from picamera2.devices import IMX500
                from picamera2.devices.imx500 import NetworkIntrinsics
            except ImportError:
                logger.error("picamera2 not available - must run on Raspberry Pi with IMX500")
                logger.info("For development/testing, this module will simulate detection")
                return False
            
            # Create camera instance
            self.camera = Picamera2(IMX500.load_model(self.model_path))
            
            # Configure camera for inference
            config = self.camera.create_still_configuration(
                main={"size": (640, 640), "format": "RGB888"},
                buffer_count=2
            )
            self.camera.configure(config)
            
            # Get network intrinsics
            intrinsics = self.camera.camera_properties['IntrinsicsData']
            logger.info(f"Model loaded: {intrinsics}")
            
            # Start camera
            self.camera.start()
            
            logger.info("IMX500 camera initialized successfully")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize IMX500: {e}")
            return False
    
    def start(self):
        """Start inference loop"""
        if self.running:
            logger.warning("Detector already running")
            return
        
        self.running = True
        self.stats['start_time'] = datetime.now()
        
        # Start inference in separate thread
        self.inference_thread = threading.Thread(target=self._inference_loop)
        self.inference_thread.daemon = True
        self.inference_thread.start()
        
        logger.info("IMX500 detector started")
    
    def stop(self):
        """Stop inference loop"""
        if not self.running:
            return
        
        self.running = False
        
        if self.inference_thread:
            self.inference_thread.join(timeout=5.0)
        
        if self.camera:
            try:
                self.camera.stop()
            except Exception as e:
                logger.error(f"Error stopping camera: {e}")
        
        logger.info("IMX500 detector stopped")
    
    def _inference_loop(self):
        """Main inference loop (runs in separate thread)"""
        frame_interval = 1.0 / self.inference_fps
        
        while self.running:
            loop_start = time.time()
            
            try:
                # Capture and run inference
                detection = self._run_inference()
                
                if detection:
                    # Update statistics
                    self.stats['total_detections'] += 1
                    self.stats['last_detection_time'] = datetime.now()
                    
                    # Trigger callbacks
                    self._trigger_callbacks(detection)
                
                # Update inference count
                self.stats['total_inferences'] += 1
                
            except Exception as e:
                logger.error(f"Inference error: {e}")
            
            # Sleep to maintain target FPS
            elapsed = time.time() - loop_start
            sleep_time = max(0, frame_interval - elapsed)
            time.sleep(sleep_time)
    
    def _run_inference(self) -> Optional[IMX500Detection]:
        """
        Run single inference on current frame
        
        Returns:
            Detection object if detection meets threshold, None otherwise
        """
        if not self.camera:
            # Simulation mode for development
            return self._simulate_detection()
        
        try:
            inference_start = time.time()
            
            # Capture frame with metadata
            request = self.camera.capture_request()
            
            # Get inference results from metadata
            metadata = request.get_metadata()
            
            # Process detections from metadata
            # The actual format depends on the model's output structure
            # This is a simplified example
            if 'InferenceResults' in metadata:
                results = metadata['InferenceResults']
                
                # Find highest confidence detection
                best_detection = None
                best_confidence = 0.0
                
                for result in results:
                    confidence = result.get('confidence', 0.0)
                    if confidence > best_confidence and confidence >= self.confidence_threshold:
                        best_confidence = confidence
                        best_detection = result
                
                if best_detection:
                    inference_time = (time.time() - inference_start) * 1000
                    
                    detection = IMX500Detection(
                        species=best_detection.get('class', 'unknown'),
                        confidence=best_confidence,
                        bbox=best_detection.get('bbox', (0, 0, 0, 0)),
                        timestamp=datetime.now(),
                        inference_time_ms=inference_time
                    )
                    
                    self.stats['total_inference_time_ms'] += inference_time
                    
                    request.release()
                    return detection
            
            request.release()
            return None
            
        except Exception as e:
            logger.error(f"Inference failed: {e}")
            return None
    
    def _simulate_detection(self) -> Optional[IMX500Detection]:
        """
        Simulate detection for development/testing
        Returns random detections occasionally
        """
        import random
        
        # 10% chance of detection per inference
        if random.random() < 0.1:
            species_list = ['deer', 'elk', 'bear', 'wolf', 'fox', 'rabbit']
            species = random.choice(species_list)
            
            return IMX500Detection(
                species=species,
                confidence=random.uniform(0.5, 0.95),
                bbox=(
                    random.uniform(0.1, 0.5),
                    random.uniform(0.1, 0.5),
                    random.uniform(0.2, 0.4),
                    random.uniform(0.2, 0.4)
                ),
                timestamp=datetime.now(),
                inference_time_ms=random.uniform(5.0, 9.0)
            )
        
        return None
    
    def register_detection_callback(self, callback: Callable[[IMX500Detection], None]):
        """
        Register callback function for detections
        
        Args:
            callback: Function to call when detection occurs
        """
        self.detection_callbacks.append(callback)
        logger.debug(f"Registered detection callback: {callback.__name__}")
    
    def _trigger_callbacks(self, detection: IMX500Detection):
        """Trigger all registered callbacks"""
        for callback in self.detection_callbacks:
            try:
                callback(detection)
            except Exception as e:
                logger.error(f"Callback error: {e}")
    
    def capture_high_res(self, save_path: Optional[str] = None) -> Optional[bytes]:
        """
        Capture high-resolution image (12.3MP)
        
        Args:
            save_path: Optional path to save image
            
        Returns:
            Image bytes if successful, None otherwise
        """
        if not self.camera:
            logger.warning("Camera not initialized, cannot capture high-res")
            return None
        
        try:
            # Temporarily switch to high-res configuration
            from picamera2 import Picamera2
            
            # Create high-res config (12.3MP = 4056x3040 for IMX500)
            config = self.camera.create_still_configuration(
                main={"size": (4056, 3040), "format": "RGB888"}
            )
            
            # Switch config, capture, then restore
            original_config = self.camera.camera_configuration()
            
            self.camera.stop()
            self.camera.configure(config)
            self.camera.start()
            
            # Capture
            image_data = self.camera.capture_array()
            
            # Restore original config
            self.camera.stop()
            self.camera.configure(original_config)
            self.camera.start()
            
            # Convert to JPEG if save path provided
            if save_path:
                from PIL import Image
                import numpy as np
                
                img = Image.fromarray(image_data.astype('uint8'), 'RGB')
                img.save(save_path, 'JPEG', quality=95)
                logger.info(f"High-res image saved to {save_path}")
            
            # Return as bytes
            from io import BytesIO
            from PIL import Image
            import numpy as np
            
            img = Image.fromarray(image_data.astype('uint8'), 'RGB')
            buffer = BytesIO()
            img.save(buffer, format='JPEG', quality=95)
            return buffer.getvalue()
            
        except Exception as e:
            logger.error(f"High-res capture failed: {e}")
            return None
    
    def get_stats(self) -> Dict:
        """
        Get detector statistics
        
        Returns:
            Dictionary with performance metrics
        """
        stats = self.stats.copy()
        
        # Calculate derived metrics
        if stats['total_inferences'] > 0:
            stats['detection_rate'] = stats['total_detections'] / stats['total_inferences']
            stats['avg_inference_time_ms'] = (
                stats['total_inference_time_ms'] / stats['total_inferences']
            )
        else:
            stats['detection_rate'] = 0.0
            stats['avg_inference_time_ms'] = 0.0
        
        # Calculate uptime
        if stats['start_time']:
            uptime = datetime.now() - stats['start_time']
            stats['uptime_seconds'] = uptime.total_seconds()
        else:
            stats['uptime_seconds'] = 0.0
        
        return stats
    
    def deploy_custom_model(self, onnx_path: str) -> bool:
        """
        Deploy custom ONNX model to IMX500
        
        Args:
            onnx_path: Path to ONNX model file
            
        Returns:
            True if successful, False otherwise
        """
        from ..utils.model_converter import convert_onnx_to_rpk
        import os
        
        try:
            # Convert ONNX to RPK
            rpk_path = onnx_path.replace('.onnx', '.rpk')
            
            logger.info(f"Converting {onnx_path} to RPK format...")
            success = convert_onnx_to_rpk(onnx_path, rpk_path)
            
            if not success:
                logger.error("Model conversion failed")
                return False
            
            # Update model path
            self.model_path = rpk_path
            
            # Reinitialize camera with new model
            if self.camera:
                self.stop()
            
            success = self.initialize()
            
            if success:
                logger.info(f"Custom model deployed successfully: {rpk_path}")
                # Restart if was running
                if self.running:
                    self.start()
            
            return success
            
        except Exception as e:
            logger.error(f"Model deployment failed: {e}")
            return False
