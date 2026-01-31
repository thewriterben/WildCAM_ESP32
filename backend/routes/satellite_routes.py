"""
Satellite Fleet Management Routes
Enterprise-level satellite communication fleet management and monitoring API endpoints

Author: WildCAM ESP32 Team
Version: 1.0.0
Date: 2026
"""

from flask import Blueprint, request, jsonify
from datetime import datetime, timedelta
import logging
import json

logger = logging.getLogger(__name__)

satellite_routes = Blueprint('satellite', __name__)

# In-memory storage for demo (use database in production)
satellite_devices = {}
satellite_messages = {}
satellite_stats = {
    'total_messages_sent': 0,
    'total_messages_received': 0,
    'total_bytes_transmitted': 0,
    'cost_this_month': 0.0
}


@satellite_routes.route('/api/v1/satellite/fleet', methods=['GET'])
def get_satellite_fleet():
    """
    Get all satellite-enabled devices in the fleet
    
    Returns:
        List of satellite devices with status
    """
    try:
        devices = []
        for device_id, device in satellite_devices.items():
            devices.append({
                'device_id': device_id,
                'device_name': device.get('device_name'),
                'satellite_type': device.get('satellite_type'),  # iridium, swarm, rockblock
                'status': device.get('status', 'unknown'),
                'signal_strength': device.get('signal_strength', 0),
                'last_transmission': device.get('last_transmission'),
                'messages_today': device.get('messages_today', 0),
                'daily_limit': device.get('daily_limit', 50),
                'location': {
                    'latitude': device.get('latitude'),
                    'longitude': device.get('longitude')
                }
            })
        
        return jsonify({
            'status': 'success',
            'fleet': devices,
            'total_devices': len(devices)
        })
        
    except Exception as e:
        logger.error(f"Error getting satellite fleet: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/fleet/<device_id>', methods=['GET'])
def get_satellite_device(device_id):
    """
    Get detailed status for a specific satellite device
    
    Args:
        device_id: Device identifier
        
    Returns:
        Detailed device status
    """
    try:
        if device_id not in satellite_devices:
            return jsonify({
                'status': 'error',
                'message': 'Device not found'
            }), 404
        
        device = satellite_devices[device_id]
        
        return jsonify({
            'status': 'success',
            'device': {
                'device_id': device_id,
                'device_name': device.get('device_name'),
                'satellite_type': device.get('satellite_type'),
                'modem_info': device.get('modem_info', {}),
                'status': device.get('status', 'unknown'),
                'signal_strength': device.get('signal_strength', 0),
                'last_transmission': device.get('last_transmission'),
                'next_pass_prediction': device.get('next_pass_prediction'),
                'messages_today': device.get('messages_today', 0),
                'daily_limit': device.get('daily_limit', 50),
                'cost_today': device.get('cost_today', 0.0),
                'queue_depth': device.get('queue_depth', 0),
                'battery_level': device.get('battery_level'),
                'location': {
                    'latitude': device.get('latitude'),
                    'longitude': device.get('longitude')
                },
                'configuration': device.get('configuration', {})
            }
        })
        
    except Exception as e:
        logger.error(f"Error getting satellite device {device_id}: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/fleet/<device_id>/register', methods=['POST'])
def register_satellite_device(device_id):
    """
    Register a new satellite-enabled device
    
    Request body:
    {
        "device_name": "Remote Camera 1",
        "satellite_type": "iridium",  # iridium, swarm, rockblock
        "modem_imei": "123456789012345",
        "daily_limit": 50,
        "latitude": 45.0,
        "longitude": -75.0
    }
    
    Returns:
        Registration status
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({
                'status': 'error',
                'message': 'No data provided'
            }), 400
        
        satellite_type = data.get('satellite_type')
        if satellite_type not in ['iridium', 'swarm', 'rockblock']:
            return jsonify({
                'status': 'error',
                'message': 'Invalid satellite type. Must be iridium, swarm, or rockblock'
            }), 400
        
        satellite_devices[device_id] = {
            'device_name': data.get('device_name', device_id),
            'satellite_type': satellite_type,
            'modem_info': {
                'imei': data.get('modem_imei'),
                'firmware': data.get('modem_firmware'),
                'hardware': data.get('modem_hardware')
            },
            'status': 'registered',
            'signal_strength': 0,
            'last_transmission': None,
            'next_pass_prediction': None,
            'messages_today': 0,
            'daily_limit': data.get('daily_limit', 50),
            'cost_today': 0.0,
            'queue_depth': 0,
            'latitude': data.get('latitude'),
            'longitude': data.get('longitude'),
            'configuration': {
                'emergency_mode_enabled': True,
                'compression_enabled': True,
                'store_and_forward': True
            },
            'registered_at': datetime.utcnow().isoformat()
        }
        
        logger.info(f"Satellite device {device_id} registered ({satellite_type})")
        
        return jsonify({
            'status': 'success',
            'message': 'Device registered for satellite communication',
            'device_id': device_id
        })
        
    except Exception as e:
        logger.error(f"Error registering satellite device: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/messages', methods=['GET'])
def get_satellite_messages():
    """
    Get satellite message history
    
    Query params:
        device_id: Filter by device
        status: Filter by status (sent, pending, failed)
        limit: Number of messages to return
        
    Returns:
        List of satellite messages
    """
    try:
        device_id = request.args.get('device_id')
        status = request.args.get('status')
        limit = int(request.args.get('limit', 50))
        
        messages = []
        for msg_id, msg in satellite_messages.items():
            if device_id and msg.get('device_id') != device_id:
                continue
            if status and msg.get('status') != status:
                continue
            messages.append({
                'message_id': msg_id,
                **msg
            })
        
        # Sort by timestamp descending
        messages.sort(key=lambda x: x.get('timestamp', ''), reverse=True)
        messages = messages[:limit]
        
        return jsonify({
            'status': 'success',
            'messages': messages,
            'total': len(messages)
        })
        
    except Exception as e:
        logger.error(f"Error getting satellite messages: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/messages', methods=['POST'])
def queue_satellite_message():
    """
    Queue a message for satellite transmission
    
    Request body:
    {
        "device_id": "device123",
        "message_type": "detection",  # detection, status, emergency, telemetry
        "payload": {
            "species": "deer",
            "confidence": 0.95,
            "timestamp": "2026-01-31T12:00:00Z"
        },
        "priority": "normal"  # low, normal, high, emergency
    }
    
    Returns:
        Queued message details
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({
                'status': 'error',
                'message': 'No data provided'
            }), 400
        
        device_id = data.get('device_id')
        if not device_id:
            return jsonify({
                'status': 'error',
                'message': 'Device ID required'
            }), 400
        
        message_id = f"msg_{datetime.utcnow().strftime('%Y%m%d%H%M%S')}_{device_id[:8]}"
        
        message = {
            'device_id': device_id,
            'message_type': data.get('message_type', 'telemetry'),
            'payload': data.get('payload', {}),
            'priority': data.get('priority', 'normal'),
            'status': 'queued',
            'timestamp': datetime.utcnow().isoformat(),
            'size_bytes': len(json.dumps(data.get('payload', {}))),
            'estimated_cost': calculate_message_cost(data.get('payload', {}), device_id)
        }
        
        satellite_messages[message_id] = message
        
        # Update device queue depth
        if device_id in satellite_devices:
            satellite_devices[device_id]['queue_depth'] = \
                satellite_devices[device_id].get('queue_depth', 0) + 1
        
        logger.info(f"Satellite message {message_id} queued for {device_id}")
        
        return jsonify({
            'status': 'success',
            'message': 'Message queued for satellite transmission',
            'message_id': message_id,
            'estimated_cost': message['estimated_cost']
        })
        
    except Exception as e:
        logger.error(f"Error queuing satellite message: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/stats', methods=['GET'])
def get_satellite_stats():
    """
    Get satellite communication statistics
    
    Query params:
        period: Time period (today, week, month)
        device_id: Filter by device
        
    Returns:
        Satellite usage statistics
    """
    try:
        period = request.args.get('period', 'month')
        device_id = request.args.get('device_id')
        
        # Calculate stats
        total_devices = len(satellite_devices)
        online_devices = sum(
            1 for d in satellite_devices.values()
            if d.get('status') in ['online', 'transmitting']
        )
        
        # Get message counts by type
        message_counts = {'detection': 0, 'status': 0, 'emergency': 0, 'telemetry': 0}
        for msg in satellite_messages.values():
            if device_id and msg.get('device_id') != device_id:
                continue
            msg_type = msg.get('message_type', 'telemetry')
            if msg_type in message_counts:
                message_counts[msg_type] += 1
        
        # Calculate costs by provider
        costs_by_provider = {'iridium': 0.0, 'swarm': 0.0, 'rockblock': 0.0}
        for msg in satellite_messages.values():
            if device_id and msg.get('device_id') != device_id:
                continue
            dev_id = msg.get('device_id')
            if dev_id in satellite_devices:
                provider = satellite_devices[dev_id].get('satellite_type', 'iridium')
                costs_by_provider[provider] += msg.get('estimated_cost', 0)
        
        return jsonify({
            'status': 'success',
            'stats': {
                'period': period,
                'fleet': {
                    'total_devices': total_devices,
                    'online_devices': online_devices,
                    'offline_devices': total_devices - online_devices
                },
                'messages': {
                    'total_sent': satellite_stats['total_messages_sent'],
                    'total_received': satellite_stats['total_messages_received'],
                    'by_type': message_counts,
                    'pending': sum(
                        1 for m in satellite_messages.values()
                        if m.get('status') == 'queued'
                    ),
                    'failed': sum(
                        1 for m in satellite_messages.values()
                        if m.get('status') == 'failed'
                    )
                },
                'bandwidth': {
                    'bytes_transmitted': satellite_stats['total_bytes_transmitted'],
                    'average_message_size': calculate_average_message_size()
                },
                'costs': {
                    'total_this_month': satellite_stats['cost_this_month'],
                    'by_provider': costs_by_provider,
                    'projected_monthly': satellite_stats['cost_this_month'] * 1.1
                }
            }
        })
        
    except Exception as e:
        logger.error(f"Error getting satellite stats: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/fleet/<device_id>/config', methods=['POST'])
def update_satellite_config(device_id):
    """
    Update satellite device configuration
    
    Request body:
    {
        "daily_limit": 100,
        "emergency_mode_enabled": true,
        "compression_enabled": true,
        "store_and_forward": true,
        "preferred_network": "swarm"  # iridium, swarm
    }
    
    Returns:
        Updated configuration
    """
    try:
        if device_id not in satellite_devices:
            return jsonify({
                'status': 'error',
                'message': 'Device not found'
            }), 404
        
        data = request.get_json()
        if not data:
            return jsonify({
                'status': 'error',
                'message': 'No data provided'
            }), 400
        
        device = satellite_devices[device_id]
        
        # Update configuration
        if 'daily_limit' in data:
            device['daily_limit'] = data['daily_limit']
        
        if 'configuration' not in device:
            device['configuration'] = {}
        
        for key in ['emergency_mode_enabled', 'compression_enabled', 
                   'store_and_forward', 'preferred_network']:
            if key in data:
                device['configuration'][key] = data[key]
        
        logger.info(f"Satellite config updated for device {device_id}")
        
        return jsonify({
            'status': 'success',
            'message': 'Configuration updated',
            'configuration': device.get('configuration', {})
        })
        
    except Exception as e:
        logger.error(f"Error updating satellite config: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/pass-predictions', methods=['GET'])
def get_pass_predictions():
    """
    Get satellite pass predictions for fleet devices
    
    Query params:
        device_id: Filter by device
        hours: Prediction window (default 24)
        
    Returns:
        List of predicted satellite passes
    """
    try:
        device_id = request.args.get('device_id')
        hours = int(request.args.get('hours', 24))
        
        predictions = []
        now = datetime.utcnow()
        
        for dev_id, device in satellite_devices.items():
            if device_id and dev_id != device_id:
                continue
            
            if not device.get('latitude') or not device.get('longitude'):
                continue
            
            # Generate sample predictions (in production, use actual TLE data)
            sat_type = device.get('satellite_type', 'iridium')
            num_passes = 3 if sat_type == 'iridium' else 8 if sat_type == 'swarm' else 4
            
            for i in range(num_passes):
                pass_time = now + timedelta(hours=i * (hours / num_passes))
                predictions.append({
                    'device_id': dev_id,
                    'satellite_type': sat_type,
                    'pass_start': pass_time.isoformat(),
                    'pass_end': (pass_time + timedelta(minutes=10)).isoformat(),
                    'max_elevation': 45 + (i * 10) % 45,
                    'direction': 'N' if i % 2 == 0 else 'S',
                    'quality': 'good' if (45 + (i * 10) % 45) > 30 else 'fair'
                })
        
        predictions.sort(key=lambda x: x['pass_start'])
        
        return jsonify({
            'status': 'success',
            'predictions': predictions,
            'prediction_window_hours': hours
        })
        
    except Exception as e:
        logger.error(f"Error getting pass predictions: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@satellite_routes.route('/api/v1/satellite/emergency', methods=['POST'])
def trigger_emergency_message():
    """
    Trigger emergency satellite message (bypasses queue and limits)
    
    Request body:
    {
        "device_id": "device123",
        "emergency_type": "poaching",  # poaching, injury, equipment_failure, weather
        "message": "Suspicious activity detected",
        "location": {
            "latitude": 45.0,
            "longitude": -75.0
        }
    }
    
    Returns:
        Emergency transmission status
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({
                'status': 'error',
                'message': 'No data provided'
            }), 400
        
        device_id = data.get('device_id')
        if not device_id:
            return jsonify({
                'status': 'error',
                'message': 'Device ID required'
            }), 400
        
        emergency_type = data.get('emergency_type', 'general')
        
        message_id = f"emg_{datetime.utcnow().strftime('%Y%m%d%H%M%S')}_{device_id[:8]}"
        
        emergency_message = {
            'device_id': device_id,
            'message_type': 'emergency',
            'emergency_type': emergency_type,
            'payload': {
                'message': data.get('message', 'Emergency alert'),
                'location': data.get('location', {}),
                'timestamp': datetime.utcnow().isoformat()
            },
            'priority': 'emergency',
            'status': 'transmitting',
            'timestamp': datetime.utcnow().isoformat()
        }
        
        satellite_messages[message_id] = emergency_message
        
        # In production, this would trigger immediate satellite transmission
        logger.warning(f"EMERGENCY satellite message triggered: {message_id} - {emergency_type}")
        
        # Update global stats
        satellite_stats['total_messages_sent'] += 1
        
        return jsonify({
            'status': 'success',
            'message': 'Emergency message transmitted',
            'message_id': message_id,
            'emergency_type': emergency_type,
            'transmission_status': 'sent'
        })
        
    except Exception as e:
        logger.error(f"Error triggering emergency message: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


def calculate_message_cost(payload, device_id):
    """Calculate estimated cost for satellite message"""
    size_bytes = len(json.dumps(payload))
    
    # Get device satellite type
    sat_type = 'iridium'
    if device_id in satellite_devices:
        sat_type = satellite_devices[device_id].get('satellite_type', 'iridium')
    
    # Cost per message (approximate)
    cost_per_message = {
        'iridium': 0.50,  # ~$0.50 per SBD message
        'swarm': 0.05,    # ~$0.05 per Swarm packet
        'rockblock': 0.50  # Similar to Iridium
    }
    
    base_cost = cost_per_message.get(sat_type, 0.50)
    
    # Add size-based cost for larger messages
    if size_bytes > 100:
        base_cost += (size_bytes - 100) * 0.001
    
    return round(base_cost, 3)


def calculate_average_message_size():
    """Calculate average message size"""
    if not satellite_messages:
        return 0
    
    total_size = sum(m.get('size_bytes', 0) for m in satellite_messages.values())
    return total_size // len(satellite_messages)


def create_satellite_routes(app):
    """Register satellite routes with Flask app"""
    app.register_blueprint(satellite_routes)
    logger.info("Satellite fleet management routes registered")
