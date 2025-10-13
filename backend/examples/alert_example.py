"""Alert System Example"""
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from ml.alert_ml_engine import MLAlertEngine, AlertLevel
from ml.alert_analytics import AlertAnalytics

def process_detection():
    print("Processing Bear Detection Example\n")
    
    ml_engine = MLAlertEngine()
    analytics = AlertAnalytics()
    
    detection = {
        'species': 'bear',
        'base_confidence': 0.87,
        'bounding_box': {'x': 100, 'y': 150, 'width': 450, 'height': 550},
        'camera_id': 'CAM_001'
    }
    
    # Calculate enhanced confidence
    confidence = ml_engine.calculate_confidence_score(
        base_confidence=0.87,
        temporal_consistency=0.9,
        size_validation=0.85,
        environmental_context=0.8
    )
    
    print(f"Base Confidence: {detection['base_confidence']:.2%}")
    print(f"Enhanced Confidence: {confidence:.2%}")
    
    # Determine alert level
    alert_level = ml_engine.determine_alert_level('bear', confidence, True, 3)
    print(f"Alert Level: {alert_level.upper()}")
    
    print("\n✓ Processing Complete")

if __name__ == '__main__':
    process_detection()
