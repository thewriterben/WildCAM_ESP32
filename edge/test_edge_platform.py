#!/usr/bin/env python3
"""
Edge Computing Platform - Smoke Tests

Basic validation tests to ensure all components are importable and configurable.
"""

import sys
import os
from pathlib import Path

# Add edge directory to path
edge_dir = Path(__file__).parent
sys.path.insert(0, str(edge_dir.parent))


def test_imports():
    """Test that all modules can be imported"""
    print("Testing imports...")
    
    try:
        # Shared components
        from edge.shared.protocols import esp32_messages, cloud_api
        from edge.shared.utils import logging_config, db_helpers, config_loader
        print("✓ Shared components imported successfully")
        
        # Jetson components
        from edge.jetson.inference import wildlife_detector
        from edge.jetson.services import cloud_sync, esp32_protocol, main
        print("✓ Jetson components imported successfully")
        
        # Raspberry Pi components
        from edge.raspberry_pi.gateway import lora_gateway, mqtt_bridge, rest_api, main as rpi_main
        print("✓ Raspberry Pi components imported successfully")
        
        return True
    except Exception as e:
        print(f"✗ Import failed: {e}")
        return False


def test_config_loading():
    """Test configuration loading"""
    print("\nTesting configuration loading...")
    
    try:
        from edge.shared.utils.config_loader import load_config
        
        # Test Jetson config
        jetson_config_path = edge_dir / "jetson" / "config" / "development.yaml"
        if jetson_config_path.exists():
            config = load_config(str(jetson_config_path.parent), "development")
            assert 'detector' in config
            assert 'cloud' in config
            print("✓ Jetson config loaded successfully")
        else:
            print("⚠ Jetson config not found (expected for fresh install)")
        
        # Test Raspberry Pi config
        rpi_config_path = edge_dir / "raspberry_pi" / "config" / "development.yaml"
        if rpi_config_path.exists():
            config = load_config(str(rpi_config_path.parent), "development")
            assert 'lora' in config
            assert 'mqtt' in config
            print("✓ Raspberry Pi config loaded successfully")
        else:
            print("⚠ Raspberry Pi config not found (expected for fresh install)")
        
        return True
    except Exception as e:
        print(f"✗ Config loading failed: {e}")
        return False


def test_data_structures():
    """Test data structure creation"""
    print("\nTesting data structures...")
    
    try:
        from edge.shared.protocols.esp32_messages import (
            DetectionMessage, TelemetryMessage, HeartbeatMessage
        )
        
        # Create detection message
        detection = DetectionMessage(
            node_id=1001,
            species="deer",
            confidence=0.85
        )
        assert detection.node_id == 1001
        assert detection.payload['species'] == "deer"
        print("✓ DetectionMessage created successfully")
        
        # Create telemetry message
        telemetry = TelemetryMessage(
            node_id=1001,
            battery_level=85,
            temperature=22.5
        )
        assert telemetry.payload['battery_level'] == 85
        print("✓ TelemetryMessage created successfully")
        
        # Create heartbeat message
        heartbeat = HeartbeatMessage(
            node_id=1001,
            role="node",
            battery_level=85,
            uptime=3600
        )
        assert heartbeat.payload['uptime'] == 3600
        print("✓ HeartbeatMessage created successfully")
        
        return True
    except Exception as e:
        print(f"✗ Data structure test failed: {e}")
        return False


def test_detector_initialization():
    """Test wildlife detector initialization"""
    print("\nTesting wildlife detector initialization...")
    
    try:
        from edge.jetson.inference.wildlife_detector import WildlifeDetector, Detection
        
        # Create detector (will use placeholder model)
        detector = WildlifeDetector(
            model_path="/tmp/test_model.onnx",
            confidence_threshold=0.5,
            use_tensorrt=False
        )
        
        # Test detection creation
        det = Detection(
            species="deer",
            confidence=0.85,
            bbox=(100, 100, 200, 200),
            camera_id="cam01"
        )
        assert det.species == "deer"
        print("✓ Wildlife detector initialized successfully")
        
        return True
    except Exception as e:
        print(f"✗ Detector initialization failed: {e}")
        return False


def test_directory_structure():
    """Verify directory structure is correct"""
    print("\nVerifying directory structure...")
    
    required_dirs = [
        "shared/protocols",
        "shared/utils",
        "jetson/inference",
        "jetson/services",
        "jetson/config",
        "raspberry_pi/gateway",
        "raspberry_pi/config"
    ]
    
    all_exist = True
    for dir_path in required_dirs:
        full_path = edge_dir / dir_path
        if full_path.exists():
            print(f"✓ {dir_path}")
        else:
            print(f"✗ {dir_path} - MISSING")
            all_exist = False
    
    return all_exist


def test_required_files():
    """Verify required files exist"""
    print("\nVerifying required files...")
    
    required_files = [
        "README.md",
        "docker-compose.yml",
        ".env.example",
        "jetson/Dockerfile",
        "jetson/requirements.txt",
        "jetson/README.md",
        "raspberry_pi/Dockerfile",
        "raspberry_pi/requirements.txt",
        "raspberry_pi/README.md"
    ]
    
    all_exist = True
    for file_path in required_files:
        full_path = edge_dir / file_path
        if full_path.exists():
            print(f"✓ {file_path}")
        else:
            print(f"✗ {file_path} - MISSING")
            all_exist = False
    
    return all_exist


def main():
    """Run all smoke tests"""
    print("=" * 60)
    print("WildCAM Edge Computing Platform - Smoke Tests")
    print("=" * 60)
    
    tests = [
        ("Directory Structure", test_directory_structure),
        ("Required Files", test_required_files),
        ("Module Imports", test_imports),
        ("Configuration Loading", test_config_loading),
        ("Data Structures", test_data_structures),
        ("Detector Initialization", test_detector_initialization)
    ]
    
    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, result))
        except Exception as e:
            print(f"\n✗ {name} failed with exception: {e}")
            results.append((name, False))
    
    # Print summary
    print("\n" + "=" * 60)
    print("Test Summary")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {name}")
    
    print(f"\nTotal: {passed}/{total} tests passed")
    
    if passed == total:
        print("\n🎉 All tests passed!")
        return 0
    else:
        print(f"\n⚠️  {total - passed} test(s) failed")
        return 1


if __name__ == "__main__":
    sys.exit(main())
