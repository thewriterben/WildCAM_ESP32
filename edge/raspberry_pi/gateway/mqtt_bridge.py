"""
MQTT Bridge - Bridge LoRa data to MQTT brokers

Supports local Mosquitto and cloud MQTT (AWS IoT, Azure IoT Hub).
"""

import asyncio
import json
from typing import Optional, Dict, Any
from datetime import datetime
import logging

try:
    import asyncio_mqtt as aiomqtt
except ImportError:
    # Fallback for older versions
    import aiomqtt

logger = logging.getLogger(__name__)


class MQTTBridge:
    """
    MQTT Bridge for LoRa gateway
    
    Bridges LoRa packets to MQTT broker with topic-based routing.
    """
    
    def __init__(
        self,
        broker_host: str,
        broker_port: int = 1883,
        username: Optional[str] = None,
        password: Optional[str] = None,
        client_id: str = "wildcam_lora_gateway",
        use_tls: bool = False,
        base_topic: str = "wildcam/lora"
    ):
        """
        Initialize MQTT bridge
        
        Args:
            broker_host: MQTT broker hostname
            broker_port: MQTT broker port
            username: Optional username for authentication
            password: Optional password for authentication
            client_id: MQTT client ID
            use_tls: Use TLS/SSL connection
            base_topic: Base topic for all messages
        """
        self.broker_host = broker_host
        self.broker_port = broker_port
        self.username = username
        self.password = password
        self.client_id = client_id
        self.use_tls = use_tls
        self.base_topic = base_topic.rstrip('/')
        
        # MQTT client
        self.client: Optional[aiomqtt.Client] = None
        
        # Control
        self.running = False
        self.reconnect_interval = 5
        self.max_reconnect_interval = 300  # 5 minutes
        
        # Statistics
        self.stats = {
            'published': 0,
            'failed': 0,
            'connected': False,
            'last_publish': None
        }
        
    async def start(self):
        """Start MQTT bridge"""
        if self.running:
            logger.warning("MQTT bridge already running")
            return
            
        self.running = True
        logger.info(f"Starting MQTT bridge to {self.broker_host}:{self.broker_port}")
        
        # Start connection loop
        asyncio.create_task(self._connection_loop())
        
    async def stop(self):
        """Stop MQTT bridge"""
        if not self.running:
            return
            
        self.running = False
        logger.info("Stopping MQTT bridge")
        
        if self.client:
            try:
                await self.client.disconnect()
            except Exception as e:
                logger.error(f"Error disconnecting MQTT: {e}")
                
        self.stats['connected'] = False
        
    async def _connection_loop(self):
        """Maintain MQTT connection with auto-reconnect"""
        reconnect_delay = self.reconnect_interval
        
        while self.running:
            try:
                # Connect to MQTT broker
                async with aiomqtt.Client(
                    hostname=self.broker_host,
                    port=self.broker_port,
                    username=self.username,
                    password=self.password,
                    identifier=self.client_id,
                    tls_context=None if not self.use_tls else True
                ) as client:
                    self.client = client
                    self.stats['connected'] = True
                    logger.info("MQTT connected")
                    
                    # Reset reconnect delay on successful connection
                    reconnect_delay = self.reconnect_interval
                    
                    # Keep connection alive
                    while self.running:
                        await asyncio.sleep(1)
                        
            except Exception as e:
                self.stats['connected'] = False
                logger.error(f"MQTT connection error: {e}")
                
                # Exponential backoff
                logger.info(f"Reconnecting in {reconnect_delay} seconds")
                await asyncio.sleep(reconnect_delay)
                reconnect_delay = min(reconnect_delay * 2, self.max_reconnect_interval)
                
    async def publish_detection(self, detection: Dict[str, Any]):
        """
        Publish wildlife detection to MQTT
        
        Args:
            detection: Detection data dictionary
        """
        if not self.client or not self.stats['connected']:
            logger.warning("MQTT not connected, skipping detection publish")
            self.stats['failed'] += 1
            return
            
        try:
            topic = f"{self.base_topic}/detections/{detection.get('node_id', 0)}"
            payload = json.dumps(detection)
            
            await self.client.publish(topic, payload, qos=1)
            
            self.stats['published'] += 1
            self.stats['last_publish'] = datetime.utcnow()
            logger.info(f"Published detection to {topic}")
            
        except Exception as e:
            logger.error(f"Detection publish error: {e}")
            self.stats['failed'] += 1
            
    async def publish_telemetry(self, telemetry: Dict[str, Any]):
        """
        Publish telemetry to MQTT
        
        Args:
            telemetry: Telemetry data dictionary
        """
        if not self.client or not self.stats['connected']:
            logger.warning("MQTT not connected, skipping telemetry publish")
            self.stats['failed'] += 1
            return
            
        try:
            topic = f"{self.base_topic}/telemetry/{telemetry.get('node_id', 0)}"
            payload = json.dumps(telemetry)
            
            await self.client.publish(topic, payload, qos=0)
            
            self.stats['published'] += 1
            self.stats['last_publish'] = datetime.utcnow()
            logger.debug(f"Published telemetry to {topic}")
            
        except Exception as e:
            logger.error(f"Telemetry publish error: {e}")
            self.stats['failed'] += 1
            
    async def publish_status(self, status: Dict[str, Any]):
        """
        Publish gateway status to MQTT
        
        Args:
            status: Status data dictionary
        """
        if not self.client or not self.stats['connected']:
            return
            
        try:
            topic = f"{self.base_topic}/gateway/status"
            payload = json.dumps(status)
            
            await self.client.publish(topic, payload, qos=1, retain=True)
            
            self.stats['published'] += 1
            logger.debug("Published gateway status")
            
        except Exception as e:
            logger.error(f"Status publish error: {e}")
            self.stats['failed'] += 1
            
    async def publish_packet(self, packet: Dict[str, Any]):
        """
        Publish raw packet to MQTT
        
        Args:
            packet: Packet data dictionary
        """
        if not self.client or not self.stats['connected']:
            logger.warning("MQTT not connected, skipping packet publish")
            self.stats['failed'] += 1
            return
            
        try:
            topic = f"{self.base_topic}/packets/{packet.get('node_id', 0)}"
            payload = json.dumps(packet)
            
            await self.client.publish(topic, payload, qos=0)
            
            self.stats['published'] += 1
            logger.debug(f"Published packet to {topic}")
            
        except Exception as e:
            logger.error(f"Packet publish error: {e}")
            self.stats['failed'] += 1
            
    def get_statistics(self) -> Dict[str, Any]:
        """Get MQTT bridge statistics"""
        return {
            **self.stats,
            'last_publish': self.stats['last_publish'].isoformat() if self.stats['last_publish'] else None
        }
