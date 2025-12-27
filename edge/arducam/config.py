"""
Configuration for IMX500 Wildlife Detection System
"""

from dataclasses import dataclass, field
from typing import Dict, List, Optional
from enum import Enum


class CommunicationProtocol(Enum):
    """Supported communication protocols"""
    LORA = "lora"
    WIFI = "wifi"
    MQTT = "mqtt"
    REST = "rest"
    SATELLITE = "satellite"


class DetectionMode(Enum):
    """Detection modes for wildlife monitoring"""
    CONTINUOUS = "continuous"
    MOTION_TRIGGERED = "motion_triggered"
    SCHEDULED = "scheduled"
    EVENT_BASED = "event_based"


@dataclass
class IMX500Config:
    """
    Configuration for IMX500 Wildlife Detection
    
    Attributes:
        device_id: Unique identifier for this edge node
        resolution: Image resolution (default: 640x640 for NPU)
        frame_rate: Target frame rate for detection (max 30fps)
        detection_threshold: Confidence threshold for detections (0.0-1.0)
        protocol: Primary communication protocol
        fallback_protocols: Alternative protocols for resilience
        mqtt_broker: MQTT broker address
        mqtt_port: MQTT broker port
        mqtt_topic_prefix: Prefix for MQTT topics
        rest_api_endpoint: REST API endpoint for detections
        lora_frequency: LoRa frequency (MHz)
        lora_spreading_factor: LoRa spreading factor (7-12)
        lora_bandwidth: LoRa bandwidth (kHz)
        detection_mode: Operating mode for detection
        metadata_only: Send only metadata (no images) to save bandwidth
        capture_high_res_on_detect: Capture 12MP stills when target detected
        target_species: List of target species to monitor
        model_path: Path to deployed RPK model file
        store_and_forward: Enable offline operation with local storage
        max_storage_mb: Maximum local storage for offline mode (MB)
        power_save_enabled: Enable power saving features
        sleep_duration_sec: Sleep duration between captures (seconds)
    """
    
    # Device identification
    device_id: str = "imx500_node_001"
    location: Optional[str] = None
    deployment_name: Optional[str] = None
    
    # Camera settings
    resolution: tuple = (640, 640)
    frame_rate: int = 30
    detection_threshold: float = 0.5
    
    # Communication settings
    protocol: CommunicationProtocol = CommunicationProtocol.MQTT
    fallback_protocols: List[CommunicationProtocol] = field(
        default_factory=lambda: [CommunicationProtocol.WIFI, CommunicationProtocol.LORA]
    )
    
    # MQTT configuration
    mqtt_broker: str = "localhost"
    mqtt_port: int = 1883
    mqtt_topic_prefix: str = "wildcam/imx500"
    mqtt_username: Optional[str] = None
    mqtt_password: Optional[str] = None
    mqtt_use_tls: bool = False
    
    # REST API configuration
    rest_api_endpoint: str = "http://localhost:5000/api/detections"
    rest_api_token: Optional[str] = None
    
    # LoRa configuration
    lora_frequency: float = 915.0  # MHz
    lora_spreading_factor: int = 7
    lora_bandwidth: float = 125.0  # kHz
    lora_tx_power: int = 14  # dBm
    lora_sync_word: int = 0x12
    
    # Detection configuration
    detection_mode: DetectionMode = DetectionMode.CONTINUOUS
    metadata_only: bool = True
    capture_high_res_on_detect: bool = True
    target_species: List[str] = field(default_factory=list)
    min_detection_interval_sec: int = 5
    
    # Model configuration
    model_path: str = "/home/pi/models/wildlife_detector.rpk"
    model_update_interval_hours: int = 24
    
    # Storage and offline operation
    store_and_forward: bool = True
    max_storage_mb: int = 1024
    storage_path: str = "/home/pi/wildcam/storage"
    sync_interval_minutes: int = 60
    
    # Power management
    power_save_enabled: bool = True
    sleep_duration_sec: int = 60
    low_battery_threshold: float = 3.3  # Volts
    
    # Advanced settings
    enable_pose_estimation: bool = False
    enable_tracking: bool = True
    max_tracks: int = 10
    track_timeout_sec: int = 30
    
    # Cloud integration
    cloud_sync_enabled: bool = False
    cloud_provider: Optional[str] = None
    cloud_bucket: Optional[str] = None
    
    def to_dict(self) -> Dict:
        """Convert configuration to dictionary"""
        config_dict = {}
        for key, value in self.__dict__.items():
            if isinstance(value, Enum):
                config_dict[key] = value.value
            elif isinstance(value, list) and value and isinstance(value[0], Enum):
                config_dict[key] = [v.value for v in value]
            else:
                config_dict[key] = value
        return config_dict
    
    @classmethod
    def from_dict(cls, config_dict: Dict) -> "IMX500Config":
        """Create configuration from dictionary"""
        # Convert string enums back to enum objects
        if "protocol" in config_dict and isinstance(config_dict["protocol"], str):
            config_dict["protocol"] = CommunicationProtocol(config_dict["protocol"])
        
        if "fallback_protocols" in config_dict:
            config_dict["fallback_protocols"] = [
                CommunicationProtocol(p) if isinstance(p, str) else p
                for p in config_dict["fallback_protocols"]
            ]
        
        if "detection_mode" in config_dict and isinstance(config_dict["detection_mode"], str):
            config_dict["detection_mode"] = DetectionMode(config_dict["detection_mode"])
        
        return cls(**config_dict)
    
    def validate(self) -> bool:
        """
        Validate configuration parameters
        
        Returns:
            True if configuration is valid
            
        Raises:
            ValueError: If configuration is invalid
        """
        if not 0.0 <= self.detection_threshold <= 1.0:
            raise ValueError("detection_threshold must be between 0.0 and 1.0")
        
        if not 1 <= self.frame_rate <= 30:
            raise ValueError("frame_rate must be between 1 and 30 fps")
        
        if not 7 <= self.lora_spreading_factor <= 12:
            raise ValueError("lora_spreading_factor must be between 7 and 12")
        
        if self.max_storage_mb < 100:
            raise ValueError("max_storage_mb should be at least 100 MB")
        
        return True


