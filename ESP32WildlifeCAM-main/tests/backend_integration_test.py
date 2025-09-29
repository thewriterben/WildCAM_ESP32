#!/usr/bin/env python3
"""
Backend Integration Test Suite for ESP32WildlifeCAM

This test suite validates the backend components including:
- API endpoints and web services
- Database operations and data persistence
- Authentication and security features
- Data processing and analytics
- Integration with cloud services
"""

import unittest
import requests
import json
import time
import os
import sqlite3
from datetime import datetime, timedelta
from typing import Dict, List, Any

class BackendIntegrationTest(unittest.TestCase):
    """Comprehensive backend integration testing"""
    
    def setUp(self):
        """Set up test environment"""
        self.base_url = "http://localhost:80"  # ESP32 web server
        self.test_device_id = "TEST_DEVICE_001"
        self.api_key = "test_api_key_123"
        self.test_data = {
            "device_id": self.test_device_id,
            "timestamp": datetime.now().isoformat(),
            "battery_level": 85,
            "temperature": 23.5,
            "humidity": 65.2,
            "motion_detected": True,
            "species_detected": "deer",
            "confidence": 0.87
        }
        
        # Test database setup
        self.test_db_path = "/tmp/test_wildlife_cam.db"
        self.setup_test_database()
        
    def tearDown(self):
        """Clean up test environment"""
        if os.path.exists(self.test_db_path):
            os.remove(self.test_db_path)
    
    def setup_test_database(self):
        """Create test database with sample data"""
        conn = sqlite3.connect(self.test_db_path)
        cursor = conn.cursor()
        
        # Create test tables
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS wildlife_detections (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id TEXT NOT NULL,
                timestamp DATETIME NOT NULL,
                species TEXT,
                confidence REAL,
                image_path TEXT,
                environmental_data TEXT
            )
        ''')
        
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS device_status (
                device_id TEXT PRIMARY KEY,
                last_seen DATETIME,
                battery_level INTEGER,
                storage_usage REAL,
                firmware_version TEXT,
                status TEXT
            )
        ''')
        
        # Insert sample data
        sample_detections = [
            ("CAM_001", "2025-01-01 10:00:00", "deer", 0.92, "/images/deer_001.jpg", '{"temp": 18.5, "humidity": 70}'),
            ("CAM_001", "2025-01-01 14:30:00", "rabbit", 0.78, "/images/rabbit_001.jpg", '{"temp": 22.1, "humidity": 65}'),
            ("CAM_002", "2025-01-01 08:15:00", "bird", 0.85, "/images/bird_001.jpg", '{"temp": 16.8, "humidity": 72}'),
        ]
        
        cursor.executemany(
            'INSERT INTO wildlife_detections (device_id, timestamp, species, confidence, image_path, environmental_data) VALUES (?, ?, ?, ?, ?, ?)',
            sample_detections
        )
        
        conn.commit()
        conn.close()
    
    def test_api_health_check(self):
        """Test API health and availability"""
        print("\n🏥 Testing API Health Check")
        
        try:
            response = requests.get(f"{self.base_url}/api/health", timeout=10)
            self.assertEqual(response.status_code, 200)
            
            health_data = response.json()
            self.assertIn('status', health_data)
            self.assertIn('uptime', health_data)
            self.assertIn('memory_free', health_data)
            
            print(f"✅ API Health: {health_data['status']}")
            print(f"   Uptime: {health_data.get('uptime', 'N/A')} seconds")
            print(f"   Free Memory: {health_data.get('memory_free', 'N/A')} bytes")
            
        except requests.RequestException as e:
            self.skipTest(f"API not available: {e}")
    
    def test_device_status_endpoint(self):
        """Test device status API endpoint"""
        print("\n📱 Testing Device Status Endpoint")
        
        try:
            response = requests.get(f"{self.base_url}/api/status", timeout=10)
            self.assertEqual(response.status_code, 200)
            
            status_data = response.json()
            required_fields = ['device_id', 'battery', 'storage', 'last_capture']
            
            for field in required_fields:
                self.assertIn(field, status_data)
                print(f"✅ {field}: {status_data[field]}")
                
        except requests.RequestException as e:
            self.skipTest(f"Status endpoint not available: {e}")
    
    def test_image_capture_api(self):
        """Test image capture API endpoint"""
        print("\n📸 Testing Image Capture API")
        
        capture_request = {
            "quality": 10,
            "resolution": "UXGA",
            "enable_flash": False
        }
        
        try:
            response = requests.post(
                f"{self.base_url}/api/capture",
                json=capture_request,
                timeout=30
            )
            
            if response.status_code == 200:
                capture_result = response.json()
                self.assertIn('success', capture_result)
                self.assertIn('filename', capture_result)
                self.assertIn('size_bytes', capture_result)
                
                print(f"✅ Capture successful: {capture_result['filename']}")
                print(f"   Size: {capture_result['size_bytes']} bytes")
                
            elif response.status_code == 503:
                print("⚠️ Camera busy or unavailable")
                self.skipTest("Camera not available for testing")
            else:
                self.fail(f"Unexpected response: {response.status_code}")
                
        except requests.RequestException as e:
            self.skipTest(f"Capture endpoint not available: {e}")
    
    def test_configuration_management(self):
        """Test configuration API endpoints"""
        print("\n⚙️ Testing Configuration Management")
        
        # Test GET configuration
        try:
            response = requests.get(f"{self.base_url}/api/config", timeout=10)
            self.assertEqual(response.status_code, 200)
            
            config_data = response.json()
            print(f"✅ Configuration retrieved: {len(config_data)} settings")
            
            # Test configuration update
            update_config = {
                "motion_sensitivity": 75,
                "capture_interval": 300,
                "power_save_mode": True
            }
            
            response = requests.put(
                f"{self.base_url}/api/config",
                json=update_config,
                timeout=10
            )
            
            if response.status_code == 200:
                print("✅ Configuration updated successfully")
            else:
                print(f"⚠️ Configuration update failed: {response.status_code}")
                
        except requests.RequestException as e:
            self.skipTest(f"Configuration endpoint not available: {e}")
    
    def test_file_management_api(self):
        """Test file management API endpoints"""
        print("\n📁 Testing File Management API")
        
        try:
            # List files
            response = requests.get(f"{self.base_url}/api/files", timeout=10)
            self.assertEqual(response.status_code, 200)
            
            files_data = response.json()
            self.assertIn('files', files_data)
            self.assertIn('total_size', files_data)
            self.assertIn('free_space', files_data)
            
            print(f"✅ Files listed: {len(files_data['files'])} files")
            print(f"   Total size: {files_data['total_size']} bytes")
            print(f"   Free space: {files_data['free_space']} bytes")
            
            # Test file access (if files exist)
            if files_data['files']:
                first_file = files_data['files'][0]['name']
                response = requests.get(f"{self.base_url}/api/files/{first_file}", timeout=15)
                
                if response.status_code == 200:
                    print(f"✅ File access successful: {first_file}")
                else:
                    print(f"⚠️ File access failed: {response.status_code}")
                    
        except requests.RequestException as e:
            self.skipTest(f"File management endpoint not available: {e}")
    
    def test_websocket_streaming(self):
        """Test WebSocket streaming functionality"""
        print("\n🔄 Testing WebSocket Streaming")
        
        try:
            import websocket
            import threading
            
            messages_received = []
            connection_established = threading.Event()
            
            def on_message(ws, message):
                messages_received.append(json.loads(message))
                print(f"📨 Received: {message[:100]}...")
            
            def on_open(ws):
                connection_established.set()
                print("✅ WebSocket connection established")
            
            def on_error(ws, error):
                print(f"❌ WebSocket error: {error}")
            
            ws = websocket.WebSocketApp(
                f"ws://localhost:80/ws",
                on_message=on_message,
                on_open=on_open,
                on_error=on_error
            )
            
            # Start WebSocket in thread
            ws_thread = threading.Thread(target=ws.run_forever)
            ws_thread.daemon = True
            ws_thread.start()
            
            # Wait for connection
            if connection_established.wait(timeout=10):
                # Keep connection open briefly to receive messages
                time.sleep(5)
                ws.close()
                
                if messages_received:
                    print(f"✅ Received {len(messages_received)} WebSocket messages")
                else:
                    print("⚠️ No WebSocket messages received")
            else:
                self.skipTest("WebSocket connection could not be established")
                
        except ImportError:
            self.skipTest("websocket-client not available")
        except Exception as e:
            self.skipTest(f"WebSocket testing failed: {e}")
    
    def test_data_persistence(self):
        """Test data persistence and database operations"""
        print("\n💾 Testing Data Persistence")
        
        # Test database connection and basic operations
        conn = sqlite3.connect(self.test_db_path)
        cursor = conn.cursor()
        
        # Test data insertion
        test_detection = {
            'device_id': self.test_device_id,
            'timestamp': datetime.now().isoformat(),
            'species': 'test_species',
            'confidence': 0.95,
            'image_path': '/test/image.jpg',
            'environmental_data': json.dumps({'temp': 25.0, 'humidity': 60.0})
        }
        
        cursor.execute('''
            INSERT INTO wildlife_detections 
            (device_id, timestamp, species, confidence, image_path, environmental_data)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', tuple(test_detection.values()))
        
        conn.commit()
        print("✅ Test data inserted successfully")
        
        # Test data retrieval
        cursor.execute('''
            SELECT COUNT(*) FROM wildlife_detections 
            WHERE device_id = ?
        ''', (self.test_device_id,))
        
        count = cursor.fetchone()[0]
        self.assertGreater(count, 0)
        print(f"✅ Data retrieval successful: {count} records found")
        
        # Test data aggregation
        cursor.execute('''
            SELECT species, COUNT(*), AVG(confidence)
            FROM wildlife_detections
            GROUP BY species
        ''')
        
        species_stats = cursor.fetchall()
        print(f"✅ Data aggregation successful: {len(species_stats)} species")
        
        for species, count, avg_confidence in species_stats:
            print(f"   {species}: {count} detections, avg confidence: {avg_confidence:.2f}")
        
        conn.close()
    
    def test_analytics_processing(self):
        """Test analytics and data processing functionality"""
        print("\n📊 Testing Analytics Processing")
        
        # Test basic analytics calculations
        conn = sqlite3.connect(self.test_db_path)
        cursor = conn.cursor()
        
        # Activity patterns analysis
        cursor.execute('''
            SELECT strftime('%H', timestamp) as hour, COUNT(*) as detections
            FROM wildlife_detections
            GROUP BY hour
            ORDER BY hour
        ''')
        
        hourly_activity = cursor.fetchall()
        print(f"✅ Hourly activity analysis: {len(hourly_activity)} data points")
        
        # Species diversity analysis
        cursor.execute('''
            SELECT COUNT(DISTINCT species) as species_count,
                   AVG(confidence) as avg_confidence
            FROM wildlife_detections
        ''')
        
        diversity_stats = cursor.fetchone()
        print(f"✅ Species diversity: {diversity_stats[0]} species")
        print(f"   Average confidence: {diversity_stats[1]:.2f}")
        
        # Environmental correlation analysis
        cursor.execute('''
            SELECT environmental_data
            FROM wildlife_detections
            WHERE environmental_data IS NOT NULL
        ''')
        
        env_records = cursor.fetchall()
        if env_records:
            temp_values = []
            humidity_values = []
            
            for record in env_records:
                try:
                    env_data = json.loads(record[0])
                    if 'temp' in env_data:
                        temp_values.append(env_data['temp'])
                    if 'humidity' in env_data:
                        humidity_values.append(env_data['humidity'])
                except json.JSONDecodeError:
                    continue
            
            if temp_values:
                avg_temp = sum(temp_values) / len(temp_values)
                print(f"✅ Environmental analysis - Avg temp: {avg_temp:.1f}°C")
            
            if humidity_values:
                avg_humidity = sum(humidity_values) / len(humidity_values)
                print(f"   Avg humidity: {avg_humidity:.1f}%")
        
        conn.close()
    
    def test_authentication_security(self):
        """Test authentication and security features"""
        print("\n🔐 Testing Authentication & Security")
        
        # Test API without authentication (should fail for protected endpoints)
        try:
            response = requests.get(f"{self.base_url}/api/admin/settings", timeout=10)
            
            if response.status_code == 401:
                print("✅ Authentication required for protected endpoints")
            elif response.status_code == 404:
                print("⚠️ Admin endpoint not implemented")
            else:
                print(f"⚠️ Unexpected response for unauthenticated request: {response.status_code}")
                
        except requests.RequestException:
            print("⚠️ Admin endpoint not available")
        
        # Test API with authentication
        headers = {"Authorization": f"Bearer {self.api_key}"}
        
        try:
            response = requests.get(
                f"{self.base_url}/api/status",
                headers=headers,
                timeout=10
            )
            
            if response.status_code == 200:
                print("✅ Authenticated request successful")
            else:
                print(f"⚠️ Authenticated request failed: {response.status_code}")
                
        except requests.RequestException:
            print("⚠️ Authentication testing not available")
    
    def test_error_handling(self):
        """Test error handling and edge cases"""
        print("\n⚠️ Testing Error Handling")
        
        # Test invalid endpoints
        response = requests.get(f"{self.base_url}/api/nonexistent", timeout=10)
        self.assertEqual(response.status_code, 404)
        print("✅ 404 handling for invalid endpoints")
        
        # Test malformed requests
        try:
            response = requests.post(
                f"{self.base_url}/api/capture",
                data="invalid json",
                headers={"Content-Type": "application/json"},
                timeout=10
            )
            
            if response.status_code == 400:
                print("✅ 400 handling for malformed requests")
            else:
                print(f"⚠️ Unexpected response for malformed request: {response.status_code}")
                
        except requests.RequestException:
            print("⚠️ Error handling test not available")
        
        # Test rate limiting (if implemented)
        rapid_requests = 0
        for i in range(20):
            try:
                response = requests.get(f"{self.base_url}/api/status", timeout=5)
                if response.status_code == 429:  # Too Many Requests
                    print("✅ Rate limiting active")
                    break
                rapid_requests += 1
            except requests.RequestException:
                break
        
        if rapid_requests >= 19:
            print("⚠️ Rate limiting not implemented")
    
    def test_performance_benchmarks(self):
        """Test performance and response times"""
        print("\n⚡ Testing Performance Benchmarks")
        
        # Test API response times
        endpoints = [
            "/api/health",
            "/api/status",
            "/api/config",
            "/api/files"
        ]
        
        for endpoint in endpoints:
            start_time = time.time()
            try:
                response = requests.get(f"{self.base_url}{endpoint}", timeout=10)
                response_time = (time.time() - start_time) * 1000  # Convert to ms
                
                if response.status_code == 200:
                    print(f"✅ {endpoint}: {response_time:.1f}ms")
                    
                    # Performance thresholds
                    if response_time > 5000:  # 5 seconds
                        print(f"⚠️ Slow response time for {endpoint}")
                else:
                    print(f"❌ {endpoint}: HTTP {response.status_code}")
                    
            except requests.RequestException as e:
                print(f"❌ {endpoint}: {e}")
        
        # Test concurrent requests
        import concurrent.futures
        
        def make_request():
            try:
                start = time.time()
                response = requests.get(f"{self.base_url}/api/health", timeout=10)
                return time.time() - start, response.status_code
            except:
                return None, None
        
        print("\n🔄 Testing concurrent requests...")
        with concurrent.futures.ThreadPoolExecutor(max_workers=5) as executor:
            futures = [executor.submit(make_request) for _ in range(10)]
            results = [future.result() for future in concurrent.futures.as_completed(futures)]
        
        successful_requests = [r for r in results if r[1] == 200]
        if successful_requests:
            avg_time = sum(r[0] for r in successful_requests) / len(successful_requests) * 1000
            print(f"✅ Concurrent requests: {len(successful_requests)}/10 successful")
            print(f"   Average response time: {avg_time:.1f}ms")
        else:
            print("❌ No successful concurrent requests")


def run_backend_integration_tests():
    """Run all backend integration tests"""
    print("🚀 Starting Backend Integration Test Suite")
    print("=========================================")
    
    # Create test suite
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(BackendIntegrationTest)
    
    # Run tests with detailed output
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # Print summary
    print("\n📊 Test Summary")
    print("===============")
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Skipped: {len(result.skipped)}")
    
    if result.failures:
        print("\n❌ Failures:")
        for test, traceback in result.failures:
            print(f"  - {test}: {traceback.split('AssertionError: ')[-1].strip()}")
    
    if result.errors:
        print("\n⚠️ Errors:")
        for test, traceback in result.errors:
            print(f"  - {test}: {traceback.split('Exception: ')[-1].strip()}")
    
    if result.skipped:
        print("\n⏭️ Skipped:")
        for test, reason in result.skipped:
            print(f"  - {test}: {reason}")
    
    success_rate = ((result.testsRun - len(result.failures) - len(result.errors)) / result.testsRun * 100) if result.testsRun > 0 else 0
    print(f"\n🎯 Success Rate: {success_rate:.1f}%")
    
    return result.wasSuccessful()


if __name__ == "__main__":
    import sys
    
    # Check for dependencies
    try:
        import requests
        import websocket
    except ImportError as e:
        print(f"❌ Missing dependency: {e}")
        print("Install with: pip install requests websocket-client")
        sys.exit(1)
    
    # Run tests
    success = run_backend_integration_tests()
    sys.exit(0 if success else 1)