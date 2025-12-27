#!/usr/bin/env python3
"""
LoRa Packet Simulator for Testing Gateway
Simulates ESP32 nodes sending packets without actual LoRa hardware
"""

import sys
import time
import struct
import json
import random
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))

from database import GatewayDatabase


# Packet type constants
PACKET_BEACON = 0x01
PACKET_WILDLIFE = 0x05
PACKET_TELEMETRY = 0x07


class LoRaSimulator:
    """Simulates LoRa packets from ESP32 nodes"""
    
    def __init__(self, db: GatewayDatabase, num_nodes: int = 3):
        """
        Initialize simulator
        
        Args:
            db: Database instance
            num_nodes: Number of simulated nodes
        """
        self.db = db
        self.num_nodes = num_nodes
        self.nodes = []
        
        # Generate simulated nodes
        for i in range(num_nodes):
            node_id = 0x10000000 + i
            self.nodes.append({
                'id': node_id,
                'id_str': f"0x{node_id:08X}",
                'name': f"CAM{i+1:02d}",
                'lat': 40.7128 + (random.random() - 0.5) * 0.01,
                'lon': -74.0060 + (random.random() - 0.5) * 0.01,
                'battery': 100 - (i * 10)
            })
    
    def simulate_beacon(self, node: dict):
        """Simulate a beacon packet"""
        print(f"\n📡 Simulating beacon from {node['name']} ({node['id_str']})")
        
        # Update node in database
        self.db.add_or_update_node(
            node_id=node['id_str'],
            node_name=node['name'],
            latitude=node['lat'],
            longitude=node['lon'],
            battery_level=node['battery'],
            last_rssi=random.randint(-100, -60),
            last_snr=random.uniform(-5, 10)
        )
        
        print(f"   Battery: {node['battery']}%")
        print(f"   Location: {node['lat']:.6f}, {node['lon']:.6f}")
    
    def simulate_wildlife_detection(self, node: dict):
        """Simulate a wildlife detection event"""
        species_list = [
            "Deer", "Bear", "Wolf", "Elk", "Fox", 
            "Raccoon", "Coyote", "Bobcat", "Mountain Lion"
        ]
        
        species = random.choice(species_list)
        confidence = random.uniform(0.7, 0.99)
        
        print(f"\n🦌 Simulating wildlife detection from {node['name']}")
        
        detection_id = self.db.add_detection(
            node_id=node['id_str'],
            species=species,
            confidence=confidence,
            latitude=node['lat'],
            longitude=node['lon'],
            image_filename=f"IMG_{int(time.time())}.jpg",
            image_size=random.randint(50000, 200000)
        )
        
        print(f"   Species: {species}")
        print(f"   Confidence: {confidence:.2%}")
        print(f"   Detection ID: {detection_id}")
    
    def simulate_telemetry(self, node: dict):
        """Simulate telemetry data"""
        print(f"\n📊 Simulating telemetry from {node['name']}")
        
        temp = random.uniform(15, 30)
        humidity = random.uniform(40, 80)
        pressure = random.uniform(980, 1020)
        
        self.db.add_telemetry(
            node_id=node['id_str'],
            temperature=temp,
            humidity=humidity,
            pressure=pressure,
            battery_voltage=3.7,
            battery_percentage=node['battery'],
            rssi=random.randint(-100, -60),
            snr=random.uniform(-5, 10)
        )
        
        print(f"   Temperature: {temp:.1f}°C")
        print(f"   Humidity: {humidity:.1f}%")
        print(f"   Pressure: {pressure:.1f} hPa")
    
    def run_simulation(self, duration_seconds: int = 60, interval: int = 5):
        """
        Run simulation for specified duration
        
        Args:
            duration_seconds: How long to run simulation
            interval: Seconds between simulated events
        """
        print("=" * 60)
        print("LoRa Gateway Simulator Starting")
        print("=" * 60)
        print(f"Simulating {self.num_nodes} nodes for {duration_seconds} seconds")
        print(f"Event interval: {interval} seconds")
        print()
        
        start_time = time.time()
        event_count = 0
        
        try:
            while (time.time() - start_time) < duration_seconds:
                # Pick random node
                node = random.choice(self.nodes)
                
                # Randomly choose event type
                event_type = random.choices(
                    ['beacon', 'detection', 'telemetry'],
                    weights=[3, 1, 2]  # More beacons and telemetry
                )[0]
                
                if event_type == 'beacon':
                    self.simulate_beacon(node)
                elif event_type == 'detection':
                    self.simulate_wildlife_detection(node)
                elif event_type == 'telemetry':
                    self.simulate_telemetry(node)
                
                event_count += 1
                
                # Decrease battery slowly
                node['battery'] = max(10, node['battery'] - 0.1)
                
                # Wait for next event
                time.sleep(interval)
            
        except KeyboardInterrupt:
            print("\n\nSimulation interrupted by user")
        
        # Print summary
        elapsed = time.time() - start_time
        print("\n" + "=" * 60)
        print("Simulation Complete")
        print("=" * 60)
        print(f"Duration: {elapsed:.1f} seconds")
        print(f"Events simulated: {event_count}")
        print(f"Events per minute: {event_count / (elapsed / 60):.1f}")
        
        # Show database stats
        stats = self.db.get_stats()
        print("\nDatabase Statistics:")
        print(f"  Total nodes: {stats['total_nodes']}")
        print(f"  Active nodes: {stats['active_nodes']}")
        print(f"  Total detections: {stats['total_detections']}")
        print(f"  Unsynced detections: {stats['unsynced_detections']}")
        print(f"  Database size: {stats['db_size_mb']:.2f} MB")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Simulate LoRa packets for gateway testing'
    )
    parser.add_argument(
        '--nodes', '-n',
        type=int,
        default=3,
        help='Number of nodes to simulate (default: 3)'
    )
    parser.add_argument(
        '--duration', '-d',
        type=int,
        default=60,
        help='Simulation duration in seconds (default: 60)'
    )
    parser.add_argument(
        '--interval', '-i',
        type=int,
        default=5,
        help='Interval between events in seconds (default: 5)'
    )
    parser.add_argument(
        '--database', '-db',
        default='gateway_data.db',
        help='Database path (default: gateway_data.db)'
    )
    
    args = parser.parse_args()
    
    # Create database
    db = GatewayDatabase(args.database)
    
    # Create and run simulator
    simulator = LoRaSimulator(db, args.nodes)
    simulator.run_simulation(args.duration, args.interval)


if __name__ == '__main__':
    main()
