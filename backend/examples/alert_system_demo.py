#!/usr/bin/env python3
"""
Alert System Integration Demo
Demonstrates how to use the advanced ML alert system with wildlife detections
"""

import sys
import os
from datetime import datetime, timedelta
import json

# Add backend to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

# Import only ML components (don't import AlertService as it requires Flask)
from ml.alert_classifier import ContextAwareAlertEngine, AlertClassifier
from ml.anomaly_detector import BehaviorAnomalyDetector


def demo_basic_alert_creation():
    """Demonstrate basic alert creation from detection data."""
    print("\n" + "="*60)
    print("Demo 1: Basic Alert Creation")
    print("="*60)
    
    # Note: AlertService requires Flask app context, skipping instantiation
    # alert_service = AlertService()
    
    # Simulate wildlife detection
    detection_data = {
        'detection_id': 1,
        'camera_id': 1,
        'detected_species': 'bear',
        'confidence_score': 0.89,
        'timestamp': datetime.utcnow(),
        'location_name': 'North Trail Camera',
        'weather_conditions': {
            'temperature': 18.5,
            'humidity': 68.0,
            'wind_speed': 12.0,
            'visibility': 80.0
        },
        'motion_trigger_data': {
            'motion_level': 0.85,
            'motion_duration': 4.5
        }
    }
    
    print(f"\nProcessing detection:")
    print(f"  Species: {detection_data['detected_species']}")
    print(f"  Confidence: {detection_data['confidence_score']:.1%}")
    print(f"  Location: {detection_data['location_name']}")
    
    # Create alert (would be done automatically in production)
    # alert = alert_service.create_alert(detection_data)
    
    # Simulate evaluation
    engine = ContextAwareAlertEngine()
    evaluation = engine.evaluate_alert(detection_data)
    
    print(f"\nAlert Evaluation:")
    print(f"  Priority: {evaluation['priority']}")
    print(f"  ML Confidence: {evaluation['ml_confidence']:.1%}")
    print(f"  False Positive: {'Yes' if evaluation['is_false_positive'] else 'No'}")
    print(f"  Should Filter: {'Yes' if evaluation['should_filter'] else 'No'}")
    print(f"  Temporal Score: {evaluation['temporal_score']:.2f}")
    print(f"  Recommendation: {evaluation['recommendation']}")


def demo_anomaly_detection():
    """Demonstrate anomaly detection in wildlife behavior."""
    print("\n" + "="*60)
    print("Demo 2: Anomaly Detection")
    print("="*60)
    
    detector = BehaviorAnomalyDetector(window_hours=24)
    
    # Simulate historical activity
    species = 'deer'
    base_time = datetime.utcnow()
    
    print(f"\nSimulating historical {species} activity...")
    
    # Add baseline - consistent activity (3-4 detections per day)
    for day in range(7):
        for detection in range(3):
            timestamp = base_time - timedelta(days=day, hours=detection*8)
            detector.record_detection(species, timestamp, 0.8)
    
    print(f"  Recorded {len(detector.activity_history[species])} historical detections")
    
    # Test normal activity
    print(f"\nTesting normal activity (3 detections today):")
    result = detector.detect_anomaly(species, base_time)
    print(f"  Anomaly detected: {result['is_anomaly']}")
    print(f"  Anomaly score: {result['anomaly_score']:.2f}")
    print(f"  Type: {result['anomaly_type']}")
    
    # Simulate unusual high activity by adding many recent detections
    print(f"\nSimulating unusual high activity (10 detections today):")
    for i in range(10):
        timestamp = base_time - timedelta(hours=i)
        detector.record_detection(species, timestamp, 0.8)
    
    result = detector.detect_anomaly(species, base_time)
    print(f"  Anomaly detected: {result['is_anomaly']}")
    print(f"  Anomaly score: {result['anomaly_score']:.2f}")
    print(f"  Type: {result['anomaly_type']}")
    print(f"  Baseline count: {result['baseline_count']:.1f}")
    print(f"  Current count: {result['current_count']}")


