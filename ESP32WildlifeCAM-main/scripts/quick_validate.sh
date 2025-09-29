#!/bin/bash
# Quick Environment Validation Script for ESP32WildlifeCAM
# Fast environment check (30 seconds max)
# Usage: ./scripts/quick_validate.sh [--verbose] [--quiet]

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
QUIET=false
START_TIME=$(date +%s)

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --quiet|-q)
            QUIET=true
            shift
            ;;
        --help|-h)
            echo "Quick Environment Validation Script for ESP32WildlifeCAM"
            echo "Usage: $0 [--verbose] [--quiet] [--help]"
            echo ""
            echo "Options:"
            echo "  --verbose, -v    Enable verbose output"
            echo "  --quiet, -q      Suppress non-essential output"
            echo "  --help, -h       Show this help message"
            echo ""
            echo "This script performs a fast environment validation in under 30 seconds:"
            echo "  • Offline development setup validation"
            echo "  • GPIO conflict status check"
            echo "  • PlatformIO cache verification"
            echo "  • Critical file existence check"
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
    if [[ "$QUIET" != "true" ]]; then
        echo -e "${BLUE}[INFO]${NC} $1"
    fi
}

log_success() {
    if [[ "$QUIET" != "true" ]]; then
        echo -e "${GREEN}[SUCCESS]${NC} $1"
    fi
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

check_result() {
    local result=$1
    local description=$2
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if [[ $result -eq 0 ]]; then
        log_success "$description"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        return 0
    else
        log_error "$description"
        return 1
    fi
}

# Main validation functions
validate_project_structure() {
    log_info "Validating project structure..."
    
    local critical_files=(
        "UNIFIED_DEVELOPMENT_PLAN.md"
        "validate_fixes.py"
        "firmware/platformio.ini"
        "firmware/src/config.h"
    )
    
    local critical_dirs=(
        "firmware/src"
        "firmware/src/hal"
        "docs"
    )
    
    for file in "${critical_files[@]}"; do
        if [[ -f "$PROJECT_ROOT/$file" ]]; then
            log_verbose "Found critical file: $file"
        else
            check_result 1 "Missing critical file: $file"
            return 1
        fi
    done
    
    for dir in "${critical_dirs[@]}"; do
        if [[ -d "$PROJECT_ROOT/$dir" ]]; then
            log_verbose "Found critical directory: $dir"
        else
            check_result 1 "Missing critical directory: $dir"
            return 1
        fi
    done
    
    check_result 0 "Project structure validation"
}

validate_offline_setup() {
    log_info "Validating offline development setup..."
    
    # Check for offline validation script
    if [[ -f "$PROJECT_ROOT/firmware/offline_validate.sh" ]]; then
        log_verbose "Found offline validation script"
    else
        log_warning "Offline validation script not found"
    fi
    
    # Check for PlatformIO configuration
    if [[ -f "$PROJECT_ROOT/firmware/platformio.ini" ]]; then
        if grep -q "esp32cam" "$PROJECT_ROOT/firmware/platformio.ini"; then
            log_verbose "PlatformIO ESP32-CAM configuration found"
        else
            log_warning "ESP32-CAM environment not found in platformio.ini"
        fi
    fi
    
    # Check for local package cache indicators
    if [[ -d "$HOME/.platformio" ]]; then
        log_verbose "PlatformIO home directory exists"
    else
        log_warning "PlatformIO not initialized - run setup_offline_environment.sh"
    fi
    
    check_result 0 "Offline development setup validation"
}

validate_gpio_conflicts() {
    log_info "Checking GPIO conflict status..."
    
    # Run quick GPIO validation using existing script
    cd "$PROJECT_ROOT"
    
    if python3 -c "
import sys
sys.path.append('.')
from validate_fixes import check_gpio_pin_conflicts
try:
    result = check_gpio_pin_conflicts()
    sys.exit(0 if result else 1)
except Exception as e:
    print(f'GPIO validation error: {e}')
    sys.exit(1)
" 2>/dev/null; then
        check_result 0 "GPIO conflict check"
    else
        check_result 1 "GPIO conflict detected - run validate_fixes.py for details"
    fi
}

validate_platformio_cache() {
    log_info "Verifying PlatformIO cache status..."
    
    # Check for common PlatformIO cache locations
    local cache_locations=(
        "$HOME/.platformio/packages"
        "$HOME/.platformio/platforms"
        "$PROJECT_ROOT/.pio"
    )
    
    local cache_found=false
    for cache_dir in "${cache_locations[@]}"; do
        if [[ -d "$cache_dir" ]]; then
            local cache_size=$(du -sh "$cache_dir" 2>/dev/null | cut -f1 || echo "unknown")
            log_verbose "Cache found: $cache_dir ($cache_size)"
            cache_found=true
        fi
    done
    
    if [[ "$cache_found" == "true" ]]; then
        check_result 0 "PlatformIO cache verification"
    else
        log_warning "No PlatformIO cache found - first build may require network"
        check_result 0 "PlatformIO cache verification (warning only)"
    fi
}

validate_development_tools() {
    log_info "Checking development tools availability..."
    
    local tools=("python3" "git")
    local optional_tools=("pio" "platformio")
    
    for tool in "${tools[@]}"; do
        if command -v "$tool" >/dev/null 2>&1; then
            local version
            case $tool in
                python3)
                    version=$(python3 --version 2>&1 | cut -d' ' -f2)
                    ;;
                git)
                    version=$(git --version 2>&1 | cut -d' ' -f3)
                    ;;
            esac
            log_verbose "Required tool available: $tool ($version)"
        else
            check_result 1 "Required tool missing: $tool"
            return 1
        fi
    done
    
    for tool in "${optional_tools[@]}"; do
        if command -v "$tool" >/dev/null 2>&1; then
            log_verbose "Optional tool available: $tool"
        else
            log_verbose "Optional tool not in PATH: $tool"
        fi
    done
    
    check_result 0 "Development tools validation"
}

