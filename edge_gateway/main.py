"""
Main Edge Gateway Service
Orchestrates all components: YOLO detection, camera processing, cloud sync, ESP32 protocol, API
"""

import asyncio
import logging
import signal
import sys
from pathlib import Path
from typing import Dict, List, Optional
import argparse
from datetime import datetime

# Import gateway components
from inference.yolo_detector import YOLODetector
from camera.stream_processor import (
    MultiStreamProcessor, CameraConfig, CameraType, DetectionResult
)
from sync.cloud_sync import CloudSyncService
from protocols.esp32_protocol import ESP32ProtocolHandler
from api.gateway_api import GatewayAPI
from config.config_manager import ConfigManager

logger = logging.getLogger(__name__)


class EdgeGatewayService:
    """
    Main edge gateway service orchestrating all components
    """
    
    def __init__(self, config_path: Optional[Path] = None):
        """
        Initialize edge gateway service
        
        Args:
            config_path: Path to configuration file
        """
        # Load configuration
        self.config_manager = ConfigManager(config_path)
        self.config = self.config_manager.get_config()
        
        # Setup logging
        self._setup_logging()
        
        logger.info(f"Initializing Edge Gateway Service: {self.config.gateway_id}")
        
        # Components
        self.detector: Optional[YOLODetector] = None
        self.stream_processor: Optional[MultiStreamProcessor] = None
        self.cloud_sync: Optional[CloudSyncService] = None
        self.esp32_protocol: Optional[ESP32ProtocolHandler] = None
        self.api: Optional[GatewayAPI] = None
        
        # State
        self.running = False
        self.recent_detections: List[Dict] = []
        self.max_recent_detections = 1000
        
        # Initialize components
        self._initialize_components()
    
    def _setup_logging(self):
        """Setup logging configuration"""
        log_dir = Path(self.config.storage.logs_dir)
        log_dir.mkdir(parents=True, exist_ok=True)
        
        log_file = log_dir / f"gateway_{datetime.now().strftime('%Y%m%d')}.log"
        
        logging.basicConfig(
            level=getattr(logging, self.config.log_level.upper()),
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file),
                logging.StreamHandler(sys.stdout)
            ]
        )
    
    def _initialize_components(self):
        """Initialize all gateway components"""
        try:
            # Initialize YOLO detector
            logger.info("Initializing YOLO detector...")
            self.detector = YOLODetector(
                model_path=self.config.detection.model_path,
                confidence_threshold=self.config.detection.confidence_threshold,
                nms_threshold=self.config.detection.nms_threshold,
                input_size=tuple(self.config.detection.input_size),
                class_labels=self.config.detection.classes
            )
            logger.info("YOLO detector initialized")
            
            # Initialize stream processor
            logger.info("Initializing multi-stream processor...")
            save_dir = Path(self.config.storage.detections_dir)
            self.stream_processor = MultiStreamProcessor(
                detector=self.detector,
                save_dir=save_dir,
                record_detections=True
            )
            
            # Add cameras from config
            for cam_config in self.config.cameras:
                camera = CameraConfig(
                    name=cam_config.name,
                    source=cam_config.source,
                    type=CameraType(cam_config.type),
                    fps=cam_config.fps,
                    resolution=tuple(cam_config.resolution),
                    enabled=cam_config.enabled
                )
                self.stream_processor.add_camera(camera)
            
            logger.info(f"Stream processor initialized with {len(self.config.cameras)} cameras")
            
            # Initialize cloud sync
            if self.config.cloud.api_key:
                logger.info("Initializing cloud sync service...")
                self.cloud_sync = CloudSyncService(
                    api_url=self.config.cloud.api_url,
                    api_key=self.config.cloud.api_key,
                    sync_interval=self.config.cloud.sync_interval,
                    max_queue_size=self.config.cloud.offline_queue_size,
                    max_retries=self.config.cloud.max_retries,
                    batch_size=self.config.cloud.batch_size,
                    offline_db_path=self.config.storage.queue_db
                )
                logger.info("Cloud sync service initialized")
            else:
                logger.warning("Cloud sync disabled (no API key configured)")
            
            # Initialize ESP32 protocol handler
            logger.info("Initializing ESP32 protocol handler...")
            self.esp32_protocol = ESP32ProtocolHandler(
                http_port=self.config.esp32.http_port,
                websocket_port=self.config.esp32.websocket_port,
                lora_port=self.config.esp32.lora_gateway
            )
            logger.info("ESP32 protocol handler initialized")
            
            logger.info("All components initialized successfully")
            
        except Exception as e:
            logger.error(f"Failed to initialize components: {e}")
            raise
    
    async def start(self):
        """Start all gateway services"""
        if self.running:
            logger.warning("Gateway already running")
            return
        
        logger.info("Starting Edge Gateway Service...")
        self.running = True
        
        try:
            # Start stream processor
            logger.info("Starting stream processor...")
            self.stream_processor.start()
            
            # Start cloud sync
            if self.cloud_sync:
                logger.info("Starting cloud sync...")
                self.cloud_sync.start()
            
            # Start ESP32 protocol handler
            logger.info("Starting ESP32 protocol handler...")
            await self.esp32_protocol.start()
            
            # Initialize and start API
            logger.info("Starting API server...")
            self.api = GatewayAPI(
                host='0.0.0.0',
                port=8000,
                gateway_service=self
            )
            await self.api.start()
            
            # Start detection processing loop
            asyncio.create_task(self._detection_processing_loop())
            
            logger.info("Edge Gateway Service started successfully")
            logger.info(f"Gateway ID: {self.config.gateway_id}")
            logger.info(f"API available at http://0.0.0.0:8000")
            logger.info(f"ESP32 HTTP API at http://0.0.0.0:{self.config.esp32.http_port}")
            logger.info(f"ESP32 WebSocket at ws://0.0.0.0:{self.config.esp32.websocket_port}")
            
        except Exception as e:
            logger.error(f"Failed to start gateway: {e}")
            await self.stop()
            raise
    
    async def stop(self):
        """Stop all gateway services"""
        logger.info("Stopping Edge Gateway Service...")
        self.running = False
        
        # Stop stream processor
        if self.stream_processor:
            logger.info("Stopping stream processor...")
            self.stream_processor.stop()
        
        # Stop cloud sync
        if self.cloud_sync:
            logger.info("Stopping cloud sync...")
            self.cloud_sync.stop()
        
        # Stop ESP32 protocol handler
        if self.esp32_protocol:
            logger.info("Stopping ESP32 protocol handler...")
            await self.esp32_protocol.stop()
        
        # Stop API
        if self.api:
            logger.info("Stopping API server...")
            await self.api.stop()
        
        logger.info("Edge Gateway Service stopped")
    
    async def _detection_processing_loop(self):
        """Process detections from camera streams"""
        logger.info("Detection processing loop started")
        
        while self.running:
            try:
                # Get detection result from stream processor
                result = self.stream_processor.get_detection(timeout=1.0)
                
                if result is None:
                    continue
                
                # Process detection
                await self._process_detection(result)
                
            except Exception as e:
                logger.error(f"Error in detection processing loop: {e}")
                await asyncio.sleep(1.0)
        
        logger.info("Detection processing loop stopped")
    
    async def _process_detection(self, result: DetectionResult):
        """
        Process detection result
        
        Args:
            result: Detection result from camera stream
        """
        logger.info(
            f"Processing detection from {result.camera_name}: "
            f"{len(result.detections)} objects detected"
        )
        
        # Create detection record
        detection_record = {
            'id': f"{result.camera_name}_{int(result.timestamp * 1000)}",
            'camera_name': result.camera_name,
            'timestamp': result.timestamp,
            'frame_id': result.frame_id,
            'detections': result.detections,
            'detection_count': len(result.detections)
        }
        
        # Add to recent detections
        self.recent_detections.append(detection_record)
        if len(self.recent_detections) > self.max_recent_detections:
            self.recent_detections.pop(0)
        
        # Sync to cloud if enabled
        if self.cloud_sync and self.config.cloud.upload_images:
            # Find image path
            image_path = None
            if result.frame is not None:
                # Image should be saved by stream processor
                timestamp_str = datetime.fromtimestamp(result.timestamp).strftime('%Y%m%d_%H%M%S')
                image_path = str(
                    Path(self.config.storage.detections_dir) / 
                    f"{result.camera_name}_{timestamp_str}_frame{result.frame_id}.jpg"
                )
            
            self.cloud_sync.add_detection(
                camera_name=result.camera_name,
                timestamp=result.timestamp,
                detections=result.detections,
                image_path=image_path,
                priority=8  # High priority for detections
            )
        
        # Broadcast to API clients
        if self.api:
            await self.api.broadcast_detection(detection_record)
    
    # Public API methods for GatewayAPI
    
    def get_status(self) -> Dict:
        """Get current gateway status"""
        return {
            'gateway_id': self.config.gateway_id,
            'running': self.running,
            'uptime': self._get_uptime(),
            'components': {
                'detector': 'running' if self.detector else 'unavailable',
                'stream_processor': 'running' if self.stream_processor and self.stream_processor.running else 'stopped',
                'cloud_sync': 'running' if self.cloud_sync and self.cloud_sync.running else 'stopped',
                'esp32_protocol': 'running' if self.esp32_protocol else 'stopped',
                'api': 'running' if self.api else 'stopped'
            }
        }
    
    def get_stats(self) -> Dict:
        """Get detailed statistics"""
        stats = {
            'gateway_id': self.config.gateway_id,
            'uptime': self._get_uptime()
        }
        
        if self.stream_processor:
            stats['stream_processor'] = self.stream_processor.get_stats()
        
        if self.cloud_sync:
            stats['cloud_sync'] = self.cloud_sync.get_stats()
        
        if self.esp32_protocol:
            stats['esp32_protocol'] = self.esp32_protocol.get_stats()
        
        return stats
    
    def get_config(self) -> Dict:
        """Get current configuration"""
        return self.config_manager._config_to_dict(self.config)
    
    def update_config(self, updates: Dict):
        """Update configuration"""
        self.config_manager.update_config(updates)
        self.config = self.config_manager.get_config()
    
    def get_cameras(self) -> List[Dict]:
        """Get list of cameras"""
        if not self.stream_processor:
            return []
        
        stats = self.stream_processor.get_stats()
        return stats.get('cameras', {})
    
    def enable_camera(self, camera_name: str):
        """Enable camera"""
        # TODO: Implement camera enable/disable
        logger.info(f"Enabling camera: {camera_name}")
    
    def disable_camera(self, camera_name: str):
        """Disable camera"""
        # TODO: Implement camera enable/disable
        logger.info(f"Disabling camera: {camera_name}")
    
    def get_recent_detections(self, limit: int = 50) -> List[Dict]:
        """Get recent detections"""
        return self.recent_detections[-limit:]
    
    def get_detection(self, detection_id: str) -> Optional[Dict]:
        """Get specific detection by ID"""
        for det in self.recent_detections:
            if det['id'] == detection_id:
                return det
        return None
    
    def get_esp32_nodes(self) -> List[Dict]:
        """Get list of ESP32 nodes"""
        if not self.esp32_protocol:
            return []
        
        # Get nodes from HTTP handler
        nodes = []
        for node in self.esp32_protocol.http_handler.nodes.values():
            nodes.append({
                'node_id': node.node_id,
                'name': node.name,
                'ip_address': node.ip_address,
                'last_seen': node.last_seen,
                'firmware_version': node.firmware_version,
                'battery_level': node.battery_level,
                'status': node.status
            })
        return nodes
    
    def get_esp32_node(self, node_id: str) -> Optional[Dict]:
        """Get specific ESP32 node"""
        nodes = self.get_esp32_nodes()
        for node in nodes:
            if node['node_id'] == node_id:
                return node
        return None
    
    async def send_node_command(self, node_id: str, command_data: Dict):
        """Send command to ESP32 node"""
        if not self.esp32_protocol:
            raise RuntimeError("ESP32 protocol not available")
        
        await self.esp32_protocol.websocket_handler.send_command(
            node_id,
            command_data.get('command'),
            command_data.get('params')
        )
    
    async def restart(self):
        """Restart gateway service"""
        logger.info("Restarting gateway service...")
        await self.stop()
        await asyncio.sleep(2)
        await self.start()
    
    async def shutdown(self):
        """Shutdown gateway service"""
        logger.info("Shutting down gateway service...")
        await self.stop()
        sys.exit(0)
    
    def _get_uptime(self) -> float:
        """Get service uptime in seconds"""
        # TODO: Track start time and calculate uptime
        return 0.0


async def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='WildCAM Edge Gateway Service')
    parser.add_argument(
        '--config',
        type=Path,
        default=Path('config/gateway_config.yaml'),
        help='Path to configuration file'
    )
    parser.add_argument(
        '--create-example-config',
        action='store_true',
        help='Create example configuration file and exit'
    )
    
    args = parser.parse_args()
    
    # Create example config if requested
    if args.create_example_config:
        output_path = Path('config/gateway_config.example.yaml')
        ConfigManager.create_example_config(output_path)
        print(f"Example configuration created: {output_path}")
        return
    
    # Initialize and start gateway
    gateway = EdgeGatewayService(config_path=args.config if args.config.exists() else None)
    
    # Setup signal handlers
    def signal_handler(signum, frame):
        logger.info(f"Received signal {signum}, shutting down...")
        asyncio.create_task(gateway.stop())
    
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Start gateway
    try:
        await gateway.start()
        
        # Keep running until stopped
        while gateway.running:
            await asyncio.sleep(1)
            
    except KeyboardInterrupt:
        logger.info("Keyboard interrupt received")
    except Exception as e:
        logger.error(f"Fatal error: {e}")
    finally:
        await gateway.stop()


if __name__ == '__main__':
    asyncio.run(main())
