"""
Alert Service for Wildlife Camera System
Handles alert creation, processing, filtering, and delivery
"""

from datetime import datetime, timedelta
from typing import Dict, List, Optional
import logging
from flask import current_app
import redis
import json

from models import db, Alert, AlertFeedback, AlertRule, AlertMetrics, WildlifeDetection, Camera
from ml.alert_classifier import ContextAwareAlertEngine, AlertClassifier
from ml.anomaly_detector import BehaviorAnomalyDetector, TimeSeriesForecaster

logger = logging.getLogger(__name__)


class AlertService:
    """
    Comprehensive alert service with ML-based filtering and intelligent routing.
    """
    
    def __init__(self, redis_url: str = None):
        self.alert_engine = ContextAwareAlertEngine()
        self.anomaly_detector = BehaviorAnomalyDetector()
        self.forecaster = TimeSeriesForecaster()
        
        # Redis for state management and caching
        if redis_url:
            self.redis_client = redis.from_url(redis_url)
        else:
            self.redis_client = None
        
        # Alert correlation tracking
        self.correlation_window = timedelta(minutes=10)
        
    def create_alert(self, detection_data: Dict, alert_type: str = 'wildlife_detection') -> Optional[Alert]:
        """
        Create an alert from detection data with ML-based evaluation.
        
        Args:
            detection_data: Detection information including species, confidence, etc.
            alert_type: Type of alert
            
        Returns:
            Created Alert object or None if filtered
        """
        try:
            # Evaluate alert using ML engine
            evaluation = self.alert_engine.evaluate_alert(detection_data)
            
            # Check for anomalies
            species = detection_data.get('detected_species', 'unknown')
            timestamp = detection_data.get('timestamp', datetime.utcnow())
            if isinstance(timestamp, str):
                timestamp = datetime.fromisoformat(timestamp.replace('Z', '+00:00'))
            
            anomaly_result = self.anomaly_detector.detect_anomaly(species, timestamp)
            
            # Create alert object
            alert = Alert(
                camera_id=detection_data.get('camera_id'),
                detection_id=detection_data.get('detection_id'),
                alert_type=alert_type,
                severity=evaluation['priority'],
                priority=self._map_priority(evaluation['priority']),
                title=self._generate_title(detection_data, evaluation),
                message=self._generate_message(detection_data, evaluation, anomaly_result),
                data=detection_data,
                ml_confidence=evaluation['ml_confidence'],
                false_positive_score=1.0 if evaluation['is_false_positive'] else 0.0,
                anomaly_score=anomaly_result.get('anomaly_score', 0.0),
                context_data={
                    'temporal_score': evaluation['temporal_score'],
                    'anomaly_info': anomaly_result,
                    'recommendation': evaluation['recommendation']
                }
            )
            
            # Apply filtering
            if self._should_filter_alert(alert, evaluation):
                alert.is_filtered = True
                alert.filter_reason = evaluation['recommendation']
                logger.info(f"Alert filtered: {alert.filter_reason}")
                # Still save for analytics, but mark as filtered
            
            # Check for duplicates and correlate
            correlation_group = self._find_correlation_group(alert)
            if correlation_group:
                alert.correlation_group = correlation_group
                # Check if this is a duplicate
                duplicate_id = self._check_duplicate(alert, correlation_group)
                if duplicate_id:
                    alert.duplicate_of = duplicate_id
                    logger.info(f"Alert marked as duplicate of {duplicate_id}")
            
            # Save to database
            db.session.add(alert)
            db.session.commit()
            
            # Update anomaly detector with new detection
            confidence = detection_data.get('confidence_score', 0.5)
            self.anomaly_detector.record_detection(species, timestamp, confidence)
            
            # Cache alert state in Redis
            if self.redis_client and not alert.is_filtered:
                self._cache_alert(alert)
            
            # Update metrics
            self._update_metrics(alert)
            
            logger.info(f"Alert {alert.id} created with priority {alert.priority}, filtered={alert.is_filtered}")
            
            return alert
            
        except Exception as e:
            logger.error(f"Error creating alert: {str(e)}", exc_info=True)
            db.session.rollback()
            return None
    
    def _map_priority(self, severity: str) -> str:
        """Map severity to priority level."""
        mapping = {
            'emergency': 'high',
            'critical': 'high',
            'warning': 'normal',
            'info': 'low'
        }
        return mapping.get(severity, 'normal')
    
    def _generate_title(self, detection_data: Dict, evaluation: Dict) -> str:
        """Generate alert title."""
        species = detection_data.get('detected_species', 'Unknown')
        confidence = detection_data.get('confidence_score', 0)
        
        if evaluation['priority'] == 'emergency':
            return f"🚨 EMERGENCY: {species} detected with {confidence:.1%} confidence"
        elif evaluation['priority'] == 'critical':
            return f"⚠️ CRITICAL: {species} detected"
        elif evaluation['priority'] == 'warning':
            return f"⚡ Wildlife Alert: {species}"
        else:
            return f"ℹ️ Detection: {species}"
    
    def _generate_message(self, detection_data: Dict, evaluation: Dict, 
                         anomaly_result: Dict) -> str:
        """Generate detailed alert message."""
        species = detection_data.get('detected_species', 'Unknown')
        confidence = detection_data.get('confidence_score', 0)
        location = detection_data.get('location_name', 'Unknown location')
        
        message = f"{species} detected at {location} with {confidence:.1%} confidence.\n\n"
        
        # Add ML insights
        message += f"ML Confidence: {evaluation['ml_confidence']:.1%}\n"
        message += f"Recommendation: {evaluation['recommendation']}\n\n"
        
        # Add anomaly information
        if anomaly_result.get('is_anomaly'):
            message += f"⚠️ Anomaly detected: {anomaly_result['anomaly_type']}\n"
            message += f"Anomaly score: {anomaly_result['anomaly_score']:.2f}\n\n"
        
        # Add temporal context
        temporal_score = evaluation.get('temporal_score', 0)
        if temporal_score > 0.8:
            message += "📊 High activity period for this species\n"
        elif temporal_score < 0.3:
            message += "📊 Unusual time for this species\n"
        
        return message
    
    def _should_filter_alert(self, alert: Alert, evaluation: Dict) -> bool:
        """Determine if alert should be filtered."""
        # Filter false positives
        if evaluation['is_false_positive'] and alert.false_positive_score > 0.8:
            return True
        
        # Filter based on environmental conditions
        if evaluation['should_filter']:
            return True
        
        # Filter low-confidence info alerts
        if alert.severity == 'info' and alert.ml_confidence < 0.4:
            return True
        
        return False
    
    def _find_correlation_group(self, alert: Alert) -> Optional[str]:
        """Find correlation group for alert based on recent similar alerts."""
        if not alert.camera_id or not alert.detection_id:
            return None
        
        # Look for recent alerts from same camera with same species
        recent_cutoff = datetime.utcnow() - self.correlation_window
        
        detection = WildlifeDetection.query.get(alert.detection_id)
        if not detection:
            return None
        
        species = detection.detected_species
        
        # Find recent similar alerts
        similar_alerts = Alert.query.filter(
            Alert.camera_id == alert.camera_id,
            Alert.created_at >= recent_cutoff,
            Alert.correlation_group.isnot(None)
        ).join(WildlifeDetection).filter(
            WildlifeDetection.detected_species == species
        ).first()
        
        if similar_alerts and similar_alerts.correlation_group:
            return similar_alerts.correlation_group
        
        # Create new correlation group
        return f"CG_{alert.camera_id}_{datetime.utcnow().strftime('%Y%m%d%H%M%S')}"
    
    def _check_duplicate(self, alert: Alert, correlation_group: str) -> Optional[int]:
        """Check if alert is a duplicate within correlation group."""
        recent_cutoff = datetime.utcnow() - timedelta(minutes=5)
        
        detection = WildlifeDetection.query.get(alert.detection_id) if alert.detection_id else None
        if not detection:
            return None
        
        # Find very recent similar alert in same group
        duplicate = Alert.query.filter(
            Alert.correlation_group == correlation_group,
            Alert.created_at >= recent_cutoff,
            Alert.duplicate_of.is_(None),
            Alert.id != alert.id
        ).first()
        
        return duplicate.id if duplicate else None
    
    def _cache_alert(self, alert: Alert):
        """Cache alert in Redis for quick access."""
        if not self.redis_client:
            return
        
        try:
            key = f"alert:{alert.id}"
            value = json.dumps(alert.to_dict())
            self.redis_client.setex(key, 3600, value)  # Cache for 1 hour
        except Exception as e:
            logger.warning(f"Failed to cache alert: {str(e)}")
    
    def _update_metrics(self, alert: Alert):
        """Update daily alert metrics."""
        try:
            today = datetime.utcnow().date()
            
            metrics = AlertMetrics.query.filter_by(
                date=today,
                camera_id=alert.camera_id
            ).first()
            
            if not metrics:
                metrics = AlertMetrics(date=today, camera_id=alert.camera_id)
                db.session.add(metrics)
            
            metrics.total_alerts = (metrics.total_alerts or 0) + 1
            
            if alert.is_filtered:
                metrics.filtered_alerts = (metrics.filtered_alerts or 0) + 1
            else:
                metrics.sent_alerts = (metrics.sent_alerts or 0) + 1
            
            if alert.false_positive_score > 0.5:
                metrics.false_positive_count = (metrics.false_positive_count or 0) + 1
            
            db.session.commit()
            
        except Exception as e:
            logger.error(f"Failed to update metrics: {str(e)}")
            db.session.rollback()
    
    def process_feedback(self, alert_id: int, user_id: int, 
                        is_false_positive: bool, rating: int = None,
                        notes: str = None):
        """
        Process user feedback on an alert for ML learning.
        
        Args:
            alert_id: Alert ID
            user_id: User providing feedback
            is_false_positive: Whether alert was a false positive
            rating: Optional 1-5 rating
            notes: Optional feedback notes
        """
        try:
            alert = Alert.query.get(alert_id)
            if not alert:
                logger.warning(f"Alert {alert_id} not found for feedback")
                return
            
            # Update alert
            alert.is_false_positive = is_false_positive
            alert.user_rating = rating
            alert.feedback_notes = notes
            
            # Create feedback record
            feedback = AlertFeedback(
                alert_id=alert_id,
                user_id=user_id,
                feedback_type='false_positive' if is_false_positive else 'correct',
                rating=rating,
                is_correct=not is_false_positive,
                notes=notes
            )
            
            db.session.add(feedback)
            
            # Learn from feedback
            if alert.detection_id:
                detection = WildlifeDetection.query.get(alert.detection_id)
                if detection:
                    features = self.alert_engine.alert_classifier.extract_features({
                        'confidence_score': detection.confidence_score,
                        'timestamp': detection.timestamp,
                        'weather_conditions': detection.weather_conditions or {},
                        'motion_trigger_data': detection.motion_trigger_data or {},
                        'detected_species': detection.detected_species
                    })
                    
                    self.alert_engine.alert_classifier.learn_from_feedback(
                        features, is_false_positive
                    )
            
            db.session.commit()
            logger.info(f"Processed feedback for alert {alert_id}: false_positive={is_false_positive}")
            
        except Exception as e:
            logger.error(f"Error processing feedback: {str(e)}", exc_info=True)
            db.session.rollback()
    
    def get_active_alerts(self, camera_id: Optional[int] = None, 
                         severity: Optional[str] = None,
                         limit: int = 50) -> List[Alert]:
        """Get active (unresolved) alerts with optional filters."""
        query = Alert.query.filter_by(
            resolved=False,
            is_filtered=False
        )
        
        if camera_id:
            query = query.filter_by(camera_id=camera_id)
        
        if severity:
            query = query.filter_by(severity=severity)
        
        return query.order_by(Alert.created_at.desc()).limit(limit).all()
    
    def calculate_ml_accuracy(self) -> Dict:
        """Calculate ML model accuracy based on user feedback."""
        # Get alerts with user feedback
        feedback_alerts = Alert.query.filter(
            Alert.is_false_positive.isnot(None)
        ).all()
        
        if not feedback_alerts:
            return {
                'accuracy': 0.0,
                'false_positive_rate': 0.0,
                'total_feedback': 0
            }
        
        total = len(feedback_alerts)
        correct = 0
        false_positives = 0
        
        for alert in feedback_alerts:
            # Check if ML prediction matched user feedback
            ml_predicted_fp = alert.false_positive_score > 0.5
            user_confirmed_fp = alert.is_false_positive
            
            if ml_predicted_fp == user_confirmed_fp:
                correct += 1
            
            if user_confirmed_fp:
                false_positives += 1
        
        accuracy = correct / total if total > 0 else 0.0
        fp_rate = false_positives / total if total > 0 else 0.0
        
        return {
            'accuracy': accuracy,
            'false_positive_rate': fp_rate,
            'total_feedback': total,
            'correct_predictions': correct
        }
