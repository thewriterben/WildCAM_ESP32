"""
Role-Based Access Control (RBAC) for Collaboration Features

Implements comprehensive role-based permissions for WildCAM collaboration platform.
"""

from enum import Enum
from functools import wraps
from flask import jsonify
from flask_jwt_extended import get_jwt_identity


class Role(Enum):
    """User roles in order of privilege level"""
    GUEST = 0
    OBSERVER = 1
    FIELD_WORKER = 2
    RESEARCHER = 3
    LEAD_RESEARCHER = 4
    ADMIN = 5


class Permission(Enum):
    """Granular permissions for collaboration features"""
    # Viewing permissions
    VIEW_DETECTIONS = "view_detections"
    VIEW_ANALYTICS = "view_analytics"
    VIEW_CAMERA_FEEDS = "view_camera_feeds"
    VIEW_FIELD_NOTES = "view_field_notes"
    
    # Basic collaboration
    SEND_CHAT_MESSAGES = "send_chat_messages"
    CREATE_ANNOTATIONS = "create_annotations"
    CREATE_BOOKMARKS = "create_bookmarks"
    
    # Advanced collaboration
    EDIT_ANNOTATIONS = "edit_annotations"
    DELETE_ANNOTATIONS = "delete_annotations"
    CREATE_TASKS = "create_tasks"
    ASSIGN_TASKS = "assign_tasks"
    
    # Management permissions
    MANAGE_USERS = "manage_users"
    MANAGE_PROJECTS = "manage_projects"
    MANAGE_CAMERAS = "manage_cameras"
    MANAGE_SETTINGS = "manage_settings"
    
    # Data permissions
    EXPORT_DATA = "export_data"
    DELETE_DATA = "delete_data"
    
    # Advanced features
    LOCK_VIEWPORT = "lock_viewport"
    INITIATE_CALLS = "initiate_calls"
    RECORD_MEETINGS = "record_meetings"


# Role to Permission mapping
ROLE_PERMISSIONS = {
    Role.GUEST: {
        Permission.VIEW_DETECTIONS,
        Permission.VIEW_ANALYTICS,
    },
    
    Role.OBSERVER: {
        Permission.VIEW_DETECTIONS,
        Permission.VIEW_ANALYTICS,
        Permission.VIEW_CAMERA_FEEDS,
        Permission.VIEW_FIELD_NOTES,
        Permission.SEND_CHAT_MESSAGES,
        Permission.CREATE_ANNOTATIONS,
        Permission.CREATE_BOOKMARKS,
    },
    
    Role.FIELD_WORKER: {
        Permission.VIEW_DETECTIONS,
        Permission.VIEW_ANALYTICS,
        Permission.VIEW_CAMERA_FEEDS,
        Permission.VIEW_FIELD_NOTES,
        Permission.SEND_CHAT_MESSAGES,
        Permission.CREATE_ANNOTATIONS,
        Permission.EDIT_ANNOTATIONS,
        Permission.CREATE_BOOKMARKS,
        Permission.CREATE_TASKS,
        Permission.MANAGE_CAMERAS,
    },
    
    Role.RESEARCHER: {
        Permission.VIEW_DETECTIONS,
        Permission.VIEW_ANALYTICS,
        Permission.VIEW_CAMERA_FEEDS,
        Permission.VIEW_FIELD_NOTES,
        Permission.SEND_CHAT_MESSAGES,
        Permission.CREATE_ANNOTATIONS,
        Permission.EDIT_ANNOTATIONS,
        Permission.DELETE_ANNOTATIONS,
        Permission.CREATE_BOOKMARKS,
        Permission.CREATE_TASKS,
        Permission.ASSIGN_TASKS,
        Permission.EXPORT_DATA,
        Permission.INITIATE_CALLS,
    },
    
    Role.LEAD_RESEARCHER: {
        Permission.VIEW_DETECTIONS,
        Permission.VIEW_ANALYTICS,
        Permission.VIEW_CAMERA_FEEDS,
        Permission.VIEW_FIELD_NOTES,
        Permission.SEND_CHAT_MESSAGES,
        Permission.CREATE_ANNOTATIONS,
        Permission.EDIT_ANNOTATIONS,
        Permission.DELETE_ANNOTATIONS,
        Permission.CREATE_BOOKMARKS,
        Permission.CREATE_TASKS,
        Permission.ASSIGN_TASKS,
        Permission.MANAGE_PROJECTS,
        Permission.MANAGE_CAMERAS,
        Permission.EXPORT_DATA,
        Permission.LOCK_VIEWPORT,
        Permission.INITIATE_CALLS,
        Permission.RECORD_MEETINGS,
    },
    
    Role.ADMIN: set(Permission),  # Admins have all permissions
}


