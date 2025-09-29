#!/usr/bin/env python3
"""
ESP32 Wildlife CAM - Field Test Automation Script

Automated field testing framework for comprehensive validation of ESP32 Wildlife CAM systems
across different deployment scenarios and environmental conditions.
"""

import os
import sys
import json
import time
import argparse
import logging
import datetime
import subprocess
import threading
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict
import csv

# Add project root to path
project_root = Path(__file__).parent.parent
sys.path.insert(0, str(project_root))

@dataclass
class TestScenario:
    """Test scenario configuration"""
    id: str
    name: str
    duration_hours: int
    environment: str
    complexity: str
    wildlife_targets: List[str]
    success_criteria: Dict[str, float]
    config_overrides: Dict[str, any]

@dataclass
class TestResult:
    """Test execution result"""
    scenario_id: str
    start_time: datetime.datetime
    end_time: Optional[datetime.datetime]
    status: str  # running, completed, failed, aborted
    metrics: Dict[str, float]
    issues: List[str]
    wildlife_encounters: int
    image_count: int
    data_size_mb: float

class FieldTestAutomation:
    """Main field test automation controller"""
    
    def __init__(self, config_file: Optional[str] = None):
        self.project_root = project_root
        self.config_file = config_file or self.project_root / "config" / "field_test_config.json"
        self.results_dir = self.project_root / "test_results" / "field_tests"
        self.data_dir = self.project_root / "data" / "field_tests"
        
        # Create directories
        self.results_dir.mkdir(parents=True, exist_ok=True)
        self.data_dir.mkdir(parents=True, exist_ok=True)
        
        # Setup logging
        self.setup_logging()
        
        # Load configuration
        self.load_configuration()
        
        # Active test tracking
        self.active_tests: Dict[str, TestResult] = {}
        self.monitoring_threads: Dict[str, threading.Thread] = {}
    
    def setup_logging(self):
        """Configure logging for field test automation"""
        log_file = self.results_dir / f"field_test_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
        
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file),
                logging.StreamHandler(sys.stdout)
            ]
        )
        
        self.logger = logging.getLogger(__name__)
        self.logger.info("Field Test Automation initialized")
    
    def load_configuration(self):
        """Load test configuration and scenarios"""
        try:
            if self.config_file.exists():
                with open(self.config_file, 'r') as f:
                    config = json.load(f)
            else:
                config = self.create_default_config()
                self.save_configuration(config)
            
            self.config = config
            self.scenarios = {s['id']: TestScenario(**s) for s in config['scenarios']}
            self.logger.info(f"Loaded {len(self.scenarios)} test scenarios")
            
        except Exception as e:
            self.logger.error(f"Failed to load configuration: {e}")
            raise
    
    def create_default_config(self) -> Dict:
        """Create default test configuration"""
        return {
            "system_config": {
                "serial_port": "/dev/ttyUSB0",
                "baud_rate": 115200,
                "upload_command": "pio run --target upload",
                "monitor_command": "pio device monitor",
                "power_measurement": {
                    "enabled": False,
                    "device": "/dev/ttyACM0",
                    "measurement_interval": 60
                }
            },
            "scenarios": [
                {
                    "id": "SCN-001",
                    "name": "Residential Garden Wildlife Monitoring",
                    "duration_hours": 72,
                    "environment": "urban",
                    "complexity": "low",
                    "wildlife_targets": ["songbirds", "squirrels", "cats"],
                    "success_criteria": {
                        "uptime_percent": 98.0,
                        "wildlife_detection_percent": 75.0,
                        "false_positive_percent": 10.0,
                        "battery_voltage_min": 3.7,
                        "image_quality_percent": 90.0
                    },
                    "config_overrides": {
                        "motion_sensitivity": "medium",
                        "pir_timeout": 30,
                        "image_quality": "high",
                        "power_conservation": "moderate"
                    }
                },
                {
                    "id": "SCN-002",
                    "name": "Urban Predator Activity Detection",
                    "duration_hours": 120,
                    "environment": "urban",
                    "complexity": "medium",
                    "wildlife_targets": ["cats", "foxes", "raccoons"],
                    "success_criteria": {
                        "uptime_percent": 95.0,
                        "wildlife_detection_percent": 70.0,
                        "false_positive_percent": 8.0,
                        "battery_voltage_min": 3.6,
                        "nocturnal_captures": 5
                    },
                    "config_overrides": {
                        "motion_sensitivity": "high",
                        "pir_timeout": 120,
                        "night_mode": True,
                        "ir_flash": True
                    }
                },
                {
                    "id": "SCN-003",
                    "name": "Nature Trail Game Camera Simulation",
                    "duration_hours": 168,
                    "environment": "trail",
                    "complexity": "medium",
                    "wildlife_targets": ["deer", "small_mammals", "birds", "predators"],
                    "success_criteria": {
                        "uptime_percent": 90.0,
                        "species_diversity": 8,
                        "wildlife_events": 50,
                        "weather_resilience": True,
                        "data_quality_percent": 90.0
                    },
                    "config_overrides": {
                        "motion_sensitivity": "adaptive",
                        "detection_zone": "wide",
                        "timestamp_overlay": True,
                        "weather_monitoring": True
                    }
                },
                {
                    "id": "SCN-004",
                    "name": "Water Source Monitoring",
                    "duration_hours": 240,
                    "environment": "trail",
                    "complexity": "high",
                    "wildlife_targets": ["diverse_species", "predator_prey_interactions"],
                    "success_criteria": {
                        "uptime_percent": 95.0,
                        "species_count": 15,
                        "behavioral_data": True,
                        "humidity_resistance": True
                    },
                    "config_overrides": {
                        "motion_zones": "multiple",
                        "capture_rate": "high",
                        "humidity_protection": True
                    }
                },
                {
                    "id": "SCN-005",
                    "name": "Remote Mountain Wildlife Survey",
                    "duration_hours": 336,
                    "environment": "wilderness",
                    "complexity": "high",
                    "wildlife_targets": ["elk", "deer", "bears", "mountain_lions"],
                    "success_criteria": {
                        "uptime_percent": 90.0,
                        "autonomous_operation_days": 14,
                        "large_wildlife_species": 3,
                        "weather_survival": True
                    },
                    "config_overrides": {
                        "power_mode": "ultra_conservative",
                        "motion_tuning": "large_animal",
                        "error_recovery": "aggressive",
                        "data_compression": "high"
                    }
                }
            ]
        }
    
    def save_configuration(self, config: Dict):
        """Save configuration to file"""
        self.config_file.parent.mkdir(parents=True, exist_ok=True)
        with open(self.config_file, 'w') as f:
            json.dump(config, f, indent=2)
    
    def execute_scenario(self, scenario_id: str, deployment_id: Optional[str] = None) -> TestResult:
        """Execute a specific test scenario"""
        if scenario_id not in self.scenarios:
            raise ValueError(f"Unknown scenario: {scenario_id}")
        
        scenario = self.scenarios[scenario_id]
        deployment_id = deployment_id or f"{scenario_id}_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
        self.logger.info(f"Starting scenario {scenario_id}: {scenario.name}")
        self.logger.info(f"Deployment ID: {deployment_id}")
        
        # Create test result
        result = TestResult(
            scenario_id=scenario_id,
            start_time=datetime.datetime.now(),
            end_time=None,
            status="running",
            metrics={},
            issues=[],
            wildlife_encounters=0,
            image_count=0,
            data_size_mb=0.0
        )
        
        self.active_tests[deployment_id] = result
        
        try:
            # Pre-deployment setup
            self.setup_deployment(scenario, deployment_id)
            
            # Start monitoring
            self.start_monitoring(scenario, deployment_id)
            
            # Execute test scenario
            if scenario.environment in ["urban", "trail"]:
                self.execute_supervised_test(scenario, deployment_id)
            else:
                self.execute_autonomous_test(scenario, deployment_id)
            
            # Complete test
            result.status = "completed"
            result.end_time = datetime.datetime.now()
            
        except Exception as e:
            self.logger.error(f"Scenario execution failed: {e}")
            result.status = "failed"
            result.issues.append(str(e))
            result.end_time = datetime.datetime.now()
        
        finally:
            # Stop monitoring
            self.stop_monitoring(deployment_id)
            
            # Post-test analysis
            self.analyze_results(scenario, deployment_id, result)
            
            # Generate report
            self.generate_test_report(scenario, deployment_id, result)
        
        return result
    
    def setup_deployment(self, scenario: TestScenario, deployment_id: str):
        """Setup system for deployment"""
        self.logger.info(f"Setting up deployment {deployment_id}")
        
        # Create deployment directory
        deployment_dir = self.data_dir / deployment_id
        deployment_dir.mkdir(exist_ok=True)
        
        # Generate configuration file
        config = self.generate_deployment_config(scenario)
        config_file = deployment_dir / "deployment_config.json"
        
        with open(config_file, 'w') as f:
            json.dump(config, f, indent=2)
        
        # Upload firmware if system is connected
        if self.is_system_connected():
            self.upload_firmware(config_file)
        else:
            self.logger.warning("System not connected - manual firmware upload required")
        
        # Initialize data collection
        self.initialize_data_collection(deployment_id)
    
    def generate_deployment_config(self, scenario: TestScenario) -> Dict:
        """Generate system configuration for deployment"""
        base_config = {
            "scenario_id": scenario.id,
            "deployment_time": datetime.datetime.now().isoformat(),
            "duration_hours": scenario.duration_hours,
            "environment": scenario.environment
        }
        
        # Apply scenario-specific overrides
        base_config.update(scenario.config_overrides)
        
        return base_config
    
    def is_system_connected(self) -> bool:
        """Check if ESP32 system is connected"""
        try:
            result = subprocess.run(
                ["pio", "device", "list"],
                capture_output=True,
                text=True,
                timeout=10
            )
            return "esp32" in result.stdout.lower() or "cp210" in result.stdout.lower()
        except Exception:
            return False
    
    def upload_firmware(self, config_file: Path):
        """Upload firmware with deployment configuration"""
        self.logger.info("Uploading firmware with deployment configuration")
        
        try:
            # Copy config to firmware directory for compilation
            firmware_config = self.project_root / "firmware" / "deployment_config.h"
            self.generate_firmware_config(config_file, firmware_config)
            
            # Upload firmware
            result = subprocess.run(
                ["pio", "run", "--target", "upload"],
                cwd=self.project_root,
                capture_output=True,
                text=True,
                timeout=300
            )
            
            if result.returncode != 0:
                raise RuntimeError(f"Firmware upload failed: {result.stderr}")
            
            self.logger.info("Firmware uploaded successfully")
            
        except Exception as e:
            self.logger.error(f"Firmware upload failed: {e}")
            raise
    
    def generate_firmware_config(self, config_file: Path, output_file: Path):
        """Generate firmware configuration header from JSON config"""
        with open(config_file, 'r') as f:
            config = json.load(f)
        
        header_content = [
            "#ifndef DEPLOYMENT_CONFIG_H",
            "#define DEPLOYMENT_CONFIG_H",
            "",
            "// Auto-generated deployment configuration",
            f"// Generated: {datetime.datetime.now().isoformat()}",
            "",
        ]
        
        # Convert JSON config to C defines
        for key, value in config.items():
            if isinstance(value, str):
                header_content.append(f'#define DEPLOYMENT_{key.upper()} "{value}"')
            elif isinstance(value, bool):
                header_content.append(f'#define DEPLOYMENT_{key.upper()} {1 if value else 0}')
            elif isinstance(value, (int, float)):
                header_content.append(f'#define DEPLOYMENT_{key.upper()} {value}')
        
        header_content.extend([
            "",
            "#endif // DEPLOYMENT_CONFIG_H"
        ])
        
        with open(output_file, 'w') as f:
            f.write('\n'.join(header_content))
    
    def start_monitoring(self, scenario: TestScenario, deployment_id: str):
        """Start monitoring thread for deployment"""
        self.logger.info(f"Starting monitoring for {deployment_id}")
        
        def monitor_thread():
            try:
                self.monitor_deployment(scenario, deployment_id)
            except Exception as e:
                self.logger.error(f"Monitoring failed for {deployment_id}: {e}")
        
        thread = threading.Thread(target=monitor_thread, daemon=True)
        self.monitoring_threads[deployment_id] = thread
        thread.start()
    
    def monitor_deployment(self, scenario: TestScenario, deployment_id: str):
        """Monitor deployment and collect metrics"""
        deployment_dir = self.data_dir / deployment_id
        metrics_file = deployment_dir / "metrics.csv"
        
        # Initialize metrics file
        with open(metrics_file, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow([
                'timestamp', 'battery_voltage', 'solar_current', 'temperature',
                'humidity', 'motion_events', 'images_captured', 'storage_used_mb',
                'system_uptime', 'errors'
            ])
        
        start_time = datetime.datetime.now()
        end_time = start_time + datetime.timedelta(hours=scenario.duration_hours)
        
        while datetime.datetime.now() < end_time:
            try:
                # Collect metrics (simulated for now)
                metrics = self.collect_system_metrics(deployment_id)
                
                # Write to metrics file
                with open(metrics_file, 'a', newline='') as f:
                    writer = csv.writer(f)
                    writer.writerow([
                        datetime.datetime.now().isoformat(),
                        metrics.get('battery_voltage', 0),
                        metrics.get('solar_current', 0),
                        metrics.get('temperature', 20),
                        metrics.get('humidity', 50),
                        metrics.get('motion_events', 0),
                        metrics.get('images_captured', 0),
                        metrics.get('storage_used_mb', 0),
                        metrics.get('system_uptime', 0),
                        metrics.get('errors', 0)
                    ])
                
                # Update active test result
                if deployment_id in self.active_tests:
                    result = self.active_tests[deployment_id]
                    result.metrics.update(metrics)
                
                # Sleep until next measurement
                time.sleep(300)  # 5 minutes
                
            except Exception as e:
                self.logger.error(f"Metrics collection error: {e}")
                time.sleep(60)  # Retry in 1 minute
    
    def collect_system_metrics(self, deployment_id: str) -> Dict[str, float]:
        """Collect system metrics (placeholder for actual implementation)"""
        # This would interface with actual hardware monitoring
        # For now, return simulated metrics
        import random
        
        return {
            'battery_voltage': round(3.6 + random.random() * 0.6, 2),
            'solar_current': round(random.random() * 100, 1),
            'temperature': round(20 + random.random() * 20, 1),
            'humidity': round(40 + random.random() * 40, 1),
            'motion_events': random.randint(0, 5),
            'images_captured': random.randint(0, 10),
            'storage_used_mb': round(random.random() * 1000, 1),
            'system_uptime': round(random.random() * 100, 1),
            'errors': random.randint(0, 2)
        }
    
    def execute_supervised_test(self, scenario: TestScenario, deployment_id: str):
        """Execute supervised test (with human monitoring capability)"""
        self.logger.info(f"Executing supervised test: {scenario.name}")
        
        duration = datetime.timedelta(hours=scenario.duration_hours)
        end_time = datetime.datetime.now() + duration
        
        # Simulate test execution with periodic checks
        check_interval = min(3600, scenario.duration_hours * 3600 // 10)  # 10 checks minimum
        
        while datetime.datetime.now() < end_time:
            # Simulate system check
            self.logger.info(f"System check for {deployment_id}")
            
            # Check if test should be aborted
            if self.should_abort_test(deployment_id):
                self.logger.warning(f"Test {deployment_id} aborted")
                break
            
            time.sleep(check_interval)
        
        self.logger.info(f"Supervised test completed: {deployment_id}")
    
    def execute_autonomous_test(self, scenario: TestScenario, deployment_id: str):
        """Execute autonomous test (no human intervention)"""
        self.logger.info(f"Executing autonomous test: {scenario.name}")
        
        duration = datetime.timedelta(hours=scenario.duration_hours)
        end_time = datetime.datetime.now() + duration
        
        # For autonomous tests, just wait for completion
        # Real implementation would involve remote monitoring
        while datetime.datetime.now() < end_time:
            time.sleep(3600)  # Check hourly
            
            if self.should_abort_test(deployment_id):
                self.logger.warning(f"Autonomous test {deployment_id} aborted")
                break
        
        self.logger.info(f"Autonomous test completed: {deployment_id}")
    
    def should_abort_test(self, deployment_id: str) -> bool:
        """Check if test should be aborted"""
        # Check for abort file
        abort_file = self.data_dir / deployment_id / "abort_test"
        return abort_file.exists()
    
    def stop_monitoring(self, deployment_id: str):
        """Stop monitoring thread"""
        if deployment_id in self.monitoring_threads:
            self.logger.info(f"Stopping monitoring for {deployment_id}")
            # Thread will stop naturally when test completes
            thread = self.monitoring_threads[deployment_id]
            thread.join(timeout=30)
            del self.monitoring_threads[deployment_id]
    
    def analyze_results(self, scenario: TestScenario, deployment_id: str, result: TestResult):
        """Analyze test results and calculate metrics"""
        self.logger.info(f"Analyzing results for {deployment_id}")
        
        deployment_dir = self.data_dir / deployment_id
        
        # Analyze metrics file
        metrics_file = deployment_dir / "metrics.csv"
        if metrics_file.exists():
            self.analyze_metrics_file(metrics_file, result)
        
        # Analyze images (if any)
        images_dir = deployment_dir / "images"
        if images_dir.exists():
            self.analyze_captured_images(images_dir, result)
        
        # Calculate success criteria
        self.evaluate_success_criteria(scenario, result)
    
    def analyze_metrics_file(self, metrics_file: Path, result: TestResult):
        """Analyze metrics CSV file"""
        try:
            import pandas as pd
            
            df = pd.read_csv(metrics_file)
            
            if not df.empty:
                # Calculate summary statistics
                result.metrics.update({
                    'avg_battery_voltage': float(df['battery_voltage'].mean()),
                    'min_battery_voltage': float(df['battery_voltage'].min()),
                    'max_battery_voltage': float(df['battery_voltage'].max()),
                    'total_motion_events': int(df['motion_events'].sum()),
                    'total_images_captured': int(df['images_captured'].sum()),
                    'uptime_percent': float((df['system_uptime'] > 0).mean() * 100),
                    'error_count': int(df['errors'].sum())
                })
                
                result.wildlife_encounters = int(df['motion_events'].sum())
                result.image_count = int(df['images_captured'].sum())
                
        except ImportError:
            self.logger.warning("pandas not available - using basic analysis")
            self.analyze_metrics_basic(metrics_file, result)
        except Exception as e:
            self.logger.error(f"Metrics analysis failed: {e}")
    
    def analyze_metrics_basic(self, metrics_file: Path, result: TestResult):
        """Basic metrics analysis without pandas"""
        try:
            with open(metrics_file, 'r') as f:
                reader = csv.DictReader(f)
                
                battery_voltages = []
                motion_events = 0
                images_captured = 0
                uptime_count = 0
                total_count = 0
                
                for row in reader:
                    battery_voltages.append(float(row['battery_voltage']))
                    motion_events += int(row['motion_events'])
                    images_captured += int(row['images_captured'])
                    
                    if float(row['system_uptime']) > 0:
                        uptime_count += 1
                    total_count += 1
                
                if battery_voltages:
                    result.metrics.update({
                        'avg_battery_voltage': sum(battery_voltages) / len(battery_voltages),
                        'min_battery_voltage': min(battery_voltages),
                        'max_battery_voltage': max(battery_voltages),
                        'total_motion_events': motion_events,
                        'total_images_captured': images_captured,
                        'uptime_percent': (uptime_count / total_count * 100) if total_count > 0 else 0
                    })
                    
                    result.wildlife_encounters = motion_events
                    result.image_count = images_captured
                    
        except Exception as e:
            self.logger.error(f"Basic metrics analysis failed: {e}")
    
    def analyze_captured_images(self, images_dir: Path, result: TestResult):
        """Analyze captured images"""
        try:
            image_files = list(images_dir.glob("*.jpg")) + list(images_dir.glob("*.png"))
            result.image_count = len(image_files)
            
            # Calculate total data size
            total_size = sum(f.stat().st_size for f in image_files)
            result.data_size_mb = total_size / (1024 * 1024)
            
            self.logger.info(f"Analyzed {len(image_files)} images, total size: {result.data_size_mb:.1f} MB")
            
        except Exception as e:
            self.logger.error(f"Image analysis failed: {e}")
    
    def evaluate_success_criteria(self, scenario: TestScenario, result: TestResult):
        """Evaluate test against success criteria"""
        criteria = scenario.success_criteria
        success_count = 0
        total_criteria = 0
        
        for criterion, target_value in criteria.items():
            total_criteria += 1
            actual_value = result.metrics.get(criterion, 0)
            
            if isinstance(target_value, (int, float)):
                if criterion.endswith('_percent') and actual_value >= target_value:
                    success_count += 1
                elif criterion.endswith('_min') and actual_value >= target_value:
                    success_count += 1
                elif criterion.endswith('_max') and actual_value <= target_value:
                    success_count += 1
                elif actual_value >= target_value:
                    success_count += 1
            elif isinstance(target_value, bool) and bool(actual_value) == target_value:
                success_count += 1
        
        result.metrics['success_rate'] = (success_count / total_criteria * 100) if total_criteria > 0 else 0
        
        self.logger.info(f"Success criteria: {success_count}/{total_criteria} ({result.metrics['success_rate']:.1f}%)")
    
    def generate_test_report(self, scenario: TestScenario, deployment_id: str, result: TestResult):
        """Generate comprehensive test report"""
        self.logger.info(f"Generating test report for {deployment_id}")
        
        report_file = self.results_dir / f"{deployment_id}_report.json"
        
        report = {
            'deployment_id': deployment_id,
            'scenario': asdict(scenario),
            'result': asdict(result),
            'generated_at': datetime.datetime.now().isoformat(),
            'duration_actual_hours': (result.end_time - result.start_time).total_seconds() / 3600 if result.end_time else None
        }
        
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2, default=str)
        
        # Generate human-readable report
        self.generate_readable_report(report, deployment_id)
        
        self.logger.info(f"Test report saved: {report_file}")
    
    def generate_readable_report(self, report: Dict, deployment_id: str):
        """Generate human-readable test report"""
        readable_file = self.results_dir / f"{deployment_id}_report.md"
        
        scenario = report['scenario']
        result = report['result']
        
        content = f"""# Field Test Report: {deployment_id}

## Scenario: {scenario['name']}

**Test ID**: {scenario['id']}  
**Environment**: {scenario['environment']}  
**Complexity**: {scenario['complexity']}  
**Duration**: {scenario['duration_hours']} hours  

## Test Execution

**Start Time**: {result['start_time']}  
**End Time**: {result['end_time']}  
**Status**: {result['status']}  

## Results Summary

**Wildlife Encounters**: {result['wildlife_encounters']}  
**Images Captured**: {result['image_count']}  
**Data Size**: {result['data_size_mb']:.1f} MB  

## Key Metrics

"""
        
        for metric, value in result['metrics'].items():
            content += f"- **{metric.replace('_', ' ').title()}**: {value}\n"
        
        content += f"""

## Success Criteria Evaluation

**Overall Success Rate**: {result['metrics'].get('success_rate', 0):.1f}%

"""
        
        for criterion, target in scenario['success_criteria'].items():
            actual = result['metrics'].get(criterion, 'N/A')
            status = "✅" if actual != 'N/A' and actual >= target else "❌"
            content += f"- {status} **{criterion.replace('_', ' ').title()}**: {actual} (target: {target})\n"
        
        if result['issues']:
            content += f"\n## Issues Encountered\n\n"
            for issue in result['issues']:
                content += f"- {issue}\n"
        
        content += f"""

## Recommendations

Based on the test results, the following recommendations are made:

1. **Performance Optimization**: [Add specific recommendations based on metrics]
2. **Configuration Adjustments**: [Add configuration change suggestions]
3. **Hardware Improvements**: [Add hardware modification suggestions]

---

*Report generated on {report['generated_at']}*
"""
        
        with open(readable_file, 'w') as f:
            f.write(content)
    
    def initialize_data_collection(self, deployment_id: str):
        """Initialize data collection structure"""
        deployment_dir = self.data_dir / deployment_id
        
        # Create subdirectories
        (deployment_dir / "images").mkdir(exist_ok=True)
        (deployment_dir / "logs").mkdir(exist_ok=True)
        (deployment_dir / "config").mkdir(exist_ok=True)
        
        # Create collection info file
        info = {
            'deployment_id': deployment_id,
            'created_at': datetime.datetime.now().isoformat(),
            'data_collection_version': '1.0'
        }
        
        with open(deployment_dir / "collection_info.json", 'w') as f:
            json.dump(info, f, indent=2)
    
    def list_scenarios(self):
        """List available test scenarios"""
        print("\nAvailable Test Scenarios:")
        print("=" * 50)
        
        for scenario_id, scenario in self.scenarios.items():
            print(f"\n{scenario_id}: {scenario.name}")
            print(f"  Environment: {scenario.environment}")
            print(f"  Duration: {scenario.duration_hours} hours")
            print(f"  Complexity: {scenario.complexity}")
            print(f"  Targets: {', '.join(scenario.wildlife_targets)}")
    
    def abort_test(self, deployment_id: str):
        """Abort a running test"""
        abort_file = self.data_dir / deployment_id / "abort_test"
        abort_file.touch()
        self.logger.info(f"Abort signal sent for {deployment_id}")

def main():
    """Main command line interface"""
    parser = argparse.ArgumentParser(description="ESP32 Wildlife CAM Field Test Automation")
    
    parser.add_argument('--scenario', type=str, help='Test scenario ID to execute')
    parser.add_argument('--list-scenarios', action='store_true', help='List available scenarios')
    parser.add_argument('--deployment-id', type=str, help='Custom deployment ID')
    parser.add_argument('--config', type=str, help='Configuration file path')
    parser.add_argument('--abort', type=str, help='Abort test with given deployment ID')
    parser.add_argument('--verbose', action='store_true', help='Enable verbose logging')
    
    args = parser.parse_args()
    
    # Setup logging level
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    try:
        # Initialize field test automation
        automation = FieldTestAutomation(config_file=args.config)
        
        if args.list_scenarios:
            automation.list_scenarios()
        elif args.abort:
            automation.abort_test(args.abort)
        elif args.scenario:
            result = automation.execute_scenario(args.scenario, args.deployment_id)
            print(f"\nTest completed with status: {result.status}")
            print(f"Success rate: {result.metrics.get('success_rate', 0):.1f}%")
        else:
            parser.print_help()
    
    except KeyboardInterrupt:
        print("\nTest automation interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()