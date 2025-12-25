#!/usr/bin/env python3
"""
Basic IMX500 Wildlife Detection Example

This example demonstrates basic usage of the IMX500 wildlife detector
with MQTT communication.
"""

import sys
import time
import signal
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent.parent))

from edge.arducam import IMX500WildlifeDetector, IMX500Config
from edge.arducam.config import CommunicationProtocol


def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\nShutting down detector...")
    sys.exit(0)


def main():
    """Run basic wildlife detection"""
    
    print("=== IMX500 Wildlife Detection Example ===\n")
    
    # Register signal handler for graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    
    # Create configuration
    config = IMX500Config(
        # Device identification
        device_id="imx500_demo_001",
        location="Demo Location",
        deployment_name="Basic Detection Demo",
        
        # Camera settings
        resolution=(640, 640),
        frame_rate=30,
        detection_threshold=0.6,
        
        # Communication - using MQTT
        protocol=CommunicationProtocol.MQTT,
        mqtt_broker="localhost",
        mqtt_port=1883,
        mqtt_topic_prefix="wildcam/demo",
        
        # Detection settings
        metadata_only=True,
        capture_high_res_on_detect=True,
        target_species=["deer", "bear", "wolf", "fox"],
        min_detection_interval_sec=5,
        
        # Storage
        store_and_forward=True,
        storage_path="/tmp/wildcam_demo",
        
        # Power management (disabled for demo)
        power_save_enabled=False,
    )
    
    print("Configuration:")
    print(f"  Device ID: {config.device_id}")
    print(f"  Protocol: {config.protocol.value}")
    print(f"  MQTT Broker: {config.mqtt_broker}:{config.mqtt_port}")
    print(f"  Detection Threshold: {config.detection_threshold}")
    print(f"  Target Species: {', '.join(config.target_species)}")
    print()
    
    # Create detector
    print("Initializing detector...")
    detector = IMX500WildlifeDetector(config)
    
    # Start detection
    if detector.start():
        print("✓ Detector started successfully!")
        print("\nMonitoring for wildlife... Press Ctrl+C to stop\n")
        
        try:
            while True:
                # Get and display status every 10 seconds
                status = detector.get_status()
                
                print(f"[{status['timestamp']}]")
                print(f"  Frames processed: {status['frame_count']}")
                print(f"  Detections: {status['detection_count']}")
                print(f"  Queue size: {status['queue_size']}")
                print(f"  Active protocols: {[str(p) for p in status['active_protocols']]}")
                print()
                
                time.sleep(10)
                
        except KeyboardInterrupt:
            print("\nStopping detector...")
            detector.stop()
            print("✓ Detector stopped")
            
    else:
        print("✗ Failed to start detector")
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
