"""
Unit tests for advanced ML alert system
"""

import pytest
import sys
import os
from datetime import datetime, timedelta
import numpy as np

# Add backend to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'backend'))

from ml.alert_classifier import AlertClassifier, ContextAwareAlertEngine
from ml.anomaly_detector import BehaviorAnomalyDetector, TimeSeriesForecaster


class TestAlertClassifier:
    """Test cases for ML-based alert classification."""
    
    def setup_method(self):
        """Set up test fixtures."""
        self.classifier = AlertClassifier()
    
    def test_extract_features(self):
        """Test feature extraction from detection data."""
        detection_data = {
            'confidence_score': 0.85,
            'timestamp': datetime(2024, 1, 15, 14, 30),
            'detected_species': 'bear',
            'weather_conditions': {
                'temperature': 22.5,
                'humidity': 65.0,
                'wind_speed': 10.0
            },
            'motion_trigger_data': {
                'motion_level': 0.8,
                'motion_duration': 5.0
            }
        }
        
        features = self.classifier.extract_features(detection_data)
        
        assert isinstance(features, np.ndarray)
        assert len(features) == 9  # Expected number of features
        assert 0 <= features[0] <= 1  # Confidence should be normalized
        assert features[8] == 1.0  # Dangerous species indicator
    
    def test_predict_false_positive_no_training(self):
        """Test false positive prediction with no training data."""
        features = np.array([0.5, 0.6, 1.0, 0.4, 0.5, 0.0, 0.7, 0.5, 0.0, 1.0])
        
        confidence, is_fp = self.classifier.predict_false_positive(features)
        
        assert isinstance(confidence, float)
        assert isinstance(is_fp, bool)
        assert 0 <= confidence <= 1
        assert is_fp is False  # Should default to not false positive
    
    def test_calculate_priority_emergency(self):
        """Test priority calculation for emergency situations."""
        detection_data = {
            'detected_species': 'bear',
            'confidence_score': 0.9
        }
        
        priority = self.classifier.calculate_priority(detection_data, 0.85)
        
        assert priority == 'emergency'
    
    def test_calculate_priority_critical(self):
        """Test priority calculation for critical situations."""
        detection_data = {
            'detected_species': 'wolf',
            'confidence_score': 0.8
        }
        
        priority = self.classifier.calculate_priority(detection_data, 0.75)
        
        assert priority == 'critical'
    
    def test_calculate_priority_warning(self):
        """Test priority calculation for warning level."""
        detection_data = {
            'detected_species': 'deer',
            'confidence_score': 0.7
        }
        
        priority = self.classifier.calculate_priority(detection_data, 0.65)
        
        assert priority == 'warning'
    
    def test_calculate_priority_info(self):
        """Test priority calculation for info level."""
        detection_data = {
            'detected_species': 'deer',
            'confidence_score': 0.5
        }
        
        priority = self.classifier.calculate_priority(detection_data, 0.4)
        
        assert priority == 'info'
    
    def test_learn_from_feedback_false_positive(self):
        """Test learning from false positive feedback."""
        features = np.array([0.5, 0.6, 1.0, 0.4, 0.5, 0.0, 0.7, 0.5, 0.0, 0.0])
        
        initial_fp_count = len(self.classifier.false_positive_patterns)
        
        self.classifier.learn_from_feedback(features, is_false_positive=True)
        
        assert len(self.classifier.false_positive_patterns) == initial_fp_count + 1
        assert len(self.classifier.true_positive_patterns) == 0
    
    def test_learn_from_feedback_true_positive(self):
        """Test learning from true positive feedback."""
        features = np.array([0.8, 0.6, 1.0, 0.4, 0.5, 0.0, 0.7, 0.5, 0.0, 1.0])
        
        initial_tp_count = len(self.classifier.true_positive_patterns)
        
        self.classifier.learn_from_feedback(features, is_false_positive=False)
        
        assert len(self.classifier.true_positive_patterns) == initial_tp_count + 1
        assert len(self.classifier.false_positive_patterns) == 0
    
    def test_pattern_statistics(self):
        """Test getting pattern statistics."""
        # Add some training data
        for i in range(5):
            features = np.random.rand(10)
            self.classifier.learn_from_feedback(features, is_false_positive=(i % 2 == 0))
        
        stats = self.classifier.get_pattern_statistics()
        
        assert 'false_positive_patterns' in stats
        assert 'true_positive_patterns' in stats
        assert 'confidence_threshold' in stats
        assert stats['total_patterns'] == 5


class TestContextAwareAlertEngine:
    """Test cases for context-aware alert engine."""
    
    def setup_method(self):
        """Set up test fixtures."""
        self.engine = ContextAwareAlertEngine()
    
    def test_evaluate_alert_basic(self):
        """Test basic alert evaluation."""
        detection_data = {
            'confidence_score': 0.75,
            'timestamp': datetime.utcnow(),
            'detected_species': 'deer',
            'weather_conditions': {
                'temperature': 20.0,
                'humidity': 60.0,
                'wind_speed': 5.0,
                'visibility': 100.0
            },
            'motion_trigger_data': {
                'motion_level': 0.7,
                'motion_duration': 3.0
            }
        }
        
        result = self.engine.evaluate_alert(detection_data)
        
        assert 'ml_confidence' in result
        assert 'is_false_positive' in result
        assert 'priority' in result
        assert 'should_filter' in result
        assert 'temporal_score' in result
        assert 'recommendation' in result
    
    def test_environmental_filtering_extreme_temperature(self):
        """Test filtering based on extreme temperature."""
        detection_data = {
            'weather_conditions': {'temperature': -10.0}
        }
        
        should_filter = self.engine._should_filter_by_environment(detection_data)
        
        assert should_filter is True
    
    def test_environmental_filtering_high_wind(self):
        """Test filtering based on high wind."""
        detection_data = {
            'weather_conditions': {'wind_speed': 45.0}
        }
        
        should_filter = self.engine._should_filter_by_environment(detection_data)
        
        assert should_filter is True
    
    def test_environmental_filtering_normal_conditions(self):
        """Test no filtering in normal conditions."""
        detection_data = {
            'weather_conditions': {
                'temperature': 20.0,
                'wind_speed': 10.0,
                'visibility': 50.0
            }
        }
        
        should_filter = self.engine._should_filter_by_environment(detection_data)
        
        assert should_filter is False
    
    def test_temporal_analysis_dawn(self):
        """Test temporal analysis during dawn (high activity)."""
        detection_data = {
            'timestamp': datetime(2024, 1, 15, 6, 30)  # 6:30 AM
        }
        
        score = self.engine._analyze_temporal_patterns(detection_data)
        
        assert score >= 0.8  # High activity period
    
    def test_temporal_analysis_midday(self):
        """Test temporal analysis during midday (moderate activity)."""
        detection_data = {
            'timestamp': datetime(2024, 1, 15, 12, 0)  # Noon
        }
        
        score = self.engine._analyze_temporal_patterns(detection_data)
        
        assert 0.4 <= score <= 0.6  # Moderate activity period
    
    def test_temporal_analysis_night(self):
        """Test temporal analysis at night."""
        detection_data = {
            'timestamp': datetime(2024, 1, 15, 23, 0)  # 11 PM
        }
        
        score = self.engine._analyze_temporal_patterns(detection_data)
        
        assert score >= 0.6  # Night activity is significant


class TestBehaviorAnomalyDetector:
    """Test cases for behavior anomaly detection."""
    
    def setup_method(self):
        """Set up test fixtures."""
        self.detector = BehaviorAnomalyDetector(window_hours=24)
    
    def test_record_detection(self):
        """Test recording a detection."""
        species = 'deer'
        timestamp = datetime.utcnow()
        confidence = 0.8
        
        initial_count = len(self.detector.activity_history[species])
        
        self.detector.record_detection(species, timestamp, confidence)
        
        assert len(self.detector.activity_history[species]) == initial_count + 1
    
    def test_detect_anomaly_insufficient_data(self):
        """Test anomaly detection with insufficient data."""
        result = self.detector.detect_anomaly('unknown_species', datetime.utcnow())
        
        assert result['is_anomaly'] is False
        assert result['anomaly_type'] == 'insufficient_data'
    
    def test_detect_anomaly_normal_activity(self):
        """Test anomaly detection with normal activity."""
        species = 'deer'
        base_time = datetime.utcnow()
        
        # Create baseline with consistent activity
        for i in range(20):
            timestamp = base_time - timedelta(hours=24 * i)
            self.detector.record_detection(species, timestamp, 0.8)
        
        # Test current activity (should be normal)
        result = self.detector.detect_anomaly(species, base_time)
        
        # With consistent activity, should not be anomalous
        assert 'is_anomaly' in result
        assert 'anomaly_score' in result
    
    def test_get_species_baseline(self):
        """Test getting species baseline."""
        species = 'deer'
        
        # Add some detections
        for i in range(15):
            timestamp = datetime.utcnow() - timedelta(hours=i)
            self.detector.record_detection(species, timestamp, 0.75)
        
        baseline = self.detector.get_species_baseline(species)
        
        assert baseline is not None
        assert 'total_detections' in baseline
        assert 'recent_24h_count' in baseline
        assert 'most_active_hours' in baseline
        assert baseline['total_detections'] == 15


class TestTimeSeriesForecaster:
    """Test cases for time series forecasting."""
    
    def setup_method(self):
        """Set up test fixtures."""
        self.forecaster = TimeSeriesForecaster()
    
    def test_learn_pattern(self):
        """Test learning activity patterns."""
        species = 'deer'
        timestamp = datetime(2024, 1, 15, 14, 0)
        
        self.forecaster.learn_pattern(species, timestamp, count=3)
        
        hour = timestamp.hour
        assert hour in self.forecaster.hourly_patterns[species]
        assert len(self.forecaster.hourly_patterns[species][hour]) == 1
    
    def test_forecast_activity_no_data(self):
        """Test forecasting with no historical data."""
        forecast = self.forecaster.forecast_activity('unknown_species', datetime.utcnow())
        
        assert forecast['has_forecast'] is False
        assert forecast['expected_count'] == 0
    
    def test_forecast_activity_with_data(self):
        """Test forecasting with historical data."""
        species = 'deer'
        timestamp = datetime(2024, 1, 15, 14, 0)
        
        # Add multiple data points for the same hour
        for i in range(5):
            self.forecaster.learn_pattern(species, timestamp, count=2 + i)
        
        forecast = self.forecaster.forecast_activity(species, timestamp)
        
        assert forecast['has_forecast'] is True
        assert forecast['expected_count'] > 0
        assert 'confidence_interval_low' in forecast
        assert 'confidence_interval_high' in forecast
    
    def test_is_unexpected_activity_no_forecast(self):
        """Test unexpected activity detection without forecast."""
        is_unexpected, deviation = self.forecaster.is_unexpected_activity(
            'unknown_species',
            datetime.utcnow(),
            5
        )
        
        assert is_unexpected is False
        assert deviation == 0.0
    
    def test_is_unexpected_activity_within_range(self):
        """Test activity within expected range."""
        species = 'deer'
        timestamp = datetime(2024, 1, 15, 14, 0)
        
        # Create consistent pattern
        for i in range(5):
            self.forecaster.learn_pattern(species, timestamp, count=3)
        
        is_unexpected, deviation = self.forecaster.is_unexpected_activity(
            species, timestamp, 3
        )
        
        assert is_unexpected is False
    
    def test_is_unexpected_activity_outside_range(self):
        """Test activity outside expected range."""
        species = 'deer'
        timestamp = datetime(2024, 1, 15, 14, 0)
        
        # Create consistent pattern
        for i in range(5):
            self.forecaster.learn_pattern(species, timestamp, count=3)
        
        # Test with very different count
        is_unexpected, deviation = self.forecaster.is_unexpected_activity(
            species, timestamp, 20
        )
        
        assert is_unexpected is True
        assert deviation > 0


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
