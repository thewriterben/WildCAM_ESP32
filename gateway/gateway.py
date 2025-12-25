#!/usr/bin/env python3
"""
WildCAM LoRa Gateway - Main Entry Point
Raspberry Pi gateway service for ESP32-CAM wildlife monitoring nodes
"""

import sys
import os
import signal
import logging
import argparse
from logging.handlers import RotatingFileHandler
from pathlib import Path

# Add gateway directory to path
sys.path.insert(0, str(Path(__file__).parent))

from config import load_config_from_env, GatewayConfig
from database import GatewayDatabase
from lora_gateway import LoRaGateway
from mqtt_bridge import MQTTBridge
from api_server import create_app
from cloud_sync import CloudSyncService
from diagnostics import GatewayDiagnostics


class WildCAMGateway:
    """Main gateway application"""
    
    def __init__(self, config: GatewayConfig):
        """
        Initialize gateway
        
        Args:
            config: Gateway configuration
        """
        self.config = config
        self.running = False
        
        # Setup logging
        self._setup_logging()
        
        # Initialize components
        self.logger.info("Initializing WildCAM LoRa Gateway...")
        self.logger.info(f"Gateway ID: {config.gateway_id}")
        self.logger.info(f"Gateway Name: {config.gateway_name}")
        
        # Database
        self.db = GatewayDatabase(config.database.db_path)
        
        # LoRa gateway
        self.lora = LoRaGateway(config.lora, self.db)
        
        # MQTT bridge
        self.mqtt = MQTTBridge(config.mqtt, self.db)
        
        # Cloud sync service
        self.cloud_sync = CloudSyncService(config.cloud_sync, self.db)
        
        # Diagnostics
        self.diagnostics = GatewayDiagnostics(config, self.db)
        
        # Flask API (created but not started here)
        self.api_app = None
        if config.api.enabled:
            self.api_app = create_app(config.api, self.db, 
                                     self.lora, self.mqtt, 
                                     self.diagnostics, self.cloud_sync)
        
        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
    
    def _setup_logging(self):
        """Setup logging configuration"""
        log_level = getattr(logging, self.config.log_level.upper(), logging.INFO)
        
        # Create logger
        self.logger = logging.getLogger('wildcam_gateway')
        self.logger.setLevel(log_level)
        
        # Console handler
        console_handler = logging.StreamHandler()
        console_handler.setLevel(log_level)
        console_format = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        console_handler.setFormatter(console_format)
        self.logger.addHandler(console_handler)
        
        # File handler if configured
        if self.config.log_file:
            file_handler = RotatingFileHandler(
                self.config.log_file,
                maxBytes=self.config.log_max_bytes,
                backupCount=self.config.log_backup_count
            )
            file_handler.setLevel(log_level)
            file_handler.setFormatter(console_format)
            self.logger.addHandler(file_handler)
        
        # Set module loggers
        for module in ['lora_gateway', 'mqtt_bridge', 'api_server', 
                      'cloud_sync', 'diagnostics']:
            logging.getLogger(module).setLevel(log_level)
    
    def start(self):
        """Start all gateway services"""
        self.logger.info("=" * 60)
        self.logger.info("Starting WildCAM LoRa Gateway")
        self.logger.info("=" * 60)
        
        self.running = True
        
        # Initialize and start LoRa gateway
        self.logger.info("Initializing LoRa radio...")
        if self.lora.initialize():
            # Set up callbacks to bridge data to MQTT
            self.lora.set_detection_callback(self._on_detection)
            self.lora.set_telemetry_callback(self._on_telemetry)
            self.lora.set_beacon_callback(self._on_beacon)
            
            self.lora.start()
            self.logger.info("✓ LoRa gateway started")
        else:
            self.logger.error("✗ Failed to initialize LoRa gateway")
            self.logger.warning("Gateway will continue without LoRa (for testing)")
        
        # Start MQTT bridge
        if self.config.mqtt.enabled:
            self.logger.info("Initializing MQTT bridge...")
            if self.mqtt.initialize():
                self.mqtt.start()
                if self.mqtt.connect():
                    self.logger.info("✓ MQTT bridge started")
                else:
                    self.logger.warning("✗ MQTT connection failed (will retry)")
            else:
                self.logger.warning("✗ MQTT bridge initialization failed")
        
        # Start cloud sync service
        if self.config.cloud_sync.enabled:
            self.logger.info("Starting cloud sync service...")
            self.cloud_sync.start()
            self.logger.info("✓ Cloud sync service started")
        
        # Start diagnostics monitoring
        self.diagnostics.start_monitoring()
        self.logger.info("✓ Diagnostics monitoring started")
        
        # Start API server (blocking)
        if self.api_app and self.config.api.enabled:
            self.logger.info(f"Starting API server on "
                           f"{self.config.api.host}:{self.config.api.port}...")
            self.logger.info("=" * 60)
            
            try:
                self.api_app.run(
                    host=self.config.api.host,
                    port=self.config.api.port,
                    debug=self.config.api.debug
                )
            except Exception as e:
                self.logger.error(f"API server error: {e}")
        else:
            self.logger.info("API server disabled, entering monitoring loop...")
            self.logger.info("=" * 60)
            
            # Just monitor without API server
            try:
                while self.running:
                    import time
                    time.sleep(1)
            except KeyboardInterrupt:
                pass
    
    def stop(self):
        """Stop all gateway services"""
        self.logger.info("=" * 60)
        self.logger.info("Stopping WildCAM LoRa Gateway")
        self.logger.info("=" * 60)
        
        self.running = False
        
        # Stop services in reverse order
        self.diagnostics.stop_monitoring()
        self.logger.info("✓ Diagnostics stopped")
        
        if self.cloud_sync:
            self.cloud_sync.stop()
            self.logger.info("✓ Cloud sync stopped")
        
        if self.mqtt:
            self.mqtt.stop()
            self.logger.info("✓ MQTT bridge stopped")
        
        if self.lora:
            self.lora.stop()
            self.logger.info("✓ LoRa gateway stopped")
        
        self.logger.info("Gateway shutdown complete")
    
    def _on_detection(self, detection: dict):
        """Handle wildlife detection event"""
        self.logger.info(f"Detection: {detection.get('species', 'unknown')} "
                        f"from node {detection.get('node_id')}")
        
        # Publish to MQTT
        if self.mqtt and self.mqtt.connected:
            self.mqtt.publish_detection(detection)
    
    def _on_telemetry(self, telemetry: dict):
        """Handle telemetry data"""
        self.logger.debug(f"Telemetry from node {telemetry.get('node_id')}")
        
        # Publish to MQTT
        if self.mqtt and self.mqtt.connected:
            self.mqtt.publish_telemetry(telemetry)
    
    def _on_beacon(self, node_id: str, battery: int, lat: float, lon: float, name: str):
        """Handle beacon from node"""
        self.logger.info(f"Beacon from {name} ({node_id}): "
                        f"battery={battery}%")
        
        # Publish node update to MQTT
        if self.mqtt and self.mqtt.connected:
            self.mqtt.publish_node_update({
                'node_id': node_id,
                'node_name': name,
                'battery_level': battery,
                'latitude': lat,
                'longitude': lon
            })
    
    def _signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        self.logger.info(f"Received signal {signum}")
        self.stop()
        sys.exit(0)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='WildCAM LoRa Gateway - Wildlife monitoring gateway service'
    )
    parser.add_argument(
        '--config', '-c',
        help='Path to configuration file (uses env vars by default)'
    )
    parser.add_argument(
        '--version', '-v',
        action='version',
        version='WildCAM Gateway 1.0.0'
    )
    
    args = parser.parse_args()
    
    # Load configuration
    if args.config:
        # TODO: Implement config file loading
        print(f"Config file loading not yet implemented: {args.config}")
        sys.exit(1)
    else:
        config = load_config_from_env()
    
    # Create and start gateway
    gateway = WildCAMGateway(config)
    
    try:
        gateway.start()
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    except Exception as e:
        logging.error(f"Fatal error: {e}", exc_info=True)
        sys.exit(1)
    finally:
        gateway.stop()


if __name__ == '__main__':
    main()
