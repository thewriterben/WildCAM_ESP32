#!/bin/bash
# Pre-Commit Validation Script for ESP32WildlifeCAM
# Code quality validation before commits
# Usage: ./scripts/pre_commit_checks.sh [--fix] [--verbose] [--quick]

set -e

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERBOSE=false
FIX_MODE=false
QUICK_MODE=false
START_TIME=$(date +%s)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --fix|-f)
            FIX_MODE=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --quick|-q)
            QUICK_MODE=true
            shift
            ;;
        --help|-h)
            echo "Pre-Commit Validation Script for ESP32WildlifeCAM"
            echo "Usage: $0 [--fix] [--verbose] [--quick] [--help]"
            echo ""
            echo "Options:"
            echo "  --fix, -f        Auto-fix issues where possible"
            echo "  --verbose, -v    Enable verbose output"
            echo "  --quick, -q      Run only essential checks (faster)"
            echo "  --help, -h       Show this help message"
            echo ""
            echo "This script performs pre-commit validation:"
            echo "  • Code quality validation"
            echo "  • GPIO conflict checking"
            echo "  • Memory leak detection"
            echo "  • Power consumption impact assessment"
            echo "  • Configuration consistency"
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

# Progress tracking
TOTAL_CHECKS=0
PASSED_CHECKS=0
FIXED_ISSUES=0

check_result() {
    local result=$1
    local description=$2
    local can_fix=$3
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if [[ $result -eq 0 ]]; then
        log_success "$description"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        return 0
    else
        if [[ "$FIX_MODE" == "true" && "$can_fix" == "true" ]]; then
            log_warning "$description - attempting auto-fix"
            return 2  # Indicates fixable issue
        else
            log_error "$description"
            return 1
        fi
    fi
}