def demo_ml_learning():
    """Demonstrate ML learning from user feedback."""
    print("\n" + "="*60)
    print("Demo 3: ML Learning from Feedback")
    print("="*60)
    
    from ml.alert_classifier import AlertClassifier
    
    classifier = AlertClassifier()
    
    print("\nInitial state:")
    stats = classifier.get_pattern_statistics()
    print(f"  False positive patterns: {stats['false_positive_patterns']}")
    print(f"  True positive patterns: {stats['true_positive_patterns']}")
    
    # Simulate false positive feedback
    print("\nSimulating false positive feedback (wind triggering sensor):")
    fp_detection = {
        'confidence_score': 0.45,
        'timestamp': datetime(2024, 1, 15, 23, 30),  # Night
        'detected_species': 'unknown',
        'weather_conditions': {
            'temperature': 22.0,
            'humidity': 50.0,
            'wind_speed': 35.0  # High wind
        },
        'motion_trigger_data': {
            'motion_level': 0.6,
            'motion_duration': 1.0  # Very short
        }
    }
    
    features = classifier.extract_features(fp_detection)
    classifier.learn_from_feedback(features, is_false_positive=True)
    
    print("  Added false positive pattern")
    
    # Simulate true positive feedback
    print("\nSimulating true positive feedback (actual bear detection):")
    tp_detection = {
        'confidence_score': 0.87,
        'timestamp': datetime(2024, 1, 15, 6, 30),  # Dawn
        'detected_species': 'bear',
        'weather_conditions': {
            'temperature': 18.0,
            'humidity': 65.0,
            'wind_speed': 8.0
        },
        'motion_trigger_data': {
            'motion_level': 0.85,
            'motion_duration': 5.5
        }
    }
    
    features = classifier.extract_features(tp_detection)
    classifier.learn_from_feedback(features, is_false_positive=False)
    
    print("  Added true positive pattern")
    
    # Check updated stats
    print("\nUpdated state:")
    stats = classifier.get_pattern_statistics()
    print(f"  False positive patterns: {stats['false_positive_patterns']}")
    print(f"  True positive patterns: {stats['true_positive_patterns']}")
    print(f"  Total patterns: {stats['total_patterns']}")
    
    # Test prediction on similar patterns
    print("\nTesting prediction on new similar detections:")
    
    # Test against false positive pattern
    test_fp = fp_detection.copy()
    test_fp['weather_conditions']['wind_speed'] = 40.0
    features_fp = classifier.extract_features(test_fp)
    conf, is_fp = classifier.predict_false_positive(features_fp)
    print(f"  High wind detection - FP prediction: {is_fp}, confidence: {conf:.2f}")
    
    # Test against true positive pattern
    test_tp = tp_detection.copy()
    test_tp['confidence_score'] = 0.85
    features_tp = classifier.extract_features(test_tp)
    conf, is_fp = classifier.predict_false_positive(features_tp)
    print(f"  Bear at dawn detection - FP prediction: {is_fp}, confidence: {conf:.2f}")


def demo_alert_priorities():
    """Demonstrate different alert priority levels."""
    print("\n" + "="*60)
    print("Demo 4: Alert Priority Levels")
    print("="*60)
    
    classifier = AlertClassifier()
    
    test_cases = [
        {
            'name': 'Emergency - Grizzly Bear',
            'detected_species': 'grizzly bear',
            'confidence_score': 0.92
        },
        {
            'name': 'Critical - Wolf Pack',
            'detected_species': 'wolf',
            'confidence_score': 0.81
        },
        {
            'name': 'Warning - Mountain Lion',
            'detected_species': 'mountain lion',
            'confidence_score': 0.68
        },
        {
            'name': 'Info - Common Deer',
            'detected_species': 'deer',
            'confidence_score': 0.55
        }
    ]
    
    print("\nTesting priority calculation:")
    for test in test_cases:
        priority = classifier.calculate_priority(test, ml_confidence=0.7)
        print(f"\n  {test['name']}")
        print(f"    Species: {test['detected_species']}")
        print(f"    Confidence: {test['confidence_score']:.1%}")
        print(f"    Priority: {priority.upper()}")


