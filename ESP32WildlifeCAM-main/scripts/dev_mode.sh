#!/bin/bash
# Development Mode Helper Script for ESP32WildlifeCAM
# Quick development environment activation
# Usage: ./scripts/dev_mode.sh [component] [--setup] [--validate] [--monitor]

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
COMPONENT=""
SETUP_MODE=false
VALIDATE_MODE=false
MONITOR_MODE=false
INTERACTIVE_MODE=true

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --setup|-s)
            SETUP_MODE=true
            INTERACTIVE_MODE=false
            shift
            ;;
        --validate|-v)
            VALIDATE_MODE=true
            INTERACTIVE_MODE=false
            shift
            ;;
        --monitor|-m)
            MONITOR_MODE=true
            INTERACTIVE_MODE=false
            shift
            ;;
        --help|-h)
            echo "Development Mode Helper Script for ESP32WildlifeCAM"
            echo "Usage: $0 [component] [--setup] [--validate] [--monitor] [--help]"
            echo ""
            echo "Components:"
            echo "  camera     Focus on camera driver development"
            echo "  power      Focus on power management development"
            echo "  motion     Focus on motion detection development"
            echo "  storage    Focus on storage system development"
            echo "  all        Work on all components"
            echo ""
            echo "Options:"
            echo "  --setup, -s      Set up development environment for component"
            echo "  --validate, -v   Run validation for component"
            echo "  --monitor, -m    Enter continuous monitoring mode"
            echo "  --help, -h       Show this help message"
            echo ""
            echo "Interactive Mode (default):"
            echo "  Provides an interactive menu for development tasks"
            echo ""
            echo "This script provides:"
            echo "  • Quick development environment activation"
            echo "  • Component-focused development sessions"
            echo "  • Continuous validation during development"
            echo "  • Smart dependency management"
            echo "  • Development workflow automation"
            exit 0
            ;;
        camera|power|motion|storage|all)
            COMPONENT="$1"
            shift
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

log_dev() {
    echo -e "${PURPLE}[DEV]${NC} $1"
}

log_monitor() {
    echo -e "${CYAN}[MONITOR]${NC} $1"
}

# Environment setup functions
setup_development_environment() {
    log_dev "Setting up development environment..."
    
    # Ensure we're in the project root
    cd "$PROJECT_ROOT"
    
    # Check quick validation
    log_info "Running quick environment validation..."
    if ./scripts/quick_validate.sh --quiet; then
        log_success "Environment validation passed"
    else
        log_warning "Environment validation issues detected"
        log_info "Running setup to fix issues..."
        ./scripts/setup_offline_environment.sh --cache-only
    fi
    
    # Set up git hooks for development
    setup_git_hooks
    
    # Create development session info
    create_dev_session_info
    
    log_success "Development environment ready"
}

setup_git_hooks() {
    log_info "Setting up git hooks for development..."
    
    local hooks_dir="$PROJECT_ROOT/.git/hooks"
    
    # Create pre-commit hook
    cat > "$hooks_dir/pre-commit" << 'EOF'
#!/bin/bash
# Pre-commit hook for ESP32WildlifeCAM

echo "Running pre-commit validation..."
cd "$(git rev-parse --show-toplevel)"

# Run pre-commit checks
if [[ -f "scripts/pre_commit_checks.sh" ]]; then
    ./scripts/pre_commit_checks.sh --quick
else
    echo "Pre-commit script not found - skipping validation"
fi
EOF
    
    chmod +x "$hooks_dir/pre-commit"
    log_dev "Git pre-commit hook installed"
}

