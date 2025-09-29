#!/bin/bash

# ESP32WildlifeCAM Complete Offline Development Setup Script
# This script prepares a completely self-contained development environment
# for ESP32WildlifeCAM that works without internet access

set -e  # Exit on any error

# Configuration
OFFLINE_DIR="$HOME/esp32_offline_dev"
PROJECT_NAME="ESP32WildlifeCAM"
SCRIPT_VERSION="1.0.0"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Function to check internet connectivity
check_internet() {
    if ping -c 1 google.com &> /dev/null; then
        return 0
    else
        return 1
    fi
}

# Function to download with retry
download_with_retry() {
    local url=$1
    local output=$2
    local retries=3
    
    for i in $(seq 1 $retries); do
        log_info "Downloading $url (attempt $i/$retries)"
        if wget -c -t 3 -T 30 "$url" -O "$output"; then
            log_success "Downloaded $url"
            return 0
        else
            log_warning "Failed to download $url (attempt $i/$retries)"
            sleep 5
        fi
    done
    
    log_error "Failed to download $url after $retries attempts"
    return 1
}

# Function to create directory structure
setup_directories() {
    log_info "Creating offline development directory structure..."
    
    mkdir -p "$OFFLINE_DIR"/{packages,tools,docs,projects,cache,libraries}
    mkdir -p "$OFFLINE_DIR"/packages/{platformio,arduino,python,espressif}
    mkdir -p "$OFFLINE_DIR"/docs/{esp32,platformio,arduino,3d_printing}
    mkdir -p "$OFFLINE_DIR"/tools/{esptool,compilers,utilities}
    
    log_success "Directory structure created at $OFFLINE_DIR"
}

# Function to install required system packages
install_system_deps() {
    log_info "Installing system dependencies..."
    
    if command -v apt-get >/dev/null; then
        sudo apt-get update
        sudo apt-get install -y \
            python3 python3-pip python3-venv \
            git wget curl unzip tar \
            build-essential cmake \
            minicom screen \
            usbutils
    elif command -v yum >/dev/null; then
        sudo yum install -y \
            python3 python3-pip \
            git wget curl unzip tar \
            gcc gcc-c++ make cmake \
            minicom screen \
            usbutils
    elif command -v pacman >/dev/null; then
        sudo pacman -Sy --noconfirm \
            python python-pip \
            git wget curl unzip tar \
            base-devel cmake \
            minicom screen \
            usbutils
    else
        log_warning "Unknown package manager. Please install dependencies manually."
    fi
    
    log_success "System dependencies installed"
}

# Function to setup Python virtual environment
setup_python_env() {
    log_info "Setting up Python virtual environment..."
    
    cd "$OFFLINE_DIR"
    python3 -m venv venv
    source venv/bin/activate
    
    # Upgrade pip
    pip install --upgrade pip
    
    # Install PlatformIO
    pip install platformio
    
    # Install additional tools
    pip install esptool pyserial
    
    log_success "Python environment setup complete"
}

# Function to download PlatformIO packages
download_platformio_packages() {
    log_info "Downloading PlatformIO packages..."
    
    source "$OFFLINE_DIR/venv/bin/activate"
    
    # Install ESP32 platform
    pio platform install espressif32@6.4.0
    
    # Install essential libraries
    pio lib install "ArduinoJson@^6.21.3"
    pio lib install "sandeepmistry/LoRa@^0.8.0"
    pio lib install "adafruit/Adafruit BME280 Library@^2.2.2"
    
    # Copy packages to offline directory
    cp -r "$HOME/.platformio/packages" "$OFFLINE_DIR/packages/platformio/"
    cp -r "$HOME/.platformio/platforms" "$OFFLINE_DIR/packages/platformio/"
    
    log_success "PlatformIO packages downloaded"
}

# Function to download ESP32 development tools
download_esp32_tools() {
    log_info "Downloading ESP32 development tools..."
    
    cd "$OFFLINE_DIR/tools"
    
    # ESP32 Arduino Core
    if [ ! -d "arduino-esp32" ]; then
        git clone --recursive https://github.com/espressif/arduino-esp32.git
        cd arduino-esp32
        git checkout tags/2.0.14  # Stable version
        cd ..
    fi
    
    # ESP-IDF
    if [ ! -d "esp-idf" ]; then
        git clone --recursive https://github.com/espressif/esp-idf.git
        cd esp-idf
        git checkout tags/v4.4.6  # LTS version
        cd ..
    fi
    
    # ESP32 Camera library
    if [ ! -d "esp32-camera" ]; then
        git clone https://github.com/espressif/esp32-camera.git
    fi
    
    log_success "ESP32 tools downloaded"
}

