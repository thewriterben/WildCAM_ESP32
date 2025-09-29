#!/usr/bin/env python3
"""
Field Tuning Utilities for ESP32WildlifeCAM Motion Detection
Real-time parameter adjustment tools for wildlife-specific optimization

Usage: python3 scripts/field_tuning.py [options]
"""

import sys
import os
import time
import argparse
import json
import math
from pathlib import Path
from typing import Dict, List, Tuple, Optional, Any
from datetime import datetime

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

class WildlifeSpecificConfig:
    """Wildlife-specific motion detection configurations"""
    
    WILDLIFE_PROFILES = {
        'deer': {
            'size_range': (0.4, 0.9),
            'speed_range': (0.5, 4.0),
            'sensitivity': 0.7,
            'ml_threshold': 0.8,
            'preferred_zones': [1, 2],
            'active_hours': [(5, 9), (17, 21)]  # Dawn and dusk
        },
        'rabbit': {
            'size_range': (0.1, 0.3),
            'speed_range': (1.0, 6.0),
            'sensitivity': 0.9,
            'ml_threshold': 0.75,
            'preferred_zones': [2, 3],
            'active_hours': [(6, 10), (16, 20)]
        },
        'fox': {
            'size_range': (0.2, 0.5),
            'speed_range': (0.8, 5.0),
            'sensitivity': 0.8,
            'ml_threshold': 0.85,
            'preferred_zones': [1, 2, 3],
            'active_hours': [(20, 23), (4, 7)]  # Nocturnal
        },
        'bird': {
            'size_range': (0.05, 0.2),
            'speed_range': (2.0, 15.0),
            'sensitivity': 0.95,
            'ml_threshold': 0.7,
            'preferred_zones': [1, 2],
            'active_hours': [(5, 19)]  # Diurnal
        },
        'general': {
            'size_range': (0.1, 0.9),
            'speed_range': (0.5, 8.0),
            'sensitivity': 0.75,
            'ml_threshold': 0.8,
            'preferred_zones': [1, 2, 3],
            'active_hours': [(0, 24)]  # All day
        }
    }

