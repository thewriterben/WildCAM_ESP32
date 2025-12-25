"""
LoRa Gateway Service - Receives and processes packets from ESP32 nodes
Compatible with SX127x and SX1262 LoRa modules on Raspberry Pi
"""

import logging
import time
import struct
import json
from typing import Optional, Callable, Dict, Any
from threading import Thread, Event
from datetime import datetime

try:
    from SX127x.LoRa import LoRa
    from SX127x.board_config import BOARD
    LORA_AVAILABLE = True
except ImportError:
    LORA_AVAILABLE = False
    logging.warning("SX127x library not available. Install with: pip install RPi.LoRa")

from config import LoRaConfig
from database import GatewayDatabase


logger = logging.getLogger(__name__)


# Packet type constants (matching ESP32 MeshManager.h)
PACKET_BEACON = 0x01
PACKET_DATA = 0x02
PACKET_ACK = 0x03
PACKET_ROUTING = 0x04
PACKET_WILDLIFE = 0x05
PACKET_IMAGE = 0x06
PACKET_TELEMETRY = 0x07
PACKET_EMERGENCY = 0x08


class LoRaGateway:
    """
    LoRa mesh gateway for receiving ESP32 wildlife camera data
    """
    
    def __init__(self, config: LoRaConfig, database: GatewayDatabase):
        """
        Initialize LoRa gateway
        
        Args:
            config: LoRa configuration
            database: Database instance for storing data
        """
        self.config = config
        self.db = database
        self.lora = None
        self.running = False
        self.stop_event = Event()
        self.receiver_thread = None
        
        # Statistics
        self.packets_received = 0
        self.packets_processed = 0
        self.packets_errors = 0
        self.start_time = None
        
        # Callbacks
        self.on_detection_callback: Optional[Callable] = None
        self.on_telemetry_callback: Optional[Callable] = None
        self.on_beacon_callback: Optional[Callable] = None
    
    def initialize(self) -> bool:
        """
        Initialize LoRa radio
        
        Returns:
            True if initialization successful
        """
        if not LORA_AVAILABLE:
            logger.error("LoRa library not available. Cannot initialize.")
            return False
        
        try:
            # Initialize GPIO
            BOARD.setup()
            
            # Create LoRa instance
            self.lora = LoRa()
            
            # Configure radio
            self.lora.set_mode(LoRa.MODE_SLEEP)
            self.lora.set_freq(self.config.frequency)
            self.lora.set_bw(self._get_bandwidth_enum(self.config.bandwidth))
            self.lora.set_spreading_factor(self.config.spreading_factor)
            self.lora.set_coding_rate(self.config.coding_rate)
            self.lora.set_sync_word(self.config.sync_word)
            self.lora.set_preamble(self.config.preamble_length)
            
            if self.config.enable_crc:
                self.lora.set_rx_crc(True)
            
            # Set to receive mode
            self.lora.set_mode(LoRa.MODE_RXCONT)
            
            logger.info(f"LoRa gateway initialized:")
            logger.info(f"  Frequency: {self.config.frequency} MHz")
            logger.info(f"  Bandwidth: {self.config.bandwidth} Hz")
            logger.info(f"  Spreading Factor: {self.config.spreading_factor}")
            logger.info(f"  Coding Rate: 4/{self.config.coding_rate}")
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize LoRa gateway: {e}")
            return False
    
    def _get_bandwidth_enum(self, bandwidth: int):
        """Convert bandwidth Hz to enum value"""
        if not LORA_AVAILABLE:
            return None
            
        bw_map = {
            7800: LoRa.BW_7_8KHZ,
            10400: LoRa.BW_10_4KHZ,
            15600: LoRa.BW_15_6KHZ,
            20800: LoRa.BW_20_8KHZ,
            31250: LoRa.BW_31_25KHZ,
            41700: LoRa.BW_41_7KHZ,
            62500: LoRa.BW_62_5KHZ,
            125000: LoRa.BW_125KHZ,
            250000: LoRa.BW_250KHZ,
            500000: LoRa.BW_500KHZ,
        }
        return bw_map.get(bandwidth, LoRa.BW_125KHZ)
    
    def start(self):
        """Start receiving packets"""
        if not self.lora:
            logger.error("LoRa not initialized. Call initialize() first.")
            return
        
        if self.running:
            logger.warning("Gateway already running")
            return
        
        self.running = True
        self.start_time = datetime.now()
        self.stop_event.clear()
        
        # Start receiver thread
        self.receiver_thread = Thread(target=self._receive_loop, daemon=True)
        self.receiver_thread.start()
        
        logger.info("LoRa gateway started")
    
    def stop(self):
        """Stop receiving packets"""
        if not self.running:
            return
        
        self.running = False
        self.stop_event.set()
        
        if self.receiver_thread:
            self.receiver_thread.join(timeout=5)
        
        if self.lora:
            self.lora.set_mode(LoRa.MODE_SLEEP)
        
        logger.info("LoRa gateway stopped")
    
    def _receive_loop(self):
        """Main receive loop"""
        logger.info("Receiver thread started")
        
        while self.running and not self.stop_event.is_set():
            try:
                # Check for incoming packet
                if self.lora.rx_done():
                    payload = self.lora.read_payload(nocheck=True)
                    rssi = self.lora.get_pkt_rssi_value()
                    snr = self.lora.get_pkt_snr_value()
                    
                    self.packets_received += 1
                    
                    # Process packet
                    self._process_packet(payload, rssi, snr)
                    
                    # Clear IRQ
                    self.lora.clear_irq_flags()
                
                # Small delay to prevent CPU spinning
                time.sleep(0.01)
                
            except Exception as e:
                logger.error(f"Error in receive loop: {e}")
                self.packets_errors += 1
                time.sleep(1)
        
        logger.info("Receiver thread stopped")
    
    def _process_packet(self, payload: bytes, rssi: int, snr: float):
        """
        Process received packet
        
        Args:
            payload: Raw packet payload
            rssi: Received Signal Strength Indicator
            snr: Signal-to-Noise Ratio
        """
        try:
            if len(payload) < 10:  # Minimum packet size
                logger.debug(f"Packet too small: {len(payload)} bytes")
                return
            
            # Parse packet header (compatible with ESP32 mesh format)
            # Format: [packet_type(1)][source_id(4)][dest_id(4)][seq(2)][data...]
            packet_type = payload[0]
            source_id = struct.unpack('>I', payload[1:5])[0]
            dest_id = struct.unpack('>I', payload[5:9])[0]
            
            # Convert node IDs to hex strings
            source_str = f"0x{source_id:08X}"
            dest_str = f"0x{dest_id:08X}"
            
            logger.debug(f"Packet from {source_str}: type={packet_type:02X}, "
                        f"RSSI={rssi}dBm, SNR={snr}dB")
            
            # Log packet to database
            self.db.add_packet(
                source_id=source_str,
                destination_id=dest_str,
                packet_type=f"0x{packet_type:02X}",
                rssi=rssi,
                snr=snr,
                payload=payload.hex(),
                payload_size=len(payload)
            )
            
            # Update node info
            self.db.add_or_update_node(
                node_id=source_str,
                last_rssi=rssi,
                last_snr=snr
            )
            
            # Process based on packet type
            if packet_type == PACKET_BEACON:
                self._process_beacon(source_str, payload[9:], rssi, snr)
            elif packet_type == PACKET_WILDLIFE:
                self._process_wildlife_event(source_str, payload[9:], rssi, snr)
            elif packet_type == PACKET_TELEMETRY:
                self._process_telemetry(source_str, payload[9:], rssi, snr)
            elif packet_type == PACKET_DATA:
                self._process_data(source_str, payload[9:], rssi, snr)
            elif packet_type == PACKET_IMAGE:
                self._process_image_chunk(source_str, payload[9:], rssi, snr)
            
            self.packets_processed += 1
            
        except Exception as e:
            logger.error(f"Error processing packet: {e}")
            self.packets_errors += 1
    
    def _process_beacon(self, node_id: str, data: bytes, rssi: int, snr: float):
        """Process beacon packet"""
        try:
            # Parse beacon data (example format)
            # [battery_level(1)][lat(4)][lon(4)][name(8)]
            if len(data) >= 17:
                battery = data[0]
                lat = struct.unpack('>f', data[1:5])[0]
                lon = struct.unpack('>f', data[5:9])[0]
                name = data[9:17].decode('utf-8', errors='ignore').strip('\x00')
                
                self.db.add_or_update_node(
                    node_id=node_id,
                    node_name=name if name else None,
                    latitude=lat if abs(lat) > 0.001 else None,
                    longitude=lon if abs(lon) > 0.001 else None,
                    battery_level=battery,
                    last_rssi=rssi,
                    last_snr=snr
                )
                
                if self.on_beacon_callback:
                    self.on_beacon_callback(node_id, battery, lat, lon, name)
                    
                logger.info(f"Beacon from {node_id}: {name}, battery={battery}%, "
                           f"RSSI={rssi}dBm")
        except Exception as e:
            logger.error(f"Error processing beacon: {e}")
    
    def _process_wildlife_event(self, node_id: str, data: bytes, rssi: int, snr: float):
        """Process wildlife detection event"""
        try:
            # Parse wildlife event
            # [timestamp(4)][confidence(4)][species_len(1)][species(var)][lat(4)][lon(4)]
            if len(data) >= 17:
                timestamp = struct.unpack('>I', data[0:4])[0]
                confidence = struct.unpack('>f', data[4:8])[0]
                species_len = data[8]
                species = data[9:9+species_len].decode('utf-8', errors='ignore')
                offset = 9 + species_len
                
                lat = lon = None
                if len(data) >= offset + 8:
                    lat = struct.unpack('>f', data[offset:offset+4])[0]
                    lon = struct.unpack('>f', data[offset+4:offset+8])[0]
                
                # Store detection
                detection_id = self.db.add_detection(
                    node_id=node_id,
                    species=species if species else None,
                    confidence=confidence,
                    latitude=lat if lat and abs(lat) > 0.001 else None,
                    longitude=lon if lon and abs(lon) > 0.001 else None,
                    metadata={'rssi': rssi, 'snr': snr, 'timestamp': timestamp}
                )
                
                if self.on_detection_callback:
                    self.on_detection_callback({
                        'id': detection_id,
                        'node_id': node_id,
                        'species': species,
                        'confidence': confidence,
                        'latitude': lat,
                        'longitude': lon,
                        'rssi': rssi,
                        'snr': snr
                    })
                
                logger.info(f"Wildlife detection from {node_id}: {species} "
                           f"(conf={confidence:.2f})")
        except Exception as e:
            logger.error(f"Error processing wildlife event: {e}")
    
    def _process_telemetry(self, node_id: str, data: bytes, rssi: int, snr: float):
        """Process telemetry packet"""
        try:
            # Parse telemetry
            # [temp(4)][humidity(4)][pressure(4)][battery_voltage(4)][battery_pct(1)]
            if len(data) >= 17:
                temp = struct.unpack('>f', data[0:4])[0]
                humidity = struct.unpack('>f', data[4:8])[0]
                pressure = struct.unpack('>f', data[8:12])[0]
                voltage = struct.unpack('>f', data[12:16])[0]
                battery_pct = data[16]
                
                # Store telemetry
                self.db.add_telemetry(
                    node_id=node_id,
                    temperature=temp if -50 < temp < 100 else None,
                    humidity=humidity if 0 <= humidity <= 100 else None,
                    pressure=pressure if 800 < pressure < 1200 else None,
                    battery_voltage=voltage if 0 < voltage < 10 else None,
                    battery_percentage=battery_pct if 0 <= battery_pct <= 100 else None,
                    rssi=rssi,
                    snr=snr
                )
                
                # Update node battery
                if 0 <= battery_pct <= 100:
                    self.db.add_or_update_node(
                        node_id=node_id,
                        battery_level=battery_pct
                    )
                
                if self.on_telemetry_callback:
                    self.on_telemetry_callback({
                        'node_id': node_id,
                        'temperature': temp,
                        'humidity': humidity,
                        'pressure': pressure,
                        'battery_voltage': voltage,
                        'battery_percentage': battery_pct,
                        'rssi': rssi,
                        'snr': snr
                    })
                
                logger.debug(f"Telemetry from {node_id}: T={temp:.1f}°C, "
                            f"H={humidity:.1f}%, B={battery_pct}%")
        except Exception as e:
            logger.error(f"Error processing telemetry: {e}")
    
    def _process_data(self, node_id: str, data: bytes, rssi: int, snr: float):
        """Process generic data packet"""
        try:
            # Try to decode as JSON
            try:
                json_data = json.loads(data.decode('utf-8'))
                logger.info(f"Data from {node_id}: {json_data}")
            except:
                # Binary data
                logger.debug(f"Binary data from {node_id}: {len(data)} bytes")
        except Exception as e:
            logger.error(f"Error processing data: {e}")
    
    def _process_image_chunk(self, node_id: str, data: bytes, rssi: int, snr: float):
        """Process image data chunk"""
        try:
            # Image chunking would be implemented here
            # For now, just log it
            logger.info(f"Image chunk from {node_id}: {len(data)} bytes")
        except Exception as e:
            logger.error(f"Error processing image chunk: {e}")
    
    def get_stats(self) -> Dict[str, Any]:
        """Get gateway statistics"""
        uptime = 0
        if self.start_time:
            uptime = int((datetime.now() - self.start_time).total_seconds())
        
        return {
            'running': self.running,
            'uptime_seconds': uptime,
            'packets_received': self.packets_received,
            'packets_processed': self.packets_processed,
            'packets_errors': self.packets_errors,
            'success_rate': (self.packets_processed / self.packets_received * 100) 
                           if self.packets_received > 0 else 0
        }
    
    def set_detection_callback(self, callback: Callable):
        """Set callback for wildlife detections"""
        self.on_detection_callback = callback
    
    def set_telemetry_callback(self, callback: Callable):
        """Set callback for telemetry data"""
        self.on_telemetry_callback = callback
    
    def set_beacon_callback(self, callback: Callable):
        """Set callback for beacons"""
        self.on_beacon_callback = callback
    
    def __del__(self):
        """Cleanup on destruction"""
        self.stop()
        if LORA_AVAILABLE:
            try:
                BOARD.teardown()
            except:
                pass
