"""
Multi-Stream Camera Processing Service
Handles simultaneous video streams from multiple sources (USB, RTSP, IP cameras)
"""

import cv2
import numpy as np
from typing import Dict, List, Optional, Callable
import threading
import queue
import logging
import time
from pathlib import Path
from dataclasses import dataclass
from enum import Enum

logger = logging.getLogger(__name__)


class CameraType(Enum):
    """Camera source types"""
    USB = "usb"
    RTSP = "rtsp"
    IP = "ip"
    FILE = "file"


@dataclass
class CameraConfig:
    """Camera configuration"""
    name: str
    source: str
    type: CameraType
    fps: int = 30
    resolution: tuple = (1920, 1080)
    enabled: bool = True


@dataclass
class DetectionResult:
    """Detection result from a camera frame"""
    camera_name: str
    timestamp: float
    frame_id: int
    detections: List[Dict]
    frame: Optional[np.ndarray] = None
    metadata: Optional[Dict] = None


class CameraStream:
    """
    Individual camera stream handler with buffering and error recovery
    """
    
    def __init__(
        self,
        config: CameraConfig,
        detector_callback: Optional[Callable] = None,
        max_queue_size: int = 30
    ):
        """
        Initialize camera stream
        
        Args:
            config: Camera configuration
            detector_callback: Callback function for detection
            max_queue_size: Maximum frame queue size
        """
        self.config = config
        self.detector_callback = detector_callback
        self.max_queue_size = max_queue_size
        
        self.capture = None
        self.running = False
        self.thread = None
        self.frame_queue = queue.Queue(maxsize=max_queue_size)
        
        # Statistics
        self.frame_count = 0
        self.dropped_frames = 0
        self.error_count = 0
        self.last_frame_time = 0
        
        # Reconnection settings
        self.reconnect_attempts = 0
        self.max_reconnect_attempts = 5
        self.reconnect_delay = 5.0
        
    def start(self):
        """Start camera stream processing"""
        if self.running:
            logger.warning(f"Camera {self.config.name} already running")
            return
        
        logger.info(f"Starting camera stream: {self.config.name}")
        self.running = True
        self.thread = threading.Thread(target=self._capture_loop, daemon=True)
        self.thread.start()
        
    def stop(self):
        """Stop camera stream processing"""
        logger.info(f"Stopping camera stream: {self.config.name}")
        self.running = False
        
        if self.thread:
            self.thread.join(timeout=5.0)
        
        if self.capture:
            self.capture.release()
        
        # Clear queue
        while not self.frame_queue.empty():
            try:
                self.frame_queue.get_nowait()
            except queue.Empty:
                break
    
    def _connect(self) -> bool:
        """
        Connect to camera source
        
        Returns:
            True if successful, False otherwise
        """
        try:
            if self.config.type == CameraType.USB:
                # USB camera (e.g., /dev/video0 or 0)
                source = int(self.config.source.split('video')[-1]) \
                    if 'video' in self.config.source else int(self.config.source)
                self.capture = cv2.VideoCapture(source)
            elif self.config.type in [CameraType.RTSP, CameraType.IP]:
                # Network camera
                self.capture = cv2.VideoCapture(self.config.source)
            elif self.config.type == CameraType.FILE:
                # Video file
                self.capture = cv2.VideoCapture(self.config.source)
            else:
                logger.error(f"Unsupported camera type: {self.config.type}")
                return False
            
            if not self.capture.isOpened():
                logger.error(f"Failed to open camera: {self.config.name}")
                return False
            
            # Set camera properties
            self.capture.set(cv2.CAP_PROP_FRAME_WIDTH, self.config.resolution[0])
            self.capture.set(cv2.CAP_PROP_FRAME_HEIGHT, self.config.resolution[1])
            self.capture.set(cv2.CAP_PROP_FPS, self.config.fps)
            
            # Verify settings
            actual_width = self.capture.get(cv2.CAP_PROP_FRAME_WIDTH)
            actual_height = self.capture.get(cv2.CAP_PROP_FRAME_HEIGHT)
            actual_fps = self.capture.get(cv2.CAP_PROP_FPS)
            
            logger.info(
                f"Camera {self.config.name} connected: "
                f"{actual_width}x{actual_height} @ {actual_fps}fps"
            )
            
            self.reconnect_attempts = 0
            return True
            
        except Exception as e:
            logger.error(f"Error connecting to camera {self.config.name}: {e}")
            return False
    
    def _capture_loop(self):
        """Main capture loop running in separate thread"""
        while self.running:
            # Connect if not connected
            if not self.capture or not self.capture.isOpened():
                if not self._connect():
                    self.reconnect_attempts += 1
                    if self.reconnect_attempts >= self.max_reconnect_attempts:
                        logger.error(
                            f"Max reconnection attempts reached for {self.config.name}"
                        )
                        self.running = False
                        break
                    
                    logger.info(
                        f"Retrying connection to {self.config.name} "
                        f"(attempt {self.reconnect_attempts}/{self.max_reconnect_attempts})"
                    )
                    time.sleep(self.reconnect_delay)
                    continue
            
            try:
                # Read frame
                ret, frame = self.capture.read()
                
                if not ret:
                    logger.warning(f"Failed to read frame from {self.config.name}")
                    self.error_count += 1
                    
                    # Release and reconnect
                    if self.capture:
                        self.capture.release()
                    time.sleep(1.0)
                    continue
                
                # Update statistics
                self.frame_count += 1
                current_time = time.time()
                
                # Try to add frame to queue (non-blocking)
                try:
                    self.frame_queue.put_nowait({
                        'frame': frame,
                        'timestamp': current_time,
                        'frame_id': self.frame_count
                    })
                    self.last_frame_time = current_time
                except queue.Full:
                    self.dropped_frames += 1
                    logger.debug(f"Frame queue full for {self.config.name}, dropping frame")
                
            except Exception as e:
                logger.error(f"Error in capture loop for {self.config.name}: {e}")
                self.error_count += 1
                time.sleep(1.0)
    
    def get_frame(self, timeout: float = 1.0) -> Optional[Dict]:
        """
        Get next frame from queue
        
        Args:
            timeout: Maximum wait time in seconds
            
        Returns:
            Frame dictionary or None if timeout
        """
        try:
            return self.frame_queue.get(timeout=timeout)
        except queue.Empty:
            return None
    
    def get_stats(self) -> Dict:
        """Get camera statistics"""
        return {
            'name': self.config.name,
            'type': self.config.type.value,
            'running': self.running,
            'frame_count': self.frame_count,
            'dropped_frames': self.dropped_frames,
            'error_count': self.error_count,
            'queue_size': self.frame_queue.qsize(),
            'fps': self._calculate_fps()
        }
    
    def _calculate_fps(self) -> float:
        """Calculate actual FPS"""
        # Track start time for FPS calculation
        if not hasattr(self, 'start_time'):
            self.start_time = time.time()
        
        if self.frame_count < 2:
            return 0.0
        
        elapsed = time.time() - self.start_time
        if elapsed > 0:
            return self.frame_count / elapsed
        return 0.0


