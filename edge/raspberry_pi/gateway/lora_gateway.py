"""
LoRa Gateway for Raspberry Pi

Receives LoRa packets from ESP32 mesh network using SX127x/SX1262 radios.
Handles mesh protocol, node tracking, and packet logging.
"""

import time
import struct
from typing import Optional, Dict, Any, List, Callable
from datetime import datetime
from dataclasses import dataclass
import logging
import asyncio

from edge.shared.utils.db_helpers import DatabaseManager
from edge.shared.protocols.esp32_messages import ESP32MessageType

logger = logging.getLogger(__name__)


@dataclass
class LoRaPacket:
    """LoRa packet structure"""
    node_id: int
    packet_type: int
    sequence: int
    payload: bytes
    rssi: int
    snr: float
    timestamp: datetime
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            'node_id': self.node_id,
            'packet_type': self.packet_type,
            'sequence': self.sequence,
            'payload': self.payload.hex(),
            'rssi': self.rssi,
            'snr': self.snr,
            'timestamp': self.timestamp.isoformat()
        }


@dataclass
class MeshNode:
    """Mesh network node information"""
    node_id: int
    last_seen: datetime
    rssi: int
    snr: float
    packet_count: int
    battery_level: int = 0
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            'node_id': self.node_id,
            'last_seen': self.last_seen.isoformat(),
            'rssi': self.rssi,
            'snr': self.snr,
            'packet_count': self.packet_count,
            'battery_level': self.battery_level
        }


