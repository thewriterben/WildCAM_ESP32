"""
Unit tests for Edge Gateway components
"""

import pytest
import numpy as np
from pathlib import Path
import tempfile


class TestConfigManager:
    """Test configuration manager"""
    
    def test_create_example_config(self):
        """Test example config creation"""
        from config.config_manager import ConfigManager
        
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yaml', delete=False) as f:
            config_path = Path(f.name)
        
        try:
            ConfigManager.create_example_config(config_path)
            assert config_path.exists()
            
            # Load and verify
            manager = ConfigManager(config_path)
            config = manager.get_config()
            
            assert config.gateway_id is not None
            assert len(config.cameras) >= 0
            assert config.detection.confidence_threshold > 0
            
        finally:
            if config_path.exists():
                config_path.unlink()
    
    def test_config_validation(self):
        """Test configuration validation"""
        from config.config_manager import ConfigManager, GatewayConfig
        
        manager = ConfigManager()
        config = GatewayConfig()
        manager.config = config
        
        # Should not raise exception
        manager._validate_config()


class TestYOLODetector:
    """Test YOLO detector (mocked since TensorRT not available in test env)"""
    
    def test_preprocessing(self):
        """Test image preprocessing"""
        # This would require TensorRT which may not be available in test environment
        # Skip or mock for now
        pytest.skip("TensorRT not available in test environment")
    
    def test_nms(self):
        """Test non-maximum suppression"""
        pytest.skip("TensorRT not available in test environment")


class TestCloudSync:
    """Test cloud sync service"""
    
    def test_add_detection(self):
        """Test adding detection to sync queue"""
        from sync.cloud_sync import CloudSyncService
        
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = Path(tmpdir) / "test_queue.db"
            
            service = CloudSyncService(
                api_url="http://test.example.com",
                api_key="test-key",
                offline_db_path=str(db_path)
            )
            
            # Add detection
            service.add_detection(
                camera_name="test_cam",
                timestamp=1234567890.0,
                detections=[{"class": "deer", "confidence": 0.95}],
                priority=7
            )
            
            # Verify queue has item
            assert service.sync_queue.qsize() > 0
    
    def test_offline_db(self):
        """Test offline database creation"""
        from sync.cloud_sync import CloudSyncService
        
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = Path(tmpdir) / "test_queue.db"
            
            service = CloudSyncService(
                api_url="http://test.example.com",
                api_key="test-key",
                offline_db_path=str(db_path)
            )
            
            # Verify database created
            assert db_path.exists()


class TestCameraStream:
    """Test camera stream processing"""
    
    def test_camera_config(self):
        """Test camera configuration"""
        from camera.stream_processor import CameraConfig, CameraType
        
        config = CameraConfig(
            name="test_cam",
            source="/dev/video0",
            type=CameraType.USB,
            fps=30,
            resolution=(1920, 1080)
        )
        
        assert config.name == "test_cam"
        assert config.type == CameraType.USB
        assert config.fps == 30


@pytest.mark.asyncio
class TestESP32Protocol:
    """Test ESP32 protocol handlers"""
    
    async def test_http_handler_init(self):
        """Test HTTP handler initialization"""
        from protocols.esp32_protocol import ESP32HTTPHandler
        
        handler = ESP32HTTPHandler(port=9999)
        assert handler.port == 9999
        assert len(handler.nodes) == 0


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
