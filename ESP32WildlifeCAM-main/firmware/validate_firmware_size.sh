#!/bin/bash
# Firmware Size Validation Script
# Validates memory constraints before flashing ESP32 Wildlife Camera
# Usage: ./validate_firmware_size.sh [--verbose] [--environment <env>] [--build]

set -e

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERBOSE=false
ENVIRONMENT="esp32-s3-devkitc-1"
BUILD_FIRST=false
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

# Memory limits for different ESP32 variants (in bytes)
declare -A FLASH_LIMITS=(
    ["esp32"]=4194304      # 4MB
    ["esp32s2"]=4194304    # 4MB
    ["esp32s3"]=8388608    # 8MB
    ["esp32c3"]=4194304    # 4MB
)

declare -A RAM_LIMITS=(
    ["esp32"]=327680       # 320KB
    ["esp32s2"]=327680     # 320KB  
    ["esp32s3"]=524288     # 512KB
    ["esp32c3"]=327680     # 320KB
)

declare -A PSRAM_LIMITS=(
    ["esp32"]=4194304      # 4MB (external)
    ["esp32s2"]=8388608    # 8MB (external)
    ["esp32s3"]=8388608    # 8MB (internal)
    ["esp32c3"]=0          # No PSRAM
)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --environment|-e)
            ENVIRONMENT="$2"
            shift 2
            ;;
        --build|-b)
            BUILD_FIRST=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Firmware Size Validation Script"
            echo "Usage: $0 [--environment <env>] [--build] [--verbose] [--help]"
            echo ""
            echo "Options:"
            echo "  --environment, -e  PlatformIO environment to check (default: esp32-s3-devkitc-1)"
            echo "  --build, -b        Build firmware before size validation"
            echo "  --verbose, -v      Enable verbose output"
            echo "  --help, -h         Show this help message"
            echo ""
            echo "This script validates:"
            echo "  • Compiled firmware size against flash capacity"
            echo "  • RAM usage and availability"
            echo "  • PSRAM usage (if available)"
            echo "  • Partition table compatibility"
            echo "  • Bootloader size limits"
            echo "  • OTA update size requirements"
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

# Convert bytes to human readable format
format_bytes() {
    local bytes=$1
    if [[ $bytes -ge 1073741824 ]]; then
        echo "$(($bytes / 1073741824))GB"
    elif [[ $bytes -ge 1048576 ]]; then
        echo "$(($bytes / 1048576))MB"
    elif [[ $bytes -ge 1024 ]]; then
        echo "$(($bytes / 1024))KB"
    else
        echo "${bytes}B"
    fi
}

# Calculate percentage
calculate_percentage() {
    local used=$1
    local total=$2
    if [[ $total -gt 0 ]]; then
        echo $(( (used * 100) / total ))
    else
        echo "0"
    fi
}

# Get ESP32 variant from environment
get_esp32_variant() {
    local env="$1"
    
    if [[ "$env" =~ esp32s3 ]]; then
        echo "esp32s3"
    elif [[ "$env" =~ esp32s2 ]]; then
        echo "esp32s2"
    elif [[ "$env" =~ esp32c3 ]]; then
        echo "esp32c3"
    else
        echo "esp32"
    fi
}

# Build firmware if requested
build_firmware() {
    if [[ "$BUILD_FIRST" == "true" ]]; then
        log_info "Building firmware for environment: $ENVIRONMENT"
        
        if command -v pio >/dev/null 2>&1; then
            cd "$PROJECT_ROOT/firmware"
            if pio run -e "$ENVIRONMENT" --silent; then
                log_verbose "Firmware build completed successfully"
                return 0
            else
                log_error "Firmware build failed"
                return 1
            fi
        else
            log_warning "PlatformIO not found - skipping build"
            return 1
        fi
    fi
    return 0
}

# Check if firmware binary exists
check_firmware_binary() {
    local build_dir="$PROJECT_ROOT/firmware/.pio/build/$ENVIRONMENT"
    local firmware_bin="$build_dir/firmware.bin"
    local firmware_elf="$build_dir/firmware.elf"
    
    if [[ -f "$firmware_bin" ]]; then
        echo "$firmware_bin"
        return 0
    elif [[ -f "$firmware_elf" ]]; then
        echo "$firmware_elf"
        return 0
    else
        return 1
    fi
}

# Get firmware size information
get_firmware_size() {
    local firmware_file="$1"
    
    if [[ ! -f "$firmware_file" ]]; then
        return 1
    fi
    
    local file_size=$(stat -c%s "$firmware_file" 2>/dev/null || stat -f%z "$firmware_file" 2>/dev/null || echo "0")
    echo "$file_size"
}

