#!/bin/bash
# Enhanced Offline Validation Script
# Comprehensive offline validation for ESP32 Wildlife Camera
# Usage: ./enhanced_offline_validate.sh [--verbose] [--fix] [--full] [--report]

set -e

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERBOSE=false
FIX_ISSUES=false
FULL_VALIDATION=false
GENERATE_REPORT=false
START_TIME=$(date +%s)

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Test counters
TESTS_PASSED=0
TESTS_TOTAL=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --fix|-f)
            FIX_ISSUES=true
            shift
            ;;
        --full)
            FULL_VALIDATION=true
            shift
            ;;
        --report|-r)
            GENERATE_REPORT=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Enhanced Offline Validation Script"
            echo "Usage: $0 [--fix] [--full] [--report] [--verbose] [--help]"
            echo ""
            echo "Options:"
            echo "  --fix, -f       Attempt to fix issues automatically"
            echo "  --full          Run comprehensive validation (includes all components)"
            echo "  --report, -r    Generate detailed validation report"
            echo "  --verbose, -v   Enable verbose output"
            echo "  --help, -h      Show this help message"
            echo ""
            echo "This script runs:"
            echo "  • All validation scripts in sequence"
            echo "  • Comprehensive test reporting"
            echo "  • Error logging and diagnosis"
            echo "  • Performance metrics"
            echo "  • Automated fix suggestions"
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

log_section() {
    echo -e "${PURPLE}[SECTION]${NC} $1"
}

test_result() {
    local result=$1
    local description="$2"
    local component="$3"
    
    ((TESTS_TOTAL++))
    
    if [[ $result -eq 0 ]]; then
        log_success "$description"
        echo "✅ $component: $description" >> "$PROJECT_ROOT/validation_results.log"
    else
        log_error "$description"
        echo "❌ $component: $description" >> "$PROJECT_ROOT/validation_results.log"
    fi
}

# Initialize validation log
initialize_validation_log() {
    local log_file="$PROJECT_ROOT/validation_results.log"
    cat > "$log_file" << EOF
ESP32 Wildlife Camera - Enhanced Validation Results
===================================================
Started: $(date)
Mode: $(if [[ "$FULL_VALIDATION" == "true" ]]; then echo "Full Validation"; else echo "Standard Validation"; fi)
Fix Issues: $(if [[ "$FIX_ISSUES" == "true" ]]; then echo "Enabled"; else echo "Disabled"; fi)

Validation Results:
EOF
    log_verbose "Validation log initialized: $log_file"
}

# Run original offline validation
run_original_validation() {
    log_section "Running Original Offline Validation"
    
    if [[ -f "$PROJECT_ROOT/firmware/offline_validate.sh" ]]; then
        cd "$PROJECT_ROOT/firmware"
        if ./offline_validate.sh >/dev/null 2>&1; then
            test_result 0 "Original offline validation" "Original"
        else
            test_result 1 "Original offline validation failed" "Original"
        fi
        cd - >/dev/null
    else
        test_result 1 "Original validation script not found" "Original"
    fi
}

# Run hardware validation
run_hardware_validation() {
    log_section "Running Hardware Validation"
    
    if [[ -f "$PROJECT_ROOT/firmware/validate_esp32_hardware.sh" ]]; then
        local hw_args="--simulation"
        if [[ "$VERBOSE" == "true" ]]; then
            hw_args="$hw_args --verbose"
        fi
        
        cd "$PROJECT_ROOT/firmware"
        if ./validate_esp32_hardware.sh $hw_args >/dev/null 2>&1; then
            test_result 0 "ESP32 hardware validation" "Hardware"
        else
            test_result 1 "ESP32 hardware validation failed" "Hardware"
        fi
        cd - >/dev/null
    else
        test_result 1 "Hardware validation script not found" "Hardware"
    fi
}

# Run firmware size validation
run_firmware_size_validation() {
    log_section "Running Firmware Size Validation"
    
    if [[ -f "$PROJECT_ROOT/firmware/validate_firmware_size.sh" ]]; then
        local size_args=""
        if [[ "$VERBOSE" == "true" ]]; then
            size_args="$size_args --verbose"
        fi
        
        cd "$PROJECT_ROOT/firmware"
        if ./validate_firmware_size.sh $size_args >/dev/null 2>&1; then
            test_result 0 "Firmware size validation" "Firmware"
        else
            test_result 1 "Firmware size validation failed" "Firmware"
        fi
        cd - >/dev/null
    else
        test_result 1 "Firmware size validation script not found" "Firmware"
    fi
}

