# Network-Free ESP32 Validation Commands

*Comprehensive command reference for ESP32 Wildlife Camera offline development*

## Overview

This document provides complete network-free validation commands for ESP32 Wildlife Camera development in restricted environments. All commands work completely offline and provide comprehensive testing coverage.

## Quick Start

```bash
# Run complete validation suite
cd firmware/
./enhanced_offline_validate.sh --verbose --report

# Test specific components
./validate_esp32_hardware.sh --simulation --component camera
./validate_gpio_config.sh --board esp32s3 --verbose
./validate_firmware_size.sh --verbose
./validate_dependencies.sh --cache-check --verbose
```

## Validation Scripts

### 1. Hardware Validation Script

**Location:** `firmware/validate_esp32_hardware.sh`

**Purpose:** Comprehensive offline ESP32 hardware testing

**Usage:**
```bash
./validate_esp32_hardware.sh [OPTIONS]

Options:
  --component, -c  Test specific component (gpio|i2c|spi|camera|sd|psram|flash|power|all)
  --verbose, -v    Enable verbose output
  --simulation, -s Enable simulation mode (no real hardware required)
  --help, -h       Show help message
```

**Examples:**
```bash
# Test all hardware components in simulation mode
./validate_esp32_hardware.sh --simulation --verbose

# Test only GPIO functionality
./validate_esp32_hardware.sh --component gpio --simulation

# Test camera module (requires hardware or simulation)
./validate_esp32_hardware.sh --component camera --verbose

# Test I2C and SPI interfaces
./validate_esp32_hardware.sh --component i2c --simulation
./validate_esp32_hardware.sh --component spi --simulation
```

**What it tests:**
- GPIO pin functionality and constraints
- I2C device detection and communication
- SPI device detection and communication  
- Camera module connectivity and configuration
- SD card accessibility and performance
- PSRAM detection and memory tests
- Flash memory verification and health
- Power supply validation and measurements

### 2. Firmware Size Validation Script

**Location:** `firmware/validate_firmware_size.sh`

**Purpose:** Validates memory constraints before flashing

**Usage:**
```bash
./validate_firmware_size.sh [OPTIONS]

Options:
  --environment, -e  PlatformIO environment (default: esp32-s3-devkitc-1)
  --build, -b        Build firmware before validation
  --verbose, -v      Enable verbose output
  --help, -h         Show help message
```

**Examples:**
```bash
# Validate firmware size without building
./validate_firmware_size.sh --verbose

# Build firmware first, then validate
./validate_firmware_size.sh --build --verbose

# Validate for specific ESP32 environment
./validate_firmware_size.sh --environment esp32-s3-devkitc-1 --verbose

# Check OTA compatibility
./validate_firmware_size.sh --build --verbose
```

**What it validates:**
- Compiled firmware size against ESP32 flash capacity
- RAM usage and availability
- PSRAM usage (if available)
- Partition table compatibility
- Bootloader size limits
- OTA update size requirements

### 3. Dependency Verification Script

**Location:** `firmware/validate_dependencies.sh`

**Purpose:** Offline library compatibility checker

**Usage:**
```bash
./validate_dependencies.sh [OPTIONS]

Options:
  --fix, -f         Attempt to fix missing dependencies
  --cache-check, -c Check cache integrity and completeness
  --verbose, -v     Enable verbose output
  --help, -h        Show help message
```

**Examples:**
```bash
# Check all dependencies
./validate_dependencies.sh --verbose

# Check and attempt to fix issues
./validate_dependencies.sh --fix --verbose

# Verify cache integrity
./validate_dependencies.sh --cache-check --verbose

# Full dependency audit
./validate_dependencies.sh --fix --cache-check --verbose
```

**What it verifies:**
- All required libraries are cached locally
- Library version compatibility
- Include file availability
- Compilation dependencies
- Framework compatibility
- Platform toolchain availability

### 4. GPIO Configuration Validator

**Location:** `firmware/validate_gpio_config.sh`

**Purpose:** Pin assignment verification tool

**Usage:**
```bash
./validate_gpio_config.sh [OPTIONS]

Options:
  --board, -b       ESP32 board type (esp32|esp32s2|esp32s3|esp32c3)
  --fix, -f         Attempt to fix GPIO conflicts
  --verbose, -v     Enable verbose output
  --help, -h        Show help message
```