class RBACService:
    """Service for managing role-based access control"""
    
    def __init__(self, db):
        self.db = db
    
    def get_user_role(self, user_id):
        """Get user's role"""
        from models import User
        user = User.query.get(user_id)
        if not user:
            return None
        
        # Map string role to Role enum
        role_map = {
            'admin': Role.ADMIN,
            'lead_researcher': Role.LEAD_RESEARCHER,
            'researcher': Role.RESEARCHER,
            'field_worker': Role.FIELD_WORKER,
            'observer': Role.OBSERVER,
            'guest': Role.GUEST,
        }
        
        return role_map.get(user.role, Role.OBSERVER)
    
    def has_permission(self, user_id, permission):
        """Check if user has a specific permission"""
        role = self.get_user_role(user_id)
        if not role:
            return False
        
        permissions = ROLE_PERMISSIONS.get(role, set())
        return permission in permissions
    
    def has_any_permission(self, user_id, permissions):
        """Check if user has any of the specified permissions"""
        return any(self.has_permission(user_id, perm) for perm in permissions)
    
    def has_all_permissions(self, user_id, permissions):
        """Check if user has all of the specified permissions"""
        return all(self.has_permission(user_id, perm) for perm in permissions)
    
    def get_user_permissions(self, user_id):
        """Get all permissions for a user"""
        role = self.get_user_role(user_id)
        if not role:
            return set()
        
        return ROLE_PERMISSIONS.get(role, set())
    
    def can_access_resource(self, user_id, resource_type, resource_id, action):
        """
        Check if user can perform action on a specific resource
        
        Args:
            user_id: User ID
            resource_type: Type of resource (detection, annotation, task, etc.)
            resource_id: ID of the resource
            action: Action to perform (view, edit, delete)
        """
        # Map resource actions to permissions
        permission_map = {
            'detection': {
                'view': Permission.VIEW_DETECTIONS,
                'edit': Permission.MANAGE_CAMERAS,
                'delete': Permission.DELETE_DATA,
            },
            'annotation': {
                'view': Permission.VIEW_DETECTIONS,
                'create': Permission.CREATE_ANNOTATIONS,
                'edit': Permission.EDIT_ANNOTATIONS,
                'delete': Permission.DELETE_ANNOTATIONS,
            },
            'task': {
                'view': Permission.VIEW_FIELD_NOTES,
                'create': Permission.CREATE_TASKS,
                'assign': Permission.ASSIGN_TASKS,
                'edit': Permission.CREATE_TASKS,
                'delete': Permission.MANAGE_PROJECTS,
            },
        }
        
        required_permission = permission_map.get(resource_type, {}).get(action)
        if not required_permission:
            return False
        
        return self.has_permission(user_id, required_permission)
    
    def is_admin(self, user_id):
        """Check if user is an admin"""
        role = self.get_user_role(user_id)
        return role == Role.ADMIN
    
    def is_lead_or_higher(self, user_id):
        """Check if user is Lead Researcher or Admin"""
        role = self.get_user_role(user_id)
        return role in [Role.LEAD_RESEARCHER, Role.ADMIN]
    
    def can_manage_user(self, manager_id, target_user_id):
        """Check if manager can manage target user"""
        manager_role = self.get_user_role(manager_id)
        target_role = self.get_user_role(target_user_id)
        
        if not manager_role or not target_role:
            return False
        
        # Admins can manage anyone
        if manager_role == Role.ADMIN:
            return True
        
        # Lead researchers can manage researchers and below
        if manager_role == Role.LEAD_RESEARCHER:
            return target_role.value <= Role.RESEARCHER.value
        
        return False


def require_permission(permission):
    """
    Decorator to require a specific permission for an endpoint
    
    Usage:
        @app.route('/api/admin/users')
        @jwt_required()
        @require_permission(Permission.MANAGE_USERS)
        def manage_users():
            # Only users with MANAGE_USERS permission can access
            pass
    """
    def decorator(f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            user_id = get_jwt_identity()
            
            # Get RBAC service from current app
            from flask import current_app
            rbac = current_app.config.get('rbac_service')
            
            if not rbac:
                return jsonify({'error': 'RBAC not configured'}), 500
            
            if not rbac.has_permission(user_id, permission):
                return jsonify({
                    'error': 'Insufficient permissions',
                    'required_permission': permission.value
                }), 403
            
            return f(*args, **kwargs)
        
        return decorated_function
    return decorator


def require_any_permission(*permissions):
    """
    Decorator to require any of the specified permissions
    
    Usage:
        @app.route('/api/data')
        @jwt_required()
        @require_any_permission(Permission.EXPORT_DATA, Permission.VIEW_ANALYTICS)
        def get_data():
            pass
    """
    def decorator(f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            user_id = get_jwt_identity()
            
            from flask import current_app
            rbac = current_app.config.get('rbac_service')
            
            if not rbac:
                return jsonify({'error': 'RBAC not configured'}), 500
            
            if not rbac.has_any_permission(user_id, permissions):
                return jsonify({
                    'error': 'Insufficient permissions',
                    'required_permissions': [p.value for p in permissions]
                }), 403
            
            return f(*args, **kwargs)
        
        return decorated_function
    return decorator


def require_role(role):
    """
    Decorator to require a minimum role level
    
    Usage:
        @app.route('/api/admin/settings')
        @jwt_required()
        @require_role(Role.ADMIN)
        def admin_settings():
            pass
    """
    def decorator(f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            user_id = get_jwt_identity()
            
            from flask import current_app
            rbac = current_app.config.get('rbac_service')
            
            if not rbac:
                return jsonify({'error': 'RBAC not configured'}), 500
            
            user_role = rbac.get_user_role(user_id)
            
            if not user_role or user_role.value < role.value:
                return jsonify({
                    'error': 'Insufficient role level',
                    'required_role': role.name,
                    'current_role': user_role.name if user_role else 'UNKNOWN'
                }), 403
            
            return f(*args, **kwargs)
        
        return decorated_function
    return decorator


def get_rbac_info(user_id):
    """
    Get complete RBAC information for a user (for debugging/display)
    
    Returns:
        dict with role, permissions, and capabilities
    """
    from flask import current_app
    rbac = current_app.config.get('rbac_service')
    
    if not rbac:
        return None
    
    role = rbac.get_user_role(user_id)
    permissions = rbac.get_user_permissions(user_id)
    
    return {
        'role': role.name if role else 'UNKNOWN',
        'role_level': role.value if role else 0,
        'permissions': [p.value for p in permissions],
        'is_admin': rbac.is_admin(user_id),
        'is_lead_or_higher': rbac.is_lead_or_higher(user_id),
    }
