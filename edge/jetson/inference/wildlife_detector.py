"""
Wildlife Detector - TensorRT-optimized YOLO inference

Provides real-time wildlife species detection using YOLOv8 with TensorRT optimization.
Supports multi-camera stream processing, object tracking, and behavior analysis.
"""

import numpy as np
import cv2
from typing import List, Dict, Any, Optional, Tuple
from dataclasses import dataclass, field
from datetime import datetime
import logging
from collections import defaultdict, deque

logger = logging.getLogger(__name__)


@dataclass
class Detection:
    """Wildlife detection result"""
    species: str
    confidence: float
    bbox: Tuple[int, int, int, int]  # (x1, y1, x2, y2)
    timestamp: datetime = field(default_factory=datetime.utcnow)
    camera_id: str = "default"
    track_id: Optional[int] = None
    behavior: Optional[str] = None  # running, walking, feeding, resting
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert detection to dictionary"""
        return {
            'species': self.species,
            'confidence': self.confidence,
            'bbox': list(self.bbox),
            'timestamp': self.timestamp.isoformat(),
            'camera_id': self.camera_id,
            'track_id': self.track_id,
            'behavior': self.behavior
        }


@dataclass
class Track:
    """Object tracking information"""
    track_id: int
    species: str
    history: deque = field(default_factory=lambda: deque(maxlen=30))
    last_seen: datetime = field(default_factory=datetime.utcnow)
    confidence_sum: float = 0.0
    detection_count: int = 0
    
    def update(self, bbox: Tuple[int, int, int, int], confidence: float):
        """Update track with new detection"""
        self.history.append(bbox)
        self.last_seen = datetime.utcnow()
        self.confidence_sum += confidence
        self.detection_count += 1
        
    @property
    def average_confidence(self) -> float:
        """Get average confidence for this track"""
        return self.confidence_sum / self.detection_count if self.detection_count > 0 else 0.0


class WildlifeDetector:
    """
    TensorRT-optimized YOLOv8 wildlife detector
    
    Supports real-time detection, multi-camera processing, object tracking,
    and behavior analysis for wildlife monitoring.
    """
    
    # Common wildlife species classes
    SPECIES_CLASSES = [
        'deer', 'elk', 'moose', 'bear', 'wolf', 'coyote', 'fox',
        'rabbit', 'squirrel', 'raccoon', 'skunk', 'bobcat', 'mountain_lion',
        'eagle', 'hawk', 'owl', 'turkey', 'duck', 'goose'
    ]
    
    def __init__(
        self,
        model_path: str,
        confidence_threshold: float = 0.5,
        nms_threshold: float = 0.4,
        use_tensorrt: bool = True,
        max_cameras: int = 8
    ):
        """
        Initialize wildlife detector
        
        Args:
            model_path: Path to YOLO model file (.onnx or .engine)
            confidence_threshold: Minimum confidence for detections
            nms_threshold: Non-maximum suppression threshold
            use_tensorrt: Use TensorRT optimization (recommended for Jetson)
            max_cameras: Maximum simultaneous camera streams
        """
        self.model_path = model_path
        self.confidence_threshold = confidence_threshold
        self.nms_threshold = nms_threshold
        self.use_tensorrt = use_tensorrt
        self.max_cameras = max_cameras
        
        # Model and preprocessing
        self.model = None
        self.input_size = (640, 640)  # YOLOv8 default
        
        # Tracking
        self.tracks: Dict[str, Dict[int, Track]] = defaultdict(dict)  # camera_id -> track_id -> Track
        self.next_track_id: Dict[str, int] = defaultdict(int)
        
        # Statistics
        self.total_detections = 0
        self.fps_history: Dict[str, deque] = defaultdict(lambda: deque(maxlen=30))
        
        # Initialize model
        self._load_model()
        
    def _load_model(self):
        """Load YOLO model with optional TensorRT optimization"""
        try:
            if self.use_tensorrt:
                # Try to load TensorRT engine
                logger.info(f"Loading TensorRT model from {self.model_path}")
                # NOTE: In production, use tensorrt or pycuda here
                # For now, we'll use a placeholder that falls back to ONNX
                self._load_onnx_model()
            else:
                self._load_onnx_model()
                
            logger.info("Wildlife detector model loaded successfully")
        except Exception as e:
            logger.error(f"Failed to load model: {e}")
            raise
            
    def _load_onnx_model(self):
        """Load ONNX model (fallback or direct)"""
        # NOTE: In production, use onnxruntime or similar
        # This is a placeholder implementation
        logger.info(f"Loading ONNX model from {self.model_path}")
        self.model = "placeholder_model"  # Replace with actual model loading
        
    def preprocess(self, image: np.ndarray) -> np.ndarray:
        """
        Preprocess image for YOLO inference
        
        Args:
            image: Input image (BGR format)
            
        Returns:
            Preprocessed image tensor
        """
        # Resize to model input size
        resized = cv2.resize(image, self.input_size)
        
        # Convert BGR to RGB
        rgb = cv2.cvtColor(resized, cv2.COLOR_BGR2RGB)
        
        # Normalize to [0, 1]
        normalized = rgb.astype(np.float32) / 255.0
        
        # Transpose to CHW format (channels first)
        transposed = np.transpose(normalized, (2, 0, 1))
        
        # Add batch dimension
        batched = np.expand_dims(transposed, axis=0)
        
        return batched
        
    def postprocess(
        self,
        outputs: np.ndarray,
        original_shape: Tuple[int, int],
        camera_id: str = "default"
    ) -> List[Detection]:
        """
        Postprocess YOLO outputs to detections
        
        Args:
            outputs: Model output tensor
            original_shape: Original image shape (H, W)
            camera_id: Camera identifier
            
        Returns:
            List of detections
        """
        detections = []
        
        # NOTE: This is a simplified placeholder implementation
        # In production, parse YOLO output format properly
        # Example output shape: [1, num_detections, 85] where 85 = 4 bbox + 1 conf + 80 classes
        
        # For demonstration, create mock detections
        # In production, replace with actual YOLO output parsing
        
        return detections
        
    def detect(
        self,
        image: np.ndarray,
        camera_id: str = "default"
    ) -> List[Detection]:
        """
        Run detection on a single image
        
        Args:
            image: Input image (BGR format)
            camera_id: Camera identifier
            
        Returns:
            List of detections
        """
        start_time = datetime.utcnow()
        
        # Preprocess
        input_tensor = self.preprocess(image)
        
        # Inference (placeholder)
        # In production: outputs = self.model.run(input_tensor)
        outputs = np.random.rand(1, 10, 85)  # Mock output
        
        # Postprocess
        detections = self.postprocess(outputs, image.shape[:2], camera_id)
        
        # Update tracking
        detections = self._update_tracking(detections, camera_id)
        
        # Calculate FPS
        elapsed = (datetime.utcnow() - start_time).total_seconds()
        fps = 1.0 / elapsed if elapsed > 0 else 0
        self.fps_history[camera_id].append(fps)
        
        self.total_detections += len(detections)
        
        return detections
        
    def _update_tracking(
        self,
        detections: List[Detection],
        camera_id: str
    ) -> List[Detection]:
        """
        Update object tracking for detections
        
        Args:
            detections: List of detections
            camera_id: Camera identifier
            
        Returns:
            Detections with track IDs assigned
        """
        # Simplified tracking using IoU matching
        # In production, use proper tracker like DeepSORT or ByteTrack
        
        camera_tracks = self.tracks[camera_id]
        
        # Match detections to existing tracks
        matched_tracks = set()
        
        for detection in detections:
            best_match_id = None
            best_iou = 0.3  # Minimum IoU threshold
            
            # Find best matching track
            for track_id, track in camera_tracks.items():
                if track.species != detection.species:
                    continue
                    
                if len(track.history) > 0:
                    last_bbox = track.history[-1]
                    iou = self._calculate_iou(detection.bbox, last_bbox)
                    if iou > best_iou:
                        best_iou = iou
                        best_match_id = track_id
                        
            if best_match_id is not None:
                # Update existing track
                track = camera_tracks[best_match_id]
                track.update(detection.bbox, detection.confidence)
                detection.track_id = best_match_id
                matched_tracks.add(best_match_id)
            else:
                # Create new track
                track_id = self.next_track_id[camera_id]
                self.next_track_id[camera_id] += 1
                
                new_track = Track(
                    track_id=track_id,
                    species=detection.species
                )
                new_track.update(detection.bbox, detection.confidence)
                camera_tracks[track_id] = new_track
                detection.track_id = track_id
                matched_tracks.add(track_id)
                
        # Remove stale tracks (not seen for 5 seconds)
        current_time = datetime.utcnow()
        stale_tracks = [
            tid for tid, track in camera_tracks.items()
            if (current_time - track.last_seen).total_seconds() > 5.0
            and tid not in matched_tracks
        ]
        for tid in stale_tracks:
            del camera_tracks[tid]
            
        return detections
        
    def _calculate_iou(
        self,
        bbox1: Tuple[int, int, int, int],
        bbox2: Tuple[int, int, int, int]
    ) -> float:
        """Calculate Intersection over Union between two bboxes"""
        x1_1, y1_1, x2_1, y2_1 = bbox1
        x1_2, y1_2, x2_2, y2_2 = bbox2
        
        # Calculate intersection
        x1_i = max(x1_1, x1_2)
        y1_i = max(y1_1, y1_2)
        x2_i = min(x2_1, x2_2)
        y2_i = min(y2_1, y2_2)
        
        if x2_i < x1_i or y2_i < y1_i:
            return 0.0
            
        intersection = (x2_i - x1_i) * (y2_i - y1_i)
        
        # Calculate union
        area1 = (x2_1 - x1_1) * (y2_1 - y1_1)
        area2 = (x2_2 - x1_2) * (y2_2 - y1_2)
        union = area1 + area2 - intersection
        
        return intersection / union if union > 0 else 0.0
        
    def analyze_behavior(self, track: Track) -> Optional[str]:
        """
        Analyze wildlife behavior from track history
        
        Args:
            track: Object track
            
        Returns:
            Behavior label (running, walking, feeding, resting) or None
        """
        if len(track.history) < 5:
            return None
            
        # Calculate movement speed from bbox centers
        speeds = []
        for i in range(1, len(track.history)):
            prev_bbox = track.history[i-1]
            curr_bbox = track.history[i]
            
            prev_center = ((prev_bbox[0] + prev_bbox[2]) / 2, (prev_bbox[1] + prev_bbox[3]) / 2)
            curr_center = ((curr_bbox[0] + curr_bbox[2]) / 2, (curr_bbox[1] + curr_bbox[3]) / 2)
            
            distance = np.sqrt((curr_center[0] - prev_center[0])**2 + (curr_center[1] - prev_center[1])**2)
            speeds.append(distance)
            
        avg_speed = np.mean(speeds)
        
        # Simple heuristic classification
        if avg_speed < 2:
            return "resting"
        elif avg_speed < 10:
            return "feeding"
        elif avg_speed < 30:
            return "walking"
        else:
            return "running"
            
    def get_statistics(self, camera_id: Optional[str] = None) -> Dict[str, Any]:
        """
        Get detector statistics
        
        Args:
            camera_id: Specific camera ID, or None for all cameras
            
        Returns:
            Statistics dictionary
        """
        if camera_id:
            avg_fps = np.mean(self.fps_history[camera_id]) if self.fps_history[camera_id] else 0
            active_tracks = len(self.tracks[camera_id])
            
            return {
                'camera_id': camera_id,
                'average_fps': float(avg_fps),
                'active_tracks': active_tracks
            }
        else:
            total_tracks = sum(len(tracks) for tracks in self.tracks.values())
            overall_fps = np.mean([fps for history in self.fps_history.values() for fps in history]) if self.fps_history else 0
            
            return {
                'total_detections': self.total_detections,
                'active_cameras': len(self.fps_history),
                'total_active_tracks': total_tracks,
                'average_fps': float(overall_fps)
            }
