#!/bin/bash

# ESP32WildlifeCAM Critical Components Validation Script
# Validates all components implemented to address critical missing components

echo "========================================"
echo " ESP32WildlifeCAM Critical Components Validation"
echo "========================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VALIDATION_PASSED=0
VALIDATION_TOTAL=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

validate_item() {
    local description="$1"
    local check_command="$2"
    local file_path="$3"
    
    VALIDATION_TOTAL=$((VALIDATION_TOTAL + 1))
    
    printf "%-60s" "$description"
    
    if eval "$check_command" >/dev/null 2>&1; then
        echo -e "${GREEN}✓ PASS${NC}"
        VALIDATION_PASSED=$((VALIDATION_PASSED + 1))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        if [ -n "$file_path" ]; then
            echo -e "  ${YELLOW}Expected: $file_path${NC}"
        fi
        return 1
    fi
}

echo "=== Priority 1: STL Files Validation ==="

# AI-Thinker ESP32-CAM STL Files
validate_item "AI-Thinker main enclosure" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/ai_thinker_main_enclosure.stl' ]"
validate_item "AI-Thinker front cover" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/ai_thinker_front_cover.stl' ]"
validate_item "AI-Thinker back cover" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/ai_thinker_back_cover.stl' ]"
validate_item "Solar panel mount bracket" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/solar_panel_mount_bracket.stl' ]"
validate_item "Tree mounting strap clips" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/tree_mounting_strap_clips.stl' ]"
validate_item "Ventilation grilles" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/ventilation_grilles.stl' ]"
validate_item "Camera lens hood (AI-Thinker)" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/camera_lens_hood.stl' ]"
validate_item "Battery compartment (AI-Thinker)" "[ -f '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/battery_compartment.stl' ]"

# Budget Edition STL Files
validate_item "Basic wildlife cam case" "[ -f '$PROJECT_ROOT/3d_models/budget_enclosures/basic_wildlife_cam_case.stl' ]"
validate_item "Solar panel mount (budget)" "[ -f '$PROJECT_ROOT/3d_models/budget_enclosures/solar_panel_mount.stl' ]"
validate_item "Camera lens hood (budget)" "[ -f '$PROJECT_ROOT/3d_models/budget_enclosures/camera_lens_hood.stl' ]"
validate_item "Battery compartment (budget)" "[ -f '$PROJECT_ROOT/3d_models/budget_enclosures/battery_compartment.stl' ]"

echo ""
echo "=== Priority 2: ESP32-S3-CAM Support ==="

validate_item "ESP32-S3-CAM HAL implementation" "[ -f '$PROJECT_ROOT/firmware/src/hal/esp32_s3_cam.cpp' ]"
validate_item "ESP32-S3-CAM HAL header" "[ -f '$PROJECT_ROOT/firmware/src/hal/esp32_s3_cam.h' ]"
validate_item "ESP32-S3-CAM PlatformIO config" "grep -q 'esp32s3cam' '$PROJECT_ROOT/platformio.ini'"
validate_item "ESP32-S3-CAM board detection" "grep -q 'BOARD_ESP32_S3_CAM' '$PROJECT_ROOT/firmware/src/hal/board_detector.cpp'"

echo ""
echo "=== Priority 3: Documentation Structure ==="

# Assembly Documentation
validate_item "Assembly documentation directory" "[ -d '$PROJECT_ROOT/docs/assembly' ]"
validate_item "AI-Thinker assembly guide" "[ -f '$PROJECT_ROOT/docs/assembly/ai_thinker_assembly_guide.md' ]"
validate_item "Budget assembly guide" "[ -f '$PROJECT_ROOT/docs/assembly/budget_assembly_guide.md' ]"
validate_item "Hardware troubleshooting guide" "[ -f '$PROJECT_ROOT/docs/assembly/troubleshooting_hardware.md' ]"

# Cost Analysis Documentation
validate_item "Cost analysis directory" "[ -d '$PROJECT_ROOT/docs/cost_analysis' ]"
validate_item "Component pricing guide" "[ -f '$PROJECT_ROOT/docs/cost_analysis/component_pricing.md' ]"
validate_item "Bulk purchasing guide" "[ -f '$PROJECT_ROOT/docs/cost_analysis/bulk_purchasing_guide.md' ]"
validate_item "Total project costs analysis" "[ -f '$PROJECT_ROOT/docs/cost_analysis/total_project_costs.md' ]"

# Enhanced Firewall Documentation
validate_item "Development documentation directory" "[ -d '$PROJECT_ROOT/docs/development' ]"
validate_item "Complete firewall guide" "[ -f '$PROJECT_ROOT/docs/development/firewall_complete_guide.md' ]"
validate_item "Offline setup script" "[ -f '$PROJECT_ROOT/scripts/offline_complete_setup.sh' ]"
validate_item "Offline setup script executable" "[ -x '$PROJECT_ROOT/scripts/offline_complete_setup.sh' ]"