class FieldTuningUtility:
    def __init__(self, verbose: bool = False, config_file: Optional[str] = None):
        self.verbose = verbose
        self.project_root = project_root
        self.config_file = config_file or str(project_root / "config" / "motion_tuning.json")
        self.current_config = self.load_config()
        self.tuning_session = {
            'start_time': datetime.now().isoformat(),
            'adjustments': [],
            'performance_history': []
        }
        
    def log_info(self, message: str):
        print(f"{Colors.BLUE}[TUNING]{Colors.NC} {message}")
        
    def log_success(self, message: str):
        print(f"{Colors.GREEN}[SUCCESS]{Colors.NC} {message}")
        
    def log_warning(self, message: str):
        print(f"{Colors.YELLOW}[WARNING]{Colors.NC} {message}")
        
    def log_error(self, message: str):
        print(f"{Colors.RED}[ERROR]{Colors.NC} {message}")
        
    def log_verbose(self, message: str):
        if self.verbose:
            print(f"{Colors.CYAN}[VERBOSE]{Colors.NC} {message}")

    def load_config(self) -> Dict[str, Any]:
        """Load current motion detection configuration"""
        config_path = Path(self.config_file)
        
        if config_path.exists():
            try:
                with open(config_path, 'r') as f:
                    config = json.load(f)
                self.log_verbose(f"Loaded config from {config_path}")
                return config
            except Exception as e:
                self.log_warning(f"Failed to load config: {e}")
        
        # Default configuration
        default_config = {
            'motion_detection': {
                'pir_sensitivity': 0.75,
                'frame_analysis_threshold': 0.6,
                'ml_confidence_threshold': 0.8,
                'multi_zone_enabled': True,
                'zones': {
                    '1': {'pin': 16, 'sensitivity': 0.7, 'priority': 0},
                    '2': {'pin': 17, 'sensitivity': 0.5, 'priority': 128},
                    '3': {'pin': 1, 'sensitivity': 0.3, 'priority': 255}
                }
            },
            'wildlife_profile': 'general',
            'environmental': {
                'wind_compensation': True,
                'temperature_adjustment': True,
                'lighting_adaptation': True
            },
            'performance': {
                'max_processing_time_ms': 500,
                'power_optimization': True,
                'adaptive_mode': True
            }
        }
        
        self.log_info("Using default configuration")
        return default_config

    def save_config(self):
        """Save current configuration to file"""
        try:
            config_path = Path(self.config_file)
            config_path.parent.mkdir(parents=True, exist_ok=True)
            
            with open(config_path, 'w') as f:
                json.dump(self.current_config, f, indent=2)
            
            self.log_success(f"Configuration saved to {config_path}")
        except Exception as e:
            self.log_error(f"Failed to save config: {e}")

    def set_wildlife_profile(self, wildlife_type: str) -> bool:
        """Set wildlife-specific detection profile"""
        if wildlife_type not in WildlifeSpecificConfig.WILDLIFE_PROFILES:
            self.log_error(f"Unknown wildlife type: {wildlife_type}")
            self.log_info(f"Available profiles: {list(WildlifeSpecificConfig.WILDLIFE_PROFILES.keys())}")
            return False
        
        profile = WildlifeSpecificConfig.WILDLIFE_PROFILES[wildlife_type]
        
        self.log_info(f"Applying {wildlife_type} detection profile...")
        
        # Update configuration
        self.current_config['wildlife_profile'] = wildlife_type
        self.current_config['motion_detection']['pir_sensitivity'] = profile['sensitivity']
        self.current_config['motion_detection']['ml_confidence_threshold'] = profile['ml_threshold']
        
        # Update zone configurations
        for zone_id, zone_config in self.current_config['motion_detection']['zones'].items():
            if int(zone_id) in profile['preferred_zones']:
                zone_config['sensitivity'] = profile['sensitivity']
            else:
                zone_config['sensitivity'] = max(0.3, profile['sensitivity'] - 0.2)
        
        self.log_success(f"{wildlife_type.title()} profile applied successfully")
        self.log_verbose(f"Size range: {profile['size_range']}")
        self.log_verbose(f"Speed range: {profile['speed_range']} m/s")
        self.log_verbose(f"Preferred zones: {profile['preferred_zones']}")
        
        # Log adjustment
        self.tuning_session['adjustments'].append({
            'timestamp': datetime.now().isoformat(),
            'action': 'set_wildlife_profile',
            'wildlife_type': wildlife_type,
            'profile_config': profile
        })
        
        return True

    def adjust_sensitivity(self, zone_id: Optional[int] = None, 
                          sensitivity: Optional[float] = None, 
                          delta: Optional[float] = None) -> bool:
        """Adjust PIR sensitivity for specific zone or all zones"""
        
        if sensitivity is not None and delta is not None:
            self.log_error("Cannot specify both absolute sensitivity and delta")
            return False
        
        if sensitivity is not None:
            if not (0.0 <= sensitivity <= 1.0):
                self.log_error("Sensitivity must be between 0.0 and 1.0")
                return False
        
        zones_to_adjust = []
        if zone_id is not None:
            if str(zone_id) not in self.current_config['motion_detection']['zones']:
                self.log_error(f"Zone {zone_id} not found")
                return False
            zones_to_adjust = [str(zone_id)]
        else:
            zones_to_adjust = list(self.current_config['motion_detection']['zones'].keys())
        
        for zone in zones_to_adjust:
            old_sensitivity = self.current_config['motion_detection']['zones'][zone]['sensitivity']
            
            if sensitivity is not None:
                new_sensitivity = sensitivity
            elif delta is not None:
                new_sensitivity = max(0.0, min(1.0, old_sensitivity + delta))
            else:
                self.log_error("Must specify either sensitivity or delta")
                return False
            
            self.current_config['motion_detection']['zones'][zone]['sensitivity'] = new_sensitivity
            
            self.log_success(f"Zone {zone} sensitivity: {old_sensitivity:.2f} → {new_sensitivity:.2f}")
            
        # Log adjustment
        self.tuning_session['adjustments'].append({
            'timestamp': datetime.now().isoformat(),
            'action': 'adjust_sensitivity',
            'zone_id': zone_id,
            'sensitivity': sensitivity,
            'delta': delta
        })
        
        return True

    def adjust_ml_threshold(self, threshold: Optional[float] = None, 
                           delta: Optional[float] = None) -> bool:
        """Adjust ML confidence threshold"""
        
        if threshold is not None and delta is not None:
            self.log_error("Cannot specify both absolute threshold and delta")
            return False
            
        if threshold is not None:
            if not (0.0 <= threshold <= 1.0):
                self.log_error("ML threshold must be between 0.0 and 1.0")
                return False
        
        old_threshold = self.current_config['motion_detection']['ml_confidence_threshold']
        
        if threshold is not None:
            new_threshold = threshold
        elif delta is not None:
            new_threshold = max(0.0, min(1.0, old_threshold + delta))
        else:
            self.log_error("Must specify either threshold or delta")
            return False
        
        self.current_config['motion_detection']['ml_confidence_threshold'] = new_threshold
        
        self.log_success(f"ML confidence threshold: {old_threshold:.2f} → {new_threshold:.2f}")
        
        # Log adjustment
        self.tuning_session['adjustments'].append({
            'timestamp': datetime.now().isoformat(),
            'action': 'adjust_ml_threshold',
            'old_threshold': old_threshold,
            'new_threshold': new_threshold
        })
        
        return True

    def optimize_for_environment(self, temperature: float, wind_speed: float, 
                               light_level: float) -> bool:
        """Optimize detection parameters for current environmental conditions"""
        
        self.log_info(f"Optimizing for environment:")
        self.log_info(f"  Temperature: {temperature:.1f}°C")
        self.log_info(f"  Wind speed: {wind_speed:.1f} km/h")
        self.log_info(f"  Light level: {light_level:.1f} lux")
        
        adjustments_made = []
        
        # Temperature-based adjustments
        if temperature < 5.0:  # Cold conditions
            # PIR sensors less sensitive in cold
            delta = 0.1
            self.adjust_sensitivity(delta=delta)
            adjustments_made.append(f"Cold weather: increased sensitivity by {delta}")
        elif temperature > 35.0:  # Hot conditions
            # PIR sensors more sensitive in heat
            delta = -0.1
            self.adjust_sensitivity(delta=delta)
            adjustments_made.append(f"Hot weather: decreased sensitivity by {delta}")
        
        # Wind-based adjustments
        if wind_speed > 15.0:  # High wind
            # Increase ML threshold to reduce false positives from vegetation
            delta = 0.05
            self.adjust_ml_threshold(delta=delta)
            adjustments_made.append(f"High wind: increased ML threshold by {delta}")
        elif wind_speed < 2.0:  # Calm conditions
            # Can lower threshold for better sensitivity
            delta = -0.05
            self.adjust_ml_threshold(delta=delta)
            adjustments_made.append(f"Calm conditions: decreased ML threshold by {delta}")
        
        # Light-based adjustments
        if light_level < 10.0:  # Low light/night
            # Increase PIR sensitivity for night detection
            delta = 0.05
            self.adjust_sensitivity(delta=delta)
            adjustments_made.append(f"Low light: increased sensitivity by {delta}")
        elif light_level > 50000.0:  # Very bright
            # May need to reduce sensitivity to avoid heat signatures
            delta = -0.05
            self.adjust_sensitivity(delta=delta)
            adjustments_made.append(f"Bright conditions: decreased sensitivity by {delta}")
        
        if adjustments_made:
            self.log_success("Environmental optimizations applied:")
            for adjustment in adjustments_made:
                self.log_success(f"  • {adjustment}")
        else:
            self.log_info("No environmental adjustments needed")
        
        # Log adjustment
        self.tuning_session['adjustments'].append({
            'timestamp': datetime.now().isoformat(),
            'action': 'optimize_for_environment',
            'temperature': temperature,
            'wind_speed': wind_speed,
            'light_level': light_level,
            'adjustments': adjustments_made
        })
        
        return True

    def run_calibration_wizard(self) -> bool:
        """Interactive calibration wizard"""
        self.log_info("Starting Motion Detection Calibration Wizard")
        self.log_info("=" * 50)
        
        try:
            # Step 1: Wildlife type selection
            self.log_info("Step 1: Select target wildlife")
            wildlife_options = list(WildlifeSpecificConfig.WILDLIFE_PROFILES.keys())
            
            print("Available wildlife profiles:")
            for i, wildlife in enumerate(wildlife_options, 1):
                profile = WildlifeSpecificConfig.WILDLIFE_PROFILES[wildlife]
                print(f"  {i}. {wildlife.title()}")
                print(f"     Size: {profile['size_range']}, Speed: {profile['speed_range']} m/s")
            
            while True:
                try:
                    choice = input(f"Select wildlife (1-{len(wildlife_options)}): ").strip()
                    if choice.isdigit() and 1 <= int(choice) <= len(wildlife_options):
                        selected_wildlife = wildlife_options[int(choice) - 1]
                        break
                    else:
                        print("Invalid choice. Please try again.")
                except KeyboardInterrupt:
                    self.log_info("\nCalibration cancelled by user")
                    return False
            
            self.set_wildlife_profile(selected_wildlife)
            
            # Step 2: Environmental conditions
            self.log_info("\nStep 2: Environmental conditions")
            
            try:
                temp = float(input("Current temperature (°C): ").strip())
                wind = float(input("Wind speed (km/h): ").strip())
                light = float(input("Light level (lux, estimated): ").strip())
                
                self.optimize_for_environment(temp, wind, light)
            except ValueError:
                self.log_warning("Invalid environmental data, using defaults")
            except KeyboardInterrupt:
                self.log_info("\nCalibration cancelled by user")
                return False
            
            # Step 3: Advanced tuning
            self.log_info("\nStep 3: Advanced tuning (optional)")
            
            try:
                advanced = input("Perform advanced tuning? (y/N): ").strip().lower()
                if advanced in ['y', 'yes']:
                    self.run_advanced_tuning()
            except KeyboardInterrupt:
                self.log_info("\nCalibration cancelled by user")
                return False
            
            # Step 4: Save configuration
            self.log_info("\nStep 4: Save configuration")
            
            try:
                save = input("Save configuration? (Y/n): ").strip().lower()
                if save not in ['n', 'no']:
                    self.save_config()
                    self.log_success("Calibration wizard completed successfully!")
                else:
                    self.log_info("Configuration not saved")
            except KeyboardInterrupt:
                self.log_info("\nCalibration cancelled by user")
                return False
            
            return True
            
        except Exception as e:
            self.log_error(f"Calibration wizard failed: {e}")
            return False

    def run_advanced_tuning(self):
        """Advanced tuning options"""
        while True:
            print("\nAdvanced Tuning Options:")
            print("1. Adjust zone sensitivity")
            print("2. Adjust ML confidence threshold")
            print("3. Zone priority configuration")
            print("4. View current configuration")
            print("5. Back to main calibration")
            
            try:
                choice = input("Select option (1-5): ").strip()
                
                if choice == '1':
                    self.interactive_sensitivity_adjustment()
                elif choice == '2':
                    self.interactive_ml_threshold_adjustment()
                elif choice == '3':
                    self.interactive_zone_priority()
                elif choice == '4':
                    self.display_current_config()
                elif choice == '5':
                    break
                else:
                    print("Invalid choice. Please try again.")
                    
            except KeyboardInterrupt:
                break

    def interactive_sensitivity_adjustment(self):
        """Interactive sensitivity adjustment"""
        zones = list(self.current_config['motion_detection']['zones'].keys())
        
        print(f"\nAvailable zones: {zones}")
        zone = input("Enter zone ID (or 'all' for all zones): ").strip()
        
        if zone.lower() == 'all':
            zone_id = None
        elif zone in zones:
            zone_id = int(zone)
        else:
            print("Invalid zone ID")
            return
        
        try:
            sensitivity = float(input("Enter new sensitivity (0.0-1.0): ").strip())
            self.adjust_sensitivity(zone_id=zone_id, sensitivity=sensitivity)
        except ValueError:
            print("Invalid sensitivity value")

    def interactive_ml_threshold_adjustment(self):
        """Interactive ML threshold adjustment"""
        current = self.current_config['motion_detection']['ml_confidence_threshold']
        print(f"\nCurrent ML threshold: {current:.2f}")
        
        try:
            threshold = float(input("Enter new ML threshold (0.0-1.0): ").strip())
            self.adjust_ml_threshold(threshold=threshold)
        except ValueError:
            print("Invalid threshold value")

    def interactive_zone_priority(self):
        """Interactive zone priority configuration"""
        print("\nZone Priority Configuration:")
        print("Priority: 0 = highest, 255 = lowest")
        
        for zone_id, zone_config in self.current_config['motion_detection']['zones'].items():
            current_priority = zone_config.get('priority', 128)
            print(f"Zone {zone_id} current priority: {current_priority}")
            
            try:
                new_priority = input(f"New priority for zone {zone_id} (0-255, Enter to skip): ").strip()
                if new_priority:
                    priority = int(new_priority)
                    if 0 <= priority <= 255:
                        zone_config['priority'] = priority
                        self.log_success(f"Zone {zone_id} priority updated to {priority}")
                    else:
                        print("Priority must be between 0 and 255")
            except ValueError:
                print("Invalid priority value")

    def display_current_config(self):
        """Display current configuration"""
        print("\nCurrent Motion Detection Configuration:")
        print("=" * 40)
        
        config = self.current_config['motion_detection']
        
        print(f"PIR Sensitivity: {config['pir_sensitivity']:.2f}")
        print(f"Frame Analysis Threshold: {config['frame_analysis_threshold']:.2f}")
        print(f"ML Confidence Threshold: {config['ml_confidence_threshold']:.2f}")
        print(f"Multi-zone Enabled: {config['multi_zone_enabled']}")
        print(f"Wildlife Profile: {self.current_config['wildlife_profile']}")
        
        print("\nZone Configuration:")
        for zone_id, zone_config in config['zones'].items():
            print(f"  Zone {zone_id}: Sensitivity={zone_config['sensitivity']:.2f}, "
                  f"Priority={zone_config.get('priority', 128)}, "
                  f"Pin=GPIO{zone_config['pin']}")

    def generate_tuning_report(self) -> str:
        """Generate tuning session report"""
        report = []
        report.append("=" * 60)
        report.append("ESP32WildlifeCAM Field Tuning Session Report")
        report.append(f"Session started: {self.tuning_session['start_time']}")
        report.append(f"Report generated: {datetime.now().isoformat()}")
        report.append("=" * 60)
        report.append("")
        
        report.append("CONFIGURATION ADJUSTMENTS:")
        if self.tuning_session['adjustments']:
            for i, adjustment in enumerate(self.tuning_session['adjustments'], 1):
                report.append(f"{i}. {adjustment['timestamp']}")
                report.append(f"   Action: {adjustment['action']}")
                for key, value in adjustment.items():
                    if key not in ['timestamp', 'action']:
                        report.append(f"   {key}: {value}")
                report.append("")
        else:
            report.append("  No adjustments made during this session")
            report.append("")
        
        report.append("FINAL CONFIGURATION:")
        config = self.current_config['motion_detection']
        report.append(f"  Wildlife Profile: {self.current_config['wildlife_profile']}")
        report.append(f"  PIR Sensitivity: {config['pir_sensitivity']:.2f}")
        report.append(f"  ML Confidence Threshold: {config['ml_confidence_threshold']:.2f}")
        report.append(f"  Multi-zone Enabled: {config['multi_zone_enabled']}")
        report.append("")
        
        report.append("ZONE CONFIGURATION:")
        for zone_id, zone_config in config['zones'].items():
            report.append(f"  Zone {zone_id}:")
            report.append(f"    GPIO Pin: {zone_config['pin']}")
            report.append(f"    Sensitivity: {zone_config['sensitivity']:.2f}")
            report.append(f"    Priority: {zone_config.get('priority', 128)}")
        
        return "\n".join(report)

