"""
WebRTC Signaling Server for WildCAM Collaboration

Handles WebRTC peer connection signaling for video/audio calls and screen sharing.
This is a foundation for Phase 3 WebRTC implementation.
"""

import logging
from datetime import datetime
from flask_socketio import emit, join_room, leave_room
from flask import session


logger = logging.getLogger(__name__)


class WebRTCSignalingServer:
    """
    Manages WebRTC signaling for peer-to-peer connections
    
    Handles:
    - ICE candidate exchange
    - SDP offer/answer exchange
    - Room management for multi-party calls
    - Call state tracking
    """
    
    def __init__(self, socketio, db):
        self.socketio = socketio
        self.db = db
        self.active_rooms = {}  # room_id -> {participants, start_time, type}
        self.peer_connections = {}  # connection_id -> {from_user, to_user, state}
        
        # Register event handlers
        self.register_handlers()
    
    def register_handlers(self):
        """Register WebSocket event handlers for WebRTC signaling"""
        
        @self.socketio.on('webrtc:join-room')
        def handle_join_room(data):
            """User joins a call room"""
            room_id = data.get('room_id')
            user_id = session.get('user_id')
            username = session.get('username')
            media_type = data.get('media_type', 'audio-video')  # audio, video, screen
            
            if not all([room_id, user_id]):
                emit('webrtc:error', {'error': 'Missing required fields'})
                return
            
            # Join Socket.IO room
            join_room(room_id)
            
            # Track room participation
            if room_id not in self.active_rooms:
                self.active_rooms[room_id] = {
                    'participants': {},
                    'start_time': datetime.utcnow(),
                    'type': media_type
                }
            
            self.active_rooms[room_id]['participants'][user_id] = {
                'username': username,
                'media_type': media_type,
                'joined_at': datetime.utcnow()
            }
            
            # Notify existing participants
            emit('webrtc:user-joined', {
                'room_id': room_id,
                'user_id': user_id,
                'username': username,
                'media_type': media_type,
                'participant_count': len(self.active_rooms[room_id]['participants'])
            }, room=room_id, skip_sid=session.get('sid'))
            
            # Send list of existing participants to new user
            emit('webrtc:room-state', {
                'room_id': room_id,
                'participants': [
                    {
                        'user_id': uid,
                        'username': info['username'],
                        'media_type': info['media_type']
                    }
                    for uid, info in self.active_rooms[room_id]['participants'].items()
                    if uid != user_id
                ]
            })
            
            logger.info(f"User {user_id} joined WebRTC room {room_id}")
        
        @self.socketio.on('webrtc:leave-room')
        def handle_leave_room(data):
            """User leaves a call room"""
            room_id = data.get('room_id')
            user_id = session.get('user_id')
            
            if not all([room_id, user_id]):
                return
            
            # Leave Socket.IO room
            leave_room(room_id)
            
            # Remove from tracking
            if room_id in self.active_rooms:
                if user_id in self.active_rooms[room_id]['participants']:
                    del self.active_rooms[room_id]['participants'][user_id]
                
                # Clean up empty rooms
                if len(self.active_rooms[room_id]['participants']) == 0:
                    del self.active_rooms[room_id]
                else:
                    # Notify remaining participants
                    emit('webrtc:user-left', {
                        'room_id': room_id,
                        'user_id': user_id,
                        'participant_count': len(self.active_rooms[room_id]['participants'])
                    }, room=room_id)
            
            logger.info(f"User {user_id} left WebRTC room {room_id}")
        
        @self.socketio.on('webrtc:offer')
        def handle_offer(data):
            """
            Forward WebRTC offer from one peer to another
            
            Offer contains SDP (Session Description Protocol) with media capabilities
            """
            room_id = data.get('room_id')
            target_user_id = data.get('target_user_id')
            offer = data.get('offer')
            from_user_id = session.get('user_id')
            
            if not all([room_id, target_user_id, offer]):
                emit('webrtc:error', {'error': 'Missing required fields'})
                return
            
            # Create connection tracking
            connection_id = f"{from_user_id}-{target_user_id}"
            self.peer_connections[connection_id] = {
                'from_user': from_user_id,
                'to_user': target_user_id,
                'state': 'offered',
                'timestamp': datetime.utcnow()
            }
            
            # Forward offer to target user
            emit('webrtc:offer', {
                'room_id': room_id,
                'from_user_id': from_user_id,
                'offer': offer
            }, room=room_id)
            
            logger.debug(f"WebRTC offer from {from_user_id} to {target_user_id}")
        
        @self.socketio.on('webrtc:answer')
        def handle_answer(data):
            """
            Forward WebRTC answer from peer accepting the call
            
            Answer contains SDP with agreed-upon media configuration
            """
            room_id = data.get('room_id')
            target_user_id = data.get('target_user_id')
            answer = data.get('answer')
            from_user_id = session.get('user_id')
            
            if not all([room_id, target_user_id, answer]):
                emit('webrtc:error', {'error': 'Missing required fields'})
                return
            
            # Update connection state
            connection_id = f"{target_user_id}-{from_user_id}"
            if connection_id in self.peer_connections:
                self.peer_connections[connection_id]['state'] = 'answered'
            
            # Forward answer to initiating user
            emit('webrtc:answer', {
                'room_id': room_id,
                'from_user_id': from_user_id,
                'answer': answer
            }, room=room_id)
            
            logger.debug(f"WebRTC answer from {from_user_id} to {target_user_id}")
        
        @self.socketio.on('webrtc:ice-candidate')
        def handle_ice_candidate(data):
            """
            Forward ICE (Interactive Connectivity Establishment) candidate
            
            ICE candidates contain network information for establishing
            peer-to-peer connection through NAT/firewalls
            """
            room_id = data.get('room_id')
            target_user_id = data.get('target_user_id')
            candidate = data.get('candidate')
            from_user_id = session.get('user_id')
            
            if not all([room_id, candidate]):
                return
            
            # Forward ICE candidate
            if target_user_id:
                # Targeted to specific user
                emit('webrtc:ice-candidate', {
                    'room_id': room_id,
                    'from_user_id': from_user_id,
                    'candidate': candidate
                }, room=room_id)
            else:
                # Broadcast to room
                emit('webrtc:ice-candidate', {
                    'room_id': room_id,
                    'from_user_id': from_user_id,
                    'candidate': candidate
                }, room=room_id, skip_sid=session.get('sid'))
            
            logger.debug(f"ICE candidate from {from_user_id} in room {room_id}")
        
        @self.socketio.on('webrtc:connection-state')
        def handle_connection_state(data):
            """Track connection state changes"""
            room_id = data.get('room_id')
            state = data.get('state')  # connecting, connected, disconnected, failed
            user_id = session.get('user_id')
            
            if not all([room_id, state]):
                return
            
            # Broadcast state change
            emit('webrtc:peer-state-changed', {
                'room_id': room_id,
                'user_id': user_id,
                'state': state
            }, room=room_id)
            
            logger.info(f"WebRTC connection state for {user_id}: {state}")
        
        @self.socketio.on('webrtc:mute-audio')
        def handle_mute_audio(data):
            """Handle audio mute/unmute"""
            room_id = data.get('room_id')
            muted = data.get('muted', True)
            user_id = session.get('user_id')
            
            emit('webrtc:audio-state-changed', {
                'room_id': room_id,
                'user_id': user_id,
                'muted': muted
            }, room=room_id)
        
        @self.socketio.on('webrtc:mute-video')
        def handle_mute_video(data):
            """Handle video mute/unmute"""
            room_id = data.get('room_id')
            muted = data.get('muted', True)
            user_id = session.get('user_id')
            
            emit('webrtc:video-state-changed', {
                'room_id': room_id,
                'user_id': user_id,
                'muted': muted
            }, room=room_id)
        
        @self.socketio.on('webrtc:screen-share')
        def handle_screen_share(data):
            """Handle screen sharing start/stop"""
            room_id = data.get('room_id')
            sharing = data.get('sharing', False)
            user_id = session.get('user_id')
            username = session.get('username')
            
            emit('webrtc:screen-share-changed', {
                'room_id': room_id,
                'user_id': user_id,
                'username': username,
                'sharing': sharing
            }, room=room_id)
            
            logger.info(f"Screen sharing {('started' if sharing else 'stopped')} by {user_id}")
    
    def get_active_rooms(self):
        """Get list of active call rooms"""
        return {
            room_id: {
                'participant_count': len(info['participants']),
                'start_time': info['start_time'].isoformat(),
                'type': info['type'],
                'participants': [
                    {
                        'user_id': uid,
                        'username': pinfo['username']
                    }
                    for uid, pinfo in info['participants'].items()
                ]
            }
            for room_id, info in self.active_rooms.items()
        }
    
    def cleanup_stale_connections(self, max_age_seconds=3600):
        """Remove stale peer connections (for periodic cleanup)"""
        now = datetime.utcnow()
        stale_connections = []
        
        for conn_id, conn_info in self.peer_connections.items():
            age = (now - conn_info['timestamp']).total_seconds()
            if age > max_age_seconds:
                stale_connections.append(conn_id)
        
        for conn_id in stale_connections:
            del self.peer_connections[conn_id]
        
        return len(stale_connections)


# Configuration for STUN/TURN servers (needed for WebRTC)
WEBRTC_CONFIG = {
    'iceServers': [
        # Public STUN servers
        {'urls': 'stun:stun.l.google.com:19302'},
        {'urls': 'stun:stun1.l.google.com:19302'},
        
        # Add your TURN server here for production
        # {
        #     'urls': 'turn:your-turn-server.com:3478',
        #     'username': 'your-username',
        #     'credential': 'your-credential'
        # }
    ],
    'iceCandidatePoolSize': 10,
}


def get_webrtc_config():
    """
    Get WebRTC configuration for client
    
    In production, you should:
    1. Set up your own TURN server (coturn is popular)
    2. Generate time-limited credentials for security
    3. Use environment variables for configuration
    """
    return WEBRTC_CONFIG