class LoRaGateway:
    """
    LoRa Gateway for receiving ESP32 mesh packets
    
    Compatible with SX1276, SX1278, SX1262 LoRa modules.
    Implements the mesh protocol from MeshManager.h
    """
    
    # Packet header format (matches ESP32 MeshManager)
    # Header: version(1) + type(1) + from(4) + to(4) + seq(2) + flags(1)
    HEADER_FORMAT = '<BBIIBB'
    HEADER_SIZE = 13
    
    def __init__(
        self,
        db_path: str,
        frequency: float = 915.0,
        spreading_factor: int = 7,
        bandwidth: int = 125000,
        coding_rate: int = 5,
        tx_power: int = 17,
        use_sx1262: bool = False
    ):
        """
        Initialize LoRa gateway
        
        Args:
            db_path: Database path for storing packets
            frequency: LoRa frequency in MHz (915.0 for US, 868.0 for EU)
            spreading_factor: LoRa spreading factor (7-12)
            bandwidth: LoRa bandwidth in Hz
            coding_rate: LoRa coding rate (5-8)
            tx_power: Transmission power in dBm
            use_sx1262: Use SX1262 module instead of SX127x
        """
        self.db = DatabaseManager(db_path)
        self.frequency = frequency
        self.spreading_factor = spreading_factor
        self.bandwidth = bandwidth
        self.coding_rate = coding_rate
        self.tx_power = tx_power
        self.use_sx1262 = use_sx1262
        
        # LoRa radio (placeholder - requires actual hardware library)
        self.radio = None
        
        # Node tracking
        self.nodes: Dict[int, MeshNode] = {}
        
        # Callbacks
        self.packet_callback: Optional[Callable] = None
        
        # Control
        self.running = False
        self.receive_task: Optional[asyncio.Task] = None
        
    async def initialize(self):
        """Initialize LoRa radio and database"""
        # Create database tables
        await self._create_tables()
        
        # Initialize LoRa radio
        self._init_radio()
        
        logger.info(f"LoRa gateway initialized: {self.frequency} MHz, SF{self.spreading_factor}")
        
    async def _create_tables(self):
        """Create database tables"""
        # Packets table
        packets_schema = """
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id INTEGER NOT NULL,
            packet_type INTEGER NOT NULL,
            sequence INTEGER NOT NULL,
            payload BLOB,
            rssi INTEGER,
            snr REAL,
            timestamp TEXT NOT NULL
        """
        await self.db.create_table("packets", packets_schema)
        
        # Detections table
        detections_schema = """
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id INTEGER NOT NULL,
            species TEXT,
            confidence REAL,
            timestamp TEXT NOT NULL,
            latitude REAL,
            longitude REAL
        """
        await self.db.create_table("detections", detections_schema)
        
        # Telemetry table
        telemetry_schema = """
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            node_id INTEGER NOT NULL,
            battery_level INTEGER,
            temperature REAL,
            humidity REAL,
            pressure REAL,
            rssi INTEGER,
            snr REAL,
            timestamp TEXT NOT NULL
        """
        await self.db.create_table("telemetry", telemetry_schema)
        
    def _init_radio(self):
        """Initialize LoRa radio hardware"""
        # NOTE: This is a placeholder. In production, use:
        # - For SX127x: https://github.com/rpsreal/pySX127x
        # - For SX1262: https://github.com/ehong-tl/micropySX126X
        
        # Example initialization (pseudocode):
        # from SX127x.LoRa import LoRa
        # self.radio = LoRa()
        # self.radio.set_freq(self.frequency)
        # self.radio.set_spreading_factor(self.spreading_factor)
        # self.radio.set_bandwidth(self.bandwidth)
        # self.radio.set_coding_rate(self.coding_rate)
        # self.radio.set_tx_power(self.tx_power)
        
        logger.info("LoRa radio initialized (placeholder)")
        
    def set_packet_callback(self, callback: Callable):
        """Set callback for received packets"""
        self.packet_callback = callback
        
    async def start(self):
        """Start receiving LoRa packets"""
        if self.running:
            logger.warning("LoRa gateway already running")
            return
            
        self.running = True
        logger.info("Starting LoRa gateway")
        
        # Start receive loop
        self.receive_task = asyncio.create_task(self._receive_loop())
        
        # Start beacon broadcast
        asyncio.create_task(self._beacon_loop())
        
    async def stop(self):
        """Stop LoRa gateway"""
        if not self.running:
            return
            
        self.running = False
        logger.info("Stopping LoRa gateway")
        
        if self.receive_task:
            self.receive_task.cancel()
            try:
                await self.receive_task
            except asyncio.CancelledError:
                pass
                
    async def _receive_loop(self):
        """Main receive loop"""
        while self.running:
            try:
                # Receive packet (placeholder)
                # In production: packet_data = self.radio.receive()
                packet_data = await self._mock_receive_packet()
                
                if packet_data:
                    packet = self._parse_packet(packet_data)
                    if packet:
                        await self._process_packet(packet)
                        
            except Exception as e:
                logger.error(f"Receive loop error: {e}")
                
            await asyncio.sleep(0.1)
            
    async def _mock_receive_packet(self) -> Optional[bytes]:
        """Mock packet receive for testing"""
        # Wait a bit to simulate no packets
        await asyncio.sleep(1.0)
        return None
        
    def _parse_packet(self, data: bytes) -> Optional[LoRaPacket]:
        """
        Parse LoRa packet
        
        Args:
            data: Raw packet bytes
            
        Returns:
            Parsed LoRaPacket or None if invalid
        """
        if len(data) < self.HEADER_SIZE:
            logger.warning("Packet too short")
            return None
            
        try:
            # Parse header
            version, packet_type, from_id, to_id, sequence, flags = struct.unpack(
                self.HEADER_FORMAT,
                data[:self.HEADER_SIZE]
            )
            
            # Extract payload
            payload = data[self.HEADER_SIZE:]
            
            # Create packet (RSSI and SNR would come from radio)
            packet = LoRaPacket(
                node_id=from_id,
                packet_type=packet_type,
                sequence=sequence,
                payload=payload,
                rssi=-100,  # Placeholder
                snr=5.0,    # Placeholder
                timestamp=datetime.utcnow()
            )
            
            return packet
            
        except Exception as e:
            logger.error(f"Packet parse error: {e}")
            return None
            
    async def _process_packet(self, packet: LoRaPacket):
        """Process received packet"""
        logger.info(f"Packet from node {packet.node_id}, type: {packet.packet_type}")
        
        # Update node tracking
        if packet.node_id in self.nodes:
            node = self.nodes[packet.node_id]
            node.last_seen = packet.timestamp
            node.rssi = packet.rssi
            node.snr = packet.snr
            node.packet_count += 1
        else:
            self.nodes[packet.node_id] = MeshNode(
                node_id=packet.node_id,
                last_seen=packet.timestamp,
                rssi=packet.rssi,
                snr=packet.snr,
                packet_count=1
            )
            
        # Store packet in database
        await self._store_packet(packet)
        
        # Parse packet type and store appropriately
        if packet.packet_type == ESP32MessageType.WILDLIFE:
            await self._process_wildlife_packet(packet)
        elif packet.packet_type == ESP32MessageType.TELEMETRY:
            await self._process_telemetry_packet(packet)
            
        # Call callback if set
        if self.packet_callback:
            await self.packet_callback(packet)
            
    async def _store_packet(self, packet: LoRaPacket):
        """Store packet in database"""
        query = """
            INSERT INTO packets (node_id, packet_type, sequence, payload, rssi, snr, timestamp)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """
        params = (
            packet.node_id,
            packet.packet_type,
            packet.sequence,
            packet.payload,
            packet.rssi,
            packet.snr,
            packet.timestamp.isoformat()
        )
        await self.db.execute(query, params)
        
    async def _process_wildlife_packet(self, packet: LoRaPacket):
        """Process wildlife detection packet"""
        # Parse detection data from payload
        # Format depends on ESP32 implementation
        # For now, store raw packet
        logger.info(f"Wildlife detection from node {packet.node_id}")
        
    async def _process_telemetry_packet(self, packet: LoRaPacket):
        """Process telemetry packet"""
        # Parse telemetry data from payload
        logger.debug(f"Telemetry from node {packet.node_id}")
        
    async def _beacon_loop(self):
        """Broadcast discovery beacons"""
        while self.running:
            try:
                # Send beacon packet
                # self._send_beacon()
                pass
            except Exception as e:
                logger.error(f"Beacon error: {e}")
                
            await asyncio.sleep(30)  # Every 30 seconds
            
    def get_mesh_stats(self) -> Dict[str, Any]:
        """Get mesh network statistics"""
        return {
            'total_nodes': len(self.nodes),
            'nodes': [node.to_dict() for node in self.nodes.values()]
        }
        
    async def get_recent_detections(self, limit: int = 10) -> List[Dict[str, Any]]:
        """Get recent detections from database"""
        query = "SELECT * FROM detections ORDER BY timestamp DESC LIMIT ?"
        return await self.db.fetchall(query, (limit,))
        
    async def get_recent_telemetry(self, node_id: Optional[int] = None, limit: int = 10) -> List[Dict[str, Any]]:
        """Get recent telemetry from database"""
        if node_id:
            query = "SELECT * FROM telemetry WHERE node_id = ? ORDER BY timestamp DESC LIMIT ?"
            params = (node_id, limit)
        else:
            query = "SELECT * FROM telemetry ORDER BY timestamp DESC LIMIT ?"
            params = (limit,)
        return await self.db.fetchall(query, params)