# Performance monitoring
show_performance_summary() {
    local end_time=$(date +%s)
    local duration=$((end_time - START_TIME))
    
    echo ""
    echo "=== Quick Validation Summary ==="
    echo "Execution time: ${duration}s"
    echo "Checks passed: $PASSED_CHECKS/$TOTAL_CHECKS"
    
    if [[ $duration -gt 30 ]]; then
        log_warning "Validation took longer than 30 seconds target"
    else
        log_success "Validation completed within 30 second target"
    fi
    
    if [[ $PASSED_CHECKS -eq $TOTAL_CHECKS ]]; then
        log_success "All validation checks passed!"
        echo ""
        echo "✅ Environment ready for ESP32WildlifeCAM development"
        echo "Next steps:"
        echo "  • Run './scripts/validate_component.py camera' for component testing"
        echo "  • Run './scripts/dev_mode.sh' to start development session"
        echo "  • See UNIFIED_DEVELOPMENT_PLAN.md for development workflow"
        return 0
    else
        log_error "Some validation checks failed"
        echo ""
        echo "❌ Environment needs attention"
        echo "Recommendations:"
        echo "  • Run './scripts/setup_offline_environment.sh' for full setup"
        echo "  • Run 'python3 validate_fixes.py' for detailed diagnostics"
        echo "  • Check UNIFIED_DEVELOPMENT_PLAN.md for setup instructions"
        return 1
    fi
}

# Main execution
main() {
    if [[ "$QUIET" != "true" ]]; then
        echo "ESP32WildlifeCAM Quick Environment Validation"
        echo "============================================="
        echo "Target: Complete validation in under 30 seconds"
        echo ""
    fi
    
    cd "$PROJECT_ROOT"
    
    # Run validation checks
    validate_project_structure
    validate_offline_setup
    validate_gpio_conflicts
    validate_platformio_cache
    validate_development_tools
    
    # Show results
    show_performance_summary
}

# Error handling
trap 'log_error "Script interrupted"; exit 1' INT TERM

# Execute main function
main "$@"