# Function to download toolchains
download_toolchains() {
    log_info "Downloading ESP32 toolchains..."
    
    cd "$OFFLINE_DIR/tools"
    
    # Determine architecture
    ARCH=$(uname -m)
    if [ "$ARCH" = "x86_64" ]; then
        TOOLCHAIN_ARCH="linux-amd64"
    elif [ "$ARCH" = "aarch64" ]; then
        TOOLCHAIN_ARCH="linux-arm64"
    else
        log_warning "Unsupported architecture: $ARCH"
        return 1
    fi
    
    # Download ESP32 toolchain
    TOOLCHAIN_URL="https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_4_0-esp-2020r3-$TOOLCHAIN_ARCH.tar.gz"
    if [ ! -f "xtensa-esp32-elf.tar.gz" ]; then
        download_with_retry "$TOOLCHAIN_URL" "xtensa-esp32-elf.tar.gz"
        tar -xzf xtensa-esp32-elf.tar.gz
    fi
    
    # Download ESP32-S3 toolchain
    S3_TOOLCHAIN_URL="https://dl.espressif.com/dl/xtensa-esp32s3-elf-gcc8_4_0-esp-2021r2-patch5-$TOOLCHAIN_ARCH.tar.gz"
    if [ ! -f "xtensa-esp32s3-elf.tar.gz" ]; then
        download_with_retry "$S3_TOOLCHAIN_URL" "xtensa-esp32s3-elf.tar.gz"
        tar -xzf xtensa-esp32s3-elf.tar.gz
    fi
    
    log_success "Toolchains downloaded"
}

# Function to download documentation
download_documentation() {
    log_info "Downloading documentation..."
    
    cd "$OFFLINE_DIR/docs"
    
    # ESP32 Documentation (selective download)
    wget -r -l 3 -k -p -E -nc -R "*.pdf" \
        https://docs.espressif.com/projects/esp-idf/en/latest/ || true
    
    # PlatformIO Documentation
    wget -r -l 2 -k -p -E -nc \
        https://docs.platformio.org/en/latest/platforms/espressif32.html || true
    
    # Arduino ESP32 Documentation
    wget -r -l 2 -k -p -E -nc \
        https://arduino-esp32.readthedocs.io/ || true
    
    log_success "Documentation downloaded"
}

# Function to clone project repository
clone_project() {
    log_info "Cloning ESP32WildlifeCAM project..."
    
    cd "$OFFLINE_DIR/projects"
    
    if [ ! -d "$PROJECT_NAME" ]; then
        git clone --recursive https://github.com/thewriterben/ESP32WildlifeCAM.git
        cd "$PROJECT_NAME"
        
        # Create development branch
        git checkout -b offline-development
        
        log_success "Project cloned and development branch created"
    else
        log_info "Project already exists, updating..."
        cd "$PROJECT_NAME"
        git fetch origin
        git merge origin/main
        log_success "Project updated"
    fi
}

# Function to create offline configuration
create_offline_config() {
    log_info "Creating offline configuration files..."
    
    # PlatformIO offline configuration
    cat > "$OFFLINE_DIR/platformio_offline.ini" << EOF
[platformio]
; Offline development configuration
packages_dir = $OFFLINE_DIR/packages/platformio/packages
cache_dir = $OFFLINE_DIR/cache
lib_dir = $OFFLINE_DIR/libraries

; Disable automatic updates
check_platformio_interval = never
check_prune_system_threshold = never

[env]
; Use local toolchain
platform_packages = 
    toolchain-xtensa-esp32@file://$OFFLINE_DIR/tools/xtensa-esp32-elf
    framework-arduinoespressif32@file://$OFFLINE_DIR/tools/arduino-esp32
EOF
    
    # Create activation script
    cat > "$OFFLINE_DIR/activate_offline.sh" << 'EOF'
#!/bin/bash
# Activate offline development environment

# Set environment variables
export PLATFORMIO_CACHE_DIR="$OFFLINE_DIR/cache"
export PLATFORMIO_LIB_DIR="$OFFLINE_DIR/libraries"
export PLATFORMIO_PACKAGES_DIR="$OFFLINE_DIR/packages/platformio/packages"
export PLATFORMIO_NO_INTERNET=1

# Add tools to PATH
export PATH="$OFFLINE_DIR/tools/xtensa-esp32-elf/bin:$PATH"
export PATH="$OFFLINE_DIR/tools/xtensa-esp32s3-elf/bin:$PATH"

# Activate Python virtual environment
source "$OFFLINE_DIR/venv/bin/activate"

echo "ESP32WildlifeCAM offline development environment activated"
echo "Project directory: $OFFLINE_DIR/projects/ESP32WildlifeCAM"
echo "Tools directory: $OFFLINE_DIR/tools"
echo "Documentation: $OFFLINE_DIR/docs"

# Change to project directory
cd "$OFFLINE_DIR/projects/ESP32WildlifeCAM"
EOF
    
    chmod +x "$OFFLINE_DIR/activate_offline.sh"
    
    # Create build script
    cat > "$OFFLINE_DIR/build_offline.sh" << 'EOF'
#!/bin/bash
# Offline build script for ESP32WildlifeCAM

set -e

# Activate offline environment
source "$OFFLINE_DIR/activate_offline.sh"

# Build for all environments
echo "Building ESP32WildlifeCAM for all environments..."

pio run -e esp32cam
pio run -e esp32s3cam

echo "Build complete. Firmware files are in .pio/build/"
EOF
    
    chmod +x "$OFFLINE_DIR/build_offline.sh"
    
    log_success "Offline configuration created"
}

# Function to download Python packages for offline use
download_python_packages() {
    log_info "Downloading Python packages for offline use..."
    
    source "$OFFLINE_DIR/venv/bin/activate"
    
    mkdir -p "$OFFLINE_DIR/packages/python"
    
    # Download commonly needed packages
    pip download -d "$OFFLINE_DIR/packages/python" \
        platformio esptool pyserial requests flask \
        numpy matplotlib pillow opencv-python
    
    log_success "Python packages downloaded"
}

# Function to create emergency recovery tools
create_recovery_tools() {
    log_info "Creating emergency recovery tools..."
    
    # Emergency ESP32 flash script
    cat > "$OFFLINE_DIR/emergency_flash.sh" << 'EOF'
#!/bin/bash
# Emergency ESP32 flash recovery

PORT=${1:-/dev/ttyUSB0}
FIRMWARE=${2:-firmware.bin}

echo "Emergency flashing ESP32..."
echo "Port: $PORT"
echo "Firmware: $FIRMWARE"

# Reset to bootloader mode
echo "Put ESP32 in bootloader mode (hold BOOT, press RESET, release BOOT)"
read -p "Press Enter when ready..."

# Flash firmware
python3 -m esptool --chip esp32 --port $PORT --baud 460800 write_flash 0x10000 $FIRMWARE

echo "Flash complete. Press RESET to boot."
EOF
    
    chmod +x "$OFFLINE_DIR/emergency_flash.sh"
    
    # System diagnostics script
    cat > "$OFFLINE_DIR/system_diagnostics.sh" << 'EOF'
#!/bin/bash
# System diagnostics for offline development

echo "=== ESP32WildlifeCAM Offline Development Diagnostics ==="
echo "Date: $(date)"
echo "User: $(whoami)"
echo "System: $(uname -a)"
echo ""

echo "=== USB Devices ==="
lsusb | grep -E "(CP2102|CH340|FTDI)"
echo ""

echo "=== Serial Ports ==="
ls -la /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || echo "No serial ports found"
echo ""

echo "=== Python Environment ==="
source "$OFFLINE_DIR/venv/bin/activate"
python3 --version
pip --version
pio --version
echo ""

echo "=== Disk Space ==="
df -h "$OFFLINE_DIR"
echo ""

echo "=== Environment Variables ==="
env | grep -E "(PLATFORMIO|ESP32|IDF)" | sort
echo ""

echo "=== Project Status ==="
if [ -d "$OFFLINE_DIR/projects/ESP32WildlifeCAM" ]; then
    cd "$OFFLINE_DIR/projects/ESP32WildlifeCAM"
    git status --porcelain
    echo "Git branch: $(git branch --show-current)"
else
    echo "Project not found"
fi
EOF
    
    chmod +x "$OFFLINE_DIR/system_diagnostics.sh"
    
    log_success "Recovery tools created"
}

# Function to test offline setup
test_offline_setup() {
    log_info "Testing offline development setup..."
    
    source "$OFFLINE_DIR/activate_offline.sh"
    
    # Test PlatformIO
    if pio --version >/dev/null 2>&1; then
        log_success "PlatformIO working"
    else
        log_error "PlatformIO not working"
        return 1
    fi
    
    # Test toolchain
    if xtensa-esp32-elf-gcc --version >/dev/null 2>&1; then
        log_success "ESP32 toolchain working"
    else
        log_error "ESP32 toolchain not working"
        return 1
    fi
    
    # Test project build (compile only)
    if [ -d "$OFFLINE_DIR/projects/ESP32WildlifeCAM" ]; then
        cd "$OFFLINE_DIR/projects/ESP32WildlifeCAM"
        if pio check --environment esp32cam >/dev/null 2>&1; then
            log_success "Project structure valid"
        else
            log_warning "Project structure issues detected"
        fi
    fi
    
    log_success "Offline setup test complete"
}

# Function to create README
create_readme() {
    cat > "$OFFLINE_DIR/README.md" << EOF
# ESP32WildlifeCAM Offline Development Environment

This directory contains a complete offline development environment for ESP32WildlifeCAM.

## Quick Start

1. Activate the environment:
   \`\`\`bash
   source activate_offline.sh
   \`\`\`

2. Build the project:
   \`\`\`bash
   ./build_offline.sh
   \`\`\`

3. Run diagnostics:
   \`\`\`bash
   ./system_diagnostics.sh
   \`\`\`

## Directory Structure

- \`packages/\` - Offline packages and libraries
- \`tools/\` - Development tools and toolchains
- \`docs/\` - Downloaded documentation
- \`projects/\` - Project source code
- \`cache/\` - Build cache
- \`venv/\` - Python virtual environment

## Emergency Procedures

- Flash recovery: \`./emergency_flash.sh\`
- System diagnostics: \`./system_diagnostics.sh\`
- Manual activation: \`source activate_offline.sh\`

## Maintenance

- Update project: \`git pull\` in projects/ESP32WildlifeCAM
- Clean cache: \`rm -rf cache/*\`
- Reset environment: Re-run offline_complete_setup.sh

## Support

Refer to docs/development/firewall_complete_guide.md for detailed offline development procedures.

Created: $(date)
Version: $SCRIPT_VERSION
EOF
    
    log_success "README created"
}

# Main execution
main() {
    echo "========================================"
    echo " ESP32WildlifeCAM Offline Setup v$SCRIPT_VERSION"
    echo "========================================"
    echo ""
    
    if ! check_internet; then
        log_error "Internet connection required for initial setup"
        log_info "Please run this script with internet access first"
        exit 1
    fi
    
    log_info "Starting offline development environment setup..."
    log_info "This may take 30-60 minutes depending on your connection"
    echo ""
    
    # Execute setup steps
    setup_directories
    install_system_deps
    setup_python_env
    download_platformio_packages
    download_esp32_tools
    download_toolchains
    download_documentation
    clone_project
    create_offline_config
    download_python_packages
    create_recovery_tools
    create_readme
    test_offline_setup
    
    echo ""
    echo "========================================"
    log_success "Offline development environment setup complete!"
    echo "========================================"
    echo ""
    log_info "Environment location: $OFFLINE_DIR"
    log_info "To activate: source $OFFLINE_DIR/activate_offline.sh"
    log_info "To build: $OFFLINE_DIR/build_offline.sh"
    log_info "Documentation: $OFFLINE_DIR/README.md"
    echo ""
    log_info "You can now develop ESP32WildlifeCAM without internet access"
    
    # Final size report
    SIZE=$(du -sh "$OFFLINE_DIR" | cut -f1)
    log_info "Total environment size: $SIZE"
}

# Execute main function
main "$@"