"""
ESP32 Protocol Handler - HTTP API and WebSocket server

Handles communication with ESP32-CAM nodes via HTTP REST API and WebSockets.
Supports detection messages, image uploads, telemetry, heartbeats, and configuration distribution.
"""

import asyncio
from aiohttp import web
import aiohttp
from typing import Dict, Any, List, Optional, Set
from datetime import datetime, timedelta
import logging
import json

from edge.shared.protocols.esp32_messages import (
    ESP32Message, ESP32MessageType,
    DetectionMessage, TelemetryMessage, HeartbeatMessage
)

logger = logging.getLogger(__name__)


class ESP32Node:
    """Represents an ESP32 camera node"""
    
    def __init__(self, node_id: int):
        self.node_id = node_id
        self.last_seen = datetime.utcnow()
        self.battery_level = 0
        self.uptime = 0
        self.firmware_version = "unknown"
        self.role = "unknown"
        self.is_online = True
        self.detections_count = 0
        self.images_count = 0
        
    def update_from_heartbeat(self, heartbeat: HeartbeatMessage):
        """Update node info from heartbeat"""
        self.last_seen = datetime.utcnow()
        self.battery_level = heartbeat.payload.get('battery_level', 0)
        self.uptime = heartbeat.payload.get('uptime', 0)
        self.firmware_version = heartbeat.payload.get('firmware_version', 'unknown')
        self.role = heartbeat.payload.get('role', 'unknown')
        self.is_online = True
        
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            'node_id': self.node_id,
            'last_seen': self.last_seen.isoformat(),
            'battery_level': self.battery_level,
            'uptime': self.uptime,
            'firmware_version': self.firmware_version,
            'role': self.role,
            'is_online': self.is_online,
            'detections_count': self.detections_count,
            'images_count': self.images_count
        }


