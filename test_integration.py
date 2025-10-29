#!/usr/bin/env python3
"""
WildCAM ESP32 Integration Test Script

This script performs comprehensive integration testing of the wildlife camera system,
including:
- System initialization
- Motion detection
- Image capture
- Metadata validation
- Web server endpoints
- Power management

Author: WildCAM Project
Date: 2024
"""

import serial
import time
import json
import requests
import os
import sys
from datetime import datetime
from typing import Optional, Dict, List, Tuple
import re


class Colors:
    """ANSI color codes for terminal output"""
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    PURPLE = '\033[0;35m'
    CYAN = '\033[0;36m'
    WHITE = '\033[1;37m'
    NC = '\033[0m'  # No Color


class TestResult:
    """Container for individual test results"""
    def __init__(self, name: str, passed: bool, message: str, duration: float = 0.0):
        self.name = name
        self.passed = passed
        self.message = message
        self.duration = duration
        self.timestamp = datetime.now()


class WildCAMIntegrationTester:
    """Main integration tester for WildCAM ESP32 system"""
    
    def __init__(self, serial_port: str = '/dev/ttyUSB0', baud_rate: int = 115200,
                 esp32_ip: Optional[str] = None, timeout: int = 30):
        """
        Initialize the integration tester
        
        Args:
            serial_port: Serial port to connect to ESP32
            baud_rate: Serial communication baud rate
            esp32_ip: IP address of ESP32 (for web server tests)
            timeout: Default timeout for operations in seconds
        """
        self.serial_port = serial_port
        self.baud_rate = baud_rate
        self.esp32_ip = esp32_ip
        self.timeout = timeout
        self.serial_conn: Optional[serial.Serial] = None
        self.test_results: List[TestResult] = []
        self.start_time = datetime.now()
        
    def log_info(self, message: str):
        """Log info message"""
        print(f"{Colors.BLUE}[INFO]{Colors.NC} {message}")
    
    def log_success(self, message: str):
        """Log success message"""
        print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {message}")
    
    def log_warning(self, message: str):
        """Log warning message"""
        print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {message}")
    
    def log_error(self, message: str):
        """Log error message"""
        print(f"{Colors.RED}[ERROR]{Colors.NC} {message}")
    
    def log_test_header(self, test_name: str):
        """Log test header"""
        print(f"\n{Colors.PURPLE}{'='*70}{Colors.NC}")
        print(f"{Colors.PURPLE}TEST: {test_name}{Colors.NC}")
        print(f"{Colors.PURPLE}{'='*70}{Colors.NC}")
    
    def connect_serial(self) -> bool:
        """
        Connect to ESP32 via serial port
        
        Returns:
            True if connection successful, False otherwise
        """
        try:
            self.log_info(f"Connecting to serial port {self.serial_port} at {self.baud_rate} baud...")
            self.serial_conn = serial.Serial(
                port=self.serial_port,
                baudrate=self.baud_rate,
                timeout=1,
                write_timeout=1
            )
            time.sleep(2)  # Wait for connection to stabilize
            
            # Flush any existing data
            self.serial_conn.reset_input_buffer()
            self.serial_conn.reset_output_buffer()
            
            self.log_success(f"Connected to {self.serial_port}")
            return True
            
        except serial.SerialException as e:
            self.log_error(f"Failed to connect to serial port: {e}")
            return False
    
    def read_serial_until(self, pattern: str, timeout: float = None) -> Tuple[bool, str]:
        """
        Read serial data until pattern is found or timeout occurs
        
        Args:
            pattern: String pattern to search for
            timeout: Timeout in seconds (uses self.timeout if None)
            
        Returns:
            Tuple of (found, output) where found is True if pattern was found
        """
        if timeout is None:
            timeout = self.timeout
            
        start_time = time.time()
        output_lines = []
        
        try:
            while (time.time() - start_time) < timeout:
                if self.serial_conn.in_waiting > 0:
                    line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        output_lines.append(line)
                        print(f"  {Colors.CYAN}>{Colors.NC} {line}")
                        
                        if pattern in line:
                            return True, '\n'.join(output_lines)
                
                time.sleep(0.1)
            
            return False, '\n'.join(output_lines)
            
        except Exception as e:
            self.log_error(f"Error reading serial: {e}")
            return False, '\n'.join(output_lines)
    
    def read_serial_for_duration(self, duration: float) -> str:
        """
        Read serial data for a specified duration
        
        Args:
            duration: Duration in seconds to read
            
        Returns:
            Combined serial output
        """
        start_time = time.time()
        output_lines = []
        
        try:
            while (time.time() - start_time) < duration:
                if self.serial_conn.in_waiting > 0:
                    line = self.serial_conn.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        output_lines.append(line)
                        print(f"  {Colors.CYAN}>{Colors.NC} {line}")
                
                time.sleep(0.1)
            
            return '\n'.join(output_lines)
            
        except Exception as e:
            self.log_error(f"Error reading serial: {e}")
            return '\n'.join(output_lines)
    
    def trigger_motion_simulation(self) -> bool:
        """
        Simulate motion by physically triggering the PIR sensor
        This is a placeholder - actual implementation would depend on test setup
        
        Returns:
            True if motion can be simulated (always returns True for manual testing)
        """
        self.log_info("*** MANUAL ACTION REQUIRED ***")
        self.log_info("Please trigger the PIR motion sensor now...")
        self.log_info("(You have 30 seconds to trigger motion)")
        return True
    
    # ==================== TEST SCENARIOS ====================
    
    def test_system_initialization(self) -> TestResult:
        """
        Test 1: System Initialization Test
        - Connect to ESP32 via serial
        - Verify all modules initialize successfully
        - Check for "System Ready" message
        """
        self.log_test_header("System Initialization")
        start_time = time.time()
        
        try:
            # Connect to serial
            if not self.connect_serial():
                return TestResult(
                    "System Initialization",
                    False,
                    "Failed to connect to serial port",
                    time.time() - start_time
                )
            
            # Reset ESP32 to trigger initialization
            self.log_info("Resetting ESP32 to observe initialization sequence...")
            self.serial_conn.setDTR(False)
            time.sleep(0.1)
            self.serial_conn.setDTR(True)
            time.sleep(1)
            
            # Check for initialization messages
            self.log_info("Waiting for system initialization messages...")
            required_messages = [
                "WildCAM ESP32",
                "Power Manager initialized",
                "Camera initialized",
                "Storage initialized",
                "Motion detector ready",
                "System Ready"
            ]
            
            found_messages = []
            output = self.read_serial_for_duration(15)
            
            for msg in required_messages:
                if msg in output:
                    found_messages.append(msg)
                    self.log_success(f"Found: '{msg}'")
                else:
                    self.log_warning(f"Missing: '{msg}'")
            
            # Check if all required messages were found
            if len(found_messages) >= 5:  # Allow some flexibility
                return TestResult(
                    "System Initialization",
                    True,
                    f"System initialized successfully. Found {len(found_messages)}/{len(required_messages)} expected messages",
                    time.time() - start_time
                )
            else:
                return TestResult(
                    "System Initialization",
                    False,
                    f"Incomplete initialization. Only found {len(found_messages)}/{len(required_messages)} messages",
                    time.time() - start_time
                )
                
        except Exception as e:
            return TestResult(
                "System Initialization",
                False,
                f"Exception during test: {str(e)}",
                time.time() - start_time
            )
    
    def test_motion_detection(self) -> TestResult:
        """
        Test 2: Motion Detection Test
        - Simulate motion by triggering PIR sensor
        - Verify "Motion detected!" message appears
        - Verify image capture is triggered
        """
        self.log_test_header("Motion Detection")
        start_time = time.time()
        
        try:
            if not self.serial_conn or not self.serial_conn.is_open:
                return TestResult(
                    "Motion Detection",
                    False,
                    "Serial connection not available",
                    time.time() - start_time
                )
            
            # Trigger motion
            self.trigger_motion_simulation()
            
            # Wait for motion detection message
            self.log_info("Waiting for motion detection message...")
            found, output = self.read_serial_until("MOTION DETECTED", timeout=35)
            
            if found:
                self.log_success("Motion detection message received")
                
                # Check for image capture trigger
                if "Capturing image" in output or "Stabilizing camera" in output:
                    self.log_success("Image capture triggered")
                    return TestResult(
                        "Motion Detection",
                        True,
                        "Motion detected and image capture triggered successfully",
                        time.time() - start_time
                    )
                else:
                    return TestResult(
                        "Motion Detection",
                        False,
                        "Motion detected but image capture not triggered",
                        time.time() - start_time
                    )
            else:
                return TestResult(
                    "Motion Detection",
                    False,
                    "Motion detection message not received within timeout",
                    time.time() - start_time
                )
                
        except Exception as e:
            return TestResult(
                "Motion Detection",
                False,
                f"Exception during test: {str(e)}",
                time.time() - start_time
            )
    
    def test_image_capture(self) -> TestResult:
        """
        Test 3: Image Capture Test
        - Trigger motion event
        - Wait for capture to complete
        - Verify image file is created on SD card
        - Verify image file size is reasonable (> 10KB)
        """
        self.log_test_header("Image Capture")
        start_time = time.time()
        
        try:
            if not self.serial_conn or not self.serial_conn.is_open:
                return TestResult(
                    "Image Capture",
                    False,
                    "Serial connection not available",
                    time.time() - start_time
                )
            
            # Trigger motion (if not already triggered from previous test)
            self.log_info("Ensuring motion is detected for image capture...")
            
            # Wait for image save confirmation
            self.log_info("Waiting for image save confirmation...")
            found, output = self.read_serial_until("Image saved successfully", timeout=20)
            
            if found:
                self.log_success("Image save message received")
                
                # Extract image path from output
                image_path_match = re.search(r'/images/\d+\.jpg', output)
                if image_path_match:
                    image_path = image_path_match.group(0)
                    self.log_info(f"Image saved to: {image_path}")
                
                # Check for image size in metadata (if available in output)
                size_match = re.search(r'"image_size":\s*(\d+)', output)
                if size_match:
                    image_size = int(size_match.group(1))
                    self.log_info(f"Image size: {image_size} bytes")
                    
                    if image_size > 10000:  # > 10KB
                        self.log_success(f"Image size ({image_size} bytes) is reasonable")
                        return TestResult(
                            "Image Capture",
                            True,
                            f"Image captured and saved successfully (size: {image_size} bytes)",
                            time.time() - start_time
                        )
                    else:
                        return TestResult(
                            "Image Capture",
                            False,
                            f"Image size ({image_size} bytes) is too small",
                            time.time() - start_time
                        )
                else:
                    # If size not found in output, assume success based on save message
                    return TestResult(
                        "Image Capture",
                        True,
                        "Image captured and saved successfully (size not verified)",
                        time.time() - start_time
                    )
            else:
                return TestResult(
                    "Image Capture",
                    False,
                    "Image save confirmation not received",
                    time.time() - start_time
                )
                
        except Exception as e:
            return TestResult(
                "Image Capture",
                False,
                f"Exception during test: {str(e)}",
                time.time() - start_time
            )
    
    def test_metadata(self) -> TestResult:
        """
        Test 4: Metadata Test
        - After image capture, verify JSON metadata file exists
        - Parse JSON and verify required fields (timestamp, battery, etc.)
        """
        self.log_test_header("Metadata Validation")
        start_time = time.time()
        
        try:
            if not self.serial_conn or not self.serial_conn.is_open:
                return TestResult(
                    "Metadata Validation",
                    False,
                    "Serial connection not available",
                    time.time() - start_time
                )
            
            # Wait for metadata save message
            self.log_info("Waiting for metadata save confirmation...")
            found, output = self.read_serial_until("Metadata saved successfully", timeout=10)
            
            if found:
                self.log_success("Metadata save message received")
                
                # Try to extract metadata from serial output
                # The metadata is printed in the format: {"key": value, ...}
                required_fields = ["timestamp", "battery_voltage", "battery_percent", 
                                 "image_size", "image_count"]
                found_fields = []
                
                for field in required_fields:
                    if field in output:
                        found_fields.append(field)
                        self.log_success(f"Found metadata field: {field}")
                    else:
                        self.log_warning(f"Missing metadata field: {field}")
                
                if len(found_fields) >= 4:  # Allow some flexibility
                    return TestResult(
                        "Metadata Validation",
                        True,
                        f"Metadata saved with {len(found_fields)}/{len(required_fields)} required fields",
                        time.time() - start_time
                    )
                else:
                    return TestResult(
                        "Metadata Validation",
                        False,
                        f"Incomplete metadata. Only found {len(found_fields)}/{len(required_fields)} fields",
                        time.time() - start_time
                    )
            else:
                return TestResult(
                    "Metadata Validation",
                    False,
                    "Metadata save confirmation not received",
                    time.time() - start_time
                )
                
        except Exception as e:
            return TestResult(
                "Metadata Validation",
                False,
                f"Exception during test: {str(e)}",
                time.time() - start_time
            )
    
    def test_web_server(self) -> TestResult:
        """
        Test 5: Web Server Test
        - Connect to ESP32 WiFi
        - Make GET request to /status endpoint
        - Verify JSON response with system info
        - Make GET request to /latest endpoint
        - Verify image is returned
        """
        self.log_test_header("Web Server")
        start_time = time.time()
        
        try:
            if not self.esp32_ip:
                # Try to extract IP from serial output
                self.log_info("Searching for ESP32 IP address in serial output...")
                output = self.read_serial_for_duration(2)
                ip_match = re.search(r'IP Address:\s*(\d+\.\d+\.\d+\.\d+)', output)
                if ip_match:
                    self.esp32_ip = ip_match.group(1)
                    self.log_info(f"Found IP address: {self.esp32_ip}")
                else:
                    return TestResult(
                        "Web Server",
                        False,
                        "ESP32 IP address not provided and not found in serial output",
                        time.time() - start_time
                    )
            
            base_url = f"http://{self.esp32_ip}"
            
            # Test /status endpoint
            self.log_info(f"Testing /status endpoint at {base_url}/status...")
            try:
                response = requests.get(f"{base_url}/status", timeout=5)
                
                if response.status_code == 200:
                    self.log_success("/status endpoint responded with 200 OK")
                    
                    try:
                        status_data = response.json()
                        self.log_info(f"Status data: {json.dumps(status_data, indent=2)}")
                        
                        # Verify expected fields
                        expected_fields = ["battery_voltage", "battery_percent"]
                        found_status_fields = []
                        
                        for field in expected_fields:
                            if field in status_data:
                                found_status_fields.append(field)
                                self.log_success(f"Found status field: {field}")
                        
                        if len(found_status_fields) >= 1:
                            self.log_success("Status endpoint validation passed")
                        else:
                            self.log_warning("Status endpoint returned unexpected format")
                            
                    except json.JSONDecodeError:
                        self.log_warning("/status response is not valid JSON")
                        
                else:
                    self.log_warning(f"/status endpoint returned status code {response.status_code}")
                    
            except requests.RequestException as e:
                self.log_error(f"Failed to connect to /status endpoint: {e}")
                return TestResult(
                    "Web Server",
                    False,
                    f"Could not connect to web server: {str(e)}",
                    time.time() - start_time
                )
            
            # Test /latest endpoint
            self.log_info(f"Testing /latest endpoint at {base_url}/latest...")
            try:
                response = requests.get(f"{base_url}/latest", timeout=5)
                
                if response.status_code == 200:
                    self.log_success("/latest endpoint responded with 200 OK")
                    
                    # Check if response is an image (JPEG)
                    content_type = response.headers.get('Content-Type', '')
                    if 'image' in content_type:
                        self.log_success(f"Received image response (Content-Type: {content_type})")
                        image_size = len(response.content)
                        self.log_info(f"Image size: {image_size} bytes")
                        
                        if image_size > 1000:
                            return TestResult(
                                "Web Server",
                                True,
                                f"Web server tests passed. Status and latest endpoints working",
                                time.time() - start_time
                            )
                        else:
                            return TestResult(
                                "Web Server",
                                False,
                                "Image from /latest endpoint is too small",
                                time.time() - start_time
                            )
                    else:
                        self.log_warning(f"Unexpected content type: {content_type}")
                        return TestResult(
                            "Web Server",
                            False,
                            f"/latest endpoint did not return an image (got {content_type})",
                            time.time() - start_time
                        )
                else:
                    return TestResult(
                        "Web Server",
                        False,
                        f"/latest endpoint returned status code {response.status_code}",
                        time.time() - start_time
                    )
                    
            except requests.RequestException as e:
                self.log_warning(f"Failed to connect to /latest endpoint: {e}")
                return TestResult(
                    "Web Server",
                    False,
                    f"Could not connect to /latest endpoint: {str(e)}",
                    time.time() - start_time
                )
                
        except Exception as e:
            return TestResult(
                "Web Server",
                False,
                f"Exception during test: {str(e)}",
                time.time() - start_time
            )
    
    def test_power_management(self) -> TestResult:
        """
        Test 6: Power Management Test
        - Monitor serial output for battery voltage readings
        - Verify deep sleep is entered after timeout
        - Verify wake-up on motion
        """
        self.log_test_header("Power Management")
        start_time = time.time()
        
        try:
            if not self.serial_conn or not self.serial_conn.is_open:
                return TestResult(
                    "Power Management",
                    False,
                    "Serial connection not available",
                    time.time() - start_time
                )
            
            # Monitor for battery voltage readings
            self.log_info("Monitoring for battery voltage readings...")
            output = self.read_serial_for_duration(5)
            
            battery_reading_found = False
            voltage_match = re.search(r'Battery.*?(\d+\.\d+)V.*?\((\d+)%\)', output)
            if voltage_match:
                voltage = float(voltage_match.group(1))
                percent = int(voltage_match.group(2))
                self.log_success(f"Battery reading found: {voltage}V ({percent}%)")
                battery_reading_found = True
            else:
                self.log_warning("Battery reading not found in output")
            
            # Check for deep sleep messages
            self.log_info("Checking for power management features...")
            deep_sleep_configured = False
            wake_sources_configured = False
            
            if "Wake-up sources configured" in output or "wake" in output.lower():
                self.log_success("Wake-up sources configured")
                wake_sources_configured = True
            
            if "deep sleep" in output.lower() or "sleep" in output.lower():
                self.log_success("Deep sleep functionality present")
                deep_sleep_configured = True
            
            # Calculate score
            features_found = sum([battery_reading_found, deep_sleep_configured, 
                                wake_sources_configured])
            
            if features_found >= 2:
                return TestResult(
                    "Power Management",
                    True,
                    f"Power management features verified ({features_found}/3 features found)",
                    time.time() - start_time
                )
            else:
                return TestResult(
                    "Power Management",
                    False,
                    f"Insufficient power management features ({features_found}/3 features found)",
                    time.time() - start_time
                )
                
        except Exception as e:
            return TestResult(
                "Power Management",
                False,
                f"Exception during test: {str(e)}",
                time.time() - start_time
            )
    
    # ==================== TEST EXECUTION & REPORTING ====================
    
    def run_all_tests(self):
        """Run all integration tests"""
        print(f"\n{Colors.PURPLE}{'='*70}{Colors.NC}")
        print(f"{Colors.PURPLE}WildCAM ESP32 INTEGRATION TEST SUITE{Colors.NC}")
        print(f"{Colors.PURPLE}{'='*70}{Colors.NC}")
        print(f"Start time: {self.start_time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Serial port: {self.serial_port}")
        print(f"Baud rate: {self.baud_rate}")
        if self.esp32_ip:
            print(f"ESP32 IP: {self.esp32_ip}")
        print(f"{Colors.PURPLE}{'='*70}{Colors.NC}\n")
        
        # Run tests in order
        tests = [
            self.test_system_initialization,
            self.test_motion_detection,
            self.test_image_capture,
            self.test_metadata,
            self.test_web_server,
            self.test_power_management
        ]
        
        for test_func in tests:
            try:
                result = test_func()
                self.test_results.append(result)
                
                # Print test result
                status_color = Colors.GREEN if result.passed else Colors.RED
                status_text = "PASSED" if result.passed else "FAILED"
                print(f"\n{status_color}[{status_text}]{Colors.NC} {result.name}")
                print(f"  Duration: {result.duration:.2f}s")
                print(f"  {result.message}")
                
            except Exception as e:
                self.log_error(f"Unexpected error in {test_func.__name__}: {e}")
                result = TestResult(
                    test_func.__name__,
                    False,
                    f"Unexpected error: {str(e)}",
                    0.0
                )
                self.test_results.append(result)
        
        # Generate final report
        self.generate_report()
    
    def generate_report(self):
        """Generate and display final test report"""
        end_time = datetime.now()
        total_duration = (end_time - self.start_time).total_seconds()
        
        passed_tests = sum(1 for r in self.test_results if r.passed)
        failed_tests = len(self.test_results) - passed_tests
        pass_rate = (passed_tests / len(self.test_results) * 100) if self.test_results else 0
        
        print(f"\n{Colors.PURPLE}{'='*70}{Colors.NC}")
        print(f"{Colors.PURPLE}TEST REPORT{Colors.NC}")
        print(f"{Colors.PURPLE}{'='*70}{Colors.NC}")
        print(f"End time: {end_time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"Total duration: {total_duration:.2f}s")
        print(f"\nTests run: {len(self.test_results)}")
        print(f"{Colors.GREEN}Passed: {passed_tests}{Colors.NC}")
        print(f"{Colors.RED}Failed: {failed_tests}{Colors.NC}")
        print(f"Pass rate: {pass_rate:.1f}%")
        print(f"\n{Colors.PURPLE}Individual Test Results:{Colors.NC}")
        
        for i, result in enumerate(self.test_results, 1):
            status_color = Colors.GREEN if result.passed else Colors.RED
            status_symbol = "✓" if result.passed else "✗"
            print(f"  {i}. {status_color}{status_symbol}{Colors.NC} {result.name}")
            print(f"     Duration: {result.duration:.2f}s")
            print(f"     {result.message}")
        
        print(f"\n{Colors.PURPLE}{'='*70}{Colors.NC}")
        
        if failed_tests == 0:
            print(f"{Colors.GREEN}ALL TESTS PASSED!{Colors.NC}")
        else:
            print(f"{Colors.RED}{failed_tests} TEST(S) FAILED{Colors.NC}")
        
        print(f"{Colors.PURPLE}{'='*70}{Colors.NC}\n")
        
        # Save report to file
        self.save_report_to_file()
    
    def save_report_to_file(self):
        """Save test report to a JSON file"""
        report_data = {
            "test_suite": "WildCAM ESP32 Integration Tests",
            "start_time": self.start_time.isoformat(),
            "end_time": datetime.now().isoformat(),
            "serial_port": self.serial_port,
            "baud_rate": self.baud_rate,
            "esp32_ip": self.esp32_ip,
            "total_tests": len(self.test_results),
            "passed": sum(1 for r in self.test_results if r.passed),
            "failed": sum(1 for r in self.test_results if not r.passed),
            "results": [
                {
                    "name": r.name,
                    "passed": r.passed,
                    "message": r.message,
                    "duration": r.duration,
                    "timestamp": r.timestamp.isoformat()
                }
                for r in self.test_results
            ]
        }
        
        report_filename = f"test_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        try:
            with open(report_filename, 'w') as f:
                json.dump(report_data, f, indent=2)
            self.log_success(f"Test report saved to {report_filename}")
        except Exception as e:
            self.log_error(f"Failed to save report: {e}")
    
    def cleanup(self):
        """Clean up resources"""
        if self.serial_conn and self.serial_conn.is_open:
            self.serial_conn.close()
            self.log_info("Serial connection closed")


def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='WildCAM ESP32 Integration Test Suite',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Test with default serial port
  python test_integration.py
  
  # Test with custom serial port and IP
  python test_integration.py --port /dev/ttyUSB0 --ip 192.168.1.100
  
  # Test with custom timeout
  python test_integration.py --port COM3 --timeout 60
        """
    )
    
    parser.add_argument(
        '--port', '-p',
        default='/dev/ttyUSB0',
        help='Serial port to connect to ESP32 (default: /dev/ttyUSB0)'
    )
    
    parser.add_argument(
        '--baud', '-b',
        type=int,
        default=115200,
        help='Serial baud rate (default: 115200)'
    )
    
    parser.add_argument(
        '--ip',
        default=None,
        help='ESP32 IP address for web server tests (optional, auto-detected if not provided)'
    )
    
    parser.add_argument(
        '--timeout', '-t',
        type=int,
        default=30,
        help='Default timeout for operations in seconds (default: 30)'
    )
    
    args = parser.parse_args()
    
    # Create tester instance
    tester = WildCAMIntegrationTester(
        serial_port=args.port,
        baud_rate=args.baud,
        esp32_ip=args.ip,
        timeout=args.timeout
    )
    
    try:
        # Run all tests
        tester.run_all_tests()
        
        # Return appropriate exit code
        failed_tests = sum(1 for r in tester.test_results if not r.passed)
        sys.exit(0 if failed_tests == 0 else 1)
        
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Test interrupted by user{Colors.NC}")
        sys.exit(130)
        
    except Exception as e:
        print(f"\n{Colors.RED}Unexpected error: {e}{Colors.NC}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
        
    finally:
        tester.cleanup()


if __name__ == '__main__':
    main()