class MultiStreamProcessor:
    """
    Manages multiple camera streams and coordinates detection
    """
    
    def __init__(
        self,
        detector,
        save_dir: Optional[Path] = None,
        record_detections: bool = True
    ):
        """
        Initialize multi-stream processor
        
        Args:
            detector: YOLO detector instance
            save_dir: Directory to save detection images
            record_detections: Whether to save detection images
        """
        self.detector = detector
        self.save_dir = Path(save_dir) if save_dir else Path("detections")
        self.record_detections = record_detections
        
        self.cameras: Dict[str, CameraStream] = {}
        self.running = False
        self.processing_threads: List[threading.Thread] = []
        
        # Detection results queue
        self.detection_queue = queue.Queue()
        
        # Create save directory
        if self.record_detections:
            self.save_dir.mkdir(parents=True, exist_ok=True)
        
        # Statistics
        self.total_detections = 0
        
    def add_camera(self, config: CameraConfig):
        """
        Add camera to processing pool
        
        Args:
            config: Camera configuration
        """
        if config.name in self.cameras:
            logger.warning(f"Camera {config.name} already exists, replacing")
            self.remove_camera(config.name)
        
        stream = CameraStream(config, detector_callback=self._process_detection)
        self.cameras[config.name] = stream
        
        logger.info(f"Added camera: {config.name}")
        
        # Start if processor is running
        if self.running:
            stream.start()
    
    def remove_camera(self, camera_name: str):
        """Remove camera from processing pool"""
        if camera_name not in self.cameras:
            logger.warning(f"Camera {camera_name} not found")
            return
        
        stream = self.cameras[camera_name]
        stream.stop()
        del self.cameras[camera_name]
        
        logger.info(f"Removed camera: {camera_name}")
    
    def start(self):
        """Start processing all cameras"""
        if self.running:
            logger.warning("Multi-stream processor already running")
            return
        
        logger.info(f"Starting multi-stream processor with {len(self.cameras)} cameras")
        self.running = True
        
        # Start all camera streams
        for stream in self.cameras.values():
            if stream.config.enabled:
                stream.start()
        
        # Start processing threads (one per camera)
        for camera_name, stream in self.cameras.items():
            thread = threading.Thread(
                target=self._processing_loop,
                args=(camera_name, stream),
                daemon=True
            )
            thread.start()
            self.processing_threads.append(thread)
        
        logger.info("Multi-stream processor started")
    
    def stop(self):
        """Stop processing all cameras"""
        logger.info("Stopping multi-stream processor")
        self.running = False
        
        # Stop all camera streams
        for stream in self.cameras.values():
            stream.stop()
        
        # Wait for processing threads
        for thread in self.processing_threads:
            thread.join(timeout=5.0)
        
        self.processing_threads.clear()
        
        logger.info("Multi-stream processor stopped")
    
    def _processing_loop(self, camera_name: str, stream: CameraStream):
        """
        Processing loop for individual camera
        
        Args:
            camera_name: Camera identifier
            stream: Camera stream instance
        """
        logger.info(f"Started processing loop for camera: {camera_name}")
        
        while self.running:
            # Get frame from camera queue
            frame_data = stream.get_frame(timeout=1.0)
            
            if frame_data is None:
                continue
            
            try:
                # Run detection
                detections = self.detector.detect(frame_data['frame'])
                
                # Create detection result
                result = DetectionResult(
                    camera_name=camera_name,
                    timestamp=frame_data['timestamp'],
                    frame_id=frame_data['frame_id'],
                    detections=detections,
                    frame=frame_data['frame'] if self.record_detections else None
                )
                
                # Add to detection queue
                self.detection_queue.put(result)
                
                # Update statistics
                self.total_detections += len(detections)
                
                # Save detection image if enabled
                if self.record_detections and len(detections) > 0:
                    self._save_detection(result)
                
                logger.debug(
                    f"Camera {camera_name}: "
                    f"Frame {frame_data['frame_id']}, "
                    f"{len(detections)} detections"
                )
                
            except Exception as e:
                logger.error(f"Error processing frame from {camera_name}: {e}")
        
        logger.info(f"Stopped processing loop for camera: {camera_name}")
    
    def _process_detection(self, result: DetectionResult):
        """Process detection result (callback for custom handling)"""
        # This can be extended for custom processing
        pass
    
    def _save_detection(self, result: DetectionResult):
        """Save detection image with bounding boxes"""
        if result.frame is None:
            return
        
        # Draw bounding boxes
        frame_annotated = result.frame.copy()
        
        for det in result.detections:
            x1, y1, x2, y2 = det['bbox']
            label = f"{det['class']}: {det['confidence']:.2f}"
            
            # Draw box
            cv2.rectangle(frame_annotated, (x1, y1), (x2, y2), (0, 255, 0), 2)
            
            # Draw label background
            (label_w, label_h), _ = cv2.getTextSize(
                label, cv2.FONT_HERSHEY_SIMPLEX, 0.6, 1
            )
            cv2.rectangle(
                frame_annotated,
                (x1, y1 - label_h - 10),
                (x1 + label_w, y1),
                (0, 255, 0),
                -1
            )
            
            # Draw label text
            cv2.putText(
                frame_annotated,
                label,
                (x1, y1 - 5),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (0, 0, 0),
                1
            )
        
        # Save image
        timestamp_str = time.strftime("%Y%m%d_%H%M%S", time.localtime(result.timestamp))
        filename = f"{result.camera_name}_{timestamp_str}_frame{result.frame_id}.jpg"
        filepath = self.save_dir / filename
        
        cv2.imwrite(str(filepath), frame_annotated)
        logger.info(f"Saved detection image: {filepath}")
    
    def get_detection(self, timeout: float = 1.0) -> Optional[DetectionResult]:
        """
        Get next detection result from queue
        
        Args:
            timeout: Maximum wait time in seconds
            
        Returns:
            DetectionResult or None if timeout
        """
        try:
            return self.detection_queue.get(timeout=timeout)
        except queue.Empty:
            return None
    
    def get_stats(self) -> Dict:
        """Get processor statistics"""
        camera_stats = {
            name: stream.get_stats()
            for name, stream in self.cameras.items()
        }
        
        detector_stats = self.detector.get_stats()
        
        return {
            'running': self.running,
            'num_cameras': len(self.cameras),
            'total_detections': self.total_detections,
            'cameras': camera_stats,
            'detector': detector_stats
        }
    
    def __del__(self):
        """Cleanup on deletion"""
        self.stop()
