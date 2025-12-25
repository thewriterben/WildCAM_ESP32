"""
IMX500 Wildlife Detector - Real-time Edge AI Wildlife Monitoring

This module implements the core wildlife detection logic using the
Arducam Pivistation 5 with Sony IMX500 smart sensor.
"""

import os
import json
import time
import logging
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass, asdict
import threading
from queue import Queue
import hashlib

from .config import IMX500Config, CommunicationProtocol, DetectionMode

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


@dataclass
class Detection:
    """
    Wildlife detection result
    
    Attributes:
        timestamp: Detection timestamp (ISO format)
        species: Detected species name
        confidence: Detection confidence (0.0-1.0)
        bbox: Bounding box [x, y, width, height]
        track_id: Tracking ID for multi-frame tracking
        frame_number: Frame number in sequence
        device_id: ID of the detecting device
        location: Optional GPS coordinates
        metadata: Additional metadata
    """
    timestamp: str
    species: str
    confidence: float
    bbox: Tuple[int, int, int, int]
    track_id: Optional[int] = None
    frame_number: int = 0
    device_id: str = ""
    location: Optional[Tuple[float, float]] = None
    metadata: Dict[str, Any] = None
    
    def to_dict(self) -> Dict:
        """Convert detection to dictionary"""
        return asdict(self)
    
    def to_json(self) -> str:
        """Convert detection to JSON string"""
        return json.dumps(self.to_dict())


