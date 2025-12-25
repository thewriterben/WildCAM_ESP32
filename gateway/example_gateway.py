#!/usr/bin/env python3
"""
Example usage of the WildCAM Gateway modules
Demonstrates integration of LoRa, MQTT, API, Diagnostics, and Cloud Sync
"""

import logging
import time
import signal
import sys
from pathlib import Path

# Import gateway modules
from config import load_config_from_env, default_config
from database import GatewayDatabase
from lora_gateway import LoRaGateway
from mqtt_bridge import MQTTBridge
from api_server import create_api_server
from diagnostics import GatewayDiagnostics
from cloud_sync import CloudSyncService


# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('gateway.log')
    ]
)

logger = logging.getLogger(__name__)


class WildCAMGateway:
    """Main gateway application"""
    
    def __init__(self, config=None):
        """
        Initialize gateway
        
        Args:
            config: GatewayConfig instance (uses default if None)
        """
        self.config = config or default_config
        self.running = False
        
        # Initialize components
        logger.info("Initializing WildCAM Gateway...")
        
        # Database
        self.db = GatewayDatabase(self.config.database.db_path)
        logger.info("Database initialized")
        
        # LoRa Gateway
        self.lora = LoRaGateway(self.config.lora, self.db)
        logger.info("LoRa gateway created")
        
        # MQTT Bridge
        self.mqtt = MQTTBridge(self.config.mqtt, self.db)
        logger.info("MQTT bridge created")
        
        # Diagnostics
        self.diagnostics = GatewayDiagnostics(self.config, self.db)
        self.diagnostics.set_lora_gateway(self.lora)
        self.diagnostics.set_mqtt_bridge(self.mqtt)
        logger.info("Diagnostics initialized")
        
        # Cloud Sync
        self.cloud_sync = CloudSyncService(self.config.cloud_sync, self.db)
        logger.info("Cloud sync service created")
        
        # API Server
        self.api_server = create_api_server(self.config.api, self.db)
        logger.info("API server created")
        
        # Set up callbacks
        self._setup_callbacks()
    
    def _setup_callbacks(self):
        """Setup callbacks between components"""
        # Forward detections to MQTT
        def on_detection(detection):
            if self.mqtt:
                self.mqtt.publish_detection(detection)
        
        # Forward telemetry to MQTT
        def on_telemetry(telemetry):
            if self.mqtt:
                self.mqtt.publish_telemetry(telemetry)
        
        # Set callbacks
        self.lora.set_detection_callback(on_detection)
        self.lora.set_telemetry_callback(on_telemetry)
    
    def start(self):
        """Start all gateway services"""
        logger.info("Starting WildCAM Gateway...")
        
        self.running = True
        
        # Initialize and start LoRa
        if self.lora.initialize():
            self.lora.start()
            logger.info("✓ LoRa gateway started")
        else:
            logger.warning("✗ LoRa gateway failed to initialize")
        
        # Initialize and start MQTT
        if self.mqtt.initialize():
            self.mqtt.start()
            if self.mqtt.connect():
                logger.info("✓ MQTT bridge connected")
            else:
                logger.warning("✗ MQTT bridge connection failed")
        else:
            logger.warning("✗ MQTT bridge failed to initialize")
        
        # Start cloud sync
        self.cloud_sync.start()
        logger.info("✓ Cloud sync service started")
        
        # Start API server (blocking, run in separate thread if needed)
        if self.api_server and self.config.api.enabled:
            logger.info("✓ Starting API server...")
            # In production, run this in a separate thread
            # self.api_server.run()
        
        logger.info("=" * 60)
        logger.info("WildCAM Gateway is running!")
        logger.info("=" * 60)
        
        # Print status
        self.print_status()
    
    def stop(self):
        """Stop all gateway services"""
        logger.info("Stopping WildCAM Gateway...")
        
        self.running = False
        
        # Stop services
        if self.lora:
            self.lora.stop()
            logger.info("LoRa gateway stopped")
        
        if self.mqtt:
            self.mqtt.stop()
            logger.info("MQTT bridge stopped")
        
        if self.cloud_sync:
            self.cloud_sync.stop()
            logger.info("Cloud sync service stopped")
        
        logger.info("WildCAM Gateway stopped")
    
    def print_status(self):
        """Print current gateway status"""
        print("\n" + "=" * 60)
        print("GATEWAY STATUS")
        print("=" * 60)
        
        # System health
        health = self.diagnostics.get_system_health()
        print(f"\nSystem Health: {health['status'].upper()}")
        for check_name, check_data in health['checks'].items():
            print(f"  {check_name.capitalize()}: {check_data['message']}")
        
        # Mesh health
        mesh = self.diagnostics.get_mesh_health()
        print(f"\nMesh Network: {mesh['status'].upper()}")
        print(f"  Active Nodes: {mesh['active_nodes']}/{mesh['total_nodes']}")
        print(f"  Packets (1h): {mesh['packets_1h']}")
        
        # LoRa status
        lora_status = self.diagnostics.get_lora_status()
        print(f"\nLoRa Radio: {lora_status['status'].upper()}")
        if lora_status['available']:
            print(f"  Packets Received: {lora_status.get('packets_received', 0)}")
            print(f"  Success Rate: {lora_status.get('success_rate_percent', 0):.1f}%")
        
        # Cloud sync
        sync_stats = self.cloud_sync.get_stats()
        print(f"\nCloud Sync: {'ENABLED' if sync_stats['enabled'] else 'DISABLED'}")
        print(f"  Syncs: {sync_stats['syncs_succeeded']}/{sync_stats['syncs_attempted']}")
        print(f"  Retry Queue: {sync_stats['retry_queue_pending']} pending")
        
        # Database stats
        db_stats = self.db.get_stats()
        print(f"\nDatabase:")
        print(f"  Nodes: {db_stats['active_nodes']}/{db_stats['total_nodes']}")
        print(f"  Detections: {db_stats['total_detections']} ({db_stats['unsynced_detections']} unsynced)")
        print(f"  Size: {db_stats['db_size_mb']:.2f} MB")
        
        print("=" * 60 + "\n")
    
    def generate_diagnostic_report(self, format='text'):
        """Generate and print diagnostic report"""
        report = self.diagnostics.generate_diagnostic_report(format=format)
        
        if format == 'text':
            print(report)
        else:
            print(report)
    
    def run_forever(self):
        """Run gateway in main loop"""
        try:
            while self.running:
                time.sleep(10)
                
                # Periodic health check
                if int(time.time()) % 300 == 0:  # Every 5 minutes
                    self.print_status()
        
        except KeyboardInterrupt:
            logger.info("Received interrupt signal")
            self.stop()


def signal_handler(signum, frame):
    """Handle shutdown signals"""
    logger.info(f"Received signal {signum}")
    sys.exit(0)


def main():
    """Main entry point"""
    # Setup signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Load configuration from environment
    # or use default_config
    config = default_config
    
    # Create and start gateway
    gateway = WildCAMGateway(config)
    
    try:
        gateway.start()
        
        # Generate initial diagnostic report
        print("\n")
        gateway.generate_diagnostic_report(format='text')
        print("\n")
        
        # Run forever
        gateway.run_forever()
        
    except Exception as e:
        logger.error(f"Fatal error: {e}", exc_info=True)
    finally:
        gateway.stop()


if __name__ == "__main__":
    main()
