# PlatformIO Configuration for Restricted Environments

*Last Updated: September 1, 2025*  
*Compatibility: PlatformIO Core 6.0+*  
*Network Requirements: None (offline operation)*

## Overview

This document provides complete PlatformIO configuration for developing ESP32 Wildlife Camera firmware in environments with firewall restrictions that block access to standard package repositories.

## Core Configuration Files

### 1. Global PlatformIO Configuration

#### ~/.platformio/pioplus.ini
```ini
[platformio]
# Core settings for offline operation
core_dir = ~/.platformio
home_dir = ~/.platformio
lib_dir = ~/.platformio/lib
workspace_dir = ~/.platformio/workspace

# Disable all online features
[settings]
enable_telemetry = false
check_libraries_interval = 0
check_platformio_interval = 0
check_prune_system_threshold = 0

# Force offline mode
[offline]
enabled = true
cache_dir = ~/.platformio/cache
update_checks = false
telemetry = false
```

### 2. Project-Specific Configuration

#### platformio.ini (Wildlife Camera Project)
```ini
[platformio]
default_envs = esp32cam
core_dir = ~/.platformio
lib_extra_dirs = ./lib

# Global settings for offline development
[env]
platform_packages = 
    toolchain-xtensa-esp32@file://~/.platformio/packages/toolchain-xtensa-esp32
    framework-arduinoespressif32@file://~/.platformio/packages/framework-arduinoespressif32

[env:esp32cam]
platform = espressif32@6.4.0
board = esp32cam
framework = arduino

# Offline-specific build flags
build_flags = 
    -DESP32_VALIDATION_BUILD=1
    -DOFFLINE_MODE=1
    -DCORE_DEBUG_LEVEL=0
    -DARDUINO_RUNNING_CORE=1
    -DARDUINO_EVENT_RUNNING_CORE=1
    -DWIFI_TASK_STACK_SIZE=4096
    -DCAMERA_MODEL_AI_THINKER

# Local library dependencies (cached offline)
lib_deps = 
    # Core ESP32 libraries (framework-included)
    WiFi
    FS
    SD
    SPI
    Wire
    EEPROM
    
    # External libraries (local cache)
    file://~/.platformio/lib/ArduinoJson
    file://~/.platformio/lib/AsyncTCP
    file://~/.platformio/lib/ESPAsyncWebServer

# Build optimization for restricted environments
build_type = release
build_unflags = -Os
build_flags = ${env.build_flags} -O2

# Monitor settings
monitor_speed = 115200
monitor_filters = 
    esp32_exception_decoder
    time

# Upload settings
upload_speed = 921600
upload_resetmethod = nodemcu

# Partition scheme for wildlife camera
board_build.partitions = partitions.csv
board_build.filesystem = spiffs
```

### 3. Advanced ESP32-S3 Configuration

#### platformio.ini (ESP32-S3-CAM)
```ini
[env:esp32s3cam]
platform = espressif32@6.4.0
board = esp32-s3-devkitc-1
framework = arduino

# ESP32-S3 specific settings
board_build.mcu = esp32s3
board_build.f_cpu = 240000000L
board_build.f_flash = 80000000L
board_build.flash_mode = qio
board_build.psram_type = qspi

# Enhanced build flags for AI capabilities
build_flags = 
    ${env.build_flags}
    -DARDUINO_ESP32S3_DEV
    -DBOARD_HAS_PSRAM
    -DESP32S3_CAM_BUILD
    -DTENSORFLOW_LITE_MICRO
    -DARDUINO_USB_MODE=1

# AI-specific library dependencies
lib_deps = 
    ${env.lib_deps}
    file://~/.platformio/lib/TensorFlowLiteMicro
    file://~/.platformio/lib/ESP32-Camera

# Memory configuration for AI processing
board_build.arduino.memory_type = qspi_qspi
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
```

## Offline Package Management

### Platform Package Configuration

#### 1. ESP32 Platform Setup
```bash
# Manual platform installation structure
~/.platformio/platforms/espressif32/
├── platform.json
├── boards/
│   ├── esp32cam.json
│   ├── esp32-s3-devkitc-1.json
│   └── ...
├── builder/
│   └── main.py
└── packages/
    ├── toolchain-xtensa-esp32/
    ├── framework-arduinoespressif32/
    └── tool-esptoolpy/
```

