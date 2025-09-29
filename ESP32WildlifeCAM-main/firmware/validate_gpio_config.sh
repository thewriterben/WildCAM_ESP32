#!/bin/bash
# GPIO Configuration Validator
# Pin assignment verification tool for ESP32 Wildlife Camera
# Usage: ./validate_gpio_config.sh [--verbose] [--fix] [--board <type>]

set -e

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERBOSE=false
FIX_ISSUES=false
BOARD_TYPE="esp32s3"
START_TIME=$(date +%s)

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TESTS_PASSED=0
TESTS_TOTAL=0

# ESP32 GPIO constraints and capabilities
declare -A GPIO_CONSTRAINTS=(
    # Input-only pins (cannot be outputs)
    [34]="input_only"
    [35]="input_only"
    [36]="input_only"
    [39]="input_only"
    
    # Strapping pins (special boot behavior)
    [0]="strapping"   # Boot mode
    [2]="strapping"   # Boot mode
    [5]="strapping"   # VSPI SS
    [12]="strapping"  # Flash voltage
    [15]="strapping"  # Boot silence
    
    # High-speed interfaces
    [6]="flash_spi"   # Flash SPI
    [7]="flash_spi"   # Flash SPI
    [8]="flash_spi"   # Flash SPI
    [9]="flash_spi"   # Flash SPI
    [10]="flash_spi"  # Flash SPI
    [11]="flash_spi"  # Flash SPI
    
    # ADC2 pins (conflicts with WiFi)
    [0]="adc2_wifi_conflict"
    [2]="adc2_wifi_conflict"
    [4]="adc2_wifi_conflict"
    [12]="adc2_wifi_conflict"
    [13]="adc2_wifi_conflict"
    [14]="adc2_wifi_conflict"
    [15]="adc2_wifi_conflict"
    [25]="adc2_wifi_conflict"
    [26]="adc2_wifi_conflict"
    [27]="adc2_wifi_conflict"
)

# ESP32-S3 specific constraints
declare -A ESP32S3_CONSTRAINTS=(
    # USB pins
    [19]="usb_dm"
    [20]="usb_dp"
    
    # Additional strapping pins
    [3]="strapping"   # JTAG enable
    [45]="strapping"  # Boot mode
    [46]="strapping"  # Boot mode
    
    # PSRAM pins (if using octal PSRAM)
    [26]="psram_octal"
    [27]="psram_octal"
    [28]="psram_octal"
    [29]="psram_octal"
    [30]="psram_octal"
    [31]="psram_octal"
    [32]="psram_octal"
)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --board|-b)
            BOARD_TYPE="$2"
            shift 2
            ;;
        --fix|-f)
            FIX_ISSUES=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "GPIO Configuration Validator"
            echo "Usage: $0 [--board <type>] [--fix] [--verbose] [--help]"
            echo ""
            echo "Options:"
            echo "  --board, -b       ESP32 board type (esp32|esp32s2|esp32s3|esp32c3)"
            echo "  --fix, -f         Attempt to fix GPIO conflicts"
            echo "  --verbose, -v     Enable verbose output"
            echo "  --help, -h        Show this help message"
            echo ""
            echo "This script validates:"
            echo "  • GPIO pin conflicts and overlaps"
            echo "  • Camera pin assignments"
            echo "  • I2C/SPI pin configurations"
            echo "  • Power management pins"
            echo "  • LoRa module pin assignments"
            echo "  • ESP32 hardware constraints"
            echo "  • Board-specific limitations"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
    ((TESTS_PASSED++))
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_verbose() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo -e "${BLUE}[VERBOSE]${NC} $1"
    fi
}

test_result() {
    local result=$1
    local description="$2"
    
    ((TESTS_TOTAL++))
    
    if [[ $result -eq 0 ]]; then
        log_success "$description"
    else
        log_error "$description"
    fi
}

