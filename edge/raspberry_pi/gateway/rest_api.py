"""
REST API for Raspberry Pi LoRa Gateway

Provides HTTP endpoints for monitoring and management.
"""

from flask import Flask, jsonify, request
from typing import Optional
import logging

logger = logging.getLogger(__name__)


def create_api_app(lora_gateway, mqtt_bridge) -> Flask:
    """
    Create Flask API application
    
    Args:
        lora_gateway: LoRaGateway instance
        mqtt_bridge: MQTTBridge instance
        
    Returns:
        Flask application
    """
    app = Flask(__name__)
    
    @app.route('/health', methods=['GET'])
    def health():
        """Health check endpoint"""
        return jsonify({
            'status': 'healthy',
            'lora_gateway': lora_gateway.running,
            'mqtt_bridge': mqtt_bridge.running
        })
        
    @app.route('/api/nodes', methods=['GET'])
    def get_nodes():
        """Get all mesh nodes"""
        stats = lora_gateway.get_mesh_stats()
        return jsonify(stats)
        
    @app.route('/api/nodes/<int:node_id>', methods=['GET'])
    def get_node(node_id: int):
        """Get specific node info"""
        if node_id in lora_gateway.nodes:
            return jsonify(lora_gateway.nodes[node_id].to_dict())
        else:
            return jsonify({'error': 'Node not found'}), 404
            
    @app.route('/api/detections', methods=['GET'])
    async def get_detections():
        """Get recent detections"""
        limit = request.args.get('limit', 10, type=int)
        detections = await lora_gateway.get_recent_detections(limit)
        return jsonify({'detections': detections})
        
    @app.route('/api/telemetry', methods=['GET'])
    async def get_telemetry():
        """Get recent telemetry"""
        node_id = request.args.get('node_id', type=int)
        limit = request.args.get('limit', 10, type=int)
        telemetry = await lora_gateway.get_recent_telemetry(node_id, limit)
        return jsonify({'telemetry': telemetry})
        
    @app.route('/api/stats', methods=['GET'])
    def get_stats():
        """Get gateway statistics"""
        return jsonify({
            'mesh': lora_gateway.get_mesh_stats(),
            'mqtt': mqtt_bridge.get_statistics()
        })
        
    @app.route('/api/config', methods=['GET'])
    def get_config():
        """Get gateway configuration"""
        return jsonify({
            'lora': {
                'frequency': lora_gateway.frequency,
                'spreading_factor': lora_gateway.spreading_factor,
                'bandwidth': lora_gateway.bandwidth,
                'coding_rate': lora_gateway.coding_rate,
                'tx_power': lora_gateway.tx_power
            },
            'mqtt': {
                'broker': mqtt_bridge.broker_host,
                'port': mqtt_bridge.broker_port,
                'connected': mqtt_bridge.stats['connected']
            }
        })
        
    @app.route('/api/config', methods=['PUT'])
    def update_config():
        """Update gateway configuration"""
        # In production, implement config updates
        return jsonify({'error': 'Not implemented'}), 501
        
    return app


def run_api_server(app: Flask, host: str = '0.0.0.0', port: int = 5000):
    """
    Run Flask API server
    
    Args:
        app: Flask application
        host: Server host
        port: Server port
    """
    logger.info(f"Starting REST API server on {host}:{port}")
    app.run(host=host, port=port, debug=False)