#### 2. Platform Manifest (platform.json)
```json
{
  "name": "espressif32",
  "title": "Espressif 32",
  "description": "ESP32 is a series of low-cost, low-power system on a chip microcontrollers",
  "version": "6.4.0",
  "homepage": "https://github.com/platformio/platform-espressif32",
  "license": "Apache-2.0",
  "engines": {
    "platformio": "^6.0.0"
  },
  "repository": {
    "type": "git",
    "url": "https://github.com/platformio/platform-espressif32.git"
  },
  "packages": {
    "toolchain-xtensa-esp32": {
      "type": "toolchain",
      "version": "11.2.0+2022r1"
    },
    "framework-arduinoespressif32": {
      "type": "framework",
      "version": "~3.20014.0"
    },
    "tool-esptoolpy": {
      "type": "uploader",
      "version": "~1.40501.0"
    }
  }
}
```

### Library Configuration

#### 1. ArduinoJson Library (local cache)
```
~/.platformio/lib/ArduinoJson/
├── library.json
├── src/
│   ├── ArduinoJson.h
│   └── ArduinoJson/
└── extras/
```

#### 2. Library Manifest (library.json)
```json
{
  "name": "ArduinoJson",
  "version": "6.21.3",
  "description": "An efficient and elegant JSON library for embedded C++",
  "homepage": "https://arduinojson.org",
  "repository": {
    "type": "git",
    "url": "https://github.com/bblanchon/ArduinoJson.git"
  },
  "authors": [
    {
      "name": "Benoit Blanchon",
      "email": "blog@benoitblanchon.fr",
      "maintainer": true
    }
  ],
  "license": "MIT",
  "frameworks": ["arduino", "espidf"],
  "platforms": ["espressif32"],
  "export": {
    "include": "src"
  }
}
```

## Environment-Specific Configurations

### Development Environment Variables

#### 1. Bash Profile Setup (.bashrc/.zshrc)
```bash
# PlatformIO offline configuration
export PLATFORMIO_CORE_DIR="$HOME/.platformio"
export PLATFORMIO_HOME_DIR="$HOME/.platformio"
export PLATFORMIO_LIB_DIR="$HOME/.platformio/lib"
export PLATFORMIO_CACHE_DIR="$HOME/.platformio/cache"

# Disable online features
export PLATFORMIO_CHECK_LIBRARIES_INTERVAL=0
export PLATFORMIO_CHECK_PLATFORMIO_INTERVAL=0
export PLATFORMIO_ENABLE_TELEMETRY=false

# Force offline mode
export PLATFORMIO_OFFLINE_MODE=true
```

#### 2. Windows Environment Setup
```cmd
# Windows Command Prompt / PowerShell
set PLATFORMIO_CORE_DIR=%USERPROFILE%\.platformio
set PLATFORMIO_HOME_DIR=%USERPROFILE%\.platformio
set PLATFORMIO_OFFLINE_MODE=true
set PLATFORMIO_ENABLE_TELEMETRY=false
```

### IDE-Specific Configurations

#### 1. Visual Studio Code Settings (settings.json)
```json
{
  "platformio-ide.autoRebuildAutocompleteIndex": false,
  "platformio-ide.useBuiltinPIOCore": true,
  "platformio-ide.disablePIORemoteAgent": true,
  "platformio-ide.disablePIOPlusMode": true,
  "platformio-ide.customPATH": "~/.platformio/penv/bin",
  
  "C_Cpp.intelliSenseEngine": "Default",
  "C_Cpp.default.compilerPath": "~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-gcc",
  "C_Cpp.default.includePath": [
    "~/.platformio/packages/framework-arduinoespressif32/cores/esp32",
    "~/.platformio/packages/framework-arduinoespressif32/libraries",
    "${workspaceFolder}/src",
    "${workspaceFolder}/lib",
    "${workspaceFolder}/include"
  ]
}
```

#### 2. CLion Configuration
```cmake
# CMakeLists.txt for CLion integration
cmake_minimum_required(VERSION 3.16)
project(ESP32WildlifeCAM)

# PlatformIO integration
set(PLATFORMIO_CMD "pio")
set(PLATFORMIO_ENV "esp32cam")

# Include paths for offline development
include_directories(
    "$ENV{HOME}/.platformio/packages/framework-arduinoespressif32/cores/esp32"
    "$ENV{HOME}/.platformio/packages/framework-arduinoespressif32/libraries"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib"
)

# Custom build targets
add_custom_target(build_firmware
    COMMAND ${PLATFORMIO_CMD} run --environment ${PLATFORMIO_ENV}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(upload_firmware
    COMMAND ${PLATFORMIO_CMD} run --environment ${PLATFORMIO_ENV} --target upload
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
```

## Validation and Testing Configuration