# Code quality checks
check_code_formatting() {
    log_info "Checking code formatting..."
    
    local issues_found=false
    
    # Check for consistent indentation in C++ files
    find "$PROJECT_ROOT/firmware/src" -name "*.cpp" -o -name "*.h" | while read -r file; do
        if [[ -f "$file" ]]; then
            # Check for mixed tabs and spaces
            if grep -P '\t' "$file" >/dev/null && grep -P '^    ' "$file" >/dev/null; then
                log_warning "Mixed tabs and spaces in: $(basename "$file")"
                if [[ "$FIX_MODE" == "true" ]]; then
                    # Convert tabs to spaces
                    expand -t 4 "$file" > "${file}.tmp" && mv "${file}.tmp" "$file"
                    log_verbose "Fixed indentation in: $(basename "$file")"
                    FIXED_ISSUES=$((FIXED_ISSUES + 1))
                else
                    issues_found=true
                fi
            fi
            
            # Check for trailing whitespace
            if grep -n ' $' "$file" >/dev/null; then
                log_warning "Trailing whitespace in: $(basename "$file")"
                if [[ "$FIX_MODE" == "true" ]]; then
                    # Remove trailing whitespace
                    sed -i 's/[[:space:]]*$//' "$file"
                    log_verbose "Fixed trailing whitespace in: $(basename "$file")"
                    FIXED_ISSUES=$((FIXED_ISSUES + 1))
                else
                    issues_found=true
                fi
            fi
        fi
    done
    
    if [[ "$issues_found" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

check_gpio_conflicts() {
    log_info "Checking GPIO pin conflicts..."
    
    cd "$PROJECT_ROOT"
    
    # Use existing validation script
    if python3 validate_fixes.py >/dev/null 2>&1; then
        log_verbose "GPIO validation passed"
        return 0
    else
        log_error "GPIO conflicts detected"
        log_info "Run 'python3 validate_fixes.py' for detailed analysis"
        return 1
    fi
}

check_memory_management() {
    log_info "Checking memory management patterns..."
    
    local issues_found=false
    
    # Check for potential memory leaks in C++ files
    find "$PROJECT_ROOT/firmware/src" -name "*.cpp" | while read -r file; do
        if [[ -f "$file" ]]; then
            local malloc_count=$(grep -c 'malloc\|new\|calloc' "$file" 2>/dev/null || true)
            local free_count=$(grep -c 'free\|delete' "$file" 2>/dev/null || true)
            
            if [[ $malloc_count -gt 0 && $free_count -eq 0 ]]; then
                log_warning "Potential memory leak in: $(basename "$file") (allocations without frees)"
                issues_found=true
            fi
            
            # Check for frame buffer management
            local fb_get=$(grep -c 'esp_camera_fb_get\|fb_get' "$file" 2>/dev/null || true)
            local fb_return=$(grep -c 'esp_camera_fb_return\|fb_return' "$file" 2>/dev/null || true)
            
            if [[ $fb_get -gt 0 && $fb_return -eq 0 ]]; then
                log_warning "Potential frame buffer leak in: $(basename "$file")"
                issues_found=true
            fi
        fi
    done
    
    if [[ "$issues_found" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

check_power_impact() {
    log_info "Checking power consumption impact..."
    
    local issues_found=false
    
    # Check for power-intensive operations without sleep
    find "$PROJECT_ROOT/firmware/src" -name "*.cpp" | while read -r file; do
        if [[ -f "$file" ]]; then
            # Check for WiFi operations without proper power management
            if grep -q 'WiFi\.' "$file" && ! grep -q 'esp_wifi_set_ps\|WiFi.setSleep' "$file"; then
                log_warning "WiFi usage without power management in: $(basename "$file")"
                issues_found=true
            fi
            
            # Check for camera operations without timeout
            if grep -q 'esp_camera_fb_get' "$file" && ! grep -q 'timeout\|delay' "$file"; then
                log_warning "Camera operations without timeout in: $(basename "$file")"
                issues_found=true
            fi
            
            # Check for infinite loops without yield
            if grep -q 'while.*true\|for.*;;' "$file" && ! grep -q 'yield\|delay\|vTaskDelay' "$file"; then
                log_warning "Infinite loop without yield in: $(basename "$file")"
                issues_found=true
            fi
        fi
    done
    
    if [[ "$issues_found" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

check_configuration_consistency() {
    log_info "Checking configuration consistency..."
    
    local config_files=(
        "$PROJECT_ROOT/firmware/src/config.h"
        "$PROJECT_ROOT/include/config_unified.h"
        "$PROJECT_ROOT/include/pins.h"
    )
    
    local issues_found=false
    
    # Check for configuration conflicts between files
    for config_file in "${config_files[@]}"; do
        if [[ -f "$config_file" ]]; then
            log_verbose "Checking configuration file: $(basename "$config_file")"
            
            # Check for undefined or conflicting feature flags
            if grep -q '#define.*_ENABLED.*true' "$config_file" && grep -q '#define.*_ENABLED.*false' "$config_file"; then
                log_warning "Conflicting feature flags in: $(basename "$config_file")"
                issues_found=true
            fi
            
            # Check for duplicate pin definitions
            local pin_defs=$(grep -E '#define.*_PIN\s+[0-9]+' "$config_file" | cut -d' ' -f3 | sort)
            local unique_pins=$(echo "$pin_defs" | sort -u)
            
            if [[ $(echo "$pin_defs" | wc -l) -ne $(echo "$unique_pins" | wc -l) ]]; then
                log_warning "Duplicate pin assignments detected in: $(basename "$config_file")"
                issues_found=true
            fi
        fi
    done
    
    if [[ "$issues_found" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

check_documentation() {
    log_info "Checking documentation consistency..."
    
    local issues_found=false
    
    # Check for TODO/FIXME comments that should be resolved
    local todo_count=$(find "$PROJECT_ROOT/firmware/src" -name "*.cpp" -o -name "*.h" | xargs grep -c 'TODO\|FIXME' 2>/dev/null | awk -F: '{sum += $2} END {print sum+0}')
    
    if [[ $todo_count -gt 50 ]]; then
        log_warning "High number of TODO/FIXME comments: $todo_count"
        log_info "Consider resolving some before committing"
        issues_found=true
    else
        log_verbose "TODO/FIXME count acceptable: $todo_count"
    fi
    
    # Check for missing function documentation in header files
    find "$PROJECT_ROOT/firmware/src" -name "*.h" | while read -r file; do
        if [[ -f "$file" ]]; then
            local function_count=$(grep -c '^[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*[[:space:]]*(' "$file" 2>/dev/null || true)
            local doc_count=$(grep -c '/\*\*\|///' "$file" 2>/dev/null || true)
            
            if [[ $function_count -gt 5 && $doc_count -eq 0 ]]; then
                log_warning "Missing documentation in header: $(basename "$file")"
                issues_found=true
            fi
        fi
    done
    
    if [[ "$issues_found" == "true" ]]; then
        return 1
    else
        return 0
    fi
}

check_build_compatibility() {
    log_info "Checking build compatibility..."
    
    # Quick syntax check using existing offline validation
    if [[ -f "$PROJECT_ROOT/firmware/offline_validate.sh" ]]; then
        cd "$PROJECT_ROOT/firmware"
        if bash offline_validate.sh >/dev/null 2>&1; then
            log_verbose "Offline build validation passed"
            return 0
        else
            log_error "Build compatibility issues detected"
            return 1
        fi
    else
        log_warning "Offline validation script not found - skipping build check"
        return 0
    fi
}

# Performance summary
show_summary() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo ""
    echo "=== Pre-Commit Validation Summary ==="
    echo "Execution time: ${duration}s"
    echo "Checks passed: $PASSED_CHECKS/$TOTAL_CHECKS"
    
    if [[ $FIXED_ISSUES -gt 0 ]]; then
        echo "Issues auto-fixed: $FIXED_ISSUES"
    fi
    
    if [[ $PASSED_CHECKS -eq $TOTAL_CHECKS ]]; then
        log_success "All pre-commit checks passed!"
        echo ""
        echo "✅ Code is ready for commit"
        echo "Next steps:"
        echo "  • git add ."
        echo "  • git commit -m 'Your commit message'"
        echo "  • git push"
        return 0
    else
        log_error "Some pre-commit checks failed"
        echo ""
        echo "❌ Code needs attention before commit"
        echo "Recommendations:"
        if [[ "$FIX_MODE" != "true" ]]; then
            echo "  • Run with --fix to auto-fix some issues"
        fi
        echo "  • Review failed checks above"
        echo "  • Run 'python3 validate_fixes.py' for detailed analysis"
        echo "  • Check UNIFIED_DEVELOPMENT_PLAN.md for guidelines"
        return 1
    fi
}

# Main execution
main() {
    echo "ESP32WildlifeCAM Pre-Commit Validation"
    echo "====================================="
    echo "Mode: $(if [[ "$QUICK_MODE" == "true" ]]; then echo "Quick"; else echo "Comprehensive"; fi)"
    echo "Auto-fix: $(if [[ "$FIX_MODE" == "true" ]]; then echo "Enabled"; else echo "Disabled"; fi)"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Essential checks (always run)
    check_gpio_conflicts
    check_result $? "GPIO conflict check" false
    
    check_configuration_consistency
    check_result $? "Configuration consistency" false
    
    if [[ "$QUICK_MODE" != "true" ]]; then
        # Comprehensive checks
        check_code_formatting
        check_result $? "Code formatting" true
        
        check_memory_management
        check_result $? "Memory management patterns" false
        
        check_power_impact
        check_result $? "Power consumption impact" false
        
        check_documentation
        check_result $? "Documentation consistency" false
        
        check_build_compatibility
        check_result $? "Build compatibility" false
    fi
    
    # Show results
    show_summary
}

# Error handling
trap 'log_error "Script interrupted"; exit 1' INT TERM

# Execute main function
main "$@"