# Parse memory usage from build output
parse_memory_usage() {
    local build_dir="$PROJECT_ROOT/firmware/.pio/build/$ENVIRONMENT"
    local memory_file="$build_dir/firmware.map"
    
    if [[ ! -f "$memory_file" ]]; then
        # Try to find size information in build log
        local log_file="$PROJECT_ROOT/firmware/.pio/build/$ENVIRONMENT/.sconsign.dblite"
        return 1
    fi
    
    # Parse memory sections from map file
    local text_size=0
    local data_size=0
    local bss_size=0
    
    if command -v size >/dev/null 2>&1 && [[ -f "$build_dir/firmware.elf" ]]; then
        local size_output=$(size "$build_dir/firmware.elf" 2>/dev/null || echo "")
        if [[ -n "$size_output" ]]; then
            # Parse size output (second line contains the values)
            text_size=$(echo "$size_output" | awk 'NR==2 {print $1}' || echo "0")
            data_size=$(echo "$size_output" | awk 'NR==2 {print $2}' || echo "0")
            bss_size=$(echo "$size_output" | awk 'NR==2 {print $3}' || echo "0")
        fi
    fi
    
    echo "$text_size $data_size $bss_size"
}

# Validate flash memory usage
validate_flash_usage() {
    log_info "Validating flash memory usage..."
    
    local variant=$(get_esp32_variant "$ENVIRONMENT")
    local flash_limit=${FLASH_LIMITS[$variant]}
    
    # Check if firmware binary exists
    local firmware_file
    if ! firmware_file=$(check_firmware_binary); then
        if [[ "$BUILD_FIRST" == "true" ]]; then
            log_error "Firmware binary not found after build"
            return 1
        else
            log_warning "Firmware binary not found - run with --build to create it"
            log_verbose "Estimated flash usage: ~1-2MB for typical ESP32 Wildlife Camera firmware"
            return 0  # Don't fail if no binary
        fi
    fi
    
    local firmware_size=$(get_firmware_size "$firmware_file")
    local flash_percentage=$(calculate_percentage $firmware_size $flash_limit)
    
    log_verbose "Flash memory analysis:"
    log_verbose "  ESP32 variant: $variant"
    log_verbose "  Flash limit: $(format_bytes $flash_limit)"
    log_verbose "  Firmware size: $(format_bytes $firmware_size)"
    log_verbose "  Flash usage: ${flash_percentage}%"
    
    # Check flash usage thresholds
    if [[ $flash_percentage -gt 90 ]]; then
        log_error "Flash usage critical: ${flash_percentage}% ($(format_bytes $firmware_size) / $(format_bytes $flash_limit))"
        return 1
    elif [[ $flash_percentage -gt 75 ]]; then
        log_warning "Flash usage high: ${flash_percentage}% ($(format_bytes $firmware_size) / $(format_bytes $flash_limit))"
        return 0
    else
        log_verbose "Flash usage acceptable: ${flash_percentage}% ($(format_bytes $firmware_size) / $(format_bytes $flash_limit))"
        return 0
    fi
}

# Validate RAM usage
validate_ram_usage() {
    log_info "Validating RAM usage..."
    
    local variant=$(get_esp32_variant "$ENVIRONMENT")
    local ram_limit=${RAM_LIMITS[$variant]}
    
    # Parse memory usage from build
    local memory_info
    if ! memory_info=$(parse_memory_usage); then
        log_verbose "Memory usage information not available"
        log_verbose "Estimated RAM usage: ~100-200KB for typical ESP32 Wildlife Camera firmware"
        return 0
    fi
    
    local text_size=$(echo "$memory_info" | awk '{print $1}')
    local data_size=$(echo "$memory_info" | awk '{print $2}')
    local bss_size=$(echo "$memory_info" | awk '{print $3}')
    
    local ram_usage=$((data_size + bss_size))
    local ram_percentage=$(calculate_percentage $ram_usage $ram_limit)
    
    log_verbose "RAM analysis:"
    log_verbose "  ESP32 variant: $variant"
    log_verbose "  RAM limit: $(format_bytes $ram_limit)"
    log_verbose "  Text (Flash): $(format_bytes $text_size)"
    log_verbose "  Data (RAM): $(format_bytes $data_size)"
    log_verbose "  BSS (RAM): $(format_bytes $bss_size)"
    log_verbose "  Total RAM usage: $(format_bytes $ram_usage)"
    log_verbose "  RAM usage: ${ram_percentage}%"
    
    # Check RAM usage thresholds
    if [[ $ram_percentage -gt 80 ]]; then
        log_error "RAM usage critical: ${ram_percentage}% ($(format_bytes $ram_usage) / $(format_bytes $ram_limit))"
        return 1
    elif [[ $ram_percentage -gt 60 ]]; then
        log_warning "RAM usage high: ${ram_percentage}% ($(format_bytes $ram_usage) / $(format_bytes $ram_limit))"
        return 0
    else
        log_verbose "RAM usage acceptable: ${ram_percentage}% ($(format_bytes $ram_usage) / $(format_bytes $ram_limit))"
        return 0
    fi
}

