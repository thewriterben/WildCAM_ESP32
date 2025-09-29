#!/bin/bash
# setup_federated_learning.sh
# Complete setup script for federated learning in offline environments

set -euo pipefail

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
LOG_FILE="$PROJECT_ROOT/setup_federated_learning.log"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log() {
    echo -e "$(date '+%Y-%m-%d %H:%M:%S') - $1" | tee -a "$LOG_FILE"
}

log_error() {
    echo -e "${RED}ERROR: $1${NC}" | tee -a "$LOG_FILE"
}

log_warning() {
    echo -e "${YELLOW}WARNING: $1${NC}" | tee -a "$LOG_FILE"
}

log_info() {
    echo -e "${BLUE}INFO: $1${NC}" | tee -a "$LOG_FILE"
}

log_success() {
    echo -e "${GREEN}SUCCESS: $1${NC}" | tee -a "$LOG_FILE"
}

# Check if running with required permissions
check_permissions() {
    log_info "Checking permissions..."
    
    if [[ ! -w "$PROJECT_ROOT" ]]; then
        log_error "No write permission to project directory: $PROJECT_ROOT"
        exit 1
    fi
    
    log_success "Permissions check passed"
}

# Validate offline environment
validate_offline_environment() {
    log_info "Validating offline development environment..."
    
    # Check for required tools
    local required_tools=("pio" "python3" "pip3" "git")
    local missing_tools=()
    
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        log_error "Missing required tools: ${missing_tools[*]}"
        log_info "Please install missing tools before continuing"
        exit 1
    fi
    
    # Check PlatformIO installation
    if ! pio --version &> /dev/null; then
        log_error "PlatformIO not properly installed"
        exit 1
    fi
    
    # Verify no network access (optional check)
    if ping -c 1 8.8.8.8 &> /dev/null; then
        log_warning "Network access detected - ensure offline mode for production deployment"
    else
        log_info "Offline mode confirmed"
    fi
    
    log_success "Environment validation passed"
}

# Setup federated learning directories
setup_directories() {
    log_info "Setting up federated learning directory structure..."
    
    local directories=(
        "$PROJECT_ROOT/firmware/src/ai/federated_learning"
        "$PROJECT_ROOT/models/federated"
        "$PROJECT_ROOT/scripts/federated_learning"
        "$PROJECT_ROOT/docs/ai"
        "$PROJECT_ROOT/tests/federated_learning"
        "$PROJECT_ROOT/configs/federated"
    )
    
    for dir in "${directories[@]}"; do
        if [[ ! -d "$dir" ]]; then
            mkdir -p "$dir"
            log_info "Created directory: $dir"
        else
            log_info "Directory exists: $dir"
        fi
    done
    
    log_success "Directory structure setup complete"
}