**Examples:**
```bash
# Validate GPIO configuration for ESP32-S3
./validate_gpio_config.sh --board esp32s3 --verbose

# Check and attempt to fix conflicts
./validate_gpio_config.sh --board esp32s3 --fix --verbose

# Validate for standard ESP32
./validate_gpio_config.sh --board esp32 --verbose

# Generate GPIO usage report
./validate_gpio_config.sh --board esp32s3 --verbose
```

**What it validates:**
- GPIO pin conflicts and overlaps
- Camera pin assignments
- I2C/SPI pin configurations
- Power management pins
- LoRa module pin assignments
- ESP32 hardware constraints
- Board-specific limitations

### 5. Enhanced Main Validation Script

**Location:** `firmware/enhanced_offline_validate.sh`

**Purpose:** Comprehensive validation orchestrator

**Usage:**
```bash
./enhanced_offline_validate.sh [OPTIONS]

Options:
  --fix, -f       Attempt to fix issues automatically
  --full          Run comprehensive validation (all components)
  --report, -r    Generate detailed validation report
  --verbose, -v   Enable verbose output
  --help, -h      Show help message
```

**Examples:**
```bash
# Run standard validation suite
./enhanced_offline_validate.sh --verbose

# Run full validation with automatic fixes
./enhanced_offline_validate.sh --full --fix --report --verbose

# Generate comprehensive report
./enhanced_offline_validate.sh --report --verbose

# Quick validation check
./enhanced_offline_validate.sh
```

**What it orchestrates:**
- All validation scripts in sequence
- Comprehensive test reporting
- Error logging and diagnosis
- Performance metrics
- Automated fix suggestions

## Validation Workflows

### Basic Offline Validation

```bash
# 1. Check project structure and dependencies
./enhanced_offline_validate.sh --verbose

# 2. Validate GPIO configuration
./validate_gpio_config.sh --board esp32s3 --verbose

# 3. Test hardware simulation
./validate_esp32_hardware.sh --simulation --verbose

# 4. Check firmware size constraints
./validate_firmware_size.sh --verbose
```

### Comprehensive Validation

```bash
# 1. Run enhanced validation with all options
./enhanced_offline_validate.sh --full --fix --report --verbose

# 2. Review generated reports
cat enhanced_validation_report.md
cat validation_fix_suggestions.txt
cat gpio_validation_report.txt

# 3. Address any issues found
# Follow suggestions in validation_fix_suggestions.txt
```

### Hardware-Specific Validation

```bash
# Test all hardware components individually
./validate_esp32_hardware.sh --component gpio --simulation --verbose
./validate_esp32_hardware.sh --component camera --simulation --verbose
./validate_esp32_hardware.sh --component i2c --simulation --verbose
./validate_esp32_hardware.sh --component spi --simulation --verbose
./validate_esp32_hardware.sh --component sd --simulation --verbose
./validate_esp32_hardware.sh --component psram --simulation --verbose
./validate_esp32_hardware.sh --component flash --simulation --verbose
./validate_esp32_hardware.sh --component power --simulation --verbose
```

### Dependency Management

```bash
# Check dependency status
./validate_dependencies.sh --cache-check --verbose

# Fix missing dependencies
./validate_dependencies.sh --fix --verbose

# Verify all includes are available
./validate_dependencies.sh --verbose
```

### Firmware Build Validation

```bash
# Check firmware size before building
./validate_firmware_size.sh --verbose

# Build and validate firmware size
./validate_firmware_size.sh --build --verbose

# Check OTA compatibility
./validate_firmware_size.sh --build --environment esp32-s3-devkitc-1 --verbose
```

## Troubleshooting

### Common Issues and Solutions

#### GPIO Conflicts
```bash
# Problem: GPIO pin conflicts detected
# Solution: Review and resolve pin assignments
./validate_gpio_config.sh --board esp32s3 --verbose
# Edit firmware/src/config.h to resolve conflicts
```

#### Missing Dependencies
```bash
# Problem: Libraries not cached offline
# Solution: Install and cache dependencies
./validate_dependencies.sh --fix --verbose
cd firmware && pio lib install
```

#### Hardware Test Failures
```bash
# Problem: Hardware tests fail in simulation
# Solution: Run in simulation mode or with actual hardware
./validate_esp32_hardware.sh --simulation --verbose
# Review pin configurations and constraints
```

#### Firmware Size Issues
```bash
# Problem: Firmware too large for flash
# Solution: Optimize code or adjust partition table
./validate_firmware_size.sh --build --verbose
# Consider reducing features or using external storage
```

### Emergency Recovery Procedures

