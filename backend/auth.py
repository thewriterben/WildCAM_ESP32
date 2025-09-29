"""
JWT Authentication system for ESP32 Wildlife Camera Backend
"""

from flask import jsonify, request, current_app
from flask_jwt_extended import (
    create_access_token, create_refresh_token, get_jwt_identity,
    jwt_required, get_jwt, verify_jwt_in_request
)
from functools import wraps
from datetime import datetime, timedelta
from models import User, db
import re

# Token blacklist for logout functionality
blacklisted_tokens = set()

def validate_email(email):
    """Validate email format"""
    pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    return re.match(pattern, email) is not None

def validate_password(password):
    """Validate password strength"""
    if len(password) < 8:
        return False, "Password must be at least 8 characters long"
    if not re.search(r'[A-Z]', password):
        return False, "Password must contain at least one uppercase letter"
    if not re.search(r'[a-z]', password):
        return False, "Password must contain at least one lowercase letter"
    if not re.search(r'\d', password):
        return False, "Password must contain at least one number"
    return True, "Password is valid"

def register_user(username, email, password, first_name=None, last_name=None, role='user'):
    """Register a new user"""
    try:
        # Validate input
        if not username or len(username) < 3:
            return {'error': 'Username must be at least 3 characters long'}, 400
        
        if not validate_email(email):
            return {'error': 'Invalid email format'}, 400
        
        is_valid_password, password_message = validate_password(password)
        if not is_valid_password:
            return {'error': password_message}, 400
        
        # Check if user already exists
        if User.query.filter_by(username=username).first():
            return {'error': 'Username already exists'}, 409
        
        if User.query.filter_by(email=email).first():
            return {'error': 'Email already exists'}, 409
        
        # Create new user
        user = User(
            username=username,
            email=email,
            first_name=first_name,
            last_name=last_name,
            role=role
        )
        user.set_password(password)
        
        db.session.add(user)
        db.session.commit()
        
        # Create tokens
        access_token = create_access_token(
            identity=user.id,
            additional_claims={'role': user.role, 'username': user.username}
        )
        refresh_token = create_refresh_token(identity=user.id)
        
        return {
            'message': 'User registered successfully',
            'user': user.to_dict(),
            'access_token': access_token,
            'refresh_token': refresh_token
        }, 201
        
    except Exception as e:
        db.session.rollback()
        current_app.logger.error(f"Registration error: {e}")
        return {'error': 'Registration failed'}, 500

def authenticate_user(username, password):
    """Authenticate user login"""
    try:
        # Find user by username or email
        user = User.query.filter(
            (User.username == username) | (User.email == username)
        ).first()
        
        if not user or not user.check_password(password):
            return {'error': 'Invalid credentials'}, 401
        
        if not user.is_active:
            return {'error': 'Account is deactivated'}, 401
        
        # Update last login
        user.last_login = datetime.utcnow()
        db.session.commit()
        
        # Create tokens
        access_token = create_access_token(
            identity=user.id,
            additional_claims={'role': user.role, 'username': user.username}
        )
        refresh_token = create_refresh_token(identity=user.id)
        
        return {
            'message': 'Login successful',
            'user': user.to_dict(),
            'access_token': access_token,
            'refresh_token': refresh_token
        }, 200
        
    except Exception as e:
        current_app.logger.error(f"Authentication error: {e}")
        return {'error': 'Authentication failed'}, 500

def refresh_token():
    """Refresh access token"""
    try:
        current_user_id = get_jwt_identity()
        user = User.query.get(current_user_id)
        
        if not user or not user.is_active:
            return {'error': 'User not found or inactive'}, 401
        
        new_token = create_access_token(
            identity=user.id,
            additional_claims={'role': user.role, 'username': user.username}
        )
        
        return {'access_token': new_token}, 200
        
    except Exception as e:
        current_app.logger.error(f"Token refresh error: {e}")
        return {'error': 'Token refresh failed'}, 500

def logout_user():
    """Logout user and blacklist token"""
    try:
        jti = get_jwt()['jti']
        blacklisted_tokens.add(jti)
        return {'message': 'Successfully logged out'}, 200
    except Exception as e:
        current_app.logger.error(f"Logout error: {e}")
        return {'error': 'Logout failed'}, 500

def check_if_token_revoked(jwt_header, jwt_payload):
    """Check if token is blacklisted"""
    jti = jwt_payload['jti']
    return jti in blacklisted_tokens

def role_required(allowed_roles):
    """Decorator to check user role"""
    def decorator(f):
        @wraps(f)
        @jwt_required()
        def decorated_function(*args, **kwargs):
            try:
                verify_jwt_in_request()
                claims = get_jwt()
                user_role = claims.get('role', 'user')
                
                if user_role not in allowed_roles:
                    return jsonify({'error': 'Insufficient permissions'}), 403
                
                return f(*args, **kwargs)
            except Exception as e:
                current_app.logger.error(f"Role check error: {e}")
                return jsonify({'error': 'Authorization failed'}), 401
        
        return decorated_function
    return decorator

def get_current_user():
    """Get current authenticated user"""
    try:
        user_id = get_jwt_identity()
        return User.query.get(user_id)
    except:
        return None

def admin_required(f):
    """Decorator for admin-only endpoints"""
    return role_required(['admin'])(f)

def researcher_required(f):
    """Decorator for researcher and admin access"""
    return role_required(['admin', 'researcher'])(f)

# Authentication routes
def create_auth_routes(app):
    """Create authentication routes"""
    
    @app.route('/api/auth/register', methods=['POST'])
    def register():
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No data provided'}), 400
        
        result, status_code = register_user(
            username=data.get('username'),
            email=data.get('email'),
            password=data.get('password'),
            first_name=data.get('first_name'),
            last_name=data.get('last_name'),
            role=data.get('role', 'user')
        )
        
        return jsonify(result), status_code
    
    @app.route('/api/auth/login', methods=['POST'])
    def login():
        data = request.get_json()
        
        if not data or not data.get('username') or not data.get('password'):
            return jsonify({'error': 'Username and password required'}), 400
        
        result, status_code = authenticate_user(
            username=data['username'],
            password=data['password']
        )
        
        return jsonify(result), status_code
    
    @app.route('/api/auth/refresh', methods=['POST'])
    @jwt_required(refresh=True)
    def refresh():
        result, status_code = refresh_token()
        return jsonify(result), status_code
    
    @app.route('/api/auth/logout', methods=['POST'])
    @jwt_required()
    def logout():
        result, status_code = logout_user()
        return jsonify(result), status_code
    
    @app.route('/api/auth/me', methods=['GET'])
    @jwt_required()
    def get_current_user_info():
        user = get_current_user()
        if user:
            return jsonify({'user': user.to_dict()}), 200
        return jsonify({'error': 'User not found'}), 404
    
    @app.route('/api/auth/users', methods=['GET'])
    @admin_required
    def list_users():
        try:
            page = request.args.get('page', 1, type=int)
            per_page = request.args.get('per_page', 20, type=int)
            
            users = User.query.paginate(
                page=page, per_page=per_page, error_out=False
            )
            
            return jsonify({
                'users': [user.to_dict() for user in users.items],
                'total': users.total,
                'pages': users.pages,
                'current_page': users.page
            }), 200
            
        except Exception as e:
            current_app.logger.error(f"List users error: {e}")
            return jsonify({'error': 'Failed to fetch users'}), 500