create_dev_session_info() {
    local session_file="$PROJECT_ROOT/.dev_session"
    
    cat > "$session_file" << EOF
# ESP32WildlifeCAM Development Session
# Created: $(date)
# Component focus: ${COMPONENT:-"all"}

# Quick Commands
alias validate="python3 validate_fixes.py"
alias quick="./scripts/quick_validate.sh"
alias component="python3 scripts/validate_component.py"
alias precommit="./scripts/pre_commit_checks.sh"
alias priority="./scripts/priority_tasks.sh"

# Component-specific aliases
alias test-camera="python3 scripts/validate_component.py camera --simulation"
alias test-power="python3 scripts/validate_component.py power --simulation"
alias test-motion="python3 scripts/validate_component.py motion --simulation"
alias test-storage="python3 scripts/validate_component.py storage --simulation"

# Development workflow aliases
alias dev-setup="./scripts/priority_tasks.sh all --setup"
alias dev-validate="./scripts/priority_tasks.sh all --validate"
alias dev-integrate="./scripts/integration_test.sh --simulation"

# Build aliases (when PlatformIO available)
alias build="cd firmware && pio run -e esp32cam"
alias upload="cd firmware && pio run -e esp32cam -t upload"
alias monitor="cd firmware && pio device monitor"

echo "ESP32WildlifeCAM Development Session Active"
echo "Use 'validate', 'quick', 'component', etc. for common tasks"
EOF
    
    log_dev "Development session aliases created in .dev_session"
    log_info "Source with: source .dev_session"
}

# Component-specific development functions
start_camera_development() {
    log_dev "Starting camera development session..."
    
    # Set up camera development environment
    ./scripts/priority_tasks.sh 1 --setup
    
    echo ""
    log_info "Camera Development Quick Start:"
    echo "  1. Edit: firmware/src/camera_handler.cpp"
    echo "  2. Test:  python3 scripts/validate_component.py camera --simulation"
    echo "  3. Validate: ./tools/validate_camera_dev.sh"
    echo "  4. GPIO Check: ./scripts/quick_validate.sh"
    echo ""
    echo "Key files to work on:"
    echo "  • firmware/src/camera_handler.cpp - Main implementation"
    echo "  • firmware/src/hal/camera_config.h - Configuration"
    echo "  • firmware/src/config.h - GPIO pin assignments"
    echo ""
    echo "Development workflow:"
    echo "  Edit → Test → Validate → Commit"
}

start_power_development() {
    log_dev "Starting power management development session..."
    
    # Set up power management development environment
    ./scripts/priority_tasks.sh 2 --setup
    
    echo ""
    log_info "Power Management Development Quick Start:"
    echo "  1. Edit: firmware/src/power_manager.cpp"
    echo "  2. Test:  python3 scripts/validate_component.py power --simulation"
    echo "  3. Validate: ./tools/validate_power_dev.sh"
    echo "  4. Hardware Test: ./scripts/hardware_test.sh --component power --simulation"
    echo ""
    echo "Key files to work on:"
    echo "  • firmware/src/power_manager.cpp - Main implementation"
    echo "  • firmware/include/power/power_manager.h - Interface"
    echo "  • firmware/src/config.h - Power GPIO pins"
    echo ""
    echo "Development focus:"
    echo "  • Battery voltage reading (ADC)"
    echo "  • Solar panel monitoring"
    echo "  • Power state management"
    echo "  • Sleep mode optimization"
}

start_motion_development() {
    log_dev "Starting motion detection development session..."
    
    # Set up motion detection development environment
    ./scripts/priority_tasks.sh 3 --setup
    
    echo ""
    log_info "Motion Detection Development Quick Start:"
    echo "  1. Edit: firmware/src/motion_filter.cpp"
    echo "  2. Test:  python3 scripts/validate_component.py motion --simulation"
    echo "  3. Validate: ./tools/validate_motion_dev.sh"
    echo "  4. Integration: ./scripts/integration_test.sh --quick --simulation"
    echo ""
    echo "Key files to work on:"
    echo "  • firmware/src/motion_filter.cpp - Main implementation"
    echo "  • firmware/include/motion/motion_detector.h - Interface"
    echo "  • firmware/src/multi_board/board_node.cpp - Integration"
    echo ""
    echo "Development focus:"
    echo "  • PIR sensor reading (GPIO 1)"
    echo "  • Motion event filtering"
    echo "  • Camera trigger integration"
    echo "  • False positive reduction"
}

