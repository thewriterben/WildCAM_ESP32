"""
Integration with Microsoft MegaDetector for wildlife detection
"""
import requests
import json
from PIL import Image
import numpy as np
import torch
from pathlib import Path

class MegaDetectorIntegration:
    """
    Wrapper for Microsoft's MegaDetector v5 model
    GitHub: https://github.com/microsoft/CameraTraps
    """
    
    def __init__(self):
        self.model_url = "https://github.com/microsoft/CameraTraps/releases/download/v5.0/md_v5a.0.0.pt"
        self.model = None
        self.device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
        
    def download_model(self):
        """Download pre-trained MegaDetector model"""
        import urllib.request
        model_path = Path("models/megadetector_v5.pt")
        model_path.parent.mkdir(exist_ok=True)
        
        if not model_path.exists():
            print("Downloading MegaDetector v5...")
            urllib.request.urlretrieve(self.model_url, model_path)
        
        # Load with YOLOv5
        self.model = torch.hub.load('ultralytics/yolov5', 'custom', 
                                    path=model_path, force_reload=True)
        self.model.to(self.device)
        
    def detect_animals(self, image_path, confidence_threshold=0.8):
        """
        Detect animals in camera trap images
        Returns bounding boxes and confidence scores
        """
        results = self.model(image_path)
        detections = []
        
        for detection in results.xyxy[0]:
            x1, y1, x2, y2, conf, cls = detection.tolist()
            if conf >= confidence_threshold:
                detections.append({
                    'bbox': [x1, y1, x2, y2],
                    'confidence': conf,
                    'category': int(cls)  # 0: animal, 1: person, 2: vehicle
                })
        
        return detections

class WildMeIntegration:
    """
    Integration with WildMe (Wildbook) platform for individual identification
    """
    
    def __init__(self, api_key):
        self.base_url = "https://api.wildme.org"
        self.api_key = api_key
        
    def submit_encounter(self, image_path, metadata):
        """Submit wildlife encounter for individual ID"""
        endpoint = f"{self.base_url}/encounters"
        
        with open(image_path, 'rb') as f:
            files = {'image': f}
            data = {
                'api_key': self.api_key,
                'species': metadata.get('species'),
                'location': metadata.get('location'),
                'datetime': metadata.get('datetime'),
                'photographer': metadata.get('device_id')
            }
            
            response = requests.post(endpoint, files=files, data=data)
            return response.json()