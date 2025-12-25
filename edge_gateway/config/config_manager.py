"""
Configuration Management for Edge Gateway
Handles YAML-based configuration with validation and defaults
"""

import yaml
import logging
from pathlib import Path
from typing import Dict, Any, Optional
from dataclasses import dataclass, field, asdict
from enum import Enum

logger = logging.getLogger(__name__)


class CameraType(Enum):
    """Camera source types"""
    USB = "usb"
    RTSP = "rtsp"
    IP = "ip"
    FILE = "file"


@dataclass
class CameraConfig:
    """Camera configuration"""
    name: str
    source: str
    type: str
    fps: int = 30
    resolution: list = field(default_factory=lambda: [1920, 1080])
    enabled: bool = True


@dataclass
class DetectionConfig:
    """Detection/inference configuration"""
    model_path: str = "models/yolov8n_wildlife_trt.engine"
    confidence_threshold: float = 0.6
    nms_threshold: float = 0.45
    input_size: list = field(default_factory=lambda: [640, 640])
    classes: list = field(default_factory=lambda: [
        "deer", "bear", "fox", "rabbit", "bird",
        "coyote", "raccoon", "skunk", "squirrel", "turkey"
    ])


@dataclass
class CloudConfig:
    """Cloud synchronization configuration"""
    api_url: str = "http://localhost:5000"
    api_key: str = ""
    sync_interval: int = 300
    offline_queue_size: int = 1000
    max_retries: int = 5
    batch_size: int = 50
    upload_images: bool = True


@dataclass
class ESP32Config:
    """ESP32 communication configuration"""
    http_port: int = 8080
    websocket_port: int = 8081
    lora_gateway: Optional[str] = None
    lora_baudrate: int = 115200


@dataclass
class StorageConfig:
    """Storage configuration"""
    detections_dir: str = "data/detections"
    logs_dir: str = "logs"
    queue_db: str = "data/sync_queue.db"
    max_detection_age_days: int = 30


@dataclass
class GatewayConfig:
    """Main gateway configuration"""
    gateway_id: str = "jetson_gateway_001"
    log_level: str = "INFO"
    cameras: list = field(default_factory=list)
    detection: DetectionConfig = field(default_factory=DetectionConfig)
    cloud: CloudConfig = field(default_factory=CloudConfig)
    esp32: ESP32Config = field(default_factory=ESP32Config)
    storage: StorageConfig = field(default_factory=StorageConfig)


