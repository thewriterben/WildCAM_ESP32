"""
Tests for IMX500 Wildlife Detector

Basic tests to validate configuration and detector initialization.
"""

import pytest
import json
from pathlib import Path
import sys

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from edge.arducam import IMX500WildlifeDetector, IMX500Config
from edge.arducam.config import (
    CommunicationProtocol,
    DetectionMode,
    PRESET_CONFIGS
)
from edge.arducam.model_deployment import ModelConverter, ModelDeployer


class TestIMX500Config:
    """Test configuration classes"""
    
    def test_default_config(self):
        """Test default configuration creation"""
        config = IMX500Config()
        assert config.device_id == "imx500_node_001"
        assert config.resolution == (640, 640)
        assert config.frame_rate == 30
        assert config.detection_threshold == 0.5
    
    def test_config_validation(self):
        """Test configuration validation"""
        config = IMX500Config(detection_threshold=0.7)
        assert config.validate() is True
        
        # Test invalid threshold
        config.detection_threshold = 1.5
        with pytest.raises(ValueError):
            config.validate()
    
    def test_config_to_dict(self):
        """Test configuration serialization to dict"""
        config = IMX500Config(device_id="test_001")
        config_dict = config.to_dict()
        
        assert isinstance(config_dict, dict)
        assert config_dict["device_id"] == "test_001"
        assert config_dict["protocol"] == "mqtt"  # Enum converted to value
    
    def test_config_from_dict(self):
        """Test configuration deserialization from dict"""
        config_dict = {
            "device_id": "test_002",
            "protocol": "lora",
            "detection_threshold": 0.8
        }
        
        config = IMX500Config.from_dict(config_dict)
        assert config.device_id == "test_002"
        assert config.protocol == CommunicationProtocol.LORA
        assert config.detection_threshold == 0.8
    
    def test_preset_configs(self):
        """Test preset configurations"""
        assert "tropical_rainforest" in PRESET_CONFIGS
        assert "arctic_deployment" in PRESET_CONFIGS
        assert "research_station" in PRESET_CONFIGS
        assert "low_power_remote" in PRESET_CONFIGS
        
        # Validate each preset
        for name, config in PRESET_CONFIGS.items():
            assert config.validate() is True


class TestIMX500WildlifeDetector:
    """Test detector class"""
    
    def test_detector_initialization(self, tmp_path):
        """Test detector can be initialized"""
        config = IMX500Config(
            device_id="test_detector",
            storage_path=str(tmp_path / "storage")
        )
        detector = IMX500WildlifeDetector(config)
        
        assert detector.device_id == "test_detector"
        assert detector.running is False
        assert detector.frame_count == 0
        assert detector.detection_count == 0
    
    def test_detector_status(self, tmp_path):
        """Test status reporting"""
        config = IMX500Config(storage_path=str(tmp_path / "storage"))
        detector = IMX500WildlifeDetector(config)
        
        status = detector.get_status()
        
        assert "device_id" in status
        assert "running" in status
        assert "frame_count" in status
        assert "detection_count" in status
        assert status["running"] is False
    
    def test_detector_with_preset(self, tmp_path):
        """Test detector with preset configuration"""
        config = PRESET_CONFIGS["research_station"]
        config.device_id = "test_preset"
        config.storage_path = str(tmp_path / "storage")
        
        detector = IMX500WildlifeDetector(config)
        assert detector.device_id == "test_preset"


class TestModelConverter:
    """Test model conversion utilities"""
    
    def test_converter_initialization(self):
        """Test converter can be initialized"""
        converter = ModelConverter()
        assert converter.converter_path is not None
    
    def test_converter_with_custom_path(self):
        """Test converter with custom path"""
        converter = ModelConverter(converter_path="/custom/path")
        assert converter.converter_path == "/custom/path"


class TestModelDeployer:
    """Test model deployment utilities"""
    
    def test_deployer_initialization(self, tmp_path):
        """Test deployer can be initialized"""
        deployer = ModelDeployer(device_path=str(tmp_path))
        assert deployer.device_path == tmp_path
        assert tmp_path.exists()
    
    def test_list_deployed_models(self, tmp_path):
        """Test listing deployed models"""
        deployer = ModelDeployer(device_path=str(tmp_path))
        models = deployer.list_deployed_models()
        
        assert isinstance(models, dict)
        assert len(models) == 0  # No models initially


class TestConfigurationFiles:
    """Test configuration file loading"""
    
    def test_load_tropical_config(self):
        """Test loading tropical rainforest config"""
        config_path = Path(__file__).parent.parent / "configs" / "tropical_rainforest.json"
        
        if config_path.exists():
            with open(config_path, 'r') as f:
                config_data = json.load(f)
            
            assert "deployment_name" in config_data
            assert "device_config" in config_data
            assert "communication" in config_data
    
    def test_load_arctic_config(self):
        """Test loading arctic deployment config"""
        config_path = Path(__file__).parent.parent / "configs" / "arctic_deployment.json"
        
        if config_path.exists():
            with open(config_path, 'r') as f:
                config_data = json.load(f)
            
            assert "deployment_name" in config_data
            assert config_data["communication"]["protocol"] == "satellite"


class TestEnumTypes:
    """Test enum types"""
    
    def test_communication_protocols(self):
        """Test communication protocol enum"""
        assert CommunicationProtocol.LORA.value == "lora"
        assert CommunicationProtocol.WIFI.value == "wifi"
        assert CommunicationProtocol.MQTT.value == "mqtt"
        assert CommunicationProtocol.REST.value == "rest"
        assert CommunicationProtocol.SATELLITE.value == "satellite"
    
    def test_detection_modes(self):
        """Test detection mode enum"""
        assert DetectionMode.CONTINUOUS.value == "continuous"
        assert DetectionMode.MOTION_TRIGGERED.value == "motion_triggered"
        assert DetectionMode.SCHEDULED.value == "scheduled"
        assert DetectionMode.EVENT_BASED.value == "event_based"


def test_integration_example(tmp_path):
    """Test complete integration example"""
    # Create config
    config = IMX500Config(
        device_id="integration_test",
        protocol=CommunicationProtocol.MQTT,
        detection_threshold=0.6,
        target_species=["deer", "bear"],
        storage_path=str(tmp_path / "storage")
    )
    
    # Create detector
    detector = IMX500WildlifeDetector(config)
    
    # Verify state
    assert detector.device_id == "integration_test"
    assert detector.config.protocol == CommunicationProtocol.MQTT
    assert "deer" in detector.config.target_species
    
    # Get status
    status = detector.get_status()
    assert status["device_id"] == "integration_test"


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
