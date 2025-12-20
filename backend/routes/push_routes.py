/**
 * Push Notification Routes
 * API endpoints for mobile push notification management
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

from flask import Blueprint, request, jsonify
from datetime import datetime
import logging
import json

logger = logging.getLogger(__name__)

push_routes = Blueprint('push', __name__)

# In-memory storage for demo (use database in production)
registered_devices = {}
notification_preferences = {}


@push_routes.route('/api/v1/push/register', methods=['POST'])
def register_push_token():
    """
    Register a device for push notifications
    
    Request body:
    {
        "token": "device_push_token",
        "platform": "ios" | "android",
        "device_id": "unique_device_identifier"
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
        
        token = data.get('token')
        platform = data.get('platform')
        device_id = data.get('device_id')
        
        if not token or not platform or not device_id:
            return jsonify({
                'status': 'error',
                'message': 'Missing required fields: token, platform, device_id'
            }), 400
        
        if platform not in ['ios', 'android']:
            return jsonify({
                'status': 'error',
                'message': 'Platform must be "ios" or "android"'
            }), 400
        
        # Store registration
        registered_devices[device_id] = {
            'token': token,
            'platform': platform,
            'registered_at': datetime.utcnow().isoformat(),
            'last_active': datetime.utcnow().isoformat()
        }
        
        logger.info(f"Push token registered for device {device_id} ({platform})")
        
        return jsonify({
            'status': 'success',
            'message': 'Device registered for push notifications',
            'device_id': device_id
        })
        
    except Exception as e:
        logger.error(f"Push registration error: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@push_routes.route('/api/v1/push/unregister', methods=['DELETE'])
def unregister_push_token():
    """
    Unregister a device from push notifications
    
    Returns:
        Unregistration status
    """
    try:
        # Get device_id from query params or auth header
        device_id = request.args.get('device_id')
        
        if not device_id:
            return jsonify({
                'status': 'error',
                'message': 'Device ID required'
            }), 400
        
        if device_id in registered_devices:
            del registered_devices[device_id]
            logger.info(f"Push token unregistered for device {device_id}")
        
        # Remove preferences too
        if device_id in notification_preferences:
            del notification_preferences[device_id]
        
        return jsonify({
            'status': 'success',
            'message': 'Device unregistered from push notifications'
        })
        
    except Exception as e:
        logger.error(f"Push unregistration error: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@push_routes.route('/api/v1/push/preferences', methods=['GET', 'POST'])
def notification_preferences_route():
    """
    Get or update notification preferences
    
    POST Request body:
    {
        "enable_push_notifications": true,
        "wildlife_alerts": true,
        "device_status_alerts": true,
        "low_battery_alerts": true,
        "min_confidence_for_alert": 0.8,
        "alert_species": ["white_tailed_deer", "black_bear"],
        "quiet_hours_start": "22:00",
        "quiet_hours_end": "07:00"
    }
    
    Returns:
        Current or updated preferences
    """
    try:
        # Get device_id from query params or auth header
        device_id = request.args.get('device_id') or request.headers.get('X-Device-ID')
        
        if not device_id:
            return jsonify({
                'status': 'error',
                'message': 'Device ID required'
            }), 400
        
        if request.method == 'GET':
            prefs = notification_preferences.get(device_id, get_default_preferences())
            return jsonify({
                'status': 'success',
                'preferences': prefs
            })
        
        elif request.method == 'POST':
            data = request.get_json()
            
            if not data:
                return jsonify({
                    'status': 'error',
                    'message': 'No preferences data provided'
                }), 400
            
            # Validate and store preferences
            prefs = {
                'enable_push_notifications': data.get('enable_push_notifications', True),
                'wildlife_alerts': data.get('wildlife_alerts', True),
                'device_status_alerts': data.get('device_status_alerts', True),
                'low_battery_alerts': data.get('low_battery_alerts', True),
                'min_confidence_for_alert': data.get('min_confidence_for_alert', 0.8),
                'alert_species': data.get('alert_species', []),
                'quiet_hours_start': data.get('quiet_hours_start'),
                'quiet_hours_end': data.get('quiet_hours_end'),
                'updated_at': datetime.utcnow().isoformat()
            }
            
            notification_preferences[device_id] = prefs
            
            logger.info(f"Updated notification preferences for device {device_id}")
            
            return jsonify({
                'status': 'success',
                'message': 'Preferences updated',
                'preferences': prefs
            })
            
    except Exception as e:
        logger.error(f"Notification preferences error: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@push_routes.route('/api/v1/push/send', methods=['POST'])
def send_push_notification():
    """
    Send a push notification to specific devices or all devices
    Internal API - should be protected
    
    Request body:
    {
        "device_ids": ["device1", "device2"] or null for all,
        "notification": {
            "title": "Wildlife Detected!",
            "body": "White-tailed deer detected at Camera 1",
            "data": {
                "type": "detection",
                "detection_id": "det_123",
                "species": "white_tailed_deer",
                "confidence": 0.95
            }
        }
    }
    
    Returns:
        Send status
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({
                'status': 'error',
                'message': 'No data provided'
            }), 400
        
        device_ids = data.get('device_ids')
        notification = data.get('notification', {})
        
        if not notification:
            return jsonify({
                'status': 'error',
                'message': 'Notification content required'
            }), 400
        
        # Get target devices
        if device_ids:
            targets = {k: v for k, v in registered_devices.items() if k in device_ids}
        else:
            targets = registered_devices
        
        if not targets:
            return jsonify({
                'status': 'warning',
                'message': 'No registered devices found',
                'sent_count': 0
            })
        
        # Filter by preferences
        filtered_targets = filter_by_preferences(targets, notification)
        
        # Send notifications (placeholder - integrate with FCM/APNS in production)
        sent_count = send_to_devices(filtered_targets, notification)
        
        logger.info(f"Sent {sent_count} push notifications")
        
        return jsonify({
            'status': 'success',
            'message': f'Notifications sent to {sent_count} devices',
            'sent_count': sent_count,
            'total_devices': len(targets)
        })
        
    except Exception as e:
        logger.error(f"Push notification send error: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


@push_routes.route('/api/v1/push/stats', methods=['GET'])
def get_push_stats():
    """
    Get push notification statistics
    
    Returns:
        Registration and delivery statistics
    """
    try:
        ios_count = sum(1 for d in registered_devices.values() if d['platform'] == 'ios')
        android_count = sum(1 for d in registered_devices.values() if d['platform'] == 'android')
        
        return jsonify({
            'status': 'success',
            'stats': {
                'total_registered': len(registered_devices),
                'ios_devices': ios_count,
                'android_devices': android_count,
                'active_preferences': len(notification_preferences)
            }
        })
        
    except Exception as e:
        logger.error(f"Push stats error: {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500


def get_default_preferences():
    """Get default notification preferences"""
    return {
        'enable_push_notifications': True,
        'wildlife_alerts': True,
        'device_status_alerts': True,
        'low_battery_alerts': True,
        'min_confidence_for_alert': 0.8,
        'alert_species': [],
        'quiet_hours_start': None,
        'quiet_hours_end': None
    }


def filter_by_preferences(targets, notification):
    """
    Filter target devices based on their notification preferences
    
    Args:
        targets: Dictionary of target devices
        notification: Notification content
        
    Returns:
        Filtered dictionary of target devices
    """
    notification_type = notification.get('data', {}).get('type', 'general')
    confidence = notification.get('data', {}).get('confidence', 1.0)
    species = notification.get('data', {}).get('species')
    
    filtered = {}
    current_time = datetime.utcnow()
    current_hour = current_time.hour
    
    for device_id, device_info in targets.items():
        prefs = notification_preferences.get(device_id, get_default_preferences())
        
        # Check if push is enabled
        if not prefs.get('enable_push_notifications', True):
            continue
        
        # Check quiet hours
        quiet_start = prefs.get('quiet_hours_start')
        quiet_end = prefs.get('quiet_hours_end')
        if quiet_start and quiet_end:
            start_hour = int(quiet_start.split(':')[0])
            end_hour = int(quiet_end.split(':')[0])
            
            if start_hour <= end_hour:
                if start_hour <= current_hour < end_hour:
                    continue
            else:
                if current_hour >= start_hour or current_hour < end_hour:
                    continue
        
        # Check notification type preferences
        if notification_type == 'detection':
            if not prefs.get('wildlife_alerts', True):
                continue
            
            # Check confidence threshold
            min_confidence = prefs.get('min_confidence_for_alert', 0.8)
            if confidence < min_confidence:
                continue
            
            # Check species filter
            alert_species = prefs.get('alert_species', [])
            if alert_species and species and species not in alert_species:
                continue
                
        elif notification_type == 'device_status':
            if not prefs.get('device_status_alerts', True):
                continue
                
        elif notification_type == 'low_battery':
            if not prefs.get('low_battery_alerts', True):
                continue
        
        filtered[device_id] = device_info
    
    return filtered


def send_to_devices(targets, notification):
    """
    Send push notification to devices
    
    In production, this would integrate with:
    - Firebase Cloud Messaging (FCM) for Android
    - Apple Push Notification Service (APNS) for iOS
    
    Args:
        targets: Dictionary of target devices
        notification: Notification content
        
    Returns:
        Number of notifications sent
    """
    sent_count = 0
    
    for device_id, device_info in targets.items():
        platform = device_info['platform']
        token = device_info['token']
        
        # Placeholder - In production, call FCM/APNS APIs
        try:
            if platform == 'ios':
                # send_apns(token, notification)
                pass
            else:
                # send_fcm(token, notification)
                pass
            
            sent_count += 1
            logger.debug(f"Notification sent to {device_id} ({platform})")
            
        except Exception as e:
            logger.error(f"Failed to send to {device_id}: {e}")
    
    return sent_count


def send_wildlife_alert(detection):
    """
    Convenience function to send wildlife detection alerts
    
    Args:
        detection: Detection data dictionary
    """
    notification = {
        'title': f"{detection.get('species', 'Wildlife')} Detected!",
        'body': f"Confidence: {int(detection.get('confidence', 0) * 100)}% at {detection.get('camera_id', 'Camera')}",
        'data': {
            'type': 'detection',
            'detection_id': detection.get('id'),
            'species': detection.get('species'),
            'confidence': detection.get('confidence'),
            'camera_id': detection.get('camera_id'),
            'image_url': detection.get('image_url')
        }
    }
    
    # This would be called internally after a detection is processed
    # send_push_notification would be invoked with this notification


def send_device_status_alert(device_id, status, message=None):
    """
    Convenience function to send device status alerts
    
    Args:
        device_id: Device identifier
        status: Status string (online, offline, error)
        message: Optional custom message
    """
    status_messages = {
        'online': 'Camera is now online',
        'offline': 'Camera is now offline',
        'error': 'Camera reported an error'
    }
    
    notification = {
        'title': f'Device Status: {status.capitalize()}',
        'body': message or status_messages.get(status, f'Status changed for {device_id}'),
        'data': {
            'type': 'device_status',
            'device_id': device_id,
            'status': status
        }
    }
    
    # This would be called internally when device status changes


def send_low_battery_alert(device_id, battery_level):
    """
    Convenience function to send low battery alerts
    
    Args:
        device_id: Device identifier
        battery_level: Battery level percentage
    """
    notification = {
        'title': 'Low Battery Warning',
        'body': f'Camera {device_id} battery at {battery_level}%',
        'data': {
            'type': 'low_battery',
            'device_id': device_id,
            'battery_level': battery_level
        }
    }
    
    # This would be called internally when low battery is detected
