"""
MQTT Bridge - Publishes gateway data to MQTT broker
Supports local Mosquitto, AWS IoT, Azure IoT Hub, and other MQTT brokers
"""

import logging
import json
import time
import ssl
from typing import Optional, Dict, Any
from threading import Thread, Event

try:
    import paho.mqtt.client as mqtt
    MQTT_AVAILABLE = True
except ImportError:
    MQTT_AVAILABLE = False
    logging.warning("Paho MQTT library not available. Install with: pip install paho-mqtt")

from config import MQTTConfig
from database import GatewayDatabase


logger = logging.getLogger(__name__)


class MQTTBridge:
    """
    MQTT bridge for publishing gateway data to brokers
    """
    
    def __init__(self, config: MQTTConfig, database: GatewayDatabase):
        """
        Initialize MQTT bridge
        
        Args:
            config: MQTT configuration
            database: Database instance
        """
        self.config = config
        self.db = database
        self.client: Optional[mqtt.Client] = None
        self.connected = False
        self.running = False
        self.reconnect_thread = None
        self.stop_event = Event()
        
        # Statistics
        self.messages_published = 0
        self.publish_errors = 0
    
    def initialize(self) -> bool:
        """
        Initialize MQTT client
        
        Returns:
            True if initialization successful
        """
        if not MQTT_AVAILABLE:
            logger.error("Paho MQTT library not available")
            return False
        
        if not self.config.enabled:
            logger.info("MQTT bridge is disabled in configuration")
            return False
        
        try:
            # Create MQTT client
            self.client = mqtt.Client(client_id=self.config.client_id)
            
            # Set callbacks
            self.client.on_connect = self._on_connect
            self.client.on_disconnect = self._on_disconnect
            self.client.on_publish = self._on_publish
            
            # Set credentials if provided
            if self.config.username and self.config.password:
                self.client.username_pw_set(
                    self.config.username,
                    self.config.password
                )
            
            # Configure TLS if enabled
            if self.config.use_tls:
                tls_context = ssl.create_default_context()
                
                if self.config.ca_cert:
                    tls_context.load_verify_locations(self.config.ca_cert)
                
                if self.config.client_cert and self.config.client_key:
                    tls_context.load_cert_chain(
                        certfile=self.config.client_cert,
                        keyfile=self.config.client_key
                    )
                
                self.client.tls_set_context(tls_context)
            
            logger.info(f"MQTT client initialized for {self.config.broker_host}:"
                       f"{self.config.broker_port}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize MQTT client: {e}")
            return False
    
    def connect(self) -> bool:
        """
        Connect to MQTT broker
        
        Returns:
            True if connection successful
        """
        if not self.client:
            logger.error("MQTT client not initialized")
            return False
        
        try:
            logger.info(f"Connecting to MQTT broker at "
                       f"{self.config.broker_host}:{self.config.broker_port}...")
            
            self.client.connect(
                self.config.broker_host,
                self.config.broker_port,
                self.config.keepalive
            )
            
            # Start network loop
            self.client.loop_start()
            
            # Wait for connection
            timeout = 10
            start = time.time()
            while not self.connected and (time.time() - start) < timeout:
                time.sleep(0.1)
            
            if self.connected:
                logger.info("Connected to MQTT broker")
                return True
            else:
                logger.error("Connection timeout")
                return False
                
        except Exception as e:
            logger.error(f"Failed to connect to MQTT broker: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from MQTT broker"""
        if self.client:
            self.client.loop_stop()
            self.client.disconnect()
            self.connected = False
            logger.info("Disconnected from MQTT broker")
    
    def start(self):
        """Start MQTT bridge with auto-reconnect"""
        if not self.config.enabled:
            return
        
        self.running = True
        self.stop_event.clear()
        
        # Start reconnect monitoring thread
        self.reconnect_thread = Thread(target=self._reconnect_loop, daemon=True)
        self.reconnect_thread.start()
        
        logger.info("MQTT bridge started")
    
    def stop(self):
        """Stop MQTT bridge"""
        self.running = False
        self.stop_event.set()
        
        if self.reconnect_thread:
            self.reconnect_thread.join(timeout=5)
        
        self.disconnect()
        logger.info("MQTT bridge stopped")
    
    def _reconnect_loop(self):
        """Monitor connection and reconnect if needed"""
        while self.running and not self.stop_event.is_set():
            if not self.connected:
                logger.info("Attempting to reconnect to MQTT broker...")
                if self.connect():
                    logger.info("Reconnection successful")
                else:
                    logger.warning(f"Reconnection failed, retrying in "
                                 f"{self.config.reconnect_delay}s")
                    time.sleep(self.config.reconnect_delay)
            
            # Check every second
            time.sleep(1)
    
    def _on_connect(self, client, userdata, flags, rc):
        """Callback for successful connection"""
        if rc == 0:
            self.connected = True
            logger.info("MQTT connected successfully")
            
            # Publish gateway online status
            self._publish_status("online")
        else:
            self.connected = False
            logger.error(f"MQTT connection failed with code {rc}")
    
    def _on_disconnect(self, client, userdata, rc):
        """Callback for disconnection"""
        self.connected = False
        if rc != 0:
            logger.warning(f"MQTT unexpected disconnect (code {rc})")
        else:
            logger.info("MQTT disconnected")
    
    def _on_publish(self, client, userdata, mid):
        """Callback for published message"""
        logger.debug(f"Message {mid} published")
    
    def publish_detection(self, detection: Dict[str, Any]) -> bool:
        """
        Publish wildlife detection event
        
        Args:
            detection: Detection data dictionary
            
        Returns:
            True if published successfully
        """
        if not self.connected:
            logger.warning("Not connected to MQTT broker")
            return False
        
        try:
            topic = f"{self.config.topic_prefix}/{self.config.detection_topic}"
            payload = json.dumps(detection)
            
            result = self.client.publish(
                topic,
                payload,
                qos=self.config.qos,
                retain=self.config.retain
            )
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                self.messages_published += 1
                logger.debug(f"Published detection to {topic}")
                return True
            else:
                self.publish_errors += 1
                logger.error(f"Failed to publish detection: {result.rc}")
                return False
                
        except Exception as e:
            self.publish_errors += 1
            logger.error(f"Error publishing detection: {e}")
            return False
    
    def publish_telemetry(self, telemetry: Dict[str, Any]) -> bool:
        """
        Publish telemetry data
        
        Args:
            telemetry: Telemetry data dictionary
            
        Returns:
            True if published successfully
        """
        if not self.connected:
            return False
        
        try:
            topic = f"{self.config.topic_prefix}/{self.config.telemetry_topic}"
            payload = json.dumps(telemetry)
            
            result = self.client.publish(
                topic,
                payload,
                qos=self.config.qos,
                retain=self.config.retain
            )
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                self.messages_published += 1
                logger.debug(f"Published telemetry to {topic}")
                return True
            else:
                self.publish_errors += 1
                return False
                
        except Exception as e:
            self.publish_errors += 1
            logger.error(f"Error publishing telemetry: {e}")
            return False
    
    def publish_health(self, health_data: Dict[str, Any]) -> bool:
        """
        Publish gateway health data
        
        Args:
            health_data: Health data dictionary
            
        Returns:
            True if published successfully
        """
        if not self.connected:
            return False
        
        try:
            topic = f"{self.config.topic_prefix}/{self.config.health_topic}"
            payload = json.dumps(health_data)
            
            result = self.client.publish(
                topic,
                payload,
                qos=self.config.qos,
                retain=True  # Retain health status
            )
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                self.messages_published += 1
                return True
            else:
                self.publish_errors += 1
                return False
                
        except Exception as e:
            self.publish_errors += 1
            logger.error(f"Error publishing health: {e}")
            return False
    
    def _publish_status(self, status: str):
        """Publish gateway status"""
        topic = f"{self.config.topic_prefix}/gateway/status"
        self.client.publish(topic, status, qos=1, retain=True)
    
    def publish_node_update(self, node_data: Dict[str, Any]) -> bool:
        """
        Publish node status update
        
        Args:
            node_data: Node data dictionary
            
        Returns:
            True if published successfully
        """
        if not self.connected:
            return False
        
        try:
            node_id = node_data.get('node_id', 'unknown')
            topic = f"{self.config.topic_prefix}/nodes/{node_id}"
            payload = json.dumps(node_data)
            
            result = self.client.publish(
                topic,
                payload,
                qos=self.config.qos,
                retain=True  # Retain node status
            )
            
            if result.rc == mqtt.MQTT_ERR_SUCCESS:
                self.messages_published += 1
                return True
            else:
                self.publish_errors += 1
                return False
                
        except Exception as e:
            self.publish_errors += 1
            logger.error(f"Error publishing node update: {e}")
            return False
    
    def get_stats(self) -> Dict[str, Any]:
        """Get MQTT bridge statistics"""
        return {
            'connected': self.connected,
            'messages_published': self.messages_published,
            'publish_errors': self.publish_errors,
            'broker': f"{self.config.broker_host}:{self.config.broker_port}",
            'client_id': self.config.client_id
        }
