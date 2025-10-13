"""
Alert Management API Routes
Enhanced alert system with ML filtering and multi-channel notifications
"""

from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from datetime import datetime, timedelta
import logging

from ..models import db, Alert, AlertFeedback, AlertPreference, User, WildlifeDetection
from ..ml.alert_ml_engine import MLAlertEngine, AlertLevel
from ..ml.alert_analytics import AlertAnalytics
from ..services.notification_service import NotificationService, NotificationChannel

logger = logging.getLogger(__name__)

# Create blueprint
alert_bp = Blueprint('alerts', __name__)

# Initialize services
ml_engine = MLAlertEngine()
analytics = AlertAnalytics()
notification_service = NotificationService()


@alert_bp.route('/alerts', methods=['GET'])
@jwt_required()
def get_alerts():
    """
    Get alerts with filtering and pagination
    """
    try:
        # Get query parameters
        severity = request.args.get('severity')
        camera_id = request.args.get('camera_id', type=int)
        acknowledged = request.args.get('acknowledged', type=bool)
        page = request.args.get('page', 1, type=int)
        per_page = request.args.get('per_page', 50, type=int)
        days = request.args.get('days', 7, type=int)
        
        # Build query
        query = Alert.query
        
        # Apply filters
        if severity:
            query = query.filter_by(severity=severity)
        if camera_id:
            query = query.filter_by(camera_id=camera_id)
        if acknowledged is not None:
            query = query.filter_by(acknowledged=acknowledged)
        
        # Time filter
        cutoff = datetime.utcnow() - timedelta(days=days)
        query = query.filter(Alert.created_at >= cutoff)
        
        # Order by priority and recency
        query = query.order_by(Alert.priority.desc(), Alert.created_at.desc())
        
        # Paginate
        pagination = query.paginate(page=page, per_page=per_page, error_out=False)
        
        return jsonify({
            'alerts': [alert.to_dict() for alert in pagination.items],
            'total': pagination.total,
            'page': page,
            'per_page': per_page,
            'pages': pagination.pages
        }), 200
        
    except Exception as e:
        logger.error(f"Error fetching alerts: {e}")
        return jsonify({'error': 'Failed to fetch alerts'}), 500


@alert_bp.route('/alerts/<int:alert_id>', methods=['GET'])
@jwt_required()
def get_alert(alert_id):
    """Get specific alert by ID"""
    try:
        alert = Alert.query.get_or_404(alert_id)
        
        # Include feedback if available
        feedback_list = [f.to_dict() for f in alert.user_feedback]
        
        result = alert.to_dict()
        result['feedback'] = feedback_list
        
        return jsonify(result), 200
        
    except Exception as e:
        logger.error(f"Error fetching alert {alert_id}: {e}")
        return jsonify({'error': 'Failed to fetch alert'}), 500


@alert_bp.route('/alerts/<int:alert_id>/acknowledge', methods=['POST'])
@jwt_required()
def acknowledge_alert(alert_id):
    """Acknowledge an alert"""
    try:
        user_id = get_jwt_identity()
        alert = Alert.query.get_or_404(alert_id)
        
        alert.acknowledged = True
        alert.acknowledged_by = user_id
        alert.acknowledged_at = datetime.utcnow()
        
        db.session.commit()
        
        return jsonify({
            'message': 'Alert acknowledged',
            'alert': alert.to_dict()
        }), 200
        
    except Exception as e:
        logger.error(f"Error acknowledging alert: {e}")
        db.session.rollback()
        return jsonify({'error': 'Failed to acknowledge alert'}), 500


@alert_bp.route('/alerts/<int:alert_id>/resolve', methods=['POST'])
@jwt_required()
def resolve_alert(alert_id):
    """Mark alert as resolved"""
    try:
        alert = Alert.query.get_or_404(alert_id)
        
        alert.resolved = True
        alert.resolved_at = datetime.utcnow()
        
        db.session.commit()
        
        return jsonify({
            'message': 'Alert resolved',
            'alert': alert.to_dict()
        }), 200
        
    except Exception as e:
        logger.error(f"Error resolving alert: {e}")
        db.session.rollback()
        return jsonify({'error': 'Failed to resolve alert'}), 500


@alert_bp.route('/alerts/<int:alert_id>/feedback', methods=['POST'])
@jwt_required()
def submit_alert_feedback(alert_id):
    """
    Submit user feedback on alert accuracy
    """
    try:
        user_id = get_jwt_identity()
        data = request.get_json()
        
        # Validate input
        if 'is_accurate' not in data:
            return jsonify({'error': 'is_accurate field is required'}), 400
        
        alert = Alert.query.get_or_404(alert_id)
        
        # Create feedback record
        feedback = AlertFeedback(
            alert_id=alert_id,
            user_id=user_id,
            is_accurate=data['is_accurate'],
            species_correction=data.get('species_correction'),
            confidence_rating=data.get('confidence_rating'),
            notes=data.get('notes')
        )
        
        db.session.add(feedback)
        db.session.commit()
        
        # Update analytics
        analytics.record_user_feedback(
            alert_id=str(alert_id),
            is_accurate=data['is_accurate'],
            species_correction=data.get('species_correction'),
            notes=data.get('notes')
        )
        
        return jsonify({
            'message': 'Feedback submitted successfully',
            'feedback': feedback.to_dict()
        }), 201
        
    except Exception as e:
        logger.error(f"Error submitting feedback: {e}")
        db.session.rollback()
        return jsonify({'error': 'Failed to submit feedback'}), 500


