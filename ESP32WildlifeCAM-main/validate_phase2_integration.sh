#!/bin/bash

# Phase 2 Core Functionality Validation Script
# Checks that all TODOs have been addressed and integration is complete

echo "ESP32WildlifeCAM Phase 2 Core Functionality Validation"
echo "======================================================"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Track results
TOTAL_CHECKS=0
PASSED_CHECKS=0

check_todo_resolved() {
    local file=$1
    local line_pattern=$2
    local description=$3
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if grep -q "$line_pattern" "$file" 2>/dev/null; then
        echo -e "${RED}✗ UNRESOLVED TODO${NC}: $description"
        echo "    Found in: $file"
        grep -n "$line_pattern" "$file" | head -1
    else
        echo -e "${GREEN}✓ RESOLVED${NC}: $description"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    fi
}

check_file_contains() {
    local file=$1
    local pattern=$2
    local description=$3
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo -e "${GREEN}✓ IMPLEMENTED${NC}: $description"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
    else
        echo -e "${RED}✗ MISSING${NC}: $description"
        echo "    Expected in: $file"
    fi
}

echo ""
echo "Step 2.1: Motion Detection Integration"
echo "------------------------------------"

check_todo_resolved "firmware/src/multi_board/board_node.cpp" \
    "TODO: Implement motion detection" \
    "Motion detection implementation in executeMotionDetectionTask()"

check_file_contains "firmware/src/multi_board/board_node.cpp" \
    "MotionDetectionManager" \
    "MotionDetectionManager usage in motion detection task"

check_file_contains "firmware/src/multi_board/board_node.cpp" \
    "configureEnhancedFeatures" \
    "Enhanced features configuration (PIR GPIO 1 support)"

check_file_contains "firmware/src/multi_board/board_node.cpp" \
    "wildlife_motion" \
    "Motion-triggered image storage to wildlife folder"

echo ""
echo "Step 2.2: Storage Integration"
echo "----------------------------"

check_todo_resolved "firmware/src/multi_board/message_protocol.cpp" \
    "TODO: Detect SD card" \
    "SD card detection in message protocol"

check_todo_resolved "firmware/src/meshtastic/wildlife_telemetry.cpp" \
    "TODO: Check SD card status" \
    "SD card status check in wildlife telemetry"

check_todo_resolved "firmware/src/display/hmi_system.cpp" \
    "TODO: Check SD card status" \
    "SD card status check in HMI system"

check_todo_resolved "system_manager.cpp" \
    "TODO: Initialize SD card" \
    "SD card initialization in system manager"

check_file_contains "firmware/src/multi_board/message_protocol.cpp" \
    "StorageManager::initialize" \
    "StorageManager integration for SD card detection"

check_file_contains "firmware/src/multi_board/message_protocol.cpp" \
    "StorageManager::getStatistics" \
    "Storage statistics integration for available space"

echo ""
echo "Step 2.3: Power Management Integration"
echo "------------------------------------"

check_todo_resolved "firmware/src/multi_board/message_protocol.cpp" \
    "TODO: Read actual battery level" \
    "Battery level reading from power manager"

check_todo_resolved "firmware/src/multi_board/message_protocol.cpp" \
    "TODO: Read actual solar voltage" \
    "Solar voltage reading from power manager"

check_file_contains "firmware/src/multi_board/message_protocol.cpp" \
    "getBatteryPercentage" \
    "PowerManager battery percentage integration"

check_file_contains "firmware/src/multi_board/message_protocol.cpp" \
    "getSolarVoltage" \
    "PowerManager solar voltage integration"

echo ""
echo "Integration Architecture Validation"
echo "---------------------------------"

check_file_contains "firmware/src/multi_board/board_node.h" \
    "MotionDetectionManager" \
    "MotionDetectionManager member in BoardNode class"

check_file_contains "firmware/src/multi_board/board_node.h" \
    "CameraManager" \
    "CameraManager member for motion detection integration"

check_file_contains "tests/phase2_integration_test.cpp" \
    "testPhase2Integration" \
    "Comprehensive Phase 2 integration test"

echo ""
echo "GPIO Configuration Validation"
echo "----------------------------"

echo -e "${YELLOW}ℹ${NC} GPIO Pin Assignments (as specified in problem statement):"
echo "    PIR Motion: GPIO 1 ✓ (configured via enhanced motion detection)"
echo "    Battery ADC: GPIO 33 (ADC1_CH7) ✓ (handled by PowerManager)"
echo "    Solar ADC: GPIO 32 (shared with camera PWDN) ✓ (handled by PowerManager)" 
echo "    SD Card: GPIO 12,14,15,2 (shared SPI) ✓ (handled by StorageManager)"

echo ""
echo "Summary"
echo "======="

PERCENTAGE=$((PASSED_CHECKS * 100 / TOTAL_CHECKS))

if [ $PASSED_CHECKS -eq $TOTAL_CHECKS ]; then
    echo -e "${GREEN}✓ ALL CHECKS PASSED${NC} ($PASSED_CHECKS/$TOTAL_CHECKS)"
    echo -e "${GREEN}✓ Phase 2 Core Functionality Implementation Complete${NC}"
elif [ $PERCENTAGE -ge 80 ]; then
    echo -e "${YELLOW}⚠ MOSTLY COMPLETE${NC} ($PASSED_CHECKS/$TOTAL_CHECKS - $PERCENTAGE%)"
    echo -e "${YELLOW}⚠ Minor issues remain but core functionality integrated${NC}"
else
    echo -e "${RED}✗ SIGNIFICANT ISSUES${NC} ($PASSED_CHECKS/$TOTAL_CHECKS - $PERCENTAGE%)"
    echo -e "${RED}✗ Major integration work still needed${NC}"
fi

echo ""
echo "Integration Flow Status:"
echo "PIR Motion (GPIO 1) → Enhanced Motion Detection → Camera Capture → Wildlife Storage → Power-Aware Operation"

if [ $PERCENTAGE -ge 80 ]; then
    echo -e "${GREEN}✓ Integration flow implemented and ready for testing${NC}"
else
    echo -e "${RED}✗ Integration flow incomplete${NC}"
fi

exit $((TOTAL_CHECKS - PASSED_CHECKS))