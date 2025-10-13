"""
Celery tasks for alert processing
Background jobs for alert evaluation, delivery, and analytics
"""

from celery import Celery
from datetime import datetime, timedelta
import logging
import requests
import json

from models import db, Alert, AlertRule, Camera, WildlifeDetection
from services.alert_service import AlertService

logger = logging.getLogger(__name__)

# Initialize Celery (will be configured by app)
celery = Celery('wildlife_alerts')


@celery.task(name='tasks.process_detection_alert')
def process_detection_alert(detection_id: int):
    """
    Process a wildlife detection and create alert if necessary.
    
    Args:
        detection_id: ID of the WildlifeDetection record
    """
    try:
        detection = WildlifeDetection.query.get(detection_id)
        if not detection:
            logger.warning(f"Detection {detection_id} not found")
            return {'status': 'not_found'}
        
        camera = Camera.query.get(detection.camera_id)
        
        # Prepare detection data for alert evaluation
        detection_data = {
            'detection_id': detection.id,
            'camera_id': detection.camera_id,
            'detected_species': detection.detected_species,
            'confidence_score': detection.confidence_score,
            'timestamp': detection.timestamp,
            'location_name': camera.location_name if camera else 'Unknown',
            'weather_conditions': detection.weather_conditions or {},
            'motion_trigger_data': detection.motion_trigger_data or {}
        }
        
        # Create alert service and process
        alert_service = AlertService()
        alert = alert_service.create_alert(detection_data)
        
        if alert and not alert.is_filtered:
            # Trigger notification delivery
            deliver_alert_notifications.delay(alert.id)
            
            return {
                'status': 'success',
                'alert_id': alert.id,
                'filtered': False
            }
        elif alert:
            return {
                'status': 'filtered',
                'alert_id': alert.id,
                'filtered': True,
                'reason': alert.filter_reason
            }
        else:
            return {'status': 'failed'}
            
    except Exception as e:
        logger.error(f"Error processing detection alert: {str(e)}", exc_info=True)
        return {'status': 'error', 'message': str(e)}


@celery.task(name='tasks.deliver_alert_notifications')
def deliver_alert_notifications(alert_id: int):
    """
    Deliver alert notifications to users based on their preferences.
    
    Args:
        alert_id: ID of the Alert record
    """
    try:
        alert = Alert.query.get(alert_id)
        if not alert or alert.is_filtered:
            return {'status': 'skipped', 'reason': 'filtered or not found'}
        
        # Get applicable alert rules
        rules = AlertRule.query.filter_by(
            is_active=True
        ).all()
        
        if alert.camera_id:
            # Get camera-specific rules
            camera_rules = AlertRule.query.filter_by(
                camera_id=alert.camera_id,
                is_active=True
            ).all()
            rules.extend(camera_rules)
        
        notifications_sent = 0
        
        for rule in rules:
            if _should_send_alert(alert, rule):
                # Send via configured channels
                if rule.email_enabled:
                    send_email_notification.delay(alert_id, rule.user_id)
                
                if rule.push_enabled:
                    send_push_notification.delay(alert_id, rule.user_id)
                
                if rule.webhook_url:
                    send_webhook_notification.delay(alert_id, rule.webhook_url)
                
                notifications_sent += 1
        
        logger.info(f"Queued notifications for alert {alert_id} to {notifications_sent} recipients")
        
        return {
            'status': 'success',
            'notifications_queued': notifications_sent
        }
        
    except Exception as e:
        logger.error(f"Error delivering alert notifications: {str(e)}", exc_info=True)
        return {'status': 'error', 'message': str(e)}


def _should_send_alert(alert: Alert, rule: AlertRule) -> bool:
    """Check if alert matches user's rule criteria."""
    # Check severity filter
    if rule.severity_levels:
        if alert.severity not in rule.severity_levels:
            return False
    
    # Check species filter
    if rule.species_filter and alert.detection_id:
        detection = WildlifeDetection.query.get(alert.detection_id)
        if detection and detection.detected_species not in rule.species_filter:
            return False
    
    # Check confidence threshold
    if alert.ml_confidence < rule.min_confidence:
        return False
    
    # Check time of day
    if rule.time_of_day:
        current_hour = datetime.utcnow().hour
        allowed_hours = rule.time_of_day
        if current_hour not in allowed_hours:
            return False
    
    # Check day of week
    if rule.days_of_week:
        current_day = datetime.utcnow().weekday()
        if current_day not in rule.days_of_week:
            return False
    
    # Check ML filtering preference
    if rule.suppress_false_positives and alert.false_positive_score > 0.7:
        return False
    
    return True


