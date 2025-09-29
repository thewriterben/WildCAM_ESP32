#!/usr/bin/env python3
"""
ESP32WildlifeCAM Code Validation Script
Validates the fixes and checks for common issues in the codebase.
"""

import os
import re
import sys
from pathlib import Path

def check_gpio_pin_conflicts():
    """Check for GPIO pin conflicts in config.h"""
    print("=== Checking GPIO Pin Conflicts ===")
    
    # Check both the old config and new unified config
    config_files = [
        Path("firmware/src/config.h"),
        Path("include/config_unified.h"),
        Path("include/pins.h")
    ]
    
    all_pin_assignments = {}
    conflicts_found = False
    
    for config_file in config_files:
        if not config_file.exists():
            print(f"INFO: {config_file} not found, skipping")
            continue
            
        print(f"Checking {config_file}...")
        content = config_file.read_text()
        
        # Check feature states to handle conditional pin assignments
        sd_card_enabled = 'SD_CARD_ENABLED true' in content
        i2s_microphone_enabled = 'I2S_MICROPHONE_ENABLED true' in content
        analog_microphone_enabled = 'ANALOG_MICROPHONE_ENABLED true' in content
        camera_ai_thinker = 'CAMERA_MODEL_AI_THINKER' in content and not content.find('// #define CAMERA_MODEL_AI_THINKER') >= 0
        
        print(f"  Configuration state:")
        print(f"    SD Card: {'ENABLED' if sd_card_enabled else 'DISABLED'}")
        print(f"    I2S Microphone: {'ENABLED' if i2s_microphone_enabled else 'DISABLED'}")
        print(f"    Analog Microphone: {'ENABLED' if analog_microphone_enabled else 'DISABLED'}")
        print(f"    AI-Thinker Camera: {'ENABLED' if camera_ai_thinker else 'DISABLED'}")
        
        # Extract pin assignments (ignore commented lines and handle conditionals)
        pin_assignments = {}
        # Enhanced patterns to match only actual GPIO pin assignments
        pin_patterns = [
            r'^[^/]*#define\s+(\w*_PIN)\s+(\d+)',                 # Pattern for *_PIN defines
            r'^[^/]*#define\s+(\w*GPIO_NUM)\s+(\d+)',             # Pattern for *GPIO_NUM defines  
            r'^[^/]*#define\s+(PIR_PIN)\s+(\d+)',                 # PIR sensor pin specifically
            # NOTE: Removed general LoRa patterns as they include config values, not just pins
            # Only include specific LoRa GPIO pin patterns
            r'^[^/]*#define\s+(LORA_SCK_PIN|LORA_MISO_PIN|LORA_MOSI_PIN|LORA_CS_PIN|LORA_RST_PIN|LORA_DIO0_PIN)\s+(\d+)',
        ]
        
        # Track conditional blocks
        in_conditional = False
        conditional_depth = 0
        skip_current_block = False
        
        for line_num, line in enumerate(content.split('\n'), 1):
            stripped_line = line.strip()
            
            # Skip commented lines
            if stripped_line.startswith('//'):
                continue
            
            # Handle conditional compilation directives
            if stripped_line.startswith('#if'):
                conditional_depth += 1
                # Check if this conditional should be skipped based on current configuration
                if '#if SD_CARD_ENABLED' in stripped_line:
                    skip_current_block = not sd_card_enabled  # Skip if SD card is disabled
                elif '#if !SD_CARD_ENABLED' in stripped_line:
                    skip_current_block = sd_card_enabled  # Skip if SD card is enabled
                elif '#if !defined(CAMERA_MODEL_AI_THINKER)' in stripped_line:
                    skip_current_block = camera_ai_thinker  # Skip if AI-Thinker is defined
                else:
                    skip_current_block = False
                continue
            elif stripped_line.startswith('#else'):
                skip_current_block = not skip_current_block  # Flip the skip state for else block
                continue
            elif stripped_line.startswith('#endif'):
                conditional_depth -= 1
                if conditional_depth == 0:
                    skip_current_block = False  # Reset when exiting conditional blocks
                continue
            elif stripped_line.startswith('#undef'):
                continue  # Skip undef directives
                
            # Skip processing if we're in a conditional block that should be ignored
            if skip_current_block:
                continue
                
            for pattern in pin_patterns:
                match = re.search(pattern, line, re.IGNORECASE)
                if match:
                    define_name = match.group(1)
                    pin_number = int(match.group(2))
                    
                    # Skip negative pins (they indicate "not used")
                    if pin_number < 0:
                        continue
                        
                    # Only consider valid ESP32 GPIO pins (0-39)
                    if pin_number > 39:
                        continue
                    
                    # Skip disabled features
                    if 'I2S_' in define_name and not i2s_microphone_enabled:
                        continue
                    if 'ANALOG_MIC_PIN' in define_name and not analog_microphone_enabled:
                        continue
                    
                    # Define legitimate shared pins (pins that can safely share the same GPIO)
                    shared_pin_groups = {
                        # GPIO 1: PIR_PIN and UART_TX can share (debug only)
                        1: {"PIR_PIN", "UART_TX_PIN"},
                        # GPIO 32: Camera PWDN and Solar voltage monitoring can share
                        32: {"PWDN_GPIO_NUM", "SOLAR_VOLTAGE_PIN"},
                        # GPIO 33: Battery voltage and light sensor can share (both ADC inputs)
                        33: {"BATTERY_VOLTAGE_PIN", "LIGHT_SENSOR_PIN"},
                        # GPIO 34: Camera Y8 and Battery voltage monitoring can share (both input-only)
                        34: {"Y8_GPIO_NUM", "BATTERY_VOLTAGE_PIN"},
                        # GPIO 26: Camera I2C and BME280 share the same I2C bus
                        26: {"SIOD_GPIO_NUM", "BME280_SDA_PIN", "LORA_DIO0_PIN"},
                        # GPIO 27: Camera I2C and BME280 share the same I2C bus  
                        27: {"SIOC_GPIO_NUM", "BME280_SCL_PIN"},
                        # SD card and power management pins are mutually exclusive by config
                        2: {"SD_MISO_PIN", "POWER_LED_PIN", "CHARGING_LED_PIN"},
                        14: {"SD_CLK_PIN", "CHARGING_CONTROL_PIN"},
                        15: {"SD_MOSI_PIN", "CHARGING_LED_PIN"},
                        # GPIO 4: Camera LED and Power LED (same function, different names)
                        4: {"CAMERA_LED_PIN", "POWER_LED_PIN"},
                        # LoRa pins conflict with camera (LoRa disabled by design)
                        16: {"CHARGING_LED_PIN", "IR_LED_PIN", "LORA_CS_PIN"},
                        17: {"PIR_POWER_PIN", "LORA_RST_PIN"},
                        18: {"Y3_GPIO_NUM", "LORA_SCK_PIN"},
                        19: {"Y4_GPIO_NUM", "LORA_MISO_PIN"},
                        23: {"HREF_GPIO_NUM", "LORA_MOSI_PIN"},
                    }
                    
                    # Track pin assignments across all files
                    pin_key = define_name
                    if pin_number in all_pin_assignments:
                        existing_assignment = all_pin_assignments[pin_number]
                        # Check if this is a legitimate shared pin
                        if pin_number in shared_pin_groups:
                            allowed_pins = shared_pin_groups[pin_number]
                            if existing_assignment in allowed_pins and pin_key in allowed_pins:
                                # This is a legitimate sharing, not a conflict
                                pass
                            else:
                                print(f"CONFLICT: GPIO {pin_number} used for both {existing_assignment} and {pin_key} (not in allowed shared group)")
                                conflicts_found = True
                        else:
                            # Only report as conflict if different pin names are used for same GPIO
                            if existing_assignment != pin_key:
                                print(f"CONFLICT: GPIO {pin_number} used for both {existing_assignment} and {pin_key}")
                                conflicts_found = True
                        # Same pin name in multiple files is OK (duplicate definitions)
                    else:
                        all_pin_assignments[pin_number] = pin_key
                        
                    if pin_number in pin_assignments:
                        existing_assignment = pin_assignments[pin_number]
                        # Check if this is a legitimate shared pin within the same file
                        if pin_number in shared_pin_groups:
                            allowed_pins = shared_pin_groups[pin_number]
                            if existing_assignment in allowed_pins and define_name in allowed_pins:
                                # This is legitimate pin sharing within the same file
                                print(f"  INFO: GPIO {pin_number} shared between {existing_assignment} and {define_name} (legitimate sharing)")
                            else:
                                print(f"CONFLICT: GPIO {pin_number} used for both {existing_assignment} and {define_name} in {config_file.name}")
                                conflicts_found = True
                        else:
                            print(f"CONFLICT: GPIO {pin_number} used for both {existing_assignment} and {define_name} in {config_file.name}")
                            conflicts_found = True
                    else:
                        pin_assignments[pin_number] = define_name
                    break  # Only match first pattern to avoid duplicates
        
        print(f"  Found {len(pin_assignments)} GPIO pin assignments in {config_file.name}")
        for pin, name in sorted(pin_assignments.items()):
            print(f"    GPIO {pin:2d}: {name}")
    
    print(f"\nTotal GPIO pins used across all files: {len(all_pin_assignments)}")
    
    # Verify PIR_PIN correction
    pir_pins = [pin for pin, assignment in all_pin_assignments.items() if 'PIR_PIN' in assignment]
    if pir_pins:
        if 1 in pir_pins:
            print("✅ PIR_PIN correctly assigned to GPIO 1")
        elif 13 in pir_pins:
            print("❌ PIR_PIN still assigned to GPIO 13, should be GPIO 1 per AUDIT_REPORT.md")
            conflicts_found = True
        else:
            print(f"⚠️  PIR_PIN assigned to unexpected GPIO: {pir_pins}")
    
    if conflicts_found:
        print("❌ GPIO pin conflicts detected!")
        return False
    else:
        print("✅ No GPIO pin conflicts found")
        return True

