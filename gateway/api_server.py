"""
Flask-based REST API Server for WildCAM Gateway
Provides endpoints for querying node status, detections, telemetry, and logs
"""

import logging
import json
from datetime import datetime
from typing import Optional, Dict, Any
from functools import wraps

try:
    from flask import Flask, request, jsonify, Response
    from flask_cors import CORS
    FLASK_AVAILABLE = True
except ImportError:
    FLASK_AVAILABLE = False
    # Define Response as a dummy type for type hints
    Response = Any
    logging.warning("Flask not available. Install with: pip install flask flask-cors")

from config import APIConfig
from database import GatewayDatabase


logger = logging.getLogger(__name__)


def require_api_key(f):
    """Decorator to require API key authentication"""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        api_server = args[0] if args else None
        if api_server and api_server.config.api_key:
            # Check API key in header or query parameter
            provided_key = request.headers.get('X-API-Key') or request.args.get('api_key')
            if not provided_key or provided_key != api_server.config.api_key:
                return jsonify({'error': 'Invalid or missing API key'}), 401
        return f(*args, **kwargs)
    return decorated_function


class APIServer:
    """
    REST API server for gateway data access
    """
    
    def __init__(self, config: APIConfig, database: GatewayDatabase):
        """
        Initialize API server
        
        Args:
            config: API configuration
            database: Database instance
        """
        self.config = config
        self.db = database
        self.app: Optional[Flask] = None
        self.server_thread = None
        
        if not FLASK_AVAILABLE:
            logger.error("Flask not available. Cannot initialize API server.")
            return
        
        # Create Flask app
        self.app = Flask(__name__)
        
        # Enable CORS if configured
        if self.config.cors_enabled:
            CORS(self.app)
        
        # Register routes
        self._register_routes()
        
        logger.info("API server initialized")
    
    def _register_routes(self):
        """Register all API routes"""
        if not self.app:
            return
        
        # Health check endpoints
        self.app.add_url_rule('/api/health', 'health', self._health_check, methods=['GET'])
        self.app.add_url_rule('/api/status', 'status', self._gateway_status, methods=['GET'])
        
        # Node endpoints
        self.app.add_url_rule('/api/nodes', 'list_nodes', self._list_nodes, methods=['GET'])
        self.app.add_url_rule('/api/nodes/<node_id>', 'get_node', self._get_node, methods=['GET'])
        self.app.add_url_rule('/api/nodes/<node_id>/telemetry', 'node_telemetry', 
                             self._get_node_telemetry, methods=['GET'])
        self.app.add_url_rule('/api/nodes/<node_id>/detections', 'node_detections', 
                             self._get_node_detections, methods=['GET'])
        
        # Detection endpoints
        self.app.add_url_rule('/api/detections', 'list_detections', 
                             self._list_detections, methods=['GET'])
        self.app.add_url_rule('/api/detections/<int:detection_id>', 'get_detection', 
                             self._get_detection, methods=['GET'])
        self.app.add_url_rule('/api/detections/batch', 'batch_detections', 
                             self._batch_detections, methods=['GET'])
        
        # Telemetry endpoints
        self.app.add_url_rule('/api/telemetry', 'list_telemetry', 
                             self._list_telemetry, methods=['GET'])
        self.app.add_url_rule('/api/telemetry/batch', 'batch_telemetry', 
                             self._batch_telemetry, methods=['GET'])
        
        # Statistics endpoints
        self.app.add_url_rule('/api/stats', 'get_stats', self._get_stats, methods=['GET'])
        self.app.add_url_rule('/api/stats/summary', 'get_summary', 
                             self._get_summary, methods=['GET'])
        
        logger.info("API routes registered")
    
    def run(self, threaded: bool = True):
        """
        Run the API server
        
        Args:
            threaded: Run in threaded mode for concurrent requests
        """
        if not self.app or not self.config.enabled:
            logger.warning("API server is disabled or Flask not available")
            return
        
        logger.info(f"Starting API server on {self.config.host}:{self.config.port}")
        
        try:
            self.app.run(
                host=self.config.host,
                port=self.config.port,
                debug=self.config.debug,
                threaded=threaded,
                use_reloader=False  # Disable reloader to prevent issues with threads
            )
        except Exception as e:
            logger.error(f"Error running API server: {e}")
    
    # Health and status endpoints
    
    def _health_check(self) -> Response:
        """Health check endpoint"""
        return jsonify({
            'status': 'healthy',
            'timestamp': datetime.now().isoformat(),
            'service': 'wildcam-gateway'
        })
    
    @require_api_key
    def _gateway_status(self) -> Response:
        """Get gateway status"""
        try:
            stats = self.db.get_stats()
            return jsonify({
                'status': 'online',
                'timestamp': datetime.now().isoformat(),
                'database': stats
            })
        except Exception as e:
            logger.error(f"Error getting gateway status: {e}")
            return jsonify({'error': str(e)}), 500
    
    # Node endpoints
    
    @require_api_key
    def _list_nodes(self) -> Response:
        """List all nodes"""
        try:
            active_only = request.args.get('active_only', 'false').lower() == 'true'
            nodes = self.db.get_all_nodes(active_only=active_only)
            
            return jsonify({
                'nodes': nodes,
                'count': len(nodes),
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error listing nodes: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _get_node(self, node_id: str) -> Response:
        """Get specific node details"""
        try:
            node = self.db.get_node(node_id)
            
            if not node:
                return jsonify({'error': 'Node not found'}), 404
            
            return jsonify({
                'node': node,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error getting node {node_id}: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _get_node_telemetry(self, node_id: str) -> Response:
        """Get telemetry data for a specific node"""
        try:
            limit = int(request.args.get('limit', 100))
            offset = int(request.args.get('offset', 0))
            
            # Validate limits
            limit = min(max(1, limit), 1000)
            offset = max(0, offset)
            
            telemetry = self.db.get_telemetry(node_id=node_id, limit=limit, offset=offset)
            
            return jsonify({
                'node_id': node_id,
                'telemetry': telemetry,
                'count': len(telemetry),
                'limit': limit,
                'offset': offset,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error getting telemetry for node {node_id}: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _get_node_detections(self, node_id: str) -> Response:
        """Get detections for a specific node"""
        try:
            limit = int(request.args.get('limit', 100))
            offset = int(request.args.get('offset', 0))
            
            # Validate limits
            limit = min(max(1, limit), 1000)
            offset = max(0, offset)
            
            detections = self.db.get_detections(node_id=node_id, limit=limit, offset=offset)
            
            return jsonify({
                'node_id': node_id,
                'detections': detections,
                'count': len(detections),
                'limit': limit,
                'offset': offset,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error getting detections for node {node_id}: {e}")
            return jsonify({'error': str(e)}), 500
    
    # Detection endpoints
    
    @require_api_key
    def _list_detections(self) -> Response:
        """List all detections"""
        try:
            limit = int(request.args.get('limit', 100))
            offset = int(request.args.get('offset', 0))
            node_id = request.args.get('node_id')
            unsynced_only = request.args.get('unsynced_only', 'false').lower() == 'true'
            
            # Validate limits
            limit = min(max(1, limit), 1000)
            offset = max(0, offset)
            
            detections = self.db.get_detections(
                node_id=node_id,
                limit=limit,
                offset=offset,
                unsynced_only=unsynced_only
            )
            
            return jsonify({
                'detections': detections,
                'count': len(detections),
                'limit': limit,
                'offset': offset,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error listing detections: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _get_detection(self, detection_id: int) -> Response:
        """Get specific detection details"""
        try:
            detections = self.db.get_detections(limit=1)
            # Find detection by ID (simplified approach)
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                cursor.execute("SELECT * FROM detections WHERE id = ?", (detection_id,))
                row = cursor.fetchone()
                
                if not row:
                    return jsonify({'error': 'Detection not found'}), 404
                
                detection = dict(row)
                
                return jsonify({
                    'detection': detection,
                    'timestamp': datetime.now().isoformat()
                })
        except Exception as e:
            logger.error(f"Error getting detection {detection_id}: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _batch_detections(self) -> Response:
        """Get batch of detections for synchronization"""
        try:
            batch_size = int(request.args.get('batch_size', 100))
            batch_size = min(max(1, batch_size), 1000)
            
            detections = self.db.get_detections(
                limit=batch_size,
                unsynced_only=True
            )
            
            return jsonify({
                'detections': detections,
                'count': len(detections),
                'batch_size': batch_size,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error getting batch detections: {e}")
            return jsonify({'error': str(e)}), 500
    
    # Telemetry endpoints
    
    @require_api_key
    def _list_telemetry(self) -> Response:
        """List telemetry data"""
        try:
            limit = int(request.args.get('limit', 100))
            offset = int(request.args.get('offset', 0))
            node_id = request.args.get('node_id')
            
            # Validate limits
            limit = min(max(1, limit), 1000)
            offset = max(0, offset)
            
            telemetry = self.db.get_telemetry(
                node_id=node_id,
                limit=limit,
                offset=offset
            )
            
            return jsonify({
                'telemetry': telemetry,
                'count': len(telemetry),
                'limit': limit,
                'offset': offset,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error listing telemetry: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _batch_telemetry(self) -> Response:
        """Get batch of telemetry data"""
        try:
            batch_size = int(request.args.get('batch_size', 100))
            batch_size = min(max(1, batch_size), 1000)
            
            telemetry = self.db.get_telemetry(limit=batch_size)
            
            return jsonify({
                'telemetry': telemetry,
                'count': len(telemetry),
                'batch_size': batch_size,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error getting batch telemetry: {e}")
            return jsonify({'error': str(e)}), 500
    
    # Statistics endpoints
    
    @require_api_key
    def _get_stats(self) -> Response:
        """Get database statistics"""
        try:
            stats = self.db.get_stats()
            
            return jsonify({
                'stats': stats,
                'timestamp': datetime.now().isoformat()
            })
        except Exception as e:
            logger.error(f"Error getting stats: {e}")
            return jsonify({'error': str(e)}), 500
    
    @require_api_key
    def _get_summary(self) -> Response:
        """Get comprehensive gateway summary"""
        try:
            stats = self.db.get_stats()
            
            # Get recent activity
            recent_detections = self.db.get_detections(limit=10)
            active_nodes = self.db.get_all_nodes(active_only=True)
            
            # Calculate additional metrics
            with self.db.get_connection() as conn:
                cursor = conn.cursor()
                
                # Detections in last 24 hours
                cursor.execute("""
                    SELECT COUNT(*) as count 
                    FROM detections 
                    WHERE timestamp >= datetime('now', '-24 hours')
                """)
                detections_24h = cursor.fetchone()['count']
                
                # Most active node
                cursor.execute("""
                    SELECT node_id, COUNT(*) as count 
                    FROM detections 
                    GROUP BY node_id 
                    ORDER BY count DESC 
                    LIMIT 1
                """)
                most_active = cursor.fetchone()
                
                # Species breakdown
                cursor.execute("""
                    SELECT species, COUNT(*) as count 
                    FROM detections 
                    WHERE species IS NOT NULL 
                    GROUP BY species 
                    ORDER BY count DESC 
                    LIMIT 10
                """)
                species_breakdown = [dict(row) for row in cursor.fetchall()]
            
            summary = {
                'database': stats,
                'active_nodes': len(active_nodes),
                'detections_24h': detections_24h,
                'recent_detections': recent_detections[:5],
                'species_breakdown': species_breakdown,
                'timestamp': datetime.now().isoformat()
            }
            
            if most_active:
                summary['most_active_node'] = {
                    'node_id': most_active['node_id'],
                    'detection_count': most_active['count']
                }
            
            return jsonify(summary)
        except Exception as e:
            logger.error(f"Error getting summary: {e}")
            return jsonify({'error': str(e)}), 500


def create_api_server(config: APIConfig, database: GatewayDatabase) -> Optional[APIServer]:
    """
    Factory function to create API server
    
    Args:
        config: API configuration
        database: Database instance
        
    Returns:
        APIServer instance or None if Flask not available
    """
    if not FLASK_AVAILABLE:
        logger.error("Flask not available. Cannot create API server.")
        return None
    
    return APIServer(config, database)
