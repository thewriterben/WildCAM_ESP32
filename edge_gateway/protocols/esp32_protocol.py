"""
ESP32 Protocol Handler
Manages communication with ESP32 field nodes via HTTP/WebSocket and LoRa relay
"""

import asyncio
import json
import logging
import serial
import threading
from typing import Dict, List, Optional, Callable
from aiohttp import web
import socketio
from pathlib import Path
from dataclasses import dataclass
from datetime import datetime
import struct

logger = logging.getLogger(__name__)


@dataclass
class ESP32Node:
    """ESP32 field node information"""
    node_id: str
    name: str
    ip_address: Optional[str] = None
    last_seen: float = 0
    firmware_version: str = "unknown"
    battery_level: float = 0
    status: str = "offline"
    metadata: Dict = None
    
    def __post_init__(self):
        if self.metadata is None:
            self.metadata = {}


class ESP32HTTPHandler:
    """
    HTTP API handler for ESP32 field nodes
    Provides REST endpoints for data upload, configuration, and control
    """
    
    def __init__(self, host: str = '0.0.0.0', port: int = 8080):
        """
        Initialize HTTP handler
        
        Args:
            host: Bind host
            port: Bind port
        """
        self.host = host
        self.port = port
        self.app = web.Application()
        self.runner = None
        self.nodes: Dict[str, ESP32Node] = {}
        
        # Callbacks
        self.data_callback: Optional[Callable] = None
        self.image_callback: Optional[Callable] = None
        
        # Setup routes
        self._setup_routes()
    
    def _setup_routes(self):
        """Setup HTTP routes"""
        self.app.router.add_post('/api/node/register', self.handle_register)
        self.app.router.add_post('/api/node/{node_id}/data', self.handle_data)
        self.app.router.add_post('/api/node/{node_id}/image', self.handle_image)
        self.app.router.add_get('/api/node/{node_id}/config', self.handle_get_config)
        self.app.router.add_post('/api/node/{node_id}/config', self.handle_set_config)
        self.app.router.add_get('/api/nodes', self.handle_list_nodes)
        self.app.router.add_get('/api/health', self.handle_health)
    
    async def start(self):
        """Start HTTP server"""
        self.runner = web.AppRunner(self.app)
        await self.runner.setup()
        site = web.TCPSite(self.runner, self.host, self.port)
        await site.start()
        logger.info(f"ESP32 HTTP handler started on {self.host}:{self.port}")
    
    async def stop(self):
        """Stop HTTP server"""
        if self.runner:
            await self.runner.cleanup()
        logger.info("ESP32 HTTP handler stopped")
    
    async def handle_register(self, request: web.Request) -> web.Response:
        """Handle node registration"""
        data = await request.json()
        
        node_id = data.get('node_id')
        if not node_id:
            return web.json_response({'error': 'node_id required'}, status=400)
        
        node = ESP32Node(
            node_id=node_id,
            name=data.get('name', node_id),
            ip_address=request.remote,
            firmware_version=data.get('firmware_version', 'unknown'),
            battery_level=data.get('battery_level', 0),
            status='online',
            metadata=data.get('metadata', {})
        )
        node.last_seen = datetime.now().timestamp()
        
        self.nodes[node_id] = node
        
        logger.info(f"Node registered: {node_id} from {request.remote}")
        
        return web.json_response({
            'status': 'registered',
            'node_id': node_id,
            'server_time': datetime.now().isoformat()
        })
    
    async def handle_data(self, request: web.Request) -> web.Response:
        """Handle data upload from node"""
        node_id = request.match_info['node_id']
        data = await request.json()
        
        # Update node status
        if node_id in self.nodes:
            self.nodes[node_id].last_seen = datetime.now().timestamp()
            self.nodes[node_id].status = 'online'
        
        # Call data callback
        if self.data_callback:
            await self.data_callback(node_id, data)
        
        logger.debug(f"Received data from node {node_id}")
        
        return web.json_response({'status': 'received'})
    
    async def handle_image(self, request: web.Request) -> web.Response:
        """Handle image upload from node"""
        node_id = request.match_info['node_id']
        
        # Read multipart form data
        reader = await request.multipart()
        image_data = None
        metadata = {}
        
        async for field in reader:
            if field.name == 'image':
                image_data = await field.read()
            elif field.name == 'metadata':
                metadata = json.loads(await field.read(decode=True))
        
        if not image_data:
            return web.json_response({'error': 'No image data'}, status=400)
        
        # Update node status
        if node_id in self.nodes:
            self.nodes[node_id].last_seen = datetime.now().timestamp()
        
        # Call image callback
        if self.image_callback:
            await self.image_callback(node_id, image_data, metadata)
        
        logger.info(f"Received image from node {node_id}, size: {len(image_data)} bytes")
        
        return web.json_response({'status': 'received'})
    
    async def handle_get_config(self, request: web.Request) -> web.Response:
        """Get node configuration"""
        node_id = request.match_info['node_id']
        
        # TODO: Load from config storage
        config = {
            'capture_interval': 300,
            'detection_enabled': True,
            'upload_enabled': True
        }
        
        return web.json_response(config)
    
    async def handle_set_config(self, request: web.Request) -> web.Response:
        """Update node configuration"""
        node_id = request.match_info['node_id']
        config = await request.json()
        
        # TODO: Save to config storage
        logger.info(f"Updated config for node {node_id}")
        
        return web.json_response({'status': 'updated'})
    
    async def handle_list_nodes(self, request: web.Request) -> web.Response:
        """List all registered nodes"""
        nodes_list = [
            {
                'node_id': node.node_id,
                'name': node.name,
                'ip_address': node.ip_address,
                'last_seen': node.last_seen,
                'firmware_version': node.firmware_version,
                'battery_level': node.battery_level,
                'status': node.status
            }
            for node in self.nodes.values()
        ]
        
        return web.json_response({'nodes': nodes_list})
    
    async def handle_health(self, request: web.Request) -> web.Response:
        """Health check endpoint"""
        return web.json_response({
            'status': 'healthy',
            'nodes_count': len(self.nodes)
        })


