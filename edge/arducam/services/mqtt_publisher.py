"""
MQTT Publisher
Publishes detection events and telemetry via MQTT
Supports local and cloud brokers
"""

import logging
import json
from typing import Dict, Optional
from datetime import datetime
import threading
import time

logger = logging.getLogger(__name__)


class MQTTPublisher:
    """
    Publishes wildlife detections and telemetry via MQTT
    Supports local Mosquitto and cloud brokers (AWS IoT, Azure IoT Hub)
    """
    
    def __init__(self, config: Dict):
        """
        Initialize MQTT publisher
        
        Args:
            config: Configuration dictionary from YAML
        """
        self.config = config.get('mqtt', {})
        self.device_config = config.get('device', {})
        
        # Configuration
        self.enabled = self.config.get('enabled', True)
        self.broker = self.config.get('broker', 'localhost')
        self.port = self.config.get('port', 1883)
        self.username = self.config.get('username', '')
        self.password = self.config.get('password', '')
        self.use_tls = self.config.get('use_tls', False)
        self.ca_certs = self.config.get('ca_certs', '')
        self.topic_prefix = self.config.get('topic_prefix', 'wildcam')
        self.qos = self.config.get('qos', 1)
        self.retain = self.config.get('retain', False)
        self.keepalive = self.config.get('keepalive', 60)
        
        # Node identification
        self.node_id = self.device_config.get('node_id', 'pivistation_unknown')
        
        # MQTT client
        self.client = None
        self.connected = False
        
        # Statistics
        self.stats = {
            'total_published': 0,
            'total_failed': 0,
            'last_publish_time': None,
            'connection_attempts': 0,
        }
        
        if self.enabled:
            self._initialize_client()
    
    def _initialize_client(self):
        """Initialize MQTT client"""
        try:
            import paho.mqtt.client as mqtt
            
            # Create client
            self.client = mqtt.Client(client_id=f"{self.node_id}_{int(time.time())}")
            
            # Set callbacks
            self.client.on_connect = self._on_connect
            self.client.on_disconnect = self._on_disconnect
            self.client.on_publish = self._on_publish
            
            # Set authentication
            if self.username and self.password:
                self.client.username_pw_set(self.username, self.password)
            
            # Set TLS
            if self.use_tls and self.ca_certs:
                self.client.tls_set(ca_certs=self.ca_certs)
            
            logger.info(f"MQTT client initialized for broker: {self.broker}:{self.port}")
            
        except ImportError:
            logger.error("paho-mqtt not installed")
            logger.info("Install with: pip install paho-mqtt")
            self.enabled = False
        except Exception as e:
            logger.error(f"Failed to initialize MQTT client: {e}")
            self.enabled = False
    
    def start(self):
        """Connect to MQTT broker"""
        if not self.enabled or not self.client:
            logger.info("MQTT publisher disabled or not initialized")
            return
        
        try:
            self.stats['connection_attempts'] += 1
            self.client.connect(self.broker, self.port, self.keepalive)
            
            # Start network loop in background thread
            self.client.loop_start()
            
            logger.info(f"Connecting to MQTT broker: {self.broker}:{self.port}")
            
        except Exception as e:
            logger.error(f"Failed to connect to MQTT broker: {e}")
    
    def stop(self):
        """Disconnect from MQTT broker"""
        if self.client and self.connected:
            try:
                self.client.loop_stop()
                self.client.disconnect()
                logger.info("Disconnected from MQTT broker")
            except Exception as e:
                logger.error(f"Error disconnecting from MQTT: {e}")
    
    def _on_connect(self, client, userdata, flags, rc):
        """Callback for connection events"""
        if rc == 0:
            self.connected = True
            logger.info(f"Connected to MQTT broker: {self.broker}")
            
            # Publish connection status
            self._publish_status("online")
        else:
            self.connected = False
            logger.error(f"MQTT connection failed with code {rc}")
    
    def _on_disconnect(self, client, userdata, rc):
        """Callback for disconnection events"""
        self.connected = False
        if rc != 0:
            logger.warning(f"Unexpected MQTT disconnection (code {rc}), will auto-reconnect")
        else:
            logger.info("Disconnected from MQTT broker")
    
    def _on_publish(self, client, userdata, mid):
        """Callback for publish events"""
        logger.debug(f"Message {mid} published")
    
    def publish_detection(self, detection) -> bool:
        """
        Publish detection event
        
        Args:
            detection: IMX500Detection object
            
        Returns:
            True if published successfully, False otherwise
        """
        if not self.enabled or not self.connected:
            return False
        
        try:
            # Build detection message
            message = {
                'node_id': self.node_id,
                'timestamp': detection.timestamp.isoformat(),
                'species': detection.species,
                'confidence': detection.confidence,
                'bbox': {
                    'x': detection.bbox[0],
                    'y': detection.bbox[1],
                    'width': detection.bbox[2],
                    'height': detection.bbox[3],
                },
                'inference_time_ms': detection.inference_time_ms,
            }
            
            # Publish to detections topic
            topic = f"{self.topic_prefix}/{self.node_id}/detections"
            payload = json.dumps(message)
            
            result = self.client.publish(topic, payload, qos=self.qos, retain=self.retain)
            
            if result.rc == 0:
                self.stats['total_published'] += 1
                self.stats['last_publish_time'] = datetime.now()
                logger.debug(f"Published detection: {detection.species} to {topic}")
                return True
            else:
                self.stats['total_failed'] += 1
                logger.error(f"Failed to publish detection (rc={result.rc})")
                return False
                
        except Exception as e:
            logger.error(f"Error publishing detection: {e}")
            self.stats['total_failed'] += 1
            return False
    
    def publish_telemetry(self, telemetry: Dict) -> bool:
        """
        Publish telemetry data
        
        Args:
            telemetry: Telemetry dictionary
            
        Returns:
            True if published successfully, False otherwise
        """
        if not self.enabled or not self.connected:
            return False
        
        try:
            # Add node ID and timestamp
            message = {
                'node_id': self.node_id,
                'timestamp': datetime.now().isoformat(),
                **telemetry
            }
            
            # Publish to telemetry topic
            topic = f"{self.topic_prefix}/{self.node_id}/telemetry"
            payload = json.dumps(message)
            
            result = self.client.publish(topic, payload, qos=self.qos, retain=self.retain)
            
            if result.rc == 0:
                logger.debug(f"Published telemetry to {topic}")
                return True
            else:
                logger.error(f"Failed to publish telemetry (rc={result.rc})")
                return False
                
        except Exception as e:
            logger.error(f"Error publishing telemetry: {e}")
            return False
    
    def publish_image_metadata(self, image_path: str, detection) -> bool:
        """
        Publish image metadata (not the full image)
        
        Args:
            image_path: Path to captured image
            detection: Detection that triggered capture
            
        Returns:
            True if published successfully, False otherwise
        """
        if not self.enabled or not self.connected:
            return False
        
        try:
            import os
            
            message = {
                'node_id': self.node_id,
                'timestamp': datetime.now().isoformat(),
                'image_path': os.path.basename(image_path),
                'file_size': os.path.getsize(image_path),
                'species': detection.species,
                'confidence': detection.confidence,
            }
            
            # Publish to images topic
            topic = f"{self.topic_prefix}/{self.node_id}/images"
            payload = json.dumps(message)
            
            result = self.client.publish(topic, payload, qos=self.qos, retain=self.retain)
            
            if result.rc == 0:
                logger.debug(f"Published image metadata to {topic}")
                return True
            else:
                logger.error(f"Failed to publish image metadata (rc={result.rc})")
                return False
                
        except Exception as e:
            logger.error(f"Error publishing image metadata: {e}")
            return False
    
    def _publish_status(self, status: str):
        """Publish device status"""
        try:
            topic = f"{self.topic_prefix}/{self.node_id}/status"
            message = {
                'node_id': self.node_id,
                'timestamp': datetime.now().isoformat(),
                'status': status,
            }
            
            self.client.publish(topic, json.dumps(message), qos=1, retain=True)
            
        except Exception as e:
            logger.error(f"Error publishing status: {e}")
    
    def get_stats(self) -> Dict:
        """
        Get publisher statistics
        
        Returns:
            Dictionary with publisher stats
        """
        stats = self.stats.copy()
        stats['enabled'] = self.enabled
        stats['connected'] = self.connected
        stats['broker'] = self.broker
        stats['port'] = self.port
        
        # Calculate success rate
        total = stats['total_published'] + stats['total_failed']
        if total > 0:
            stats['success_rate'] = stats['total_published'] / total
        else:
            stats['success_rate'] = 0.0
        
        return stats