# Run dependency validation
run_dependency_validation() {
    log_section "Running Dependency Validation"
    
    if [[ -f "$PROJECT_ROOT/firmware/validate_dependencies.sh" ]]; then
        local dep_args=""
        if [[ "$VERBOSE" == "true" ]]; then
            dep_args="$dep_args --verbose"
        fi
        if [[ "$FIX_ISSUES" == "true" ]]; then
            dep_args="$dep_args --fix"
        fi
        
        cd "$PROJECT_ROOT/firmware"
        if ./validate_dependencies.sh $dep_args >/dev/null 2>&1; then
            test_result 0 "Dependency validation" "Dependencies"
        else
            test_result 1 "Dependency validation failed" "Dependencies"
        fi
        cd - >/dev/null
    else
        test_result 1 "Dependency validation script not found" "Dependencies"
    fi
}

# Run GPIO configuration validation
run_gpio_validation() {
    log_section "Running GPIO Configuration Validation"
    
    if [[ -f "$PROJECT_ROOT/firmware/validate_gpio_config.sh" ]]; then
        local gpio_args="--board esp32s3"
        if [[ "$VERBOSE" == "true" ]]; then
            gpio_args="$gpio_args --verbose"
        fi
        if [[ "$FIX_ISSUES" == "true" ]]; then
            gpio_args="$gpio_args --fix"
        fi
        
        cd "$PROJECT_ROOT/firmware"
        if ./validate_gpio_config.sh $gpio_args >/dev/null 2>&1; then
            test_result 0 "GPIO configuration validation" "GPIO"
        else
            test_result 1 "GPIO configuration validation failed" "GPIO"
        fi
        cd - >/dev/null
    else
        test_result 1 "GPIO validation script not found" "GPIO"
    fi
}

# Run existing validation scripts
run_existing_validations() {
    log_section "Running Existing Validation Scripts"
    
    # Run validate_fixes.py
    if [[ -f "$PROJECT_ROOT/validate_fixes.py" ]]; then
        cd "$PROJECT_ROOT"
        if python3 validate_fixes.py >/dev/null 2>&1; then
            test_result 0 "Code validation (validate_fixes.py)" "Code"
        else
            test_result 1 "Code validation failed" "Code"
        fi
        cd - >/dev/null
    fi
    
    # Run quick_validate.sh
    if [[ -f "$PROJECT_ROOT/scripts/quick_validate.sh" ]]; then
        cd "$PROJECT_ROOT"
        if ./scripts/quick_validate.sh >/dev/null 2>&1; then
            test_result 0 "Quick validation" "Quick"
        else
            test_result 1 "Quick validation failed" "Quick"
        fi
        cd - >/dev/null
    fi
    
    # Run validate_critical_components.sh
    if [[ -f "$PROJECT_ROOT/scripts/validate_critical_components.sh" ]]; then
        cd "$PROJECT_ROOT"
        if ./scripts/validate_critical_components.sh >/dev/null 2>&1; then
            test_result 0 "Critical components validation" "Critical"
        else
            test_result 1 "Critical components validation failed" "Critical"
        fi
        cd - >/dev/null
    fi
}

# Run comprehensive hardware tests
run_comprehensive_hardware_tests() {
    if [[ "$FULL_VALIDATION" != "true" ]]; then
        return 0
    fi
    
    log_section "Running Comprehensive Hardware Tests"
    
    # Test all hardware components
    local components=("gpio" "i2c" "spi" "camera" "sd" "psram" "flash" "power")
    
    for component in "${components[@]}"; do
        if [[ -f "$PROJECT_ROOT/firmware/validate_esp32_hardware.sh" ]]; then
            local hw_args="--simulation --component $component"
            if [[ "$VERBOSE" == "true" ]]; then
                hw_args="$hw_args --verbose"
            fi
            
            cd "$PROJECT_ROOT/firmware"
            if ./validate_esp32_hardware.sh $hw_args >/dev/null 2>&1; then
                test_result 0 "$component hardware test" "Hardware-$component"
            else
                test_result 1 "$component hardware test failed" "Hardware-$component"
            fi
            cd - >/dev/null
        fi
    done
}

# Check build system
check_build_system() {
    log_section "Checking Build System"
    
    # Check if PlatformIO is available
    if command -v pio >/dev/null 2>&1; then
        test_result 0 "PlatformIO installation" "Build"
        
        # Check if we can list environments
        cd "$PROJECT_ROOT/firmware"
        if pio project config --json-output >/dev/null 2>&1; then
            test_result 0 "PlatformIO configuration" "Build"
        else
            test_result 1 "PlatformIO configuration issues" "Build"
        fi
        cd - >/dev/null
    else
        test_result 1 "PlatformIO not found" "Build"
    fi
    
    # Check for essential build files
    local build_files=(
        "$PROJECT_ROOT/firmware/platformio.ini"
        "$PROJECT_ROOT/firmware/src"
    )
    
    for build_file in "${build_files[@]}"; do
        if [[ -e "$build_file" ]]; then
            test_result 0 "Build file $(basename "$build_file") exists" "Build"
        else
            test_result 1 "Build file $(basename "$build_file") missing" "Build"
        fi
    done
}

# Measure performance metrics
measure_performance() {
    log_section "Measuring Performance Metrics"
    
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    # Log performance metrics
    echo "" >> "$PROJECT_ROOT/validation_results.log"
    echo "Performance Metrics:" >> "$PROJECT_ROOT/validation_results.log"
    echo "  Total execution time: ${duration}s" >> "$PROJECT_ROOT/validation_results.log"
    echo "  Tests completed: $TESTS_TOTAL" >> "$PROJECT_ROOT/validation_results.log"
    echo "  Tests passed: $TESTS_PASSED" >> "$PROJECT_ROOT/validation_results.log"
    echo "  Success rate: $(( TESTS_PASSED * 100 / TESTS_TOTAL ))%" >> "$PROJECT_ROOT/validation_results.log"
    
    log_verbose "Performance metrics recorded"
    
    # Performance targets
    if [[ $duration -lt 60 ]]; then
        test_result 0 "Validation completed within 1 minute" "Performance"
    elif [[ $duration -lt 120 ]]; then
        test_result 0 "Validation completed within 2 minutes" "Performance"
    else
        test_result 1 "Validation took longer than expected ($duration seconds)" "Performance"
    fi
}