# Validate federated learning implementation
validate_implementation() {
    log_info "Validating federated learning implementation..."
    
    local required_files=(
        "$PROJECT_ROOT/firmware/src/ai/federated_learning/federated_learning_system.h"
        "$PROJECT_ROOT/firmware/src/ai/federated_learning/federated_learning_system.cpp"
        "$PROJECT_ROOT/firmware/src/ai/federated_learning/local_training_module.h"
        "$PROJECT_ROOT/firmware/src/ai/federated_learning/local_training_module.cpp"
        "$PROJECT_ROOT/firmware/src/ai/federated_learning/privacy_preserving_aggregation.h"
        "$PROJECT_ROOT/firmware/src/ai/wildlife_classifier.h"
        "$PROJECT_ROOT/firmware/src/ai/wildlife_classifier.cpp"
        "$PROJECT_ROOT/firmware/src/ai/model_aggregator.h"
        "$PROJECT_ROOT/firmware/src/ai/model_aggregator.cpp"
    )
    
    local missing_files=()
    
    for file in "${required_files[@]}"; do
        if [[ ! -f "$file" ]]; then
            missing_files+=("$(basename "$file")")
        fi
    done
    
    if [[ ${#missing_files[@]} -gt 0 ]]; then
        log_error "Missing implementation files: ${missing_files[*]}"
        log_info "Please ensure all federated learning components are implemented"
        exit 1
    fi
    
    log_success "Implementation validation passed"
}

# Configure PlatformIO for federated learning
configure_platformio() {
    log_info "Configuring PlatformIO for federated learning..."
    
    local platformio_ini="$PROJECT_ROOT/platformio.ini"
    
    # Check if TensorFlow Lite dependency exists
    if grep -q "tensorflow/TensorFlow Lite for Microcontrollers" "$platformio_ini"; then
        log_info "TensorFlow Lite dependency already configured"
    else
        log_warning "TensorFlow Lite dependency not found in platformio.ini"
        log_info "Please add: tensorflow/TensorFlow Lite for Microcontrollers@^2.4.0"
    fi
    
    # Check build flags for AI support
    if grep -q "BOARD_HAS_PSRAM" "$platformio_ini"; then
        log_info "PSRAM support already configured"
    else
        log_warning "PSRAM support not configured"
    fi
    
    log_success "PlatformIO configuration checked"
}

# Setup model files
setup_models() {
    log_info "Setting up model files..."
    
    local model_dir="$PROJECT_ROOT/models/federated"
    
    # Create model placeholder files if they don't exist
    local models=(
        "wildlife_classifier_v1.tflite"
        "behavior_model_v1.tflite"
        "environment_adapter_v1.tflite"
    )
    
    for model in "${models[@]}"; do
        local model_file="$model_dir/$model"
        if [[ ! -f "$model_file" ]]; then
            # Create minimal placeholder model file
            echo "# Placeholder model file for $model" > "$model_file.placeholder"
            log_info "Created placeholder for: $model"
        fi
    done
    
    log_success "Model setup complete"
}

# Create configuration files
create_configurations() {
    log_info "Creating federated learning configurations..."
    
    local config_dir="$PROJECT_ROOT/configs/federated"
    local config_file="$config_dir/federated_config.json"
    
    if [[ ! -f "$config_file" ]]; then
        cat > "$config_file" << 'EOF'
{
  "federatedLearning": {
    "enabled": true,
    "contributionThreshold": 0.85,
    "minSamplesBeforeContribution": 50,
    "maxContributionsPerDay": 20,
    "privacyLevel": "HIGH",
    "privacyEpsilon": 0.5,
    "anonymizeLocation": true,
    "anonymizeTimestamps": true
  },
  "training": {
    "learningRate": 0.001,
    "localEpochs": 3,
    "batchSize": 4,
    "maxTrainingTimeMs": 180000
  },
  "network": {
    "topology": "MESH",
    "maxNodes": 15,
    "maxHops": 3,
    "protocolPriority": ["WiFi", "LoRa", "Cellular"]
  },
  "power": {
    "enablePowerManagement": true,
    "minBatteryLevel": 0.3,
    "solarOptimized": true
  }
}
EOF
        log_info "Created configuration file: $config_file"
    else
        log_info "Configuration file already exists: $config_file"
    fi
    
    log_success "Configuration setup complete"
}

# Test compilation
test_compilation() {
    log_info "Testing federated learning compilation..."
    
    cd "$PROJECT_ROOT"
    
    # Test compilation without uploading
    if pio run --environment esp32cam --target checkprogsize --silent >> "$LOG_FILE" 2>&1; then
        log_success "Compilation test passed"
    else
        log_error "Compilation failed - check implementation"
        log_info "Check log file: $LOG_FILE"
        return 1
    fi
}

# Generate documentation
generate_documentation() {
    log_info "Generating federated learning documentation..."
    
    local docs_dir="$PROJECT_ROOT/docs/ai"
    
    # Create basic documentation files
    cat > "$docs_dir/federated_learning_setup.md" << 'EOF'
# Federated Learning Setup Guide

## Overview
This guide covers the setup and configuration of federated learning for ESP32WildlifeCAM.

## Prerequisites
- ESP32 with PSRAM
- PlatformIO development environment
- Offline development capability
- TensorFlow Lite Micro support

## Configuration
Edit `configs/federated/federated_config.json` to customize:
- Privacy settings
- Training parameters
- Network topology
- Power management

## Usage
1. Initialize federated learning system
2. Configure wildlife classifier
3. Start federated training
4. Monitor through web interface

## Troubleshooting
- Check memory usage
- Verify model files
- Monitor power consumption
- Validate network connectivity
EOF
    
    log_info "Created setup documentation"
    
    log_success "Documentation generation complete"
}

# Verify system integrity
verify_system() {
    log_info "Verifying federated learning system integrity..."
    
    # Check memory requirements
    local available_memory
    available_memory=$(free -m | grep "Mem:" | awk '{print $7}')
    
    if [[ "$available_memory" -lt 512 ]]; then
        log_warning "Low system memory: ${available_memory}MB (recommended: >512MB)"
    fi
    
    # Check disk space
    local available_space
    available_space=$(df "$PROJECT_ROOT" | tail -1 | awk '{print $4}')
    
    if [[ "$available_space" -lt 1048576 ]]; then  # 1GB in KB
        log_warning "Low disk space: $(($available_space / 1024))MB (recommended: >1GB)"
    fi
    
    log_success "System integrity verification complete"
}

# Main setup function
main() {
    log_info "Starting federated learning setup..."
    log_info "Project root: $PROJECT_ROOT"
    
    # Create log file
    touch "$LOG_FILE"
    
    # Run setup steps
    check_permissions
    validate_offline_environment
    setup_directories
    validate_implementation
    configure_platformio
    setup_models
    create_configurations
    
    # Optional steps (can fail without stopping setup)
    if test_compilation; then
        log_success "Compilation test passed"
    else
        log_warning "Compilation test failed - check implementation"
    fi
    
    generate_documentation
    verify_system
    
    # Final summary
    log_success "Federated learning setup completed successfully!"
    log_info "Next steps:"
    log_info "1. Review configuration in configs/federated/federated_config.json"
    log_info "2. Add actual model files to models/federated/"
    log_info "3. Test compilation with 'pio run --environment esp32cam'"
    log_info "4. Deploy to ESP32 devices"
    log_info ""
    log_info "Log file: $LOG_FILE"
}

# Script entry point
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi