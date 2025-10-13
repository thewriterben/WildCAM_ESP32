"""
Tests for ML-Enhanced Alert System
"""

import pytest
import sys
import os

# Add backend to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'backend'))

from ml.alert_ml_engine import MLAlertEngine, AlertLevel
from ml.alert_analytics import AlertAnalytics
from services.notification_service import NotificationService, NotificationChannel
from datetime import datetime


class TestMLAlertEngine:
    """Test ML Alert Engine functionality"""
    
    def setup_method(self):
        """Setup for each test"""
        self.engine = MLAlertEngine()
    
    def test_confidence_scoring(self):
        """Test enhanced confidence score calculation"""
        score = self.engine.calculate_confidence_score(
            base_confidence=0.8,
            temporal_consistency=0.9,
            size_validation=0.85,
            environmental_context=0.75
        )
        
        assert 0.0 <= score <= 1.0
        assert score > 0.7  # Should be reasonably high with these inputs
        print(f"✓ Confidence scoring: {score:.3f}")
    
    def test_temporal_consistency(self):
        """Test temporal consistency checking"""
        current = {'species': 'bear', 'confidence': 0.8}
        history = [
            {'species': 'bear', 'confidence': 0.75},
            {'species': 'bear', 'confidence': 0.82},
            {'species': 'deer', 'confidence': 0.7}
        ]
        
        consistency = self.engine.check_temporal_consistency(current, history)
        
        assert 0.0 <= consistency <= 1.0
        assert consistency > 0.5  # Should be high with 2/3 matching
        print(f"✓ Temporal consistency: {consistency:.3f}")
    
    def test_object_size_validation(self):
        """Test object size validation"""
        image_dims = (1920, 1080)
        
        # Test bear with appropriate size (20% of image)
        bbox_bear = {'x': 100, 'y': 100, 'width': 400, 'height': 500}
        score_bear = self.engine.validate_object_size(bbox_bear, 'bear', image_dims)
        
        # Test deer with reasonable size
        bbox_deer = {'x': 100, 'y': 100, 'width': 300, 'height': 400}
        score_deer = self.engine.validate_object_size(bbox_deer, 'deer', image_dims)
        
        # Both should return valid scores
        assert 0.0 <= score_bear <= 1.0
        assert 0.0 <= score_deer <= 1.0
        
        print(f"✓ Size validation - bear: {score_bear:.3f}, deer: {score_deer:.3f}")
    
    def test_environmental_context(self):
        """Test environmental context evaluation"""
        # Nocturnal animal at night
        score_night = self.engine.evaluate_environmental_context(
            'raccoon', 'night', {'temperature': 15}
        )
        
        # Same animal during day
        score_day = self.engine.evaluate_environmental_context(
            'raccoon', 'day', {'temperature': 15}
        )
        
        assert score_night > score_day
        print(f"✓ Environmental context - night: {score_night:.3f}, day: {score_day:.3f}")
    
    def test_false_positive_prediction(self):
        """Test false positive prediction"""
        # Good detection
        detection_good = {'bounding_box': {'width': 300, 'height': 400}}
        is_fp_good = self.engine.predict_false_positive(
            detection_good, 0.85, 0.9
        )
        assert not is_fp_good
        
        # Poor detection
        detection_poor = {'bounding_box': {'width': 10, 'height': 10}}
        is_fp_poor = self.engine.predict_false_positive(
            detection_poor, 0.45, 0.2
        )
        assert is_fp_poor
        
        print(f"✓ False positive prediction - good: {is_fp_good}, poor: {is_fp_poor}")
    
    def test_alert_level_determination(self):
        """Test alert level determination"""
        # Emergency level for dangerous species
        level_emergency = self.engine.determine_alert_level(
            'bear', 0.9, True, 3
        )
        assert level_emergency == AlertLevel.EMERGENCY
        
        # Info level for non-dangerous
        level_info = self.engine.determine_alert_level(
            'deer', 0.8, False, 1
        )
        assert level_info == AlertLevel.INFO
        
        print(f"✓ Alert levels - emergency: {level_emergency}, info: {level_info}")
    
    def test_threshold_adjustment(self):
        """Test dynamic threshold adjustment"""
        # High false positive rate should increase threshold
        threshold_high_fp = self.engine.adjust_threshold_dynamically(
            recent_false_positives=30,
            recent_true_positives=10,
            time_of_day='day'
        )
        assert threshold_high_fp > 0.7
        
        # Low false positive rate should decrease threshold
        threshold_low_fp = self.engine.adjust_threshold_dynamically(
            recent_false_positives=2,
            recent_true_positives=50,
            time_of_day='day'
        )
        assert threshold_low_fp <= 0.7
        
        print(f"✓ Threshold adjustment - high FP: {threshold_high_fp:.3f}, low FP: {threshold_low_fp:.3f}")


class TestAlertAnalytics:
    """Test Alert Analytics functionality"""
    
    def setup_method(self):
        """Setup for each test"""
        self.analytics = AlertAnalytics()
    
    def test_track_alert(self):
        """Test alert tracking"""
        alert_data = {
            'species': 'bear',
            'confidence': 0.85,
            'alert_level': 'critical',
            'camera_id': 'CAM_001'
        }
        
        self.analytics.track_alert(alert_data, 150.0, {'email': {'success': True}})
        
        assert self.analytics.total_alerts == 1
        assert len(self.analytics.alert_history) == 1
        print("✓ Alert tracking works")
    
    def test_user_feedback(self):
        """Test user feedback recording"""
        self.analytics.record_user_feedback(
            alert_id='alert_123',
            is_accurate=True,
            species_correction=None,
            notes='Correct detection'
        )
        
        assert self.analytics.user_feedback_count == 1
        assert self.analytics.true_positives == 1
        print("✓ User feedback recording works")
    
    def test_accuracy_metrics(self):
        """Test accuracy metrics calculation"""
        # Record some feedback
        for i in range(10):
            self.analytics.record_user_feedback(
                f'alert_{i}',
                is_accurate=(i % 4 != 0),  # 75% accuracy
                species_correction=None
            )
        
        metrics = self.analytics.get_accuracy_metrics(days=7)
        
        assert 'accuracy' in metrics
        assert 0.7 <= metrics['accuracy'] <= 0.8  # Should be around 75%
        print(f"✓ Accuracy metrics: {metrics['accuracy']:.3f}")
    
    def test_performance_metrics(self):
        """Test performance metrics"""
        # Create fresh analytics instance
        analytics_fresh = AlertAnalytics()
        
        # Track some alerts
        for i in range(5):
            alert_data = {
                'species': 'deer',
                'confidence': 0.8,
                'alert_level': 'info',
                'camera_id': 'CAM_001'
            }
            analytics_fresh.track_alert(alert_data, 100.0 + i * 10, {})
        
        metrics = analytics_fresh.get_performance_metrics()
        
        assert metrics['total_alerts'] == 5
        assert 'avg_processing_time_ms' in metrics
        print(f"✓ Performance metrics - alerts: {metrics['total_alerts']}, avg time: {metrics['avg_processing_time_ms']:.2f}ms")
    
    def test_anomaly_detection(self):
        """Test anomaly detection"""
        # Create normal pattern
        for i in range(20):
            alert_data = {
                'species': 'deer',
                'confidence': 0.8,
                'alert_level': 'info',
                'camera_id': 'CAM_001'
            }
            self.analytics.track_alert(alert_data, 100.0, {})
        
        anomalies = self.analytics.detect_anomalies(window_hours=24)
        
        # Should detect something or return empty list
        assert isinstance(anomalies, list)
        print(f"✓ Anomaly detection - found {len(anomalies)} anomalies")
    
    def test_threshold_recommendations(self):
        """Test threshold adjustment recommendations"""
        # Add some performance data
        for i in range(30):
            self.analytics.record_user_feedback(
                f'alert_{i}',
                is_accurate=(i % 3 != 0),  # 66% accuracy
                species_correction=None
            )
        
        recommendations = self.analytics.recommend_threshold_adjustments()
        
        assert 'adjustments' in recommendations
        assert isinstance(recommendations['adjustments'], list)
        print(f"✓ Threshold recommendations - {len(recommendations['adjustments'])} suggestions")


class TestNotificationService:
    """Test Notification Service functionality"""
    
    def setup_method(self):
        """Setup for each test"""
        self.service = NotificationService()
    
    def test_rate_limiting(self):
        """Test rate limiting"""
        # Should allow first alert
        allowed1 = self.service._check_rate_limit('test_camera')
        assert allowed1
        
        # Artificially set high count
        self.service.rate_limits['test_camera']['count'] = 60
        
        # Should block next alert
        allowed2 = self.service._check_rate_limit('test_camera')
        assert not allowed2
        
        print("✓ Rate limiting works")
    
    def test_duplicate_detection(self):
        """Test duplicate alert detection"""
        alert = {
            'species': 'bear',
            'camera_id': 'CAM_001',
            'alert_level': 'critical'
        }
        
        # First alert should not be duplicate
        is_dup1 = self.service._is_duplicate(alert)
        assert not is_dup1
        
        # Track it
        self.service._track_notification(alert)
        
        # Same alert should be duplicate
        is_dup2 = self.service._is_duplicate(alert)
        assert is_dup2
        
        print("✓ Duplicate detection works")
    
    def test_message_formatting(self):
        """Test message formatting"""
        alert = {
            'species': 'Black Bear',
            'confidence': 0.87,
            'alert_level': 'critical',
            'camera_id': 'CAM_001',
            'location': 'Trail A',
            'timestamp': datetime.utcnow().isoformat()
        }
        
        # Test subject
        subject = self.service._format_subject(alert)
        assert 'CRITICAL' in subject
        assert 'Black Bear' in subject
        
        # Test text body
        body = self.service._format_text_body(alert)
        assert 'Species: Black Bear' in body
        assert '87' in body  # Confidence
        
        print("✓ Message formatting works")


def run_tests():
    """Run all tests"""
    print("\n" + "="*60)
    print("Testing ML-Enhanced Alert System")
    print("="*60 + "\n")
    
    # Test ML Engine
    print("Testing ML Alert Engine...")
    test_engine = TestMLAlertEngine()
    test_engine.setup_method()
    test_engine.test_confidence_scoring()
    test_engine.test_temporal_consistency()
    test_engine.test_object_size_validation()
    test_engine.test_environmental_context()
    test_engine.test_false_positive_prediction()
    test_engine.test_alert_level_determination()
    test_engine.test_threshold_adjustment()
    
    print("\nTesting Alert Analytics...")
    test_analytics = TestAlertAnalytics()
    test_analytics.setup_method()
    test_analytics.test_track_alert()
    test_analytics.test_user_feedback()
    test_analytics.test_accuracy_metrics()
    test_analytics.test_performance_metrics()
    test_analytics.test_anomaly_detection()
    test_analytics.test_threshold_recommendations()
    
    print("\nTesting Notification Service...")
    test_notification = TestNotificationService()
    test_notification.setup_method()
    test_notification.test_rate_limiting()
    test_notification.test_duplicate_detection()
    test_notification.test_message_formatting()
    
    print("\n" + "="*60)
    print("All tests passed! ✓")
    print("="*60 + "\n")


if __name__ == '__main__':
    run_tests()