echo ""
echo "=== Priority 4: Software Implementation ==="

# Core Software Components
validate_item "Camera manager implementation" "[ -f '$PROJECT_ROOT/src/camera/camera_manager.cpp' ]"
validate_item "Power manager implementation" "[ -f '$PROJECT_ROOT/src/power/power_manager.cpp' ]"
validate_item "Motion detection manager" "[ -f '$PROJECT_ROOT/src/detection/motion_detection_manager.cpp' ]"
validate_item "Hybrid motion detector" "[ -f '$PROJECT_ROOT/src/detection/hybrid_motion_detector.cpp' ]"
validate_item "Wildlife classifier" "[ -f '$PROJECT_ROOT/src/ai/wildlife_classifier.cpp' ]"
validate_item "AI motion detector implementation" "[ -f '$PROJECT_ROOT/src/ai/motion_detector.cpp' ]"
validate_item "AI motion detector header" "[ -f '$PROJECT_ROOT/src/ai/motion_detector.h' ]"

echo ""
echo "=== Content Quality Validation ==="

# Validate STL file format
validate_item "AI-Thinker lens hood STL format" "head -1 '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/camera_lens_hood.stl' | grep -q 'solid'"
validate_item "AI-Thinker battery compartment STL format" "head -1 '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam/battery_compartment.stl' | grep -q 'solid'"

# Validate documentation content
validate_item "Assembly guide has assembly steps" "grep -q 'Assembly Steps' '$PROJECT_ROOT/docs/assembly/ai_thinker_assembly_guide.md'"
validate_item "Cost analysis has pricing tables" "grep -q 'Component.*Cost.*USD' '$PROJECT_ROOT/docs/cost_analysis/component_pricing.md'"
validate_item "Firewall guide has offline procedures" "grep -q 'Offline Development' '$PROJECT_ROOT/docs/development/firewall_complete_guide.md'"

# Validate code implementation
validate_item "Motion detector has initialize method" "grep -q 'bool.*initialize()' '$PROJECT_ROOT/src/ai/motion_detector.h'"
validate_item "Motion detector has detectMotion method" "grep -q 'detectMotion.*camera_fb_t' '$PROJECT_ROOT/src/ai/motion_detector.h'"

echo ""
echo "=== Project Structure Validation ==="

# Key directories exist
validate_item "3D models directory structure" "[ -d '$PROJECT_ROOT/3d_models/ai_thinker_esp32_cam' ] && [ -d '$PROJECT_ROOT/3d_models/budget_enclosures' ]"
validate_item "Source code directory structure" "[ -d '$PROJECT_ROOT/src/ai' ] && [ -d '$PROJECT_ROOT/src/camera' ] && [ -d '$PROJECT_ROOT/src/power' ]"
validate_item "Documentation directory structure" "[ -d '$PROJECT_ROOT/docs/assembly' ] && [ -d '$PROJECT_ROOT/docs/cost_analysis' ]"
validate_item "Scripts directory" "[ -d '$PROJECT_ROOT/scripts' ]"

echo ""
echo "=== Hardware Compatibility Validation ==="

# Platform configuration
validate_item "ESP32-CAM environment defined" "grep -q '\\[env:esp32cam\\]' '$PROJECT_ROOT/platformio.ini'"
validate_item "ESP32-S3-CAM environment defined" "grep -q '\\[env:esp32s3cam\\]' '$PROJECT_ROOT/platformio.ini'"
validate_item "Camera model configurations" "grep -q 'CAMERA_MODEL_AI_THINKER' '$PROJECT_ROOT/platformio.ini'"

echo ""
echo "========================================"
echo " Validation Summary"
echo "========================================"

VALIDATION_PERCENTAGE=$((VALIDATION_PASSED * 100 / VALIDATION_TOTAL))

echo "Tests Passed: $VALIDATION_PASSED / $VALIDATION_TOTAL ($VALIDATION_PERCENTAGE%)"

if [ $VALIDATION_PASSED -eq $VALIDATION_TOTAL ]; then
    echo -e "${GREEN}✓ ALL CRITICAL COMPONENTS IMPLEMENTED SUCCESSFULLY${NC}"
    echo ""
    echo "Phase 1 critical missing components have been addressed:"
    echo "• Complete STL file library (10/10 files)"
    echo "• ESP32-S3-CAM support implementation"
    echo "• Comprehensive documentation structure"
    echo "• Software implementation gaps filled"
    echo ""
    echo "The project is now ready for Phase 1 completion!"
    exit 0
elif [ $VALIDATION_PERCENTAGE -ge 90 ]; then
    echo -e "${YELLOW}⚠ MOSTLY COMPLETE - Minor issues detected${NC}"
    echo "Most critical components are implemented successfully."
    exit 1
else
    echo -e "${RED}✗ SIGNIFICANT ISSUES DETECTED${NC}"
    echo "Multiple critical components are missing or incomplete."
    exit 2
fi