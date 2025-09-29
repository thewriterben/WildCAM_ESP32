"""
Custom YOLO model trained specifically for wildlife
Based on YOLOv8 architecture
"""
from ultralytics import YOLO
import cv2
import numpy as np
from collections import defaultdict
import supervision as sv

class WildlifeYOLO:
    """
    Wildlife-specific YOLO implementation
    Trained on merged datasets from:
    - iNaturalist
    - Snapshot Serengeti
    - Caltech Camera Traps
    """
    
    def __init__(self):
        self.model = None
        self.species_map = {
            0: 'deer', 1: 'elk', 2: 'moose', 3: 'bear_black',
            4: 'bear_grizzly', 5: 'wolf', 6: 'coyote', 7: 'fox',
            8: 'lynx', 9: 'bobcat', 10: 'mountain_lion', 11: 'rabbit',
            12: 'squirrel', 13: 'raccoon', 14: 'skunk', 15: 'opossum',
            16: 'turkey', 17: 'eagle', 18: 'owl', 19: 'raven',
            20: 'human', 21: 'vehicle', 22: 'dog', 23: 'cat'
        }
        
    def train_custom_model(self, dataset_path):
        """Train YOLO on custom wildlife dataset"""
        model = YOLO('yolov8x.pt')  # Start with pretrained
        
        results = model.train(
            data=f'{dataset_path}/wildlife.yaml',
            epochs=300,
            imgsz=640,
            batch=16,
            device='0',  # GPU
            project='wildlife_detection',
            name='yolo_wildlife_v1',
            patience=50,
            save=True,
            plots=True
        )
        
        # Export for edge deployment
        model.export(format='tflite', int8=True)  # For ESP32
        model.export(format='onnx', simplify=True)  # For edge devices
        
    def track_animals(self, video_path):
        """Track individual animals across video frames"""
        model = YOLO('runs/wildlife_detection/yolo_wildlife_v1/weights/best.pt')
        
        # Initialize tracker
        tracker = sv.ByteTrack()
        box_annotator = sv.BoxAnnotator()
        
        cap = cv2.VideoCapture(video_path)
        animal_tracks = defaultdict(list)
        
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break
                
            results = model(frame)
            detections = sv.Detections.from_ultralytics(results[0])
            detections = tracker.update_with_detections(detections)
            
            for detection_id, bbox, class_id in zip(
                detections.tracker_id,
                detections.xyxy,
                detections.class_id
            ):
                animal_tracks[detection_id].append({
                    'frame': cap.get(cv2.CAP_PROP_POS_FRAMES),
                    'bbox': bbox.tolist(),
                    'species': self.species_map[class_id]
                })
                
        return dict(animal_tracks)