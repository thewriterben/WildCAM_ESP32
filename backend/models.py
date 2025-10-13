"""
Database models for ESP32 Wildlife Camera Backend
SQLAlchemy models for PostgreSQL database
"""

from flask_sqlalchemy import SQLAlchemy
from flask_jwt_extended import JWTManager
from datetime import datetime, timedelta
from werkzeug.security import generate_password_hash, check_password_hash
import json

db = SQLAlchemy()
jwt = JWTManager()

class User(db.Model):
    """User authentication and authorization"""
    __tablename__ = 'users'
    
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    password_hash = db.Column(db.String(255), nullable=False)
    first_name = db.Column(db.String(50))
    last_name = db.Column(db.String(50))
    role = db.Column(db.String(20), default='user')  # admin, user, researcher
    is_active = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    last_login = db.Column(db.DateTime)
    
    def set_password(self, password):
        self.password_hash = generate_password_hash(password)
    
    def check_password(self, password):
        return check_password_hash(self.password_hash, password)
    
    def to_dict(self):
        return {
            'id': self.id,
            'username': self.username,
            'email': self.email,
            'first_name': self.first_name,
            'last_name': self.last_name,
            'role': self.role,
            'is_active': self.is_active,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class Camera(db.Model):
    """Camera device registration and metadata"""
    __tablename__ = 'cameras'
    
    id = db.Column(db.Integer, primary_key=True)
    device_id = db.Column(db.String(50), unique=True, nullable=False)
    name = db.Column(db.String(100))
    location_name = db.Column(db.String(200))
    latitude = db.Column(db.Float)
    longitude = db.Column(db.Float)
    altitude = db.Column(db.Float)
    last_seen = db.Column(db.DateTime)
    battery_level = db.Column(db.Float)
    solar_voltage = db.Column(db.Float)
    temperature = db.Column(db.Float)
    humidity = db.Column(db.Float)
    firmware_version = db.Column(db.String(20))
    hardware_version = db.Column(db.String(20))
    config = db.Column(db.JSON)
    status = db.Column(db.String(20), default='active')  # active, inactive, maintenance
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Relationships
    detections = db.relationship('WildlifeDetection', backref='camera', lazy='dynamic', cascade='all, delete-orphan')
    alerts = db.relationship('Alert', backref='camera', lazy='dynamic')
    
    def to_dict(self):
        return {
            'id': self.id,
            'device_id': self.device_id,
            'name': self.name,
            'location_name': self.location_name,
            'latitude': self.latitude,
            'longitude': self.longitude,
            'altitude': self.altitude,
            'last_seen': self.last_seen.isoformat() if self.last_seen else None,
            'battery_level': self.battery_level,
            'solar_voltage': self.solar_voltage,
            'temperature': self.temperature,
            'humidity': self.humidity,
            'firmware_version': self.firmware_version,
            'hardware_version': self.hardware_version,
            'config': self.config,
            'status': self.status,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class Species(db.Model):
    """Wildlife species reference data"""
    __tablename__ = 'species'
    
    id = db.Column(db.Integer, primary_key=True)
    common_name = db.Column(db.String(100), nullable=False)
    scientific_name = db.Column(db.String(100))
    category = db.Column(db.String(50))  # mammal, bird, reptile, etc.
    conservation_status = db.Column(db.String(30))  # endangered, threatened, etc.
    description = db.Column(db.Text)
    habitat = db.Column(db.String(200))
    behavior_notes = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    # Relationships
    detections = db.relationship('WildlifeDetection', backref='species_ref', lazy='dynamic')
    
    def to_dict(self):
        return {
            'id': self.id,
            'common_name': self.common_name,
            'scientific_name': self.scientific_name,
            'category': self.category,
            'conservation_status': self.conservation_status,
            'description': self.description,
            'habitat': self.habitat,
            'behavior_notes': self.behavior_notes
        }

class WildlifeDetection(db.Model):
    """Wildlife detection records with AI classification"""
    __tablename__ = 'wildlife_detections'
    
    id = db.Column(db.Integer, primary_key=True)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'), nullable=False)
    species_id = db.Column(db.Integer, db.ForeignKey('species.id'))
    timestamp = db.Column(db.DateTime, default=datetime.utcnow, nullable=False)
    image_url = db.Column(db.String(500))
    thumbnail_url = db.Column(db.String(500))
    
    # AI Classification results
    detected_species = db.Column(db.String(100))  # AI detected species
    confidence_score = db.Column(db.Float)  # AI confidence (0.0-1.0)
    bounding_box = db.Column(db.JSON)  # {x, y, width, height}
    additional_detections = db.Column(db.JSON)  # Multiple detections in same image
    
    # Manual verification
    verified = db.Column(db.Boolean, default=False)
    verified_species = db.Column(db.String(100))
    verified_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    verification_notes = db.Column(db.Text)
    verification_timestamp = db.Column(db.DateTime)
    
    # Context data
    weather_conditions = db.Column(db.JSON)
    motion_trigger_data = db.Column(db.JSON)
    metadata = db.Column(db.JSON)
    
    # Flags
    is_false_positive = db.Column(db.Boolean, default=False)
    is_research_grade = db.Column(db.Boolean, default=False)
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'camera_id': self.camera_id,
            'species_id': self.species_id,
            'timestamp': self.timestamp.isoformat() if self.timestamp else None,
            'image_url': self.image_url,
            'thumbnail_url': self.thumbnail_url,
            'detected_species': self.detected_species,
            'confidence_score': self.confidence_score,
            'bounding_box': self.bounding_box,
            'additional_detections': self.additional_detections,
            'verified': self.verified,
            'verified_species': self.verified_species,
            'verified_by': self.verified_by,
            'verification_notes': self.verification_notes,
            'verification_timestamp': self.verification_timestamp.isoformat() if self.verification_timestamp else None,
            'weather_conditions': self.weather_conditions,
            'motion_trigger_data': self.motion_trigger_data,
            'metadata': self.metadata,
            'is_false_positive': self.is_false_positive,
            'is_research_grade': self.is_research_grade,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class Alert(db.Model):
    """System alerts and notifications"""
    __tablename__ = 'alerts'
    
    id = db.Column(db.Integer, primary_key=True)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'))
    alert_type = db.Column(db.String(50), nullable=False)  # rare_species, low_battery, offline, etc.
    severity = db.Column(db.String(20), default='info')  # emergency, critical, warning, info
    priority = db.Column(db.String(20), default='normal')  # high, normal, low
    title = db.Column(db.String(200), nullable=False)
    message = db.Column(db.Text)
    data = db.Column(db.JSON)  # Additional alert data
    
    # ML-enhanced fields
    ml_confidence = db.Column(db.Float, default=0.5)  # ML confidence score
    false_positive_score = db.Column(db.Float, default=0.0)  # Probability of false positive
    anomaly_score = db.Column(db.Float, default=0.0)  # Anomaly detection score
    context_data = db.Column(db.JSON)  # Environmental and temporal context
    
    # Alert filtering and deduplication
    is_filtered = db.Column(db.Boolean, default=False)  # Whether alert was filtered out
    filter_reason = db.Column(db.String(200))  # Reason for filtering
    duplicate_of = db.Column(db.Integer, db.ForeignKey('alerts.id'))  # Reference to original alert
    correlation_group = db.Column(db.String(50))  # Group ID for correlated alerts
    
    # Status tracking
    acknowledged = db.Column(db.Boolean, default=False)
    acknowledged_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    acknowledged_at = db.Column(db.DateTime)
    resolved = db.Column(db.Boolean, default=False)
    resolved_at = db.Column(db.DateTime)
    
    # User feedback for learning
    user_rating = db.Column(db.Integer)  # 1-5 rating of alert usefulness
    is_false_positive = db.Column(db.Boolean)  # User-confirmed false positive
    feedback_notes = db.Column(db.Text)
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    # Relationships
    feedback = db.relationship('AlertFeedback', backref='alert', lazy='dynamic')
    
    def to_dict(self):
        return {
            'id': self.id,
            'camera_id': self.camera_id,
            'detection_id': self.detection_id,
            'alert_type': self.alert_type,
            'severity': self.severity,
            'priority': self.priority,
            'title': self.title,
            'message': self.message,
            'data': self.data,
            'ml_confidence': self.ml_confidence,
            'false_positive_score': self.false_positive_score,
            'anomaly_score': self.anomaly_score,
            'context_data': self.context_data,
            'is_filtered': self.is_filtered,
            'filter_reason': self.filter_reason,
            'duplicate_of': self.duplicate_of,
            'correlation_group': self.correlation_group,
            'acknowledged': self.acknowledged,
            'acknowledged_by': self.acknowledged_by,
            'acknowledged_at': self.acknowledged_at.isoformat() if self.acknowledged_at else None,
            'resolved': self.resolved,
            'resolved_at': self.resolved_at.isoformat() if self.resolved_at else None,
            'user_rating': self.user_rating,
            'is_false_positive': self.is_false_positive,
            'feedback_notes': self.feedback_notes,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

class AnalyticsData(db.Model):
    """Aggregated analytics data for dashboard"""
    __tablename__ = 'analytics_data'
    
    id = db.Column(db.Integer, primary_key=True)
    metric_type = db.Column(db.String(50), nullable=False)  # species_count, activity_pattern, etc.
    time_period = db.Column(db.String(20), nullable=False)  # hourly, daily, weekly, monthly
    timestamp = db.Column(db.DateTime, nullable=False)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    species_id = db.Column(db.Integer, db.ForeignKey('species.id'))
    value = db.Column(db.Float, nullable=False)
    metadata = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'metric_type': self.metric_type,
            'time_period': self.time_period,
            'timestamp': self.timestamp.isoformat() if self.timestamp else None,
            'camera_id': self.camera_id,
            'species_id': self.species_id,
            'value': self.value,
            'metadata': self.metadata,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class AlertFeedback(db.Model):
    """User feedback on alerts for machine learning improvement"""
    __tablename__ = 'alert_feedback'
    
    id = db.Column(db.Integer, primary_key=True)
    alert_id = db.Column(db.Integer, db.ForeignKey('alerts.id'), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    feedback_type = db.Column(db.String(50), nullable=False)  # false_positive, useful, not_useful, etc.
    rating = db.Column(db.Integer)  # 1-5 rating
    is_correct = db.Column(db.Boolean)  # Was the alert classification correct
    notes = db.Column(db.Text)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'alert_id': self.alert_id,
            'user_id': self.user_id,
            'feedback_type': self.feedback_type,
            'rating': self.rating,
            'is_correct': self.is_correct,
            'notes': self.notes,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class AlertRule(db.Model):
    """Customizable alert rules and preferences"""
    __tablename__ = 'alert_rules'
    
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))  # Optional: specific camera
    name = db.Column(db.String(100), nullable=False)
    description = db.Column(db.Text)
    
    # Rule conditions
    species_filter = db.Column(db.JSON)  # List of species to alert on
    min_confidence = db.Column(db.Float, default=0.7)
    severity_levels = db.Column(db.JSON)  # List of severity levels to include
    time_of_day = db.Column(db.JSON)  # Time windows for alerts
    days_of_week = db.Column(db.JSON)  # Days to receive alerts
    
    # Alert delivery preferences
    email_enabled = db.Column(db.Boolean, default=True)
    sms_enabled = db.Column(db.Boolean, default=False)
    push_enabled = db.Column(db.Boolean, default=True)
    webhook_url = db.Column(db.String(500))
    
    # Smart filtering
    enable_ml_filtering = db.Column(db.Boolean, default=True)
    suppress_false_positives = db.Column(db.Boolean, default=True)
    batch_similar_alerts = db.Column(db.Boolean, default=True)
    batch_window_minutes = db.Column(db.Integer, default=30)
    
    is_active = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'user_id': self.user_id,
            'camera_id': self.camera_id,
            'name': self.name,
            'description': self.description,
            'species_filter': self.species_filter,
            'min_confidence': self.min_confidence,
            'severity_levels': self.severity_levels,
            'time_of_day': self.time_of_day,
            'days_of_week': self.days_of_week,
            'email_enabled': self.email_enabled,
            'sms_enabled': self.sms_enabled,
            'push_enabled': self.push_enabled,
            'webhook_url': self.webhook_url,
            'enable_ml_filtering': self.enable_ml_filtering,
            'suppress_false_positives': self.suppress_false_positives,
            'batch_similar_alerts': self.batch_similar_alerts,
            'batch_window_minutes': self.batch_window_minutes,
            'is_active': self.is_active,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

class AlertMetrics(db.Model):
    """Alert effectiveness metrics and analytics"""
    __tablename__ = 'alert_metrics'
    
    id = db.Column(db.Integer, primary_key=True)
    date = db.Column(db.Date, nullable=False)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    
    # Volume metrics
    total_alerts = db.Column(db.Integer, default=0)
    filtered_alerts = db.Column(db.Integer, default=0)
    sent_alerts = db.Column(db.Integer, default=0)
    
    # Quality metrics
    false_positive_count = db.Column(db.Integer, default=0)
    true_positive_count = db.Column(db.Integer, default=0)
    user_confirmed_count = db.Column(db.Integer, default=0)
    
    # Response metrics
    avg_acknowledgment_time_seconds = db.Column(db.Float)
    avg_resolution_time_seconds = db.Column(db.Float)
    unacknowledged_count = db.Column(db.Integer, default=0)
    
    # ML performance
    ml_accuracy = db.Column(db.Float)
    false_positive_rate = db.Column(db.Float)
    false_negative_rate = db.Column(db.Float)
    
    metadata = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'date': self.date.isoformat() if self.date else None,
            'camera_id': self.camera_id,
            'total_alerts': self.total_alerts,
            'filtered_alerts': self.filtered_alerts,
            'sent_alerts': self.sent_alerts,
            'false_positive_count': self.false_positive_count,
            'true_positive_count': self.true_positive_count,
            'user_confirmed_count': self.user_confirmed_count,
            'avg_acknowledgment_time_seconds': self.avg_acknowledgment_time_seconds,
            'avg_resolution_time_seconds': self.avg_resolution_time_seconds,
            'unacknowledged_count': self.unacknowledged_count,
            'ml_accuracy': self.ml_accuracy,
            'false_positive_rate': self.false_positive_rate,
            'false_negative_rate': self.false_negative_rate,
            'metadata': self.metadata,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }