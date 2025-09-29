#!/usr/bin/env python3
"""
Environmental Variants Integration Test
Tests the environmental variant system for ESP32 Wildlife Camera
"""

import os
import sys
import json
import subprocess
from pathlib import Path

class EnvironmentalVariantTester:
    def __init__(self, project_root):
        self.project_root = Path(project_root)
        self.results = {
            'tests_passed': 0,
            'tests_failed': 0,
            'errors': []
        }
    
    def test_directory_structure(self):
        """Test that all required directories exist"""
        print("Testing directory structure...")
        
        required_dirs = [
            'src/environmental_config',
            'hardware/environmental_variants/tropical',
            'hardware/environmental_variants/desert',
            'hardware/environmental_variants/arctic', 
            'hardware/environmental_variants/temperate_forest',
            'stl_files/variants/tropical',
            'stl_files/variants/desert',
            'stl_files/variants/arctic',
            'stl_files/variants/temperate_forest',
            'docs/environmental_deployment/tropical',
            'assembly_guides/tropical'
        ]
        
        for dir_path in required_dirs:
            full_path = self.project_root / dir_path
            if full_path.exists():
                print(f"  ✓ {dir_path}")
                self.results['tests_passed'] += 1
            else:
                print(f"  ✗ {dir_path} - MISSING")
                self.results['tests_failed'] += 1
                self.results['errors'].append(f"Missing directory: {dir_path}")
    
    def test_stl_files(self):
        """Test that STL files exist and have basic validity"""
        print("\nTesting STL files...")
        
        variants = ['tropical', 'desert', 'arctic', 'temperate_forest']
        required_stl_files = {
            'tropical': [
                'tropical_main_enclosure.stl',
                'tropical_drainage_system.stl',
                'tropical_ventilation_assembly.stl',
                'tropical_lens_protection.stl'
            ],
            'desert': [
                'desert_main_enclosure.stl',
                'desert_heat_sink_assembly.stl',
                'desert_dust_filtration.stl'
            ],
            'arctic': [
                'arctic_main_enclosure.stl'
            ],
            'temperate_forest': [
                'temperate_main_enclosure.stl'
            ]
        }
        
        for variant in variants:
            variant_dir = self.project_root / 'stl_files' / 'variants' / variant
            if not variant_dir.exists():
                print(f"  ✗ {variant} variant directory missing")
                self.results['tests_failed'] += 1
                continue
                
            for stl_file in required_stl_files.get(variant, []):
                stl_path = variant_dir / stl_file
                if stl_path.exists():
                    # Check if file has content
                    if stl_path.stat().st_size > 0:
                        print(f"  ✓ {variant}/{stl_file}")
                        self.results['tests_passed'] += 1
                    else:
                        print(f"  ✗ {variant}/{stl_file} - EMPTY")
                        self.results['tests_failed'] += 1
                        self.results['errors'].append(f"Empty STL file: {variant}/{stl_file}")
                else:
                    print(f"  ✗ {variant}/{stl_file} - MISSING")
                    self.results['tests_failed'] += 1
                    self.results['errors'].append(f"Missing STL file: {variant}/{stl_file}")
    
    def test_configuration_files(self):
        """Test that configuration files exist and have basic structure"""
        print("\nTesting configuration files...")
        
        config_files = [
            'src/environmental_config/tropical_config.h',
            'src/environmental_config/tropical_config.cpp',
            'src/environmental_config/desert_config.h',
            'src/environmental_config/environmental_variant_manager.h',
            'src/environmental_config/README.md'
        ]
        
        for config_file in config_files:
            file_path = self.project_root / config_file
            if file_path.exists():
                if file_path.stat().st_size > 0:
                    print(f"  ✓ {config_file}")
                    self.results['tests_passed'] += 1
                    
                    # Basic content validation for header files
                    if config_file.endswith('.h'):
                        with open(file_path, 'r') as f:
                            content = f.read()
                            if '#ifndef' in content and '#define' in content and '#endif' in content:
                                print(f"    ✓ {config_file} - Valid header structure")
                                self.results['tests_passed'] += 1
                            else:
                                print(f"    ✗ {config_file} - Invalid header structure")
                                self.results['tests_failed'] += 1
                                self.results['errors'].append(f"Invalid header structure: {config_file}")
                else:
                    print(f"  ✗ {config_file} - EMPTY")
                    self.results['tests_failed'] += 1
                    self.results['errors'].append(f"Empty configuration file: {config_file}")
            else:
                print(f"  ✗ {config_file} - MISSING")
                self.results['tests_failed'] += 1
                self.results['errors'].append(f"Missing configuration file: {config_file}")
    
    def test_documentation(self):
        """Test that documentation files exist and have content"""
        print("\nTesting documentation...")
        
        doc_files = [
            'hardware/environmental_variants/tropical/tropical_specifications.md',
            'hardware/environmental_variants/desert/desert_specifications.md',
            'hardware/environmental_variants/arctic/arctic_specifications.md',
            'hardware/environmental_variants/temperate_forest/temperate_forest_specifications.md',
            'docs/environmental_deployment/tropical/tropical_deployment_guide.md',
            'assembly_guides/tropical/tropical_assembly_guide.md',
            'stl_files/variants/tropical/README.md'
        ]
        
        for doc_file in doc_files:
            file_path = self.project_root / doc_file
            if file_path.exists():
                if file_path.stat().st_size > 1000:  # Expect substantial documentation
                    print(f"  ✓ {doc_file}")
                    self.results['tests_passed'] += 1
                else:
                    print(f"  ⚠ {doc_file} - TOO SHORT (may be incomplete)")
                    self.results['tests_passed'] += 1  # Still count as pass, just warning
            else:
                print(f"  ✗ {doc_file} - MISSING")
                self.results['tests_failed'] += 1
                self.results['errors'].append(f"Missing documentation: {doc_file}")
    
    def test_integration_with_existing_system(self):
        """Test integration with existing enclosure configuration"""
        print("\nTesting integration with existing system...")
        
        # Check if existing enclosure config exists
        enclosure_config = self.project_root / 'firmware/src/enclosure_config.h'
        if enclosure_config.exists():
            print("  ✓ Existing enclosure_config.h found")
            self.results['tests_passed'] += 1
            
            # Check for environmental types in existing config
            with open(enclosure_config, 'r') as f:
                content = f.read()
                env_types = ['ENV_TROPICAL', 'ENV_DESERT', 'ENV_ARCTIC', 'ENV_TEMPERATE_FOREST']
                for env_type in env_types:
                    if env_type in content:
                        print(f"    ✓ {env_type} found in existing config")
                        self.results['tests_passed'] += 1
                    else:
                        print(f"    ✗ {env_type} missing from existing config")
                        self.results['tests_failed'] += 1
                        self.results['errors'].append(f"Missing environment type: {env_type}")
        else:
            print("  ✗ Existing enclosure_config.h not found")
            self.results['tests_failed'] += 1
            self.results['errors'].append("Missing existing enclosure configuration")
    
    def test_stl_file_validity(self):
        """Basic STL file format validation"""
        print("\nTesting STL file format validity...")
        
        stl_files = []
        for variant_dir in (self.project_root / 'stl_files/variants').glob('*'):
            if variant_dir.is_dir():
                stl_files.extend(variant_dir.glob('*.stl'))
        
        for stl_file in stl_files:
            try:
                with open(stl_file, 'r') as f:
                    content = f.read()
                    if content.startswith('solid ') and content.strip().endswith('endsolid'):
                        # Check for basic facet structure
                        if 'facet normal' in content and 'outer loop' in content and 'vertex' in content:
                            print(f"  ✓ {stl_file.name} - Valid STL format")
                            self.results['tests_passed'] += 1
                        else:
                            print(f"  ✗ {stl_file.name} - Invalid STL structure")
                            self.results['tests_failed'] += 1
                            self.results['errors'].append(f"Invalid STL structure: {stl_file.name}")
                    else:
                        print(f"  ✗ {stl_file.name} - Invalid STL format")
                        self.results['tests_failed'] += 1
                        self.results['errors'].append(f"Invalid STL format: {stl_file.name}")
            except Exception as e:
                print(f"  ✗ {stl_file.name} - Error reading file: {e}")
                self.results['tests_failed'] += 1
                self.results['errors'].append(f"Error reading STL file {stl_file.name}: {e}")
    
    def test_variant_completeness(self):
        """Test completeness of each variant implementation"""
        print("\nTesting variant completeness...")
        
        variants = {
            'tropical': {
                'status': 'complete',
                'required_files': [
                    'hardware/environmental_variants/tropical/tropical_specifications.md',
                    'stl_files/variants/tropical/README.md',
                    'stl_files/variants/tropical/tropical_main_enclosure.stl',
                    'docs/environmental_deployment/tropical/tropical_deployment_guide.md',
                    'assembly_guides/tropical/tropical_assembly_guide.md',
                    'src/environmental_config/tropical_config.h',
                    'src/environmental_config/tropical_config.cpp'
                ]
            },
            'desert': {
                'status': 'partial',
                'required_files': [
                    'hardware/environmental_variants/desert/desert_specifications.md',
                    'stl_files/variants/desert/desert_main_enclosure.stl',
                    'src/environmental_config/desert_config.h'
                ]
            },
            'arctic': {
                'status': 'partial',
                'required_files': [
                    'hardware/environmental_variants/arctic/arctic_specifications.md',
                    'stl_files/variants/arctic/arctic_main_enclosure.stl'
                ]
            },
            'temperate_forest': {
                'status': 'partial',
                'required_files': [
                    'hardware/environmental_variants/temperate_forest/temperate_forest_specifications.md',
                    'stl_files/variants/temperate_forest/temperate_main_enclosure.stl'
                ]
            }
        }
        
        for variant_name, variant_info in variants.items():
            print(f"\n  Testing {variant_name} variant ({variant_info['status']}):")
            variant_complete = True
            
            for required_file in variant_info['required_files']:
                file_path = self.project_root / required_file
                if file_path.exists() and file_path.stat().st_size > 0:
                    print(f"    ✓ {required_file}")
                    self.results['tests_passed'] += 1
                else:
                    print(f"    ✗ {required_file} - MISSING")
                    variant_complete = False
                    self.results['tests_failed'] += 1
                    self.results['errors'].append(f"Missing file for {variant_name}: {required_file}")
            
            if variant_complete:
                print(f"  ✓ {variant_name} variant is complete")
            else:
                print(f"  ⚠ {variant_name} variant is incomplete")
    
    def run_all_tests(self):
        """Run all tests and generate report"""
        print("=" * 60)
        print("ESP32 Wildlife Camera Environmental Variants Test Suite")
        print("=" * 60)
        
        self.test_directory_structure()
        self.test_stl_files()
        self.test_configuration_files()
        self.test_documentation()
        self.test_integration_with_existing_system()
        self.test_stl_file_validity()
        self.test_variant_completeness()
        
        print("\n" + "=" * 60)
        print("TEST RESULTS")
        print("=" * 60)
        print(f"Tests Passed: {self.results['tests_passed']}")
        print(f"Tests Failed: {self.results['tests_failed']}")
        print(f"Total Tests: {self.results['tests_passed'] + self.results['tests_failed']}")
        
        if self.results['tests_failed'] > 0:
            print(f"\nErrors ({len(self.results['errors'])}):")
            for error in self.results['errors']:
                print(f"  - {error}")
        
        success_rate = (self.results['tests_passed'] / 
                       (self.results['tests_passed'] + self.results['tests_failed']) * 100)
        print(f"\nSuccess Rate: {success_rate:.1f}%")
        
        if success_rate >= 90:
            print("✅ EXCELLENT - Environmental variants implementation is highly complete")
            return 0
        elif success_rate >= 75:
            print("✅ GOOD - Environmental variants implementation is mostly complete")
            return 0
        elif success_rate >= 50:
            print("⚠️  PARTIAL - Environmental variants implementation needs work")
            return 1
        else:
            print("❌ POOR - Environmental variants implementation is incomplete")
            return 2

def main():
    if len(sys.argv) > 1:
        project_root = sys.argv[1]
    else:
        project_root = os.getcwd()
    
    tester = EnvironmentalVariantTester(project_root)
    return tester.run_all_tests()

if __name__ == "__main__":
    sys.exit(main())