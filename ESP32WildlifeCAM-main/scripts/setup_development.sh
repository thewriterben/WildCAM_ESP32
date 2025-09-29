#!/bin/bash

# ESP32 Wildlife CAM - Development Environment Setup Script
# Comprehensive setup for ESP32 Wildlife CAM development environment

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
ESP32 Wildlife CAM - Development Environment Setup

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -h, --help                  Show this help message
    -v, --verbose               Enable verbose output
    --offline                   Setup for offline development only
    --complete                  Complete setup including all tools
    --minimal                   Minimal setup for basic development
    --cache-all                 Cache all dependencies for offline use
    --validate-only             Only validate existing setup
    --force                     Force reinstall even if tools exist

SETUP MODES:
    minimal     Basic PlatformIO and ESP32 platform
    standard    Above plus development tools and validation
    complete    Above plus advanced tools and documentation
    offline     Complete offline-capable setup with cached dependencies

EXAMPLES:
    $0 --complete               # Full setup with all features
    $0 --offline --cache-all    # Offline development setup
    $0 --minimal --validate-only # Check minimal setup
    $0 --force --complete       # Force reinstall everything

EOF
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install Python and pip
install_python() {
    local os_type
    os_type=$(detect_os)
    
    log_info "Installing Python and pip..."
    
    if command_exists python3 && command_exists pip3; then
        log_success "Python and pip already installed"
        return 0
    fi
    
    case "$os_type" in
        linux)
            if command_exists apt; then
                sudo apt update
                sudo apt install -y python3 python3-pip python3-venv
            elif command_exists yum; then
                sudo yum install -y python3 python3-pip
            elif command_exists pacman; then
                sudo pacman -S python python-pip
            else
                log_error "Unsupported Linux distribution"
                return 1
            fi
            ;;
        macos)
            if command_exists brew; then
                brew install python3
            else
                log_error "Homebrew not found. Please install Homebrew first."
                return 1
            fi
            ;;
        windows)
            log_error "Please install Python from python.org manually"
            return 1
            ;;
        *)
            log_error "Unsupported operating system"
            return 1
            ;;
    esac
    
    log_success "Python and pip installed"
}

# Install Node.js and npm
install_nodejs() {
    log_info "Installing Node.js and npm..."
    
    if command_exists node && command_exists npm; then
        log_success "Node.js and npm already installed"
        return 0
    fi
    
    local os_type
    os_type=$(detect_os)
    
    case "$os_type" in
        linux)
            # Install Node.js via NodeSource repository
            curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
            sudo apt-get install -y nodejs
            ;;
        macos)
            if command_exists brew; then
                brew install node
            else
                log_error "Homebrew not found"
                return 1
            fi
            ;;
        windows)
            log_error "Please install Node.js from nodejs.org manually"
            return 1
            ;;
        *)
            log_error "Unsupported operating system"
            return 1
            ;;
    esac
    
    log_success "Node.js and npm installed"
}

# Install PlatformIO
install_platformio() {
    log_info "Installing PlatformIO..."
    
    if command_exists pio; then
        if [[ "${FORCE_INSTALL:-false}" == "true" ]]; then
            log_warning "Forcing PlatformIO reinstall"
            pip3 uninstall -y platformio
        else
            log_success "PlatformIO already installed"
            return 0
        fi
    fi
    
    # Install PlatformIO via pip
    pip3 install --user platformio
    
    # Add to PATH if not already there
    local pio_path="$HOME/.local/bin"
    if [[ ":$PATH:" != *":$pio_path:"* ]]; then
        echo "export PATH=\"$pio_path:\$PATH\"" >> ~/.bashrc
        export PATH="$pio_path:$PATH"
    fi
    
    # Verify installation
    if ! command_exists pio; then
        log_error "PlatformIO installation failed"
        return 1
    fi
    
    log_success "PlatformIO installed successfully"
}

# Setup ESP32 platform and dependencies
setup_esp32_platform() {
    log_info "Setting up ESP32 platform..."
    
    cd "$PROJECT_ROOT"
    
    # Install ESP32 platform
    pio platform install espressif32
    
    # Install required libraries
    log_info "Installing required libraries..."
    
    local libraries=(
        "ArduinoJson"
        "AsyncTCP"
        "ESPAsyncWebServer"
        "WiFiManager"
        "PubSubClient"
        "Adafruit BME280 Library"
        "OneWire"
        "DallasTemperature"
        "ESP32Servo"
    )
    
    for lib in "${libraries[@]}"; do
        pio lib -g install "$lib"
    done
    
    # Cache ESP32 tools and packages for offline use
    if [[ "${CACHE_DEPENDENCIES:-false}" == "true" ]]; then
        log_info "Caching ESP32 dependencies for offline use..."
        pio platform update espressif32
        pio run --target upload-port  # This will download all tools
    fi
    
    log_success "ESP32 platform setup complete"
}

# Install development tools
install_development_tools() {
    log_info "Installing development tools..."
    
    local os_type
    os_type=$(detect_os)
    
    # Git
    if ! command_exists git; then
        case "$os_type" in
            linux)
                sudo apt install -y git
                ;;
            macos)
                if command_exists brew; then
                    brew install git
                fi
                ;;
            windows)
                log_warning "Please install Git from git-scm.com manually"
                ;;
        esac
    fi
    
    # Essential Python packages
    pip3 install --user \
        pyserial \
        matplotlib \
        numpy \
        pandas \
        requests \
        Pillow \
        opencv-python \
        scikit-image
    
    # Code formatting and linting tools
    pip3 install --user \
        black \
        flake8 \
        pylint \
        autopep8
    
    log_success "Development tools installed"
}

# Setup VS Code and extensions
setup_vscode() {
    log_info "Setting up VS Code extensions..."
    
    if ! command_exists code; then
        log_warning "VS Code not found. Please install VS Code manually."
        return 0
    fi
    
    # Install essential extensions
    local extensions=(
        "ms-vscode.cpptools"
        "platformio.platformio-ide"
        "ms-python.python"
        "ms-vscode.vscode-json"
        "redhat.vscode-yaml"
        "ms-vscode.hexeditor"
        "ms-vscode.cmake-tools"
        "twxs.cmake"
    )
    
    for ext in "${extensions[@]}"; do
        code --install-extension "$ext" || log_warning "Failed to install extension: $ext"
    done
    
    # Create workspace configuration
    cat > "$PROJECT_ROOT/.vscode/settings.json" << EOF
{
    "platformio-ide.useBuiltinPIOCore": true,
    "platformio-ide.disablePIOHomeStartup": true,
    "python.defaultInterpreterPath": "python3",
    "C_Cpp.default.includePath": [
        "\${workspaceFolder}/**",
        "~/.platformio/packages/framework-arduinoespressif32/cores/esp32/**",
        "~/.platformio/packages/framework-arduinoespressif32/libraries/**"
    ],
    "files.associations": {
        "*.ino": "cpp",
        "*.h": "c",
        "*.hpp": "cpp"
    }
}
EOF
    
    log_success "VS Code configured for ESP32 development"
}

# Create project configuration
create_project_config() {
    log_info "Creating project configuration files..."
    
    cd "$PROJECT_ROOT"
    
    # Create development configuration
    cat > "dev_config.json" << EOF
{
    "project_name": "ESP32WildlifeCAM",
    "version": "2.0.0",
    "board": "esp32cam",
    "framework": "arduino",
    "development": {
        "serial_speed": 115200,
        "upload_speed": 921600,
        "monitor_filters": ["esp32_exception_decoder"],
        "build_flags": [
            "-DCORE_DEBUG_LEVEL=4",
            "-DDEV_MODE=1"
        ]
    },
    "production": {
        "build_flags": [
            "-DCORE_DEBUG_LEVEL=1",
            "-DDEV_MODE=0",
            "-O2"
        ]
    }
}
EOF
    
    # Create environment-specific files
    if [[ ! -f "wifi_config.h" ]]; then
        cat > "wifi_config.h" << EOF
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// WiFi Configuration - DO NOT COMMIT TO VERSION CONTROL
// Copy this file and customize for your environment

#define WIFI_SSID "your_wifi_network"
#define WIFI_PASSWORD "your_wifi_password"

// Optional: Access Point mode for field configuration
#define AP_SSID "ESP32-WildlifeCAM-Setup"
#define AP_PASSWORD "wildlife123"

#endif // WIFI_CONFIG_H
EOF
    fi
    
    # Update .gitignore to protect sensitive files
    if [[ -f ".gitignore" ]]; then
        if ! grep -q "wifi_config.h" .gitignore; then
            echo "" >> .gitignore
            echo "# Development environment specific files" >> .gitignore
            echo "wifi_config.h" >> .gitignore
            echo "dev_config_local.json" >> .gitignore
            echo ".vscode/c_cpp_properties.json" >> .gitignore
        fi
    fi
    
    log_success "Project configuration created"
}

# Setup offline development environment
setup_offline_environment() {
    log_info "Setting up offline development environment..."
    
    # Create offline tools directory
    mkdir -p "$PROJECT_ROOT/tools/offline"
    cd "$PROJECT_ROOT/tools/offline"
    
    # Download offline documentation
    if command_exists wget; then
        log_info "Downloading offline documentation..."
        
        # ESP32 documentation (if accessible)
        wget -r -np -k -L -p -e robots=off -P esp32-docs/ \
            "https://docs.espressif.com/projects/esp-idf/en/latest/" \
            2>/dev/null || log_warning "Could not download ESP32 docs (firewall/network issue)"
        
        # Arduino reference (if accessible)
        wget -r -np -k -L -p -e robots=off -P arduino-docs/ \
            "https://www.arduino.cc/reference/en/" \
            2>/dev/null || log_warning "Could not download Arduino docs (firewall/network issue)"
    fi
    
    # Create offline build script
    cat > "$PROJECT_ROOT/tools/offline_build.sh" << 'EOF'
#!/bin/bash
# Offline build script for ESP32 Wildlife CAM

set -e

PROJECT_ROOT="$(dirname "$(dirname "$(readlink -f "$0")")")"
cd "$PROJECT_ROOT"

echo "Building ESP32 Wildlife CAM (offline mode)..."

# Check if PlatformIO is available
if ! command -v pio >/dev/null 2>&1; then
    echo "Error: PlatformIO not found"
    exit 1
fi

# Build the project
pio run

echo "Build completed successfully"
EOF
    
    chmod +x "$PROJECT_ROOT/tools/offline_build.sh"
    
    log_success "Offline environment configured"
}

