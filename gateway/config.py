"""
Configuration management for Raspberry Pi LoRa Gateway
"""

import os
from dataclasses import dataclass
from typing import Optional


@dataclass
class LoRaConfig:
    """LoRa radio configuration"""
    frequency: float = 915.0  # MHz (915 for US, 868 for EU, 433 for Asia)
    bandwidth: int = 125000  # Hz
    spreading_factor: int = 7  # 7-12
    coding_rate: int = 5  # 5-8
    tx_power: int = 17  # dBm
    sync_word: int = 0x12
    preamble_length: int = 8
    enable_crc: bool = True
    
    # SPI pins for SX127x/SX1262 on Raspberry Pi
    spi_bus: int = 0
    spi_device: int = 0
    cs_pin: int = 8  # GPIO 8 (CE0)
    reset_pin: int = 25  # GPIO 25
    dio0_pin: int = 24  # GPIO 24
    dio1_pin: Optional[int] = None  # Optional for SX1262


@dataclass
class MQTTConfig:
    """MQTT broker configuration"""
    enabled: bool = True
    broker_host: str = "localhost"
    broker_port: int = 1883
    username: Optional[str] = None
    password: Optional[str] = None
    client_id: str = "wildcam_gateway"
    
    # Topic configuration
    topic_prefix: str = "wildcam"
    detection_topic: str = "detections"
    telemetry_topic: str = "telemetry"
    health_topic: str = "health"
    
    # QoS and connection settings
    qos: int = 1
    retain: bool = False
    keepalive: int = 60
    reconnect_delay: int = 5  # seconds
    
    # TLS/SSL configuration
    use_tls: bool = False
    ca_cert: Optional[str] = None
    client_cert: Optional[str] = None
    client_key: Optional[str] = None


@dataclass
class DatabaseConfig:
    """SQLite database configuration"""
    db_path: str = "gateway_data.db"
    max_records: int = 100000  # Max records before rotation
    retention_days: int = 30  # Days to keep old data
    backup_enabled: bool = True
    backup_interval: int = 86400  # seconds (24 hours)


@dataclass
class APIConfig:
    """REST API configuration"""
    enabled: bool = True
    host: str = "0.0.0.0"
    port: int = 5000
    debug: bool = False
    cors_enabled: bool = True
    api_key: Optional[str] = None  # Optional API key for security


@dataclass
class CloudSyncConfig:
    """Cloud synchronization configuration"""
    enabled: bool = True
    sync_interval: int = 300  # seconds (5 minutes)
    batch_size: int = 100
    retry_attempts: int = 3
    retry_delay: int = 60  # seconds
    
    # Cloud endpoints
    cloud_api_url: Optional[str] = None
    cloud_api_key: Optional[str] = None


@dataclass
class GatewayConfig:
    """Main gateway configuration"""
    # Component configurations (required fields first)
    lora: LoRaConfig
    mqtt: MQTTConfig
    database: DatabaseConfig
    api: APIConfig
    cloud_sync: CloudSyncConfig
    
    # Gateway identification
    gateway_id: str = "gateway_001"
    gateway_name: str = "WildCAM Gateway"
    location: Optional[str] = None
    
    # Operational settings
    max_nodes: int = 100
    beacon_interval: int = 60  # seconds
    health_check_interval: int = 30  # seconds
    node_timeout: int = 300  # seconds (5 minutes)
    
    # Logging
    log_level: str = "INFO"
    log_file: Optional[str] = "gateway.log"
    log_max_bytes: int = 10485760  # 10MB
    log_backup_count: int = 5


def load_config_from_env() -> GatewayConfig:
    """Load configuration from environment variables"""
    
    lora_config = LoRaConfig(
        frequency=float(os.getenv('LORA_FREQUENCY', '915.0')),
        bandwidth=int(os.getenv('LORA_BANDWIDTH', '125000')),
        spreading_factor=int(os.getenv('LORA_SPREADING_FACTOR', '7')),
        coding_rate=int(os.getenv('LORA_CODING_RATE', '5')),
        tx_power=int(os.getenv('LORA_TX_POWER', '17')),
        cs_pin=int(os.getenv('LORA_CS_PIN', '8')),
        reset_pin=int(os.getenv('LORA_RESET_PIN', '25')),
        dio0_pin=int(os.getenv('LORA_DIO0_PIN', '24')),
    )
    
    mqtt_config = MQTTConfig(
        enabled=os.getenv('MQTT_ENABLED', 'true').lower() == 'true',
        broker_host=os.getenv('MQTT_BROKER_HOST', 'localhost'),
        broker_port=int(os.getenv('MQTT_BROKER_PORT', '1883')),
        username=os.getenv('MQTT_USERNAME'),
        password=os.getenv('MQTT_PASSWORD'),
        client_id=os.getenv('MQTT_CLIENT_ID', 'wildcam_gateway'),
        topic_prefix=os.getenv('MQTT_TOPIC_PREFIX', 'wildcam'),
        use_tls=os.getenv('MQTT_USE_TLS', 'false').lower() == 'true',
        ca_cert=os.getenv('MQTT_CA_CERT'),
        client_cert=os.getenv('MQTT_CLIENT_CERT'),
        client_key=os.getenv('MQTT_CLIENT_KEY'),
    )
    
    database_config = DatabaseConfig(
        db_path=os.getenv('DB_PATH', 'gateway_data.db'),
        max_records=int(os.getenv('DB_MAX_RECORDS', '100000')),
        retention_days=int(os.getenv('DB_RETENTION_DAYS', '30')),
        backup_enabled=os.getenv('DB_BACKUP_ENABLED', 'true').lower() == 'true',
    )
    
    api_config = APIConfig(
        enabled=os.getenv('API_ENABLED', 'true').lower() == 'true',
        host=os.getenv('API_HOST', '0.0.0.0'),
        port=int(os.getenv('API_PORT', '5000')),
        debug=os.getenv('API_DEBUG', 'false').lower() == 'true',
        api_key=os.getenv('API_KEY'),
    )
    
    cloud_sync_config = CloudSyncConfig(
        enabled=os.getenv('CLOUD_SYNC_ENABLED', 'true').lower() == 'true',
        sync_interval=int(os.getenv('CLOUD_SYNC_INTERVAL', '300')),
        cloud_api_url=os.getenv('CLOUD_API_URL'),
        cloud_api_key=os.getenv('CLOUD_API_KEY'),
    )
    
    return GatewayConfig(
        gateway_id=os.getenv('GATEWAY_ID', 'gateway_001'),
        gateway_name=os.getenv('GATEWAY_NAME', 'WildCAM Gateway'),
        location=os.getenv('GATEWAY_LOCATION'),
        lora=lora_config,
        mqtt=mqtt_config,
        database=database_config,
        api=api_config,
        cloud_sync=cloud_sync_config,
        log_level=os.getenv('LOG_LEVEL', 'INFO'),
        log_file=os.getenv('LOG_FILE', 'gateway.log'),
    )


# Default configuration instance
default_config = GatewayConfig(
    gateway_id="gateway_001",
    gateway_name="WildCAM Gateway",
    lora=LoRaConfig(),
    mqtt=MQTTConfig(),
    database=DatabaseConfig(),
    api=APIConfig(),
    cloud_sync=CloudSyncConfig(),
)
