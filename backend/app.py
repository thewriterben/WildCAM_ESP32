"""
ESP32 Wildlife Camera - Backend API
Handles data ingestion, storage, and analysis
"""

from flask import Flask, request, jsonify, send_file
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
from datetime import datetime, timedelta
import boto3
import cv2
import numpy as np
from werkzeug.utils import secure_filename
import os
from celery import Celery
import redis

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = os.environ.get('DATABASE_URL', 
    'postgresql://wildlife:wildlife@localhost/wildlife_db')
app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'dev-secret-key')
app.config['UPLOAD_FOLDER'] = 'uploads'
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max file size

db = SQLAlchemy(app)
CORS(app)

# Celery for async tasks
celery = Celery(app.name, broker=os.environ.get('REDIS_URL', 'redis://localhost:6379'))

# S3 client for image storage
s3_client = boto3.client('s3',
    aws_access_key_id=os.environ.get('AWS_ACCESS_KEY_ID'),
    aws_secret_access_key=os.environ.get('AWS_SECRET_ACCESS_KEY')
)

# Database Models
class Camera(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    device_id = db.Column(db.String(50), unique=True, nullable=False)
    name = db.Column(db.String(100))
    location = db.Column(db.JSON)
    last_seen = db.Column(db.DateTime)
    battery_level = db.Column(db.Float)
    firmware_version = db.Column(db.String(20))
    config = db.Column(db.JSON)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)
    
    detections = db.relationship('Detection', backref='camera', lazy='dynamic')

class Detection(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    camera_id = db.Column(db.Integer, db.ForeignKey('camera.id'))
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)
    image_url = db.Column(db.String(500))
    thumbnail_url = db.Column(db.String(500))
    species = db.Column(db.String(100))
    confidence = db.Column(db.Float)
    bounding_box = db.Column(db.JSON)
    metadata = db.Column(db.JSON)
    verified = db.Column(db.Boolean, default=False)
    verified_species = db.Column(db.String(100))
    verified_by = db.Column(db.String(100))

class Alert(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    detection_id = db.Column(db.Integer, db.ForeignKey('detection.id'))
    alert_type = db.Column(db.String(50))
    message = db.Column(db.Text)
    sent = db.Column(db.Boolean, default=False)
    created_at = db.Column(db.DateTime, default=datetime.utcnow)

# API Routes
@app.route('/api/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'timestamp': datetime.utcnow().isoformat()
    })

@app.route('/api/cameras/register', methods=['POST'])
def register_camera():
    """Register a new camera device"""
    data = request.json
    
    # Check if camera exists
    camera = Camera.query.filter_by(device_id=data['device_id']).first()
    
    if not camera:
        camera = Camera(
            device_id=data['device_id'],
            name=data.get('name', f"Camera {data['device_id'][-6:]}"),
            location=data.get('location'),
            firmware_version=data.get('firmware_version')
        )
        db.session.add(camera)
    else:
        # Update existing camera
        camera.last_seen = datetime.utcnow()
        camera.battery_level = data.get('battery_level')
        camera.firmware_version = data.get('firmware_version')
    
    db.session.commit()
    
    return jsonify({
        'camera_id': camera.id,
        'status': 'registered'
    }), 201

@app.route('/api/detections', methods=['POST'])
def upload_detection():
    """Upload a new wildlife detection"""
    # Get camera
    device_id = request.form.get('device_id')
    camera = Camera.query.filter_by(device_id=device_id).first()
    
    if not camera:
        return jsonify({'error': 'Camera not registered'}), 404
    
    # Update camera last seen
    camera.last_seen = datetime.utcnow()
    camera.battery_level = float(request.form.get('battery', 0))
    
    # Handle image upload
    image_file = request.files.get('image')
    if not image_file:
        return jsonify({'error': 'No image provided'}), 400
    
    # Save image
    filename = secure_filename(f"{device_id}_{datetime.utcnow().timestamp()}.jpg")
    image_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
    image_file.save(image_path)
    
    # Upload to S3
    s3_url = upload_to_s3.delay(image_path, filename).get()
    
    # Create thumbnail
    thumbnail_url = create_thumbnail.delay(image_path).get()
    
    # Parse metadata
    metadata = request.form.get('metadata', '{}')
    import json
    metadata = json.loads(metadata)
    
    # Create detection record
    detection = Detection(
        camera_id=camera.id,
        timestamp=datetime.fromisoformat(metadata.get('timestamp', 
                                                      datetime.utcnow().isoformat())),
        image_url=s3_url,
        thumbnail_url=thumbnail_url,
        species=metadata.get('species'),
        confidence=metadata.get('confidence'),
        bounding_box=metadata.get('bbox'),
        metadata=metadata
    )
    
    db.session.add(detection)
    db.session.commit()
    
    # Check for alerts
    check_for_alerts.delay(detection.id)
    
    return jsonify({
        'detection_id': detection.id,
        'status': 'success'
    }), 201