# Generate comprehensive report
generate_comprehensive_report() {
    if [[ "$GENERATE_REPORT" != "true" ]]; then
        return 0
    fi
    
    log_section "Generating Comprehensive Report"
    
    local report_file="$PROJECT_ROOT/enhanced_validation_report.md"
    
    cat > "$report_file" << EOF
# ESP32 Wildlife Camera - Enhanced Validation Report

**Generated:** $(date)  
**Mode:** $(if [[ "$FULL_VALIDATION" == "true" ]]; then echo "Full Validation"; else echo "Standard Validation"; fi)  
**Fix Issues:** $(if [[ "$FIX_ISSUES" == "true" ]]; then echo "Enabled"; else echo "Disabled"; fi)

## Summary

- **Tests Total:** $TESTS_TOTAL
- **Tests Passed:** $TESTS_PASSED
- **Success Rate:** $(( TESTS_PASSED * 100 / TESTS_TOTAL ))%
- **Execution Time:** $(($(date +%s) - START_TIME))s

## Validation Results

EOF
    
    # Add detailed results from log
    if [[ -f "$PROJECT_ROOT/validation_results.log" ]]; then
        echo '```' >> "$report_file"
        grep -E '^(✅|❌)' "$PROJECT_ROOT/validation_results.log" >> "$report_file"
        echo '```' >> "$report_file"
    fi
    
    cat >> "$report_file" << EOF

## System Information

- **Operating System:** $(uname -a)
- **Python Version:** $(python3 --version 2>/dev/null || echo "Not available")
- **PlatformIO Version:** $(pio --version 2>/dev/null | head -1 || echo "Not available")
- **Git Status:** $(cd "$PROJECT_ROOT" && git status --porcelain | wc -l) modified files

## Recommendations

EOF
    
    if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then
        cat >> "$report_file" << EOF
✅ **All validations passed!**

The ESP32 Wildlife Camera project is ready for offline development:
- All dependencies are properly cached
- Hardware configurations are valid
- Build system is functional
- Code quality checks pass

### Next Steps:
1. Begin development in offline mode
2. Run periodic validations to ensure stability
3. Test on actual hardware when available
EOF
    else
        cat >> "$report_file" << EOF
⚠️ **Some validations failed**

Issues detected that need attention:
- Review failed tests in the validation log
- Consider running with \`--fix\` option to attempt automatic repairs
- Manually address configuration conflicts
- Ensure all required dependencies are installed

### Priority Actions:
1. Fix GPIO configuration conflicts
2. Resolve dependency issues
3. Address build system problems
4. Re-run validation after fixes
EOF
    fi
    
    log_success "Comprehensive report generated: $report_file"
}

# Automated fix suggestions
provide_fix_suggestions() {
    log_section "Providing Fix Suggestions"
    
    local suggestions_file="$PROJECT_ROOT/validation_fix_suggestions.txt"
    
    cat > "$suggestions_file" << EOF
ESP32 Wildlife Camera - Validation Fix Suggestions
=================================================
Generated: $(date)

Based on the validation results, here are suggested fixes:

EOF
    
    # Analyze common failures and suggest fixes
    if grep -q "❌.*GPIO" "$PROJECT_ROOT/validation_results.log" 2>/dev/null; then
        cat >> "$suggestions_file" << EOF
GPIO Configuration Issues:
- Review GPIO pin assignments in firmware/src/config.h
- Resolve conflicts between camera, LoRa, and power management pins
- Consider alternative pin assignments for conflicting functions
- Use GPIO sharing only for compatible functions

Commands to fix:
  ./firmware/validate_gpio_config.sh --verbose --fix

EOF
    fi
    
    if grep -q "❌.*Dependency" "$PROJECT_ROOT/validation_results.log" 2>/dev/null; then
        cat >> "$suggestions_file" << EOF
Dependency Issues:
- Install missing PlatformIO libraries
- Verify library versions match requirements
- Cache all dependencies for offline use

Commands to fix:
  ./firmware/validate_dependencies.sh --fix
  cd firmware && pio lib install

EOF
    fi
    
    if grep -q "❌.*Hardware" "$PROJECT_ROOT/validation_results.log" 2>/dev/null; then
        cat >> "$suggestions_file" << EOF
Hardware Configuration Issues:
- Verify pin assignments match hardware capabilities
- Check ESP32 variant constraints
- Ensure proper pull-up/pull-down resistors

Commands to test:
  ./firmware/validate_esp32_hardware.sh --simulation --verbose

EOF
    fi
    
    log_success "Fix suggestions generated: $suggestions_file"
}

# Show final summary
show_final_summary() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo ""
    echo "==============================================="
    echo " Enhanced Offline Validation Summary"
    echo "==============================================="
    echo "Mode: $(if [[ "$FULL_VALIDATION" == "true" ]]; then echo "Full Validation"; else echo "Standard Validation"; fi)"
    echo "Execution time: ${duration}s"
    echo "Tests completed: $TESTS_TOTAL"
    echo "Tests passed: $TESTS_PASSED"
    echo "Success rate: $(( TESTS_PASSED * 100 / TESTS_TOTAL ))%"
    
    if [[ $TESTS_PASSED -eq $TESTS_TOTAL ]]; then
        echo ""
        echo "🎉 ALL VALIDATIONS PASSED!"
        echo ""
        echo "The ESP32 Wildlife Camera project is ready for offline development."
        echo "All systems validated and dependencies cached."
        return 0
    else
        echo ""
        echo "⚠️  SOME VALIDATIONS FAILED"
        echo ""
        echo "$(( TESTS_TOTAL - TESTS_PASSED )) issues detected that need attention."
        echo ""
        echo "Recommendations:"
        echo "  • Review validation_results.log for details"
        echo "  • Check validation_fix_suggestions.txt for solutions"
        echo "  • Run with --fix to attempt automatic repairs"
        echo "  • Address configuration conflicts manually"
        return 1
    fi
}

# Main execution
main() {
    echo "Enhanced Offline Validation for ESP32 Wildlife Camera"
    echo "===================================================="
    echo "Mode: $(if [[ "$FULL_VALIDATION" == "true" ]]; then echo "Full Validation"; else echo "Standard Validation"; fi)"
    echo "Fix issues: $(if [[ "$FIX_ISSUES" == "true" ]]; then echo "Enabled"; else echo "Disabled"; fi)"
    echo "Generate report: $(if [[ "$GENERATE_REPORT" == "true" ]]; then echo "Enabled"; else echo "Disabled"; fi)"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Initialize validation log
    initialize_validation_log
    
    # Run validation phases
    run_original_validation
    run_hardware_validation
    run_firmware_size_validation
    run_dependency_validation
    run_gpio_validation
    run_existing_validations
    check_build_system
    
    # Run comprehensive tests if requested
    run_comprehensive_hardware_tests
    
    # Measure performance
    measure_performance
    
    # Generate outputs
    provide_fix_suggestions
    generate_comprehensive_report
    
    # Show final summary
    show_final_summary
}

# Error handling
trap 'log_error "Enhanced validation interrupted"; exit 1' INT TERM

# Execute main function
main "$@"