class ESP32WebSocketHandler:
    """
    WebSocket handler for real-time communication with ESP32 nodes
    """
    
    def __init__(self, port: int = 8081):
        """
        Initialize WebSocket handler
        
        Args:
            port: WebSocket server port
        """
        self.port = port
        self.sio = socketio.AsyncServer(async_mode='aiohttp', cors_allowed_origins='*')
        self.app = web.Application()
        self.sio.attach(self.app)
        self.runner = None
        
        # Connected clients
        self.clients: Dict[str, str] = {}  # sid -> node_id
        
        # Setup event handlers
        self._setup_handlers()
    
    def _setup_handlers(self):
        """Setup Socket.IO event handlers"""
        
        @self.sio.event
        async def connect(sid, environ):
            logger.info(f"WebSocket client connected: {sid}")
        
        @self.sio.event
        async def disconnect(sid):
            if sid in self.clients:
                node_id = self.clients[sid]
                del self.clients[sid]
                logger.info(f"Node {node_id} disconnected")
            else:
                logger.info(f"WebSocket client disconnected: {sid}")
        
        @self.sio.event
        async def register(sid, data):
            node_id = data.get('node_id')
            if node_id:
                self.clients[sid] = node_id
                logger.info(f"Node registered via WebSocket: {node_id}")
                await self.sio.emit('registered', {'node_id': node_id}, room=sid)
        
        @self.sio.event
        async def data(sid, data):
            if sid in self.clients:
                node_id = self.clients[sid]
                logger.debug(f"Received data from {node_id} via WebSocket")
                # Process data here
        
        @self.sio.event
        async def heartbeat(sid, data):
            if sid in self.clients:
                await self.sio.emit('heartbeat_ack', {}, room=sid)
    
    async def start(self):
        """Start WebSocket server"""
        self.runner = web.AppRunner(self.app)
        await self.runner.setup()
        site = web.TCPSite(self.runner, '0.0.0.0', self.port)
        await site.start()
        logger.info(f"ESP32 WebSocket handler started on port {self.port}")
    
    async def stop(self):
        """Stop WebSocket server"""
        if self.runner:
            await self.runner.cleanup()
        logger.info("ESP32 WebSocket handler stopped")
    
    async def send_command(self, node_id: str, command: str, params: Dict = None):
        """Send command to specific node"""
        # Find socket ID for node
        sid = None
        for s, n in self.clients.items():
            if n == node_id:
                sid = s
                break
        
        if sid:
            await self.sio.emit('command', {
                'command': command,
                'params': params or {}
            }, room=sid)
            logger.info(f"Sent command '{command}' to node {node_id}")
        else:
            logger.warning(f"Node {node_id} not connected via WebSocket")