### Offline Build Validation

#### 1. Test Project Structure
```
test_offline_build/
├── platformio.ini
├── src/
│   └── main.cpp
├── lib/
├── test/
│   └── test_basic/
│       └── test_basic.cpp
└── .gitignore
```

#### 2. Minimal Test Code (main.cpp)
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <FS.h>
#include <SD.h>

#ifdef OFFLINE_MODE
#pragma message("Building in OFFLINE MODE")
#endif

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera - Offline Build Test");
    
    // Test core functionality
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("CPU frequency: %d MHz\n", getCpuFrequencyMhz());
    
    #ifdef ESP32_VALIDATION_BUILD
    Serial.println("Validation build: PASSED");
    #endif
}

void loop() {
    delay(5000);
    Serial.println("Offline build test running...");
}
```

#### 3. Unit Test Configuration (test_basic.cpp)
```cpp
#include <unity.h>
#include <Arduino.h>

void test_heap_available() {
    uint32_t free_heap = ESP.getFreeHeap();
    TEST_ASSERT_GREATER_THAN(50000, free_heap);
}

void test_cpu_frequency() {
    uint32_t cpu_freq = getCpuFrequencyMhz();
    TEST_ASSERT_TRUE(cpu_freq == 240 || cpu_freq == 160 || cpu_freq == 80);
}

void test_offline_mode() {
    #ifdef OFFLINE_MODE
    TEST_ASSERT_TRUE(true);
    #else
    TEST_FAIL_MESSAGE("OFFLINE_MODE not defined");
    #endif
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_heap_available);
    RUN_TEST(test_cpu_frequency);
    RUN_TEST(test_offline_mode);
    UNITY_END();
}

void loop() {
    // Empty
}
```

## Troubleshooting Configuration Issues

### Common Configuration Problems

#### 1. Platform Not Found
```bash
# Check platform installation
ls ~/.platformio/platforms/espressif32/

# Verify platform.json exists and is valid
cat ~/.platformio/platforms/espressif32/platform.json | jq '.'

# Reinstall platform from cache
tar -xzf platform_cache/espressif32.tar.gz -C ~/.platformio/platforms/
```

#### 2. Toolchain Issues
```bash
# Verify toolchain installation
ls ~/.platformio/packages/toolchain-xtensa-esp32/bin/

# Test toolchain functionality
~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-gcc --version

# Fix permissions if needed
chmod +x ~/.platformio/packages/toolchain-xtensa-esp32/bin/*
```

#### 3. Library Dependencies
```bash
# Check library installation
pio lib list --global

# Verify library structure
ls ~/.platformio/lib/ArduinoJson/

# Install missing libraries from cache
cp -r offline_libraries/* ~/.platformio/lib/
```

### Advanced Diagnostics

#### 1. PlatformIO Debug Mode
```bash
# Enable verbose output
pio run --environment esp32cam --verbose

# Check dependency resolution
pio lib deps --environment esp32cam --tree

# Validate configuration
pio check --environment esp32cam --fail-on-defect=high
```

#### 2. Build System Debugging
```bash
# Show build commands
pio run --environment esp32cam --verbose | grep -E "xtensa-esp32-elf"

# Check preprocessor definitions
pio run --environment esp32cam --target clean
pio run --environment esp32cam --verbose 2>&1 | grep -E "DESP32|DOFFLINE"
```

## Performance Optimization

### Build Time Optimization

#### 1. Parallel Compilation
```ini
[env:esp32cam]
# Use all available CPU cores
build_flags = ${env.build_flags} -j$(nproc)

# Enable ccache if available
platform_packages = 
    ${env.platform_packages}
    tool-ccache
```

#### 2. Incremental Builds
```ini
[env:esp32cam]
# Enable incremental linking
build_flags = ${env.build_flags} -Wl,--incremental

# Optimize build directory
build_dir = .pio/build_optimized
```

### Memory Optimization

#### 1. Flash Partitioning
```ini
[env:esp32cam]
# Custom partition table for wildlife camera
board_build.partitions = partitions_wildlife_cam.csv

# Optimize flash usage
board_build.embed_files = data/config.json
```

#### 2. PSRAM Configuration
```ini
[env:esp32s3cam]
# Enable PSRAM for large buffers
build_flags = 
    ${env.build_flags}
    -DBOARD_HAS_PSRAM
    -DARDUINO_LOOP_STACK_SIZE=8192
```

---

*This configuration guide ensures reliable ESP32 Wildlife Camera development in completely offline environments, with comprehensive troubleshooting and optimization procedures.*