start_storage_development() {
    log_dev "Starting storage development session..."
    
    echo ""
    log_info "Storage Development Quick Start:"
    echo "  1. Test:  python3 scripts/validate_component.py storage --simulation"
    echo "  2. Hardware: ./scripts/hardware_test.sh --component storage --simulation"
    echo "  3. Check: ./scripts/quick_validate.sh"
    echo ""
    echo "Key files to work on:"
    echo "  • src/data/storage_manager.cpp - Storage implementation"
    echo "  • firmware/src/config.h - SD card configuration"
    echo ""
    echo "Development focus:"
    echo "  • SD card initialization and management"
    echo "  • File system operations"
    echo "  • Storage space management"
    echo "  • Power-aware storage strategies"
}

# Continuous validation monitoring
start_monitoring_mode() {
    log_monitor "Starting continuous development monitoring..."
    
    echo ""
    echo "=== Development Monitor ==="
    echo "Monitoring code changes and running validations..."
    echo "Press Ctrl+C to stop monitoring"
    echo ""
    
    local last_check=0
    local check_interval=10  # seconds
    
    while true; do
        local current_time=$(date +%s)
        
        if [[ $((current_time - last_check)) -ge $check_interval ]]; then
            echo ""
            log_monitor "Running periodic validation... $(date)"
            
            # Run quick validation
            if ./scripts/quick_validate.sh --quiet; then
                log_success "Quick validation: PASSED"
            else
                log_warning "Quick validation: ISSUES DETECTED"
            fi
            
            # Check component status if specific component selected
            if [[ -n "$COMPONENT" && "$COMPONENT" != "all" ]]; then
                if python3 scripts/validate_component.py "$COMPONENT" --simulation >/dev/null 2>&1; then
                    log_success "$COMPONENT component: VALIDATED"
                else
                    log_warning "$COMPONENT component: NEEDS ATTENTION"
                fi
            fi
            
            # Show priority task progress
            local overall_progress=$(./scripts/priority_tasks.sh all | grep "Overall Priority Tasks Progress" | grep -o '[0-9]\+%' || echo "0%")
            log_monitor "Priority tasks progress: $overall_progress"
            
            last_check=$current_time
        fi
        
        sleep 2
    done
}

# Interactive development menu
show_interactive_menu() {
    clear
    echo "=========================================="
    echo "  ESP32WildlifeCAM Development Mode"
    echo "=========================================="
    echo ""
    echo "Choose your development focus:"
    echo ""
    echo "1) Camera Driver Development"
    echo "2) Power Management Development"
    echo "3) Motion Detection Development"
    echo "4) Storage System Development"
    echo "5) All Components (Full Development)"
    echo ""
    echo "Quick Actions:"
    echo "6) Run Quick Validation"
    echo "7) Run Integration Tests"
    echo "8) Check Priority Task Progress"
    echo "9) Start Monitoring Mode"
    echo ""
    echo "0) Exit"
    echo ""
    echo -n "Select option [0-9]: "
}

handle_interactive_choice() {
    local choice=$1
    
    case $choice in
        1)
            COMPONENT="camera"
            start_camera_development
            ;;
        2)
            COMPONENT="power"
            start_power_development
            ;;
        3)
            COMPONENT="motion"
            start_motion_development
            ;;
        4)
            COMPONENT="storage"
            start_storage_development
            ;;
        5)
            COMPONENT="all"
            log_dev "Setting up full development environment..."
            ./scripts/priority_tasks.sh all --setup
            echo ""
            log_info "Full development environment ready"
            echo "Run './scripts/priority_tasks.sh --progress' to see current status"
            ;;
        6)
            log_info "Running quick validation..."
            ./scripts/quick_validate.sh
            ;;
        7)
            log_info "Running integration tests..."
            ./scripts/integration_test.sh --quick --simulation
            ;;
        8)
            log_info "Checking priority task progress..."
            ./scripts/priority_tasks.sh all
            ;;
        9)
            start_monitoring_mode
            ;;
        0)
            log_info "Exiting development mode"
            exit 0
            ;;
        *)
            log_error "Invalid choice: $choice"
            ;;
    esac
}

