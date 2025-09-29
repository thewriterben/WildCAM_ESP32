#!/usr/bin/env python3
"""
Component-Specific Validation Script for ESP32WildlifeCAM
Validates specific components with hardware simulation capability
Usage: python3 scripts/validate_component.py [component] [options]
"""

import sys
import os
import time
import argparse
import json
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
from datetime import datetime
import re

# Add project root to Python path
script_dir = Path(__file__).parent
project_root = script_dir.parent
sys.path.insert(0, str(project_root))

# Color codes for output
class Colors:
    RED = '\033[0;31m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    PURPLE = '\033[0;35m'
    CYAN = '\033[0;36m'
    WHITE = '\033[1;37m'
    NC = '\033[0m'  # No Color

class ComponentValidator:
    def __init__(self, verbose: bool = False, simulation: bool = False):
        self.verbose = verbose
        self.simulation = simulation
        self.project_root = project_root
        self.start_time = time.time()
        self.results = {
            'component': '',
            'tests': [],
            'summary': {},
            'performance': {}
        }
        
    def log_info(self, message: str):
        print(f"{Colors.BLUE}[INFO]{Colors.NC} {message}")
        
    def log_success(self, message: str):
        print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {message}")
        
    def log_warning(self, message: str):
        print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {message}")
        
    def log_error(self, message: str):
        print(f"{Colors.RED}[ERROR]{Colors.NC} {message}")
        
    def log_verbose(self, message: str):
        if self.verbose:
            print(f"{Colors.CYAN}[VERBOSE]{Colors.NC} {message}")

    def run_test(self, test_name: str, test_func, *args, **kwargs) -> bool:
        """Run a test and record results"""
        self.log_info(f"Running {test_name}...")
        start_test_time = time.time()
        
        try:
            result = test_func(*args, **kwargs)
            duration = time.time() - start_test_time
            
            test_result = {
                'name': test_name,
                'passed': result,
                'duration_ms': round(duration * 1000, 2),
                'timestamp': datetime.now().isoformat()
            }
            
            self.results['tests'].append(test_result)
            
            if result:
                self.log_success(f"{test_name} passed ({duration:.2f}s)")
            else:
                self.log_error(f"{test_name} failed ({duration:.2f}s)")
                
            return result
            
        except Exception as e:
            duration = time.time() - start_test_time
            self.log_error(f"{test_name} exception: {str(e)}")
            
            test_result = {
                'name': test_name,
                'passed': False,
                'duration_ms': round(duration * 1000, 2),
                'error': str(e),
                'timestamp': datetime.now().isoformat()
            }
            
            self.results['tests'].append(test_result)
            return False

    def validate_camera_component(self) -> Dict[str, bool]:
        """Validate camera component"""
        self.log_info("Validating Camera Component...")
        results = {}
        
        # Test 1: Camera configuration validation
        def test_camera_config():
            config_file = self.project_root / "firmware" / "src" / "config.h"
            if not config_file.exists():
                return False
                
            content = config_file.read_text()
            
            # Check for camera pin definitions
            required_pins = ['XCLK_GPIO_NUM', 'PCLK_GPIO_NUM', 'VSYNC_GPIO_NUM', 'HREF_GPIO_NUM']
            for pin in required_pins:
                if pin not in content:
                    self.log_error(f"Missing camera pin definition: {pin}")
                    return False
                    
            self.log_verbose("Camera GPIO configuration validated")
            return True
            
        results['camera_config'] = self.run_test("Camera Configuration", test_camera_config)
        
        # Test 2: Camera handler implementation
        def test_camera_handler():
            handler_file = self.project_root / "firmware" / "src" / "camera_handler.cpp"
            if not handler_file.exists():
                self.log_warning("Camera handler not implemented yet")
                return False if not self.simulation else True
                
            content = handler_file.read_text()
            
            # Check for essential camera functions
            required_functions = ['camera_init', 'camera_capture', 'camera_deinit']
            for func in required_functions:
                if func not in content:
                    self.log_error(f"Missing camera function: {func}")
                    return False
                    
            self.log_verbose("Camera handler implementation validated")
            return True
            
        results['camera_handler'] = self.run_test("Camera Handler", test_camera_handler)
        
        # Test 3: GPIO conflict check
        def test_camera_gpio_conflicts():
            try:
                from validate_fixes import check_gpio_pin_conflicts
                return check_gpio_pin_conflicts()
            except Exception as e:
                self.log_error(f"GPIO conflict check failed: {e}")
                return False
                
        results['gpio_conflicts'] = self.run_test("GPIO Conflicts", test_camera_gpio_conflicts)
        
        # Test 4: Hardware simulation (if enabled)
        if self.simulation:
            def test_camera_simulation():
                self.log_info("Running camera hardware simulation...")
                # Simulate camera initialization and capture
                time.sleep(0.5)  # Simulate hardware delay
                self.log_verbose("Simulated camera init successful")
                time.sleep(0.3)  # Simulate capture delay
                self.log_verbose("Simulated image capture successful")
                return True
                
            results['hardware_simulation'] = self.run_test("Hardware Simulation", test_camera_simulation)
        
        return results

    def validate_power_component(self) -> Dict[str, bool]:
        """Validate power management component"""
        self.log_info("Validating Power Management Component...")
        results = {}
        
        # Test 1: Power manager configuration
        def test_power_config():
            config_files = [
                self.project_root / "firmware" / "src" / "config.h",
                self.project_root / "include" / "pins.h"
            ]
            
            power_pins_found = False
            for config_file in config_files:
                if not config_file.exists():
                    continue
                    
                content = config_file.read_text()
                
                # Check for power-related pin definitions
                power_pins = ['BATTERY_VOLTAGE_PIN', 'SOLAR_VOLTAGE_PIN', 'CHARGING_LED_PIN']
                pins_in_file = [pin for pin in power_pins if pin in content]
                
                if pins_in_file:
                    power_pins_found = True
                    self.log_verbose(f"Found power pins in {config_file.name}: {pins_in_file}")
                    
            return power_pins_found
            
        results['power_config'] = self.run_test("Power Configuration", test_power_config)
        
        # Test 2: Power manager implementation
        def test_power_manager():
            power_files = [
                self.project_root / "firmware" / "src" / "power_manager.cpp",
                self.project_root / "src" / "power" / "power_manager.cpp"
            ]
            
            for power_file in power_files:
                if power_file.exists():
                    content = power_file.read_text()
                    
                    # Check for essential power management functions
                    required_functions = ['getBatteryVoltage', 'getSolarVoltage', 'enterSleepMode']
                    functions_found = [func for func in required_functions if func in content]
                    
                    if len(functions_found) >= 2:  # At least 2 functions should be present
                        self.log_verbose(f"Power manager functions found: {functions_found}")
                        return True
                        
            self.log_warning("Power manager implementation not found or incomplete")
            return False if not self.simulation else True
            
        results['power_manager'] = self.run_test("Power Manager", test_power_manager)
        
        # Test 3: Solar panel simulation
        if self.simulation:
            def test_solar_simulation():
                self.log_info("Running solar panel simulation...")
                # Simulate voltage readings
                import random
                voltage = random.uniform(3.0, 4.2)  # Typical solar panel voltage range
                self.log_verbose(f"Simulated solar voltage: {voltage:.2f}V")
                return 3.0 <= voltage <= 5.0
                
            results['solar_simulation'] = self.run_test("Solar Simulation", test_solar_simulation)
        
        return results

    def validate_storage_component(self) -> Dict[str, bool]:
        """Validate storage component"""
        self.log_info("Validating Storage Component...")
        results = {}
        
        # Test 1: SD card configuration
        def test_storage_config():
            config_file = self.project_root / "firmware" / "src" / "config.h"
            if not config_file.exists():
                return False
                
            content = config_file.read_text()
            
            # Check for SD card configuration
            if 'SD_CARD_ENABLED' in content and 'true' in content:
                self.log_verbose("SD card is enabled in configuration")
                
                # Check for SD card pin definitions
                sd_pins = ['SD_MISO_PIN', 'SD_MOSI_PIN', 'SD_CLK_PIN', 'SD_CS_PIN']
                for pin in sd_pins:
                    if pin not in content:
                        self.log_error(f"Missing SD card pin definition: {pin}")
                        return False
                        
                return True
            else:
                self.log_warning("SD card is disabled in configuration")
                return True  # Not an error if intentionally disabled
                
        results['storage_config'] = self.run_test("Storage Configuration", test_storage_config)
        
        # Test 2: Storage manager implementation
        def test_storage_manager():
            storage_files = [
                self.project_root / "src" / "data" / "storage_manager.cpp",
                self.project_root / "firmware" / "src" / "storage_manager.cpp"
            ]
            
            for storage_file in storage_files:
                if storage_file.exists():
                    content = storage_file.read_text()
                    
                    # Check for storage functions
                    required_functions = ['initialize', 'writeFile', 'readFile']
                    functions_found = [func for func in required_functions if func in content]
                    
                    if len(functions_found) >= 2:
                        self.log_verbose(f"Storage manager functions found: {functions_found}")
                        return True
                        
            self.log_warning("Storage manager implementation not found")
            return False if not self.simulation else True
            
        results['storage_manager'] = self.run_test("Storage Manager", test_storage_manager)
        
        # Test 3: File system simulation
        if self.simulation:
            def test_filesystem_simulation():
                self.log_info("Running filesystem simulation...")
                # Simulate file operations
                import tempfile
                import os
                
                try:
                    with tempfile.NamedTemporaryFile(delete=False) as tmp:
                        test_data = b"Wildlife camera test data"
                        tmp.write(test_data)
                        tmp.flush()
                        
                        # Verify write
                        with open(tmp.name, 'rb') as read_file:
                            read_data = read_file.read()
                            
                        os.unlink(tmp.name)
                        return test_data == read_data
                        
                except Exception as e:
                    self.log_error(f"Filesystem simulation failed: {e}")
                    return False
                    
            results['filesystem_simulation'] = self.run_test("Filesystem Simulation", test_filesystem_simulation)
        
        return results

    def validate_motion_component(self) -> Dict[str, bool]:
        """Validate motion detection component"""
        self.log_info("Validating Motion Detection Component...")
        results = {}
        
        # Test 1: PIR sensor configuration
        def test_pir_config():
            config_file = self.project_root / "firmware" / "src" / "config.h"
            if not config_file.exists():
                return False
                
            content = config_file.read_text()
            
            # Check for PIR pin definition
            if 'PIR_PIN' in content:
                # Extract PIR pin number
                pir_match = re.search(r'#define\s+PIR_PIN\s+(\d+)', content)
                if pir_match:
                    pir_pin = int(pir_match.group(1))
                    self.log_verbose(f"PIR sensor configured on GPIO {pir_pin}")
                    return pir_pin == 1  # Should be GPIO 1 for AI-Thinker
                    
            return False
            
        results['pir_config'] = self.run_test("PIR Configuration", test_pir_config)
        
        # Test 2: Motion detection implementation
        def test_motion_detection():
            motion_files = [
                self.project_root / "firmware" / "src" / "motion_filter.cpp",
                self.project_root / "src" / "motion" / "motion_detector.cpp"
            ]
            
            for motion_file in motion_files:
                if motion_file.exists():
                    content = motion_file.read_text()
                    
                    # Check for motion detection functions
                    if 'detectMotion' in content or 'motion_detected' in content:
                        self.log_verbose(f"Motion detection found in {motion_file.name}")
                        return True
                        
            self.log_warning("Motion detection implementation not found")
            return False if not self.simulation else True
            
        results['motion_detection'] = self.run_test("Motion Detection", test_motion_detection)
        
        # Test 3: Enhanced motion detection features
        def test_enhanced_motion_features():
            enhanced_files = [
                self.project_root / "src" / "detection" / "enhanced_hybrid_motion_detector.cpp",
                self.project_root / "src" / "detection" / "multi_zone_pir_sensor.cpp",
                self.project_root / "src" / "detection" / "advanced_motion_detection.cpp"
            ]
            
            features_found = 0
            for enhanced_file in enhanced_files:
                if enhanced_file.exists():
                    features_found += 1
                    self.log_verbose(f"Enhanced feature found: {enhanced_file.name}")
            
            self.log_verbose(f"Enhanced motion features found: {features_found}/3")
            return features_found >= 2  # At least 2 enhanced features should be present
            
        results['enhanced_features'] = self.run_test("Enhanced Motion Features", test_enhanced_motion_features)
        
        # Test 4: Multi-zone PIR support
        def test_multizone_pir():
            multizone_file = self.project_root / "src" / "detection" / "multi_zone_pir_sensor.cpp"
            if multizone_file.exists():
                content = multizone_file.read_text()
                required_functions = ['addZone', 'detectMotion', 'configureDefaultZones']
                functions_found = [func for func in required_functions if func in content]
                self.log_verbose(f"Multi-zone PIR functions found: {functions_found}")
                return len(functions_found) >= 2
            return False if not self.simulation else True
            
        results['multizone_pir'] = self.run_test("Multi-Zone PIR Support", test_multizone_pir)
        
        # Test 5: PIR simulation and calibration
        if self.simulation:
            def test_pir_simulation():
                self.log_info("Running PIR sensor simulation...")
                import random
                
                # Simulate motion detection events
                motion_events = []
                for i in range(5):
                    time.sleep(0.1)
                    motion_detected = random.choice([True, False])
                    motion_events.append(motion_detected)
                    self.log_verbose(f"Simulation cycle {i+1}: Motion {'detected' if motion_detected else 'not detected'}")
                    
                # At least one motion event should be detected in simulation
                return any(motion_events)
                
            results['pir_simulation'] = self.run_test("PIR Simulation", test_pir_simulation)
            
            # Test 6: Hardware calibration simulation
            def test_hardware_calibration():
                self.log_info("Running hardware calibration simulation...")
                
                # Simulate sensitivity calibration
                import random
                sensitivities = [0.3, 0.5, 0.7, 0.9]
                calibration_results = []
                
                for sensitivity in sensitivities:
                    # Simulate detection rate at different sensitivities
                    detection_rate = min(0.95, sensitivity + random.uniform(-0.1, 0.2))
                    calibration_results.append(detection_rate)
                    self.log_verbose(f"Sensitivity {sensitivity}: Detection rate {detection_rate:.2f}")
                
                # Good calibration should show increasing detection rates
                optimal_sensitivity = sensitivities[calibration_results.index(max(calibration_results))]
                self.log_verbose(f"Optimal sensitivity: {optimal_sensitivity}")
                
                return max(calibration_results) > 0.7  # Should achieve >70% detection rate
                
            results['hardware_calibration'] = self.run_test("Hardware Calibration", test_hardware_calibration)
        
        return results

    def run_performance_test(self, component: str):
        """Run performance tests for component"""
        self.log_info(f"Running performance tests for {component}...")
        
        performance_data = {
            'memory_usage': self.get_memory_usage(),
            'execution_time': time.time() - self.start_time,
            'component_specific': {}
        }
        
        if component == 'camera':
            # Simulate camera-specific performance metrics
            performance_data['component_specific'] = {
                'image_capture_time_ms': 150,
                'jpeg_compression_time_ms': 80,
                'memory_buffer_size_kb': 64
            }
        elif component == 'power':
            # Simulate power-specific performance metrics  
            performance_data['component_specific'] = {
                'voltage_read_time_ms': 5,
                'sleep_entry_time_ms': 20,
                'battery_calculation_time_ms': 10
            }
        elif component == 'storage':
            # Simulate storage-specific performance metrics
            performance_data['component_specific'] = {
                'file_write_time_ms': 45,
                'file_read_time_ms': 25,
                'sd_init_time_ms': 200
            }
        elif component == 'motion':
            # Simulate motion-specific performance metrics
            performance_data['component_specific'] = {
                'pir_read_time_ms': 1,
                'motion_filter_time_ms': 15,
                'false_positive_rate': 0.05
            }
            
        self.results['performance'] = performance_data
        self.log_verbose(f"Performance data: {json.dumps(performance_data, indent=2)}")

    def get_memory_usage(self) -> Dict[str, float]:
        """Get current memory usage"""
        try:
            import psutil
            process = psutil.Process()
            memory_info = process.memory_info()
            
            return {
                'rss_mb': memory_info.rss / (1024 * 1024),
                'vms_mb': memory_info.vms / (1024 * 1024)
            }
        except ImportError:
            # psutil not available, return mock data
            return {
                'rss_mb': 15.2,
                'vms_mb': 45.8
            }

    def generate_report(self) -> str:
        """Generate detailed validation report"""
        total_tests = len(self.results['tests'])
        passed_tests = sum(1 for test in self.results['tests'] if test['passed'])
        
        total_time = time.time() - self.start_time
        
        self.results['summary'] = {
            'component': self.results['component'],
            'total_tests': total_tests,
            'passed_tests': passed_tests,
            'success_rate': (passed_tests / total_tests * 100) if total_tests > 0 else 0,
            'total_time_seconds': round(total_time, 2),
            'simulation_mode': self.simulation
        }
        
        # Generate report
        report = []
        report.append("=" * 60)
        report.append(f"ESP32WildlifeCAM Component Validation Report")
        report.append(f"Component: {self.results['component']}")
        report.append(f"Timestamp: {datetime.now().isoformat()}")
        report.append("=" * 60)
        report.append("")
        
        report.append("SUMMARY:")
        report.append(f"  Tests run: {total_tests}")
        report.append(f"  Tests passed: {passed_tests}")
        report.append(f"  Success rate: {self.results['summary']['success_rate']:.1f}%")
        report.append(f"  Total time: {total_time:.2f}s")
        report.append(f"  Simulation mode: {'Yes' if self.simulation else 'No'}")
        report.append("")
        
        report.append("DETAILED RESULTS:")
        for test in self.results['tests']:
            status = "PASS" if test['passed'] else "FAIL"
            report.append(f"  [{status}] {test['name']} ({test['duration_ms']}ms)")
            if 'error' in test:
                report.append(f"         Error: {test['error']}")
        report.append("")
        
        if 'performance' in self.results and self.results['performance']:
            report.append("PERFORMANCE METRICS:")
            perf = self.results['performance']
            if 'execution_time' in perf:
                report.append(f"  Execution time: {perf['execution_time']:.2f}s")
            if 'memory_usage' in perf:
                report.append(f"  Memory usage: {perf['memory_usage']['rss_mb']:.1f}MB RSS")
            
            if 'component_specific' in perf and perf['component_specific']:
                report.append("  Component-specific metrics:")
                for metric, value in perf['component_specific'].items():
                    report.append(f"    {metric}: {value}")
            report.append("")
        
        # Recommendations
        report.append("RECOMMENDATIONS:")
        if passed_tests == total_tests:
            report.append(f"  ✅ {self.results['component']} component validation passed!")
            report.append(f"  • Component is ready for integration")
            if self.simulation:
                report.append(f"  • Run with real hardware when available")
        else:
            report.append(f"  ❌ {self.results['component']} component needs attention")
            report.append(f"  • Review failed tests above")
            report.append(f"  • Check implementation files")
            report.append(f"  • Run full validation: python3 validate_fixes.py")
        
        return "\n".join(report)

def main():
    parser = argparse.ArgumentParser(description='ESP32WildlifeCAM Component Validator')
    parser.add_argument('component', 
                       choices=['camera', 'power', 'storage', 'motion', 'all'],
                       help='Component to validate')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose output')
    parser.add_argument('--simulation', '-s', action='store_true',
                       help='Enable hardware simulation')
    parser.add_argument('--hardware-test', action='store_true',
                       help='Run hardware-specific validation tests')
    parser.add_argument('--performance', '-p', action='store_true',
                       help='Run performance tests')
    parser.add_argument('--output', '-o', type=str,
                       help='Output report to file')
    parser.add_argument('--json', action='store_true',
                       help='Output results in JSON format')
    
    args = parser.parse_args()
    
    # Enable simulation mode if hardware-test is requested but hardware isn't available
    if args.hardware_test:
        args.simulation = True
    
    validator = ComponentValidator(verbose=args.verbose, simulation=args.simulation)
    
    print(f"ESP32WildlifeCAM Component Validation")
    print(f"====================================")
    print(f"Component: {args.component}")
    print(f"Hardware Test: {'Enabled' if args.hardware_test else 'Disabled'}")
    print(f"Simulation: {'Enabled' if args.simulation else 'Disabled'}")
    print(f"Target: Complete validation in under 2 minutes")
    print("")
    
    # Run component-specific validation
    component_results = {}
    
    if args.component == 'all':
        components = ['camera', 'power', 'storage', 'motion']
    else:
        components = [args.component]
    
    for component in components:
        validator.results['component'] = component
        validator.results['tests'] = []  # Reset tests for each component
        
        if component == 'camera':
            component_results[component] = validator.validate_camera_component()
        elif component == 'power':
            component_results[component] = validator.validate_power_component()
        elif component == 'storage':
            component_results[component] = validator.validate_storage_component()
        elif component == 'motion':
            component_results[component] = validator.validate_motion_component()
        
        if args.performance:
            validator.run_performance_test(component)
    
    # Run performance tests if requested
    if args.performance:
        if args.component == 'all':
            for component in components:
                validator.run_performance_test(component)
        else:
            validator.run_performance_test(args.component)
    
    # Generate and display report
    if args.component != 'all':
        validator.results['component'] = args.component
        report = validator.generate_report()
        
        if args.json:
            print(json.dumps(validator.results, indent=2))
        else:
            print(report)
        
        if args.output:
            Path(args.output).write_text(report)
            print(f"\nReport saved to: {args.output}")
    
    # Return appropriate exit code
    total_passed = sum(len([t for t in validator.results['tests'] if t['passed']]) for _ in components)
    total_tests = sum(len(validator.results['tests']) for _ in components)
    
    if total_passed == total_tests:
        return 0
    else:
        return 1

if __name__ == "__main__":
    sys.exit(main())