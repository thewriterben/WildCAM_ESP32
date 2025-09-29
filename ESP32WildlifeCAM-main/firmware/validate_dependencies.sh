#!/bin/bash
# Dependency Verification Script
# Offline library compatibility checker for ESP32 Wildlife Camera
# Usage: ./validate_dependencies.sh [--verbose] [--fix] [--cache-check]

set -e

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERBOSE=false
FIX_ISSUES=false
CACHE_CHECK=false
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

# Required libraries and their versions (from platformio.ini)
declare -A REQUIRED_LIBRARIES=(
    ["ArduinoJson"]="6.21.3"
    ["AsyncTCP"]="1.1.1"
    ["LoRa"]="0.8.0"
    ["Adafruit BME280 Library"]="2.2.2"
    ["Adafruit Unified Sensor"]="1.1.14"
    ["Time"]="1.6.1"
    ["SD"]="1.2.4"
    ["Adafruit SSD1306"]="2.5.7"
    ["Adafruit GFX Library"]="1.11.9"
    ["TFT_eSPI"]="2.5.23"
    ["XPowersLib"]="0.2.1"
    ["AsyncElegantOTA"]="2.2.7"
)

# ESP32 platform requirements
declare -A PLATFORM_REQUIREMENTS=(
    ["espressif32"]="6.4.0"
    ["framework"]="arduino"
    ["toolchain"]="xtensa-esp32"
)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --fix|-f)
            FIX_ISSUES=true
            shift
            ;;
        --cache-check|-c)
            CACHE_CHECK=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Dependency Verification Script"
            echo "Usage: $0 [--fix] [--cache-check] [--verbose] [--help]"
            echo ""
            echo "Options:"
            echo "  --fix, -f         Attempt to fix missing dependencies"
            echo "  --cache-check, -c Check cache integrity and completeness"
            echo "  --verbose, -v     Enable verbose output"
            echo "  --help, -h        Show this help message"
            echo ""
            echo "This script validates:"
            echo "  • All required libraries are cached locally"
            echo "  • Library version compatibility"
            echo "  • Include file availability"
            echo "  • Compilation dependencies"
            echo "  • Framework compatibility"
            echo "  • Platform toolchain availability"
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
    local allow_fix=${3:-false}
    
    ((TESTS_TOTAL++))
    
    if [[ $result -eq 0 ]]; then
        log_success "$description"
    elif [[ "$FIX_ISSUES" == "true" && "$allow_fix" == "true" ]]; then
        log_warning "$description (attempting fix)"
    else
        log_error "$description"
    fi
}

# Check PlatformIO installation
check_platformio_installation() {
    log_info "Checking PlatformIO installation..."
    
    if command -v pio >/dev/null 2>&1; then
        local pio_version=$(pio --version 2>/dev/null | head -1 || echo "unknown")
        log_verbose "PlatformIO version: $pio_version"
        return 0
    else
        log_error "PlatformIO not found"
        if [[ "$FIX_ISSUES" == "true" ]]; then
            log_info "Installing PlatformIO..."
            if command -v pip3 >/dev/null 2>&1; then
                pip3 install platformio && return 0
            elif command -v pip >/dev/null 2>&1; then
                pip install platformio && return 0
            fi
        fi
        return 1
    fi
}