class LoRaGateway:
    """
    LoRa gateway interface for ESP32 communication
    Handles serial communication with LoRa transceiver
    """
    
    def __init__(
        self,
        port: str = '/dev/ttyUSB0',
        baudrate: int = 115200,
        timeout: float = 1.0
    ):
        """
        Initialize LoRa gateway
        
        Args:
            port: Serial port device
            baudrate: Serial baud rate
            timeout: Serial timeout in seconds
        """
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        
        self.serial: Optional[serial.Serial] = None
        self.running = False
        self.receive_thread = None
        
        # Message handlers
        self.message_handlers: Dict[str, Callable] = {}
        
        # Statistics
        self.messages_sent = 0
        self.messages_received = 0
    
    def start(self):
        """Start LoRa gateway"""
        try:
            self.serial = serial.Serial(
                self.port,
                self.baudrate,
                timeout=self.timeout
            )
            self.running = True
            
            # Start receive thread
            self.receive_thread = threading.Thread(
                target=self._receive_loop,
                daemon=True
            )
            self.receive_thread.start()
            
            logger.info(f"LoRa gateway started on {self.port}")
        except Exception as e:
            logger.error(f"Failed to start LoRa gateway: {e}")
            raise
    
    def stop(self):
        """Stop LoRa gateway"""
        self.running = False
        
        if self.receive_thread:
            self.receive_thread.join(timeout=5.0)
        
        if self.serial:
            self.serial.close()
        
        logger.info("LoRa gateway stopped")
    
    def send_message(self, node_id: str, message: Dict) -> bool:
        """
        Send message to ESP32 node via LoRa
        
        Args:
            node_id: Target node ID
            message: Message payload
            
        Returns:
            True if sent successfully
        """
        if not self.serial or not self.serial.is_open:
            logger.error("LoRa gateway not connected")
            return False
        
        try:
            # Create LoRa packet
            # Format: [node_id][message_type][payload_length][payload][checksum]
            message_json = json.dumps(message)
            payload = message_json.encode('utf-8')
            
            packet = struct.pack(
                f'<16sHH{len(payload)}sH',
                node_id.encode('utf-8').ljust(16, b'\x00'),
                1,  # message_type
                len(payload),
                payload,
                self._calculate_checksum(payload)
            )
            
            self.serial.write(packet)
            self.messages_sent += 1
            
            logger.debug(f"Sent LoRa message to {node_id}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to send LoRa message: {e}")
            return False
    
    def _receive_loop(self):
        """Receive loop running in separate thread"""
        logger.info("LoRa receive loop started")
        
        while self.running:
            try:
                if self.serial.in_waiting > 0:
                    # Read packet header
                    header = self.serial.read(20)  # 16 + 2 + 2
                    
                    if len(header) < 20:
                        continue
                    
                    node_id = header[:16].decode('utf-8').strip('\x00')
                    message_type, payload_length = struct.unpack('<HH', header[16:20])
                    
                    # Read payload and checksum
                    payload = self.serial.read(payload_length)
                    checksum = struct.unpack('<H', self.serial.read(2))[0]
                    
                    # Verify checksum
                    if checksum != self._calculate_checksum(payload):
                        logger.warning("LoRa packet checksum mismatch")
                        continue
                    
                    # Parse message
                    try:
                        message = json.loads(payload.decode('utf-8'))
                        self.messages_received += 1
                        self._handle_message(node_id, message)
                    except json.JSONDecodeError as e:
                        logger.error(f"Failed to parse LoRa message: {e}")
                
            except Exception as e:
                logger.error(f"Error in LoRa receive loop: {e}")
                
            # Small delay to prevent CPU spinning
            threading.Event().wait(0.1)
        
        logger.info("LoRa receive loop stopped")
    
    def _handle_message(self, node_id: str, message: Dict):
        """Handle received message"""
        message_type = message.get('type', 'unknown')
        
        if message_type in self.message_handlers:
            self.message_handlers[message_type](node_id, message)
        else:
            logger.debug(f"Received LoRa message from {node_id}: {message_type}")
    
    def register_handler(self, message_type: str, handler: Callable):
        """Register message handler"""
        self.message_handlers[message_type] = handler
        logger.info(f"Registered LoRa handler for message type: {message_type}")
    
    @staticmethod
    def _calculate_checksum(data: bytes) -> int:
        """Calculate simple checksum"""
        return sum(data) & 0xFFFF
    
    def get_stats(self) -> Dict:
        """Get gateway statistics"""
        return {
            'running': self.running,
            'port': self.port,
            'messages_sent': self.messages_sent,
            'messages_received': self.messages_received
        }


