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
from models import db, User, Camera, Species, WildlifeDetection, Alert, AnalyticsData
from models import UserSession, Annotation, ChatMessage, SharedBookmark, Task, FieldNote
from auth import create_auth_routes, check_if_token_revoked
from collaboration import CollaborationService

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
    
    # Initialize collaboration services
    collab_service = CollaborationService(socketio)
    app.config['collab_service'] = collab_service
    
    # Initialize RBAC service
    from rbac import RBACService
    rbac_service = RBACService(db)
    app.config['rbac_service'] = rbac_service
    
    # Initialize WebRTC signaling (Phase 3 preparation)
    from webrtc_signaling import WebRTCSignalingServer
    webrtc_signaling = WebRTCSignalingServer(socketio, db)
    app.config['webrtc_signaling'] = webrtc_signaling
    
    # Register authentication routes
    create_auth_routes(app)
    
    # Register alert routes
    from routes import alert_bp
    app.register_blueprint(alert_bp, url_prefix='/api')
    
    # Register research platform routes
    from routes.research_routes import create_research_routes
    create_research_routes(app)
    
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
    
    # ==== COLLABORATION API ROUTES ====
    
    @app.route('/api/collaboration/users/active', methods=['GET'])
    @jwt_required()
    def get_active_users_api():
        """Get list of currently active users"""
        try:
            users = collab_service.get_active_users()
            return jsonify({'active_users': users, 'count': len(users)}), 200
        except Exception as e:
            logger.error(f"Get active users error: {e}")
            return jsonify({'error': 'Failed to fetch active users'}), 500
    
    @app.route('/api/collaboration/annotations', methods=['GET', 'POST'])
    @jwt_required()
    def handle_annotations():
        """Get or create annotations"""
        user_id = get_jwt_identity()
        
        if request.method == 'GET':
            detection_id = request.args.get('detection_id', type=int)
            if not detection_id:
                return jsonify({'error': 'detection_id is required'}), 400
            
            annotations = collab_service.get_annotations(detection_id)
            return jsonify({'annotations': annotations}), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            required = ['detection_id', 'annotation_type', 'content', 'position']
            if not all(k in data for k in required):
                return jsonify({'error': 'Missing required fields'}), 400
            
            annotation = collab_service.create_annotation(
                detection_id=data['detection_id'],
                user_id=user_id,
                annotation_type=data['annotation_type'],
                content=data['content'],
                position=data['position'],
                metadata=data.get('metadata')
            )
            
            if annotation:
                return jsonify({'annotation': annotation}), 201
            return jsonify({'error': 'Failed to create annotation'}), 500
    
    @app.route('/api/collaboration/chat', methods=['GET', 'POST'])
    @jwt_required()
    def handle_chat():
        """Get or send chat messages"""
        user_id = get_jwt_identity()
        
        if request.method == 'GET':
            channel = request.args.get('channel', 'general')
            limit = request.args.get('limit', 50, type=int)
            
            messages = collab_service.get_chat_messages(channel, limit)
            return jsonify({'messages': messages}), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            if not data or not data.get('message'):
                return jsonify({'error': 'message is required'}), 400
            
            message = collab_service.send_chat_message(
                user_id=user_id,
                channel=data.get('channel', 'general'),
                message=data['message'],
                parent_id=data.get('parent_id'),
                mentions=data.get('mentions')
            )
            
            if message:
                return jsonify({'message': message}), 201
            return jsonify({'error': 'Failed to send message'}), 500
    
    @app.route('/api/collaboration/bookmarks', methods=['GET', 'POST'])
    @jwt_required()
    def handle_bookmarks():
        """Get or create bookmarks"""
        user_id = get_jwt_identity()
        
        if request.method == 'GET':
            bookmarks = collab_service.get_bookmarks(user_id)
            return jsonify({'bookmarks': bookmarks}), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            if not data or not data.get('title'):
                return jsonify({'error': 'title is required'}), 400
            
            bookmark = collab_service.create_bookmark(
                user_id=user_id,
                title=data['title'],
                description=data.get('description'),
                detection_id=data.get('detection_id'),
                camera_id=data.get('camera_id'),
                tags=data.get('tags'),
                is_shared=data.get('is_shared', False),
                shared_with=data.get('shared_with')
            )
            
            if bookmark:
                return jsonify({'bookmark': bookmark}), 201
            return jsonify({'error': 'Failed to create bookmark'}), 500
    
    @app.route('/api/collaboration/tasks', methods=['GET', 'POST'])
    @jwt_required()
    def handle_tasks():
        """Get or create tasks"""
        user_id = get_jwt_identity()
        
        if request.method == 'GET':
            status = request.args.get('status')
            my_tasks = request.args.get('my_tasks', 'true').lower() == 'true'
            
            tasks = collab_service.get_tasks(
                user_id=user_id if my_tasks else None,
                status=status
            )
            return jsonify({'tasks': tasks}), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            if not data or not data.get('title'):
                return jsonify({'error': 'title is required'}), 400
            
            task = collab_service.create_task(
                title=data['title'],
                description=data.get('description'),
                created_by=user_id,
                task_type=data.get('task_type'),
                priority=data.get('priority', 'medium'),
                assigned_to=data.get('assigned_to'),
                camera_id=data.get('camera_id'),
                detection_id=data.get('detection_id'),
                due_date=datetime.fromisoformat(data['due_date']) if data.get('due_date') else None
            )
            
            if task:
                return jsonify({'task': task}), 201
            return jsonify({'error': 'Failed to create task'}), 500
    
    @app.route('/api/collaboration/tasks/<int:task_id>', methods=['PATCH'])
    @jwt_required()
    def update_task(task_id):
        """Update task status"""
        user_id = get_jwt_identity()
        data = request.get_json()
        
        if not data or not data.get('status'):
            return jsonify({'error': 'status is required'}), 400
        
        task = collab_service.update_task_status(task_id, data['status'], user_id)
        if task:
            return jsonify({'task': task}), 200
        return jsonify({'error': 'Failed to update task'}), 500
    
    @app.route('/api/collaboration/field-notes', methods=['GET', 'POST'])
    @jwt_required()
    def handle_field_notes():
        """Get or create field notes"""
        user_id = get_jwt_identity()
        
        if request.method == 'GET':
            camera_id = request.args.get('camera_id', type=int)
            detection_id = request.args.get('detection_id', type=int)
            
            notes = collab_service.get_field_notes(camera_id, detection_id)
            return jsonify({'notes': notes}), 200
        
        elif request.method == 'POST':
            data = request.get_json()
            required = ['title', 'content']
            if not all(k in data for k in required):
                return jsonify({'error': 'title and content are required'}), 400
            
            note = collab_service.create_field_note(
                user_id=user_id,
                title=data['title'],
                content=data['content'],
                note_type=data.get('note_type'),
                camera_id=data.get('camera_id'),
                detection_id=data.get('detection_id'),
                tags=data.get('tags'),
                attachments=data.get('attachments')
            )
            
            if note:
                return jsonify({'note': note}), 201
            return jsonify({'error': 'Failed to create field note'}), 500
    
    # ==== RBAC API ROUTES ====
    
    @app.route('/api/rbac/permissions', methods=['GET'])
    @jwt_required()
    def get_user_permissions():
        """Get current user's role and permissions"""
        try:
            user_id = get_jwt_identity()
            from rbac import get_rbac_info
            
            rbac_info = get_rbac_info(user_id)
            if not rbac_info:
                return jsonify({'error': 'RBAC information not available'}), 500
            
            return jsonify({
                'user_id': user_id,
                'rbac': rbac_info
            }), 200
        except Exception as e:
            logger.error(f"Get RBAC info error: {e}")
            return jsonify({'error': 'Failed to fetch RBAC information'}), 500
    
    @app.route('/api/rbac/check-permission', methods=['POST'])
    @jwt_required()
    def check_permission():
        """Check if current user has a specific permission"""
        try:
            user_id = get_jwt_identity()
            data = request.get_json()
            
            if not data or not data.get('permission'):
                return jsonify({'error': 'permission is required'}), 400
            
            from rbac import Permission
            permission_name = data['permission']
            
            # Convert string to Permission enum
            try:
                permission = Permission[permission_name.upper()]
            except KeyError:
                return jsonify({'error': f'Invalid permission: {permission_name}'}), 400
            
            rbac = app.config.get('rbac_service')
            has_permission = rbac.has_permission(user_id, permission)
            
            return jsonify({
                'has_permission': has_permission,
                'permission': permission_name
            }), 200
        except Exception as e:
            logger.error(f"Check permission error: {e}")
            return jsonify({'error': 'Failed to check permission'}), 500
    
    @app.route('/api/rbac/roles', methods=['GET'])
    @jwt_required()
    def get_available_roles():
        """Get list of available roles and their permissions"""
        from rbac import Role, ROLE_PERMISSIONS, Permission
        
        roles_info = {}
        for role in Role:
            permissions = ROLE_PERMISSIONS.get(role, set())
            roles_info[role.name.lower()] = {
                'name': role.name,
                'level': role.value,
                'permissions': [p.value for p in permissions]
            }
        
        return jsonify({'roles': roles_info}), 200
    
    # ==== WEBSOCKET EVENTS FOR COLLABORATION ====
    
    @socketio.on('user_presence')
    def handle_user_presence(data):
        """Handle user presence updates"""
        try:
            session_id = data.get('session_id')
            user_id = data.get('user_id')
            
            if data.get('action') == 'connect':
                collab_service.user_connected(
                    user_id=user_id,
                    session_id=session_id,
                    socket_id=request.sid,
                    ip_address=request.remote_addr,
                    user_agent=request.headers.get('User-Agent', '')
                )
            elif data.get('action') == 'disconnect':
                collab_service.user_disconnected(session_id)
            elif data.get('action') == 'update':
                collab_service.update_user_activity(
                    session_id=session_id,
                    current_page=data.get('current_page'),
                    cursor_position=data.get('cursor_position')
                )
        except Exception as e:
            logger.error(f"User presence error: {e}")
    
    @socketio.on('join_channel')
    def handle_join_channel(data):
        """Join a chat channel or page room"""
        channel = data.get('channel')
        if channel:
            join_room(channel)
            logger.info(f"Client {request.sid} joined channel: {channel}")
    
    @socketio.on('leave_channel')
    def handle_leave_channel(data):
        """Leave a chat channel or page room"""
        channel = data.get('channel')
        if channel:
            leave_room(channel)
            logger.info(f"Client {request.sid} left channel: {channel}")
    
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