#### Complete Environment Reset
```bash
# 1. Clean all build artifacts
cd firmware
pio run --target clean

# 2. Reset PlatformIO environment
rm -rf .pio/

# 3. Re-run dependency validation
./validate_dependencies.sh --fix --cache-check --verbose

# 4. Validate complete setup
./enhanced_offline_validate.sh --full --verbose
```

#### Cache Corruption Recovery
```bash
# 1. Check cache integrity
./validate_dependencies.sh --cache-check --verbose

# 2. Clear corrupted cache
rm -rf ~/.platformio/lib_tmp/
rm -rf ~/.platformio/.cache/

# 3. Reinstall dependencies
./validate_dependencies.sh --fix --verbose
```

## Integration with CI/CD

### GitHub Actions Integration

```yaml
name: Offline Validation
on: [push, pull_request]

jobs:
  validate:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.x'
          
      - name: Install PlatformIO
        run: pip install platformio
        
      - name: Cache PlatformIO
        uses: actions/cache@v3
        with:
          path: ~/.platformio
          key: platformio-${{ hashFiles('firmware/platformio.ini') }}
          
      - name: Run Enhanced Validation
        run: |
          cd firmware
          ./enhanced_offline_validate.sh --full --report --verbose
          
      - name: Upload Validation Report
        uses: actions/upload-artifact@v3
        with:
          name: validation-report
          path: enhanced_validation_report.md
```

### Local Development Integration

```bash
# Add to .bashrc or .zshrc for quick access
alias esp32-validate='cd ~/ESP32WildlifeCAM/firmware && ./enhanced_offline_validate.sh --verbose'
alias esp32-gpio='cd ~/ESP32WildlifeCAM/firmware && ./validate_gpio_config.sh --board esp32s3 --verbose'
alias esp32-deps='cd ~/ESP32WildlifeCAM/firmware && ./validate_dependencies.sh --cache-check --verbose'
alias esp32-hw='cd ~/ESP32WildlifeCAM/firmware && ./validate_esp32_hardware.sh --simulation --verbose'
```

## Performance Benchmarks

### Expected Execution Times

| Validation Type | Time (Typical) | Time (Full) |
|----------------|----------------|-------------|
| GPIO Config    | 5-10 seconds   | 15-20 seconds |
| Dependencies   | 10-15 seconds  | 30-45 seconds |
| Hardware       | 15-20 seconds  | 60-90 seconds |
| Firmware Size  | 5-10 seconds   | 20-30 seconds |
| Enhanced Full  | 45-60 seconds  | 120-180 seconds |

### Optimization Tips

```bash
# Use specific component tests for faster iteration
./validate_esp32_hardware.sh --component gpio --simulation

# Cache validation results for repeated runs
./enhanced_offline_validate.sh --report

# Use verbose mode only when debugging
./validate_dependencies.sh --cache-check
```

## Advanced Usage

### Custom Validation Scenarios

```bash
# Wildlife camera specific validation
./validate_esp32_hardware.sh --component camera --simulation --verbose
./validate_gpio_config.sh --board esp32s3 --verbose | grep -E "(camera|pir|lora)"

# Power-focused validation
./validate_esp32_hardware.sh --component power --simulation --verbose
./validate_gpio_config.sh --board esp32s3 --verbose | grep -E "(battery|solar|charging)"

# Connectivity validation
./validate_esp32_hardware.sh --component i2c --simulation --verbose
./validate_esp32_hardware.sh --component spi --simulation --verbose
```

### Batch Validation

```bash
#!/bin/bash
# batch_validate.sh - Run all validations and collect results

echo "Starting batch validation..."

# Run all validation scripts
./validate_esp32_hardware.sh --simulation > hw_results.log 2>&1
./validate_gpio_config.sh --board esp32s3 > gpio_results.log 2>&1
./validate_firmware_size.sh > size_results.log 2>&1
./validate_dependencies.sh --cache-check > deps_results.log 2>&1

# Generate summary
echo "Batch validation complete. Results:"
echo "Hardware: $(if grep -q "✅" hw_results.log; then echo "PASS"; else echo "FAIL"; fi)"
echo "GPIO: $(if grep -q "✅" gpio_results.log; then echo "PASS"; else echo "FAIL"; fi)"
echo "Size: $(if grep -q "✅" size_results.log; then echo "PASS"; else echo "FAIL"; fi)"
echo "Dependencies: $(if grep -q "✅" deps_results.log; then echo "PASS"; else echo "FAIL"; fi)"
```

---

*This documentation ensures complete offline ESP32 development capability with zero network dependencies, comprehensive hardware testing coverage, and clear workflows for restricted environments.*