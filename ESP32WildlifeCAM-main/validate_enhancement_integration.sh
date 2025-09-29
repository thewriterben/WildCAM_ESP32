#!/bin/bash

# ESP32 Wildlife CAM Enhancement Checklist Integration Validation
# This script validates that all integration points mentioned in the checklists exist

echo "🔍 ESP32 Wildlife CAM Enhancement Checklist Integration Validation"
echo "=================================================================="

# Color codes for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

validation_passed=0
validation_total=0

check_file() {
    local file_path="$1"
    local description="$2"
    
    ((validation_total++))
    
    if [ -f "$file_path" ] || [ -d "$file_path" ]; then
        echo -e "${GREEN}✅ $description${NC}: $file_path"
        ((validation_passed++))
    else
        echo -e "${RED}❌ $description${NC}: $file_path (NOT FOUND)"
    fi
}

check_content() {
    local file_path="$1"
    local search_term="$2"
    local description="$3"
    
    ((validation_total++))
    
    if [ -f "$file_path" ] && grep -q "$search_term" "$file_path"; then
        echo -e "${GREEN}✅ $description${NC}: Found '$search_term' in $file_path"
        ((validation_passed++))
    else
        echo -e "${RED}❌ $description${NC}: '$search_term' not found in $file_path"
    fi
}

echo ""
echo "🔧 Core System Integration Points"
echo "--------------------------------"

# Power Management Integration Points
check_file "firmware/src/power_manager.cpp" "Power Manager Implementation"
check_file "src/power/power_manager.cpp" "Alternative Power Manager"
check_file "firmware/src/solar_manager.cpp" "Solar Manager Implementation"
check_content "firmware/src/config.h" "WIFI_ENABLED" "WiFi Configuration"

# Camera System Integration Points  
check_file "firmware/src/camera_handler.cpp" "Camera Handler Implementation"
check_content "firmware/src/config.h" "CAMERA_FRAME_SIZE" "Camera Configuration"

# Storage System Integration Points
check_file "src/data/storage_manager.cpp" "Storage Manager Implementation"
check_content "firmware/src/config.h" "SD_CARD_ENABLED" "SD Card Configuration"

# Motion Detection Integration Points
check_file "firmware/src/motion_filter.cpp" "Motion Filter Implementation"
check_content "firmware/src/motion_filter.cpp" "BME280" "Environmental Sensor Integration"

# AI System Integration Points
check_file "firmware/src/ai" "AI System Directory"
check_file "src/ai" "Alternative AI System Directory"
check_content "platformio.ini" "ESP32_AI_ENABLED" "AI Build Configuration"

# Network Integration Points
check_file "firmware/src/wifi_manager.cpp" "WiFi Manager Implementation"
check_file "firmware/src/lora_mesh.cpp" "LoRa Mesh Implementation"
check_file "firmware/src/cellular_manager.cpp" "Cellular Manager Implementation"

echo ""
echo "📋 Enhancement Checklist Files"
echo "------------------------------"

# Validate all created checklist files
check_file "docs/enhancement-checklists/POWER_OPTIMIZATION_CHECKLIST.md" "Power Optimization Checklist"
check_file "docs/enhancement-checklists/CAMERA_ENHANCEMENT_CHECKLIST.md" "Camera Enhancement Checklist"
check_file "docs/enhancement-checklists/storage_manager_checklist.h" "Storage Manager Checklist"
check_file "docs/enhancement-checklists/DATA_STRUCTURE_CHECKLIST.md" "Data Structure Checklist"
check_file "docs/enhancement-checklists/NETWORK_CHECKLIST.md" "Network Configuration Checklist"
check_file "docs/enhancement-checklists/REMOTE_ACCESS_CHECKLIST.md" "Remote Access Checklist"
check_file "docs/enhancement-checklists/motion_detection_checklist.cpp" "Motion Detection Checklist"
check_file "docs/enhancement-checklists/AI_FEATURES_CHECKLIST.md" "AI Features Checklist"
check_file "docs/enhancement-checklists/ENVIRONMENTAL_CHECKLIST.md" "Environmental Protection Checklist"
check_file "docs/enhancement-checklists/TEST_PROCEDURES_CHECKLIST.md" "Test Procedures Checklist"
check_file "docs/enhancement-checklists/DOCUMENTATION_CHECKLIST.md" "Documentation Checklist"
check_file "docs/enhancement-checklists/SUCCESS_METRICS.md" "Success Metrics"

# Main roadmap document
check_file "ESP32_WILDLIFE_CAM_ENHANCEMENT_ROADMAP.md" "Main Enhancement Roadmap"

echo ""
echo "📊 Validation Summary"
echo "===================="

echo -e "Total Checks: $validation_total"
echo -e "${GREEN}Passed: $validation_passed${NC}"

failed=$((validation_total - validation_passed))
if [ $failed -gt 0 ]; then
    echo -e "${RED}Failed: $failed${NC}"
else
    echo -e "${GREEN}All validations passed!${NC}"
fi

echo ""
echo "🎯 Integration Status"
echo "===================="

if [ $validation_passed -eq $validation_total ]; then
    echo -e "${GREEN}✅ COMPLETE: All enhancement checklists properly integrated with existing codebase${NC}"
    echo "🚀 Ready for implementation using the structured enhancement roadmap!"
else
    echo -e "${YELLOW}⚠️ PARTIAL: Some integration points may need verification${NC}"
    echo "📝 Review failed checks above for missing components"
fi

echo ""
echo "📖 Next Steps"
echo "============="
echo "1. Review the main roadmap: ESP32_WILDLIFE_CAM_ENHANCEMENT_ROADMAP.md"
echo "2. Select enhancement phase based on your deployment needs"
echo "3. Follow specific checklists in docs/enhancement-checklists/"
echo "4. Validate enhancements using existing debug and test infrastructure"

exit $failed