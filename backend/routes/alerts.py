"""
Alert Management API Routes
RESTful endpoints for alert CRUD, feedback, and analytics
"""

from flask import Blueprint, request, jsonify
from flask_jwt_extended import jwt_required, get_jwt_identity
from datetime import datetime, timedelta
import logging

from models import db, Alert, AlertFeedback, AlertRule, AlertMetrics, User
from services.alert_service import AlertService

logger = logging.getLogger(__name__)

# Create blueprint
alerts_bp = Blueprint('alerts', __name__, url_prefix='/api/alerts')

# Initialize alert service
alert_service = AlertService()


@alerts_bp.route('/', methods=['GET'])
@jwt_required()
def get_alerts():
    """
    Get alerts with filtering options.
    
    Query parameters:
        - camera_id: Filter by camera
        - severity: Filter by severity (info, warning, critical, emergency)
        - resolved: Filter by resolved status (true/false)
        - limit: Maximum number of results (default 50)
        - offset: Pagination offset
        - start_date: Filter alerts after this date (ISO format)
        - end_date: Filter alerts before this date (ISO format)
    """
    try:
        # Get query parameters
        camera_id = request.args.get('camera_id', type=int)
        severity = request.args.get('severity')
        resolved = request.args.get('resolved', type=lambda x: x.lower() == 'true')
        limit = request.args.get('limit', 50, type=int)
        offset = request.args.get('offset', 0, type=int)
        start_date = request.args.get('start_date')
        end_date = request.args.get('end_date')
        
        # Build query
        query = Alert.query.filter_by(is_filtered=False)
        
        if camera_id:
            query = query.filter_by(camera_id=camera_id)
        
        if severity:
            query = query.filter_by(severity=severity)
        
        if resolved is not None:
            query = query.filter_by(resolved=resolved)
        
        if start_date:
            start_dt = datetime.fromisoformat(start_date.replace('Z', '+00:00'))
            query = query.filter(Alert.created_at >= start_dt)
        
        if end_date:
            end_dt = datetime.fromisoformat(end_date.replace('Z', '+00:00'))
            query = query.filter(Alert.created_at <= end_dt)
        
        # Get total count
        total = query.count()
        
        # Apply pagination
        alerts = query.order_by(Alert.created_at.desc()).offset(offset).limit(limit).all()
        
        return jsonify({
            'alerts': [alert.to_dict() for alert in alerts],
            'total': total,
            'limit': limit,
            'offset': offset
        }), 200
        
    except Exception as e:
        logger.error(f"Error fetching alerts: {str(e)}", exc_info=True)
        return jsonify({'error': 'Failed to fetch alerts'}), 500


@alerts_bp.route('/<int:alert_id>', methods=['GET'])
@jwt_required()
def get_alert(alert_id):
    """Get a specific alert by ID."""
    try:
        alert = Alert.query.get(alert_id)
        
        if not alert:
            return jsonify({'error': 'Alert not found'}), 404
        
        return jsonify(alert.to_dict()), 200
        
    except Exception as e:
        logger.error(f"Error fetching alert: {str(e)}", exc_info=True)
        return jsonify({'error': 'Failed to fetch alert'}), 500


@alerts_bp.route('/<int:alert_id>/acknowledge', methods=['POST'])
@jwt_required()
def acknowledge_alert(alert_id):
    """Acknowledge an alert."""
    try:
        current_user_id = get_jwt_identity()
        alert = Alert.query.get(alert_id)
        
        if not alert:
            return jsonify({'error': 'Alert not found'}), 404
        
        alert.acknowledged = True
        alert.acknowledged_by = current_user_id
        alert.acknowledged_at = datetime.utcnow()
        
        db.session.commit()
        
        return jsonify({
            'message': 'Alert acknowledged',
            'alert': alert.to_dict()
        }), 200
        
    except Exception as e:
        logger.error(f"Error acknowledging alert: {str(e)}", exc_info=True)
        db.session.rollback()
        return jsonify({'error': 'Failed to acknowledge alert'}), 500


@alerts_bp.route('/<int:alert_id>/resolve', methods=['POST'])
@jwt_required()
def resolve_alert(alert_id):
    """Mark an alert as resolved."""
    try:
        alert = Alert.query.get(alert_id)
        
        if not alert:
            return jsonify({'error': 'Alert not found'}), 404
        
        alert.resolved = True
        alert.resolved_at = datetime.utcnow()
        
        db.session.commit()
        
        return jsonify({
            'message': 'Alert resolved',
            'alert': alert.to_dict()
        }), 200
        
    except Exception as e:
        logger.error(f"Error resolving alert: {str(e)}", exc_info=True)
        db.session.rollback()
        return jsonify({'error': 'Failed to resolve alert'}), 500