run_interactive_mode() {
    # Set up development environment first
    setup_development_environment
    
    while true; do
        echo ""
        show_interactive_menu
        read -r choice
        echo ""
        
        handle_interactive_choice "$choice"
        
        if [[ "$choice" != "0" ]]; then
            echo ""
            echo "Press Enter to return to menu..."
            read -r
        fi
    done
}

# Development session summary
show_development_summary() {
    echo ""
    echo "=== Development Session Summary ==="
    
    # Show current environment status
    log_info "Environment Status:"
    if ./scripts/quick_validate.sh --quiet >/dev/null 2>&1; then
        log_success "Development environment: READY"
    else
        log_warning "Development environment: NEEDS ATTENTION"
    fi
    
    # Show priority task progress
    echo ""
    log_info "Priority Task Progress:"
    local camera_progress=$(./scripts/priority_tasks.sh 1 | grep -o '[0-9]\+%' | head -1 || echo "0%")
    local power_progress=$(./scripts/priority_tasks.sh 2 | grep -o '[0-9]\+%' | head -1 || echo "0%")
    local motion_progress=$(./scripts/priority_tasks.sh 3 | grep -o '[0-9]\+%' | head -1 || echo "0%")
    
    echo "  • Camera Driver: $camera_progress"
    echo "  • Power Management: $power_progress"
    echo "  • Motion Detection: $motion_progress"
    
    # Show next steps
    echo ""
    log_info "Recommended Next Steps:"
    if [[ -n "$COMPONENT" ]]; then
        case "$COMPONENT" in
            camera)
                echo "  • Implement camera_init() function"
                echo "  • Test with: python3 scripts/validate_component.py camera --simulation"
                ;;
            power)
                echo "  • Implement power_manager_init() with ADC setup"
                echo "  • Test with: python3 scripts/validate_component.py power --simulation"
                ;;
            motion)
                echo "  • Implement motion_detector_init() with PIR GPIO"
                echo "  • Test with: python3 scripts/validate_component.py motion --simulation"
                ;;
            storage)
                echo "  • Test SD card functionality"
                echo "  • Test with: python3 scripts/validate_component.py storage --simulation"
                ;;
        esac
    else
        echo "  • Run: ./scripts/priority_tasks.sh 1 --setup"
        echo "  • Begin camera driver implementation"
        echo "  • Use: ./scripts/dev_mode.sh for guided development"
    fi
    
    echo ""
    log_info "Development Tools Available:"
    echo "  • Quick validation: ./scripts/quick_validate.sh"
    echo "  • Component testing: python3 scripts/validate_component.py [component]"
    echo "  • Priority tasks: ./scripts/priority_tasks.sh [1-3]"
    echo "  • Integration testing: ./scripts/integration_test.sh"
    echo "  • Hardware testing: ./scripts/hardware_test.sh"
}

# Main execution
main() {
    echo "ESP32WildlifeCAM Development Mode"
    echo "================================"
    echo "Optimized for offline/firewall-constrained development"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    # Handle non-interactive modes
    if [[ "$SETUP_MODE" == "true" ]]; then
        setup_development_environment
        if [[ -n "$COMPONENT" && "$COMPONENT" != "all" ]]; then
            case "$COMPONENT" in
                camera) start_camera_development ;;
                power) start_power_development ;;
                motion) start_motion_development ;;
                storage) start_storage_development ;;
            esac
        fi
        show_development_summary
        
    elif [[ "$VALIDATE_MODE" == "true" ]]; then
        if [[ -n "$COMPONENT" && "$COMPONENT" != "all" ]]; then
            log_info "Validating $COMPONENT component..."
            python3 scripts/validate_component.py "$COMPONENT" --simulation
        else
            log_info "Running comprehensive validation..."
            ./scripts/quick_validate.sh
            ./scripts/priority_tasks.sh all --validate
        fi
        
    elif [[ "$MONITOR_MODE" == "true" ]]; then
        start_monitoring_mode
        
    elif [[ "$INTERACTIVE_MODE" == "true" ]]; then
        run_interactive_mode
        
    else
        # Default: show development summary
        show_development_summary
    fi
}

# Error handling
trap 'log_error "Development mode interrupted"; exit 1' INT TERM

# Execute main function
main "$@"