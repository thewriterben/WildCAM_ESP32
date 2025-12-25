"""
Main Service Orchestrator
Coordinates all Arducam services for wildlife monitoring
"""

import logging
import signal
import sys
import time
import yaml
from pathlib import Path
from typing import Optional, Dict
from datetime import datetime

from ..inference.imx500_wildlife_detector import IMX500WildlifeDetector, IMX500Detection
from .image_capture import ImageCaptureService
from .lora_transmitter import LoRaTransmitter
from .mqtt_publisher import MQTTPublisher

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class ArducamService:
    """
    Main service orchestrator for Arducam Pivistation 5
    Manages detector, capture, LoRa, and MQTT services
    """
    
    def __init__(self, config_path: Optional[str] = None):
        """
        Initialize Arducam service
        
        Args:
            config_path: Path to YAML configuration file
        """
        # Load configuration
        self.config = self._load_config(config_path)
        
        # Setup logging from config
        self._setup_logging()
        
        # Initialize services
        logger.info("Initializing Arducam Wildlife Monitoring Service")
        
        self.detector = IMX500WildlifeDetector(self.config.get('inference', {}))
        self.capture_service = ImageCaptureService(self.config)
        self.lora_transmitter = LoRaTransmitter(self.config)
        self.mqtt_publisher = MQTTPublisher(self.config)
        
        # Service state
        self.running = False
        self.start_time = None
        
        # Register signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
        logger.info("Arducam service initialized")
    
    def _load_config(self, config_path: Optional[str]) -> Dict:
        """
        Load configuration from YAML file
        
        Args:
            config_path: Path to config file
            
        Returns:
            Configuration dictionary
        """
        if config_path is None:
            # Try default locations
            default_paths = [
                '/etc/wildcam/arducam.yaml',
                '/etc/wildcam/arducam/production.yaml',
                './config/production.yaml',
                './config/development.yaml',
            ]
            
            for path in default_paths:
                if Path(path).exists():
                    config_path = path
                    break
        
        if config_path and Path(config_path).exists():
            logger.info(f"Loading configuration from {config_path}")
            with open(config_path, 'r') as f:
                return yaml.safe_load(f)
        else:
            logger.warning("No configuration file found, using defaults")
            return {}
    
    def _setup_logging(self):
        """Setup logging based on configuration"""
        log_config = self.config.get('logging', {})
        
        # Set log level
        level_name = log_config.get('level', 'INFO')
        level = getattr(logging, level_name, logging.INFO)
        logging.getLogger().setLevel(level)
        
        # Add file handler if configured
        log_file = log_config.get('log_file')
        if log_file:
            try:
                from logging.handlers import RotatingFileHandler
                
                # Create log directory if needed
                Path(log_file).parent.mkdir(parents=True, exist_ok=True)
                
                handler = RotatingFileHandler(
                    log_file,
                    maxBytes=log_config.get('max_bytes', 10485760),
                    backupCount=log_config.get('backup_count', 5)
                )
                
                formatter = logging.Formatter(
                    '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
                )
                handler.setFormatter(formatter)
                
                logging.getLogger().addHandler(handler)
                logger.info(f"Logging to file: {log_file}")
                
            except Exception as e:
                logger.error(f"Failed to setup file logging: {e}")
    
    def _on_detection(self, detection: IMX500Detection):
        """
        Callback for wildlife detections
        Coordinates all downstream processing
        
        Args:
            detection: Detection object from IMX500
        """
        logger.info(
            f"Detection: {detection.species} "
            f"(confidence: {detection.confidence:.2f}, "
            f"inference: {detection.inference_time_ms:.1f}ms)"
        )
        
        # Publish to MQTT
        self.mqtt_publisher.publish_detection(detection)
        
        # Send via LoRa
        self.lora_transmitter.send_detection(detection)
        
        # Trigger high-res capture if applicable
        if self.capture_service.should_capture(detection.species, detection.confidence):
            image_path = self.capture_service.capture_image(self.detector, detection)
            
            if image_path:
                # Publish image metadata
                self.mqtt_publisher.publish_image_metadata(image_path, detection)
    
    def start(self):
        """Start all services"""
        if self.running:
            logger.warning("Service already running")
            return
        
        logger.info("Starting Arducam Wildlife Monitoring Service")
        self.start_time = datetime.now()
        
        # Initialize detector
        if not self.detector.initialize():
            logger.warning("Detector initialization failed, continuing in simulation mode")
        
        # Register detection callback
        self.detector.register_detection_callback(self._on_detection)
        
        # Start services
        self.lora_transmitter.start()
        self.mqtt_publisher.start()
        self.detector.start()
        
        self.running = True
        
        logger.info("All services started successfully")
        
        # Start health monitoring loop
        self._health_monitor_loop()
    
    def stop(self):
        """Stop all services"""
        if not self.running:
            return
        
        logger.info("Stopping Arducam Wildlife Monitoring Service")
        
        self.running = False
        
        # Stop services in reverse order
        self.detector.stop()
        self.mqtt_publisher.stop()
        self.lora_transmitter.stop()
        
        logger.info("All services stopped")
    
    def _health_monitor_loop(self):
        """
        Main loop for health monitoring and telemetry
        Runs until service is stopped
        """
        health_config = self.config.get('health', {})
        check_interval = health_config.get('check_interval', 30)
        telemetry_interval = health_config.get('telemetry_interval', 300)
        
        last_telemetry = time.time()
        
        try:
            while self.running:
                # Sleep for check interval
                time.sleep(check_interval)
                
                # Check if it's time for telemetry
                if time.time() - last_telemetry >= telemetry_interval:
                    self._publish_telemetry()
                    last_telemetry = time.time()
                
                # Log health status
                self._log_health_status()
                
        except KeyboardInterrupt:
            logger.info("Interrupted by user")
        finally:
            self.stop()
    
    def _publish_telemetry(self):
        """Publish telemetry data"""
        try:
            telemetry = {
                'detector': self.detector.get_stats(),
                'capture': self.capture_service.get_stats(),
                'lora': self.lora_transmitter.get_stats(),
                'mqtt': self.mqtt_publisher.get_stats(),
                'uptime_seconds': (datetime.now() - self.start_time).total_seconds() if self.start_time else 0,
            }
            
            self.mqtt_publisher.publish_telemetry(telemetry)
            self.lora_transmitter.send_telemetry(telemetry)
            
        except Exception as e:
            logger.error(f"Error publishing telemetry: {e}")
    
    def _log_health_status(self):
        """Log health status"""
        log_config = self.config.get('logging', {})
        if not log_config.get('log_inference_stats', True):
            return
        
        try:
            stats = self.detector.get_stats()
            
            logger.info(
                f"Health: "
                f"{stats['total_inferences']} inferences, "
                f"{stats['total_detections']} detections, "
                f"avg {stats.get('avg_inference_time_ms', 0):.1f}ms"
            )
            
        except Exception as e:
            logger.error(f"Error logging health status: {e}")
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        logger.info(f"Received signal {signum}, shutting down...")
        self.stop()
        sys.exit(0)


def main():
    """Main entry point for service"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Arducam Pivistation 5 Wildlife Monitoring Service'
    )
    parser.add_argument(
        '-c', '--config',
        help='Path to configuration file',
        default=None
    )
    
    args = parser.parse_args()
    
    # Create and start service
    service = ArducamService(config_path=args.config)
    
    try:
        service.start()
    except Exception as e:
        logger.error(f"Service error: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