def demo_environmental_filtering():
    """Demonstrate environmental condition filtering."""
    print("\n" + "="*60)
    print("Demo 5: Environmental Filtering")
    print("="*60)
    
    engine = ContextAwareAlertEngine()
    
    test_conditions = [
        {
            'name': 'Normal Conditions',
            'weather_conditions': {
                'temperature': 20.0,
                'wind_speed': 10.0,
                'visibility': 80.0
            }
        },
        {
            'name': 'Extreme Cold',
            'weather_conditions': {
                'temperature': -8.0,
                'wind_speed': 15.0,
                'visibility': 70.0
            }
        },
        {
            'name': 'High Wind',
            'weather_conditions': {
                'temperature': 22.0,
                'wind_speed': 45.0,
                'visibility': 60.0
            }
        },
        {
            'name': 'Low Visibility',
            'weather_conditions': {
                'temperature': 18.0,
                'wind_speed': 8.0,
                'visibility': 5.0
            }
        }
    ]
    
    print("\nTesting environmental filtering:")
    for test in test_conditions:
        should_filter = engine._should_filter_by_environment(test)
        print(f"\n  {test['name']}")
        print(f"    Temperature: {test['weather_conditions']['temperature']}°C")
        print(f"    Wind: {test['weather_conditions']['wind_speed']} km/h")
        print(f"    Visibility: {test['weather_conditions']['visibility']}m")
        print(f"    Should Filter: {'YES' if should_filter else 'NO'}")


def demo_api_usage():
    """Demonstrate API usage patterns."""
    print("\n" + "="*60)
    print("Demo 6: API Usage Examples")
    print("="*60)
    
    print("\nExample API Calls:")
    
    print("\n1. Get Active Critical Alerts:")
    print("   GET /api/alerts?severity=critical&resolved=false")
    
    print("\n2. Submit Alert Feedback:")
    print("   POST /api/alerts/123/feedback")
    print("   Body: {")
    print('     "is_false_positive": false,')
    print('     "rating": 5,')
    print('     "notes": "Accurate detection"')
    print("   }")
    
    print("\n3. Create Alert Rule:")
    print("   POST /api/alerts/rules")
    print("   Body: {")
    print('     "name": "Bear Alerts",')
    print('     "species_filter": ["bear", "grizzly"],')
    print('     "min_confidence": 0.7,')
    print('     "severity_levels": ["critical", "emergency"],')
    print('     "email_enabled": true')
    print("   }")
    
    print("\n4. Get Alert Analytics:")
    print("   GET /api/alerts/analytics?days=7&camera_id=5")
    
    print("\n5. Acknowledge Alert:")
    print("   POST /api/alerts/123/acknowledge")


def main():
    """Run all demos."""
    print("\n" + "="*60)
    print("Advanced ML Alert System - Interactive Demo")
    print("="*60)
    
    try:
        demo_basic_alert_creation()
        demo_anomaly_detection()
        demo_ml_learning()
        demo_alert_priorities()
        demo_environmental_filtering()
        demo_api_usage()
        
        print("\n" + "="*60)
        print("Demo Complete!")
        print("="*60)
        print("\nFor more information, see:")
        print("  - Documentation: docs/ALERT_SYSTEM.md")
        print("  - Tests: tests/test_alert_system.py")
        print("  - API Routes: backend/routes/alerts.py")
        print()
        
    except Exception as e:
        print(f"\nError running demo: {str(e)}")
        import traceback
        traceback.print_exc()


if __name__ == '__main__':
    main()
