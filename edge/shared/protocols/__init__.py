"""
Shared protocol definitions for edge computing
"""
from .esp32_messages import (
    ESP32MessageType,
    ESP32Message,
    DetectionMessage,
    TelemetryMessage,
    HeartbeatMessage,
    ImageMessage
)

__all__ = [
    'ESP32MessageType',
    'ESP32Message',
    'DetectionMessage',
    'TelemetryMessage',
    'HeartbeatMessage',
    'ImageMessage'
]
