"""
WebSocket Service for Real-Time Alert Delivery
Handles real-time push notifications to connected clients
"""

from flask_socketio import emit, join_room, leave_room, disconnect
from flask_jwt_extended import decode_token
from functools import wraps
import logging
from datetime import datetime
from typing import Dict, List, Optional

logger = logging.getLogger(__name__)


class WebSocketService:
    """
    Real-time WebSocket service for alert delivery
    """
    
    def __init__(self, socketio):
        self.socketio = socketio
        self.connected_users = {}  # user_id -> socket_id mapping
        self.user_rooms = {}  # user_id -> list of camera rooms
        
        # Register event handlers
        self._register_handlers()
    
    def _register_handlers(self):
        """Register WebSocket event handlers"""
        
        @self.socketio.on('connect')
        def handle_connect():
            """Handle client connection"""
            try:
                logger.info(f"Client attempting to connect")
                emit('connection_response', {
                    'status': 'connected',
                    'timestamp': datetime.utcnow().isoformat()
                })
            except Exception as e:
                logger.error(f"Connection error: {e}")
        
        @self.socketio.on('authenticate')
        def handle_authenticate(data):
            """Authenticate WebSocket connection with JWT"""
            try:
                token = data.get('token')
                if not token:
                    emit('auth_error', {'message': 'Token required'})
                    return
                
                # Decode JWT token
                decoded = decode_token(token)
                user_id = decoded.get('sub')
                
                if user_id:
                    # Store connection
                    from flask import request
                    self.connected_users[user_id] = request.sid
                    
                    logger.info(f"User {user_id} authenticated on WebSocket")
                    emit('authenticated', {
                        'user_id': user_id,
                        'timestamp': datetime.utcnow().isoformat()
                    })
                else:
                    emit('auth_error', {'message': 'Invalid token'})
                    
            except Exception as e:
                logger.error(f"Authentication error: {e}")
                emit('auth_error', {'message': 'Authentication failed'})
        
        @self.socketio.on('subscribe_camera')
        def handle_subscribe_camera(data):
            """Subscribe to alerts from specific camera"""
            try:
                from flask import request
                user_id = self._get_user_id(request.sid)
                if not user_id:
                    emit('error', {'message': 'Not authenticated'})
                    return
                
                camera_id = data.get('camera_id')
                if not camera_id:
                    emit('error', {'message': 'camera_id required'})
                    return
                
                # Join camera room
                room = f"camera_{camera_id}"
                join_room(room)
                
                # Track subscription
                if user_id not in self.user_rooms:
                    self.user_rooms[user_id] = []
                self.user_rooms[user_id].append(camera_id)
                
                logger.info(f"User {user_id} subscribed to camera {camera_id}")
                emit('subscribed', {
                    'camera_id': camera_id,
                    'room': room
                })
                
            except Exception as e:
                logger.error(f"Subscribe error: {e}")
                emit('error', {'message': 'Subscription failed'})
        
        @self.socketio.on('unsubscribe_camera')
        def handle_unsubscribe_camera(data):
            """Unsubscribe from camera alerts"""
            try:
                from flask import request
                user_id = self._get_user_id(request.sid)
                if not user_id:
                    return
                
                camera_id = data.get('camera_id')
                if not camera_id:
                    return
                
                # Leave camera room
                room = f"camera_{camera_id}"
                leave_room(room)
                
                # Update tracking
                if user_id in self.user_rooms:
                    if camera_id in self.user_rooms[user_id]:
                        self.user_rooms[user_id].remove(camera_id)
                
                logger.info(f"User {user_id} unsubscribed from camera {camera_id}")
                emit('unsubscribed', {'camera_id': camera_id})
                
            except Exception as e:
                logger.error(f"Unsubscribe error: {e}")
        
        @self.socketio.on('disconnect')
        def handle_disconnect():
            """Handle client disconnection"""
            try:
                from flask import request
                user_id = self._get_user_id(request.sid)
                if user_id:
                    # Clean up tracking
                    if user_id in self.connected_users:
                        del self.connected_users[user_id]
                    if user_id in self.user_rooms:
                        del self.user_rooms[user_id]
                    logger.info(f"User {user_id} disconnected")
            except Exception as e:
                logger.error(f"Disconnect error: {e}")
    
    def _get_user_id(self, socket_id: str) -> Optional[int]:
        """Get user_id from socket_id"""
        for user_id, sid in self.connected_users.items():
            if sid == socket_id:
                return user_id
        return None
    
    def broadcast_alert(self, alert_data: Dict, camera_id: Optional[str] = None):
        """
        Broadcast alert to connected clients
        
        Args:
            alert_data: Alert information
            camera_id: Optional camera ID to target specific room
        """
        try:
            if camera_id:
                # Send to specific camera room
                room = f"camera_{camera_id}"
                self.socketio.emit('new_alert', alert_data, room=room)
                logger.info(f"Alert broadcast to camera room {camera_id}")
            else:
                # Broadcast to all connected clients
                self.socketio.emit('new_alert', alert_data, broadcast=True)
                logger.info("Alert broadcast to all clients")
                
        except Exception as e:
            logger.error(f"Error broadcasting alert: {e}")
    
    def send_to_user(self, user_id: int, event: str, data: Dict):
        """
        Send message to specific user
        
        Args:
            user_id: Target user ID
            event: Event name
            data: Event data
        """
        try:
            socket_id = self.connected_users.get(user_id)
            if socket_id:
                self.socketio.emit(event, data, room=socket_id)
                logger.info(f"Message sent to user {user_id}: {event}")
            else:
                logger.warning(f"User {user_id} not connected")
                
        except Exception as e:
            logger.error(f"Error sending to user: {e}")
    
    def notify_alert_update(self, alert_id: int, update_type: str, data: Dict):
        """
        Notify clients about alert updates (acknowledged, resolved, feedback)
        
        Args:
            alert_id: Alert ID
            update_type: Type of update (acknowledged, resolved, feedback)
            data: Update data
        """
        try:
            event_data = {
                'alert_id': alert_id,
                'update_type': update_type,
                'data': data,
                'timestamp': datetime.utcnow().isoformat()
            }
            
            self.socketio.emit('alert_update', event_data, broadcast=True)
            logger.info(f"Alert update broadcast: {alert_id} - {update_type}")
            
        except Exception as e:
            logger.error(f"Error notifying alert update: {e}")
    
    def send_analytics_update(self, analytics_data: Dict):
        """
        Send analytics updates to dashboard clients
        
        Args:
            analytics_data: Analytics information
        """
        try:
            self.socketio.emit('analytics_update', analytics_data, broadcast=True)
            logger.info("Analytics update broadcast")
            
        except Exception as e:
            logger.error(f"Error sending analytics update: {e}")
    
    def get_connected_users_count(self) -> int:
        """Get count of connected users"""
        return len(self.connected_users)
    
    def is_user_connected(self, user_id: int) -> bool:
        """Check if user is connected"""
        return user_id in self.connected_users
    
    def get_user_subscriptions(self, user_id: int) -> List[str]:
        """Get list of camera subscriptions for user"""
        return self.user_rooms.get(user_id, [])


# Global websocket service instance (initialized in app factory)
websocket_service = None


def init_websocket_service(socketio):
    """Initialize WebSocket service"""
    global websocket_service
    websocket_service = WebSocketService(socketio)
    return websocket_service
