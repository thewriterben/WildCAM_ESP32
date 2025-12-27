"""
Raspberry Pi LoRa Gateway - Main Entry Point

Orchestrates LoRa gateway, MQTT bridge, and REST API services.
"""

import asyncio
import signal
import sys
from pathlib import Path
import logging
from threading import Thread

from edge.shared.utils.logging_config import setup_logging
from edge.shared.utils.config_loader import load_config
from edge.raspberry_pi.gateway.lora_gateway import LoRaGateway
from edge.raspberry_pi.gateway.mqtt_bridge import MQTTBridge
from edge.raspberry_pi.gateway.rest_api import create_api_app, run_api_server

logger = logging.getLogger(__name__)


class RaspberryPiGatewayService:
    """
    Main Raspberry Pi gateway service
    
    Coordinates LoRa gateway, MQTT bridge, and REST API.
    """
    
    def __init__(self, config_path: str, environment: str = "production"):
        """
        Initialize gateway service
        
        Args:
            config_path: Path to configuration directory
            environment: Environment name (production, development, etc.)
        """
        self.config_path = config_path
        self.environment = environment
        self.config = None
        
        # Services
        self.lora_gateway = None
        self.mqtt_bridge = None
        self.api_app = None
        self.api_thread = None
        
        # Control
        self.running = False
        
    async def initialize(self):
        """Initialize all services"""
        logger.info("Initializing Raspberry Pi Gateway Service")
        
        # Load configuration
        self.config = load_config(self.config_path, self.environment)
        logger.info(f"Configuration loaded for environment: {self.environment}")
        
        # Setup logging from config
        log_config = self.config.get('logging', {})
        setup_logging(
            name="rpi_gateway",
            level=log_config.get('level', 'INFO'),
            log_file=log_config.get('file')
        )
        
        # Initialize LoRa gateway
        lora_config = self.config.get('lora', {})
        self.lora_gateway = LoRaGateway(
            db_path=lora_config.get('db_path', '/data/lora/gateway.db'),
            frequency=lora_config.get('frequency', 915.0),
            spreading_factor=lora_config.get('spreading_factor', 7),
            bandwidth=lora_config.get('bandwidth', 125000),
            coding_rate=lora_config.get('coding_rate', 5),
            tx_power=lora_config.get('tx_power', 17),
            use_sx1262=lora_config.get('use_sx1262', False)
        )
        await self.lora_gateway.initialize()
        logger.info("LoRa gateway initialized")
        
        # Initialize MQTT bridge
        mqtt_config = self.config.get('mqtt', {})
        self.mqtt_bridge = MQTTBridge(
            broker_host=mqtt_config.get('broker_host', 'localhost'),
            broker_port=mqtt_config.get('broker_port', 1883),
            username=mqtt_config.get('username'),
            password=mqtt_config.get('password'),
            client_id=mqtt_config.get('client_id', 'wildcam_rpi_gateway'),
            use_tls=mqtt_config.get('use_tls', False),
            base_topic=mqtt_config.get('base_topic', 'wildcam/lora')
        )
        logger.info("MQTT bridge initialized")
        
        # Set up LoRa packet callback to publish to MQTT
        self.lora_gateway.set_packet_callback(self._handle_lora_packet)
        
        # Create REST API app
        self.api_app = create_api_app(self.lora_gateway, self.mqtt_bridge)
        logger.info("REST API initialized")
        
    async def start(self):
        """Start all services"""
        if self.running:
            logger.warning("Gateway service already running")
            return
            
        self.running = True
        logger.info("Starting Raspberry Pi Gateway Service")
        
        # Start LoRa gateway
        await self.lora_gateway.start()
        
        # Start MQTT bridge
        await self.mqtt_bridge.start()
        
        # Start REST API in separate thread
        api_config = self.config.get('api', {})
        self.api_thread = Thread(
            target=run_api_server,
            args=(
                self.api_app,
                api_config.get('host', '0.0.0.0'),
                api_config.get('port', 5000)
            ),
            daemon=True
        )
        self.api_thread.start()
        
        logger.info("All services started successfully")
        
    async def stop(self):
        """Stop all services"""
        if not self.running:
            return
            
        self.running = False
        logger.info("Stopping Raspberry Pi Gateway Service")
        
        # Stop services
        await self.mqtt_bridge.stop()
        await self.lora_gateway.stop()
        
        logger.info("All services stopped")
        
    async def _handle_lora_packet(self, packet):
        """Handle LoRa packet and publish to MQTT"""
        # Publish packet to MQTT
        await self.mqtt_bridge.publish_packet(packet.to_dict())
        
        # Publish gateway status periodically
        # (This could be improved with a separate periodic task)
        
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
            
    def get_status(self):
        """Get service status"""
        return {
            'running': self.running,
            'environment': self.environment,
            'lora': self.lora_gateway.get_mesh_stats() if self.lora_gateway else {},
            'mqtt': self.mqtt_bridge.get_statistics() if self.mqtt_bridge else {}
        }


async def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Raspberry Pi LoRa Gateway Service')
    parser.add_argument('--config', default='/etc/wildcam/rpi',
                       help='Configuration directory path')
    parser.add_argument('--env', default='production',
                       help='Environment (production, development, etc.)')
    args = parser.parse_args()
    
    # Setup logging
    setup_logging(name="rpi_gateway", level="INFO")
    
    # Create service
    service = RaspberryPiGatewayService(args.config, args.env)
    
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