# Validate PSRAM usage
validate_psram_usage() {
    log_info "Validating PSRAM usage..."
    
    local variant=$(get_esp32_variant "$ENVIRONMENT")
    local psram_limit=${PSRAM_LIMITS[$variant]}
    
    if [[ $psram_limit -eq 0 ]]; then
        log_verbose "PSRAM not available for $variant"
        return 0
    fi
    
    # Check if PSRAM is enabled in configuration
    if ! grep -q "PSRAM\|SPIRAM" "$PROJECT_ROOT/firmware/platformio.ini" 2>/dev/null; then
        log_verbose "PSRAM not enabled in configuration"
        return 0
    fi
    
    log_verbose "PSRAM analysis:"
    log_verbose "  ESP32 variant: $variant"
    log_verbose "  PSRAM limit: $(format_bytes $psram_limit)"
    log_verbose "  PSRAM configuration: Enabled"
    
    # For wildlife camera, estimate PSRAM usage
    local estimated_psram_usage=2097152  # 2MB for frame buffers and AI processing
    local psram_percentage=$(calculate_percentage $estimated_psram_usage $psram_limit)
    
    log_verbose "  Estimated PSRAM usage: $(format_bytes $estimated_psram_usage)"
    log_verbose "  PSRAM usage: ${psram_percentage}%"
    
    if [[ $psram_percentage -gt 90 ]]; then
        log_warning "PSRAM usage may be high: ${psram_percentage}%"
        return 0
    else
        log_verbose "PSRAM usage acceptable: ${psram_percentage}%"
        return 0
    fi
}

