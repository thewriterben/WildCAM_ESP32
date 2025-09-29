#!/bin/bash
# Critical Priority Task Scripts for ESP32WildlifeCAM
# Automated setup for critical priority tasks from UNIFIED_DEVELOPMENT_PLAN.md
# Usage: ./scripts/priority_tasks.sh [task] [--setup] [--validate] [--progress]

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SETUP_MODE=false
VALIDATE_MODE=false
PROGRESS_MODE=false
SELECTED_TASK=""

# Priority task definitions (from UNIFIED_DEVELOPMENT_PLAN.md)
declare -A PRIORITY_TASKS=(
    ["1"]="Camera Driver Implementation"
    ["2"]="Power Management Implementation"
    ["3"]="Motion Detection Integration"
)

declare -A TASK_DESCRIPTIONS=(
    ["1"]="Implement ESP32-CAM OV2640 driver with frame buffer management"
    ["2"]="Implement battery monitoring, solar charging, and power optimization"
    ["3"]="Integrate PIR sensor with camera triggering and motion filtering"
)

declare -A TASK_FILES=(
    ["1"]="firmware/src/camera_handler.cpp,firmware/src/hal/camera_config.h"
    ["2"]="firmware/src/power_manager.cpp,firmware/src/solar_manager.cpp"
    ["3"]="firmware/src/motion_filter.cpp,firmware/src/multi_board/board_node.cpp"
)

declare -A TASK_DEPENDENCIES=(
    ["1"]="Hardware Abstraction Layer,GPIO Configuration"
    ["2"]="ADC Configuration,I2C Sensors"
    ["3"]="Camera Driver,PIR Sensor Configuration"
)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --setup|-s)
            SETUP_MODE=true
            shift
            ;;
        --validate|-v)
            VALIDATE_MODE=true
            shift
            ;;
        --progress|-p)
            PROGRESS_MODE=true
            shift
            ;;
        --help|-h)
            echo "Critical Priority Task Scripts for ESP32WildlifeCAM"
            echo "Usage: $0 [task_number] [--setup] [--validate] [--progress] [--help]"
            echo ""
            echo "Options:"
            echo "  --setup, -s      Set up development environment for task"
            echo "  --validate, -v   Validate task implementation status"
            echo "  --progress, -p   Show detailed progress tracking"
            echo "  --help, -h       Show this help message"
            echo ""
            echo "Tasks (from UNIFIED_DEVELOPMENT_PLAN.md):"
            echo "  1  Priority 1: Camera Driver Implementation"
            echo "  2  Priority 2: Power Management Implementation"
            echo "  3  Priority 3: Motion Detection Integration"
            echo "  all            Run all priority tasks"
            echo ""
            echo "Examples:"
            echo "  $0 1 --setup     # Set up for camera driver development"
            echo "  $0 2 --validate  # Check power management implementation"
            echo "  $0 all --progress # Show progress on all priority tasks"
            exit 0
            ;;
        [1-3]|all)
            SELECTED_TASK="$1"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Default to progress mode if no mode specified
if [[ "$SETUP_MODE" != "true" && "$VALIDATE_MODE" != "true" && "$PROGRESS_MODE" != "true" ]]; then
    PROGRESS_MODE=true
fi

# Default to all tasks if no specific task selected
if [[ -z "$SELECTED_TASK" ]]; then
    SELECTED_TASK="all"
fi

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_priority() {
    echo -e "${PURPLE}[PRIORITY]${NC} $1"
}

log_progress() {
    echo -e "${CYAN}[PROGRESS]${NC} $1"
}

