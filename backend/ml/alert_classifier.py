"""
ML-based Alert Classification System
Intelligent alert filtering and prioritization using machine learning
"""

import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Tuple, Optional
import logging

logger = logging.getLogger(__name__)


class AlertClassifier:
    """
    ML-based alert classifier for false positive reduction and prioritization.
    Uses historical patterns and environmental context for intelligent filtering.
    """
    
    def __init__(self):
        self.false_positive_patterns = []
        self.true_positive_patterns = []
        self.confidence_threshold = 0.7
        self.learning_rate = 0.1
        
    def extract_features(self, detection_data: Dict) -> np.ndarray:
        """
        Extract features from detection data for ML analysis.
        
        Args:
            detection_data: Detection metadata including confidence, environmental data, etc.
            
        Returns:
            Feature vector for ML processing
        """
        features = []
        
        # Detection confidence
        features.append(detection_data.get('confidence_score', 0.5))
        
        # Time-based features
        timestamp = detection_data.get('timestamp', datetime.utcnow())
        if isinstance(timestamp, str):
            timestamp = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
        
        hour = timestamp.hour
        features.append(hour / 24.0)  # Normalized hour
        features.append(1.0 if 6 <= hour <= 18 else 0.0)  # Day/night indicator
        
        # Environmental context
        weather = detection_data.get('weather_conditions', {})
        features.append(weather.get('temperature', 20.0) / 50.0)  # Normalized temperature
        features.append(weather.get('humidity', 50.0) / 100.0)  # Normalized humidity
        features.append(weather.get('wind_speed', 0.0) / 50.0)  # Normalized wind speed
        
        # Motion data
        motion_data = detection_data.get('motion_trigger_data', {})
        features.append(motion_data.get('motion_level', 0.0))
        features.append(motion_data.get('motion_duration', 0.0) / 10.0)  # Normalized duration
        
        # Species-specific features
        detected_species = detection_data.get('detected_species', '')
        dangerous_species = ['bear', 'wolf', 'mountain_lion', 'cougar', 'tiger', 'leopard']
        features.append(1.0 if any(sp in detected_species.lower() for sp in dangerous_species) else 0.0)
        
        return np.array(features)
    
    def predict_false_positive(self, features: np.ndarray) -> Tuple[float, bool]:
        """
        Predict if alert is a false positive using pattern matching.
        
        Args:
            features: Feature vector from detection
            
        Returns:
            Tuple of (confidence, is_false_positive)
        """
        if len(self.false_positive_patterns) == 0:
            # No training data yet, use default threshold
            return 0.5, False
        
        # Calculate similarity to known false positive patterns
        fp_similarities = []
        for fp_pattern in self.false_positive_patterns[-50:]:  # Use last 50 patterns
            similarity = self._calculate_similarity(features, fp_pattern)
            fp_similarities.append(similarity)
        
        avg_fp_similarity = np.mean(fp_similarities) if fp_similarities else 0.0
        
        # Calculate similarity to true positive patterns
        tp_similarities = []
        for tp_pattern in self.true_positive_patterns[-50:]:
            similarity = self._calculate_similarity(features, tp_pattern)
            tp_similarities.append(similarity)
        
        avg_tp_similarity = np.mean(tp_similarities) if tp_similarities else 0.0
        
        # Decision logic
        if avg_fp_similarity > 0.75 and avg_fp_similarity > avg_tp_similarity:
            return 1.0 - avg_fp_similarity, True
        elif avg_tp_similarity > 0.7:
            return avg_tp_similarity, False
        else:
            return 0.5, False
    
    def _calculate_similarity(self, features1: np.ndarray, features2: np.ndarray) -> float:
        """Calculate cosine similarity between two feature vectors."""
        if len(features1) != len(features2):
            return 0.0
        
        norm1 = np.linalg.norm(features1)
        norm2 = np.linalg.norm(features2)
        
        if norm1 == 0 or norm2 == 0:
            return 0.0
        
        similarity = np.dot(features1, features2) / (norm1 * norm2)
        return max(0.0, min(1.0, similarity))
    
    def calculate_priority(self, detection_data: Dict, ml_confidence: float) -> str:
        """
        Calculate alert priority based on detection data and ML confidence.
        
        Priority levels: info, warning, critical, emergency
        """
        confidence = detection_data.get('confidence_score', 0.5)
        detected_species = detection_data.get('detected_species', '').lower()
        
        # Emergency level - critical dangerous species with high confidence
        emergency_species = ['bear', 'grizzly', 'tiger', 'leopard']
        if any(sp in detected_species for sp in emergency_species) and confidence > 0.85:
            return 'emergency'
        
        # Critical level - dangerous species or very high confidence
        critical_species = ['wolf', 'mountain_lion', 'cougar', 'coyote']
        if any(sp in detected_species for sp in critical_species) and confidence > 0.75:
            return 'critical'
        
        # Warning level - moderate confidence or rare species
        if confidence > 0.6 and ml_confidence > 0.6:
            return 'warning'
        
        # Info level - low confidence or common species
        return 'info'
    
    def learn_from_feedback(self, features: np.ndarray, is_false_positive: bool):
        """
        Update internal patterns based on user feedback.
        
        Args:
            features: Feature vector from detection
            is_false_positive: Whether user confirmed this as false positive
        """
        if is_false_positive:
            self.false_positive_patterns.append(features)
            # Limit memory usage
            if len(self.false_positive_patterns) > 200:
                self.false_positive_patterns = self.false_positive_patterns[-200:]
        else:
            self.true_positive_patterns.append(features)
            if len(self.true_positive_patterns) > 200:
                self.true_positive_patterns = self.true_positive_patterns[-200:]
        
        logger.info(f"Updated patterns: FP={len(self.false_positive_patterns)}, TP={len(self.true_positive_patterns)}")
    
    def get_pattern_statistics(self) -> Dict:
        """Get statistics about learned patterns."""
        return {
            'false_positive_patterns': len(self.false_positive_patterns),
            'true_positive_patterns': len(self.true_positive_patterns),
            'confidence_threshold': self.confidence_threshold,
            'total_patterns': len(self.false_positive_patterns) + len(self.true_positive_patterns)
        }


