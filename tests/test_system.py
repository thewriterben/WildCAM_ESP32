import pytest
import requests
import cv2
import numpy as np
from unittest.mock import Mock, patch

class TestWildlifeSystem:
    
    @pytest.fixture
    def mock_camera(self):
        return {
            'device_id': 'ESP32_CAM_001',
            'location': 'Forest Trail A',
            'latitude': 45.5231,
            'longitude': -122.6765
        }
    
    def test_camera_registration(self, mock_camera):
        """Test camera registration with backend"""
        response = requests.post(
            'http://localhost:5000/api/cameras/register',
            json=mock_camera
        )
        assert response.status_code == 201
        assert 'camera_id' in response.json()
    
    def test_image_upload(self):
        """Test image upload from camera to backend"""
        # Create test image
        test_image = np.random.randint(0, 255, (480, 640, 3), dtype=np.uint8)
        cv2.imwrite('test_capture.jpg', test_image)
        
        with open('test_capture.jpg', 'rb') as f:
            files = {'image': f}
            data = {
                'device_id': 'ESP32_CAM_001',
                'metadata': '{"temperature": 22.5, "humidity": 65}'
            }
            response = requests.post(
                'http://localhost:5000/api/upload',
                files=files,
                data=data
            )
        
        assert response.status_code == 200
        assert 'capture_id' in response.json()
    
    def test_wildlife_detection(self):
        """Test AI wildlife detection"""
        from ml.train_model import WildlifeModelTrainer
        
        detector = WildlifeModelTrainer()
        detector.build_model()
        
        # Test with sample image
        result = detector.detect_wildlife('samples/deer.jpg')
        assert 'species' in result
        assert 'confidence' in result
        assert result['confidence'] >= 0 and result['confidence'] <= 1
    
    def test_power_management(self):
        """Test power optimization features"""
        # Simulate battery levels
        battery_levels = [4.2, 3.8, 3.5, 3.2]
        
        for level in battery_levels:
            power_mode = self.get_power_mode(level)
            assert power_mode in ['normal', 'low_power', 'critical']
    
    @staticmethod
    def get_power_mode(voltage):
        if voltage >= 3.7:
            return 'normal'
        elif voltage >= 3.4:
            return 'low_power'
        else:
            return 'critical'