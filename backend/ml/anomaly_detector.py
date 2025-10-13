"""
Anomaly Detection for Wildlife Behavior Patterns
Identifies unusual patterns and behaviors for enhanced alerting
"""

import numpy as np
from datetime import datetime, timedelta
from typing import Dict, List, Optional
from collections import defaultdict
import logging

logger = logging.getLogger(__name__)


class BehaviorAnomalyDetector:
    """
    Detects anomalous wildlife behavior patterns using statistical methods.
    Identifies unusual activity that may indicate environmental changes or threats.
    """
    
    def __init__(self, window_hours: int = 24):
        self.window_hours = window_hours
        self.activity_history = defaultdict(list)  # species -> list of timestamps
        self.baseline_patterns = {}  # species -> statistical baseline
        self.anomaly_threshold = 2.5  # Standard deviations
        
    def record_detection(self, species: str, timestamp: datetime, confidence: float):
        """Record a detection for pattern learning."""
        if confidence < 0.5:
            return  # Ignore low confidence detections
        
        self.activity_history[species].append({
            'timestamp': timestamp,
            'confidence': confidence
        })
        
        # Clean old data
        cutoff = timestamp - timedelta(hours=self.window_hours * 7)  # Keep 7 windows
        self.activity_history[species] = [
            d for d in self.activity_history[species]
            if d['timestamp'] > cutoff
        ]
    
    def detect_anomaly(self, species: str, current_timestamp: datetime) -> Dict:
        """
        Detect if current activity is anomalous.
        
        Returns:
            Dict with anomaly information including score and type
        """
        history = self.activity_history.get(species, [])
        
        if len(history) < 10:
            # Not enough data for anomaly detection
            return {
                'is_anomaly': False,
                'anomaly_score': 0.0,
                'anomaly_type': 'insufficient_data',
                'baseline_count': 0,
                'current_count': 0
            }
        
        # Calculate activity in current window
        window_start = current_timestamp - timedelta(hours=self.window_hours)
        current_activity = [
            d for d in history
            if window_start <= d['timestamp'] <= current_timestamp
        ]
        current_count = len(current_activity)
        
        # Calculate historical baseline
        baseline_counts = self._calculate_baseline_counts(history, current_timestamp)
        
        if len(baseline_counts) < 3:
            return {
                'is_anomaly': False,
                'anomaly_score': 0.0,
                'anomaly_type': 'insufficient_baseline',
                'baseline_count': 0,
                'current_count': current_count
            }
        
        mean_count = np.mean(baseline_counts)
        std_count = np.std(baseline_counts)
        
        # Avoid division by zero
        if std_count < 0.1:
            std_count = 0.1
        
        # Calculate z-score
        z_score = (current_count - mean_count) / std_count
        
        # Determine anomaly type
        anomaly_type = 'normal'
        is_anomaly = False
        
        if z_score > self.anomaly_threshold:
            anomaly_type = 'unusual_high_activity'
            is_anomaly = True
        elif z_score < -self.anomaly_threshold:
            anomaly_type = 'unusual_low_activity'
            is_anomaly = True
        
        # Check for temporal anomalies (wrong time of day)
        temporal_anomaly = self._check_temporal_anomaly(species, current_timestamp)
        if temporal_anomaly:
            anomaly_type = 'unusual_timing'
            is_anomaly = True
        
        return {
            'is_anomaly': is_anomaly,
            'anomaly_score': abs(z_score),
            'anomaly_type': anomaly_type,
            'baseline_count': mean_count,
            'current_count': current_count,
            'z_score': z_score
        }
    
    def _calculate_baseline_counts(self, history: List[Dict], 
                                   current_timestamp: datetime) -> List[int]:
        """Calculate historical activity counts for baseline."""
        counts = []
        
        # Look at previous windows (excluding current)
        for i in range(1, 8):  # Previous 7 windows
            window_start = current_timestamp - timedelta(hours=self.window_hours * (i + 1))
            window_end = current_timestamp - timedelta(hours=self.window_hours * i)
            
            window_activity = [
                d for d in history
                if window_start <= d['timestamp'] <= window_end
            ]
            counts.append(len(window_activity))
        
        return counts
    
    def _check_temporal_anomaly(self, species: str, timestamp: datetime) -> bool:
        """Check if detection occurs at unusual time for this species."""
        history = self.activity_history.get(species, [])
        
        if len(history) < 20:
            return False
        
        # Calculate hour distribution
        hour_counts = defaultdict(int)
        for detection in history:
            hour = detection['timestamp'].hour
            hour_counts[hour] += 1
        
        current_hour = timestamp.hour
        current_hour_count = hour_counts.get(current_hour, 0)
        avg_count = np.mean(list(hour_counts.values()))
        
        # If this hour has significantly less historical activity
        if avg_count > 0 and current_hour_count < avg_count * 0.2:
            return True
        
        return False
    
    def get_species_baseline(self, species: str) -> Optional[Dict]:
        """Get baseline activity pattern for a species."""
        history = self.activity_history.get(species, [])
        
        if len(history) < 10:
            return None
        
        # Calculate statistics
        recent_24h = [
            d for d in history
            if d['timestamp'] > datetime.utcnow() - timedelta(hours=24)
        ]
        
        hour_distribution = defaultdict(int)
        for detection in history:
            hour = detection['timestamp'].hour
            hour_distribution[hour] += 1
        
        return {
            'total_detections': len(history),
            'recent_24h_count': len(recent_24h),
            'most_active_hours': sorted(
                hour_distribution.items(), 
                key=lambda x: x[1], 
                reverse=True
            )[:5],
            'avg_confidence': np.mean([d['confidence'] for d in history])
        }