class ConfigManager:
    """
    Configuration manager for edge gateway
    """
    
    def __init__(self, config_path: Optional[Path] = None):
        """
        Initialize configuration manager
        
        Args:
            config_path: Path to configuration file (YAML)
        """
        self.config_path = config_path
        self.config: Optional[GatewayConfig] = None
        
        if config_path and config_path.exists():
            self.load_config(config_path)
        else:
            # Use default configuration
            self.config = GatewayConfig()
            logger.warning("Using default configuration")
    
    def load_config(self, config_path: Path):
        """
        Load configuration from YAML file
        
        Args:
            config_path: Path to config file
        """
        logger.info(f"Loading configuration from {config_path}")
        
        try:
            with open(config_path, 'r') as f:
                config_dict = yaml.safe_load(f)
            
            self.config = self._dict_to_config(config_dict)
            self.config_path = config_path
            
            # Validate configuration
            self._validate_config()
            
            logger.info("Configuration loaded successfully")
            
        except Exception as e:
            logger.error(f"Failed to load configuration: {e}")
            raise
    
    def save_config(self, config_path: Optional[Path] = None):
        """
        Save configuration to YAML file
        
        Args:
            config_path: Path to save config (uses loaded path if not specified)
        """
        if config_path is None:
            config_path = self.config_path
        
        if config_path is None:
            raise ValueError("No config path specified")
        
        logger.info(f"Saving configuration to {config_path}")
        
        try:
            config_dict = self._config_to_dict(self.config)
            
            config_path.parent.mkdir(parents=True, exist_ok=True)
            
            with open(config_path, 'w') as f:
                yaml.dump(config_dict, f, default_flow_style=False, sort_keys=False)
            
            logger.info("Configuration saved successfully")
            
        except Exception as e:
            logger.error(f"Failed to save configuration: {e}")
            raise
    
    def _dict_to_config(self, config_dict: Dict) -> GatewayConfig:
        """Convert dictionary to config dataclass"""
        # Parse cameras
        cameras = []
        for cam_dict in config_dict.get('cameras', []):
            cameras.append(CameraConfig(**cam_dict))
        
        # Parse detection config
        detection_dict = config_dict.get('detection', {})
        detection = DetectionConfig(**detection_dict)
        
        # Parse cloud config
        cloud_dict = config_dict.get('cloud', {})
        cloud = CloudConfig(**cloud_dict)
        
        # Parse ESP32 config
        esp32_dict = config_dict.get('esp32', {})
        esp32 = ESP32Config(**esp32_dict)
        
        # Parse storage config
        storage_dict = config_dict.get('storage', {})
        storage = StorageConfig(**storage_dict)
        
        return GatewayConfig(
            gateway_id=config_dict.get('gateway_id', 'jetson_gateway_001'),
            log_level=config_dict.get('log_level', 'INFO'),
            cameras=cameras,
            detection=detection,
            cloud=cloud,
            esp32=esp32,
            storage=storage
        )
    
    def _config_to_dict(self, config: GatewayConfig) -> Dict:
        """Convert config dataclass to dictionary"""
        return {
            'gateway_id': config.gateway_id,
            'log_level': config.log_level,
            'cameras': [asdict(cam) for cam in config.cameras],
            'detection': asdict(config.detection),
            'cloud': asdict(config.cloud),
            'esp32': asdict(config.esp32),
            'storage': asdict(config.storage)
        }
    
    def _validate_config(self):
        """Validate configuration"""
        if not self.config:
            raise ValueError("No configuration loaded")
        
        # Validate cameras
        if len(self.config.cameras) == 0:
            logger.warning("No cameras configured")
        
        # Validate detection model path
        model_path = Path(self.config.detection.model_path)
        if not model_path.exists() and not model_path.is_absolute():
            logger.warning(f"Detection model not found: {model_path}")
        
        # Validate API key for cloud sync
        if not self.config.cloud.api_key:
            logger.warning("No cloud API key configured - cloud sync disabled")
        
        # Validate log level
        valid_levels = ['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL']
        if self.config.log_level.upper() not in valid_levels:
            logger.warning(f"Invalid log level: {self.config.log_level}, using INFO")
            self.config.log_level = 'INFO'
    
    def get_config(self) -> GatewayConfig:
        """Get current configuration"""
        if self.config is None:
            self.config = GatewayConfig()
        return self.config
    
    def update_config(self, updates: Dict):
        """
        Update configuration with dictionary
        
        Args:
            updates: Dictionary of updates (nested keys supported)
        """
        if not self.config:
            self.config = GatewayConfig()
        
        config_dict = self._config_to_dict(self.config)
        config_dict = self._deep_update(config_dict, updates)
        self.config = self._dict_to_config(config_dict)
        
        logger.info("Configuration updated")
    
    @staticmethod
    def _deep_update(base_dict: Dict, update_dict: Dict) -> Dict:
        """Deep update dictionary"""
        for key, value in update_dict.items():
            if isinstance(value, dict) and key in base_dict:
                base_dict[key] = ConfigManager._deep_update(
                    base_dict.get(key, {}),
                    value
                )
            else:
                base_dict[key] = value
        return base_dict
    
    @staticmethod
    def create_example_config(output_path: Path):
        """
        Create example configuration file
        
        Args:
            output_path: Path to save example config
        """
        example_config = GatewayConfig(
            gateway_id="jetson_gateway_001",
            log_level="INFO",
            cameras=[
                CameraConfig(
                    name="cam_front",
                    source="/dev/video0",
                    type="usb",
                    fps=30,
                    resolution=[1920, 1080],
                    enabled=True
                ),
                CameraConfig(
                    name="cam_trail",
                    source="rtsp://192.168.1.100:554/stream",
                    type="rtsp",
                    fps=30,
                    resolution=[1920, 1080],
                    enabled=True
                )
            ],
            detection=DetectionConfig(
                model_path="models/yolov8n_wildlife_trt.engine",
                confidence_threshold=0.6,
                nms_threshold=0.45,
                input_size=[640, 640],
                classes=[
                    "deer", "bear", "fox", "rabbit", "bird",
                    "coyote", "raccoon", "skunk", "squirrel", "turkey"
                ]
            ),
            cloud=CloudConfig(
                api_url="https://api.wildcam.example.com",
                api_key="your-api-key-here",
                sync_interval=300,
                offline_queue_size=1000,
                max_retries=5,
                batch_size=50,
                upload_images=True
            ),
            esp32=ESP32Config(
                http_port=8080,
                websocket_port=8081,
                lora_gateway="/dev/ttyUSB0",
                lora_baudrate=115200
            ),
            storage=StorageConfig(
                detections_dir="data/detections",
                logs_dir="logs",
                queue_db="data/sync_queue.db",
                max_detection_age_days=30
            )
        )
        
        manager = ConfigManager()
        manager.config = example_config
        manager.save_config(output_path)
        
        logger.info(f"Example configuration created: {output_path}")
