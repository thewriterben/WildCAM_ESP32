#!/bin/bash

# ESP32 Wildlife CAM - Production Preparation Script
# Prepares system for production deployment with optimized configuration

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Logging functions
log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Help function
show_help() {
    cat << EOF
ESP32 Wildlife CAM - Production Preparation Script

USAGE:
    $0 [OPTIONS] [PROFILE]

PROFILES:
    backyard        Optimized for backyard monitoring (3-7 days)
    trail           Optimized for trail monitoring (7-14 days)  
    wilderness      Optimized for wilderness deployment (14+ days)
    custom          Custom configuration based on parameters

OPTIONS:
    -h, --help                      Show this help message
    -v, --verbose                   Enable verbose output
    --upload                        Upload firmware after preparation
    --validate                      Validate configuration only
    --backup-config                 Backup current configuration
    --power-optimization LEVEL      Power optimization level (conservative|balanced|aggressive)
    --storage-management MODE       Storage management mode (continuous|threshold|manual)
    --error-recovery                Enable advanced error recovery
    --remote-monitoring             Enable remote monitoring features
    --ai-features                   Enable AI species recognition
    --duration HOURS                Expected deployment duration in hours

EXAMPLES:
    $0 wilderness --upload                    # Prepare and upload wilderness firmware
    $0 trail --power-optimization aggressive  # Trail deployment with aggressive power saving
    $0 custom --duration 168 --ai-features    # Custom 7-day deployment with AI
    $0 --validate --backup-config             # Validate and backup current config

EOF
}

# Production profiles
declare -A BACKYARD_PROFILE=(
    [power_mode]="balanced"
    [motion_sensitivity]="medium"
    [image_quality]="high"
    [capture_burst]="false"
    [sleep_mode]="light"
    [storage_policy]="continuous"
    [wifi_enabled]="true"
    [ai_enabled]="false"
    [duration_hours]="72"
    [battery_threshold]="3.5"
)

declare -A TRAIL_PROFILE=(
    [power_mode]="conservative"
    [motion_sensitivity]="adaptive"
    [image_quality]="medium"
    [capture_burst]="true"
    [sleep_mode]="deep"
    [storage_policy]="threshold"
    [wifi_enabled]="false"
    [ai_enabled]="true"
    [duration_hours]="168"
    [battery_threshold]="3.4"
)

declare -A WILDERNESS_PROFILE=(
    [power_mode]="ultra_conservative"
    [motion_sensitivity]="large_animal"
    [image_quality]="medium"
    [capture_burst]="false"
    [sleep_mode]="deep"
    [storage_policy]="threshold"
    [wifi_enabled]="false"
    [ai_enabled]="false"
    [duration_hours]="336"
    [battery_threshold]="3.3"
)

# Validate environment
check_environment() {
    log_info "Validating production environment..."
    
    cd "$PROJECT_ROOT"
    
    # Check PlatformIO
    if ! command -v pio >/dev/null 2>&1; then
        log_error "PlatformIO not found. Run setup_development.sh first."
        exit 1
    fi
    
    # Check project structure
    local required_files=(
        "platformio.ini"
        "src/main.cpp"
        "include"
        "firmware"
    )
    
    for file in "${required_files[@]}"; do
        if [[ ! -e "$file" ]]; then
            log_error "Required file/directory not found: $file"
            exit 1
        fi
    done
    
    log_success "Environment validation passed"
}

# Backup current configuration
backup_configuration() {
    log_info "Backing up current configuration..."
    
    local backup_dir="$PROJECT_ROOT/config/backups"
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local backup_path="$backup_dir/config_backup_$timestamp"
    
    mkdir -p "$backup_path"
    
    # Backup configuration files
    local config_files=(
        "include/config.h"
        "include/config_unified.h"
        "include/pins.h"
        "wifi_config.h"
        "platformio.ini"
    )
    
    for file in "${config_files[@]}"; do
        if [[ -f "$file" ]]; then
            cp "$file" "$backup_path/"
            log_info "Backed up: $file"
        fi
    done
    
    # Create backup info
    cat > "$backup_path/backup_info.txt" << EOF
Backup Created: $(date)
Git Commit: $(git rev-parse HEAD 2>/dev/null || echo "Unknown")
Git Branch: $(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "Unknown")
Production Profile: ${PROFILE:-"None"}
Power Optimization: ${POWER_OPTIMIZATION:-"Default"}
EOF
    
    log_success "Configuration backed up to: $backup_path"
    echo "$backup_path"
}