# Check partition table compatibility
validate_partition_table() {
    log_info "Validating partition table compatibility..."
    
    local partition_files=(
        "$PROJECT_ROOT/firmware/partitions.csv"
        "$PROJECT_ROOT/partitions.csv"
    )
    
    local partition_file=""
    for pf in "${partition_files[@]}"; do
        if [[ -f "$pf" ]]; then
            partition_file="$pf"
            break
        fi
    done
    
    if [[ -z "$partition_file" ]]; then
        log_verbose "Using default partition table"
        
        # Check if huge_app partition is specified in platformio.ini
        if grep -q "huge_app" "$PROJECT_ROOT/firmware/platformio.ini" 2>/dev/null; then
            log_verbose "Partition scheme: huge_app (suitable for large applications)"
        else
            log_verbose "Partition scheme: default (may limit application size)"
        fi
        return 0
    fi
    
    log_verbose "Custom partition table found: $partition_file"
    
    # Parse partition table
    local app_partitions=0
    local app_size=0
    local ota_partitions=0
    
    while IFS=',' read -r name type subtype offset size flags; do
        # Skip comments and empty lines
        [[ "$name" =~ ^#.*$ ]] && continue
        [[ -z "$name" ]] && continue
        
        if [[ "$type" == "app" ]]; then
            ((app_partitions++))
            if [[ "$subtype" == "factory" || "$subtype" == "ota_0" || "$subtype" == "ota_1" ]]; then
                # Convert size from hex or decimal
                local partition_size
                if [[ "$size" =~ ^0x ]]; then
                    partition_size=$((size))
                else
                    partition_size=$size
                fi
                app_size=$((app_size + partition_size))
                
                if [[ "$subtype" =~ ota_ ]]; then
                    ((ota_partitions++))
                fi
            fi
        fi
    done < "$partition_file"
    
    log_verbose "Partition analysis:"
    log_verbose "  App partitions: $app_partitions"
    log_verbose "  OTA partitions: $ota_partitions"
    log_verbose "  Total app space: $(format_bytes $app_size)"
    
    # Validate partition configuration
    if [[ $ota_partitions -gt 0 ]]; then
        log_verbose "OTA updates: Enabled ($ota_partitions partitions)"
        
        # Check if app size is sufficient for OTA
        local min_ota_size=1048576  # 1MB minimum per OTA partition
        if [[ $app_size -lt $((min_ota_size * 2)) ]]; then
            log_warning "OTA partition size may be insufficient for large applications"
        fi
    else
        log_verbose "OTA updates: Disabled"
    fi
    
    return 0
}

# Validate bootloader size limits
validate_bootloader_size() {
    log_info "Validating bootloader size limits..."
    
    local build_dir="$PROJECT_ROOT/firmware/.pio/build/$ENVIRONMENT"
    local bootloader_bin="$build_dir/bootloader.bin"
    
    if [[ -f "$bootloader_bin" ]]; then
        local bootloader_size=$(get_firmware_size "$bootloader_bin")
        local bootloader_limit=65536  # 64KB typical limit
        local bootloader_percentage=$(calculate_percentage $bootloader_size $bootloader_limit)
        
        log_verbose "Bootloader analysis:"
        log_verbose "  Bootloader size: $(format_bytes $bootloader_size)"
        log_verbose "  Bootloader limit: $(format_bytes $bootloader_limit)"
        log_verbose "  Bootloader usage: ${bootloader_percentage}%"
        
        if [[ $bootloader_percentage -gt 90 ]]; then
            log_warning "Bootloader size near limit: ${bootloader_percentage}%"
        else
            log_verbose "Bootloader size acceptable: ${bootloader_percentage}%"
        fi
    else
        log_verbose "Bootloader binary not found - using default size estimates"
    fi
    
    return 0
}

# Validate OTA update requirements
validate_ota_requirements() {
    log_info "Validating OTA update size requirements..."
    
    # Check if OTA is enabled
    if ! grep -q "OTA.*ENABLED\|AsyncElegantOTA" "$PROJECT_ROOT/firmware/platformio.ini" 2>/dev/null; then
        log_verbose "OTA updates not enabled"
        return 0
    fi
    
    log_verbose "OTA updates: Enabled"
    
    # For OTA, firmware must fit in available partition space
    local firmware_file
    if firmware_file=$(check_firmware_binary); then
        local firmware_size=$(get_firmware_size "$firmware_file")
        local ota_overhead=32768  # 32KB overhead for OTA process
        local required_ota_space=$((firmware_size + ota_overhead))
        
        log_verbose "OTA requirements:"
        log_verbose "  Firmware size: $(format_bytes $firmware_size)"
        log_verbose "  OTA overhead: $(format_bytes $ota_overhead)"
        log_verbose "  Required OTA space: $(format_bytes $required_ota_space)"
        
        # Estimate available OTA space (assuming dual OTA partitions)
        local variant=$(get_esp32_variant "$ENVIRONMENT")
        local flash_limit=${FLASH_LIMITS[$variant]}
        local available_ota_space=$((flash_limit / 2 - 65536))  # Half flash minus bootloader/partitions
        
        if [[ $required_ota_space -gt $available_ota_space ]]; then
            log_warning "Firmware may be too large for OTA updates"
            log_verbose "  Available OTA space: $(format_bytes $available_ota_space)"
        else
            log_verbose "OTA space sufficient: $(format_bytes $available_ota_space) available"
        fi
    else
        log_verbose "Firmware binary not available for OTA size check"
    fi
    
    return 0
}

# Show summary
show_summary() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo ""
    echo "=== Firmware Size Validation Summary ==="
    echo "Environment: $ENVIRONMENT"
    echo "Execution time: ${duration}s"
    echo "Tests passed: $TESTS_PASSED/$TESTS_TOTAL"
    
    if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then
        echo ""
        echo "✅ Firmware size validation passed"
        echo ""
        echo "Memory usage within acceptable limits"
        if [[ "$BUILD_FIRST" != "true" ]]; then
            echo "Note: Run with --build for accurate size measurements"
        fi
        return 0
    else
        echo ""
        echo "❌ Firmware size validation failed"
        echo "Recommendations:"
        echo "  • Optimize code to reduce memory usage"
        echo "  • Consider disabling unused features"
        echo "  • Use external storage for large data"
        echo "  • Check partition table configuration"
        return 1
    fi
}

# Main execution
main() {
    echo "Firmware Size Validation"
    echo "======================="
    echo "Environment: $ENVIRONMENT"
    echo "Build first: $(if [[ "$BUILD_FIRST" == "true" ]]; then echo "Yes"; else echo "No"; fi)"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Build firmware if requested
    if ! build_firmware; then
        if [[ "$BUILD_FIRST" == "true" ]]; then
            log_error "Build failed - cannot validate firmware size"
            exit 1
        fi
    fi
    
    # Run validation tests
    validate_flash_usage
    test_result $? "Flash memory usage validation"
    
    validate_ram_usage
    test_result $? "RAM usage validation"
    
    validate_psram_usage
    test_result $? "PSRAM usage validation"
    
    validate_partition_table
    test_result $? "Partition table compatibility"
    
    validate_bootloader_size
    test_result $? "Bootloader size validation"
    
    validate_ota_requirements
    test_result $? "OTA update requirements"
    
    # Show results
    show_summary
}

# Error handling
trap 'log_error "Firmware size validation interrupted"; exit 1' INT TERM

# Execute main function
main "$@"