# Progress tracking functions
get_task_progress() {
    local task_num=$1
    local task_files_str="${TASK_FILES[$task_num]}"
    local total_files=0
    local implemented_files=0
    
    if [[ -n "$task_files_str" ]]; then
        IFS=',' read -ra files <<< "$task_files_str"
        total_files=${#files[@]}
        
        for file in "${files[@]}"; do
            local full_path="$PROJECT_ROOT/$file"
            if [[ -f "$full_path" ]]; then
                # Check if file has substantial implementation (more than just TODOs)
                local line_count=$(wc -l < "$full_path" 2>/dev/null || echo "0")
                local todo_count=$(grep -c "TODO\|FIXME" "$full_path" 2>/dev/null || echo "0")
                local impl_lines=$((line_count - todo_count))
                
                if [[ $impl_lines -gt 20 ]]; then  # Substantial implementation
                    implemented_files=$((implemented_files + 1))
                fi
            fi
        done
    fi
    
    if [[ $total_files -gt 0 ]]; then
        echo "$implemented_files/$total_files"
    else
        echo "0/0"
    fi
}

get_task_percentage() {
    local task_num=$1
    local progress=$(get_task_progress "$task_num")
    local implemented=$(echo "$progress" | cut -d'/' -f1)
    local total=$(echo "$progress" | cut -d'/' -f2)
    
    if [[ $total -gt 0 && -n "$implemented" && -n "$total" ]]; then
        echo $(( implemented * 100 / total ))
    else
        echo "0"
    fi
}

# Task-specific setup functions
setup_camera_driver_task() {
    log_priority "Setting up Camera Driver Implementation (Priority 1)..."
    
    # Create camera-related directories
    mkdir -p "$PROJECT_ROOT/firmware/src/hal"
    mkdir -p "$PROJECT_ROOT/firmware/include/camera"
    
    # Create camera configuration header if it doesn't exist
    local camera_config_file="$PROJECT_ROOT/firmware/src/hal/camera_config.h"
    if [[ ! -f "$camera_config_file" ]]; then
        log_info "Creating camera configuration header..."
        cat > "$camera_config_file" << 'EOF'
/**
 * @file camera_config.h
 * @brief Camera configuration for ESP32-CAM
 * @author ESP32WildlifeCAM Project
 */

#ifndef CAMERA_CONFIG_H
#define CAMERA_CONFIG_H

#include "esp_camera.h"
#include "board_detector.h"

/**
 * Camera initialization configuration
 */
typedef struct {
    camera_config_t esp_config;
    bool auto_exposure;
    int quality;
    framesize_t frame_size;
    pixformat_t pixel_format;
} wildlife_camera_config_t;

/**
 * Initialize camera with board-specific configuration
 * @param config Pointer to camera configuration
 * @return ESP_OK on success, error code on failure
 */
esp_err_t wildlife_camera_init(wildlife_camera_config_t* config);

/**
 * Capture image with automatic settings
 * @return Camera frame buffer or NULL on failure
 */
camera_fb_t* wildlife_camera_capture(void);

/**
 * Cleanup camera resources
 */
void wildlife_camera_deinit(void);

/**
 * Set camera quality based on available power/storage
 * @param quality Quality level (0-63, lower is higher quality)
 */
void wildlife_camera_set_quality(int quality);

#endif // CAMERA_CONFIG_H
EOF
        log_success "Created camera configuration header"
    fi
    
    # Create camera handler implementation template if it doesn't exist
    local camera_handler_file="$PROJECT_ROOT/firmware/src/camera_handler.cpp"
    if [[ ! -f "$camera_handler_file" ]]; then
        log_info "Creating camera handler implementation template..."
        cat > "$camera_handler_file" << 'EOF'
/**
 * @file camera_handler.cpp
 * @brief Camera handler implementation for ESP32WildlifeCAM
 * @author ESP32WildlifeCAM Project
 */

#include "camera_handler.h"
#include "hal/camera_config.h"
#include "hal/board_detector.h"
#include "config.h"

// TODO: Implement camera initialization
esp_err_t camera_init() {
    // TODO: Get board-specific camera configuration
    // TODO: Initialize ESP32-CAM with OV2640 sensor
    // TODO: Configure frame buffer management
    // TODO: Set up automatic exposure and white balance
    return ESP_ERR_NOT_SUPPORTED;
}

// TODO: Implement camera capture
camera_fb_t* camera_capture() {
    // TODO: Capture image with current settings
    // TODO: Apply any post-processing filters
    // TODO: Validate frame buffer
    return nullptr;
}

// TODO: Implement camera cleanup
void camera_deinit() {
    // TODO: Release frame buffers
    // TODO: Cleanup camera resources
    // TODO: Power down camera module
}

// TODO: Implement power-aware quality adjustment
void camera_set_quality_for_power(int battery_percentage) {
    // TODO: Adjust quality based on available power
    // TODO: Reduce frame size if battery is low
    // TODO: Adjust compression settings
}

// TODO: Implement motion-triggered capture
camera_fb_t* camera_capture_on_motion(motion_event_t* motion_data) {
    // TODO: Wake camera from sleep mode
    // TODO: Adjust settings based on motion confidence
    // TODO: Capture image with metadata
    // TODO: Return to sleep mode if configured
    return nullptr;
}
EOF
        log_success "Created camera handler implementation template"
    fi
    
    # Create development validation script
    local validation_script="$PROJECT_ROOT/tools/validate_camera_dev.sh"
    mkdir -p "$PROJECT_ROOT/tools"
    cat > "$validation_script" << 'EOF'
#!/bin/bash
# Camera Development Validation Script

echo "Camera Driver Development Validation"
echo "===================================="

# Check camera configuration
if [[ -f "firmware/src/hal/camera_config.h" ]]; then
    echo "✅ Camera configuration header exists"
else
    echo "❌ Camera configuration header missing"
fi

# Check camera handler
if [[ -f "firmware/src/camera_handler.cpp" ]]; then
    echo "✅ Camera handler implementation exists"
    
    # Check for key functions
    if grep -q "camera_init\|camera_capture\|camera_deinit" "firmware/src/camera_handler.cpp"; then
        echo "✅ Key camera functions defined"
    else
        echo "❌ Key camera functions missing"
    fi
else
    echo "❌ Camera handler implementation missing"
fi

# Check GPIO configuration for camera
if python3 scripts/validate_component.py camera --simulation; then
    echo "✅ Camera component validation passed"
else
    echo "⚠️  Camera component validation issues"
fi
EOF
    chmod +x "$validation_script"
    
    log_info "Camera driver development environment ready"
    log_info "Next steps:"
    log_info "  1. Implement camera_init() in firmware/src/camera_handler.cpp"
    log_info "  2. Implement camera_capture() with frame buffer management"
    log_info "  3. Test with: python3 scripts/validate_component.py camera"
    log_info "  4. Validate with: ./tools/validate_camera_dev.sh"
}

setup_power_management_task() {
    log_priority "Setting up Power Management Implementation (Priority 2)..."
    
    # Create power management directories
    mkdir -p "$PROJECT_ROOT/firmware/src/power"
    mkdir -p "$PROJECT_ROOT/firmware/include/power"
    
    # Create power manager header if it doesn't exist
    local power_header_file="$PROJECT_ROOT/firmware/include/power/power_manager.h"
    if [[ ! -f "$power_header_file" ]]; then
        log_info "Creating power manager header..."
        cat > "$power_header_file" << 'EOF'
/**
 * @file power_manager.h
 * @brief Power management for ESP32WildlifeCAM
 * @author ESP32WildlifeCAM Project
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Power management states
 */
typedef enum {
    POWER_STATE_ACTIVE,
    POWER_STATE_LIGHT_SLEEP,
    POWER_STATE_DEEP_SLEEP,
    POWER_STATE_HIBERNATION
} power_state_t;

/**
 * Battery information structure
 */
typedef struct {
    float voltage;
    float percentage;
    bool charging;
    float solar_voltage;
    uint32_t estimated_runtime_hours;
} battery_info_t;

/**
 * Initialize power management system
 */
esp_err_t power_manager_init(void);

/**
 * Get current battery information
 */
battery_info_t power_manager_get_battery_info(void);

/**
 * Set power state
 */
esp_err_t power_manager_set_state(power_state_t state, uint32_t duration_ms);

/**
 * Check if system should enter power saving mode
 */
bool power_manager_should_save_power(void);

/**
 * Get recommended camera quality based on power level
 */
int power_manager_get_camera_quality(void);

#endif // POWER_MANAGER_H
EOF
        log_success "Created power manager header"
    fi
    
    # Create power manager implementation template
    local power_impl_file="$PROJECT_ROOT/firmware/src/power_manager.cpp"
    if [[ ! -f "$power_impl_file" ]]; then
        log_info "Creating power manager implementation template..."
        cat > "$power_impl_file" << 'EOF'
/**
 * @file power_manager.cpp
 * @brief Power management implementation
 * @author ESP32WildlifeCAM Project
 */

#include "power/power_manager.h"
#include "driver/adc.h"
#include "esp_sleep.h"
#include "config.h"

// TODO: Implement power manager initialization
esp_err_t power_manager_init(void) {
    // TODO: Configure ADC for battery voltage reading
    // TODO: Configure GPIO for solar panel monitoring
    // TODO: Set up charging LED control
    // TODO: Initialize power state tracking
    return ESP_ERR_NOT_SUPPORTED;
}

// TODO: Implement battery information reading
battery_info_t power_manager_get_battery_info(void) {
    battery_info_t info = {0};
    
    // TODO: Read battery voltage from ADC (GPIO 33)
    // TODO: Calculate battery percentage
    // TODO: Read solar panel voltage (GPIO 32)
    // TODO: Determine charging status
    // TODO: Estimate runtime based on current consumption
    
    return info;
}

// TODO: Implement power state management
esp_err_t power_manager_set_state(power_state_t state, uint32_t duration_ms) {
    // TODO: Configure appropriate sleep mode
    // TODO: Set wake-up sources (PIR, timer)
    // TODO: Save power state before sleep
    // TODO: Enter sleep mode
    return ESP_ERR_NOT_SUPPORTED;
}

// TODO: Implement power saving decision logic
bool power_manager_should_save_power(void) {
    // TODO: Check battery level
    // TODO: Check solar charging status
    // TODO: Consider time of day
    // TODO: Return recommendation
    return false;
}

// TODO: Implement camera quality recommendation
int power_manager_get_camera_quality(void) {
    // TODO: Get current battery level
    // TODO: Adjust quality based on power availability
    // TODO: Return quality setting (0-63)
    return 10; // Default medium quality
}
EOF
        log_success "Created power manager implementation template"
    fi
    
    # Create power development validation script
    local power_validation_script="$PROJECT_ROOT/tools/validate_power_dev.sh"
    cat > "$power_validation_script" << 'EOF'
#!/bin/bash
# Power Management Development Validation Script

echo "Power Management Development Validation"
echo "======================================"

# Check power manager files
if [[ -f "firmware/include/power/power_manager.h" ]]; then
    echo "✅ Power manager header exists"
else
    echo "❌ Power manager header missing"
fi

if [[ -f "firmware/src/power_manager.cpp" ]]; then
    echo "✅ Power manager implementation exists"
    
    # Check for key functions
    if grep -q "power_manager_init\|get_battery_info\|set_state" "firmware/src/power_manager.cpp"; then
        echo "✅ Key power management functions defined"
    else
        echo "❌ Key power management functions missing"
    fi
else
    echo "❌ Power manager implementation missing"
fi

# Check power component validation
if python3 scripts/validate_component.py power --simulation; then
    echo "✅ Power component validation passed"
else
    echo "⚠️  Power component validation issues"
fi

# Check GPIO configuration for power pins
if grep -q "BATTERY_VOLTAGE_PIN\|SOLAR_VOLTAGE_PIN" "firmware/src/config.h"; then
    echo "✅ Power GPIO pins configured"
else
    echo "❌ Power GPIO pins not configured"
fi
EOF
    chmod +x "$power_validation_script"
    
    log_info "Power management development environment ready"
    log_info "Next steps:"
    log_info "  1. Implement power_manager_init() with ADC configuration"
    log_info "  2. Implement battery voltage reading and percentage calculation"
    log_info "  3. Test with: python3 scripts/validate_component.py power"
    log_info "  4. Validate with: ./tools/validate_power_dev.sh"
}

setup_motion_detection_task() {
    log_priority "Setting up Motion Detection Integration (Priority 3)..."
    
    # Create motion detection directories
    mkdir -p "$PROJECT_ROOT/firmware/src/motion"
    mkdir -p "$PROJECT_ROOT/firmware/include/motion"
    
    # Create motion detection header if it doesn't exist
    local motion_header_file="$PROJECT_ROOT/firmware/include/motion/motion_detector.h"
    if [[ ! -f "$motion_header_file" ]]; then
        log_info "Creating motion detector header..."
        cat > "$motion_header_file" << 'EOF'
/**
 * @file motion_detector.h
 * @brief Motion detection for ESP32WildlifeCAM
 * @author ESP32WildlifeCAM Project
 */

#ifndef MOTION_DETECTOR_H
#define MOTION_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Motion event structure
 */
typedef struct {
    uint32_t timestamp;
    float confidence;
    uint16_t duration_ms;
    bool filtered;
    float environmental_factor;
} motion_event_t;

/**
 * Motion detection configuration
 */
typedef struct {
    uint8_t sensitivity;
    uint16_t debounce_ms;
    bool enable_filtering;
    float temperature_compensation;
} motion_config_t;

/**
 * Initialize motion detection system
 */
esp_err_t motion_detector_init(motion_config_t* config);

/**
 * Check for motion detection
 */
motion_event_t* motion_detector_check(void);

/**
 * Set motion detection sensitivity
 */
void motion_detector_set_sensitivity(uint8_t sensitivity);

/**
 * Enable/disable environmental filtering
 */
void motion_detector_set_filtering(bool enable);

#endif // MOTION_DETECTOR_H
EOF
        log_success "Created motion detector header"
    fi
    
    # Update motion filter implementation if it exists
    local motion_filter_file="$PROJECT_ROOT/firmware/src/motion_filter.cpp"
    if [[ -f "$motion_filter_file" ]]; then
        log_info "Motion filter implementation already exists"
    else
        log_info "Creating motion detection implementation template..."
        cat > "$motion_filter_file" << 'EOF'
/**
 * @file motion_filter.cpp
 * @brief Motion detection and filtering implementation
 * @author ESP32WildlifeCAM Project
 */

#include "motion/motion_detector.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"

static motion_config_t motion_config;
static bool motion_initialized = false;

// TODO: Implement motion detector initialization
esp_err_t motion_detector_init(motion_config_t* config) {
    if (config == nullptr) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // TODO: Configure PIR sensor GPIO (GPIO 1)
    // TODO: Set up interrupt for motion detection
    // TODO: Initialize filtering parameters
    // TODO: Configure environmental compensation
    
    motion_config = *config;
    motion_initialized = true;
    
    return ESP_OK;
}

// TODO: Implement motion detection check
motion_event_t* motion_detector_check(void) {
    if (!motion_initialized) {
        return nullptr;
    }
    
    // TODO: Read PIR sensor state
    // TODO: Apply debouncing
    // TODO: Calculate confidence based on duration
    // TODO: Apply environmental filtering
    // TODO: Return motion event if detected
    
    return nullptr;
}

// TODO: Implement sensitivity adjustment
void motion_detector_set_sensitivity(uint8_t sensitivity) {
    // TODO: Adjust PIR sensor sensitivity
    // TODO: Update filtering thresholds
    // TODO: Store new sensitivity value
    motion_config.sensitivity = sensitivity;
}

// TODO: Implement filtering control
void motion_detector_set_filtering(bool enable) {
    // TODO: Enable/disable environmental filtering
    // TODO: Update filtering algorithms
    motion_config.enable_filtering = enable;
}

// TODO: Implement camera trigger integration
esp_err_t motion_trigger_camera_capture(motion_event_t* motion) {
    // TODO: Wake camera from sleep
    // TODO: Adjust camera settings based on motion confidence
    // TODO: Trigger image capture
    // TODO: Store motion metadata with image
    return ESP_ERR_NOT_SUPPORTED;
}
EOF
        log_success "Created motion detection implementation template"
    fi
    
    # Create motion development validation script
    local motion_validation_script="$PROJECT_ROOT/tools/validate_motion_dev.sh"
    cat > "$motion_validation_script" << 'EOF'
#!/bin/bash
# Motion Detection Development Validation Script

echo "Motion Detection Development Validation"
echo "======================================"

# Check motion detector files
if [[ -f "firmware/include/motion/motion_detector.h" ]]; then
    echo "✅ Motion detector header exists"
else
    echo "❌ Motion detector header missing"
fi

if [[ -f "firmware/src/motion_filter.cpp" ]]; then
    echo "✅ Motion filter implementation exists"
    
    # Check for key functions
    if grep -q "motion_detector_init\|motion_detector_check\|motion_trigger" "firmware/src/motion_filter.cpp"; then
        echo "✅ Key motion detection functions defined"
    else
        echo "❌ Key motion detection functions missing"
    fi
else
    echo "❌ Motion filter implementation missing"
fi

# Check motion component validation
if python3 scripts/validate_component.py motion --simulation; then
    echo "✅ Motion component validation passed"
else
    echo "⚠️  Motion component validation issues"
fi

# Check PIR pin configuration
if grep -q "PIR_PIN.*1" "firmware/src/config.h"; then
    echo "✅ PIR sensor configured on correct GPIO"
else
    echo "❌ PIR sensor GPIO configuration issue"
fi

# Check integration with camera
if [[ -f "firmware/src/camera_handler.cpp" ]] && grep -q "motion" "firmware/src/camera_handler.cpp"; then
    echo "✅ Camera-motion integration detected"
else
    echo "⚠️  Camera-motion integration not yet implemented"
fi
EOF
    chmod +x "$motion_validation_script"
    
    log_info "Motion detection development environment ready"
    log_info "Next steps:"
    log_info "  1. Implement motion_detector_init() with PIR GPIO setup"
    log_info "  2. Implement motion detection with environmental filtering"
    log_info "  3. Integrate with camera triggering"
    log_info "  4. Test with: python3 scripts/validate_component.py motion"
    log_info "  5. Validate with: ./tools/validate_motion_dev.sh"
}

# Task validation functions
validate_camera_driver_task() {
    log_progress "Validating Camera Driver Implementation (Priority 1)..."
    
    local issues=0
    local total_checks=0
    
    # Check camera configuration header
    ((total_checks++))
    if [[ -f "$PROJECT_ROOT/firmware/src/hal/camera_config.h" ]]; then
        log_success "Camera configuration header exists"
    else
        log_error "Camera configuration header missing"
        ((issues++))
    fi
    
    # Check camera handler implementation
    ((total_checks++))
    if [[ -f "$PROJECT_ROOT/firmware/src/camera_handler.cpp" ]]; then
        log_success "Camera handler implementation exists"
        
        # Check implementation progress
        local func_count=$(grep -c "camera_init\|camera_capture\|camera_deinit" "$PROJECT_ROOT/firmware/src/camera_handler.cpp" 2>/dev/null || echo "0")
        local todo_count=$(grep -c "TODO:" "$PROJECT_ROOT/firmware/src/camera_handler.cpp" 2>/dev/null || echo "0")
        
        if [[ $func_count -ge 3 && $todo_count -lt $func_count ]]; then
            log_success "Camera functions implemented (${func_count} functions, ${todo_count} TODOs remaining)"
        else
            log_warning "Camera functions need implementation (${func_count} functions, ${todo_count} TODOs)"
        fi
    else
        log_error "Camera handler implementation missing"
        ((issues++))
    fi
    
    # Check component validation
    ((total_checks++))
    log_info "Running camera component validation..."
    if python3 "$PROJECT_ROOT/scripts/validate_component.py" camera --simulation >/dev/null 2>&1; then
        log_success "Camera component validation passed"
    else
        log_warning "Camera component validation has issues"
        ((issues++))
    fi
    
    # Progress calculation
    local progress_percentage=$(get_task_percentage "1")
    log_progress "Task 1 Progress: ${progress_percentage}% complete"
    
    if [[ $issues -eq 0 ]]; then
        log_success "Camera Driver Implementation validation passed"
        return 0
    else
        log_warning "Camera Driver Implementation has $issues issues"
        return 1
    fi
}

validate_power_management_task() {
    log_progress "Validating Power Management Implementation (Priority 2)..."
    
    local issues=0
    local total_checks=0
    
    # Check power manager header
    ((total_checks++))
    if [[ -f "$PROJECT_ROOT/firmware/include/power/power_manager.h" ]]; then
        log_success "Power manager header exists"
    else
        log_error "Power manager header missing"
        ((issues++))
    fi
    
    # Check power manager implementation
    ((total_checks++))
    if [[ -f "$PROJECT_ROOT/firmware/src/power_manager.cpp" ]]; then
        log_success "Power manager implementation exists"
        
        # Check implementation progress
        local func_count=$(grep -c "power_manager_init\|get_battery_info\|set_state" "$PROJECT_ROOT/firmware/src/power_manager.cpp" 2>/dev/null || echo "0")
        local todo_count=$(grep -c "TODO:" "$PROJECT_ROOT/firmware/src/power_manager.cpp" 2>/dev/null || echo "0")
        
        if [[ $func_count -ge 3 && $todo_count -lt $func_count ]]; then
            log_success "Power management functions implemented (${func_count} functions, ${todo_count} TODOs remaining)"
        else
            log_warning "Power management functions need implementation (${func_count} functions, ${todo_count} TODOs)"
        fi
    else
        log_error "Power manager implementation missing"
        ((issues++))
    fi
    
    # Check GPIO configuration for power
    ((total_checks++))
    if grep -q "BATTERY_VOLTAGE_PIN\|SOLAR_VOLTAGE_PIN" "$PROJECT_ROOT/firmware/src/config.h" 2>/dev/null; then
        log_success "Power GPIO pins configured"
    else
        log_warning "Power GPIO pins not configured"
        ((issues++))
    fi
    
    # Check component validation
    ((total_checks++))
    log_info "Running power component validation..."
    if python3 "$PROJECT_ROOT/scripts/validate_component.py" power --simulation >/dev/null 2>&1; then
        log_success "Power component validation passed"
    else
        log_warning "Power component validation has issues"
        ((issues++))
    fi
    
    # Progress calculation
    local progress_percentage=$(get_task_percentage "2")
    log_progress "Task 2 Progress: ${progress_percentage}% complete"
    
    if [[ $issues -eq 0 ]]; then
        log_success "Power Management Implementation validation passed"
        return 0
    else
        log_warning "Power Management Implementation has $issues issues"
        return 1
    fi
}

validate_motion_detection_task() {
    log_progress "Validating Motion Detection Integration (Priority 3)..."
    
    local issues=0
    local total_checks=0
    
    # Check motion detector header
    ((total_checks++))
    if [[ -f "$PROJECT_ROOT/firmware/include/motion/motion_detector.h" ]]; then
        log_success "Motion detector header exists"
    else
        log_error "Motion detector header missing"
        ((issues++))
    fi
    
    # Check motion filter implementation
    ((total_checks++))
    if [[ -f "$PROJECT_ROOT/firmware/src/motion_filter.cpp" ]]; then
        log_success "Motion filter implementation exists"
        
        # Check implementation progress
        local func_count=$(grep -c "motion_detector_init\|motion_detector_check\|motion_trigger" "$PROJECT_ROOT/firmware/src/motion_filter.cpp" 2>/dev/null || echo "0")
        local todo_count=$(grep -c "TODO:" "$PROJECT_ROOT/firmware/src/motion_filter.cpp" 2>/dev/null || echo "0")
        
        if [[ $func_count -ge 3 && $todo_count -lt $func_count ]]; then
            log_success "Motion detection functions implemented (${func_count} functions, ${todo_count} TODOs remaining)"
        else
            log_warning "Motion detection functions need implementation (${func_count} functions, ${todo_count} TODOs)"
        fi
    else
        log_error "Motion filter implementation missing"
        ((issues++))
    fi
    
    # Check PIR pin configuration
    ((total_checks++))
    if grep -q "PIR_PIN.*1" "$PROJECT_ROOT/firmware/src/config.h" 2>/dev/null; then
        log_success "PIR sensor configured on correct GPIO"
    else
        log_warning "PIR sensor GPIO configuration issue"
        ((issues++))
    fi
    
    # Check component validation
    ((total_checks++))
    log_info "Running motion component validation..."
    if python3 "$PROJECT_ROOT/scripts/validate_component.py" motion --simulation >/dev/null 2>&1; then
        log_success "Motion component validation passed"
    else
        log_warning "Motion component validation has issues"
        ((issues++))
    fi
    
    # Progress calculation
    local progress_percentage=$(get_task_percentage "3")
    log_progress "Task 3 Progress: ${progress_percentage}% complete"
    
    if [[ $issues -eq 0 ]]; then
        log_success "Motion Detection Integration validation passed"
        return 0
    else
        log_warning "Motion Detection Integration has $issues issues"
        return 1
    fi
}

# Show detailed progress for all tasks
show_detailed_progress() {
    echo ""
    echo "=== ESP32WildlifeCAM Priority Tasks Progress ==="
    echo "From UNIFIED_DEVELOPMENT_PLAN.md v2.0"
    echo ""
    
    for task_num in {1..3}; do
        local task_name="${PRIORITY_TASKS[$task_num]}"
        local task_desc="${TASK_DESCRIPTIONS[$task_num]}"
        local progress=$(get_task_progress "$task_num")
        local percentage=$(get_task_percentage "$task_num")
        
        echo "Priority $task_num: $task_name"
        echo "Description: $task_desc"
        echo "Files Progress: $progress"
        echo "Completion: ${percentage}%"
        
        # Show status
        if [[ $percentage -ge 80 ]]; then
            log_success "Near completion - ready for testing"
        elif [[ $percentage -ge 50 ]]; then
            log_warning "Partially implemented - needs completion"
        elif [[ $percentage -gt 0 ]]; then
            log_info "Implementation started - needs development"
        else
            log_error "Not started - needs setup and implementation"
        fi
        
        echo ""
    done
    
    # Calculate overall progress
    local overall_progress=0
    for task_num in {1..3}; do
        local task_percentage=$(get_task_percentage "$task_num")
        overall_progress=$((overall_progress + task_percentage))
    done
    overall_progress=$((overall_progress / 3))
    
    echo "Overall Priority Tasks Progress: ${overall_progress}%"
    echo ""
    
    # Recommendations based on progress
    echo "Next Steps Recommendations:"
    if [[ $overall_progress -lt 25 ]]; then
        echo "  1. Run: ./scripts/priority_tasks.sh 1 --setup"
        echo "  2. Run: ./scripts/priority_tasks.sh 2 --setup"
        echo "  3. Start implementing camera driver functions"
    elif [[ $overall_progress -lt 50 ]]; then
        echo "  1. Complete camera driver implementation"
        echo "  2. Begin power management implementation"
        echo "  3. Run component validations regularly"
    elif [[ $overall_progress -lt 75 ]]; then
        echo "  1. Finish power management implementation"
        echo "  2. Begin motion detection integration"
        echo "  3. Run integration tests"
    else
        echo "  1. Complete remaining implementations"
        echo "  2. Run full integration testing"
        echo "  3. Prepare for hardware testing"
    fi
    
    echo ""
    echo "Available Commands:"
    echo "  ./scripts/priority_tasks.sh [1-3] --setup      # Set up task environment"
    echo "  ./scripts/priority_tasks.sh [1-3] --validate   # Validate task progress"
    echo "  python3 scripts/validate_component.py [component] # Test components"
    echo "  ./scripts/integration_test.sh --simulation    # Full integration test"
}

# Main execution
main() {
    echo "ESP32WildlifeCAM Critical Priority Tasks"
    echo "======================================="
    echo "Based on UNIFIED_DEVELOPMENT_PLAN.md v2.0"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    if [[ "$SELECTED_TASK" == "all" ]]; then
        if [[ "$SETUP_MODE" == "true" ]]; then
            setup_camera_driver_task
            echo ""
            setup_power_management_task
            echo ""
            setup_motion_detection_task
        elif [[ "$VALIDATE_MODE" == "true" ]]; then
            validate_camera_driver_task
            echo ""
            validate_power_management_task
            echo ""
            validate_motion_detection_task
        else
            show_detailed_progress
        fi
    else
        case "$SELECTED_TASK" in
            "1")
                if [[ "$SETUP_MODE" == "true" ]]; then
                    setup_camera_driver_task
                elif [[ "$VALIDATE_MODE" == "true" ]]; then
                    validate_camera_driver_task
                else
                    echo "Priority 1: ${PRIORITY_TASKS[1]}"
                    echo "Progress: $(get_task_percentage "1")% complete"
                fi
                ;;
            "2")
                if [[ "$SETUP_MODE" == "true" ]]; then
                    setup_power_management_task
                elif [[ "$VALIDATE_MODE" == "true" ]]; then
                    validate_power_management_task
                else
                    echo "Priority 2: ${PRIORITY_TASKS[2]}"
                    echo "Progress: $(get_task_percentage "2")% complete"
                fi
                ;;
            "3")
                if [[ "$SETUP_MODE" == "true" ]]; then
                    setup_motion_detection_task
                elif [[ "$VALIDATE_MODE" == "true" ]]; then
                    validate_motion_detection_task
                else
                    echo "Priority 3: ${PRIORITY_TASKS[3]}"
                    echo "Progress: $(get_task_percentage "3")% complete"
                fi
                ;;
        esac
    fi
}

# Execute main function
main "$@"