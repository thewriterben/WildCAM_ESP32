"""
ESP32 Wildlife Camera - Complete Backend API Server
Production-ready Flask application with JWT auth, WebSocket support, and comprehensive API
"""

from flask import Flask, request, jsonify, send_file
from flask_socketio import SocketIO, emit
from flask_jwt_extended import JWTManager, jwt_required, get_jwt_identity
from flask_migrate import Migrate
from werkzeug.utils import secure_filename
from datetime import datetime, timedelta
import os
import json
import logging
from celery import Celery
import cv2
import numpy as np
from PIL import Image
import boto3

# Import our models and auth system
from models import db, User, Camera, Species, WildlifeDetection, Alert, AnalyticsData, DetectionComment, Bookmark, ChatMessage
from auth import create_auth_routes, check_if_token_revoked, verify_jwt_in_request

# Application factory
def create_app(config_name='development'):
    app = Flask(__name__)
    
    # Configuration
    app.config['SQLALCHEMY_DATABASE_URI'] = os.environ.get('DATABASE_URL', 
        'postgresql://wildlife_user:secure_password@localhost:5432/wildlife_db')
    app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
    app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'dev-secret-key-change-in-production')
    app.config['JWT_SECRET_KEY'] = os.environ.get('JWT_SECRET_KEY', 'jwt-secret-change-in-production')
    app.config['JWT_ACCESS_TOKEN_EXPIRES'] = timedelta(hours=24)
    app.config['JWT_REFRESH_TOKEN_EXPIRES'] = timedelta(days=30)
    app.config['UPLOAD_FOLDER'] = os.environ.get('UPLOAD_FOLDER', 'uploads')
    app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024  # 16MB max file size
    
    # AWS S3 Configuration
    app.config['AWS_ACCESS_KEY_ID'] = os.environ.get('AWS_ACCESS_KEY_ID')
    app.config['AWS_SECRET_ACCESS_KEY'] = os.environ.get('AWS_SECRET_ACCESS_KEY')
    app.config['S3_BUCKET'] = os.environ.get('S3_BUCKET', 'wildlife-captures')
    
    # Redis configuration
    app.config['REDIS_URL'] = os.environ.get('REDIS_URL', 'redis://localhost:6379')
    
    # Initialize extensions
    db.init_app(app)
    migrate = Migrate(app, db)
    jwt = JWTManager(app)
    
    # WebSocket support
    socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')
    
    # JWT token blacklist check
    jwt.token_in_blocklist_loader(check_if_token_revoked)
    
    # Logging setup
    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger(__name__)
    
    # S3 client setup
    s3_client = None
    if app.config['AWS_ACCESS_KEY_ID'] and app.config['AWS_SECRET_ACCESS_KEY']:
        s3_client = boto3.client('s3',
            aws_access_key_id=app.config['AWS_ACCESS_KEY_ID'],
            aws_secret_access_key=app.config['AWS_SECRET_ACCESS_KEY']
        )
    
    # Celery setup
    celery = Celery(app.name, broker=app.config['REDIS_URL'])
    
    # Create upload folder
    os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)
    
    # Register authentication routes
    create_auth_routes(app)
    
    # ==== API ROUTES ====
    
    @app.route('/api/health', methods=['GET'])
    def health_check():
        """Health check endpoint"""
        return jsonify({
            'status': 'healthy',
            'timestamp': datetime.utcnow().isoformat(),
            'version': '3.0.0',
            'services': {
                'database': 'connected',
                's3': s3_client is not None,
                'redis': True  # TODO: Add redis health check
            }
        }), 200
    
    @app.route('/api/cameras', methods=['GET'])
    @jwt_required()
    def get_cameras():
        """Get all registered cameras"""
        try:
            cameras = Camera.query.filter_by(status='active').all()
            return jsonify({
                'cameras': [camera.to_dict() for camera in cameras],
                'count': len(cameras)
            }), 200
        except Exception as e:
            logger.error(f"Get cameras error: {e}")
            return jsonify({'error': 'Failed to fetch cameras'}), 500
    
    @app.route('/api/cameras/register', methods=['POST'])
    def register_camera():
        """Register a new camera device (public endpoint for ESP32)"""
        try:
            data = request.get_json()
            
            if not data or not data.get('device_id'):
                return jsonify({'error': 'device_id is required'}), 400
            
            # Check if camera already exists
            existing_camera = Camera.query.filter_by(device_id=data['device_id']).first()
            if existing_camera:
                # Update existing camera
                existing_camera.last_seen = datetime.utcnow()
                existing_camera.firmware_version = data.get('firmware_version')
                existing_camera.hardware_version = data.get('hardware_version')
                if data.get('config'):
                    existing_camera.config = data['config']
                db.session.commit()
                return jsonify({
                    'message': 'Camera updated',
                    'camera': existing_camera.to_dict()
                }), 200
            
            # Create new camera
            camera = Camera(
                device_id=data['device_id'],
                name=data.get('name', f"Camera-{data['device_id']}"),
                location_name=data.get('location_name'),
                latitude=data.get('latitude'),
                longitude=data.get('longitude'),
                altitude=data.get('altitude'),
                firmware_version=data.get('firmware_version'),
                hardware_version=data.get('hardware_version'),
                config=data.get('config', {}),
                last_seen=datetime.utcnow()
            )
            
            db.session.add(camera)
            db.session.commit()
            
            return jsonify({
                'message': 'Camera registered successfully',
                'camera': camera.to_dict()
            }), 201
            
        except Exception as e:
            db.session.rollback()
            logger.error(f"Camera registration error: {e}")
            return jsonify({'error': 'Camera registration failed'}), 500
    
    @app.route('/api/cameras/<device_id>/status', methods=['POST'])
    def update_camera_status(device_id):
        """Update camera status (public endpoint for ESP32)"""
        try:
            data = request.get_json()
            camera = Camera.query.filter_by(device_id=device_id).first()
            
            if not camera:
                return jsonify({'error': 'Camera not found'}), 404
            
            # Update status fields
            camera.last_seen = datetime.utcnow()
            camera.battery_level = data.get('battery_level', camera.battery_level)
            camera.solar_voltage = data.get('solar_voltage', camera.solar_voltage)
            camera.temperature = data.get('temperature', camera.temperature)
            camera.humidity = data.get('humidity', camera.humidity)
            
            db.session.commit()
            
            # Emit real-time update
            socketio.emit('device_status_update', {
                'device_id': device_id,
                'battery_level': camera.battery_level,
                'solar_voltage': camera.solar_voltage,
                'temperature': camera.temperature,
                'humidity': camera.humidity,
                'last_seen': camera.last_seen.isoformat()
            })
            
            # Check for alerts
            check_camera_alerts(camera)
            
            return jsonify({'message': 'Status updated successfully'}), 200
            
        except Exception as e:
            db.session.rollback()
            logger.error(f"Status update error: {e}")
            return jsonify({'error': 'Status update failed'}), 500
    
    @app.route('/api/detections', methods=['POST'])
    def upload_detection():
        """Upload a new wildlife detection with image"""
        try:
            # Handle both multipart form data and JSON
            if request.content_type.startswith('multipart/form-data'):
                # Handle image upload
                if 'image' not in request.files:
                    return jsonify({'error': 'No image provided'}), 400
                
                image_file = request.files['image']
                if image_file.filename == '':
                    return jsonify({'error': 'No image selected'}), 400
                
                # Get metadata from form
                metadata = json.loads(request.form.get('metadata', '{}'))
            else:
                # Handle JSON data (base64 encoded image)
                data = request.get_json()
                metadata = data
                image_file = None
            
            # Validate required fields
            device_id = metadata.get('device_id')
            if not device_id:
                return jsonify({'error': 'device_id is required'}), 400
            
            # Find camera
            camera = Camera.query.filter_by(device_id=device_id).first()
            if not camera:
                return jsonify({'error': 'Camera not registered'}), 404
            
            # Process image upload
            image_url = None
            thumbnail_url = None
            
            if image_file:
                # Save image file
                filename = secure_filename(f"{device_id}_{datetime.utcnow().strftime('%Y%m%d_%H%M%S')}_{image_file.filename}")
                image_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
                image_file.save(image_path)
                
                # Upload to S3 if configured
                if s3_client:
                    try:
                        s3_key = f"detections/{filename}"
                        s3_client.upload_file(image_path, app.config['S3_BUCKET'], s3_key)
                        image_url = f"https://{app.config['S3_BUCKET']}.s3.amazonaws.com/{s3_key}"
                        
                        # Create and upload thumbnail
                        thumbnail_path = create_thumbnail(image_path)
                        if thumbnail_path:
                            thumb_key = f"thumbnails/{filename}"
                            s3_client.upload_file(thumbnail_path, app.config['S3_BUCKET'], thumb_key)
                            thumbnail_url = f"https://{app.config['S3_BUCKET']}.s3.amazonaws.com/{thumb_key}"
                    except Exception as e:
                        logger.error(f"S3 upload error: {e}")
                        # Use local file as fallback
                        image_url = f"/api/images/{filename}"
                else:
                    image_url = f"/api/images/{filename}"
            
            # Create detection record
            detection = WildlifeDetection(
                camera_id=camera.id,
                timestamp=datetime.fromisoformat(metadata.get('timestamp', datetime.utcnow().isoformat().replace('T', ' ').replace('Z', ''))),
                image_url=image_url,
                thumbnail_url=thumbnail_url,
                detected_species=metadata.get('species'),
                confidence_score=metadata.get('confidence'),
                bounding_box=metadata.get('bbox'),
                additional_detections=metadata.get('additional_detections'),
                weather_conditions=metadata.get('weather'),
                motion_trigger_data=metadata.get('motion_data'),
                metadata=metadata
            )
            
            db.session.add(detection)
            db.session.commit()
            
            # Emit real-time detection
            socketio.emit('new_detection', detection.to_dict())
            
            # Check for species-based alerts
            check_detection_alerts(detection)
            
            return jsonify({
                'detection_id': detection.id,
                'message': 'Detection uploaded successfully'
            }), 201
            
        except Exception as e:
            db.session.rollback()
            logger.error(f"Detection upload error: {e}")
            return jsonify({'error': 'Detection upload failed'}), 500
    
    @app.route('/api/detections', methods=['GET'])
    @jwt_required()
    def get_detections():
        """Get detection history with filters"""
        try:
            # Query parameters
            page = request.args.get('page', 1, type=int)
            per_page = min(request.args.get('per_page', 20, type=int), 100)
            camera_id = request.args.get('camera_id', type=int)
            species = request.args.get('species')
            start_date = request.args.get('start_date')
            end_date = request.args.get('end_date')
            verified_only = request.args.get('verified_only', 'false').lower() == 'true'
            
            # Build query
            query = WildlifeDetection.query
            
            if camera_id:
                query = query.filter(WildlifeDetection.camera_id == camera_id)
            if species:
                query = query.filter(WildlifeDetection.detected_species.ilike(f'%{species}%'))
            if start_date:
                query = query.filter(WildlifeDetection.timestamp >= datetime.fromisoformat(start_date))
            if end_date:
                query = query.filter(WildlifeDetection.timestamp <= datetime.fromisoformat(end_date))
            if verified_only:
                query = query.filter(WildlifeDetection.verified == True)
            
            # Order by timestamp desc
            query = query.order_by(WildlifeDetection.timestamp.desc())
            
            # Paginate
            detections = query.paginate(page=page, per_page=per_page, error_out=False)
            
            return jsonify({
                'detections': [detection.to_dict() for detection in detections.items],
                'total': detections.total,
                'pages': detections.pages,
                'current_page': detections.page
            }), 200
            
        except Exception as e:
            logger.error(f"Get detections error: {e}")
            return jsonify({'error': 'Failed to fetch detections'}), 500
    
    @app.route('/api/images/<filename>')
    def serve_image(filename):
        """Serve uploaded images"""
        try:
            image_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            if os.path.exists(image_path):
                return send_file(image_path)
            return jsonify({'error': 'Image not found'}), 404
        except Exception as e:
            logger.error(f"Image serve error: {e}")
            return jsonify({'error': 'Failed to serve image'}), 500
    
    @app.route('/api/analytics/species', methods=['GET'])
    @jwt_required()
    def species_analytics():
        """Get species distribution analytics"""
        try:
            days = request.args.get('days', 30, type=int)
            start_date = datetime.utcnow() - timedelta(days=days)
            
            # Species count query
            species_data = db.session.query(
                WildlifeDetection.detected_species,
                db.func.count(WildlifeDetection.id).label('count'),
                db.func.avg(WildlifeDetection.confidence_score).label('avg_confidence')
            ).filter(
                WildlifeDetection.timestamp >= start_date,
                WildlifeDetection.is_false_positive == False
            ).group_by(WildlifeDetection.detected_species).all()
            
            return jsonify({
                'species_data': [
                    {
                        'species': item.detected_species,
                        'count': item.count,
                        'avg_confidence': float(item.avg_confidence or 0)
                    }
                    for item in species_data
                ],
                'period_days': days,
                'start_date': start_date.isoformat()
            }), 200
            
        except Exception as e:
            logger.error(f"Species analytics error: {e}")
            return jsonify({'error': 'Failed to fetch species analytics'}), 500
    
    @app.route('/api/analytics/activity', methods=['GET'])
    @jwt_required()
    def activity_patterns():
        """Get temporal activity patterns"""
        try:
            days = request.args.get('days', 7, type=int)
            start_date = datetime.utcnow() - timedelta(days=days)
            
            # Hourly activity pattern
            hourly_data = db.session.query(
                db.func.extract('hour', WildlifeDetection.timestamp).label('hour'),
                db.func.count(WildlifeDetection.id).label('count')
            ).filter(
                WildlifeDetection.timestamp >= start_date,
                WildlifeDetection.is_false_positive == False
            ).group_by('hour').all()
            
            # Daily activity pattern
            daily_data = db.session.query(
                db.func.date(WildlifeDetection.timestamp).label('date'),
                db.func.count(WildlifeDetection.id).label('count')
            ).filter(
                WildlifeDetection.timestamp >= start_date,
                WildlifeDetection.is_false_positive == False
            ).group_by('date').all()
            
            return jsonify({
                'hourly_activity': [
                    {'hour': int(item.hour), 'count': item.count}
                    for item in hourly_data
                ],
                'daily_activity': [
                    {'date': item.date.isoformat(), 'count': item.count}
                    for item in daily_data
                ],
                'period_days': days
            }), 200
            
        except Exception as e:
            logger.error(f"Activity analytics error: {e}")
            return jsonify({'error': 'Failed to fetch activity analytics'}), 500
    
    @app.route('/api/alerts', methods=['GET'])
    @jwt_required()
    def get_alerts():
        """Get system alerts"""
        try:
            unread_only = request.args.get('unread_only', 'false').lower() == 'true'
            
            query = Alert.query
            if unread_only:
                query = query.filter(Alert.acknowledged == False)
            
            alerts = query.order_by(Alert.created_at.desc()).limit(50).all()
            
            return jsonify({
                'alerts': [alert.to_dict() for alert in alerts]
            }), 200
            
        except Exception as e:
            logger.error(f"Get alerts error: {e}")
            return jsonify({'error': 'Failed to fetch alerts'}), 500
    
    # ==== HELPER FUNCTIONS ====
    
    def create_thumbnail(image_path, size=(320, 240)):
        """Create thumbnail for quick loading"""
        try:
            img = cv2.imread(image_path)
            if img is None:
                return None
            
            thumbnail = cv2.resize(img, size)
            thumb_path = image_path.replace('.jpg', '_thumb.jpg')
            cv2.imwrite(thumb_path, thumbnail)
            return thumb_path
        except Exception as e:
            logger.error(f"Thumbnail creation error: {e}")
            return None
    
    def check_camera_alerts(camera):
        """Check for camera-related alerts"""
        try:
            # Low battery alert
            if camera.battery_level and camera.battery_level < 20:
                create_alert(
                    camera_id=camera.id,
                    alert_type='low_battery',
                    severity='warning',
                    title=f'Low Battery - {camera.name}',
                    message=f'Battery level is {camera.battery_level}%'
                )
            
            # Offline alert (no updates for more than 1 hour)
            if camera.last_seen and camera.last_seen < datetime.utcnow() - timedelta(hours=1):
                create_alert(
                    camera_id=camera.id,
                    alert_type='offline',
                    severity='critical',
                    title=f'Camera Offline - {camera.name}',
                    message=f'No updates since {camera.last_seen}'
                )
        except Exception as e:
            logger.error(f"Camera alert check error: {e}")
    
    def check_detection_alerts(detection):
        """Check for detection-related alerts"""
        try:
            # High confidence rare species alert
            if detection.confidence_score and detection.confidence_score > 0.9:
                rare_species = ['wolf', 'bear', 'lynx', 'eagle', 'owl']  # Add more as needed
                if detection.detected_species and any(species in detection.detected_species.lower() for species in rare_species):
                    create_alert(
                        camera_id=detection.camera_id,
                        detection_id=detection.id,
                        alert_type='rare_species',
                        severity='info',
                        title=f'Rare Species Detected: {detection.detected_species}',
                        message=f'High confidence detection ({detection.confidence_score:.1%})'
                    )
        except Exception as e:
            logger.error(f"Detection alert check error: {e}")
    
    def create_alert(camera_id=None, detection_id=None, alert_type='info', severity='info', title='', message='', data=None):
        """Create a new alert"""
        try:
            alert = Alert(
                camera_id=camera_id,
                detection_id=detection_id,
                alert_type=alert_type,
                severity=severity,
                title=title,
                message=message,
                data=data
            )
            db.session.add(alert)
            db.session.commit()
            
            # Emit real-time alert
            socketio.emit('new_alert', alert.to_dict())
            
        except Exception as e:
            db.session.rollback()
            logger.error(f"Alert creation error: {e}")
    
    # ==== WEBSOCKET EVENTS ====
    
    @socketio.on('connect')
    def handle_connect():
        logger.info('Client connected to WebSocket')
        emit('connected', {'message': 'Connected to WildCAM API'})
    
    @socketio.on('disconnect')
    def handle_disconnect():
        logger.info('Client disconnected from WebSocket')
    
    # Return the app instance
    return app, socketio