class ContextAwareAlertEngine:
    """
    Context-aware alert engine that considers environmental and temporal factors.
    """
    
    def __init__(self):
        self.alert_classifier = AlertClassifier()
        
    def evaluate_alert(self, detection_data: Dict) -> Dict:
        """
        Evaluate an alert considering all context factors.
        
        Returns:
            Dict with evaluation results including priority, confidence, and recommendations
        """
        # Extract features and predict false positive
        features = self.alert_classifier.extract_features(detection_data)
        ml_confidence, is_false_positive = self.alert_classifier.predict_false_positive(features)
        
        # Calculate priority
        priority = self.alert_classifier.calculate_priority(detection_data, ml_confidence)
        
        # Environmental filtering
        should_filter = self._should_filter_by_environment(detection_data)
        
        # Temporal analysis
        temporal_score = self._analyze_temporal_patterns(detection_data)
        
        return {
            'ml_confidence': ml_confidence,
            'is_false_positive': is_false_positive,
            'priority': priority,
            'should_filter': should_filter,
            'temporal_score': temporal_score,
            'recommendation': self._generate_recommendation(
                is_false_positive, priority, should_filter, temporal_score
            )
        }
    
    def _should_filter_by_environment(self, detection_data: Dict) -> bool:
        """Check if alert should be filtered based on environmental conditions."""
        weather = detection_data.get('weather_conditions', {})
        
        # Filter in extreme weather
        temperature = weather.get('temperature', 20.0)
        if temperature < -5 or temperature > 40:
            return True
        
        # Filter in very high wind
        wind_speed = weather.get('wind_speed', 0.0)
        if wind_speed > 40:
            return True
        
        # Filter in very low visibility
        visibility = weather.get('visibility', 100.0)
        if visibility < 10:
            return True
        
        return False
    
    def _analyze_temporal_patterns(self, detection_data: Dict) -> float:
        """
        Analyze temporal patterns to score alert importance.
        Returns score 0.0-1.0, higher is more significant.
        """
        timestamp = detection_data.get('timestamp', datetime.utcnow())
        if isinstance(timestamp, str):
            timestamp = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
        
        hour = timestamp.hour
        
        # Wildlife typically more active during dawn/dusk
        if 5 <= hour <= 8 or 17 <= hour <= 20:
            return 0.9  # High activity period
        elif 9 <= hour <= 16:
            return 0.5  # Moderate activity period
        else:
            return 0.7  # Night activity, moderate-high significance
    
    def _generate_recommendation(self, is_false_positive: bool, priority: str, 
                                 should_filter: bool, temporal_score: float) -> str:
        """Generate human-readable recommendation for the alert."""
        if is_false_positive and should_filter:
            return "SUPPRESS - High probability false positive with poor environmental conditions"
        elif is_false_positive:
            return "REVIEW - Possible false positive, manual review recommended"
        elif priority == 'emergency':
            return "IMMEDIATE ACTION - Critical wildlife detection requiring urgent response"
        elif priority == 'critical' and temporal_score > 0.7:
            return "ALERT - Significant detection during active period"
        elif should_filter:
            return "QUEUE - Valid detection but poor conditions, queue for review"
        elif temporal_score < 0.3:
            return "LOG - Unusual timing, log for pattern analysis"
        else:
            return "NOTIFY - Standard alert notification"