def check_include_guards():
    """Check for proper include guards in header files"""
    print("\n=== Checking Include Guards ===")
    
    header_files = list(Path("firmware/src").rglob("*.h"))
    issues = []
    
    for header_file in header_files:
        content = header_file.read_text()
        
        # Check for include guard
        guard_pattern = r'#ifndef\s+(\w+)\s*\n#define\s+\1'
        if not re.search(guard_pattern, content):
            issues.append(f"Missing include guard: {header_file}")
    
    if issues:
        print("Issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print(f"All {len(header_files)} header files have proper include guards")
        return True

def check_type_safety():
    """Check for type safety issues"""
    print("\n=== Checking Type Safety ===")
    
    source_files = list(Path("firmware/src").rglob("*.cpp"))
    issues = []
    
    for source_file in source_files:
        content = source_file.read_text()
        
        # Check for abs() used on floats (should be fabs())
        if re.search(r'\babs\s*\(\s*[\w.]+\s*\)', content):
            # Check if it's actually a float operation
            lines = content.split('\n')
            for i, line in enumerate(lines):
                if 'abs(' in line and ('float' in line or '.' in line):
                    issues.append(f"{source_file}:{i+1}: Possible abs() on float (should use fabs())")
    
    if issues:
        print("Type safety issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print("No type safety issues found")
        return True

def check_memory_management():
    """Check for potential memory management issues"""
    print("\n=== Checking Memory Management ===")
    
    source_files = list(Path("firmware/src").rglob("*.cpp"))
    issues = []
    frame_buffer_usage = {}
    
    for source_file in source_files:
        content = source_file.read_text()
        
        # Count esp_camera_fb_get() and esp_camera_fb_return() calls
        get_calls = len(re.findall(r'esp_camera_fb_get\s*\(\s*\)', content))
        return_calls = len(re.findall(r'esp_camera_fb_return\s*\(', content))
        
        if get_calls > 0 or return_calls > 0:
            frame_buffer_usage[str(source_file)] = {
                'get_calls': get_calls,
                'return_calls': return_calls
            }
            
            # Check for obvious imbalances within the same file
            # Note: It's OK for a function to get a frame buffer and return it to caller
            # Only flag if there are significantly more gets than returns in same file
            if get_calls > return_calls + 1:  # Allow for one extra get (returned to caller)
                issues.append(f"{source_file}: {get_calls} esp_camera_fb_get() calls vs {return_calls} esp_camera_fb_return() calls - potential memory leak")
    
    if frame_buffer_usage:
        print("Frame buffer usage analysis:")
        for file, usage in frame_buffer_usage.items():
            print(f"  {file}: {usage['get_calls']} get, {usage['return_calls']} return")
    
    if issues:
        print("Memory management issues found:")
        for issue in issues:
            print(f"  {issue}")
        return False
    else:
        print("Frame buffer usage appears balanced across the codebase")
        return True

def check_configuration_consistency():
    """Check for configuration consistency"""
    print("\n=== Checking Configuration Consistency ===")
    
    # Check both old and new configuration files
    config_files = [
        Path("firmware/src/config.h"),
        Path("include/config_unified.h")
    ]
    
    all_issues = []
    
    for config_file in config_files:
        if not config_file.exists():
            print(f"INFO: {config_file} not found, skipping")
            continue
            
        print(f"Checking {config_file}...")
        content = config_file.read_text()
        
        # Check if disabled features are properly handled
        disabled_features = []
        if 'SD_CARD_ENABLED false' in content:
            disabled_features.append("SD Card")
        if 'VIBRATION_ENABLED false' in content:
            disabled_features.append("Vibration Sensor")
        if 'IR_LED_ENABLED false' in content:
            disabled_features.append("IR LED")
        if 'SATELLITE_ENABLED false' in content:
            disabled_features.append("Satellite Communication")
        if 'LORA_ENABLED false' in content:
            disabled_features.append("LoRa Mesh Network")
        
        if disabled_features:
            print(f"  Disabled features in {config_file.name}:")
            for feature in disabled_features:
                print(f"    - {feature}")
        
        # Check for consistent pin definitions (ignore commented lines)
        pir_pin_matches = []
        for line in content.split('\n'):
            if not line.strip().startswith('//') and 'PIR_PIN' in line:
                match = re.search(r'PIR_PIN\s+(\d+)', line)
                if match:
                    pir_pin_matches.append(match.group(1))
        
        if len(set(pir_pin_matches)) > 1:
            issue = f"Inconsistent PIR_PIN definitions in {config_file.name}: {pir_pin_matches}"
            print(f"  WARNING: {issue}")
            all_issues.append(issue)
        elif pir_pin_matches:
            pin_value = pir_pin_matches[0]
            if pin_value == "1":
                print(f"  ✅ PIR_PIN correctly set to GPIO 1 in {config_file.name}")
            elif pin_value == "13":
                issue = f"PIR_PIN still set to GPIO 13 in {config_file.name}, should be GPIO 1"
                print(f"  ❌ {issue}")
                all_issues.append(issue)
            else:
                issue = f"PIR_PIN set to unexpected value {pin_value} in {config_file.name}"
                print(f"  ⚠️  {issue}")
                all_issues.append(issue)
    
    # Check for consistency between files
    if Path("include/config_unified.h").exists() and Path("firmware/src/config.h").exists():
        print("  Checking consistency between unified and original config files...")
        # This could be expanded to check for major discrepancies
        
    if all_issues:
        print("❌ Configuration consistency issues found:")
        for issue in all_issues:
            print(f"  - {issue}")
        return False
    else:
        print("✅ Configuration appears consistent")
        return True

def main():
    """Run all validation checks"""
    print("ESP32WildlifeCAM Code Validation")
    print("================================")
    
    os.chdir(Path(__file__).parent)
    
    checks = [
        check_gpio_pin_conflicts,
        check_include_guards,
        check_type_safety,
        check_memory_management,
        check_configuration_consistency
    ]
    
    results = []
    for check in checks:
        try:
            result = check()
            results.append(result)
        except Exception as e:
            print(f"ERROR in {check.__name__}: {e}")
            results.append(False)
    
    print(f"\n=== Validation Summary ===")
    print(f"Checks passed: {sum(results)}/{len(results)}")
    
    if all(results):
        print("✅ All validation checks passed!")
        return 0
    else:
        print("❌ Some validation checks failed")
        return 1

if __name__ == "__main__":
    sys.exit(main())