class TimeSeriesForecaster:
    """
    Simple time series forecasting for predictable vs unexpected wildlife events.
    Uses historical patterns to predict expected activity levels.
    """
    
    def __init__(self):
        self.hourly_patterns = defaultdict(lambda: defaultdict(list))  # species -> hour -> counts
        
    def learn_pattern(self, species: str, timestamp: datetime, count: int):
        """Learn activity pattern for a specific hour."""
        hour = timestamp.hour
        day_of_week = timestamp.weekday()
        
        # Store both hourly and day-of-week patterns
        self.hourly_patterns[species][hour].append(count)
        
        # Limit memory
        if len(self.hourly_patterns[species][hour]) > 100:
            self.hourly_patterns[species][hour] = self.hourly_patterns[species][hour][-100:]
    
    def forecast_activity(self, species: str, timestamp: datetime) -> Dict:
        """
        Forecast expected activity level.
        
        Returns:
            Dict with forecast information
        """
        hour = timestamp.hour
        
        if species not in self.hourly_patterns:
            return {
                'expected_count': 0,
                'confidence_interval_low': 0,
                'confidence_interval_high': 0,
                'has_forecast': False
            }
        
        hour_data = self.hourly_patterns[species].get(hour, [])
        
        if len(hour_data) < 3:
            return {
                'expected_count': 0,
                'confidence_interval_low': 0,
                'confidence_interval_high': 0,
                'has_forecast': False
            }
        
        mean = np.mean(hour_data)
        std = np.std(hour_data)
        
        # 95% confidence interval (approximately 2 standard deviations)
        return {
            'expected_count': mean,
            'confidence_interval_low': max(0, mean - 2 * std),
            'confidence_interval_high': mean + 2 * std,
            'has_forecast': True,
            'sample_size': len(hour_data)
        }
    
    def is_unexpected_activity(self, species: str, timestamp: datetime, 
                              actual_count: int) -> Tuple[bool, float]:
        """
        Determine if activity level is unexpected.
        
        Returns:
            Tuple of (is_unexpected, deviation_score)
        """
        forecast = self.forecast_activity(species, timestamp)
        
        if not forecast['has_forecast']:
            return False, 0.0
        
        expected = forecast['expected_count']
        ci_low = forecast['confidence_interval_low']
        ci_high = forecast['confidence_interval_high']
        
        # Check if outside confidence interval
        if actual_count < ci_low:
            deviation = (ci_low - actual_count) / max(expected, 1)
            return True, deviation
        elif actual_count > ci_high:
            deviation = (actual_count - ci_high) / max(expected, 1)
            return True, deviation
        
        return False, 0.0