class IMX500WildlifeDetector:
    """
    Main detector class for IMX500-based wildlife monitoring
    
    This class handles:
    - Real-time detection using on-sensor NPU
    - Multi-protocol communication (LoRa, WiFi, MQTT, REST)
    - Local event filtering and metadata-only transmission
    - Store-and-forward for offline operation
    - High-resolution image capture on target detection
    - Power management and sleep modes
    """
    
    def __init__(self, config: IMX500Config):
        """
        Initialize IMX500 Wildlife Detector
        
        Args:
            config: IMX500 configuration object
        """
        self.config = config
        self.config.validate()
        
        self.device_id = config.device_id
        self.running = False
        self.frame_count = 0
        self.detection_count = 0
        
        # Detection queue for asynchronous processing
        self.detection_queue = Queue(maxsize=100)
        
        # Storage for offline mode
        self.storage_path = Path(config.storage_path)
        self.storage_path.mkdir(parents=True, exist_ok=True)
        
        # Communication handlers
        self.comm_handlers = {}
        
        # Tracking state
        self.active_tracks = {}
        
        logger.info(f"IMX500WildlifeDetector initialized: {self.device_id}")
        logger.info(f"Detection mode: {config.detection_mode.value}")
        logger.info(f"Primary protocol: {config.protocol.value}")
    
    def initialize_camera(self) -> bool:
        """
        Initialize IMX500 camera and NPU
        
        Returns:
            True if initialization successful
        """
        try:
            logger.info("Initializing IMX500 camera...")
            
            # In a real implementation, this would initialize the Arducam SDK
            # For now, we provide the interface that would be used
            
            # Example pseudocode for actual implementation:
            # from picamera2 import Picamera2
            # from picamera2.devices import IMX500
            # 
            # self.camera = Picamera2()
            # self.imx500 = IMX500(self.camera)
            # 
            # # Load model onto IMX500 NPU
            # self.imx500.load_model(self.config.model_path)
            # 
            # # Configure camera
            # camera_config = self.camera.create_still_configuration(
            #     main={"size": self.config.resolution}
            # )
            # self.camera.configure(camera_config)
            # self.camera.start()
            
            logger.info("Camera initialized successfully")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize camera: {e}")
            return False
    
    def initialize_communication(self) -> bool:
        """
        Initialize communication protocols
        
        Returns:
            True if at least one protocol initialized successfully
        """
        try:
            logger.info("Initializing communication protocols...")
            
            # Initialize primary protocol
            if self._init_protocol(self.config.protocol):
                logger.info(f"Primary protocol {self.config.protocol.value} ready")
            
            # Initialize fallback protocols
            for protocol in self.config.fallback_protocols:
                if self._init_protocol(protocol):
                    logger.info(f"Fallback protocol {protocol.value} ready")
            
            return len(self.comm_handlers) > 0
            
        except Exception as e:
            logger.error(f"Failed to initialize communication: {e}")
            return False
    
    def _init_protocol(self, protocol: CommunicationProtocol) -> bool:
        """Initialize a specific communication protocol"""
        try:
            if protocol == CommunicationProtocol.MQTT:
                return self._init_mqtt()
            elif protocol == CommunicationProtocol.LORA:
                return self._init_lora()
            elif protocol == CommunicationProtocol.WIFI:
                return self._init_wifi()
            elif protocol == CommunicationProtocol.REST:
                return self._init_rest()
            elif protocol == CommunicationProtocol.SATELLITE:
                return self._init_satellite()
            return False
        except Exception as e:
            logger.error(f"Failed to initialize {protocol.value}: {e}")
            return False
    
    def _init_mqtt(self) -> bool:
        """Initialize MQTT client"""
        try:
            # Placeholder for MQTT initialization
            # In real implementation:
            # import paho.mqtt.client as mqtt
            # client = mqtt.Client()
            # if self.config.mqtt_username:
            #     client.username_pw_set(self.config.mqtt_username, self.config.mqtt_password)
            # client.connect(self.config.mqtt_broker, self.config.mqtt_port)
            # self.comm_handlers[CommunicationProtocol.MQTT] = client
            
            logger.info(f"MQTT client initialized: {self.config.mqtt_broker}:{self.config.mqtt_port}")
            self.comm_handlers[CommunicationProtocol.MQTT] = "mqtt_placeholder"
            return True
        except Exception as e:
            logger.error(f"MQTT initialization failed: {e}")
            return False
    
    def _init_lora(self) -> bool:
        """Initialize LoRa radio"""
        try:
            # Placeholder for LoRa initialization
            # In real implementation, use appropriate LoRa library for Pi
            # from RFM9x import RFM9x or similar
            
            logger.info(f"LoRa initialized: {self.config.lora_frequency} MHz, SF{self.config.lora_spreading_factor}")
            self.comm_handlers[CommunicationProtocol.LORA] = "lora_placeholder"
            return True
        except Exception as e:
            logger.error(f"LoRa initialization failed: {e}")
            return False
    
    def _init_wifi(self) -> bool:
        """Initialize WiFi connection"""
        try:
            # WiFi is typically always available on Pi
            logger.info("WiFi connection verified")
            self.comm_handlers[CommunicationProtocol.WIFI] = "wifi_placeholder"
            return True
        except Exception as e:
            logger.error(f"WiFi initialization failed: {e}")
            return False
    
    def _init_rest(self) -> bool:
        """Initialize REST API client"""
        try:
            # Placeholder for REST client
            # import requests
            # self.comm_handlers[CommunicationProtocol.REST] = requests.Session()
            
            logger.info(f"REST API client initialized: {self.config.rest_api_endpoint}")
            self.comm_handlers[CommunicationProtocol.REST] = "rest_placeholder"
            return True
        except Exception as e:
            logger.error(f"REST initialization failed: {e}")
            return False
    
    def _init_satellite(self) -> bool:
        """Initialize satellite communication"""
        try:
            # Placeholder for satellite modem (e.g., RockBLOCK, Swarm)
            logger.info("Satellite communication initialized")
            self.comm_handlers[CommunicationProtocol.SATELLITE] = "satellite_placeholder"
            return True
        except Exception as e:
            logger.error(f"Satellite initialization failed: {e}")
            return False
    
    def start(self) -> bool:
        """
        Start the wildlife detection system
        
        Returns:
            True if system started successfully
        """
        if self.running:
            logger.warning("Detector already running")
            return False
        
        # Initialize camera
        if not self.initialize_camera():
            logger.error("Failed to start: camera initialization failed")
            return False
        
        # Initialize communication
        if not self.initialize_communication():
            logger.error("Failed to start: no communication protocols available")
            return False
        
        # Start detection thread
        self.running = True
        self.detection_thread = threading.Thread(target=self._detection_loop, daemon=True)
        self.detection_thread.start()
        
        # Start transmission thread
        self.transmission_thread = threading.Thread(target=self._transmission_loop, daemon=True)
        self.transmission_thread.start()
        
        logger.info("Wildlife detection system started")
        return True
    
    def stop(self):
        """Stop the wildlife detection system"""
        logger.info("Stopping wildlife detection system...")
        self.running = False
        
        if hasattr(self, 'detection_thread'):
            self.detection_thread.join(timeout=5)
        if hasattr(self, 'transmission_thread'):
            self.transmission_thread.join(timeout=5)
        
        logger.info("Wildlife detection system stopped")
    
    def _detection_loop(self):
        """Main detection loop running on separate thread"""
        logger.info("Detection loop started")
        
        while self.running:
            try:
                # Run detection based on mode
                if self.config.detection_mode == DetectionMode.CONTINUOUS:
                    self._run_continuous_detection()
                elif self.config.detection_mode == DetectionMode.MOTION_TRIGGERED:
                    self._run_motion_triggered_detection()
                elif self.config.detection_mode == DetectionMode.SCHEDULED:
                    self._run_scheduled_detection()
                else:
                    time.sleep(1)
                
                # Power management
                if self.config.power_save_enabled:
                    self._handle_power_save()
                    
            except Exception as e:
                logger.error(f"Error in detection loop: {e}")
                time.sleep(1)
    
    def _run_continuous_detection(self):
        """Run continuous detection at configured frame rate"""
        # Simulate frame capture and detection
        # In real implementation, this would capture from camera
        # and run inference on IMX500 NPU
        
        detections = self._process_frame()
        
        for detection in detections:
            if self._should_report_detection(detection):
                self.detection_queue.put(detection)
                self.detection_count += 1
                logger.info(f"Detection: {detection.species} ({detection.confidence:.2f})")
        
        self.frame_count += 1
        
        # Frame rate control
        time.sleep(1.0 / self.config.frame_rate)
    
    def _run_motion_triggered_detection(self):
        """Run detection only when motion is detected"""
        # In real implementation, this would use PIR or camera-based motion detection
        # For now, simulate by checking periodically
        
        if self._detect_motion():
            logger.info("Motion detected, starting detection")
            detections = self._process_frame()
            
            for detection in detections:
                if self._should_report_detection(detection):
                    self.detection_queue.put(detection)
                    self.detection_count += 1
        
        time.sleep(1)
    
    def _run_scheduled_detection(self):
        """Run detection on a schedule"""
        # Implement scheduled detection based on time of day
        current_hour = datetime.now().hour
        
        # Example: only run during dawn/dusk hours (5-8 AM, 5-8 PM)
        if (5 <= current_hour <= 8) or (17 <= current_hour <= 20):
            self._run_continuous_detection()
        else:
            time.sleep(60)  # Sleep 1 minute during inactive hours
    
    def _detect_motion(self) -> bool:
        """Detect motion using camera or PIR sensor"""
        # Placeholder for motion detection
        # In real implementation, use camera frame differencing or PIR
        return False
    
    def _process_frame(self) -> List[Detection]:
        """
        Process a single frame for wildlife detection
        
        Returns:
            List of detections found in frame
        """
        # Placeholder for actual IMX500 NPU inference
        # In real implementation:
        # 1. Capture frame from camera
        # 2. NPU inference happens automatically on-sensor
        # 3. Parse detection results from IMX500
        
        # Example detection format from IMX500:
        # detections = self.imx500.get_detections()
        
        # For now, return empty list (no detections in placeholder)
        return []
    
    def _should_report_detection(self, detection: Detection) -> bool:
        """
        Determine if a detection should be reported
        
        Args:
            detection: Detection to evaluate
            
        Returns:
            True if detection should be reported
        """
        # Check confidence threshold
        if detection.confidence < self.config.detection_threshold:
            return False
        
        # Check if species is in target list (if specified)
        if self.config.target_species:
            if detection.species not in self.config.target_species:
                return False
        
        # Check minimum interval between detections
        # (implemented in transmission loop)
        
        return True
    
    def _transmission_loop(self):
        """Handle transmission of detections to gateway/cloud"""
        logger.info("Transmission loop started")
        
        last_transmission = time.time()
        
        while self.running:
            try:
                # Check if it's time to transmit
                if not self.detection_queue.empty():
                    current_time = time.time()
                    
                    if current_time - last_transmission >= self.config.min_detection_interval_sec:
                        detection = self.detection_queue.get()
                        self._transmit_detection(detection)
                        last_transmission = current_time
                
                time.sleep(0.1)
                
            except Exception as e:
                logger.error(f"Error in transmission loop: {e}")
                time.sleep(1)
    
    def _transmit_detection(self, detection: Detection):
        """
        Transmit detection using configured protocol
        
        Args:
            detection: Detection to transmit
        """
        try:
            # Try primary protocol first
            if self._send_via_protocol(self.config.protocol, detection):
                logger.info(f"Detection transmitted via {self.config.protocol.value}")
                return
            
            # Try fallback protocols
            for protocol in self.config.fallback_protocols:
                if self._send_via_protocol(protocol, detection):
                    logger.info(f"Detection transmitted via fallback {protocol.value}")
                    return
            
            # If all protocols fail, store locally
            if self.config.store_and_forward:
                self._store_detection_locally(detection)
                logger.warning("All protocols failed, stored locally")
            
        except Exception as e:
            logger.error(f"Failed to transmit detection: {e}")
    
    def _send_via_protocol(self, protocol: CommunicationProtocol, detection: Detection) -> bool:
        """Send detection via specific protocol"""
        if protocol not in self.comm_handlers:
            return False
        
        try:
            if protocol == CommunicationProtocol.MQTT:
                return self._send_mqtt(detection)
            elif protocol == CommunicationProtocol.LORA:
                return self._send_lora(detection)
            elif protocol == CommunicationProtocol.REST:
                return self._send_rest(detection)
            elif protocol == CommunicationProtocol.SATELLITE:
                return self._send_satellite(detection)
            return False
        except Exception as e:
            logger.error(f"Error sending via {protocol.value}: {e}")
            return False
    
    def _send_mqtt(self, detection: Detection) -> bool:
        """Send detection via MQTT"""
        try:
            topic = f"{self.config.mqtt_topic_prefix}/{self.device_id}/detections"
            payload = detection.to_json()
            
            # In real implementation:
            # client = self.comm_handlers[CommunicationProtocol.MQTT]
            # client.publish(topic, payload)
            
            logger.debug(f"MQTT publish to {topic}: {payload}")
            return True
        except Exception as e:
            logger.error(f"MQTT send failed: {e}")
            return False
    
    def _send_lora(self, detection: Detection) -> bool:
        """Send detection via LoRa (compressed metadata only)"""
        try:
            # LoRa has limited bandwidth, send compressed metadata
            compact_data = {
                "d": self.device_id,
                "t": detection.timestamp,
                "s": detection.species,
                "c": round(detection.confidence, 2),
            }
            
            payload = json.dumps(compact_data).encode()
            
            # In real implementation:
            # radio = self.comm_handlers[CommunicationProtocol.LORA]
            # radio.send(payload)
            
            logger.debug(f"LoRa send: {len(payload)} bytes")
            return True
        except Exception as e:
            logger.error(f"LoRa send failed: {e}")
            return False
    
    def _send_rest(self, detection: Detection) -> bool:
        """Send detection via REST API"""
        try:
            # In real implementation:
            # session = self.comm_handlers[CommunicationProtocol.REST]
            # headers = {"Authorization": f"Bearer {self.config.rest_api_token}"}
            # response = session.post(
            #     self.config.rest_api_endpoint,
            #     json=detection.to_dict(),
            #     headers=headers
            # )
            # return response.status_code == 200
            
            logger.debug(f"REST API post to {self.config.rest_api_endpoint}")
            return True
        except Exception as e:
            logger.error(f"REST API send failed: {e}")
            return False
    
    def _send_satellite(self, detection: Detection) -> bool:
        """Send detection via satellite (ultra-compressed)"""
        try:
            # Satellite has very limited bandwidth and high cost
            # Send only essential data
            minimal_data = f"{self.device_id},{detection.species},{int(detection.confidence*100)}"
            
            # In real implementation:
            # modem = self.comm_handlers[CommunicationProtocol.SATELLITE]
            # modem.send_message(minimal_data)
            
            logger.debug(f"Satellite send: {minimal_data}")
            return True
        except Exception as e:
            logger.error(f"Satellite send failed: {e}")
            return False
    
    def _store_detection_locally(self, detection: Detection):
        """Store detection locally for later sync"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"detection_{timestamp}_{detection.species}.json"
            filepath = self.storage_path / filename
            
            with open(filepath, 'w') as f:
                json.dump(detection.to_dict(), f, indent=2)
            
            logger.info(f"Detection stored locally: {filepath}")
            
            # Check storage limits
            self._manage_storage()
            
        except Exception as e:
            logger.error(f"Failed to store detection locally: {e}")
    
    def _manage_storage(self):
        """Manage local storage to stay within limits"""
        try:
            # Calculate total storage used
            total_size = sum(f.stat().st_size for f in self.storage_path.glob("*") if f.is_file())
            total_mb = total_size / (1024 * 1024)
            
            if total_mb > self.config.max_storage_mb:
                logger.warning(f"Storage limit exceeded: {total_mb:.1f} MB")
                
                # Delete oldest files until under limit
                files = sorted(self.storage_path.glob("*"), key=lambda f: f.stat().st_mtime)
                
                while total_mb > self.config.max_storage_mb * 0.9 and files:
                    oldest = files.pop(0)
                    size_mb = oldest.stat().st_size / (1024 * 1024)
                    oldest.unlink()
                    total_mb -= size_mb
                    logger.info(f"Deleted old file: {oldest.name}")
                    
        except Exception as e:
            logger.error(f"Storage management failed: {e}")
    
    def _handle_power_save(self):
        """Handle power saving sleep mode"""
        if self.config.power_save_enabled:
            if self.config.detection_mode != DetectionMode.CONTINUOUS:
                logger.debug(f"Entering power save sleep: {self.config.sleep_duration_sec}s")
                time.sleep(self.config.sleep_duration_sec)
    
    def get_status(self) -> Dict:
        """
        Get current system status
        
        Returns:
            Dictionary with system status information
        """
        return {
            "device_id": self.device_id,
            "running": self.running,
            "frame_count": self.frame_count,
            "detection_count": self.detection_count,
            "queue_size": self.detection_queue.qsize(),
            "active_protocols": list(self.comm_handlers.keys()),
            "config": self.config.to_dict(),
            "timestamp": datetime.now().isoformat(),
        }
    
    def update_model(self, model_path: str) -> bool:
        """
        Update the wildlife detection model
        
        Args:
            model_path: Path to new RPK model file
            
        Returns:
            True if model updated successfully
        """
        try:
            logger.info(f"Updating model: {model_path}")
            
            # Verify model file exists
            if not os.path.exists(model_path):
                logger.error(f"Model file not found: {model_path}")
                return False
            
            # In real implementation:
            # 1. Stop inference
            # 2. Load new model to IMX500
            # 3. Restart inference
            # self.imx500.load_model(model_path)
            
            self.config.model_path = model_path
            logger.info("Model updated successfully")
            return True
            
        except Exception as e:
            logger.error(f"Model update failed: {e}")
            return False


def main():
    """Example usage of IMX500WildlifeDetector"""
    # Create configuration
    config = IMX500Config(
        device_id="test_imx500_001",
        protocol=CommunicationProtocol.MQTT,
        detection_threshold=0.6,
        target_species=["deer", "bear", "wolf"],
    )
    
    # Create detector
    detector = IMX500WildlifeDetector(config)
    
    # Start detection
    if detector.start():
        try:
            # Run for a while
            logger.info("Detector running... Press Ctrl+C to stop")
            while True:
                status = detector.get_status()
                logger.info(f"Status: {status['detection_count']} detections, {status['frame_count']} frames")
                time.sleep(10)
        except KeyboardInterrupt:
            logger.info("Stopping detector...")
        finally:
            detector.stop()
    else:
        logger.error("Failed to start detector")


if __name__ == "__main__":
    main()
