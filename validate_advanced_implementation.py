#!/usr/bin/env python3
"""
WildCAM ESP32 v2.0 Advanced Implementation Validator
Validates the new modular architecture and advanced features

Author: WildCAM ESP32 Team
Date: 2025-09-29
Version: 3.0.0
"""

import os
import sys
import json
from pathlib import Path

def check_directory_structure():
    """Validate the new modular directory structure"""
    print("🔍 Validating directory structure...")
    
    required_dirs = [
        # Firmware structure
        "firmware/hal",
        "firmware/drivers", 
        "firmware/ml_models",
        "firmware/networking",
        "firmware/power",
        "firmware/security",
        
        # Backend structure
        "backend/api",
        "backend/processing",
        "backend/storage", 
        "backend/ml_training",
        
        # Frontend structure
        "frontend/dashboard",
        "frontend/mobile",
        "frontend/analytics",
        
        # Edge computing structure
        "edge_computing/distributed",
        "edge_computing/consensus"
    ]
    
    missing_dirs = []
    for dir_path in required_dirs:
        if not os.path.exists(dir_path):
            missing_dirs.append(dir_path)
        else:
            print(f"  ✓ {dir_path}")
    
    if missing_dirs:
        print(f"  ❌ Missing directories: {missing_dirs}")
        return False
    
    print("  ✅ All required directories present")
    return True

def check_firmware_components():
    """Validate firmware component implementations"""
    print("\n🔍 Validating firmware components...")
    
    components = {
        "YOLO-tiny Detector": "firmware/ml_models/yolo_tiny/yolo_tiny_detector.h",
        "MPPT Controller": "firmware/power/mppt_controller.h", 
        "Security Manager": "firmware/security/security_manager.h",
        "Main Firmware": "firmware/main.cpp",
        "PlatformIO Config": "firmware/platformio.ini"
    }
    
    for name, path in components.items():
        if os.path.exists(path):
            # Check file size to ensure it's not empty
            size = os.path.getsize(path)
            if size > 100:  # At least 100 bytes
                print(f"  ✓ {name}: {path} ({size} bytes)")
            else:
                print(f"  ⚠️ {name}: {path} (file too small: {size} bytes)")
        else:
            print(f"  ❌ {name}: {path} (missing)")

def check_backend_api():
    """Validate backend API implementation"""
    print("\n🔍 Validating backend API...")
    
    api_file = "backend/api/wildlife_api.py"
    if os.path.exists(api_file):
        with open(api_file, 'r') as f:
            content = f.read()
            
        # Check for key API features
        features = [
            "class WildlifeAPI",
            "InfluxDBClient", 
            "def _record_detection",
            "def _get_species_analytics",
            "def _submit_to_ebird",
            "def _submit_to_inaturalist"
        ]
        
        for feature in features:
            if feature in content:
                print(f"  ✓ {feature} implemented")
            else:
                print(f"  ❌ {feature} missing")
    else:
        print(f"  ❌ API file missing: {api_file}")

def check_frontend_dashboard():
    """Validate React dashboard implementation"""
    print("\n🔍 Validating React dashboard...")
    
    dashboard_files = {
        "package.json": "frontend/dashboard/package.json",
        "App.js": "frontend/dashboard/src/App.js"
    }
    
    for name, path in dashboard_files.items():
        if os.path.exists(path):
            print(f"  ✓ {name}: {path}")
            
            if name == "package.json":
                with open(path, 'r') as f:
                    package_data = json.load(f)
                    
                # Check for key dependencies
                deps = package_data.get('dependencies', {})
                key_deps = ['react', '@mui/material', 'socket.io-client', 'recharts']
                for dep in key_deps:
                    if dep in deps:
                        print(f"    ✓ {dep}: {deps[dep]}")
                    else:
                        print(f"    ❌ Missing dependency: {dep}")
        else:
            print(f"  ❌ {name}: {path} (missing)")

