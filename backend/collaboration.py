"""
Collaboration Service for Real-time Team Coordination
Handles user presence, annotations, chat, and collaborative features
"""

from flask_socketio import emit, join_room, leave_room
from datetime import datetime, timedelta
from models import db, User, UserSession, Annotation, ChatMessage, SharedBookmark, Task, FieldNote
import logging
import json

logger = logging.getLogger(__name__)

class CollaborationService:
    """Service for managing collaborative features"""
    
    def __init__(self, socketio):
        self.socketio = socketio
        self.active_sessions = {}  # session_id -> user_info
        
    # ===== USER PRESENCE =====
    
    def user_connected(self, user_id, session_id, socket_id, ip_address, user_agent):
        """Register user connection and create session"""
        try:
            # Get user info
            user = User.query.get(user_id)
            if not user:
                return None
            
            # Create or update session
            session = UserSession.query.filter_by(session_id=session_id).first()
            if not session:
                session = UserSession(
                    user_id=user_id,
                    session_id=session_id,
                    socket_id=socket_id,
                    ip_address=ip_address,
                    user_agent=user_agent
                )
                db.session.add(session)
            else:
                session.socket_id = socket_id
                session.last_activity = datetime.utcnow()
                session.is_active = True
            
            db.session.commit()
            
            # Cache session info
            self.active_sessions[session_id] = {
                'user_id': user_id,
                'username': user.username,
                'role': user.role,
                'socket_id': socket_id,
                'connected_at': datetime.utcnow().isoformat()
            }
            
            # Broadcast presence to all users
            self.broadcast_presence_update()
            
            logger.info(f"User {user.username} connected with session {session_id}")
            return session.to_dict()
            
        except Exception as e:
            logger.error(f"Error in user_connected: {e}")
            db.session.rollback()
            return None
    
    def user_disconnected(self, session_id):
        """Handle user disconnection"""
        try:
            session = UserSession.query.filter_by(session_id=session_id).first()
            if session:
                session.is_active = False
                session.last_activity = datetime.utcnow()
                db.session.commit()
            
            # Remove from active sessions
            if session_id in self.active_sessions:
                user_info = self.active_sessions[session_id]
                del self.active_sessions[session_id]
                logger.info(f"User {user_info['username']} disconnected")
            
            # Broadcast presence update
            self.broadcast_presence_update()
            
        except Exception as e:
            logger.error(f"Error in user_disconnected: {e}")
            db.session.rollback()
    
    def update_user_activity(self, session_id, current_page=None, cursor_position=None):
        """Update user activity and position"""
        try:
            session = UserSession.query.filter_by(session_id=session_id).first()
            if session:
                session.last_activity = datetime.utcnow()
                if current_page:
                    session.current_page = current_page
                if cursor_position:
                    session.cursor_position = cursor_position
                db.session.commit()
                
                # Broadcast cursor position to other users on same page
                if cursor_position and current_page:
                    self.broadcast_cursor_update(session_id, current_page, cursor_position)
                    
        except Exception as e:
            logger.error(f"Error updating user activity: {e}")
            db.session.rollback()
    
    def get_active_users(self):
        """Get list of currently active users"""
        try:
            # Clean up stale sessions (inactive for > 5 minutes)
            stale_threshold = datetime.utcnow() - timedelta(minutes=5)
            UserSession.query.filter(
                UserSession.last_activity < stale_threshold,
                UserSession.is_active == True
            ).update({'is_active': False})
            db.session.commit()
            
            # Get active sessions
            active_sessions = UserSession.query.filter_by(is_active=True).all()
            
            users = []
            for session in active_sessions:
                user = User.query.get(session.user_id)
                if user:
                    users.append({
                        'user_id': user.id,
                        'username': user.username,
                        'role': user.role,
                        'current_page': session.current_page,
                        'last_activity': session.last_activity.isoformat()
                    })
            
            return users
            
        except Exception as e:
            logger.error(f"Error getting active users: {e}")
            return []
    
    def broadcast_presence_update(self):
        """Broadcast active users list to all connected clients"""
        try:
            active_users = self.get_active_users()
            self.socketio.emit('presence_update', {
                'active_users': active_users,
                'count': len(active_users)
            }, namespace='/')
        except Exception as e:
            logger.error(f"Error broadcasting presence: {e}")
    
    def broadcast_cursor_update(self, session_id, page, position):
        """Broadcast cursor position to users on same page"""
        try:
            if session_id in self.active_sessions:
                user_info = self.active_sessions[session_id]
                self.socketio.emit('cursor_update', {
                    'user_id': user_info['user_id'],
                    'username': user_info['username'],
                    'page': page,
                    'position': position
                }, namespace='/', room=page)
        except Exception as e:
            logger.error(f"Error broadcasting cursor: {e}")
    
    # ===== ANNOTATIONS =====
    
    def create_annotation(self, detection_id, user_id, annotation_type, content, position, metadata=None):
        """Create a new annotation on a detection"""
        try:
            annotation = Annotation(
                detection_id=detection_id,
                user_id=user_id,
                annotation_type=annotation_type,
                content=content,
                position=position,
                metadata=metadata or {}
            )
            db.session.add(annotation)
            db.session.commit()
            
            # Broadcast to all users viewing this detection
            self.socketio.emit('annotation_created', {
                'annotation': annotation.to_dict(),
                'detection_id': detection_id
            }, namespace='/')
            
            return annotation.to_dict()
            
        except Exception as e:
            logger.error(f"Error creating annotation: {e}")
            db.session.rollback()
            return None
    
    def get_annotations(self, detection_id):
        """Get all annotations for a detection"""
        try:
            annotations = Annotation.query.filter_by(detection_id=detection_id).all()
            return [ann.to_dict() for ann in annotations]
        except Exception as e:
            logger.error(f"Error getting annotations: {e}")
            return []
    
    # ===== CHAT =====
    
    def send_chat_message(self, user_id, channel, message, parent_id=None, mentions=None):
        """Send a chat message"""
        try:
            user = User.query.get(user_id)
            if not user:
                return None
            
            chat_msg = ChatMessage(
                user_id=user_id,
                channel=channel,
                message=message,
                parent_id=parent_id,
                mentions=mentions or []
            )
            db.session.add(chat_msg)
            db.session.commit()
            
            # Broadcast to channel
            self.socketio.emit('chat_message', {
                'id': chat_msg.id,
                'user': {
                    'id': user.id,
                    'username': user.username,
                    'role': user.role
                },
                'channel': channel,
                'message': message,
                'parent_id': parent_id,
                'mentions': mentions,
                'created_at': chat_msg.created_at.isoformat()
            }, namespace='/', room=channel)
            
            # Send notifications to mentioned users
            if mentions:
                for mentioned_user_id in mentions:
                    self.send_mention_notification(mentioned_user_id, user.username, message, channel)
            
            return chat_msg.to_dict()
            
        except Exception as e:
            logger.error(f"Error sending chat message: {e}")
            db.session.rollback()
            return None
    
    def get_chat_messages(self, channel, limit=50):
        """Get recent chat messages for a channel"""
        try:
            messages = ChatMessage.query.filter_by(
                channel=channel,
                deleted=False
            ).order_by(ChatMessage.created_at.desc()).limit(limit).all()
            
            result = []
            for msg in messages:
                user = User.query.get(msg.user_id)
                result.append({
                    **msg.to_dict(),
                    'user': {
                        'id': user.id,
                        'username': user.username,
                        'role': user.role
                    } if user else None
                })
            
            return result[::-1]  # Reverse to get chronological order
            
        except Exception as e:
            logger.error(f"Error getting chat messages: {e}")
            return []
    
    def send_mention_notification(self, user_id, mentioning_user, message, channel):
        """Send notification when user is mentioned"""
        try:
            # Find user's active sessions
            sessions = UserSession.query.filter_by(user_id=user_id, is_active=True).all()
            for session in sessions:
                if session.socket_id:
                    self.socketio.emit('mention_notification', {
                        'from_user': mentioning_user,
                        'message': message,
                        'channel': channel
                    }, namespace='/', room=session.socket_id)
        except Exception as e:
            logger.error(f"Error sending mention notification: {e}")
    
    # ===== SHARED BOOKMARKS =====
    
    def create_bookmark(self, user_id, title, description, detection_id=None, camera_id=None, 
                       tags=None, is_shared=False, shared_with=None):
        """Create a shared bookmark"""
        try:
            bookmark = SharedBookmark(
                user_id=user_id,
                title=title,
                description=description,
                detection_id=detection_id,
                camera_id=camera_id,
                tags=tags or [],
                is_shared=is_shared,
                shared_with=shared_with or []
            )
            db.session.add(bookmark)
            db.session.commit()
            
            # Broadcast if shared
            if is_shared:
                self.socketio.emit('bookmark_created', {
                    'bookmark': bookmark.to_dict()
                }, namespace='/')
            
            return bookmark.to_dict()
            
        except Exception as e:
            logger.error(f"Error creating bookmark: {e}")
            db.session.rollback()
            return None
    
    def get_bookmarks(self, user_id):
        """Get bookmarks accessible to user"""
        try:
            # Get user's own bookmarks
            own_bookmarks = SharedBookmark.query.filter_by(user_id=user_id).all()
            
            # Get shared bookmarks
            shared_bookmarks = SharedBookmark.query.filter(
                SharedBookmark.is_shared == True,
                db.or_(
                    SharedBookmark.shared_with.contains([user_id]),
                    SharedBookmark.shared_with.contains(['all'])
                )
            ).all()
            
            all_bookmarks = own_bookmarks + shared_bookmarks
            return [bm.to_dict() for bm in all_bookmarks]
            
        except Exception as e:
            logger.error(f"Error getting bookmarks: {e}")
            return []
    
    # ===== TASKS =====
    
    def create_task(self, title, description, created_by, task_type=None, priority='medium',
                   assigned_to=None, camera_id=None, detection_id=None, due_date=None):
        """Create a new task"""
        try:
            task = Task(
                title=title,
                description=description,
                created_by=created_by,
                task_type=task_type,
                priority=priority,
                assigned_to=assigned_to,
                camera_id=camera_id,
                detection_id=detection_id,
                due_date=due_date
            )
            db.session.add(task)
            db.session.commit()
            
            # Notify assigned user
            if assigned_to:
                self.socketio.emit('task_assigned', {
                    'task': task.to_dict()
                }, namespace='/', room=f'user_{assigned_to}')
            
            # Broadcast to all
            self.socketio.emit('task_created', {
                'task': task.to_dict()
            }, namespace='/')
            
            return task.to_dict()
            
        except Exception as e:
            logger.error(f"Error creating task: {e}")
            db.session.rollback()
            return None
    
    def update_task_status(self, task_id, status, user_id):
        """Update task status"""
        try:
            task = Task.query.get(task_id)
            if not task:
                return None
            
            task.status = status
            if status == 'completed':
                task.completed_at = datetime.utcnow()
            db.session.commit()
            
            # Broadcast update
            self.socketio.emit('task_updated', {
                'task': task.to_dict()
            }, namespace='/')
            
            return task.to_dict()
            
        except Exception as e:
            logger.error(f"Error updating task: {e}")
            db.session.rollback()
            return None
    
    def get_tasks(self, user_id=None, status=None):
        """Get tasks, optionally filtered by user or status"""
        try:
            query = Task.query
            
            if user_id:
                query = query.filter(
                    db.or_(
                        Task.assigned_to == user_id,
                        Task.created_by == user_id
                    )
                )
            
            if status:
                query = query.filter_by(status=status)
            
            tasks = query.order_by(Task.created_at.desc()).all()
            return [task.to_dict() for task in tasks]
            
        except Exception as e:
            logger.error(f"Error getting tasks: {e}")
            return []
    
    # ===== FIELD NOTES =====
    
    def create_field_note(self, user_id, title, content, note_type=None, camera_id=None,
                         detection_id=None, tags=None, attachments=None):
        """Create a shared field note"""
        try:
            note = FieldNote(
                user_id=user_id,
                title=title,
                content=content,
                note_type=note_type,
                camera_id=camera_id,
                detection_id=detection_id,
                tags=tags or [],
                attachments=attachments or []
            )
            db.session.add(note)
            db.session.commit()
            
            # Broadcast to team
            self.socketio.emit('field_note_created', {
                'note': note.to_dict()
            }, namespace='/')
            
            return note.to_dict()
            
        except Exception as e:
            logger.error(f"Error creating field note: {e}")
            db.session.rollback()
            return None
    
    def get_field_notes(self, camera_id=None, detection_id=None):
        """Get field notes, optionally filtered"""
        try:
            query = FieldNote.query.filter_by(is_shared=True)
            
            if camera_id:
                query = query.filter_by(camera_id=camera_id)
            if detection_id:
                query = query.filter_by(detection_id=detection_id)
            
            notes = query.order_by(FieldNote.created_at.desc()).all()
            
            result = []
            for note in notes:
                user = User.query.get(note.user_id)
                result.append({
                    **note.to_dict(),
                    'user': {
                        'id': user.id,
                        'username': user.username
                    } if user else None
                })
            
            return result
            
        except Exception as e:
            logger.error(f"Error getting field notes: {e}")
            return []