@alerts_bp.route('/<int:alert_id>/feedback', methods=['POST'])
@jwt_required()
def submit_alert_feedback(alert_id):
    """
    Submit feedback on an alert for ML learning.
    
    Request body:
        - is_false_positive: boolean
        - rating: integer 1-5
        - notes: string (optional)
    """
    try:
        current_user_id = get_jwt_identity()
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No data provided'}), 400
        
        is_false_positive = data.get('is_false_positive')
        rating = data.get('rating')
        notes = data.get('notes')
        
        if is_false_positive is None:
            return jsonify({'error': 'is_false_positive is required'}), 400
        
        # Process feedback through alert service
        alert_service.process_feedback(
            alert_id=alert_id,
            user_id=current_user_id,
            is_false_positive=is_false_positive,
            rating=rating,
            notes=notes
        )
        
        return jsonify({
            'message': 'Feedback submitted successfully',
            'alert_id': alert_id
        }), 200
        
    except Exception as e:
        logger.error(f"Error submitting feedback: {str(e)}", exc_info=True)
        return jsonify({'error': 'Failed to submit feedback'}), 500


@alerts_bp.route('/rules', methods=['GET'])
@jwt_required()
def get_alert_rules():
    """Get alert rules for current user."""
    try:
        current_user_id = get_jwt_identity()
        
        rules = AlertRule.query.filter_by(user_id=current_user_id).all()
        
        return jsonify({
            'rules': [rule.to_dict() for rule in rules]
        }), 200
        
    except Exception as e:
        logger.error(f"Error fetching alert rules: {str(e)}", exc_info=True)
        return jsonify({'error': 'Failed to fetch alert rules'}), 500


@alerts_bp.route('/rules', methods=['POST'])
@jwt_required()
def create_alert_rule():
    """
    Create a new alert rule.
    
    Request body:
        - name: string (required)
        - description: string
        - camera_id: integer (optional)
        - species_filter: array of strings
        - min_confidence: float (0.0-1.0)
        - severity_levels: array of strings
        - time_of_day: array of integers (0-23)
        - days_of_week: array of integers (0-6)
        - email_enabled: boolean
        - sms_enabled: boolean
        - push_enabled: boolean
        - webhook_url: string
        - enable_ml_filtering: boolean
        - suppress_false_positives: boolean
        - batch_similar_alerts: boolean
        - batch_window_minutes: integer
    """
    try:
        current_user_id = get_jwt_identity()
        data = request.get_json()
        
        if not data or not data.get('name'):
            return jsonify({'error': 'Rule name is required'}), 400
        
        rule = AlertRule(
            user_id=current_user_id,
            camera_id=data.get('camera_id'),
            name=data['name'],
            description=data.get('description'),
            species_filter=data.get('species_filter'),
            min_confidence=data.get('min_confidence', 0.7),
            severity_levels=data.get('severity_levels'),
            time_of_day=data.get('time_of_day'),
            days_of_week=data.get('days_of_week'),
            email_enabled=data.get('email_enabled', True),
            sms_enabled=data.get('sms_enabled', False),
            push_enabled=data.get('push_enabled', True),
            webhook_url=data.get('webhook_url'),
            enable_ml_filtering=data.get('enable_ml_filtering', True),
            suppress_false_positives=data.get('suppress_false_positives', True),
            batch_similar_alerts=data.get('batch_similar_alerts', True),
            batch_window_minutes=data.get('batch_window_minutes', 30)
        )
        
        db.session.add(rule)
        db.session.commit()
        
        return jsonify({
            'message': 'Alert rule created',
            'rule': rule.to_dict()
        }), 201
        
    except Exception as e:
        logger.error(f"Error creating alert rule: {str(e)}", exc_info=True)
        db.session.rollback()
        return jsonify({'error': 'Failed to create alert rule'}), 500


@alerts_bp.route('/rules/<int:rule_id>', methods=['PUT'])
@jwt_required()
def update_alert_rule(rule_id):
    """Update an existing alert rule."""
    try:
        current_user_id = get_jwt_identity()
        rule = AlertRule.query.filter_by(id=rule_id, user_id=current_user_id).first()
        
        if not rule:
            return jsonify({'error': 'Alert rule not found'}), 404
        
        data = request.get_json()
        
        # Update fields if provided
        if 'name' in data:
            rule.name = data['name']
        if 'description' in data:
            rule.description = data['description']
        if 'camera_id' in data:
            rule.camera_id = data['camera_id']
        if 'species_filter' in data:
            rule.species_filter = data['species_filter']
        if 'min_confidence' in data:
            rule.min_confidence = data['min_confidence']
        if 'severity_levels' in data:
            rule.severity_levels = data['severity_levels']
        if 'time_of_day' in data:
            rule.time_of_day = data['time_of_day']
        if 'days_of_week' in data:
            rule.days_of_week = data['days_of_week']
        if 'email_enabled' in data:
            rule.email_enabled = data['email_enabled']
        if 'sms_enabled' in data:
            rule.sms_enabled = data['sms_enabled']
        if 'push_enabled' in data:
            rule.push_enabled = data['push_enabled']
        if 'webhook_url' in data:
            rule.webhook_url = data['webhook_url']
        if 'enable_ml_filtering' in data:
            rule.enable_ml_filtering = data['enable_ml_filtering']
        if 'suppress_false_positives' in data:
            rule.suppress_false_positives = data['suppress_false_positives']
        if 'batch_similar_alerts' in data:
            rule.batch_similar_alerts = data['batch_similar_alerts']
        if 'batch_window_minutes' in data:
            rule.batch_window_minutes = data['batch_window_minutes']
        if 'is_active' in data:
            rule.is_active = data['is_active']
        
        rule.updated_at = datetime.utcnow()
        
        db.session.commit()
        
        return jsonify({
            'message': 'Alert rule updated',
            'rule': rule.to_dict()
        }), 200
        
    except Exception as e:
        logger.error(f"Error updating alert rule: {str(e)}", exc_info=True)
        db.session.rollback()
        return jsonify({'error': 'Failed to update alert rule'}), 500


