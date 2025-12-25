"""
Compact LoRa Payload Encoder/Decoder
Encodes detection data into compact binary format for LoRa transmission
"""

import struct
from typing import Dict, Tuple, Optional
from datetime import datetime


def encode_lora_payload(
    species_id: int,
    confidence: float,
    bbox: Tuple[float, float, float, float],
    timestamp: Optional[datetime] = None
) -> bytes:
    """
    Encode detection data into compact 10-byte LoRa payload
    
    Payload format (10 bytes):
    | species_id (1) | confidence (1) | bbox_x (2) | bbox_y (2) | bbox_w (2) | bbox_h (2) |
    
    Args:
        species_id: Species ID (0-255)
        confidence: Detection confidence (0.0 - 1.0)
        bbox: Bounding box as (x, y, width, height) normalized to 0.0-1.0
        timestamp: Optional timestamp (not included in basic payload)
        
    Returns:
        10-byte binary payload
    """
    # Validate inputs
    if not 0 <= species_id <= 255:
        raise ValueError(f"species_id must be 0-255, got {species_id}")
    
    if not 0.0 <= confidence <= 1.0:
        raise ValueError(f"confidence must be 0.0-1.0, got {confidence}")
    
    x, y, w, h = bbox
    for val, name in [(x, 'x'), (y, 'y'), (w, 'width'), (h, 'height')]:
        if not 0.0 <= val <= 1.0:
            raise ValueError(f"bbox {name} must be 0.0-1.0, got {val}")
    
    # Convert confidence to uint8 (0-255)
    confidence_byte = int(confidence * 255)
    
    # Convert normalized bbox coordinates to uint16 (0-65535)
    bbox_x = int(x * 65535)
    bbox_y = int(y * 65535)
    bbox_w = int(w * 65535)
    bbox_h = int(h * 65535)
    
    # Pack into binary format
    # Format: B = unsigned char (1 byte), H = unsigned short (2 bytes)
    payload = struct.pack('>BHHHH', 
                         species_id,
                         bbox_x,
                         bbox_y, 
                         bbox_w,
                         bbox_h)
    
    # Add confidence byte
    payload = struct.pack('B', confidence_byte) + payload[1:]
    
    # Recreate with correct format
    payload = struct.pack('>BBHHHH',
                         species_id,
                         confidence_byte,
                         bbox_x,
                         bbox_y,
                         bbox_w,
                         bbox_h)
    
    return payload


def decode_lora_payload(payload: bytes) -> Dict:
    """
    Decode compact LoRa payload back to detection data
    
    Args:
        payload: 10-byte binary payload
        
    Returns:
        Dictionary with decoded detection data
    """
    if len(payload) != 10:
        raise ValueError(f"Payload must be 10 bytes, got {len(payload)}")
    
    # Unpack binary data
    species_id, confidence_byte, bbox_x, bbox_y, bbox_w, bbox_h = struct.unpack(
        '>BBHHHH', payload
    )
    
    # Convert back to original values
    confidence = confidence_byte / 255.0
    
    # Convert uint16 back to normalized coordinates
    x = bbox_x / 65535.0
    y = bbox_y / 65535.0
    w = bbox_w / 65535.0
    h = bbox_h / 65535.0
    
    return {
        'species_id': species_id,
        'confidence': confidence,
        'bbox': (x, y, w, h),
    }


def encode_extended_payload(
    species_id: int,
    confidence: float,
    bbox: Tuple[float, float, float, float],
    timestamp: datetime,
    node_id: int = 0
) -> bytes:
    """
    Encode detection data with timestamp into 16-byte extended payload
    
    Extended format (16 bytes):
    | species_id (1) | confidence (1) | bbox_x (2) | bbox_y (2) | 
    | bbox_w (2) | bbox_h (2) | timestamp (4) | node_id (2) |
    
    Args:
        species_id: Species ID (0-255)
        confidence: Detection confidence (0.0 - 1.0)
        bbox: Bounding box as (x, y, width, height) normalized
        timestamp: Detection timestamp
        node_id: Node identifier (0-65535)
        
    Returns:
        16-byte binary payload
    """
    # Get basic 10-byte payload
    basic_payload = encode_lora_payload(species_id, confidence, bbox)
    
    # Add timestamp (Unix timestamp as uint32)
    timestamp_int = int(timestamp.timestamp())
    
    # Add node_id
    extended = basic_payload + struct.pack('>IH', timestamp_int, node_id)
    
    return extended


def decode_extended_payload(payload: bytes) -> Dict:
    """
    Decode extended 16-byte LoRa payload
    
    Args:
        payload: 16-byte binary payload
        
    Returns:
        Dictionary with decoded detection data including timestamp and node_id
    """
    if len(payload) != 16:
        raise ValueError(f"Extended payload must be 16 bytes, got {len(payload)}")
    
    # Decode basic fields
    basic_data = decode_lora_payload(payload[:10])
    
    # Decode extended fields
    timestamp_int, node_id = struct.unpack('>IH', payload[10:16])
    
    basic_data['timestamp'] = datetime.fromtimestamp(timestamp_int)
    basic_data['node_id'] = node_id
    
    return basic_data


def validate_payload(payload: bytes) -> bool:
    """
    Validate that a payload can be decoded
    
    Args:
        payload: Binary payload
        
    Returns:
        True if payload is valid, False otherwise
    """
    try:
        if len(payload) == 10:
            decode_lora_payload(payload)
            return True
        elif len(payload) == 16:
            decode_extended_payload(payload)
            return True
        else:
            return False
    except Exception:
        return False