# Get configuration files
get_config_files() {
    local config_files=()
    
    local possible_configs=(
        "$PROJECT_ROOT/firmware/src/config.h"
        "$PROJECT_ROOT/include/config_unified.h"
        "$PROJECT_ROOT/include/pins.h"
        "$PROJECT_ROOT/firmware/include/config.h"
    )
    
    for config_file in "${possible_configs[@]}"; do
        if [[ -f "$config_file" ]]; then
            config_files+=("$config_file")
        fi
    done
    
    printf '%s\n' "${config_files[@]}"
}

# Parse GPIO assignments from configuration files
parse_gpio_assignments() {
    local config_file="$1"
    local -A pin_assignments=()
    
    if [[ ! -f "$config_file" ]]; then
        return 1
    fi
    
    log_verbose "Parsing GPIO assignments from: $(basename "$config_file")"
    
    # Read file and extract GPIO pin definitions
    while IFS= read -r line; do
        # Skip comments and empty lines
        [[ "$line" =~ ^[[:space:]]*// ]] && continue
        [[ "$line" =~ ^[[:space:]]*$ ]] && continue
        
        # Match GPIO pin definitions
        if [[ "$line" =~ ^[[:space:]]*#define[[:space:]]+([A-Z_]+)[[:space:]]+([0-9]+) ]]; then
            local pin_name="${BASH_REMATCH[1]}"
            local pin_number="${BASH_REMATCH[2]}"
            
            # Only process GPIO pin definitions
            if [[ "$pin_name" =~ _PIN$|_GPIO_NUM$|GPIO ]]; then
                pin_assignments["$pin_number"]+="$pin_name "
                log_verbose "  GPIO $pin_number: $pin_name"
            fi
        fi
    done < "$config_file"
    
    # Output assignments
    for pin in "${!pin_assignments[@]}"; do
        echo "$pin:${pin_assignments[$pin]}"
    done
}

# Check for GPIO pin conflicts
check_gpio_conflicts() {
    log_info "Checking GPIO pin conflicts..."
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    if [[ ${#config_files[@]} -eq 0 ]]; then
        log_warning "No configuration files found"
        return 1
    fi
    
    local -A all_assignments=()
    local conflicts_found=false
    
    # Parse all configuration files
    for config_file in "${config_files[@]}"; do
        while IFS=':' read -r pin_number pin_names; do
            if [[ -n "$pin_number" && -n "$pin_names" ]]; then
                if [[ -n "${all_assignments[$pin_number]}" ]]; then
                    all_assignments["$pin_number"]+=" $pin_names"
                else
                    all_assignments["$pin_number"]="$pin_names"
                fi
            fi
        done < <(parse_gpio_assignments "$config_file")
    done
    
    # Check for conflicts
    for pin in "${!all_assignments[@]}"; do
        local pin_names=(${all_assignments[$pin]})
        local unique_names=()
        
        # Remove duplicates and count unique assignments
        for name in "${pin_names[@]}"; do
            local found=false
            for unique in "${unique_names[@]}"; do
                if [[ "$name" == "$unique" ]]; then
                    found=true
                    break
                fi
            done
            if [[ "$found" == "false" ]]; then
                unique_names+=("$name")
            fi
        done
        
        if [[ ${#unique_names[@]} -gt 1 ]]; then
            # Check if it's a legitimate shared usage
            local legitimate_sharing=false
            
            # Camera data pins can share with other functions when camera is off
            if [[ "${unique_names[*]}" =~ Y[0-9]_GPIO_NUM ]] && [[ "${unique_names[*]}" =~ (BATTERY|SOLAR|BME280|LORA) ]]; then
                legitimate_sharing=true
                log_verbose "GPIO $pin: Legitimate sharing between camera and sensor (${unique_names[*]})"
            fi
            
            # I2C pins can be shared between devices
            if [[ "${unique_names[*]}" =~ (SDA|SCL) ]] && [[ ${#unique_names[@]} -eq 2 ]]; then
                legitimate_sharing=true
                log_verbose "GPIO $pin: I2C sharing (${unique_names[*]})"
            fi
            
            # UART pins can be shared for different purposes
            if [[ "${unique_names[*]}" =~ UART ]] && [[ "${unique_names[*]}" =~ PIR ]]; then
                legitimate_sharing=true
                log_verbose "GPIO $pin: UART/PIR sharing (${unique_names[*]})"
            fi
            
            if [[ "$legitimate_sharing" == "false" ]]; then
                log_error "GPIO $pin conflict: ${unique_names[*]}"
                conflicts_found=true
            fi
        else
            log_verbose "GPIO $pin: ${unique_names[0]}"
        fi
    done
    
    if [[ "$conflicts_found" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

# Validate camera pin assignments
validate_camera_pins() {
    log_info "Validating camera pin assignments..."
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    local camera_pins=(
        "XCLK_GPIO_NUM"
        "PCLK_GPIO_NUM"
        "HREF_GPIO_NUM"
        "VSYNC_GPIO_NUM"
        "SIOD_GPIO_NUM"
        "SIOC_GPIO_NUM"
        "Y2_GPIO_NUM"
        "Y3_GPIO_NUM"
        "Y4_GPIO_NUM"
        "Y5_GPIO_NUM"
        "Y6_GPIO_NUM"
        "Y7_GPIO_NUM"
        "Y8_GPIO_NUM"
        "Y9_GPIO_NUM"
        "PWDN_GPIO_NUM"
        "RESET_GPIO_NUM"
    )
    
    local found_pins=0
    local -A camera_assignments=()
    
    for config_file in "${config_files[@]}"; do
        if [[ ! -f "$config_file" ]]; then
            continue
        fi
        
        for pin_name in "${camera_pins[@]}"; do
            local pin_number=$(grep "^[[:space:]]*#define[[:space:]]\+${pin_name}[[:space:]]\+[0-9]\+" "$config_file" 2>/dev/null | awk '{print $3}' | head -1)
            if [[ -n "$pin_number" ]]; then
                camera_assignments["$pin_name"]="$pin_number"
                ((found_pins++))
                log_verbose "Camera pin: $pin_name = GPIO $pin_number"
            fi
        done
    done
    
    if [[ $found_pins -lt 10 ]]; then
        log_warning "Incomplete camera configuration: only $found_pins pins found"
        return 1
    fi
    
    # Validate camera pin constraints
    local camera_errors=false
    
    for pin_name in "${!camera_assignments[@]}"; do
        local pin_number="${camera_assignments[$pin_name]}"
        
        # Check input-only pins
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "input_only" ]]; then
            if [[ "$pin_name" =~ (XCLK|PCLK|HREF|VSYNC|SIOD|SIOC|PWDN|RESET) ]]; then
                log_error "Camera pin $pin_name (GPIO $pin_number) cannot use input-only pin"
                camera_errors=true
            fi
        fi
        
        # Check strapping pins
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "strapping" ]]; then
            log_warning "Camera pin $pin_name (GPIO $pin_number) uses strapping pin - may affect boot"
        fi
    done
    
    if [[ "$camera_errors" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

# Validate I2C/SPI configurations
validate_i2c_spi_pins() {
    log_info "Validating I2C/SPI pin configurations..."
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    local interface_pins=(
        "SDA_PIN"
        "SCL_PIN"
        "BME280_SDA_PIN"
        "BME280_SCL_PIN"
        "MOSI_PIN"
        "MISO_PIN"
        "SCK_PIN"
        "CS_PIN"
        "SD_MOSI_PIN"
        "SD_MISO_PIN"
        "SD_CLK_PIN"
        "SD_CS_PIN"
        "LORA_MOSI_PIN"
        "LORA_MISO_PIN"
        "LORA_SCK_PIN"
        "LORA_CS_PIN"
    )
    
    local -A interface_assignments=()
    local found_interfaces=0
    
    for config_file in "${config_files[@]}"; do
        if [[ ! -f "$config_file" ]]; then
            continue
        fi
        
        for pin_name in "${interface_pins[@]}"; do
            local pin_number=$(grep "^[[:space:]]*#define[[:space:]]\+${pin_name}[[:space:]]\+[0-9]\+" "$config_file" 2>/dev/null | awk '{print $3}' | head -1)
            if [[ -n "$pin_number" ]]; then
                interface_assignments["$pin_name"]="$pin_number"
                ((found_interfaces++))
                log_verbose "Interface pin: $pin_name = GPIO $pin_number"
            fi
        done
    done
    
    # Validate interface pin constraints
    local interface_errors=false
    
    for pin_name in "${!interface_assignments[@]}"; do
        local pin_number="${interface_assignments[$pin_name]}"
        
        # Check input-only pins for output signals
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "input_only" ]]; then
            if [[ "$pin_name" =~ (MOSI|SCK|CS|CLK|SDA|SCL) ]]; then
                log_error "Interface pin $pin_name (GPIO $pin_number) cannot use input-only pin"
                interface_errors=true
            fi
        fi
        
        # Check flash SPI pins
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "flash_spi" ]]; then
            log_error "Interface pin $pin_name (GPIO $pin_number) conflicts with flash SPI"
            interface_errors=true
        fi
        
        # Check ADC2 WiFi conflicts
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "adc2_wifi_conflict" ]]; then
            log_warning "Interface pin $pin_name (GPIO $pin_number) may conflict with WiFi (ADC2)"
        fi
    done
    
    log_verbose "Found $found_interfaces interface pin assignments"
    
    if [[ "$interface_errors" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

# Validate power management pins
validate_power_pins() {
    log_info "Validating power management pins..."
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    local power_pins=(
        "BATTERY_VOLTAGE_PIN"
        "SOLAR_VOLTAGE_PIN"
        "CHARGING_LED_PIN"
        "POWER_LED_PIN"
        "CHARGING_CONTROL_PIN"
        "PIR_POWER_PIN"
    )
    
    local -A power_assignments=()
    local found_power_pins=0
    
    for config_file in "${config_files[@]}"; do
        if [[ ! -f "$config_file" ]]; then
            continue
        fi
        
        for pin_name in "${power_pins[@]}"; do
            local pin_number=$(grep "^[[:space:]]*#define[[:space:]]\+${pin_name}[[:space:]]\+[0-9]\+" "$config_file" 2>/dev/null | awk '{print $3}' | head -1)
            if [[ -n "$pin_number" ]]; then
                power_assignments["$pin_name"]="$pin_number"
                ((found_power_pins++))
                log_verbose "Power pin: $pin_name = GPIO $pin_number"
            fi
        done
    done
    
    # Validate power pin constraints
    local power_errors=false
    
    for pin_name in "${!power_assignments[@]}"; do
        local pin_number="${power_assignments[$pin_name]}"
        
        # Voltage monitoring pins should be ADC capable
        if [[ "$pin_name" =~ VOLTAGE ]]; then
            local is_adc_pin=false
            
            # ADC1 pins (preferred for voltage monitoring)
            local adc1_pins=(32 33 34 35 36 37 38 39)
            for adc_pin in "${adc1_pins[@]}"; do
                if [[ "$pin_number" == "$adc_pin" ]]; then
                    is_adc_pin=true
                    break
                fi
            done
            
            if [[ "$is_adc_pin" == "false" ]]; then
                log_warning "Voltage pin $pin_name (GPIO $pin_number) is not on ADC1 - consider ADC1 pins for accuracy"
            fi
        fi
        
        # LED and control pins should not be input-only
        if [[ "$pin_name" =~ (LED|CONTROL) ]]; then
            if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "input_only" ]]; then
                log_error "Power pin $pin_name (GPIO $pin_number) cannot use input-only pin"
                power_errors=true
            fi
        fi
    done
    
    log_verbose "Found $found_power_pins power management pin assignments"
    
    if [[ "$power_errors" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

# Validate LoRa module pins
validate_lora_pins() {
    log_info "Validating LoRa module pin assignments..."
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    local lora_pins=(
        "LORA_CS_PIN"
        "LORA_RST_PIN"
        "LORA_DIO0_PIN"
        "LORA_DIO1_PIN"
        "LORA_MOSI_PIN"
        "LORA_MISO_PIN"
        "LORA_SCK_PIN"
    )
    
    local -A lora_assignments=()
    local found_lora_pins=0
    
    for config_file in "${config_files[@]}"; do
        if [[ ! -f "$config_file" ]]; then
            continue
        fi
        
        for pin_name in "${lora_pins[@]}"; do
            local pin_number=$(grep "^[[:space:]]*#define[[:space:]]\+${pin_name}[[:space:]]\+[0-9]\+" "$config_file" 2>/dev/null | awk '{print $3}' | head -1)
            if [[ -n "$pin_number" ]]; then
                lora_assignments["$pin_name"]="$pin_number"
                ((found_lora_pins++))
                log_verbose "LoRa pin: $pin_name = GPIO $pin_number"
            fi
        done
    done
    
    if [[ $found_lora_pins -eq 0 ]]; then
        log_verbose "LoRa module not configured"
        return 0
    fi
    
    # Validate LoRa pin constraints
    local lora_errors=false
    
    for pin_name in "${!lora_assignments[@]}"; do
        local pin_number="${lora_assignments[$pin_name]}"
        
        # Check input-only pins for output signals
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "input_only" ]]; then
            if [[ "$pin_name" =~ (CS|RST|MOSI|SCK) ]]; then
                log_error "LoRa pin $pin_name (GPIO $pin_number) cannot use input-only pin"
                lora_errors=true
            fi
        fi
        
        # Check flash SPI pins
        if [[ "${GPIO_CONSTRAINTS[$pin_number]}" == "flash_spi" ]]; then
            log_error "LoRa pin $pin_name (GPIO $pin_number) conflicts with flash SPI"
            lora_errors=true
        fi
    done
    
    log_verbose "Found $found_lora_pins LoRa pin assignments"
    
    if [[ "$lora_errors" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

# Check ESP32 hardware constraints
check_esp32_constraints() {
    log_info "Checking ESP32 hardware constraints for $BOARD_TYPE..."
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    local constraint_violations=false
    local -A all_pins=()
    
    # Collect all pin assignments
    for config_file in "${config_files[@]}"; do
        while IFS=':' read -r pin_number pin_names; do
            if [[ -n "$pin_number" && -n "$pin_names" ]]; then
                all_pins["$pin_number"]="$pin_names"
            fi
        done < <(parse_gpio_assignments "$config_file")
    done
    
    # Check board-specific constraints
    for pin in "${!all_pins[@]}"; do
        local pin_names="${all_pins[$pin]}"
        
        # Check basic ESP32 constraints
        if [[ -n "${GPIO_CONSTRAINTS[$pin]}" ]]; then
            local constraint="${GPIO_CONSTRAINTS[$pin]}"
            case "$constraint" in
                "input_only")
                    if [[ "$pin_names" =~ (LED|CONTROL|MOSI|SCK|CS|RST|SDA|SCL) ]]; then
                        log_error "GPIO $pin is input-only but assigned to output function: $pin_names"
                        constraint_violations=true
                    fi
                    ;;
                "flash_spi")
                    log_error "GPIO $pin is reserved for flash SPI but assigned to: $pin_names"
                    constraint_violations=true
                    ;;
                "strapping")
                    log_warning "GPIO $pin is a strapping pin assigned to: $pin_names"
                    ;;
                "adc2_wifi_conflict")
                    if [[ "$pin_names" =~ VOLTAGE ]]; then
                        log_warning "GPIO $pin (ADC2) may conflict with WiFi for voltage monitoring: $pin_names"
                    fi
                    ;;
            esac
        fi
        
        # Check ESP32-S3 specific constraints
        if [[ "$BOARD_TYPE" == "esp32s3" && -n "${ESP32S3_CONSTRAINTS[$pin]}" ]]; then
            local s3_constraint="${ESP32S3_CONSTRAINTS[$pin]}"
            case "$s3_constraint" in
                "usb_dm"|"usb_dp")
                    log_error "GPIO $pin is reserved for USB but assigned to: $pin_names"
                    constraint_violations=true
                    ;;
                "psram_octal")
                    log_warning "GPIO $pin may conflict with octal PSRAM: $pin_names"
                    ;;
            esac
        fi
    done
    
    if [[ "$constraint_violations" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

# Generate GPIO usage report
generate_gpio_report() {
    local report_file="$PROJECT_ROOT/gpio_validation_report.txt"
    
    log_info "Generating GPIO usage report: $report_file"
    
    local config_files
    readarray -t config_files < <(get_config_files)
    
    cat > "$report_file" << EOF
ESP32 Wildlife Camera - GPIO Configuration Report
================================================
Generated: $(date)
Board Type: $BOARD_TYPE

Configuration Files:
EOF
    
    for config_file in "${config_files[@]}"; do
        echo "  $(basename "$config_file")" >> "$report_file"
    done
    
    cat >> "$report_file" << EOF

GPIO Pin Assignments:
EOF
    
    local -A all_pins=()
    for config_file in "${config_files[@]}"; do
        while IFS=':' read -r pin_number pin_names; do
            if [[ -n "$pin_number" && -n "$pin_names" ]]; then
                all_pins["$pin_number"]="$pin_names"
            fi
        done < <(parse_gpio_assignments "$config_file")
    done
    
    for pin in $(printf '%s\n' "${!all_pins[@]}" | sort -n); do
        echo "  GPIO $pin: ${all_pins[$pin]}" >> "$report_file"
    done
    
    cat >> "$report_file" << EOF

Hardware Constraints:
EOF
    
    for pin in $(printf '%s\n' "${!all_pins[@]}" | sort -n); do
        if [[ -n "${GPIO_CONSTRAINTS[$pin]}" ]]; then
            echo "  GPIO $pin: ${GPIO_CONSTRAINTS[$pin]}" >> "$report_file"
        fi
        if [[ "$BOARD_TYPE" == "esp32s3" && -n "${ESP32S3_CONSTRAINTS[$pin]}" ]]; then
            echo "  GPIO $pin: ${ESP32S3_CONSTRAINTS[$pin]} (ESP32-S3)" >> "$report_file"
        fi
    done
    
    cat >> "$report_file" << EOF

Validation Results:
  Tests Passed: $TESTS_PASSED/$TESTS_TOTAL
  Status: $(if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then echo "PASSED"; else echo "FAILED"; fi)
EOF
    
    log_verbose "GPIO report generated: $report_file"
}

# Show summary
show_summary() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo ""
    echo "=== GPIO Configuration Validation Summary ==="
    echo "Board Type: $BOARD_TYPE"
    echo "Execution time: ${duration}s"
    echo "Tests passed: $TESTS_PASSED/$TESTS_TOTAL"
    
    if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then
        echo ""
        echo "✅ GPIO configuration validation passed"
        echo ""
        echo "All pin assignments are valid for $BOARD_TYPE"
        return 0
    else
        echo ""
        echo "❌ GPIO configuration validation failed"
        echo "Recommendations:"
        echo "  • Review pin assignments in configuration files"
        echo "  • Avoid input-only pins for output functions"
        echo "  • Consider ESP32 hardware constraints"
        echo "  • Use alternative pins for conflicting assignments"
        return 1
    fi
}

# Main execution
main() {
    echo "GPIO Configuration Validation"
    echo "============================"
    echo "Board Type: $BOARD_TYPE"
    echo "Fix issues: $(if [[ "$FIX_ISSUES" == "true" ]]; then echo "Yes"; else echo "No"; fi)"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Run GPIO validation tests
    check_gpio_conflicts
    test_result $? "GPIO pin conflict check"
    
    validate_camera_pins
    test_result $? "Camera pin validation"
    
    validate_i2c_spi_pins
    test_result $? "I2C/SPI pin validation"
    
    validate_power_pins
    test_result $? "Power management pin validation"
    
    validate_lora_pins
    test_result $? "LoRa module pin validation"
    
    check_esp32_constraints
    test_result $? "ESP32 hardware constraints"
    
    # Generate report
    generate_gpio_report
    
    # Show results
    show_summary
}

# Error handling
trap 'log_error "GPIO validation interrupted"; exit 1' INT TERM

# Execute main function
main "$@"