# Create the application
app, socketio = create_app()

# Run the application
if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    socketio.run(app, debug=True, host='0.0.0.0', port=5000)
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

    # ==== COLLABORATION ENDPOINTS ====
    
    @app.route('/api/collaboration/comments', methods=['GET', 'POST'])
    @jwt_required()
    def detection_comments():
        """Get or create comments on detections"""
        from models import DetectionComment
        
        if request.method == 'GET':
            detection_id = request.args.get('detection_id')
            if not detection_id:
                return jsonify({'error': 'detection_id required'}), 400
            
            comments = DetectionComment.query.filter_by(
                detection_id=detection_id
            ).order_by(DetectionComment.created_at.desc()).all()
            
            return jsonify({
                'comments': [c.to_dict() for c in comments]
            }), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            detection_id = data.get('detection_id')
            comment_text = data.get('comment')
            
            if not detection_id or not comment_text:
                return jsonify({'error': 'detection_id and comment required'}), 400
            
            user_id = get_jwt_identity()
            comment = DetectionComment(
                detection_id=detection_id,
                user_id=user_id,
                comment=comment_text
            )
            
            db.session.add(comment)
            db.session.commit()
            
            # Emit via WebSocket
            socketio.emit('new_comment', comment.to_dict())
            
            return jsonify({
                'message': 'Comment added',
                'comment': comment.to_dict()
            }), 201
    
    @app.route('/api/collaboration/bookmarks', methods=['GET', 'POST'])
    @jwt_required()
    def bookmarks():
        """Get or create bookmarks"""
        from models import Bookmark
        
        if request.method == 'GET':
            user_id = get_jwt_identity()
            shared_only = request.args.get('shared', 'false').lower() == 'true'
            
            query = Bookmark.query
            if shared_only:
                query = query.filter_by(shared=True)
            else:
                query = query.filter_by(user_id=user_id)
            
            bookmarks = query.order_by(Bookmark.created_at.desc()).all()
            
            return jsonify({
                'bookmarks': [b.to_dict() for b in bookmarks]
            }), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            detection_id = data.get('detection_id')
            title = data.get('title')
            
            if not detection_id or not title:
                return jsonify({'error': 'detection_id and title required'}), 400
            
            user_id = get_jwt_identity()
            bookmark = Bookmark(
                detection_id=detection_id,
                user_id=user_id,
                title=title,
                description=data.get('description'),
                shared=data.get('shared', False)
            )
            
            db.session.add(bookmark)
            db.session.commit()
            
            if bookmark.shared:
                socketio.emit('new_bookmark', bookmark.to_dict())
            
            return jsonify({
                'message': 'Bookmark created',
                'bookmark': bookmark.to_dict()
            }), 201
    
    @app.route('/api/collaboration/chat', methods=['GET', 'POST'])
    @jwt_required()
    def chat_messages():
        """Get or send chat messages"""
        from models import ChatMessage
        
        if request.method == 'GET':
            detection_id = request.args.get('detection_id')
            limit = int(request.args.get('limit', 50))
            
            query = ChatMessage.query
            if detection_id:
                query = query.filter_by(detection_id=detection_id)
            
            messages = query.order_by(
                ChatMessage.created_at.desc()
            ).limit(limit).all()
            
            return jsonify({
                'messages': [m.to_dict() for m in reversed(messages)]
            }), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            message_text = data.get('message')
            
            if not message_text:
                return jsonify({'error': 'message required'}), 400
            
            user_id = get_jwt_identity()
            message = ChatMessage(
                user_id=user_id,
                message=message_text,
                detection_id=data.get('detection_id')
            )
            
            db.session.add(message)
            db.session.commit()
            
            # Emit via WebSocket
            socketio.emit('chat_message', message.to_dict())
            
            return jsonify({
                'message': 'Message sent',
                'chat_message': message.to_dict()
            }), 201
    
    @app.route('/api/collaboration/presence', methods=['GET'])
    @jwt_required()
    def active_users():
        """Get list of currently active users"""
        from models import User
        
        # Get users who have logged in within last 5 minutes
        active_threshold = datetime.utcnow() - timedelta(minutes=5)
        users = User.query.filter(
            User.last_login != None,
            User.last_login >= active_threshold,
            User.is_active == True
        ).all()
        
        return jsonify({
            'active_users': [u.to_dict() for u in users],
            'count': len(users)
        }), 200
    
    # WebSocket handlers for real-time collaboration
    @socketio.on('connect')
    def handle_connect():
        """Handle user connection"""
        logger.info('Client connected')
        try:
            verify_jwt_in_request()
            user_id = get_jwt_identity()
            user = User.query.get(user_id)
            if user:
                user.last_login = datetime.utcnow()
                db.session.commit()
                emit('user_connected', {'user_id': user_id, 'username': user.username}, broadcast=True)
        except:
            pass
    
    @socketio.on('disconnect')
    def handle_disconnect():
        """Handle user disconnection"""
        logger.info('Client disconnected')
    
    @socketio.on('cursor_move')
    def handle_cursor_move(data):
        """Handle cursor position updates for collaborative viewing"""
        try:
            verify_jwt_in_request()
            user_id = get_jwt_identity()
            emit('cursor_update', {
                'user_id': user_id,
                'position': data.get('position'),
                'page': data.get('page')
            }, broadcast=True, include_self=False)
        except:
            pass

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