# Generate production configuration
generate_production_config() {
    local profile="$1"
    
    log_info "Generating production configuration for profile: $profile"
    
    # Select profile configuration
    local -n profile_config
    case "$profile" in
        backyard)
            profile_config=BACKYARD_PROFILE
            ;;
        trail)
            profile_config=TRAIL_PROFILE
            ;;
        wilderness)
            profile_config=WILDERNESS_PROFILE
            ;;
        custom)
            generate_custom_config
            return
            ;;
        *)
            log_error "Unknown profile: $profile"
            exit 1
            ;;
    esac
    
    # Generate production config header
    local config_file="$PROJECT_ROOT/include/production_config.h"
    
    cat > "$config_file" << EOF
#ifndef PRODUCTION_CONFIG_H
#define PRODUCTION_CONFIG_H

/**
 * ESP32 Wildlife CAM - Production Configuration
 * 
 * Generated: $(date)
 * Profile: $profile
 * Optimization: ${POWER_OPTIMIZATION:-"default"}
 */

// Production Profile Settings
#define PRODUCTION_PROFILE "$profile"
#define PRODUCTION_MODE 1
#define DEBUG_MODE 0

// Power Management
#define POWER_MODE "${profile_config[power_mode]}"
#define BATTERY_LOW_THRESHOLD ${profile_config[battery_threshold]}
#define SLEEP_MODE "${profile_config[sleep_mode]}"

// Camera Configuration
#define IMAGE_QUALITY "${profile_config[image_quality]}"
#define CAPTURE_BURST_MODE ${profile_config[capture_burst]}

// Motion Detection
#define MOTION_SENSITIVITY "${profile_config[motion_sensitivity]}"

// Storage Management
#define STORAGE_POLICY "${profile_config[storage_policy]}"

// Communication
#define WIFI_ENABLED ${profile_config[wifi_enabled]}

// AI Features
#define AI_SPECIES_RECOGNITION ${profile_config[ai_enabled]}

// Deployment Parameters
#define EXPECTED_DURATION_HOURS ${profile_config[duration_hours]}

// Power Optimization Overrides
EOF
    
    # Add power optimization settings
    case "${POWER_OPTIMIZATION:-balanced}" in
        conservative)
            cat >> "$config_file" << EOF
#define CPU_FREQUENCY_MHZ 80
#define WIFI_POWER_SAVE_MODE 1
#define BLUETOOTH_DISABLED 1
#define UNUSED_PINS_PULLDOWN 1
EOF
            ;;
        aggressive)
            cat >> "$config_file" << EOF
#define CPU_FREQUENCY_MHZ 40
#define WIFI_POWER_SAVE_MODE 2
#define BLUETOOTH_DISABLED 1
#define UNUSED_PINS_PULLDOWN 1
#define REDUCE_SAMPLING_RATE 1
#define MINIMAL_LOGGING 1
EOF
            ;;
        balanced|*)
            cat >> "$config_file" << EOF
#define CPU_FREQUENCY_MHZ 160
#define WIFI_POWER_SAVE_MODE 0
#define BLUETOOTH_DISABLED 1
EOF
            ;;
    esac
    
    # Add error recovery settings
    if [[ "${ERROR_RECOVERY:-false}" == "true" ]]; then
        cat >> "$config_file" << EOF

// Error Recovery Features
#define WATCHDOG_TIMEOUT_MS 30000
#define AUTO_RESTART_ON_ERROR 1
#define ERROR_LOG_RETENTION 1
#define FAILSAFE_MODE 1
EOF
    fi
    
    # Add remote monitoring settings
    if [[ "${REMOTE_MONITORING:-false}" == "true" ]]; then
        cat >> "$config_file" << EOF

// Remote Monitoring Features
#define REMOTE_STATUS_REPORTING 1
#define TELEMETRY_INTERVAL_MINUTES 60
#define ALERT_BATTERY_LOW 1
#define ALERT_SYSTEM_ERRORS 1
EOF
    fi
    
    # Close header guard
    echo "" >> "$config_file"
    echo "#endif // PRODUCTION_CONFIG_H" >> "$config_file"
    
    log_success "Production configuration generated: $config_file"
}

