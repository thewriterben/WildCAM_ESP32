"""
YOLO Object Detection Module with TensorRT Optimization
Supports YOLOv8 models optimized for NVIDIA Jetson hardware
"""

import numpy as np
import cv2
import tensorrt as trt
import pycuda.driver as cuda
import pycuda.autoinit
from typing import List, Tuple, Dict, Optional
import logging
from pathlib import Path
import json

logger = logging.getLogger(__name__)


class YOLODetector:
    """
    TensorRT-optimized YOLO detector for real-time wildlife species detection
    """
    
    def __init__(
        self,
        model_path: str,
        confidence_threshold: float = 0.6,
        nms_threshold: float = 0.45,
        input_size: Tuple[int, int] = (640, 640),
        class_labels: Optional[List[str]] = None
    ):
        """
        Initialize YOLO detector with TensorRT engine
        
        Args:
            model_path: Path to TensorRT engine file (.engine or .trt)
            confidence_threshold: Minimum confidence for valid detections
            nms_threshold: Non-maximum suppression threshold
            input_size: Model input dimensions (width, height)
            class_labels: List of class names for detection
        """
        self.model_path = Path(model_path)
        self.confidence_threshold = confidence_threshold
        self.nms_threshold = nms_threshold
        self.input_size = input_size
        self.class_labels = class_labels or self._load_class_labels()
        
        # TensorRT runtime components
        self.logger_trt = trt.Logger(trt.Logger.WARNING)
        self.engine = None
        self.context = None
        self.inputs = []
        self.outputs = []
        self.bindings = []
        self.stream = None
        
        # Performance metrics
        self.inference_count = 0
        self.total_inference_time = 0.0
        
        # Load TensorRT engine
        self._load_engine()
        
    def _load_class_labels(self) -> List[str]:
        """Load class labels from JSON file"""
        label_path = self.model_path.parent / "class_labels.json"
        if label_path.exists():
            with open(label_path, 'r') as f:
                data = json.load(f)
                return data.get('classes', [])
        
        # Default wildlife classes
        return [
            "deer", "bear", "fox", "rabbit", "bird",
            "coyote", "raccoon", "skunk", "squirrel", "turkey"
        ]
    
    def _load_engine(self):
        """Load TensorRT engine from file"""
        if not self.model_path.exists():
            raise FileNotFoundError(f"Model file not found: {self.model_path}")
        
        logger.info(f"Loading TensorRT engine from {self.model_path}")
        
        with open(self.model_path, 'rb') as f:
            runtime = trt.Runtime(self.logger_trt)
            self.engine = runtime.deserialize_cuda_engine(f.read())
        
        if self.engine is None:
            raise RuntimeError("Failed to load TensorRT engine")
        
        self.context = self.engine.create_execution_context()
        self.stream = cuda.Stream()
        
        # Allocate buffers
        self._allocate_buffers()
        
        logger.info(f"TensorRT engine loaded successfully")
        logger.info(f"Model input size: {self.input_size}")
        logger.info(f"Number of classes: {len(self.class_labels)}")
    
    def _allocate_buffers(self):
        """Allocate GPU memory buffers for inference"""
        self.inputs = []
        self.outputs = []
        self.bindings = []
        
        for binding in self.engine:
            size = trt.volume(self.engine.get_binding_shape(binding))
            dtype = trt.nptype(self.engine.get_binding_dtype(binding))
            
            # Allocate host and device buffers
            host_mem = cuda.pagelocked_empty(size, dtype)
            device_mem = cuda.mem_alloc(host_mem.nbytes)
            
            self.bindings.append(int(device_mem))
            
            if self.engine.binding_is_input(binding):
                self.inputs.append({'host': host_mem, 'device': device_mem})
            else:
                self.outputs.append({'host': host_mem, 'device': device_mem})
    
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
        
        # Normalize to [0, 1] and transpose to CHW format
        normalized = rgb.astype(np.float32) / 255.0
        transposed = np.transpose(normalized, (2, 0, 1))
        
        # Add batch dimension
        batched = np.expand_dims(transposed, axis=0)
        
        return np.ascontiguousarray(batched)
    
    def detect(self, image: np.ndarray) -> List[Dict]:
        """
        Perform object detection on input image
        
        Args:
            image: Input image (BGR format)
            
        Returns:
            List of detections with format:
            [
                {
                    'class': 'deer',
                    'confidence': 0.95,
                    'bbox': [x1, y1, x2, y2],
                    'bbox_normalized': [x1_norm, y1_norm, x2_norm, y2_norm]
                },
                ...
            ]
        """
        import time
        start_time = time.time()
        
        # Preprocess image
        input_tensor = self.preprocess(image)
        
        # Copy input to device
        np.copyto(self.inputs[0]['host'], input_tensor.ravel())
        cuda.memcpy_htod_async(
            self.inputs[0]['device'],
            self.inputs[0]['host'],
            self.stream
        )
        
        # Run inference
        self.context.execute_async_v2(
            bindings=self.bindings,
            stream_handle=self.stream.handle
        )
        
        # Copy output back to host
        cuda.memcpy_dtoh_async(
            self.outputs[0]['host'],
            self.outputs[0]['device'],
            self.stream
        )
        
        self.stream.synchronize()
        
        # Parse detections
        detections = self._parse_detections(
            self.outputs[0]['host'],
            image.shape
        )
        
        # Update metrics
        inference_time = time.time() - start_time
        self.inference_count += 1
        self.total_inference_time += inference_time
        
        logger.debug(f"Inference completed in {inference_time*1000:.2f}ms, "
                    f"found {len(detections)} detections")
        
        return detections
    
    def _parse_detections(
        self,
        output: np.ndarray,
        image_shape: Tuple[int, int, int]
    ) -> List[Dict]:
        """
        Parse YOLO output tensor into detection objects
        
        Args:
            output: Raw model output
            image_shape: Original image dimensions
            
        Returns:
            List of detection dictionaries
        """
        detections = []
        img_h, img_w = image_shape[:2]
        
        # Reshape output (format depends on YOLO version)
        # For YOLOv8: [1, num_boxes, 4+num_classes]
        # 4 = bbox coords (x_center, y_center, width, height)
        expected_cols = 4 + len(self.class_labels)
        
        # Validate output shape before reshaping
        if output.size % expected_cols != 0:
            logger.warning(f"Output size {output.size} not divisible by {expected_cols}, using fallback parsing")
            return []
        
        output = output.reshape(-1, expected_cols)
        
        for detection in output:
            # Extract bbox coordinates
            x_center, y_center, width, height = detection[:4]
            
            # Extract class scores
            class_scores = detection[4:]
            class_id = np.argmax(class_scores)
            confidence = class_scores[class_id]
            
            # Filter by confidence threshold
            if confidence < self.confidence_threshold:
                continue
            
            # Convert to corner format and scale to image size
            x1 = int((x_center - width / 2) * img_w / self.input_size[0])
            y1 = int((y_center - height / 2) * img_h / self.input_size[1])
            x2 = int((x_center + width / 2) * img_w / self.input_size[0])
            y2 = int((y_center + height / 2) * img_h / self.input_size[1])
            
            # Clamp to image boundaries
            x1, y1 = max(0, x1), max(0, y1)
            x2, y2 = min(img_w, x2), min(img_h, y2)
            
            # Normalized coordinates
            x1_norm = x1 / img_w
            y1_norm = y1 / img_h
            x2_norm = x2 / img_w
            y2_norm = y2 / img_h
            
            detections.append({
                'class': self.class_labels[class_id],
                'class_id': int(class_id),
                'confidence': float(confidence),
                'bbox': [x1, y1, x2, y2],
                'bbox_normalized': [x1_norm, y1_norm, x2_norm, y2_norm]
            })
        
        # Apply non-maximum suppression
        detections = self._nms(detections)
        
        return detections
    
    def _nms(self, detections: List[Dict]) -> List[Dict]:
        """
        Apply non-maximum suppression to remove duplicate detections
        
        Args:
            detections: List of detections
            
        Returns:
            Filtered list of detections
        """
        if len(detections) == 0:
            return []
        
        # Group by class
        class_groups = {}
        for det in detections:
            class_name = det['class']
            if class_name not in class_groups:
                class_groups[class_name] = []
            class_groups[class_name].append(det)
        
        # Apply NMS per class
        final_detections = []
        for class_name, class_dets in class_groups.items():
            # Sort by confidence
            class_dets.sort(key=lambda x: x['confidence'], reverse=True)
            
            keep = []
            while len(class_dets) > 0:
                # Keep highest confidence detection
                best = class_dets.pop(0)
                keep.append(best)
                
                # Remove overlapping detections
                class_dets = [
                    det for det in class_dets
                    if self._iou(best['bbox'], det['bbox']) < self.nms_threshold
                ]
            
            final_detections.extend(keep)
        
        return final_detections
    
    @staticmethod
    def _iou(box1: List[int], box2: List[int]) -> float:
        """
        Calculate Intersection over Union (IoU) between two bounding boxes
        
        Args:
            box1: [x1, y1, x2, y2]
            box2: [x1, y1, x2, y2]
            
        Returns:
            IoU value between 0 and 1
        """
        x1_1, y1_1, x2_1, y2_1 = box1
        x1_2, y1_2, x2_2, y2_2 = box2
        
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
    
    def get_stats(self) -> Dict:
        """Get performance statistics"""
        avg_inference_time = (
            self.total_inference_time / self.inference_count
            if self.inference_count > 0 else 0
        )
        
        return {
            'inference_count': self.inference_count,
            'avg_inference_time_ms': avg_inference_time * 1000,
            'avg_fps': 1.0 / avg_inference_time if avg_inference_time > 0 else 0
        }
    
    def reset_stats(self):
        """Reset performance counters"""
        self.inference_count = 0
        self.total_inference_time = 0.0
    
    def __del__(self):
        """Cleanup resources"""
        if hasattr(self, 'stream') and self.stream:
            self.stream.synchronize()