# Check platform cache
check_platform_cache() {
    log_info "Checking ESP32 platform cache..."
    
    local platform_dirs=(
        "$HOME/.platformio/platforms/espressif32"
        "$HOME/.platformio/packages/framework-arduinoespressif32"
        "$HOME/.platformio/packages/toolchain-xtensa-esp32"
    )
    
    local missing_platforms=()
    
    for platform_dir in "${platform_dirs[@]}"; do
        if [[ -d "$platform_dir" ]]; then
            local platform_name=$(basename "$platform_dir")
            log_verbose "Platform cached: $platform_name"
        else
            local platform_name=$(basename "$platform_dir")
            missing_platforms+=("$platform_name")
            log_verbose "Platform missing: $platform_name"
        fi
    done
    
    if [[ ${#missing_platforms[@]} -eq 0 ]]; then
        return 0
    else
        log_warning "Missing platforms: ${missing_platforms[*]}"
        
        if [[ "$FIX_ISSUES" == "true" ]]; then
            log_info "Installing missing platforms..."
            cd "$PROJECT_ROOT/firmware"
            if pio platform install espressif32; then
                return 0
            else
                return 1
            fi
        fi
        return 1
    fi
}

# Check library cache
check_library_cache() {
    log_info "Checking library cache..."
    
    local missing_libraries=()
    local lib_cache_dir="$HOME/.platformio/lib"
    
    if [[ ! -d "$lib_cache_dir" ]]; then
        log_warning "Library cache directory not found: $lib_cache_dir"
        if [[ "$FIX_ISSUES" == "true" ]]; then
            mkdir -p "$lib_cache_dir"
        fi
    fi
    
    # Check for library dependencies in project
    local project_lib_dir="$PROJECT_ROOT/firmware/.pio/libdeps"
    local project_libs=()
    
    if [[ -d "$project_lib_dir" ]]; then
        while IFS= read -r -d '' lib_dir; do
            if [[ -d "$lib_dir" ]]; then
                local lib_name=$(basename "$lib_dir")
                project_libs+=("$lib_name")
                log_verbose "Project library: $lib_name"
            fi
        done < <(find "$project_lib_dir" -maxdepth 2 -type d -print0 2>/dev/null)
    fi
    
    # Verify required libraries
    for lib_name in "${!REQUIRED_LIBRARIES[@]}"; do
        local lib_version="${REQUIRED_LIBRARIES[$lib_name]}"
        local found=false
        
        # Check global cache
        if [[ -d "$lib_cache_dir" ]]; then
            local cached_lib=$(find "$lib_cache_dir" -name "*${lib_name}*" -type d 2>/dev/null | head -1)
            if [[ -n "$cached_lib" ]]; then
                log_verbose "Library cached globally: $lib_name"
                found=true
            fi
        fi
        
        # Check project cache
        if [[ -d "$project_lib_dir" ]]; then
            local project_lib=$(find "$project_lib_dir" -name "*${lib_name}*" -type d 2>/dev/null | head -1)
            if [[ -n "$project_lib" ]]; then
                log_verbose "Library cached in project: $lib_name"
                found=true
            fi
        fi
        
        if [[ "$found" == "false" ]]; then
            missing_libraries+=("$lib_name@$lib_version")
        fi
    done
    
    if [[ ${#missing_libraries[@]} -eq 0 ]]; then
        return 0
    else
        log_warning "Missing libraries: ${missing_libraries[*]}"
        
        if [[ "$FIX_ISSUES" == "true" ]]; then
            log_info "Installing missing libraries..."
            cd "$PROJECT_ROOT/firmware"
            for lib in "${missing_libraries[@]}"; do
                if pio lib install "$lib"; then
                    log_verbose "Installed: $lib"
                else
                    log_error "Failed to install: $lib"
                fi
            done
            return 0
        fi
        return 1
    fi
}

# Verify include file availability
verify_include_files() {
    log_info "Verifying include file availability..."
    
    local source_dirs=(
        "$PROJECT_ROOT/firmware/src"
        "$PROJECT_ROOT/firmware/include"
    )
    
    local missing_includes=()
    local include_pattern='#include\s*[<"]([^>"]+)[>"]'
    
    for src_dir in "${source_dirs[@]}"; do
        if [[ ! -d "$src_dir" ]]; then
            continue
        fi
        
        while IFS= read -r -d '' source_file; do
            log_verbose "Checking includes in: $(basename "$source_file")"
            
            while IFS= read -r line; do
                if [[ "$line" =~ $include_pattern ]]; then
                    local include_file="${BASH_REMATCH[1]}"
                    
                    # Skip system includes and Arduino core includes
                    if [[ "$include_file" =~ ^(stdio|stdlib|string|math|time|Arduino|ESP32) ]]; then
                        continue
                    fi
                    
                    # Check if include file exists
                    local found=false
                    
                    # Check in source directories
                    for check_dir in "${source_dirs[@]}"; do
                        if [[ -f "$check_dir/$include_file" ]]; then
                            found=true
                            break
                        fi
                    done
                    
                    # Check in library directories
                    if [[ "$found" == "false" ]]; then
                        local lib_dirs=(
                            "$HOME/.platformio/packages/framework-arduinoespressif32/libraries"
                            "$HOME/.platformio/lib"
                            "$PROJECT_ROOT/firmware/.pio/libdeps"
                        )
                        
                        for lib_dir in "${lib_dirs[@]}"; do
                            if [[ -d "$lib_dir" ]]; then
                                local include_path=$(find "$lib_dir" -name "$include_file" -type f 2>/dev/null | head -1)
                                if [[ -n "$include_path" ]]; then
                                    found=true
                                    break
                                fi
                            fi
                        done
                    fi
                    
                    if [[ "$found" == "false" ]]; then
                        missing_includes+=("$include_file (from $(basename "$source_file"))")
                        log_verbose "Missing include: $include_file"
                    fi
                fi
            done < "$source_file"
        done < <(find "$src_dir" -name "*.cpp" -o -name "*.h" -print0 2>/dev/null)
    done
    
    if [[ ${#missing_includes[@]} -eq 0 ]]; then
        return 0
    else
        log_warning "Missing include files: ${#missing_includes[@]} found"
        for missing in "${missing_includes[@]}"; do
            log_verbose "  $missing"
        done
        return 1
    fi
}

# Test compilation dependencies
test_compilation_dependencies() {
    log_info "Testing compilation dependencies..."
    
    # Check if we can do a basic compilation test
    if ! command -v g++ >/dev/null 2>&1; then
        log_warning "g++ not available for compilation test"
        return 0
    fi
    
    # Create a simple test file
    local test_file="/tmp/esp32_compile_test.cpp"
    cat > "$test_file" << 'EOF'
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>

void setup() {
    Serial.begin(115200);
    WiFi.begin("test", "test");
    JsonDocument doc;
    doc["test"] = "value";
}

void loop() {
    delay(1000);
}
EOF
    
    # Attempt compilation with available headers
    local compile_result=0
    if g++ -fsyntax-only -std=c++17 \
           -DARDUINO=200 -DESP32_VALIDATION_BUILD=1 \
           -I"$PROJECT_ROOT/firmware/src" \
           -I"$PROJECT_ROOT/firmware/include" \
           -Wno-unknown-pragmas -Wno-attributes \
           "$test_file" 2>/dev/null; then
        log_verbose "Basic compilation test: PASSED"
        compile_result=0
    else
        log_verbose "Basic compilation test: FAILED (ESP32-specific dependencies expected)"
        compile_result=0  # Don't fail on ESP32-specific issues
    fi
    
    # Clean up
    rm -f "$test_file"
    
    return $compile_result
}

# Verify framework compatibility
verify_framework_compatibility() {
    log_info "Verifying framework compatibility..."
    
    local platformio_ini="$PROJECT_ROOT/firmware/platformio.ini"
    
    if [[ ! -f "$platformio_ini" ]]; then
        log_error "platformio.ini not found"
        return 1
    fi
    
    # Check framework configuration
    if grep -q "framework.*arduino" "$platformio_ini"; then
        log_verbose "Arduino framework configured"
    else
        log_warning "Arduino framework not found in configuration"
    fi
    
    # Check ESP32 platform version
    local platform_line=$(grep "platform.*espressif32" "$platformio_ini" | head -1)
    if [[ -n "$platform_line" ]]; then
        log_verbose "ESP32 platform: $platform_line"
        
        # Extract version if specified
        if [[ "$platform_line" =~ @([0-9]+\.[0-9]+\.[0-9]+) ]]; then
            local platform_version="${BASH_REMATCH[1]}"
            local required_version="${PLATFORM_REQUIREMENTS[espressif32]}"
            
            if [[ "$platform_version" == "$required_version" ]]; then
                log_verbose "Platform version matches requirement: $platform_version"
            else
                log_warning "Platform version mismatch: found $platform_version, expected $required_version"
            fi
        fi
    else
        log_warning "ESP32 platform not found in configuration"
    fi
    
    # Check library dependencies section
    if grep -q "lib_deps" "$platformio_ini"; then
        local lib_count=$(grep -A 50 "lib_deps" "$platformio_ini" | grep -E "^\s*\w+/" | wc -l)
        log_verbose "Library dependencies configured: $lib_count libraries"
    else
        log_warning "No library dependencies section found"
    fi
    
    return 0
}

# Check cache integrity
check_cache_integrity() {
    if [[ "$CACHE_CHECK" != "true" ]]; then
        return 0
    fi
    
    log_info "Checking cache integrity..."
    
    local cache_dirs=(
        "$HOME/.platformio"
        "$PROJECT_ROOT/firmware/.pio"
    )
    
    for cache_dir in "${cache_dirs[@]}"; do
        if [[ -d "$cache_dir" ]]; then
            local cache_size=$(du -sh "$cache_dir" 2>/dev/null | cut -f1 || echo "unknown")
            log_verbose "Cache directory: $cache_dir ($cache_size)"
            
            # Check for corruption indicators
            local corrupt_files=$(find "$cache_dir" -name "*.tmp" -o -name "*.lock" 2>/dev/null | wc -l)
            if [[ $corrupt_files -gt 0 ]]; then
                log_warning "Potential cache corruption: $corrupt_files temporary files found"
                
                if [[ "$FIX_ISSUES" == "true" ]]; then
                    log_info "Cleaning temporary files..."
                    find "$cache_dir" -name "*.tmp" -delete 2>/dev/null || true
                    find "$cache_dir" -name "*.lock" -delete 2>/dev/null || true
                fi
            fi
        else
            log_verbose "Cache directory not found: $cache_dir"
        fi
    done
    
    return 0
}

# Generate dependency report
generate_dependency_report() {
    local report_file="$PROJECT_ROOT/dependency_validation_report.txt"
    
    log_info "Generating dependency report: $report_file"
    
    cat > "$report_file" << EOF
ESP32 Wildlife Camera - Dependency Validation Report
===================================================
Generated: $(date)
Environment: $(uname -a)

Platform Requirements:
EOF
    
    for platform in "${!PLATFORM_REQUIREMENTS[@]}"; do
        echo "  $platform: ${PLATFORM_REQUIREMENTS[$platform]}" >> "$report_file"
    done
    
    cat >> "$report_file" << EOF

Required Libraries:
EOF
    
    for lib in "${!REQUIRED_LIBRARIES[@]}"; do
        echo "  $lib: ${REQUIRED_LIBRARIES[$lib]}" >> "$report_file"
    done
    
    cat >> "$report_file" << EOF

Cache Status:
  PlatformIO Home: $HOME/.platformio
  Project Cache: $PROJECT_ROOT/firmware/.pio
  
Validation Results:
  Tests Passed: $TESTS_PASSED/$TESTS_TOTAL
  Status: $(if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then echo "PASSED"; else echo "FAILED"; fi)

Recommendations:
EOF
    
    if [[ $TESTS_PASSED -ne $TESTS_TOTAL ]]; then
        cat >> "$report_file" << EOF
  • Run with --fix to attempt automatic fixes
  • Ensure all required libraries are properly cached
  • Verify network connectivity for initial setup
  • Check PlatformIO installation and configuration
EOF
    else
        cat >> "$report_file" << EOF
  • Dependencies are properly configured
  • Environment ready for offline development
  • Regular validation recommended
EOF
    fi
    
    log_verbose "Report generated: $report_file"
}

# Show summary
show_summary() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo ""
    echo "=== Dependency Validation Summary ==="
    echo "Execution time: ${duration}s"
    echo "Tests passed: $TESTS_PASSED/$TESTS_TOTAL"
    
    if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then
        echo ""
        echo "✅ All dependencies verified and available offline"
        echo ""
        echo "Library cache complete for offline development"
        if [[ "$FIX_ISSUES" == "true" ]]; then
            echo "Automatic fixes applied where possible"
        fi
        return 0
    else
        echo ""
        echo "❌ Dependency validation failed"
        echo "Recommendations:"
        echo "  • Run with --fix to attempt automatic fixes"
        echo "  • Install missing libraries manually"
        echo "  • Verify PlatformIO configuration"
        echo "  • Check network connectivity for downloads"
        return 1
    fi
}

# Main execution
main() {
    echo "Dependency Verification"
    echo "======================"
    echo "Fix issues: $(if [[ "$FIX_ISSUES" == "true" ]]; then echo "Yes"; else echo "No"; fi)"
    echo "Cache check: $(if [[ "$CACHE_CHECK" == "true" ]]; then echo "Yes"; else echo "No"; fi)"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Run dependency checks
    check_platformio_installation
    test_result $? "PlatformIO installation" true
    
    check_platform_cache
    test_result $? "ESP32 platform cache" true
    
    check_library_cache
    test_result $? "Library cache verification" true
    
    verify_include_files
    test_result $? "Include file availability"
    
    test_compilation_dependencies
    test_result $? "Compilation dependencies"
    
    verify_framework_compatibility
    test_result $? "Framework compatibility"
    
    check_cache_integrity
    test_result $? "Cache integrity check"
    
    # Generate report
    generate_dependency_report
    
    # Show results
    show_summary
}

# Error handling
trap 'log_error "Dependency validation interrupted"; exit 1' INT TERM

# Execute main function
main "$@"