@app.route('/api/detections', methods=['GET'])
def get_detections():
    """Get detection history with filters"""
    # Parse query parameters
    camera_id = request.args.get('camera_id')
    species = request.args.get('species')
    start_date = request.args.get('start_date')
    end_date = request.args.get('end_date')
    verified_only = request.args.get('verified_only', 'false').lower() == 'true'
    page = int(request.args.get('page', 1))
    per_page = int(request.args.get('per_page', 50))
    
    # Build query
    query = Detection.query
    
    if camera_id:
        query = query.filter_by(camera_id=camera_id)
    if species:
        query = query.filter_by(species=species)
    if start_date:
        query = query.filter(Detection.timestamp >= datetime.fromisoformat(start_date))
    if end_date:
        query = query.filter(Detection.timestamp <= datetime.fromisoformat(end_date))
    if verified_only:
        query = query.filter_by(verified=True)
    
    # Paginate results
    paginated = query.order_by(Detection.timestamp.desc()).paginate(
        page=page, per_page=per_page, error_out=False
    )
    
    # Format response
    detections = []
    for d in paginated.items:
        detections.append({
            'id': d.id,
            'camera_id': d.camera_id,
            'timestamp': d.timestamp.isoformat(),
            'image_url': d.image_url,
            'thumbnail_url': d.thumbnail_url,
            'species': d.species,
            'confidence': d.confidence,
            'verified': d.verified,
            'metadata': d.metadata
        })
    
    return jsonify({
        'detections': detections,
        'total': paginated.total,
        'page': page,
        'per_page': per_page,
        'total_pages': paginated.pages
    })

@app.route('/api/analytics/species', methods=['GET'])
def species_analytics():
    """Get species distribution analytics"""
    days = int(request.args.get('days', 30))
    start_date = datetime.utcnow() - timedelta(days=days)
    
    # Query species counts
    results = db.session.query(
        Detection.species,
        db.func.count(Detection.id).label('count')
    ).filter(
        Detection.timestamp >= start_date
    ).group_by(Detection.species).all()
    
    # Format response
    species_data = [
        {'species': r.species, 'count': r.count}
        for r in results if r.species
    ]
    
    return jsonify({
        'period_days': days,
        'species_distribution': species_data,
        'total_detections': sum(r.count for r in results)
    })

@app.route('/api/analytics/activity', methods=['GET'])
def activity_patterns():
    """Get temporal activity patterns"""
    species = request.args.get('species')
    days = int(request.args.get('days', 30))
    start_date = datetime.utcnow() - timedelta(days=days)
    
    # Query hourly activity
    query = db.session.query(
        db.func.extract('hour', Detection.timestamp).label('hour'),
        db.func.count(Detection.id).label('count')
    ).filter(Detection.timestamp >= start_date)
    
    if species:
        query = query.filter(Detection.species == species)
    
    results = query.group_by('hour').all()
    
    # Create 24-hour array
    hourly_activity = [0] * 24
    for r in results:
        hourly_activity[int(r.hour)] = r.count
    
    return jsonify({
        'species': species or 'all',
        'period_days': days,
        'hourly_activity': hourly_activity
    })

# Async Tasks
@celery.task
def upload_to_s3(image_path, filename):
    """Upload image to S3 bucket"""
    bucket_name = os.environ.get('S3_BUCKET', 'wildlife-images')
    
    with open(image_path, 'rb') as f:
        s3_client.upload_fileobj(
            f,
            bucket_name,
            f'detections/{filename}',
            ExtraArgs={'ContentType': 'image/jpeg'}
        )
    
    return f'https://{bucket_name}.s3.amazonaws.com/detections/{filename}'

@celery.task
def create_thumbnail(image_path):
    """Create thumbnail for quick loading"""
    img = cv2.imread(image_path)
    thumbnail = cv2.resize(img, (320, 240))
    
    thumb_path = image_path.replace('.jpg', '_thumb.jpg')
    cv2.imwrite(thumb_path, thumbnail)
    
    # Upload thumbnail to S3
    filename = os.path.basename(thumb_path)
    return upload_to_s3(thumb_path, filename)

@celery.task
def check_for_alerts(detection_id):
    """Check if detection should trigger alerts"""
    detection = Detection.query.get(detection_id)
    
    # Check for rare species
    rare_species = ['tiger', 'leopard', 'wolf', 'bear']
    if detection.species in rare_species and detection.confidence > 0.8:
        alert = Alert(
            detection_id=detection_id,
            alert_type='rare_species',
            message=f'Rare species detected: {detection.species}'
        )
        db.session.add(alert)
        db.session.commit()
        
        # Send notification
        send_notification.delay(alert.id)

@celery.task
def send_notification(alert_id):
    """Send alert notifications"""
    alert = Alert.query.get(alert_id)
    # Implement notification logic (email, SMS, push, etc.)
    alert.sent = True
    db.session.commit()

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    app.run(debug=True, host='0.0.0.0', port=5000)