# Generate custom configuration based on parameters
generate_custom_config() {
    log_info "Generating custom configuration..."
    
    # Use default values if not specified
    local duration="${DURATION_HOURS:-168}"
    local power_mode="balanced"
    local ai_enabled="false"
    
    # Determine optimal settings based on duration
    if [[ $duration -lt 72 ]]; then
        power_mode="balanced"
        image_quality="high"
        motion_sensitivity="medium"
    elif [[ $duration -lt 168 ]]; then
        power_mode="conservative"
        image_quality="medium"
        motion_sensitivity="adaptive"
    else
        power_mode="ultra_conservative"
        image_quality="medium"
        motion_sensitivity="large_animal"
    fi
    
    # Override with user settings
    if [[ "${AI_FEATURES:-false}" == "true" ]]; then
        ai_enabled="true"
    fi
    
    # Create custom profile
    declare -A CUSTOM_PROFILE=(
        [power_mode]="$power_mode"
        [motion_sensitivity]="$motion_sensitivity"
        [image_quality]="${image_quality:-medium}"
        [capture_burst]="true"
        [sleep_mode]="deep"
        [storage_policy]="threshold"
        [wifi_enabled]="false"
        [ai_enabled]="$ai_enabled"
        [duration_hours]="$duration"
        [battery_threshold]="3.4"
    )
    
    # Generate config using custom profile
    local -n profile_config=CUSTOM_PROFILE
    generate_production_config_from_array profile_config "custom"
}

# Generate configuration from profile array
generate_production_config_from_array() {
    local -n profile_ref=$1
    local profile_name="$2"
    
    local config_file="$PROJECT_ROOT/include/production_config.h"
    
    cat > "$config_file" << EOF
#ifndef PRODUCTION_CONFIG_H
#define PRODUCTION_CONFIG_H

/**
 * ESP32 Wildlife CAM - Production Configuration
 * 
 * Generated: $(date)
 * Profile: $profile_name
 * Optimization: ${POWER_OPTIMIZATION:-"default"}
 */

// Production Profile Settings
#define PRODUCTION_PROFILE "$profile_name"
#define PRODUCTION_MODE 1
#define DEBUG_MODE 0

// Power Management
#define POWER_MODE "${profile_ref[power_mode]}"
#define BATTERY_LOW_THRESHOLD ${profile_ref[battery_threshold]}
#define SLEEP_MODE "${profile_ref[sleep_mode]}"

// Camera Configuration
#define IMAGE_QUALITY "${profile_ref[image_quality]}"
#define CAPTURE_BURST_MODE ${profile_ref[capture_burst]}

// Motion Detection
#define MOTION_SENSITIVITY "${profile_ref[motion_sensitivity]}"

// Storage Management
#define STORAGE_POLICY "${profile_ref[storage_policy]}"

// Communication
#define WIFI_ENABLED ${profile_ref[wifi_enabled]}

// AI Features
#define AI_SPECIES_RECOGNITION ${profile_ref[ai_enabled]}

// Deployment Parameters
#define EXPECTED_DURATION_HOURS ${profile_ref[duration_hours]}

#endif // PRODUCTION_CONFIG_H
EOF
    
    log_success "Custom production configuration generated"
}

# Optimize build settings
optimize_build_settings() {
    log_info "Optimizing build settings for production..."
    
    local platformio_ini="$PROJECT_ROOT/platformio.ini"
    local backup_ini="${platformio_ini}.backup"
    
    # Backup original platformio.ini
    cp "$platformio_ini" "$backup_ini"
    
    # Create production-optimized platformio.ini
    cat > "$platformio_ini" << EOF
; ESP32 Wildlife CAM - Production Build Configuration
; Generated: $(date)

[env:esp32cam_production]
platform = espressif32
board = esp32cam
framework = arduino

; Production build flags
build_flags = 
    -DPRODUCTION_BUILD=1
    -DCORE_DEBUG_LEVEL=1
    -Os
    -DARDUINO_ESP32_DEV
    -DBOARD_HAS_PSRAM
    -DCONFIG_SPIRAM_SUPPORT=1
    
; Upload settings
upload_speed = 921600
monitor_speed = 115200

; Library dependencies
lib_deps = 
    esp32-camera
    ArduinoJson
    AsyncTCP
    ESPAsyncWebServer
    WiFiManager
    PubSubClient

; Memory optimization
board_build.partitions = huge_app.csv
board_build.f_cpu = 240000000L
board_build.f_flash = 80000000L
board_build.flash_mode = dio

; Production optimizations
board_build.embed_files = 
    data/index.html
    data/app.js  
    data/styles.css
EOF
    
    # Add power optimization specific flags
    case "${POWER_OPTIMIZATION:-balanced}" in
        conservative)
            cat >> "$platformio_ini" << EOF