@celery.task(name='tasks.send_email_notification')
def send_email_notification(alert_id: int, user_id: int):
    """Send email notification for an alert."""
    # This is a placeholder - implement actual email sending
    logger.info(f"Would send email for alert {alert_id} to user {user_id}")
    return {'status': 'success', 'method': 'email'}


@celery.task(name='tasks.send_push_notification')
def send_push_notification(alert_id: int, user_id: int):
    """Send push notification for an alert."""
    # This is a placeholder - implement actual push notification
    logger.info(f"Would send push notification for alert {alert_id} to user {user_id}")
    return {'status': 'success', 'method': 'push'}


@celery.task(name='tasks.send_webhook_notification')
def send_webhook_notification(alert_id: int, webhook_url: str):
    """
    Send webhook notification for an alert.
    
    Args:
        alert_id: Alert ID
        webhook_url: Webhook endpoint URL
    """
    try:
        alert = Alert.query.get(alert_id)
        if not alert:
            return {'status': 'not_found'}
        
        # Prepare webhook payload
        payload = {
            'alert_id': alert.id,
            'alert_type': alert.alert_type,
            'severity': alert.severity,
            'priority': alert.priority,
            'title': alert.title,
            'message': alert.message,
            'camera_id': alert.camera_id,
            'detection_id': alert.detection_id,
            'ml_confidence': alert.ml_confidence,
            'anomaly_score': alert.anomaly_score,
            'timestamp': alert.created_at.isoformat() if alert.created_at else None,
            'data': alert.data
        }
        
        # Send webhook with retry logic
        max_retries = 3
        for attempt in range(max_retries):
            try:
                response = requests.post(
                    webhook_url,
                    json=payload,
                    timeout=10,
                    headers={'Content-Type': 'application/json'}
                )
                
                if response.status_code in [200, 201, 204]:
                    logger.info(f"Webhook sent successfully for alert {alert_id}")
                    return {'status': 'success', 'method': 'webhook'}
                else:
                    logger.warning(f"Webhook failed with status {response.status_code}")
                    
            except requests.exceptions.RequestException as e:
                logger.warning(f"Webhook attempt {attempt + 1} failed: {str(e)}")
                if attempt == max_retries - 1:
                    raise
        
        return {'status': 'failed', 'method': 'webhook'}
        
    except Exception as e:
        logger.error(f"Error sending webhook: {str(e)}", exc_info=True)
        return {'status': 'error', 'message': str(e)}


@celery.task(name='tasks.update_daily_metrics')
def update_daily_metrics():
    """
    Periodic task to update daily alert metrics.
    Should be scheduled to run once per day.
    """
    try:
        from models import AlertMetrics
        from services.alert_service import AlertService
        
        alert_service = AlertService()
        
        # Calculate ML accuracy
        accuracy_data = alert_service.calculate_ml_accuracy()
        
        # Update metrics for today
        today = datetime.utcnow().date()
        
        cameras = Camera.query.filter_by(status='active').all()
        
        for camera in cameras:
            metrics = AlertMetrics.query.filter_by(
                date=today,
                camera_id=camera.id
            ).first()
            
            if metrics:
                # Update ML performance metrics
                metrics.ml_accuracy = accuracy_data.get('accuracy', 0.0)
                metrics.false_positive_rate = accuracy_data.get('false_positive_rate', 0.0)
                
                db.session.commit()
        
        logger.info(f"Updated daily metrics for {len(cameras)} cameras")
        
        return {
            'status': 'success',
            'cameras_updated': len(cameras),
            'ml_accuracy': accuracy_data.get('accuracy', 0.0)
        }
        
    except Exception as e:
        logger.error(f"Error updating daily metrics: {str(e)}", exc_info=True)
        db.session.rollback()
        return {'status': 'error', 'message': str(e)}


@celery.task(name='tasks.cleanup_old_alerts')
def cleanup_old_alerts(days_to_keep: int = 90):
    """
    Clean up old resolved alerts to manage database size.
    
    Args:
        days_to_keep: Number of days to keep resolved alerts
    """
    try:
        cutoff_date = datetime.utcnow() - timedelta(days=days_to_keep)
        
        # Delete old resolved alerts
        deleted_count = Alert.query.filter(
            Alert.resolved == True,
            Alert.resolved_at < cutoff_date
        ).delete()
        
        db.session.commit()
        
        logger.info(f"Cleaned up {deleted_count} old alerts")
        
        return {
            'status': 'success',
            'deleted_count': deleted_count
        }
        
    except Exception as e:
        logger.error(f"Error cleaning up alerts: {str(e)}", exc_info=True)
        db.session.rollback()
        return {'status': 'error', 'message': str(e)}
