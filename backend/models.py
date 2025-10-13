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
    severity = db.Column(db.String(20), default='info')  # critical, warning, info
    title = db.Column(db.String(200), nullable=False)
    message = db.Column(db.Text)
    data = db.Column(db.JSON)  # Additional alert data
    
    # Status tracking
    acknowledged = db.Column(db.Boolean, default=False)
    acknowledged_by = db.Column(db.Integer, db.ForeignKey('users.id'))
    acknowledged_at = db.Column(db.DateTime)
    resolved = db.Column(db.Boolean, default=False)
    resolved_at = db.Column(db.DateTime)
    
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'camera_id': self.camera_id,
            'detection_id': self.detection_id,
            'alert_type': self.alert_type,
            'severity': self.severity,
            'title': self.title,
            'message': self.message,
            'data': self.data,
            'acknowledged': self.acknowledged,
            'acknowledged_by': self.acknowledged_by,
            'acknowledged_at': self.acknowledged_at.isoformat() if self.acknowledged_at else None,
            'resolved': self.resolved,
            'resolved_at': self.resolved_at.isoformat() if self.resolved_at else None,
            'created_at': self.created_at.isoformat() if self.created_at else None
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

# ===== COLLABORATION MODELS =====

class UserSession(db.Model):
    """Track active user sessions for presence awareness"""
    __tablename__ = 'user_sessions'
    
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    session_id = db.Column(db.String(100), unique=True, nullable=False)
    socket_id = db.Column(db.String(100))
    ip_address = db.Column(db.String(50))
    user_agent = db.Column(db.String(500))
    last_activity = db.Column(db.DateTime, default=datetime.utcnow)
    current_page = db.Column(db.String(200))  # Current page/view
    cursor_position = db.Column(db.JSON)  # {x, y, page}
    is_active = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'user_id': self.user_id,
            'session_id': self.session_id,
            'last_activity': self.last_activity.isoformat() if self.last_activity else None,
            'current_page': self.current_page,
            'cursor_position': self.cursor_position,
            'is_active': self.is_active
        }

class Annotation(db.Model):
    """Collaborative annotations on wildlife detections"""
    __tablename__ = 'annotations'
    
    id = db.Column(db.Integer, primary_key=True)
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'), nullable=False)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    annotation_type = db.Column(db.String(30), nullable=False)  # comment, marker, region, correction
    content = db.Column(db.Text)
    position = db.Column(db.JSON)  # {x, y, width, height} or {x, y} for markers
    metadata = db.Column(db.JSON)  # Color, shape, additional data
    is_public = db.Column(db.Boolean, default=True)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'detection_id': self.detection_id,
            'user_id': self.user_id,
            'annotation_type': self.annotation_type,
            'content': self.content,
            'position': self.position,
            'metadata': self.metadata,
            'is_public': self.is_public,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

class ChatMessage(db.Model):
    """Team chat messages for collaboration"""
    __tablename__ = 'chat_messages'
    
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    channel = db.Column(db.String(50), default='general')  # general, detection-{id}, camera-{id}
    message = db.Column(db.Text, nullable=False)
    parent_id = db.Column(db.Integer, db.ForeignKey('chat_messages.id'))  # For threaded replies
    mentions = db.Column(db.JSON)  # List of user IDs mentioned
    attachments = db.Column(db.JSON)  # File attachments
    metadata = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    edited_at = db.Column(db.DateTime)
    deleted = db.Column(db.Boolean, default=False)
    
    def to_dict(self):
        return {
            'id': self.id,
            'user_id': self.user_id,
            'channel': self.channel,
            'message': self.message,
            'parent_id': self.parent_id,
            'mentions': self.mentions,
            'attachments': self.attachments,
            'metadata': self.metadata,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'edited_at': self.edited_at.isoformat() if self.edited_at else None,
            'deleted': self.deleted
        }

class SharedBookmark(db.Model):
    """Shared bookmarks and favorites for team collaboration"""
    __tablename__ = 'shared_bookmarks'
    
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'))
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    title = db.Column(db.String(200), nullable=False)
    description = db.Column(db.Text)
    tags = db.Column(db.JSON)  # List of tags
    is_shared = db.Column(db.Boolean, default=False)
    shared_with = db.Column(db.JSON)  # List of user IDs or 'all'
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'user_id': self.user_id,
            'detection_id': self.detection_id,
            'camera_id': self.camera_id,
            'title': self.title,
            'description': self.description,
            'tags': self.tags,
            'is_shared': self.is_shared,
            'shared_with': self.shared_with,
            'created_at': self.created_at.isoformat() if self.created_at else None
        }

class Task(db.Model):
    """Project tasks for field work coordination"""
    __tablename__ = 'tasks'
    
    id = db.Column(db.Integer, primary_key=True)
    title = db.Column(db.String(200), nullable=False)
    description = db.Column(db.Text)
    task_type = db.Column(db.String(30))  # maintenance, data_collection, verification, analysis
    priority = db.Column(db.String(20), default='medium')  # low, medium, high, urgent
    status = db.Column(db.String(20), default='pending')  # pending, in_progress, completed, cancelled
    assigned_to = db.Column(db.Integer, db.ForeignKey('users.id'))
    created_by = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'))
    due_date = db.Column(db.DateTime)
    completed_at = db.Column(db.DateTime)
    metadata = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'title': self.title,
            'description': self.description,
            'task_type': self.task_type,
            'priority': self.priority,
            'status': self.status,
            'assigned_to': self.assigned_to,
            'created_by': self.created_by,
            'camera_id': self.camera_id,
            'detection_id': self.detection_id,
            'due_date': self.due_date.isoformat() if self.due_date else None,
            'completed_at': self.completed_at.isoformat() if self.completed_at else None,
            'metadata': self.metadata,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }

class FieldNote(db.Model):
    """Shared field notes and observations"""
    __tablename__ = 'field_notes'
    
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    camera_id = db.Column(db.Integer, db.ForeignKey('cameras.id'))
    detection_id = db.Column(db.Integer, db.ForeignKey('wildlife_detections.id'))
    title = db.Column(db.String(200), nullable=False)
    content = db.Column(db.Text, nullable=False)
    note_type = db.Column(db.String(30))  # observation, analysis, maintenance, incident
    tags = db.Column(db.JSON)
    attachments = db.Column(db.JSON)  # Photos, videos, files
    is_shared = db.Column(db.Boolean, default=True)
    metadata = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    updated_at = db.Column(db.DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)
    
    def to_dict(self):
        return {
            'id': self.id,
            'user_id': self.user_id,
            'camera_id': self.camera_id,
            'detection_id': self.detection_id,
            'title': self.title,
            'content': self.content,
            'note_type': self.note_type,
            'tags': self.tags,
            'attachments': self.attachments,
            'is_shared': self.is_shared,
            'metadata': self.metadata,
            'created_at': self.created_at.isoformat() if self.created_at else None,
            'updated_at': self.updated_at.isoformat() if self.updated_at else None
        }