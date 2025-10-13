"""
Advanced ML Alert Engine
Implements intelligent alert system with confidence scoring and false positive reduction
"""

import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple
import logging

logger = logging.getLogger(__name__)


class AlertLevel:
    """Multi-tier alert levels"""
    INFO = 'info'
    WARNING = 'warning'
    CRITICAL = 'critical'
    EMERGENCY = 'emergency'


class MLAlertEngine:
    """
    Machine Learning powered alert engine with intelligent filtering
    """
    
    def __init__(self):
        self.confidence_threshold = 0.7
        self.temporal_window = 5  # frames for temporal consistency
        self.detection_history = []  # Store recent detections
        self.false_positive_threshold = 0.3
        
    def calculate_confidence_score(
        self,
        base_confidence: float,
        temporal_consistency: float = 1.0,
        size_validation: float = 1.0,
        environmental_context: float = 1.0
    ) -> float:
        """
        Calculate enhanced confidence score using ensemble methods
        
        Args:
            base_confidence: Base ML model confidence (0-1)
            temporal_consistency: Consistency across frames (0-1)
            size_validation: Object size validation score (0-1)
            environmental_context: Environmental context score (0-1)
            
        Returns:
            Enhanced confidence score (0-1)
        """
        # Weighted ensemble scoring
        weights = {
            'base': 0.4,
            'temporal': 0.25,
            'size': 0.2,
            'context': 0.15
        }
        
        enhanced_score = (
            weights['base'] * base_confidence +
            weights['temporal'] * temporal_consistency +
            weights['size'] * size_validation +
            weights['context'] * environmental_context
        )
        
        return np.clip(enhanced_score, 0.0, 1.0)
    
    def check_temporal_consistency(
        self,
        current_detection: Dict,
        history: List[Dict]
    ) -> float:
        """
        Check detection consistency across multiple frames
        
        Args:
            current_detection: Current detection data
            history: List of recent detections
            
        Returns:
            Consistency score (0-1)
        """
        if not history:
            return 0.5  # Neutral score for first detection
        
        # Count matching detections in recent history
        species = current_detection.get('species', 'unknown')
        matching = sum(1 for d in history[-self.temporal_window:] 
                      if d.get('species') == species)
        
        consistency = matching / min(len(history), self.temporal_window)
        return consistency
    
    def validate_object_size(
        self,
        bounding_box: Dict,
        species: str,
        image_dimensions: Tuple[int, int]
    ) -> float:
        """
        Validate if object size is reasonable for detected species
        
        Args:
            bounding_box: Detection bounding box {x, y, width, height}
            species: Detected species name
            image_dimensions: (width, height) of image
            
        Returns:
            Validation score (0-1)
        """
        if not bounding_box:
            return 0.5
        
        # Calculate object size as percentage of image
        obj_width = bounding_box.get('width', 0)
        obj_height = bounding_box.get('height', 0)
        img_width, img_height = image_dimensions
        
        size_ratio = (obj_width * obj_height) / (img_width * img_height)
        
        # Species-specific size expectations
        size_ranges = {
            'bear': (0.15, 0.8),
            'wolf': (0.08, 0.5),
            'deer': (0.1, 0.6),
            'bird': (0.01, 0.3),
            'small_mammal': (0.01, 0.15)
        }
        
        # Default range for unknown species
        expected_min, expected_max = size_ranges.get(species.lower(), (0.01, 0.9))
        
        # Score based on how well size fits expected range
        if expected_min <= size_ratio <= expected_max:
            return 1.0
        elif size_ratio < expected_min:
            # Too small - linearly decrease score
            return max(0.3, size_ratio / expected_min)
        else:
            # Too large - linearly decrease score
            return max(0.3, expected_max / size_ratio)
    
    def evaluate_environmental_context(
        self,
        species: str,
        time_of_day: str,
        weather: Optional[Dict] = None,
        location: Optional[Dict] = None
    ) -> float:
        """
        Evaluate if detection makes sense in environmental context
        
        Args:
            species: Detected species
            time_of_day: 'dawn', 'day', 'dusk', 'night'
            weather: Weather conditions dict
            location: Location metadata
            
        Returns:
            Context validation score (0-1)
        """
        score = 0.5  # Neutral baseline
        
        # Time-based patterns (species activity patterns)
        nocturnal = ['owl', 'bat', 'raccoon', 'opossum']
        diurnal = ['eagle', 'hawk', 'deer', 'squirrel']
        crepuscular = ['bear', 'deer', 'rabbit', 'coyote']
        
        species_lower = species.lower()
        
        if time_of_day in ['dawn', 'dusk']:
            if any(s in species_lower for s in crepuscular):
                score += 0.3
        elif time_of_day == 'night':
            if any(s in species_lower for s in nocturnal):
                score += 0.3
            elif any(s in species_lower for s in diurnal):
                score -= 0.2
        elif time_of_day == 'day':
            if any(s in species_lower for s in diurnal):
                score += 0.3
            elif any(s in species_lower for s in nocturnal):
                score -= 0.2
        
        # Weather-based adjustments
        if weather:
            temp = weather.get('temperature', 20)
            # Cold-weather activity adjustment
            if temp < 5:
                if 'reptile' in species_lower:
                    score -= 0.3  # Reptiles less active in cold
                elif 'bear' in species_lower:
                    score -= 0.1  # Bears less active in cold
        
        return np.clip(score, 0.0, 1.0)
    
    def predict_false_positive(
        self,
        detection: Dict,
        confidence: float,
        temporal_consistency: float
    ) -> bool:
        """
        Predict if detection is likely a false positive
        
        Args:
            detection: Detection data
            confidence: Enhanced confidence score
            temporal_consistency: Temporal consistency score
            
        Returns:
            True if likely false positive
        """
        # Multiple factors indicate false positive
        is_false_positive = False
        
        # Low confidence
        if confidence < self.confidence_threshold:
            is_false_positive = True
        
        # Poor temporal consistency
        if temporal_consistency < 0.3:
            is_false_positive = True
        
        # Suspicious patterns (e.g., very small objects)
        bbox = detection.get('bounding_box', {})
        if bbox:
            area = bbox.get('width', 0) * bbox.get('height', 0)
            if area < 100:  # Very small detection
                is_false_positive = True
        
        return is_false_positive
    
    def determine_alert_level(
        self,
        species: str,
        confidence: float,
        is_dangerous: bool,
        detection_count: int = 1
    ) -> str:
        """
        Determine appropriate alert level based on detection
        
        Args:
            species: Detected species
            confidence: Detection confidence
            is_dangerous: Whether species is dangerous
            detection_count: Number of consecutive detections
            
        Returns:
            Alert level (info, warning, critical, emergency)
        """
        if not is_dangerous:
            # Non-dangerous species
            if confidence > 0.9:
                return AlertLevel.INFO
            else:
                return AlertLevel.INFO
        
        # Dangerous species alerts
        dangerous_critical = ['bear', 'mountain lion', 'cougar']
        dangerous_high = ['wolf', 'alligator', 'moose']
        
        species_lower = species.lower()
        
        # Emergency level for critical dangerous species with high confidence
        if any(s in species_lower for s in dangerous_critical):
            if confidence > 0.85 and detection_count >= 2:
                return AlertLevel.EMERGENCY
            elif confidence > 0.75:
                return AlertLevel.CRITICAL
            else:
                return AlertLevel.WARNING
        
        # Critical/Warning for other dangerous species
        if any(s in species_lower for s in dangerous_high):
            if confidence > 0.8 and detection_count >= 2:
                return AlertLevel.CRITICAL
            else:
                return AlertLevel.WARNING
        
        # Default for other dangerous species
        if confidence > 0.75:
            return AlertLevel.WARNING
        
        return AlertLevel.INFO
    
    def update_detection_history(self, detection: Dict):
        """
        Update detection history for temporal analysis
        
        Args:
            detection: Detection data with timestamp
        """
        # Add to history
        self.detection_history.append({
            **detection,
            'timestamp': datetime.utcnow()
        })
        
        # Keep only recent detections (last 10 minutes)
        cutoff = datetime.utcnow() - timedelta(minutes=10)
        self.detection_history = [
            d for d in self.detection_history 
            if d['timestamp'] > cutoff
        ]
    
    def adjust_threshold_dynamically(
        self,
        recent_false_positives: int,
        recent_true_positives: int,
        time_of_day: str
    ) -> float:
        """
        Dynamically adjust confidence threshold based on performance
        
        Args:
            recent_false_positives: Count of recent false positives
            recent_true_positives: Count of recent true positives
            time_of_day: Current time of day
            
        Returns:
            Adjusted confidence threshold
        """
        base_threshold = 0.7
        
        # Adjust based on false positive rate
        total = recent_false_positives + recent_true_positives
        if total > 10:
            fp_rate = recent_false_positives / total
            if fp_rate > 0.3:  # High false positive rate
                base_threshold += 0.1  # Increase threshold
            elif fp_rate < 0.05:  # Very low false positive rate
                base_threshold -= 0.05  # Slightly decrease threshold
        
        # Time-based adjustments (less certain at night)
        if time_of_day == 'night':
            base_threshold += 0.05
        
        return np.clip(base_threshold, 0.5, 0.95)
    
    def generate_alert_context(
        self,
        detection: Dict,
        confidence: float,
        alert_level: str,
        temporal_consistency: float
    ) -> Dict:
        """
        Generate rich alert context with metadata
        
        Args:
            detection: Detection data
            confidence: Enhanced confidence score
            alert_level: Determined alert level
            temporal_consistency: Temporal consistency score
            
        Returns:
            Alert context dictionary
        """
        return {
            'species': detection.get('species', 'unknown'),
            'confidence': round(confidence, 3),
            'alert_level': alert_level,
            'temporal_consistency': round(temporal_consistency, 3),
            'timestamp': datetime.utcnow().isoformat(),
            'location': detection.get('location'),
            'camera_id': detection.get('camera_id'),
            'bounding_box': detection.get('bounding_box'),
            'environmental_data': detection.get('environmental_data'),
            'consecutive_detections': detection.get('consecutive_detections', 1),
            'image_url': detection.get('image_url'),
            'requires_verification': confidence < 0.8,
            'priority': self._calculate_priority(alert_level, confidence)
        }
    
    def _calculate_priority(self, alert_level: str, confidence: float) -> int:
        """Calculate numeric priority for alert routing"""
        priorities = {
            AlertLevel.EMERGENCY: 1000,
            AlertLevel.CRITICAL: 500,
            AlertLevel.WARNING: 100,
            AlertLevel.INFO: 10
        }
        base_priority = priorities.get(alert_level, 10)
        # Increase priority with confidence
        return int(base_priority * (0.5 + 0.5 * confidence))
