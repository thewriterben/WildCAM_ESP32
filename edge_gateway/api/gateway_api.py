"""
REST/WebSocket API for Edge Gateway Management
Provides endpoints for monitoring, configuration, and control
"""

from aiohttp import web
import socketio
import json
import logging
from typing import Dict, Optional
from pathlib import Path
import asyncio

logger = logging.getLogger(__name__)


class GatewayAPI:
    """
    REST and WebSocket API for edge gateway management
    """
    
    def __init__(
        self,
        host: str = '0.0.0.0',
        port: int = 8000,
        gateway_service=None
    ):
        """
        Initialize gateway API
        
        Args:
            host: Bind host
            port: Bind port
            gateway_service: Reference to main gateway service
        """
        self.host = host
        self.port = port
        self.gateway_service = gateway_service
        
        # Create aiohttp app
        self.app = web.Application()
        
        # Create Socket.IO server
        self.sio = socketio.AsyncServer(
            async_mode='aiohttp',
            cors_allowed_origins='*'
        )
        self.sio.attach(self.app)
        
        # App runner
        self.runner = None
        
        # Connected WebSocket clients
        self.clients = set()
        
        # Setup routes
        self._setup_routes()
        self._setup_socketio()
    
    def _setup_routes(self):
        """Setup HTTP routes"""
        # Health and status
        self.app.router.add_get('/api/health', self.handle_health)
        self.app.router.add_get('/api/status', self.handle_status)
        self.app.router.add_get('/api/stats', self.handle_stats)
        
        # Configuration
        self.app.router.add_get('/api/config', self.handle_get_config)
        self.app.router.add_post('/api/config', self.handle_update_config)
        
        # Camera control
        self.app.router.add_get('/api/cameras', self.handle_list_cameras)
        self.app.router.add_post('/api/cameras/{camera_name}/enable', self.handle_enable_camera)
        self.app.router.add_post('/api/cameras/{camera_name}/disable', self.handle_disable_camera)
        
        # Detection results
        self.app.router.add_get('/api/detections/recent', self.handle_recent_detections)
        self.app.router.add_get('/api/detections/{detection_id}', self.handle_get_detection)
        
        # ESP32 nodes
        self.app.router.add_get('/api/nodes', self.handle_list_nodes)
        self.app.router.add_get('/api/nodes/{node_id}', self.handle_get_node)
        self.app.router.add_post('/api/nodes/{node_id}/command', self.handle_send_command)
        
        # System control
        self.app.router.add_post('/api/system/restart', self.handle_restart)
        self.app.router.add_post('/api/system/shutdown', self.handle_shutdown)
    
    def _setup_socketio(self):
        """Setup Socket.IO event handlers"""
        
        @self.sio.event
        async def connect(sid, environ):
            self.clients.add(sid)
            logger.info(f"Client connected: {sid}")
            await self.sio.emit('connected', {'sid': sid}, room=sid)
        
        @self.sio.event
        async def disconnect(sid):
            self.clients.discard(sid)
            logger.info(f"Client disconnected: {sid}")
        
        @self.sio.event
        async def subscribe_detections(sid, data):
            """Subscribe to real-time detection events"""
            logger.info(f"Client {sid} subscribed to detections")
            await self.sio.emit('subscribed', {'stream': 'detections'}, room=sid)
        
        @self.sio.event
        async def get_status(sid, data):
            """Get current system status"""
            status = self._get_system_status()
            await self.sio.emit('status', status, room=sid)
    
    async def start(self):
        """Start API server"""
        self.runner = web.AppRunner(self.app)
        await self.runner.setup()
        site = web.TCPSite(self.runner, self.host, self.port)
        await site.start()
        logger.info(f"Gateway API started on {self.host}:{self.port}")
    
    async def stop(self):
        """Stop API server"""
        if self.runner:
            await self.runner.cleanup()
        logger.info("Gateway API stopped")
    
    # HTTP Handlers
    
    async def handle_health(self, request: web.Request) -> web.Response:
        """Health check endpoint"""
        return web.json_response({
            'status': 'healthy',
            'service': 'wildcam-jetson-gateway'
        })
    
    async def handle_status(self, request: web.Request) -> web.Response:
        """Get system status"""
        status = self._get_system_status()
        return web.json_response(status)
    
    async def handle_stats(self, request: web.Request) -> web.Response:
        """Get detailed statistics"""
        stats = self._get_system_stats()
        return web.json_response(stats)
    
    async def handle_get_config(self, request: web.Request) -> web.Response:
        """Get current configuration"""
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        config = self.gateway_service.get_config()
        return web.json_response(config)
    
    async def handle_update_config(self, request: web.Request) -> web.Response:
        """Update configuration"""
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        try:
            updates = await request.json()
            self.gateway_service.update_config(updates)
            
            return web.json_response({
                'status': 'updated',
                'message': 'Configuration updated successfully'
            })
        except Exception as e:
            logger.error(f"Failed to update config: {e}")
            return web.json_response({
                'error': str(e)
            }, status=400)
    
    async def handle_list_cameras(self, request: web.Request) -> web.Response:
        """List all cameras"""
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        cameras = self.gateway_service.get_cameras()
        return web.json_response({'cameras': cameras})
    
    async def handle_enable_camera(self, request: web.Request) -> web.Response:
        """Enable camera"""
        camera_name = request.match_info['camera_name']
        
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        try:
            self.gateway_service.enable_camera(camera_name)
            return web.json_response({
                'status': 'enabled',
                'camera': camera_name
            })
        except Exception as e:
            return web.json_response({'error': str(e)}, status=400)
    
    async def handle_disable_camera(self, request: web.Request) -> web.Response:
        """Disable camera"""
        camera_name = request.match_info['camera_name']
        
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        try:
            self.gateway_service.disable_camera(camera_name)
            return web.json_response({
                'status': 'disabled',
                'camera': camera_name
            })
        except Exception as e:
            return web.json_response({'error': str(e)}, status=400)
    
    async def handle_recent_detections(self, request: web.Request) -> web.Response:
        """Get recent detections"""
        limit = int(request.query.get('limit', 50))
        
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        detections = self.gateway_service.get_recent_detections(limit)
        return web.json_response({'detections': detections})
    
    async def handle_get_detection(self, request: web.Request) -> web.Response:
        """Get specific detection"""
        detection_id = request.match_info['detection_id']
        
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        detection = self.gateway_service.get_detection(detection_id)
        if detection:
            return web.json_response(detection)
        else:
            return web.json_response({'error': 'Detection not found'}, status=404)
    
    async def handle_list_nodes(self, request: web.Request) -> web.Response:
        """List ESP32 nodes"""
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        nodes = self.gateway_service.get_esp32_nodes()
        return web.json_response({'nodes': nodes})
    
    async def handle_get_node(self, request: web.Request) -> web.Response:
        """Get specific ESP32 node"""
        node_id = request.match_info['node_id']
        
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        node = self.gateway_service.get_esp32_node(node_id)
        if node:
            return web.json_response(node)
        else:
            return web.json_response({'error': 'Node not found'}, status=404)
    
    async def handle_send_command(self, request: web.Request) -> web.Response:
        """Send command to ESP32 node"""
        node_id = request.match_info['node_id']
        
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        try:
            command_data = await request.json()
            await self.gateway_service.send_node_command(node_id, command_data)
            
            return web.json_response({
                'status': 'sent',
                'node_id': node_id
            })
        except Exception as e:
            return web.json_response({'error': str(e)}, status=400)
    
    async def handle_restart(self, request: web.Request) -> web.Response:
        """Restart gateway service"""
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        asyncio.create_task(self._delayed_restart())
        
        return web.json_response({
            'status': 'restarting',
            'message': 'Gateway will restart in 5 seconds'
        })
    
    async def handle_shutdown(self, request: web.Request) -> web.Response:
        """Shutdown gateway service"""
        if not self.gateway_service:
            return web.json_response({'error': 'Service not available'}, status=503)
        
        asyncio.create_task(self._delayed_shutdown())
        
        return web.json_response({
            'status': 'shutting_down',
            'message': 'Gateway will shutdown in 5 seconds'
        })
    
    # Helper methods
    
    def _get_system_status(self) -> Dict:
        """Get current system status"""
        if not self.gateway_service:
            return {'status': 'unavailable'}
        
        return self.gateway_service.get_status()
    
    def _get_system_stats(self) -> Dict:
        """Get detailed system statistics"""
        if not self.gateway_service:
            return {'stats': 'unavailable'}
        
        return self.gateway_service.get_stats()
    
    async def _delayed_restart(self):
        """Restart after delay"""
        await asyncio.sleep(5)
        if self.gateway_service:
            await self.gateway_service.restart()
    
    async def _delayed_shutdown(self):
        """Shutdown after delay"""
        await asyncio.sleep(5)
        if self.gateway_service:
            await self.gateway_service.shutdown()
    
    async def broadcast_detection(self, detection: Dict):
        """Broadcast detection event to all connected clients"""
        await self.sio.emit('detection', detection)
        logger.debug(f"Broadcasted detection to {len(self.clients)} clients")
    
    async def broadcast_status_update(self, status: Dict):
        """Broadcast status update to all connected clients"""
        await self.sio.emit('status_update', status)