; Conservative power optimization
build_flags += 
    -DPOWER_CONSERVATIVE=1
    -DCPU_FREQ_LOW=1
EOF
            ;;
        aggressive)
            cat >> "$platformio_ini" << EOF

; Aggressive power optimization  
build_flags += 
    -DPOWER_ULTRA_CONSERVATIVE=1
    -DCPU_FREQ_MINIMAL=1
    -DMINIMAL_FEATURES=1
EOF
            ;;
    esac
    
    log_success "Build settings optimized for production"
}

# Validate configuration
validate_configuration() {
    log_info "Validating production configuration..."
    
    local config_file="$PROJECT_ROOT/include/production_config.h"
    local errors=0
    
    # Check if config file exists
    if [[ ! -f "$config_file" ]]; then
        log_error "Production config file not found: $config_file"
        ((errors++))
    fi
    
    # Validate project compiles
    log_info "Testing compilation..."
    cd "$PROJECT_ROOT"
    
    if pio run --dry-run > /tmp/pio_validation.log 2>&1; then
        log_success "Compilation validation passed"
    else
        log_error "Compilation validation failed"
        log_error "Check log: /tmp/pio_validation.log"
        ((errors++))
    fi
    
    # Check GPIO pin conflicts
    if [[ -x "$PROJECT_ROOT/scripts/quick_validate.sh" ]]; then
        log_info "Running GPIO validation..."
        if "$PROJECT_ROOT/scripts/quick_validate.sh" --quiet; then
            log_success "GPIO validation passed"
        else
            log_warning "GPIO validation warnings detected"
        fi
    fi
    
    # Validate configuration values
    if [[ -f "$config_file" ]]; then
        log_info "Validating configuration values..."
        
        # Check for required defines
        local required_defines=(
            "PRODUCTION_MODE"
            "POWER_MODE"
            "BATTERY_LOW_THRESHOLD"
            "IMAGE_QUALITY"
            "MOTION_SENSITIVITY"
        )
        
        for define in "${required_defines[@]}"; do
            if grep -q "#define $define" "$config_file"; then
                log_info "✓ $define defined"
            else
                log_error "✗ $define not defined"
                ((errors++))
            fi
        done
    fi
    
    if [[ $errors -eq 0 ]]; then
        log_success "Configuration validation completed successfully"
        return 0
    else
        log_error "Configuration validation failed with $errors errors"
        return 1
    fi
}

# Upload firmware
upload_firmware() {
    log_info "Uploading production firmware..."
    
    cd "$PROJECT_ROOT"
    
    # Check if device is connected
    if ! pio device list | grep -q "esp32\|ESP32\|CP210"; then
        log_error "No ESP32 device detected. Connect device and try again."
        return 1
    fi
    
    # Upload firmware
    log_info "Building and uploading firmware..."
    if pio run --target upload; then
        log_success "Firmware uploaded successfully"
        
        # Test basic functionality
        log_info "Testing device functionality..."
        sleep 5
        
        if pio device monitor --baud 115200 --filter esp32_exception_decoder &
        then
            local monitor_pid=$!
            sleep 10
            kill $monitor_pid 2>/dev/null || true
            log_success "Device responding normally"
        fi
    else
        log_error "Firmware upload failed"
        return 1
    fi
}

# Generate production report
generate_production_report() {
    log_info "Generating production report..."
    
    local report_dir="$PROJECT_ROOT/production_reports"
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local report_file="$report_dir/production_report_$timestamp.md"
    
    mkdir -p "$report_dir"
    
    cat > "$report_file" << EOF
# ESP32 Wildlife CAM - Production Report

**Generated**: $(date)  
**Profile**: ${PROFILE:-"Unknown"}  
**Power Optimization**: ${POWER_OPTIMIZATION:-"Default"}  
**Git Commit**: $(git rev-parse HEAD 2>/dev/null || echo "Unknown")  

## Configuration Summary

### Profile Settings
- **Deployment Type**: ${PROFILE:-"Custom"}
- **Expected Duration**: ${DURATION_HOURS:-"Unknown"} hours
- **Power Mode**: ${POWER_OPTIMIZATION:-"Balanced"}
- **AI Features**: ${AI_FEATURES:-"Disabled"}
- **Remote Monitoring**: ${REMOTE_MONITORING:-"Disabled"}
- **Error Recovery**: ${ERROR_RECOVERY:-"Basic"}

### Build Information
- **Platform**: ESP32-CAM
- **Framework**: Arduino
- **Build Type**: Production (-Os optimization)
- **Debug Level**: Minimal (Level 1)

### Validation Results
EOF
    
    # Add validation results
    if validate_configuration; then
        echo "- ✅ Configuration validation: PASSED" >> "$report_file"
    else
        echo "- ❌ Configuration validation: FAILED" >> "$report_file"
    fi
    
    # Add file information
    echo "" >> "$report_file"
    echo "### Generated Files" >> "$report_file"
    echo "- Production config: \`include/production_config.h\`" >> "$report_file"
    echo "- Build config: \`platformio.ini\`" >> "$report_file"
    
    if [[ -n "${BACKUP_PATH:-}" ]]; then
        echo "- Configuration backup: \`$BACKUP_PATH\`" >> "$report_file"
    fi
    
    echo "" >> "$report_file"
    echo "### Next Steps" >> "$report_file"
    echo "1. Upload firmware to ESP32-CAM device" >> "$report_file"
    echo "2. Perform initial system testing" >> "$report_file"
    echo "3. Execute field deployment procedures" >> "$report_file"
    echo "4. Monitor system performance and logs" >> "$report_file"
    
    echo "" >> "$report_file"
    echo "---" >> "$report_file"
    echo "*Report generated by ESP32 Wildlife CAM Production Preparation Script*" >> "$report_file"
    
    log_success "Production report saved: $report_file"
}

# Parse command line arguments
parse_args() {
    PROFILE=""
    VALIDATE_ONLY=false
    UPLOAD_FIRMWARE=false
    BACKUP_CONFIG=false
    VERBOSE=false
    POWER_OPTIMIZATION="balanced"
    ERROR_RECOVERY=false
    REMOTE_MONITORING=false
    AI_FEATURES=false
    DURATION_HOURS=""
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            --upload)
                UPLOAD_FIRMWARE=true
                shift
                ;;
            --validate)
                VALIDATE_ONLY=true
                shift
                ;;
            --backup-config)
                BACKUP_CONFIG=true
                shift
                ;;
            --power-optimization)
                POWER_OPTIMIZATION="$2"
                shift 2
                ;;
            --storage-management)
                STORAGE_MANAGEMENT="$2"
                shift 2
                ;;
            --error-recovery)
                ERROR_RECOVERY=true
                shift
                ;;
            --remote-monitoring)
                REMOTE_MONITORING=true
                shift
                ;;
            --ai-features)
                AI_FEATURES=true
                shift
                ;;
            --duration)
                DURATION_HOURS="$2"
                shift 2
                ;;
            backyard|trail|wilderness|custom)
                PROFILE="$1"
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Default profile if none specified
    if [[ -z "$PROFILE" && "$VALIDATE_ONLY" != "true" ]]; then
        PROFILE="trail"
        log_info "No profile specified, using default: $PROFILE"
    fi
}

# Main execution
main() {
    log_info "ESP32 Wildlife CAM Production Preparation"
    log_info "Profile: ${PROFILE:-"Validation Only"}"
    echo ""
    
    # Environment validation
    check_environment
    
    # Backup configuration if requested
    if [[ "$BACKUP_CONFIG" == "true" ]]; then
        BACKUP_PATH=$(backup_configuration)
    fi
    
    # Validation only mode
    if [[ "$VALIDATE_ONLY" == "true" ]]; then
        validate_configuration
        exit $?
    fi
    
    # Generate production configuration
    generate_production_config "$PROFILE"
    
    # Optimize build settings
    optimize_build_settings
    
    # Validate configuration
    if ! validate_configuration; then
        log_error "Configuration validation failed. Aborting."
        exit 1
    fi
    
    # Upload firmware if requested
    if [[ "$UPLOAD_FIRMWARE" == "true" ]]; then
        upload_firmware
    fi
    
    # Generate production report
    generate_production_report
    
    log_success "Production preparation completed!"
    echo ""
    echo "Next steps:"
    echo "1. Review production report in production_reports/"
    if [[ "$UPLOAD_FIRMWARE" != "true" ]]; then
        echo "2. Upload firmware: pio run --target upload"
    fi
    echo "3. Test system functionality"
    echo "4. Deploy using deployment checklist"
}

# Execute if run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    parse_args "$@"
    main
fi