class ESP32ProtocolHandler:
    """
    ESP32 Protocol Handler
    
    Provides HTTP API and WebSocket server for ESP32 node communication.
    """
    
    def __init__(
        self,
        host: str = "0.0.0.0",
        http_port: int = 8080,
        ws_port: int = 8081,
        node_timeout: int = 300
    ):
        """
        Initialize ESP32 protocol handler
        
        Args:
            host: Server bind address
            http_port: HTTP API port
            ws_port: WebSocket server port
            node_timeout: Node offline timeout in seconds
        """
        self.host = host
        self.http_port = http_port
        self.ws_port = ws_port
        self.node_timeout = node_timeout
        
        # Node tracking
        self.nodes: Dict[int, ESP32Node] = {}
        
        # WebSocket connections
        self.ws_connections: Set[web.WebSocketResponse] = set()
        
        # Message callbacks
        self.detection_callback = None
        self.telemetry_callback = None
        self.image_callback = None
        
        # Servers
        self.http_app = None
        self.http_runner = None
        self.ws_app = None
        self.ws_runner = None
        
        # Monitoring task
        self.monitor_task: Optional[asyncio.Task] = None
        self.running = False
        
    def set_detection_callback(self, callback):
        """Set callback for detection messages"""
        self.detection_callback = callback
        
    def set_telemetry_callback(self, callback):
        """Set callback for telemetry messages"""
        self.telemetry_callback = callback
        
    def set_image_callback(self, callback):
        """Set callback for image messages"""
        self.image_callback = callback
        
    async def start(self):
        """Start HTTP and WebSocket servers"""
        if self.running:
            logger.warning("ESP32 protocol handler already running")
            return
            
        self.running = True
        logger.info("Starting ESP32 protocol handler")
        
        # Setup HTTP routes
        self.http_app = web.Application()
        self.http_app.router.add_post('/api/detection', self.handle_detection)
        self.http_app.router.add_post('/api/image', self.handle_image)
        self.http_app.router.add_post('/api/telemetry', self.handle_telemetry)
        self.http_app.router.add_post('/api/heartbeat', self.handle_heartbeat)
        self.http_app.router.add_get('/api/nodes', self.handle_get_nodes)
        self.http_app.router.add_get('/api/nodes/{node_id}/config', self.handle_get_config)
        
        # Start HTTP server
        self.http_runner = web.AppRunner(self.http_app)
        await self.http_runner.setup()
        http_site = web.TCPSite(self.http_runner, self.host, self.http_port)
        await http_site.start()
        logger.info(f"HTTP API server listening on {self.host}:{self.http_port}")
        
        # Setup WebSocket server
        self.ws_app = web.Application()
        self.ws_app.router.add_get('/ws', self.handle_websocket)
        
        # Start WebSocket server
        self.ws_runner = web.AppRunner(self.ws_app)
        await self.ws_runner.setup()
        ws_site = web.TCPSite(self.ws_runner, self.host, self.ws_port)
        await ws_site.start()
        logger.info(f"WebSocket server listening on {self.host}:{self.ws_port}")
        
        # Start node monitoring
        self.monitor_task = asyncio.create_task(self._monitor_nodes())
        
    async def stop(self):
        """Stop servers"""
        if not self.running:
            return
            
        self.running = False
        logger.info("Stopping ESP32 protocol handler")
        
        # Stop monitoring
        if self.monitor_task:
            self.monitor_task.cancel()
            try:
                await self.monitor_task
            except asyncio.CancelledError:
                pass
                
        # Close WebSocket connections
        for ws in self.ws_connections:
            await ws.close()
        self.ws_connections.clear()
        
        # Stop servers
        if self.http_runner:
            await self.http_runner.cleanup()
        if self.ws_runner:
            await self.ws_runner.cleanup()
            
    async def handle_detection(self, request: web.Request) -> web.Response:
        """Handle detection POST request"""
        try:
            data = await request.json()
            
            # Create detection message
            detection = DetectionMessage(
                node_id=data['node_id'],
                species=data.get('species', 'unknown'),
                confidence=data.get('confidence', 0.0),
                latitude=data.get('latitude', 0.0),
                longitude=data.get('longitude', 0.0),
                image_size=data.get('image_size', 0),
                has_image=data.get('has_image', False)
            )
            
            # Update node
            node = self._get_or_create_node(detection.node_id)
            node.last_seen = datetime.utcnow()
            node.detections_count += 1
            
            # Call callback
            if self.detection_callback:
                await self.detection_callback(detection)
                
            # Broadcast to WebSocket clients
            await self._broadcast_message(detection.to_dict())
            
            logger.info(f"Detection received from node {detection.node_id}: {detection.payload['species']}")
            
            return web.json_response({'status': 'ok'})
            
        except Exception as e:
            logger.error(f"Error handling detection: {e}")
            return web.json_response({'error': str(e)}, status=400)
            
    async def handle_image(self, request: web.Request) -> web.Response:
        """Handle image POST request"""
        try:
            reader = await request.multipart()
            
            node_id = None
            image_data = None
            metadata = {}
            
            async for field in reader:
                if field.name == 'node_id':
                    node_id = int(await field.text())
                elif field.name == 'metadata':
                    metadata = json.loads(await field.text())
                elif field.name == 'image':
                    image_data = await field.read()
                    
            if node_id is None or image_data is None:
                return web.json_response({'error': 'Missing node_id or image'}, status=400)
                
            # Update node
            node = self._get_or_create_node(node_id)
            node.last_seen = datetime.utcnow()
            node.images_count += 1
            
            # Call callback
            if self.image_callback:
                await self.image_callback(node_id, image_data, metadata)
                
            logger.info(f"Image received from node {node_id}, size: {len(image_data)} bytes")
            
            return web.json_response({'status': 'ok'})
            
        except Exception as e:
            logger.error(f"Error handling image: {e}")
            return web.json_response({'error': str(e)}, status=400)
            
    async def handle_telemetry(self, request: web.Request) -> web.Response:
        """Handle telemetry POST request"""
        try:
            data = await request.json()
            
            telemetry = TelemetryMessage(
                node_id=data['node_id'],
                battery_level=data.get('battery_level', 0),
                temperature=data.get('temperature'),
                humidity=data.get('humidity'),
                pressure=data.get('pressure'),
                rssi=data.get('rssi'),
                snr=data.get('snr')
            )
            
            # Update node
            node = self._get_or_create_node(telemetry.node_id)
            node.last_seen = datetime.utcnow()
            node.battery_level = telemetry.payload['battery_level']
            
            # Call callback
            if self.telemetry_callback:
                await self.telemetry_callback(telemetry)
                
            logger.debug(f"Telemetry received from node {telemetry.node_id}")
            
            return web.json_response({'status': 'ok'})
            
        except Exception as e:
            logger.error(f"Error handling telemetry: {e}")
            return web.json_response({'error': str(e)}, status=400)
            
    async def handle_heartbeat(self, request: web.Request) -> web.Response:
        """Handle heartbeat POST request"""
        try:
            data = await request.json()
            
            heartbeat = HeartbeatMessage(
                node_id=data['node_id'],
                role=data.get('role', 'node'),
                battery_level=data.get('battery_level', 0),
                uptime=data.get('uptime', 0),
                firmware_version=data.get('firmware_version', '3.1.0')
            )
            
            # Update node
            node = self._get_or_create_node(heartbeat.node_id)
            node.update_from_heartbeat(heartbeat)
            
            logger.debug(f"Heartbeat received from node {heartbeat.node_id}")
            
            return web.json_response({'status': 'ok'})
            
        except Exception as e:
            logger.error(f"Error handling heartbeat: {e}")
            return web.json_response({'error': str(e)}, status=400)
            
    async def handle_get_nodes(self, request: web.Request) -> web.Response:
        """Get all nodes status"""
        nodes = [node.to_dict() for node in self.nodes.values()]
        return web.json_response({'nodes': nodes})
        
    async def handle_get_config(self, request: web.Request) -> web.Response:
        """Get configuration for a node"""
        node_id = int(request.match_info['node_id'])
        
        # Return default configuration
        # In production, this could be customized per node
        config = {
            'detection_threshold': 0.5,
            'capture_interval': 30,
            'lora_frequency': 915.0,
            'sync_interval': 300
        }
        
        return web.json_response(config)
        
    async def handle_websocket(self, request: web.Request) -> web.WebSocketResponse:
        """Handle WebSocket connection"""
        ws = web.WebSocketResponse()
        await ws.prepare(request)
        
        self.ws_connections.add(ws)
        logger.info("WebSocket client connected")
        
        try:
            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    # Handle incoming WS messages if needed
                    logger.debug(f"WebSocket message: {msg.data}")
                elif msg.type == aiohttp.WSMsgType.ERROR:
                    logger.error(f"WebSocket error: {ws.exception()}")
        finally:
            self.ws_connections.discard(ws)
            logger.info("WebSocket client disconnected")
            
        return ws
        
    async def _broadcast_message(self, message: Dict[str, Any]):
        """Broadcast message to all WebSocket clients"""
        if not self.ws_connections:
            return
            
        message_json = json.dumps(message)
        for ws in self.ws_connections:
            try:
                await ws.send_str(message_json)
            except Exception as e:
                logger.error(f"Error broadcasting to WebSocket: {e}")
                
    def _get_or_create_node(self, node_id: int) -> ESP32Node:
        """Get existing node or create new one"""
        if node_id not in self.nodes:
            self.nodes[node_id] = ESP32Node(node_id)
            logger.info(f"New node registered: {node_id}")
        return self.nodes[node_id]
        
    async def _monitor_nodes(self):
        """Monitor node health and mark offline nodes"""
        while self.running:
            try:
                current_time = datetime.utcnow()
                timeout = timedelta(seconds=self.node_timeout)
                
                for node in self.nodes.values():
                    if current_time - node.last_seen > timeout:
                        if node.is_online:
                            node.is_online = False
                            logger.warning(f"Node {node.node_id} marked offline")
                            
            except Exception as e:
                logger.error(f"Node monitoring error: {e}")
                
            await asyncio.sleep(60)  # Check every minute
