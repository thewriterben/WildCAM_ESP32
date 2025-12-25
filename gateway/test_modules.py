#!/usr/bin/env python3
"""
Quick test script for WildCAM Gateway modules
Tests basic functionality without requiring hardware
"""

import sys
import logging
from pathlib import Path

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)


def test_imports():
    """Test that all modules can be imported"""
    print("\n" + "=" * 60)
    print("TEST 1: Module Imports")
    print("=" * 60)
    
    try:
        from config import default_config, load_config_from_env
        print("✓ config.py imported")
        
        from database import GatewayDatabase
        print("✓ database.py imported")
        
        from lora_gateway import LoRaGateway
        print("✓ lora_gateway.py imported")
        
        from mqtt_bridge import MQTTBridge
        print("✓ mqtt_bridge.py imported")
        
        from api_server import create_api_server
        print("✓ api_server.py imported")
        
        from diagnostics import GatewayDiagnostics
        print("✓ diagnostics.py imported")
        
        from cloud_sync import CloudSyncService
        print("✓ cloud_sync.py imported")
        
        print("\n✓ All modules imported successfully!")
        return True
        
    except Exception as e:
        print(f"\n✗ Import failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_database():
    """Test database initialization"""
    print("\n" + "=" * 60)
    print("TEST 2: Database")
    print("=" * 60)
    
    try:
        from database import GatewayDatabase
        import tempfile
        import os
        
        # Use temporary file database instead of in-memory
        temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        temp_db_path = temp_db.name
        temp_db.close()
        
        try:
            db = GatewayDatabase(temp_db_path)
            print("✓ Database created")
            
            # Test node operations
            db.add_or_update_node("0x12345678", node_name="Test Node", battery_level=85)
            print("✓ Node added")
            
            node = db.get_node("0x12345678")
            assert node is not None
            assert node['node_name'] == "Test Node"
            print(f"✓ Node retrieved: {node['node_name']}")
            
            # Test detection
            detection_id = db.add_detection("0x12345678", species="Deer", confidence=0.95)
            print(f"✓ Detection added with ID: {detection_id}")
            
            # Test telemetry
            telemetry_id = db.add_telemetry("0x12345678", temperature=25.5, humidity=60.0)
            print(f"✓ Telemetry added with ID: {telemetry_id}")
            
            # Get stats
            stats = db.get_stats()
            print(f"✓ Database stats: {stats['total_nodes']} nodes, {stats['total_detections']} detections")
            
            print("\n✓ Database tests passed!")
            return True
        finally:
            # Clean up temp database
            if os.path.exists(temp_db_path):
                os.unlink(temp_db_path)
        
    except Exception as e:
        print(f"\n✗ Database test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_diagnostics():
    """Test diagnostics module"""
    print("\n" + "=" * 60)
    print("TEST 3: Diagnostics")
    print("=" * 60)
    
    try:
        from config import default_config
        from database import GatewayDatabase
        from diagnostics import GatewayDiagnostics
        import tempfile
        import os
        
        # Use temporary file database
        temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        temp_db_path = temp_db.name
        temp_db.close()
        
        try:
            db = GatewayDatabase(temp_db_path)
            diag = GatewayDiagnostics(default_config, db)
            print("✓ Diagnostics initialized")
            
            # Test system health
            health = diag.get_system_health()
            print(f"✓ System health: {health['status']}")
            
            # Test uptime
            uptime = diag.get_uptime()
            print(f"✓ Gateway uptime: {uptime['gateway_uptime_formatted']}")
            
            # Test mesh health
            mesh = diag.get_mesh_health()
            print(f"✓ Mesh network status: {mesh['status']}")
            
            # Generate report
            report = diag.generate_diagnostic_report(format='text')
            print("✓ Diagnostic report generated")
            
            print("\n✓ Diagnostics tests passed!")
            return True
        finally:
            # Clean up
            if os.path.exists(temp_db_path):
                os.unlink(temp_db_path)
        
    except Exception as e:
        print(f"\n✗ Diagnostics test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_cloud_sync():
    """Test cloud sync module"""
    print("\n" + "=" * 60)
    print("TEST 4: Cloud Sync")
    print("=" * 60)
    
    try:
        from config import CloudSyncConfig
        from database import GatewayDatabase
        from cloud_sync import CloudSyncService
        import tempfile
        import os
        
        # Use temporary file database
        temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        temp_db_path = temp_db.name
        temp_db.close()
        
        try:
            db = GatewayDatabase(temp_db_path)
            
            # Create config with sync disabled for testing
            sync_config = CloudSyncConfig(enabled=False)
            sync = CloudSyncService(sync_config, db)
            print("✓ Cloud sync service initialized (disabled for testing)")
            
            # Get stats
            stats = sync.get_stats()
            print(f"✓ Sync stats: {stats['syncs_attempted']} attempts, {stats['syncs_succeeded']} succeeded")
            
            print("\n✓ Cloud sync tests passed!")
            return True
        finally:
            # Clean up
            if os.path.exists(temp_db_path):
                os.unlink(temp_db_path)
        
    except Exception as e:
        print(f"\n✗ Cloud sync test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_api_server():
    """Test API server module"""
    print("\n" + "=" * 60)
    print("TEST 5: API Server")
    print("=" * 60)
    
    try:
        from config import APIConfig
        from database import GatewayDatabase
        from api_server import create_api_server
        import tempfile
        import os
        
        # Use temporary file database
        temp_db = tempfile.NamedTemporaryFile(delete=False, suffix='.db')
        temp_db_path = temp_db.name
        temp_db.close()
        
        try:
            db = GatewayDatabase(temp_db_path)
            
            # Add some test data
            db.add_or_update_node("0x12345678", node_name="Test Node")
            db.add_detection("0x12345678", species="Deer", confidence=0.95)
            
            # Create API server
            api_config = APIConfig(enabled=True)
            api = create_api_server(api_config, db)
            
            if api:
                print("✓ API server created")
            else:
                print("⚠ API server not created (Flask not available)")
            
            print("\n✓ API server tests passed!")
            return True
        finally:
            # Clean up
            if os.path.exists(temp_db_path):
                os.unlink(temp_db_path)
        
    except Exception as e:
        print(f"\n✗ API server test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    """Run all tests"""
    print("\n" + "=" * 60)
    print("WildCAM Gateway Module Tests")
    print("=" * 60)
    
    results = []
    
    results.append(("Imports", test_imports()))
    results.append(("Database", test_database()))
    results.append(("Diagnostics", test_diagnostics()))
    results.append(("Cloud Sync", test_cloud_sync()))
    results.append(("API Server", test_api_server()))
    
    # Print summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{name:20s} {status}")
    
    print("-" * 60)
    print(f"Total: {passed}/{total} tests passed")
    print("=" * 60 + "\n")
    
    return all(result for _, result in results)


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