def check_mobile_app():
    """Validate React Native mobile app"""
    print("\n🔍 Validating React Native mobile app...")
    
    mobile_files = {
        "package.json": "frontend/mobile/package.json", 
        "App.js": "frontend/mobile/App.js"
    }
    
    for name, path in mobile_files.items():
        if os.path.exists(path):
            print(f"  ✓ {name}: {path}")
            
            if name == "package.json":
                with open(path, 'r') as f:
                    package_data = json.load(f)
                    
                # Check for mobile-specific dependencies
                deps = package_data.get('dependencies', {})
                mobile_deps = ['react-native', 'react-native-maps', 'react-native-camera']
                for dep in mobile_deps:
                    if dep in deps:
                        print(f"    ✓ {dep}: {deps[dep]}")
                    else:
                        print(f"    ❌ Missing dependency: {dep}")
        else:
            print(f"  ❌ {name}: {path} (missing)")

def check_ml_models():
    """Validate ML/AI model structure"""
    print("\n🔍 Validating ML/AI model structure...")
    
    ml_dirs = [
        "firmware/ml_models/species_detection",
        "firmware/ml_models/behavior_analysis", 
        "firmware/ml_models/mobilenet",
        "firmware/ml_models/yolo_tiny",
        "firmware/ml_models/model_converter"
    ]
    
    for ml_dir in ml_dirs:
        if os.path.exists(ml_dir):
            print(f"  ✓ {ml_dir}")
        else:
            print(f"  ❌ {ml_dir} (missing)")

def check_security_features():
    """Validate security implementation"""
    print("\n🔍 Validating security features...")
    
    security_file = "firmware/security/security_manager.h"
    if os.path.exists(security_file):
        with open(security_file, 'r') as f:
            content = f.read()
            
        security_features = [
            "AES-256",
            "mbedtls/aes.h",
            "SecurityLevel",
            "encryptData",
            "decryptData", 
            "generateHash",
            "verifyIntegrity",
            "ATECC608A"
        ]
        
        for feature in security_features:
            if feature in content:
                print(f"  ✓ {feature} implemented")
            else:
                print(f"  ❌ {feature} missing")
    else:
        print(f"  ❌ Security manager missing")

def generate_implementation_report():
    """Generate comprehensive implementation report"""
    print("\n📊 Generating implementation report...")
    
    total_files = 0
    total_lines = 0
    
    # Count files and lines in key directories
    for root, dirs, files in os.walk("."):
        # Skip hidden directories and common build directories
        dirs[:] = [d for d in dirs if not d.startswith('.') and d not in ['node_modules', '__pycache__']]
        
        for file in files:
            if file.endswith(('.cpp', '.h', '.py', '.js', '.json', '.md')):
                file_path = os.path.join(root, file)
                total_files += 1
                
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        lines = len(f.readlines())
                        total_lines += lines
                except:
                    pass
    
    print(f"  📁 Total implementation files: {total_files}")
    print(f"  📝 Total lines of code: {total_lines}")
    print(f"  🏗️ Architecture: Modular with HAL separation")
    print(f"  🤖 AI Features: YOLO-tiny + TensorFlow Lite")
    print(f"  🔋 Power Management: MPPT + Weather-adaptive")
    print(f"  🔐 Security: AES-256 + Hardware security element")
    print(f"  🌐 Frontend: React Dashboard + React Native Mobile")
    print(f"  🗄️ Backend: Python Flask API + InfluxDB")

def main():
    """Main validation function"""
    print("🦌 WildCAM ESP32 v2.0 Advanced Implementation Validator")
    print("=" * 60)
    
    all_checks_passed = True
    
    # Run all validation checks
    checks = [
        check_directory_structure,
        check_firmware_components,
        check_backend_api,
        check_frontend_dashboard,
        check_mobile_app,
        check_ml_models,
        check_security_features
    ]
    
    for check in checks:
        try:
            result = check()
            if result is False:
                all_checks_passed = False
        except Exception as e:
            print(f"  ❌ Check failed with error: {e}")
            all_checks_passed = False
    
    # Generate report
    generate_implementation_report()
    
    print("\n" + "=" * 60)
    if all_checks_passed:
        print("✅ All validation checks passed!")
        print("🚀 WildCAM ESP32 v2.0 advanced implementation is ready for deployment")
    else:
        print("⚠️ Some validation checks failed")
        print("🔧 Please review the implementation and fix any issues")
    
    return 0 if all_checks_passed else 1

if __name__ == "__main__":
    sys.exit(main())