class ESP32ProtocolHandler:
    """
    Main protocol handler coordinating HTTP, WebSocket, and LoRa interfaces
    """
    
    def __init__(
        self,
        http_port: int = 8080,
        websocket_port: int = 8081,
        lora_port: Optional[str] = None
    ):
        """
        Initialize ESP32 protocol handler
        
        Args:
            http_port: HTTP API port
            websocket_port: WebSocket port
            lora_port: LoRa serial port (optional)
        """
        self.http_handler = ESP32HTTPHandler(port=http_port)
        self.websocket_handler = ESP32WebSocketHandler(port=websocket_port)
        self.lora_gateway = LoRaGateway(lora_port) if lora_port else None
        
        # Setup callbacks
        self.http_handler.data_callback = self._handle_node_data
        self.http_handler.image_callback = self._handle_node_image
        
        if self.lora_gateway:
            self.lora_gateway.register_handler('data', self._handle_lora_data)
    
    async def start(self):
        """Start all protocol handlers"""
        await self.http_handler.start()
        await self.websocket_handler.start()
        
        if self.lora_gateway:
            self.lora_gateway.start()
        
        logger.info("ESP32 protocol handler started")
    
    async def stop(self):
        """Stop all protocol handlers"""
        await self.http_handler.stop()
        await self.websocket_handler.stop()
        
        if self.lora_gateway:
            self.lora_gateway.stop()
        
        logger.info("ESP32 protocol handler stopped")
    
    async def _handle_node_data(self, node_id: str, data: Dict):
        """Handle data from ESP32 node"""
        logger.info(f"Received data from node {node_id}")
        # Process node data (e.g., add to cloud sync queue)
    
    async def _handle_node_image(self, node_id: str, image_data: bytes, metadata: Dict):
        """Handle image from ESP32 node"""
        logger.info(f"Received image from node {node_id}, size: {len(image_data)} bytes")
        # Process image (e.g., save, detect, sync to cloud)
    
    def _handle_lora_data(self, node_id: str, message: Dict):
        """Handle data from LoRa"""
        logger.info(f"Received LoRa data from node {node_id}")
        # Process LoRa message
    
    def get_stats(self) -> Dict:
        """Get protocol handler statistics"""
        stats = {
            'http': {
                'nodes_count': len(self.http_handler.nodes)
            },
            'websocket': {
                'clients_count': len(self.websocket_handler.clients)
            }
        }
        
        if self.lora_gateway:
            stats['lora'] = self.lora_gateway.get_stats()
        
        return stats