@alerts_bp.route('/rules/<int:rule_id>', methods=['DELETE'])
@jwt_required()
def delete_alert_rule(rule_id):
    """Delete an alert rule."""
    try:
        current_user_id = get_jwt_identity()
        rule = AlertRule.query.filter_by(id=rule_id, user_id=current_user_id).first()
        
        if not rule:
            return jsonify({'error': 'Alert rule not found'}), 404
        
        db.session.delete(rule)
        db.session.commit()
        
        return jsonify({'message': 'Alert rule deleted'}), 200
        
    except Exception as e:
        logger.error(f"Error deleting alert rule: {str(e)}", exc_info=True)
        db.session.rollback()
        return jsonify({'error': 'Failed to delete alert rule'}), 500


@alerts_bp.route('/analytics', methods=['GET'])
@jwt_required()
def get_alert_analytics():
    """
    Get alert analytics and metrics.
    
    Query parameters:
        - camera_id: Filter by camera
        - start_date: Start date for metrics
        - end_date: End date for metrics
        - days: Number of days to include (default 7)
    """
    try:
        camera_id = request.args.get('camera_id', type=int)
        start_date = request.args.get('start_date')
        end_date = request.args.get('end_date')
        days = request.args.get('days', 7, type=int)
        
        # Calculate date range
        if not end_date:
            end_dt = datetime.utcnow().date()
        else:
            end_dt = datetime.fromisoformat(end_date).date()
        
        if not start_date:
            start_dt = end_dt - timedelta(days=days)
        else:
            start_dt = datetime.fromisoformat(start_date).date()
        
        # Build query
        query = AlertMetrics.query.filter(
            AlertMetrics.date >= start_dt,
            AlertMetrics.date <= end_dt
        )
        
        if camera_id:
            query = query.filter_by(camera_id=camera_id)
        
        metrics = query.all()
        
        # Calculate ML accuracy
        ml_accuracy = alert_service.calculate_ml_accuracy()
        
        return jsonify({
            'metrics': [m.to_dict() for m in metrics],
            'ml_performance': ml_accuracy,
            'date_range': {
                'start': start_dt.isoformat(),
                'end': end_dt.isoformat()
            }
        }), 200
        
    except Exception as e:
        logger.error(f"Error fetching analytics: {str(e)}", exc_info=True)
        return jsonify({'error': 'Failed to fetch analytics'}), 500


@alerts_bp.route('/stats', methods=['GET'])
@jwt_required()
def get_alert_stats():
    """Get current alert statistics."""
    try:
        # Count active alerts by severity
        severity_counts = {}
        for severity in ['info', 'warning', 'critical', 'emergency']:
            count = Alert.query.filter_by(
                severity=severity,
                resolved=False,
                is_filtered=False
            ).count()
            severity_counts[severity] = count
        
        # Total alerts today
        today_start = datetime.utcnow().replace(hour=0, minute=0, second=0, microsecond=0)
        today_count = Alert.query.filter(
            Alert.created_at >= today_start
        ).count()
        
        # Unacknowledged alerts
        unacknowledged_count = Alert.query.filter_by(
            acknowledged=False,
            resolved=False,
            is_filtered=False
        ).count()
        
        # ML stats
        ml_stats = alert_service.alert_engine.alert_classifier.get_pattern_statistics()
        
        return jsonify({
            'severity_counts': severity_counts,
            'total_active': sum(severity_counts.values()),
            'today_count': today_count,
            'unacknowledged_count': unacknowledged_count,
            'ml_stats': ml_stats
        }), 200
        
    except Exception as e:
        logger.error(f"Error fetching stats: {str(e)}", exc_info=True)
        return jsonify({'error': 'Failed to fetch stats'}), 500
