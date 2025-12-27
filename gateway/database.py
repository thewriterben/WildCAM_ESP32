"""
SQLite database models and storage for gateway data
"""

import sqlite3
import json
import logging
from datetime import datetime, timedelta
from typing import List, Dict, Optional, Any
from contextlib import contextmanager
from pathlib import Path


logger = logging.getLogger(__name__)


class GatewayDatabase:
    """Database manager for gateway data storage"""
    
    def __init__(self, db_path: str = "gateway_data.db"):
        """
        Initialize database connection
        
        Args:
            db_path: Path to SQLite database file
        """
        self.db_path = db_path
        self._init_database()
    
    def _init_database(self):
        """Initialize database schema"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            # Nodes table - track ESP32 nodes
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS nodes (
                    node_id TEXT PRIMARY KEY,
                    node_name TEXT,
                    latitude REAL,
                    longitude REAL,
                    first_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    last_seen TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    last_rssi INTEGER,
                    last_snr REAL,
                    battery_level INTEGER,
                    firmware_version TEXT,
                    status TEXT DEFAULT 'active',
                    total_packets INTEGER DEFAULT 0,
                    metadata TEXT
                )
            """)
            
            # Detections table - wildlife detection events
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS detections (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    node_id TEXT NOT NULL,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    species TEXT,
                    confidence REAL,
                    latitude REAL,
                    longitude REAL,
                    image_filename TEXT,
                    image_size INTEGER,
                    synced_to_cloud BOOLEAN DEFAULT 0,
                    metadata TEXT,
                    FOREIGN KEY (node_id) REFERENCES nodes (node_id)
                )
            """)
            
            # Telemetry table - environmental and health data
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS telemetry (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    node_id TEXT NOT NULL,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    temperature REAL,
                    humidity REAL,
                    pressure REAL,
                    battery_voltage REAL,
                    battery_percentage INTEGER,
                    rssi INTEGER,
                    snr REAL,
                    packet_loss REAL,
                    metadata TEXT,
                    FOREIGN KEY (node_id) REFERENCES nodes (node_id)
                )
            """)
            
            # Packets table - raw LoRa packet log
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS packets (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    source_id TEXT,
                    destination_id TEXT,
                    packet_type TEXT,
                    rssi INTEGER,
                    snr REAL,
                    payload_size INTEGER,
                    hop_count INTEGER,
                    processed BOOLEAN DEFAULT 0,
                    payload TEXT
                )
            """)
            
            # Mesh health table - mesh network statistics
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS mesh_health (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    total_nodes INTEGER,
                    active_nodes INTEGER,
                    packets_received INTEGER,
                    packets_forwarded INTEGER,
                    average_rssi REAL,
                    average_snr REAL,
                    network_uptime INTEGER,
                    metadata TEXT
                )
            """)
            
            # Cloud sync queue - pending uploads
            cursor.execute("""
                CREATE TABLE IF NOT EXISTS sync_queue (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    data_type TEXT NOT NULL,
                    data_id INTEGER NOT NULL,
                    retry_count INTEGER DEFAULT 0,
                    last_attempt TIMESTAMP,
                    status TEXT DEFAULT 'pending',
                    error_message TEXT
                )
            """)
            
            # Create indexes for performance
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_detections_timestamp 
                ON detections(timestamp)
            """)
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_detections_node_id 
                ON detections(node_id)
            """)
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_detections_synced 
                ON detections(synced_to_cloud)
            """)
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_telemetry_timestamp 
                ON telemetry(timestamp)
            """)
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_telemetry_node_id 
                ON telemetry(node_id)
            """)
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_packets_timestamp 
                ON packets(timestamp)
            """)
            cursor.execute("""
                CREATE INDEX IF NOT EXISTS idx_sync_queue_status 
                ON sync_queue(status)
            """)
            
            conn.commit()
            logger.info(f"Database initialized at {self.db_path}")
    
    @contextmanager
    def get_connection(self):
        """Get database connection with context manager"""
        conn = sqlite3.connect(self.db_path, 
                              detect_types=sqlite3.PARSE_DECLTYPES)
        conn.row_factory = sqlite3.Row
        try:
            yield conn
        finally:
            conn.close()
    
    # Node operations
    def add_or_update_node(self, node_id: str, **kwargs):
        """Add or update a node record"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            # Check if node exists
            cursor.execute("SELECT node_id FROM nodes WHERE node_id = ?", (node_id,))
            exists = cursor.fetchone() is not None
            
            if exists:
                # Update existing node
                set_clause = ", ".join([f"{k} = ?" for k in kwargs.keys()])
                set_clause += ", last_seen = CURRENT_TIMESTAMP"
                values = list(kwargs.values()) + [node_id]
                
                cursor.execute(f"""
                    UPDATE nodes 
                    SET {set_clause}
                    WHERE node_id = ?
                """, values)
            else:
                # Insert new node
                columns = ["node_id"] + list(kwargs.keys())
                placeholders = ", ".join(["?"] * len(columns))
                values = [node_id] + list(kwargs.values())
                
                cursor.execute(f"""
                    INSERT INTO nodes ({", ".join(columns)})
                    VALUES ({placeholders})
                """, values)
            
            # Increment packet count
            cursor.execute("""
                UPDATE nodes 
                SET total_packets = total_packets + 1 
                WHERE node_id = ?
            """, (node_id,))
            
            conn.commit()
    
    def get_node(self, node_id: str) -> Optional[Dict]:
        """Get node information"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("SELECT * FROM nodes WHERE node_id = ?", (node_id,))
            row = cursor.fetchone()
            return dict(row) if row else None
    
    def get_all_nodes(self, active_only: bool = False) -> List[Dict]:
        """Get all nodes"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            query = "SELECT * FROM nodes"
            if active_only:
                query += " WHERE status = 'active'"
            query += " ORDER BY last_seen DESC"
            
            cursor.execute(query)
            return [dict(row) for row in cursor.fetchall()]
    
    def mark_node_inactive(self, node_id: str):
        """Mark a node as inactive"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
                UPDATE nodes 
                SET status = 'inactive' 
                WHERE node_id = ?
            """, (node_id,))
            conn.commit()
    
    # Detection operations
    def add_detection(self, node_id: str, species: Optional[str] = None, 
                     confidence: Optional[float] = None, **kwargs) -> int:
        """Add a wildlife detection event"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            metadata = kwargs.pop('metadata', {})
            if isinstance(metadata, dict):
                metadata = json.dumps(metadata)
            
            cursor.execute("""
                INSERT INTO detections (
                    node_id, species, confidence, latitude, longitude,
                    image_filename, image_size, metadata
                )
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                node_id, species, confidence,
                kwargs.get('latitude'), kwargs.get('longitude'),
                kwargs.get('image_filename'), kwargs.get('image_size'),
                metadata
            ))
            
            detection_id = cursor.lastrowid
            conn.commit()
            return detection_id
    
    def get_detections(self, node_id: Optional[str] = None, 
                       limit: int = 100, offset: int = 0,
                       unsynced_only: bool = False) -> List[Dict]:
        """Get detection records"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            query = "SELECT * FROM detections WHERE 1=1"
            params = []
            
            if node_id:
                query += " AND node_id = ?"
                params.append(node_id)
            
            if unsynced_only:
                query += " AND synced_to_cloud = 0"
            
            query += " ORDER BY timestamp DESC LIMIT ? OFFSET ?"
            params.extend([limit, offset])
            
            cursor.execute(query, params)
            return [dict(row) for row in cursor.fetchall()]
    
    def mark_detection_synced(self, detection_id: int):
        """Mark a detection as synced to cloud"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
                UPDATE detections 
                SET synced_to_cloud = 1 
                WHERE id = ?
            """, (detection_id,))
            conn.commit()
    
    # Telemetry operations
    def add_telemetry(self, node_id: str, **kwargs) -> int:
        """Add telemetry data"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            metadata = kwargs.pop('metadata', {})
            if isinstance(metadata, dict):
                metadata = json.dumps(metadata)
            
            cursor.execute("""
                INSERT INTO telemetry (
                    node_id, temperature, humidity, pressure,
                    battery_voltage, battery_percentage, rssi, snr,
                    packet_loss, metadata
                )
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                node_id,
                kwargs.get('temperature'), kwargs.get('humidity'),
                kwargs.get('pressure'), kwargs.get('battery_voltage'),
                kwargs.get('battery_percentage'), kwargs.get('rssi'),
                kwargs.get('snr'), kwargs.get('packet_loss'),
                metadata
            ))
            
            telemetry_id = cursor.lastrowid
            conn.commit()
            return telemetry_id
    
    def get_telemetry(self, node_id: Optional[str] = None,
                     limit: int = 100, offset: int = 0) -> List[Dict]:
        """Get telemetry records"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            if node_id:
                query = """
                    SELECT * FROM telemetry 
                    WHERE node_id = ? 
                    ORDER BY timestamp DESC 
                    LIMIT ? OFFSET ?
                """
                cursor.execute(query, (node_id, limit, offset))
            else:
                query = """
                    SELECT * FROM telemetry 
                    ORDER BY timestamp DESC 
                    LIMIT ? OFFSET ?
                """
                cursor.execute(query, (limit, offset))
            
            return [dict(row) for row in cursor.fetchall()]
    
    # Packet operations
    def add_packet(self, source_id: str, destination_id: str,
                   packet_type: str, rssi: int, snr: float,
                   payload: Any, **kwargs) -> int:
        """Add a raw packet record"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            payload_str = payload
            if isinstance(payload, (dict, list)):
                payload_str = json.dumps(payload)
            elif isinstance(payload, bytes):
                payload_str = payload.hex()
            
            cursor.execute("""
                INSERT INTO packets (
                    source_id, destination_id, packet_type,
                    rssi, snr, payload_size, hop_count, payload
                )
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                source_id, destination_id, packet_type,
                rssi, snr,
                kwargs.get('payload_size', len(str(payload))),
                kwargs.get('hop_count', 0),
                payload_str
            ))
            
            packet_id = cursor.lastrowid
            conn.commit()
            return packet_id
    
    def mark_packet_processed(self, packet_id: int):
        """Mark a packet as processed"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cursor.execute("""
                UPDATE packets 
                SET processed = 1 
                WHERE id = ?
            """, (packet_id,))
            conn.commit()
    
    # Mesh health operations
    def add_mesh_health_snapshot(self, **kwargs):
        """Add a mesh network health snapshot"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            metadata = kwargs.pop('metadata', {})
            if isinstance(metadata, dict):
                metadata = json.dumps(metadata)
            
            cursor.execute("""
                INSERT INTO mesh_health (
                    total_nodes, active_nodes, packets_received,
                    packets_forwarded, average_rssi, average_snr,
                    network_uptime, metadata
                )
                VALUES (?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                kwargs.get('total_nodes', 0),
                kwargs.get('active_nodes', 0),
                kwargs.get('packets_received', 0),
                kwargs.get('packets_forwarded', 0),
                kwargs.get('average_rssi', 0),
                kwargs.get('average_snr', 0),
                kwargs.get('network_uptime', 0),
                metadata
            ))
            
            conn.commit()
    
    # Data retention and cleanup
    def cleanup_old_data(self, retention_days: int = 30):
        """Remove data older than retention period"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            cutoff_date = datetime.now() - timedelta(days=retention_days)
            
            # Clean old telemetry
            cursor.execute("""
                DELETE FROM telemetry 
                WHERE timestamp < ?
            """, (cutoff_date,))
            
            # Clean old packets
            cursor.execute("""
                DELETE FROM packets 
                WHERE timestamp < ? AND processed = 1
            """, (cutoff_date,))
            
            # Clean old mesh health snapshots
            cursor.execute("""
                DELETE FROM mesh_health 
                WHERE timestamp < ?
            """, (cutoff_date,))
            
            conn.commit()
            logger.info(f"Cleaned data older than {retention_days} days")
    
    # Statistics
    def get_stats(self) -> Dict[str, Any]:
        """Get database statistics"""
        with self.get_connection() as conn:
            cursor = conn.cursor()
            
            stats = {}
            
            # Node stats
            cursor.execute("SELECT COUNT(*) as total FROM nodes")
            stats['total_nodes'] = cursor.fetchone()['total']
            
            cursor.execute("""
                SELECT COUNT(*) as active 
                FROM nodes 
                WHERE status = 'active'
            """)
            stats['active_nodes'] = cursor.fetchone()['active']
            
            # Detection stats
            cursor.execute("SELECT COUNT(*) as total FROM detections")
            stats['total_detections'] = cursor.fetchone()['total']
            
            cursor.execute("""
                SELECT COUNT(*) as unsynced 
                FROM detections 
                WHERE synced_to_cloud = 0
            """)
            stats['unsynced_detections'] = cursor.fetchone()['unsynced']
            
            # Packet stats
            cursor.execute("""
                SELECT COUNT(*) as total 
                FROM packets 
                WHERE timestamp >= datetime('now', '-24 hours')
            """)
            stats['packets_24h'] = cursor.fetchone()['total']
            
            # Database size
            stats['db_size_mb'] = Path(self.db_path).stat().st_size / (1024 * 1024)
            
            return stats
    
    def backup(self, backup_path: str):
        """Create database backup"""
        import shutil
        shutil.copy2(self.db_path, backup_path)
        logger.info(f"Database backed up to {backup_path}")
