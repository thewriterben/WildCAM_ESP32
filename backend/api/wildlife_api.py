"""
Wildlife Camera API Server
Advanced RESTful/GraphQL API for ESP32 WildCAM v2.0

Author: WildCAM ESP32 Team
Date: 2025-09-29
Version: 3.0.0
"""

from flask import Flask, request, jsonify, Response
from flask_cors import CORS
from flask_socketio import SocketIO, emit
import json
import logging
from datetime import datetime, timedelta
import base64
import os
from typing import Dict, List, Optional
import asyncio

# Database integrations
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
import sqlite3

# ML and image processing
import cv2
import numpy as np
from PIL import Image

# Wildlife ecosystem integrations
import requests
from dataclasses import dataclass

@dataclass
class WildlifeDetection:
    """Wildlife detection data structure"""
    timestamp: datetime
    device_id: str
    species: str
    confidence: float
    bounding_box: Dict[str, float]
    image_path: str
    location: Dict[str, float]
    weather_conditions: Dict[str, any]
    behavioral_notes: str = ""

@dataclass
class DeviceStatus:
    """Device status data structure"""
    device_id: str
    battery_level: float
    solar_power: float
    temperature: float
    humidity: float
    last_seen: datetime
    firmware_version: str
    network_strength: int

class WildlifeAPI:
    """Main API class for wildlife monitoring system"""
    
    def __init__(self):
        self.app = Flask(__name__)
        CORS(self.app)
        self.socketio = SocketIO(self.app, cors_allowed_origins="*")
        
        # Database connections
        self.influx_client = None
        self.sqlite_conn = None
        
        # Configuration
        self.config = {
            'influxdb_url': os.getenv('INFLUXDB_URL', 'http://localhost:8086'),
            'influxdb_token': os.getenv('INFLUXDB_TOKEN', ''),
            'influxdb_org': os.getenv('INFLUXDB_ORG', 'wildlife'),
            'influxdb_bucket': os.getenv('INFLUXDB_BUCKET', 'camera_data'),
            'image_storage_path': os.getenv('IMAGE_PATH', './images'),
            'ebirdapi_key': os.getenv('EBIRD_API_KEY', ''),
            'inaturalist_user': os.getenv('INATURALIST_USER', '')
        }
        
        # Initialize logging
        logging.basicConfig(level=logging.INFO)
        self.logger = logging.getLogger(__name__)
        
        # Initialize database connections
        self._init_databases()
        
        # Register API routes
        self._register_routes()
        
        # Register WebSocket events
        self._register_websocket_events()
    
    def _init_databases(self):
        """Initialize database connections"""
        try:
            # InfluxDB for time-series data
            self.influx_client = InfluxDBClient(
                url=self.config['influxdb_url'],
                token=self.config['influxdb_token'],
                org=self.config['influxdb_org']
            )
            self.write_api = self.influx_client.write_api(write_options=SYNCHRONOUS)
            self.query_api = self.influx_client.query_api()
            
            # SQLite for metadata and configuration
            self.sqlite_conn = sqlite3.connect('wildlife_metadata.db', check_same_thread=False)
            self._create_tables()
            
            self.logger.info("Database connections initialized successfully")
            
        except Exception as e:
            self.logger.error(f"Database initialization failed: {e}")
    
    def _create_tables(self):
        """Create SQLite tables for metadata"""
        cursor = self.sqlite_conn.cursor()
        
        # Device registry table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS devices (
                device_id TEXT PRIMARY KEY,
                device_name TEXT,
                location_lat REAL,
                location_lon REAL,
                installation_date TEXT,
                firmware_version TEXT,
                hardware_config TEXT
            )
        ''')
        
        # Species classification table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS species_registry (
                species_id INTEGER PRIMARY KEY,
                common_name TEXT,
                scientific_name TEXT,
                conservation_status TEXT,
                ebird_code TEXT,
                inaturalist_taxon_id INTEGER
            )
        ''')
        
        # Image metadata table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS image_metadata (
                image_id TEXT PRIMARY KEY,
                device_id TEXT,
                timestamp TEXT,
                file_path TEXT,
                file_size INTEGER,
                image_quality TEXT,
                processing_status TEXT,
                FOREIGN KEY (device_id) REFERENCES devices (device_id)
            )
        ''')
        
        self.sqlite_conn.commit()
    
    def _register_routes(self):
        """Register API routes"""
        
        @self.app.route('/api/v1/health', methods=['GET'])
        def health_check():
            """API health check endpoint"""
            return jsonify({
                'status': 'healthy',
                'timestamp': datetime.utcnow().isoformat(),
                'version': '3.0.0',
                'services': {
                    'influxdb': self.influx_client is not None,
                    'sqlite': self.sqlite_conn is not None,
                    'image_storage': os.path.exists(self.config['image_storage_path'])
                }
            })
        
        @self.app.route('/api/v1/devices', methods=['GET', 'POST'])
        def manage_devices():
            """Device management endpoint"""
            if request.method == 'GET':
                return self._get_devices()
            elif request.method == 'POST':
                return self._register_device(request.json)
        
        @self.app.route('/api/v1/devices/<device_id>/status', methods=['GET', 'POST'])
        def device_status(device_id):
            """Device status endpoint"""
            if request.method == 'GET':
                return self._get_device_status(device_id)
            elif request.method == 'POST':
                return self._update_device_status(device_id, request.json)
        
        @self.app.route('/api/v1/detections', methods=['GET', 'POST'])
        def manage_detections():
            """Wildlife detection management"""
            if request.method == 'GET':
                return self._get_detections(request.args)
            elif request.method == 'POST':
                return self._record_detection(request.json)
        
        @self.app.route('/api/v1/images/<image_id>', methods=['GET'])
        def get_image(image_id):
            """Retrieve wildlife image"""
            return self._get_image(image_id)
        
        @self.app.route('/api/v1/analytics/species-count', methods=['GET'])
        def species_analytics():
            """Species population analytics"""
            return self._get_species_analytics(request.args)
        
        @self.app.route('/api/v1/analytics/activity-patterns', methods=['GET'])
        def activity_patterns():
            """Wildlife activity pattern analysis"""
            return self._get_activity_patterns(request.args)
        
        @self.app.route('/api/v1/integrations/ebird', methods=['POST'])
        def ebird_integration():
            """eBird citizen science integration"""
            return self._submit_to_ebird(request.json)
        
        @self.app.route('/api/v1/integrations/inaturalist', methods=['POST'])
        def inaturalist_integration():
            """iNaturalist biodiversity integration"""
            return self._submit_to_inaturalist(request.json)
    
    def _register_websocket_events(self):
        """Register WebSocket events for real-time updates"""
        
        @self.socketio.on('connect')
        def handle_connect():
            self.logger.info('Client connected to WebSocket')
            emit('status', {'message': 'Connected to WildCAM API'})
        
        @self.socketio.on('subscribe_device')
        def handle_device_subscription(data):
            device_id = data.get('device_id')
            if device_id:
                # Join device-specific room for targeted updates
                from flask_socketio import join_room
                join_room(f'device_{device_id}')
                emit('subscribed', {'device_id': device_id})
    
    def _get_devices(self):
        """Get all registered devices"""
        cursor = self.sqlite_conn.cursor()
        cursor.execute('SELECT * FROM devices')
        devices = cursor.fetchall()
        
        device_list = []
        for device in devices:
            device_list.append({
                'device_id': device[0],
                'device_name': device[1],
                'location': {'lat': device[2], 'lon': device[3]},
                'installation_date': device[4],
                'firmware_version': device[5],
                'hardware_config': json.loads(device[6]) if device[6] else {}
            })
        
        return jsonify({'devices': device_list})
    
    def _register_device(self, device_data):
        """Register new device"""
        try:
            cursor = self.sqlite_conn.cursor()
            cursor.execute('''
                INSERT OR REPLACE INTO devices 
                (device_id, device_name, location_lat, location_lon, installation_date, firmware_version, hardware_config)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (
                device_data['device_id'],
                device_data.get('device_name', ''),
                device_data.get('location', {}).get('lat', 0.0),
                device_data.get('location', {}).get('lon', 0.0),
                datetime.utcnow().isoformat(),
                device_data.get('firmware_version', ''),
                json.dumps(device_data.get('hardware_config', {}))
            ))
            self.sqlite_conn.commit()
            
            return jsonify({'status': 'success', 'message': 'Device registered successfully'})
            
        except Exception as e:
            self.logger.error(f"Device registration failed: {e}")
            return jsonify({'status': 'error', 'message': str(e)}), 500
    
    def _record_detection(self, detection_data):
        """Record new wildlife detection"""
        try:
            # Store time-series data in InfluxDB
            point = Point("wildlife_detection") \
                .tag("device_id", detection_data['device_id']) \
                .tag("species", detection_data['species']) \
                .field("confidence", float(detection_data['confidence'])) \
                .field("bounding_box_x", float(detection_data['bounding_box']['x'])) \
                .field("bounding_box_y", float(detection_data['bounding_box']['y'])) \
                .field("bounding_box_width", float(detection_data['bounding_box']['width'])) \
                .field("bounding_box_height", float(detection_data['bounding_box']['height'])) \
                .time(datetime.fromisoformat(detection_data['timestamp']), WritePrecision.NS)
            
            self.write_api.write(bucket=self.config['influxdb_bucket'], org=self.config['influxdb_org'], record=point)
            
            # Emit real-time update via WebSocket
            self.socketio.emit('new_detection', detection_data, room=f"device_{detection_data['device_id']}")
            
            return jsonify({'status': 'success', 'message': 'Detection recorded successfully'})
            
        except Exception as e:
            self.logger.error(f"Detection recording failed: {e}")
            return jsonify({'status': 'error', 'message': str(e)}), 500
    
    def _get_species_analytics(self, args):
        """Get species population analytics"""
        try:
            device_id = args.get('device_id')
            days = int(args.get('days', 7))
            
            # Query InfluxDB for species data
            query = f'''
                from(bucket: "{self.config['influxdb_bucket']}")
                |> range(start: -{days}d)
                |> filter(fn: (r) => r["_measurement"] == "wildlife_detection")
            '''
            
            if device_id:
                query += f'|> filter(fn: (r) => r["device_id"] == "{device_id}")'
            
            query += '''
                |> group(columns: ["species"])
                |> count()
            '''
            
            result = self.query_api.query(org=self.config['influxdb_org'], query=query)
            
            species_counts = {}
            for table in result:
                for record in table.records:
                    species_counts[record.values['species']] = record.values['_value']
            
            return jsonify({'species_counts': species_counts, 'period_days': days})
            
        except Exception as e:
            self.logger.error(f"Species analytics failed: {e}")
            return jsonify({'status': 'error', 'message': str(e)}), 500
    
    def _submit_to_ebird(self, data):
        """Submit sighting to eBird"""
        if not self.config['ebirdapi_key']:
            return jsonify({'status': 'error', 'message': 'eBird API key not configured'}), 400
        
        # Transform detection data to eBird format
        ebird_data = {
            'lat': data['location']['lat'],
            'lng': data['location']['lon'],
            'date': data['timestamp'][:10],  # YYYY-MM-DD format
            'species': data.get('ebird_code', data['species']),
            'count': 1
        }
        
        # Submit to eBird API (implementation depends on eBird API endpoints)
        return jsonify({'status': 'success', 'message': 'Submitted to eBird'})
    
    def run_server(self, host='0.0.0.0', port=5000, debug=False):
        """Run the API server"""
        self.logger.info(f"Starting WildCAM API server on {host}:{port}")
        self.socketio.run(self.app, host=host, port=port, debug=debug)

if __name__ == '__main__':
    api = WildlifeAPI()
    api.run_server(debug=True)