@alert_bp.route('/alerts/preferences', methods=['GET'])
@jwt_required()
def get_alert_preferences():
    """Get user's alert preferences"""
    try:
        user_id = get_jwt_identity()
        
        preferences = AlertPreference.query.filter_by(user_id=user_id).first()
        
        if not preferences:
            # Create default preferences
            preferences = AlertPreference(user_id=user_id)
            db.session.add(preferences)
            db.session.commit()
        
        return jsonify(preferences.to_dict()), 200
        
    except Exception as e:
        logger.error(f"Error fetching preferences: {e}")
        return jsonify({'error': 'Failed to fetch preferences'}), 500


@alert_bp.route('/alerts/preferences', methods=['PUT'])
@jwt_required()
def update_alert_preferences():
    """Update user's alert preferences"""
    try:
        user_id = get_jwt_identity()
        data = request.get_json()
        
        preferences = AlertPreference.query.filter_by(user_id=user_id).first()
        
        if not preferences:
            preferences = AlertPreference(user_id=user_id)
            db.session.add(preferences)
        
        # Update fields
        for field in ['email_enabled', 'sms_enabled', 'push_enabled', 
                     'slack_webhook', 'discord_webhook', 'custom_webhook',
                     'min_severity', 'dangerous_species_only', 'min_confidence',
                     'quiet_hours_enabled', 'quiet_start_hour', 'quiet_end_hour',
                     'max_alerts_per_hour', 'batch_alerts']:
            if field in data:
                setattr(preferences, field, data[field])
        
        preferences.updated_at = datetime.utcnow()
        db.session.commit()
        
        return jsonify({
            'message': 'Preferences updated successfully',
            'preferences': preferences.to_dict()
        }), 200
        
    except Exception as e:
        logger.error(f"Error updating preferences: {e}")
        db.session.rollback()
        return jsonify({'error': 'Failed to update preferences'}), 500


@alert_bp.route('/alerts/analytics', methods=['GET'])
@jwt_required()
def get_alert_analytics():
    """Get alert system analytics and performance metrics"""
    try:
        days = request.args.get('days', 7, type=int)
        
        # Generate comprehensive report
        report = analytics.generate_report(days=days)
        
        return jsonify(report), 200
        
    except Exception as e:
        logger.error(f"Error generating analytics: {e}")
        return jsonify({'error': 'Failed to generate analytics'}), 500


@alert_bp.route('/alerts/analytics/species', methods=['GET'])
@jwt_required()
def get_species_performance():
    """Get species-specific performance metrics"""
    try:
        species = request.args.get('species')
        
        performance = analytics.get_species_performance(species)
        
        return jsonify(performance), 200
        
    except Exception as e:
        logger.error(f"Error fetching species performance: {e}")
        return jsonify({'error': 'Failed to fetch species performance'}), 500


@alert_bp.route('/alerts/test-notification', methods=['POST'])
@jwt_required()
def test_notification():
    """
    Test notification delivery
    """
    try:
        user_id = get_jwt_identity()
        data = request.get_json()
        
        # Get user preferences
        preferences = AlertPreference.query.filter_by(user_id=user_id).first()
        user = User.query.get(user_id)
        
        if not preferences or not user:
            return jsonify({'error': 'User preferences not found'}), 404
        
        # Prepare test alert
        test_alert = {
            'species': 'Test Species',
            'confidence': 0.95,
            'alert_level': 'info',
            'camera_id': 'TEST_CAM',
            'location': 'Test Location',
            'timestamp': datetime.utcnow().isoformat(),
            'message': 'This is a test alert notification'
        }
        
        # Determine channels to use
        channels = []
        if preferences.email_enabled:
            channels.append(NotificationChannel.EMAIL)
        if preferences.slack_webhook:
            channels.append(NotificationChannel.SLACK)
        if preferences.discord_webhook:
            channels.append(NotificationChannel.DISCORD)
        
        # Prepare recipients
        recipients = [{
            'email': user.email if preferences.email_enabled else None,
            'slack_webhook': preferences.slack_webhook,
            'discord_webhook': preferences.discord_webhook
        }]
        
        # Send test notification
        result = notification_service.send_alert(
            test_alert,
            channels,
            recipients,
            priority=10  # Low priority for test
        )
        
        return jsonify({
            'message': 'Test notification sent',
            'channels': channels,
            'result': result
        }), 200
        
    except Exception as e:
        logger.error(f"Error sending test notification: {e}")
        return jsonify({'error': 'Failed to send test notification'}), 500