def main():
    parser = argparse.ArgumentParser(description='ESP32WildlifeCAM Field Tuning Utility')
    parser.add_argument('--wildlife', '-w', choices=list(WildlifeSpecificConfig.WILDLIFE_PROFILES.keys()),
                       help='Set wildlife profile')
    parser.add_argument('--sensitivity', '-s', type=float, metavar='0.0-1.0',
                       help='Set PIR sensitivity')
    parser.add_argument('--ml-threshold', '-m', type=float, metavar='0.0-1.0',
                       help='Set ML confidence threshold')
    parser.add_argument('--zone', '-z', type=int,
                       help='Target specific zone (for sensitivity adjustment)')
    parser.add_argument('--temperature', '-t', type=float,
                       help='Current temperature for environmental optimization')
    parser.add_argument('--wind', type=float,
                       help='Current wind speed for environmental optimization')
    parser.add_argument('--light', type=float,
                       help='Current light level for environmental optimization')
    parser.add_argument('--wizard', action='store_true',
                       help='Run interactive calibration wizard')
    parser.add_argument('--config', '-c', type=str,
                       help='Configuration file path')
    parser.add_argument('--save', action='store_true',
                       help='Save configuration after adjustments')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Enable verbose output')
    parser.add_argument('--output', '-o', type=str,
                       help='Output tuning report to file')
    
    args = parser.parse_args()
    
    tuner = FieldTuningUtility(verbose=args.verbose, config_file=args.config)
    
    print(f"ESP32WildlifeCAM Field Tuning Utility")
    print(f"====================================")
    print(f"Config file: {tuner.config_file}")
    print("")
    
    if args.wizard:
        # Run interactive calibration wizard
        success = tuner.run_calibration_wizard()
        if not success:
            return 1
    else:
        # Apply command-line arguments
        changes_made = False
        
        if args.wildlife:
            if tuner.set_wildlife_profile(args.wildlife):
                changes_made = True
        
        if args.sensitivity is not None:
            if tuner.adjust_sensitivity(zone_id=args.zone, sensitivity=args.sensitivity):
                changes_made = True
        
        if args.ml_threshold is not None:
            if tuner.adjust_ml_threshold(threshold=args.ml_threshold):
                changes_made = True
        
        if all(x is not None for x in [args.temperature, args.wind, args.light]):
            if tuner.optimize_for_environment(args.temperature, args.wind, args.light):
                changes_made = True
        
        if args.save and changes_made:
            tuner.save_config()
        elif changes_made:
            save = input("Save configuration changes? (Y/n): ").strip().lower()
            if save not in ['n', 'no']:
                tuner.save_config()
    
    # Display current configuration
    tuner.display_current_config()
    
    # Generate report if requested
    if args.output:
        report = tuner.generate_tuning_report()
        Path(args.output).write_text(report)
        print(f"\nTuning report saved to: {args.output}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())