# Validate installation
validate_installation() {
    log_info "Validating installation..."
    
    local errors=0
    
    # Check essential tools
    local required_tools=(
        "python3:Python 3"
        "pip3:pip3"
        "pio:PlatformIO"
        "git:Git"
    )
    
    for tool_desc in "${required_tools[@]}"; do
        IFS=':' read -r tool name <<< "$tool_desc"
        if command_exists "$tool"; then
            log_success "$name: $(command -v "$tool")"
        else
            log_error "$name not found"
            ((errors++))
        fi
    done
    
    # Check PlatformIO platforms
    if command_exists pio; then
        if pio platform list | grep -q "espressif32"; then
            log_success "ESP32 platform installed"
        else
            log_error "ESP32 platform not installed"
            ((errors++))
        fi
    fi
    
    # Check project structure
    cd "$PROJECT_ROOT"
    
    local required_files=(
        "platformio.ini:PlatformIO configuration"
        "src:Source directory"
        "docs:Documentation directory"
        "scripts:Scripts directory"
    )
    
    for file_desc in "${required_files[@]}"; do
        IFS=':' read -r file name <<< "$file_desc"
        if [[ -e "$file" ]]; then
            log_success "$name: $file"
        else
            log_warning "$name not found: $file"
        fi
    done
    
    # Test build
    if [[ "${VALIDATE_BUILD:-true}" == "true" ]]; then
        log_info "Testing build..."
        if pio run --dry-run > /dev/null 2>&1; then
            log_success "Build configuration valid"
        else
            log_error "Build configuration has issues"
            ((errors++))
        fi
    fi
    
    if [[ $errors -eq 0 ]]; then
        log_success "Installation validation completed successfully"
        return 0
    else
        log_error "Installation validation failed with $errors errors"
        return 1
    fi
}

# Show setup summary
show_summary() {
    log_info "Setup Summary:"
    echo ""
    echo "Development Environment: ESP32 Wildlife CAM"
    echo "Setup Mode: ${SETUP_MODE:-standard}"
    echo "Project Root: $PROJECT_ROOT"
    echo ""
    
    if command_exists pio; then
        echo "PlatformIO Version: $(pio --version)"
    fi
    
    if command_exists python3; then
        echo "Python Version: $(python3 --version)"
    fi
    
    echo ""
    echo "Next Steps:"
    echo "1. Source your shell configuration: source ~/.bashrc"
    echo "2. Test the setup: ./scripts/quick_validate.sh"
    echo "3. Build the project: pio run"
    echo "4. Upload to device: pio run --target upload"
    echo ""
    echo "Documentation:"
    echo "- Project docs: docs/"
    echo "- Hardware setup: docs/Hardware_Setup.md"
    echo "- Development guide: UNIFIED_DEVELOPMENT_PLAN.md"
}

# Parse command line arguments
parse_args() {
    SETUP_MODE="standard"
    OFFLINE_MODE=false
    CACHE_DEPENDENCIES=false
    VALIDATE_ONLY=false
    FORCE_INSTALL=false
    VERBOSE=false
    
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
            --offline)
                OFFLINE_MODE=true
                SETUP_MODE="offline"
                shift
                ;;
            --complete)
                SETUP_MODE="complete"
                shift
                ;;
            --minimal)
                SETUP_MODE="minimal"
                shift
                ;;
            --cache-all)
                CACHE_DEPENDENCIES=true
                shift
                ;;
            --validate-only)
                VALIDATE_ONLY=true
                shift
                ;;
            --force)
                FORCE_INSTALL=true
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Main setup function
main() {
    log_info "ESP32 Wildlife CAM Development Environment Setup"
    log_info "Setup Mode: $SETUP_MODE"
    echo ""
    
    cd "$PROJECT_ROOT"
    
    if [[ "$VALIDATE_ONLY" == "true" ]]; then
        validate_installation
        return $?
    fi
    
    # Core installation steps
    case "$SETUP_MODE" in
        minimal)
            install_python
            install_platformio
            setup_esp32_platform
            ;;
        standard)
            install_python
            install_platformio
            setup_esp32_platform
            install_development_tools
            create_project_config
            ;;
        complete|offline)
            install_python
            install_nodejs
            install_platformio
            setup_esp32_platform
            install_development_tools
            setup_vscode
            create_project_config
            setup_offline_environment
            ;;
    esac
    
    # Validation
    if ! validate_installation; then
        log_error "Setup validation failed"
        exit 1
    fi
    
    show_summary
    log_success "Development environment setup completed!"
}

# Execute if run directly
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    parse_args "$@"
    main
fi