"""
ESP32 Message Protocol Definitions

Compatible with ESP32 MeshManager packet types and structures.
Matches the packet format defined in MeshManager.h
"""

from enum import IntEnum
from dataclasses import dataclass, field
from typing import Optional, Dict, Any
from datetime import datetime
import json


class ESP32MessageType(IntEnum):
    """Message types matching MeshPacketType from MeshManager.h"""
    BEACON = 0x01
    DATA = 0x02
    ACK = 0x03
    ROUTING = 0x04
    WILDLIFE = 0x05
    IMAGE = 0x06
    TELEMETRY = 0x07
    EMERGENCY = 0x08


@dataclass
class ESP32Message:
    """Base message structure for ESP32 communication"""
    node_id: int
    message_type: ESP32MessageType
    timestamp: datetime = field(default_factory=datetime.utcnow)
    sequence: int = 0
    payload: Dict[str, Any] = field(default_factory=dict)
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert message to dictionary"""
        return {
            'node_id': self.node_id,
            'message_type': self.message_type.value,
            'timestamp': self.timestamp.isoformat(),
            'sequence': self.sequence,
            'payload': self.payload
        }
    
    def to_json(self) -> str:
        """Convert message to JSON string"""
        return json.dumps(self.to_dict())
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'ESP32Message':
        """Create message from dictionary"""
        return cls(
            node_id=data['node_id'],
            message_type=ESP32MessageType(data['message_type']),
            timestamp=datetime.fromisoformat(data.get('timestamp', datetime.utcnow().isoformat())),
            sequence=data.get('sequence', 0),
            payload=data.get('payload', {})
        )


@dataclass
class DetectionMessage(ESP32Message):
    """Wildlife detection event message"""
    
    def __init__(self, node_id: int, species: str, confidence: float,
                 latitude: float = 0.0, longitude: float = 0.0,
                 image_size: int = 0, has_image: bool = False, **kwargs):
        super().__init__(
            node_id=node_id,
            message_type=ESP32MessageType.WILDLIFE,
            **kwargs
        )
        self.payload = {
            'species': species,
            'confidence': confidence,
            'latitude': latitude,
            'longitude': longitude,
            'image_size': image_size,
            'has_image': has_image
        }


@dataclass
class TelemetryMessage(ESP32Message):
    """Environmental telemetry message"""
    
    def __init__(self, node_id: int, battery_level: int,
                 temperature: Optional[float] = None,
                 humidity: Optional[float] = None,
                 pressure: Optional[float] = None,
                 rssi: Optional[int] = None,
                 snr: Optional[float] = None,
                 **kwargs):
        super().__init__(
            node_id=node_id,
            message_type=ESP32MessageType.TELEMETRY,
            **kwargs
        )
        self.payload = {
            'battery_level': battery_level,
            'temperature': temperature,
            'humidity': humidity,
            'pressure': pressure,
            'rssi': rssi,
            'snr': snr
        }


@dataclass
class HeartbeatMessage(ESP32Message):
    """Node heartbeat/status message"""
    
    def __init__(self, node_id: int, role: str, battery_level: int,
                 uptime: int, firmware_version: str = "3.1.0", **kwargs):
        super().__init__(
            node_id=node_id,
            message_type=ESP32MessageType.BEACON,
            **kwargs
        )
        self.payload = {
            'role': role,
            'battery_level': battery_level,
            'uptime': uptime,
            'firmware_version': firmware_version
        }


@dataclass
class ImageMessage(ESP32Message):
    """Image data chunk message"""
    
    def __init__(self, node_id: int, image_id: str, chunk_index: int,
                 total_chunks: int, data: bytes, **kwargs):
        super().__init__(
            node_id=node_id,
            message_type=ESP32MessageType.IMAGE,
            **kwargs
        )
        self.payload = {
            'image_id': image_id,
            'chunk_index': chunk_index,
            'total_chunks': total_chunks,
            'data_size': len(data)
        }
        self._data = data
    
    @property
    def data(self) -> bytes:
        return self._data
