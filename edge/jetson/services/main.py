"""
Jetson Edge Service - Main orchestrator

Coordinates all edge services including wildlife detection, cloud sync,
and ESP32 protocol handling.
"""

import asyncio
import signal
import sys
from pathlib import Path
from typing import Optional
import logging

from edge.shared.utils.logging_config import setup_logging
from edge.shared.utils.config_loader import load_config
from edge.jetson.inference.wildlife_detector import WildlifeDetector
from edge.jetson.services.cloud_sync import CloudSyncService
from edge.jetson.services.esp32_protocol import ESP32ProtocolHandler

logger = logging.getLogger(__name__)


class JetsonEdgeService:
    """
    Main Jetson edge service orchestrator
    
    Manages lifecycle of all edge services and coordinates their operation.
    """
    
    def __init__(self, config_path: str, environment: str = "production"):
        """
        Initialize Jetson edge service
        
        Args:
            config_path: Path to configuration directory
            environment: Environment name (production, development, etc.)
        """
        self.config_path = config_path
        self.environment = environment
        self.config = None
        
        # Services
        self.detector: Optional[WildlifeDetector] = None
        self.cloud_sync: Optional[CloudSyncService] = None
        self.esp32_handler: Optional[ESP32ProtocolHandler] = None
        
        # Control
        self.running = False
        
    async def initialize(self):
        """Initialize all services"""
        logger.info("Initializing Jetson Edge Service")
        
        # Load configuration
        self.config = load_config(self.config_path, self.environment)
        logger.info(f"Configuration loaded for environment: {self.environment}")
        
        # Setup logging from config
        log_config = self.config.get('logging', {})
        setup_logging(
            name="jetson_edge",
            level=log_config.get('level', 'INFO'),
            log_file=log_config.get('file')
        )
        
        # Initialize wildlife detector
        detector_config = self.config.get('detector', {})
        self.detector = WildlifeDetector(
            model_path=detector_config.get('model_path', '/models/yolov8.onnx'),
            confidence_threshold=detector_config.get('confidence_threshold', 0.5),
            nms_threshold=detector_config.get('nms_threshold', 0.4),
            use_tensorrt=detector_config.get('use_tensorrt', True),
            max_cameras=detector_config.get('max_cameras', 8)
        )
        logger.info("Wildlife detector initialized")
        
        # Initialize cloud sync service
        cloud_config = self.config.get('cloud', {})
        self.cloud_sync = CloudSyncService(
            db_path=cloud_config.get('sync_db_path', '/data/sync_queue.db'),
            cloud_api_url=cloud_config.get('api_url', 'https://api.wildcam.example.com'),
            cloud_api_key=cloud_config.get('api_key', ''),
            batch_size=cloud_config.get('batch_size', 10),
            sync_interval=cloud_config.get('sync_interval', 60),
            max_retry_count=cloud_config.get('max_retry_count', 5),
            retry_base_delay=cloud_config.get('retry_base_delay', 60)
        )
        await self.cloud_sync.initialize()
        logger.info("Cloud sync service initialized")
        
        # Initialize ESP32 protocol handler
        esp32_config = self.config.get('esp32', {})
        self.esp32_handler = ESP32ProtocolHandler(
            host=esp32_config.get('host', '0.0.0.0'),
            http_port=esp32_config.get('http_port', 8080),
            ws_port=esp32_config.get('ws_port', 8081),
            node_timeout=esp32_config.get('node_timeout', 300)
        )
        
        # Set up callbacks
        self.esp32_handler.set_detection_callback(self._handle_detection)
        self.esp32_handler.set_telemetry_callback(self._handle_telemetry)
        self.esp32_handler.set_image_callback(self._handle_image)
        
        logger.info("ESP32 protocol handler initialized")
        
    async def start(self):
        """Start all services"""
        if self.running:
            logger.warning("Jetson Edge Service already running")
            return
            
        self.running = True
        logger.info("Starting Jetson Edge Service")
        
        # Start services
        await self.cloud_sync.start()
        await self.esp32_handler.start()
        
        logger.info("All services started successfully")
        
    async def stop(self):
        """Stop all services"""
        if not self.running:
            return
            
        self.running = False
        logger.info("Stopping Jetson Edge Service")
        
        # Stop services gracefully
        await self.esp32_handler.stop()
        await self.cloud_sync.stop()
        
        logger.info("All services stopped")
        
    async def _handle_detection(self, detection):
        """Handle detection from ESP32 node"""
        logger.info(f"Detection: {detection.payload['species']} from node {detection.node_id}")
        
        # Queue for cloud sync
        await self.cloud_sync.queue_item('detection', detection.to_dict())
        
    async def _handle_telemetry(self, telemetry):
        """Handle telemetry from ESP32 node"""
        logger.debug(f"Telemetry from node {telemetry.node_id}")
        
        # Queue for cloud sync
        await self.cloud_sync.queue_item('telemetry', telemetry.to_dict())
        
    async def _handle_image(self, node_id: int, image_data: bytes, metadata: dict):
        """Handle image from ESP32 node"""
        logger.info(f"Image from node {node_id}, size: {len(image_data)} bytes")
        
        # TODO: Run detection on image with wildlife detector
        # detections = self.detector.detect(image, camera_id=str(node_id))
        
        # Queue for cloud sync
        payload = {
            'node_id': node_id,
            'data': image_data,
            **metadata
        }
        await self.cloud_sync.queue_item('image', payload)
        
    async def run(self):
        """Run service until stopped"""
        await self.initialize()
        await self.start()
        
        # Wait for shutdown signal
        try:
            while self.running:
                await asyncio.sleep(1)
        except asyncio.CancelledError:
            pass
        finally:
            await self.stop()
            
    def get_status(self) -> dict:
        """Get service status"""
        return {
            'running': self.running,
            'environment': self.environment,
            'detector': self.detector.get_statistics() if self.detector else {},
            'cloud_sync': self.cloud_sync.get_statistics() if self.cloud_sync else {},
            'esp32_nodes': len(self.esp32_handler.nodes) if self.esp32_handler else 0
        }


async def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Jetson Edge Gateway Service')
    parser.add_argument('--config', default='/etc/wildcam/jetson',
                       help='Configuration directory path')
    parser.add_argument('--env', default='production',
                       help='Environment (production, development, etc.)')
    args = parser.parse_args()
    
    # Setup logging
    setup_logging(name="jetson_edge", level="INFO")
    
    # Create service
    service = JetsonEdgeService(args.config, args.env)
    
    # Setup signal handlers
    loop = asyncio.get_running_loop()
    
    def signal_handler():
        logger.info("Shutdown signal received")
        asyncio.create_task(service.stop())
        
    for sig in (signal.SIGTERM, signal.SIGINT):
        loop.add_signal_handler(sig, signal_handler)
        
    # Run service
    try:
        await service.run()
    except Exception as e:
        logger.error(f"Service error: {e}")
        sys.exit(1)
        

if __name__ == '__main__':
    asyncio.run(main())
