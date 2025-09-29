#!/usr/bin/env python3
"""
ESP32 Wildlife Camera Quick Deployment Script

This script automates the deployment process for ESP32 wildlife cameras,
including board detection, firmware configuration, and initial setup.

Usage:
    python3 quick-deploy.py --scenario research --board auto
    python3 quick-deploy.py --scenario conservation --board ai-thinker
    python3 quick-deploy.py --help
"""

import argparse
import json
import os
import sys
import time
import subprocess
import serial
import serial.tools.list_ports
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import requests
from datetime import datetime

class ESP32DeploymentManager:
    """Manages ESP32 wildlife camera deployments"""
    
    SUPPORTED_SCENARIOS = [
        'research', 'conservation', 'urban', 'extreme', 'ai-research'
    ]
    
    SUPPORTED_BOARDS = [
        'auto', 'ai-thinker', 'esp32-s3-cam', 'esp-eye', 
        'm5stack-timer', 'xiao-esp32s3', 'ttgo-t-camera'
    ]
    
    def __init__(self):
        self.script_dir = Path(__file__).parent.absolute()
        self.repo_root = self.script_dir.parent.parent
        self.firmware_dir = self.repo_root / 'firmware'
        self.config_dir = self.script_dir.parent / 'deployment-configs'
        
        # Deployment state
        self.deployment_config = {}
        self.detected_board = None
        self.selected_scenario = None
        self.serial_port = None
        
    def detect_board(self) -> Optional[str]:
        """Auto-detect connected ESP32 board"""
        print("🔍 Detecting connected ESP32 board...")
        
        # List available serial ports
        ports = serial.tools.list_ports.comports()
        esp32_ports = []
        
        for port in ports:
            # Check for ESP32-specific USB identifiers
            if any(identifier in str(port.description).lower() for identifier in 
                   ['cp210', 'ch340', 'ft232', 'esp32', 'silicon labs']):
                esp32_ports.append(port.device)
                print(f"  Found potential ESP32 port: {port.device} ({port.description})")
        
        if not esp32_ports:
            print("❌ No ESP32 boards detected. Please check connection.")
            return None
        
        # Use the first detected port
        selected_port = esp32_ports[0]
        self.serial_port = selected_port
        
        # Try to communicate with the board to identify it
        try:
            with serial.Serial(selected_port, 115200, timeout=2) as ser:
                # Send a command to get board info
                ser.write(b'\n')
                time.sleep(0.5)
                response = ser.read_all().decode('utf-8', errors='ignore')
                
                # Analyze response to determine board type
                board_type = self._analyze_board_response(response)
                if board_type:
                    print(f"✅ Detected board: {board_type}")
                    self.detected_board = board_type
                    return board_type
                else:
                    print("⚠️  Board detected but type unknown, will use auto-detection")
                    self.detected_board = 'auto'
                    return 'auto'
                    
        except Exception as e:
            print(f"⚠️  Communication error: {e}")
            print("   Will proceed with manual board selection")
            return None
    
    def _analyze_board_response(self, response: str) -> Optional[str]:
        """Analyze board response to determine type"""
        response_lower = response.lower()
        
        if 'esp32-s3' in response_lower:
            if 'eye' in response_lower:
                return 'esp32-s3-eye'
            elif 'cam' in response_lower:
                return 'esp32-s3-cam'
            else:
                return 'esp32-s3-cam'  # Default S3 variant
        elif 'esp-eye' in response_lower:
            return 'esp-eye'
        elif 'ai-thinker' in response_lower or 'esp32-cam' in response_lower:
            return 'ai-thinker'
        elif 'm5stack' in response_lower:
            return 'm5stack-timer'
        elif 'xiao' in response_lower:
            return 'xiao-esp32s3'
        elif 'ttgo' in response_lower:
            return 'ttgo-t-camera'
        
        return None
    
    def load_scenario_config(self, scenario: str) -> Dict:
        """Load configuration for deployment scenario"""
        print(f"📋 Loading configuration for scenario: {scenario}")
        
        config_file = self.config_dir / f'{scenario}-config.h'
        if not config_file.exists():
            raise FileNotFoundError(f"Configuration file not found: {config_file}")
        
        # Parse the configuration file to extract settings
        config = self._parse_config_file(config_file)
        config['scenario'] = scenario
        config['config_file'] = str(config_file)
        
        print(f"✅ Configuration loaded successfully")
        return config
    
    def _parse_config_file(self, config_file: Path) -> Dict:
        """Parse C header configuration file"""
        config = {}
        
        with open(config_file, 'r') as f:
            content = f.read()
        
        # Extract key configuration values using simple parsing
        import re
        
        # Find #define statements
        defines = re.findall(r'#define\s+(\w+)\s+(.+)', content)
        
        for name, value in defines:
            # Clean up the value
            value = value.strip()
            if value.startswith('"') and value.endswith('"'):
                config[name] = value[1:-1]  # String value
            elif value.isdigit():
                config[name] = int(value)   # Integer value
            elif value in ['true', 'false']:
                config[name] = value == 'true'  # Boolean value
            else:
                config[name] = value        # Raw value
        
        return config
    
    def configure_firmware(self, scenario: str, board: str) -> bool:
        """Configure firmware for deployment"""
        print(f"⚙️  Configuring firmware for {scenario} deployment on {board}")
        
        # Load scenario configuration
        self.deployment_config = self.load_scenario_config(scenario)
        
        # Copy configuration to firmware
        scenario_config = self.config_dir / f'{scenario}-config.h'
        firmware_config = self.firmware_dir / 'src' / 'config.h'
        
        try:
            # Backup existing config
            if firmware_config.exists():
                backup_config = firmware_config.with_suffix('.h.backup')
                import shutil
                shutil.copy2(firmware_config, backup_config)
                print(f"📄 Backed up existing config to {backup_config}")
            
            # Copy scenario config
            import shutil
            shutil.copy2(scenario_config, firmware_config)
            print(f"✅ Configuration applied: {scenario_config} -> {firmware_config}")
            
            return True
            
        except Exception as e:
            print(f"❌ Failed to configure firmware: {e}")
            return False
    
    def build_firmware(self) -> bool:
        """Build firmware using PlatformIO"""
        print("🔨 Building firmware...")
        
        try:
            # Check if PlatformIO is installed
            result = subprocess.run(['pio', '--version'], 
                                  capture_output=True, text=True, timeout=10)
            if result.returncode != 0:
                print("❌ PlatformIO not found. Please install PlatformIO first:")
                print("   pip3 install platformio")
                return False
            
            print(f"✅ PlatformIO version: {result.stdout.strip()}")
            
            # Build firmware
            build_env = 'esp32-s3-devkitc-1'  # Default build environment
            if self.detected_board:
                build_env = self._get_build_environment(self.detected_board)
            
            print(f"🏗️  Building for environment: {build_env}")
            
            build_cmd = ['pio', 'run', '-e', build_env]
            result = subprocess.run(build_cmd, cwd=self.firmware_dir, 
                                  timeout=600)  # 10 minute timeout
            
            if result.returncode == 0:
                print("✅ Firmware built successfully")
                return True
            else:
                print("❌ Firmware build failed")
                return False
                
        except subprocess.TimeoutExpired:
            print("❌ Build timeout (10 minutes) - this may be normal for first build")
            print("   Try running the build command manually:")
            print(f"   cd {self.firmware_dir} && pio run -e {build_env}")
            return False
        except Exception as e:
            print(f"❌ Build error: {e}")
            return False
    
    def _get_build_environment(self, board: str) -> str:
        """Get PlatformIO build environment for board"""
        env_map = {
            'ai-thinker': 'esp32cam',
            'esp32-s3-cam': 'esp32-s3-devkitc-1',
            'esp-eye': 'esp-eye',
            'm5stack-timer': 'm5stack-timer-cam',
            'xiao-esp32s3': 'seeed_xiao_esp32s3',
            'ttgo-t-camera': 'ttgo-t-camera'
        }
        return env_map.get(board, 'esp32-s3-devkitc-1')
    
    def flash_firmware(self) -> bool:
        """Flash firmware to ESP32"""
        if not self.serial_port:
            print("❌ No serial port detected for flashing")
            return False
        
        print(f"📱 Flashing firmware to {self.serial_port}...")
        
        try:
            build_env = self._get_build_environment(self.detected_board or 'esp32-s3-cam')
            
            flash_cmd = ['pio', 'run', '-e', build_env, '--target', 'upload', 
                        '--upload-port', self.serial_port]
            
            result = subprocess.run(flash_cmd, cwd=self.firmware_dir, timeout=120)
            
            if result.returncode == 0:
                print("✅ Firmware flashed successfully")
                return True
            else:
                print("❌ Firmware flashing failed")
                return False
                
        except subprocess.TimeoutExpired:
            print("❌ Flash timeout - check board connection and try again")
            return False
        except Exception as e:
            print(f"❌ Flash error: {e}")
            return False
    
    def validate_deployment(self) -> bool:
        """Validate deployment configuration and operation"""
        print("🔍 Validating deployment...")
        
        # Run validation script
        validate_script = self.repo_root / 'validate_fixes.py'
        if validate_script.exists():
            try:
                result = subprocess.run(['python3', str(validate_script)], 
                                      capture_output=True, text=True, timeout=60)
                
                if result.returncode == 0:
                    print("✅ Configuration validation passed")
                    return True
                else:
                    print("❌ Configuration validation failed:")
                    print(result.stdout)
                    print(result.stderr)
                    return False
                    
            except Exception as e:
                print(f"⚠️  Validation error: {e}")
                print("   Proceeding without validation")
        
        # Test serial communication
        if self.serial_port:
            try:
                print(f"📡 Testing communication with {self.serial_port}")
                with serial.Serial(self.serial_port, 115200, timeout=5) as ser:
                    time.sleep(2)  # Wait for boot
                    ser.write(b'\n')
                    time.sleep(1)
                    response = ser.read_all().decode('utf-8', errors='ignore')
                    
                    if response:
                        print(f"✅ Communication successful")
                        print(f"   Device response: {response[:100]}...")
                        return True
                    else:
                        print("⚠️  No response from device")
                        return False
                        
            except Exception as e:
                print(f"⚠️  Communication test failed: {e}")
        
        return True
    
    def generate_deployment_report(self) -> None:
        """Generate deployment report"""
        print("\n" + "="*60)
        print("📊 DEPLOYMENT REPORT")
        print("="*60)
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'scenario': self.selected_scenario,
            'board': self.detected_board,
            'serial_port': self.serial_port,
            'configuration': self.deployment_config.get('DEPLOYMENT_SCENARIO', 'unknown'),
            'status': 'completed'
        }
        
        print(f"Deployment Scenario: {report['scenario']}")
        print(f"Board Type: {report['board']}")
        print(f"Serial Port: {report['serial_port']}")
        print(f"Configuration: {report['configuration']}")
        print(f"Timestamp: {report['timestamp']}")
        
        # Save report
        report_file = self.script_dir / f"deployment_report_{int(time.time())}.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"\n📄 Report saved: {report_file}")
        
        # Print next steps
        print("\n" + "="*60)
        print("🚀 NEXT STEPS")
        print("="*60)
        print("1. Physically install the camera in the field")
        print("2. Configure network settings if using mesh/cellular")
        print("3. Test motion detection and image capture")
        print("4. Set up data collection schedule")
        print("5. Monitor system performance")
        
        if self.selected_scenario == 'research':
            print("\nResearch-specific next steps:")
            print("• Connect to research database")
            print("• Configure AI model validation")
            print("• Set up peer review workflow")
        elif self.selected_scenario == 'conservation':
            print("\nConservation-specific next steps:")
            print("• Train local team on basic maintenance")
            print("• Set up community data sharing")
            print("• Configure conservation alerts")
    
    def deploy(self, scenario: str, board: str = 'auto') -> bool:
        """Execute complete deployment process"""
        print("🌍 ESP32 Wildlife Camera Deployment")
        print("=" * 40)
        
        self.selected_scenario = scenario
        
        # Step 1: Board detection
        if board == 'auto':
            detected = self.detect_board()
            if not detected:
                print("❌ Board detection failed. Please specify board type manually.")
                return False
        else:
            self.detected_board = board
            print(f"📱 Using specified board: {board}")
        
        # Step 2: Configure firmware
        if not self.configure_firmware(scenario, self.detected_board):
            return False
        
        # Step 3: Build firmware
        if not self.build_firmware():
            return False
        
        # Step 4: Flash firmware
        if not self.flash_firmware():
            return False
        
        # Step 5: Validate deployment
        if not self.validate_deployment():
            print("⚠️  Validation issues detected, but deployment may still work")
        
        # Step 6: Generate report
        self.generate_deployment_report()
        
        print("\n🎉 Deployment completed successfully!")
        return True

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='ESP32 Wildlife Camera Quick Deployment Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --scenario research --board auto
  %(prog)s --scenario conservation --board ai-thinker
  %(prog)s --scenario urban --board esp32-s3-cam
  %(prog)s --list-scenarios
  %(prog)s --list-boards
        """
    )
    
    parser.add_argument('--scenario', 
                       choices=ESP32DeploymentManager.SUPPORTED_SCENARIOS,
                       help='Deployment scenario')
    
    parser.add_argument('--board',
                       choices=ESP32DeploymentManager.SUPPORTED_BOARDS,
                       default='auto',
                       help='Target board type (default: auto-detect)')
    
    parser.add_argument('--list-scenarios', action='store_true',
                       help='List supported deployment scenarios')
    
    parser.add_argument('--list-boards', action='store_true',
                       help='List supported board types')
    
    parser.add_argument('--detect-only', action='store_true',
                       help='Only detect connected boards, do not deploy')
    
    parser.add_argument('--validate-only', action='store_true',
                       help='Only validate configuration, do not build/flash')
    
    args = parser.parse_args()
    
    # Handle list commands
    if args.list_scenarios:
        print("Supported deployment scenarios:")
        for scenario in ESP32DeploymentManager.SUPPORTED_SCENARIOS:
            print(f"  • {scenario}")
        return 0
    
    if args.list_boards:
        print("Supported board types:")
        for board in ESP32DeploymentManager.SUPPORTED_BOARDS:
            print(f"  • {board}")
        return 0
    
    # Create deployment manager
    manager = ESP32DeploymentManager()
    
    # Handle detect-only
    if args.detect_only:
        detected = manager.detect_board()
        if detected:
            print(f"✅ Detected board: {detected}")
            return 0
        else:
            print("❌ No board detected")
            return 1
    
    # Handle validate-only
    if args.validate_only:
        if not args.scenario:
            print("❌ Scenario required for validation")
            return 1
        
        try:
            config = manager.load_scenario_config(args.scenario)
            print("✅ Configuration validation passed")
            return 0
        except Exception as e:
            print(f"❌ Configuration validation failed: {e}")
            return 1
    
    # Full deployment
    if not args.scenario:
        print("❌ Deployment scenario required. Use --help for options.")
        return 1
    
    try:
        success = manager.deploy(args.scenario, args.board)
        return 0 if success else 1
        
    except KeyboardInterrupt:
        print("\n🛑 Deployment interrupted by user")
        return 1
    except Exception as e:
        print(f"\n❌ Deployment failed: {e}")
        return 1

if __name__ == '__main__':
    sys.exit(main())