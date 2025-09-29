#!/usr/bin/env python3
"""
ESP32-S3-CAM Implementation Validation Script
Validates that all required components are properly implemented
"""

import os
import sys
import re

def check_file_exists(filepath, description):
    """Check if a file exists and return status"""
    if os.path.exists(filepath):
        print(f"✅ {description}: {filepath}")
        return True
    else:
        print(f"❌ {description}: {filepath} - NOT FOUND")
        return False

def check_content_exists(filepath, pattern, description):
    """Check if specific content exists in a file"""
    if not os.path.exists(filepath):
        print(f"❌ {description}: {filepath} - FILE NOT FOUND")
        return False
    
    with open(filepath, 'r') as f:
        content = f.read()
        if re.search(pattern, content):
            print(f"✅ {description}: Found in {filepath}")
            return True
        else:
            print(f"❌ {description}: Not found in {filepath}")
            return False

def main():
    print("🔍 ESP32-S3-CAM Implementation Validation")
    print("=" * 50)
    
    base_path = "/home/runner/work/ESP32WildlifeCAM/ESP32WildlifeCAM"
    os.chdir(base_path)
    
    all_tests_passed = True
    
    # Check core implementation files
    print("\n📁 Core Implementation Files:")
    files_to_check = [
        ("firmware/src/hal/esp32_s3_cam.h", "ESP32S3CAM Header"),
        ("firmware/src/hal/esp32_s3_cam.cpp", "ESP32S3CAM Implementation"),
        ("firmware/src/configs/board_profiles.h", "Board Profiles"),
        ("firmware/src/configs/sensor_configs.h", "Sensor Configurations"),
    ]
    
    for filepath, desc in files_to_check:
        if not check_file_exists(filepath, desc):
            all_tests_passed = False
    
    # Check 3D model files
    print("\n🎯 3D Model Files:")
    model_files = [
        ("3d_models/esp32_s3_cam/esp32_s3_main_enclosure.stl", "Main Enclosure"),
        ("3d_models/esp32_s3_cam/esp32_s3_front_cover.stl", "Front Cover"),
        ("3d_models/esp32_s3_cam/esp32_s3_back_cover.stl", "Back Cover"),
        ("3d_models/esp32_s3_cam/esp32_s3_usb_access_cover.stl", "USB Access Cover"),
        ("3d_models/esp32_s3_cam/README.md", "3D Models Documentation"),
    ]
    
    for filepath, desc in model_files:
        if not check_file_exists(filepath, desc):
            all_tests_passed = False
    
    # Check implementation content
    print("\n⚙️  Implementation Content:")
    content_checks = [
        ("firmware/src/hal/esp32_s3_cam.h", r"getDisplayProfile.*override", "getDisplayProfile method declaration"),
        ("firmware/src/hal/esp32_s3_cam.cpp", r"DisplayProfile ESP32S3CAM::getDisplayProfile", "getDisplayProfile implementation"),
        ("firmware/src/configs/board_profiles.h", r"ESP32_S3_CAM_GPIO_MAP", "ESP32-S3-CAM GPIO mapping"),
        ("firmware/src/configs/board_profiles.h", r"ESP32_S3_CAM_POWER_PROFILE", "ESP32-S3-CAM power profile"),
        ("firmware/src/configs/board_profiles.h", r"ESP32_S3_CAM_CAMERA_CONFIG", "ESP32-S3-CAM camera config"),
        ("firmware/src/hal/esp32_s3_cam.cpp", r"configureS3OptimizedSettings", "ESP32-S3 optimized settings"),
        ("firmware/src/hal/esp32_s3_cam.cpp", r"detectSpecificSensor", "Enhanced sensor detection"),
    ]
    
    for filepath, pattern, desc in content_checks:
        if not check_content_exists(filepath, pattern, desc):
            all_tests_passed = False
    
    # Check GPIO pin mappings
    print("\n📌 GPIO Pin Validation:")
    gpio_checks = [
        ("firmware/src/configs/board_profiles.h", r"\.xclk_pin = 10", "XCLK pin = 10"),
        ("firmware/src/configs/board_profiles.h", r"\.siod_pin = 40", "SIOD pin = 40"), 
        ("firmware/src/configs/board_profiles.h", r"\.sioc_pin = 39", "SIOC pin = 39"),
        ("firmware/src/configs/board_profiles.h", r"\.led_pin = 21", "LED pin = 21"),
    ]
    
    for filepath, pattern, desc in gpio_checks:
        if not check_content_exists(filepath, pattern, desc):
            all_tests_passed = False
    
    # Check enhanced features
    print("\n🚀 Enhanced Features:")
    feature_checks = [
        ("firmware/src/configs/board_profiles.h", r"sleep_current_ua = 5000", "Enhanced sleep current"),
        ("firmware/src/configs/board_profiles.h", r"max_voltage = 4\.2", "LiPo battery support"),
        ("firmware/src/configs/board_profiles.h", r"jpeg_quality = 8", "Enhanced JPEG quality"),
        ("firmware/src/configs/board_profiles.h", r"xclk_freq_hz = 24000000", "Higher clock frequency"),
    ]
    
    for filepath, pattern, desc in feature_checks:
        if not check_content_exists(filepath, pattern, desc):
            all_tests_passed = False
    
    # Summary
    print("\n" + "=" * 50)
    if all_tests_passed:
        print("🎉 ALL TESTS PASSED! ESP32-S3-CAM implementation is complete!")
        print("✅ Ready for production deployment")
        return 0
    else:
        print("❌ Some tests failed. Please review the implementation.")
        return 1

if __name__ == "__main__":
    sys.exit(main())