# Preset configurations for common deployment scenarios
PRESET_CONFIGS = {
    "tropical_rainforest": IMX500Config(
        deployment_name="Tropical Rainforest Monitoring",
        resolution=(640, 640),
        detection_threshold=0.6,
        protocol=CommunicationProtocol.LORA,
        metadata_only=True,
        capture_high_res_on_detect=True,
        target_species=["jaguar", "tapir", "macaw", "howler_monkey"],
        store_and_forward=True,
        power_save_enabled=True,
        sleep_duration_sec=120,
    ),
    
    "arctic_deployment": IMX500Config(
        deployment_name="Arctic Wildlife Monitoring",
        resolution=(640, 640),
        detection_threshold=0.7,
        protocol=CommunicationProtocol.SATELLITE,
        fallback_protocols=[CommunicationProtocol.LORA],
        metadata_only=True,
        capture_high_res_on_detect=True,
        target_species=["polar_bear", "arctic_fox", "caribou"],
        store_and_forward=True,
        power_save_enabled=True,
        sleep_duration_sec=300,
        low_battery_threshold=3.5,
    ),
    
    "research_station": IMX500Config(
        deployment_name="Research Station - High Bandwidth",
        resolution=(640, 640),
        detection_threshold=0.5,
        protocol=CommunicationProtocol.WIFI,
        fallback_protocols=[CommunicationProtocol.MQTT],
        metadata_only=False,
        capture_high_res_on_detect=True,
        detection_mode=DetectionMode.CONTINUOUS,
        store_and_forward=False,
        power_save_enabled=False,
        cloud_sync_enabled=True,
        enable_tracking=True,
        enable_pose_estimation=True,
    ),
    
    "low_power_remote": IMX500Config(
        deployment_name="Low Power Remote Monitoring",
        resolution=(640, 640),
        detection_threshold=0.65,
        protocol=CommunicationProtocol.LORA,
        metadata_only=True,
        capture_high_res_on_detect=False,
        detection_mode=DetectionMode.MOTION_TRIGGERED,
        store_and_forward=True,
        power_save_enabled=True,
        sleep_duration_sec=600,
        min_detection_interval_sec=30,
    ),
}
