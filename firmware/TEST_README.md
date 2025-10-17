# WildCAM ESP32 Testing Guide

## Overview

This document describes the comprehensive testing framework for the WildCAM ESP32 wildlife monitoring platform. The testing infrastructure uses PlatformIO's Unity test framework and is integrated into our CI/CD pipeline.

## Test Structure

### Test Organization

All tests are located in the `firmware/test/` directory:

```
firmware/test/
├── test_camera_capture.cpp          # Camera capture functionality
├── test_camera_module.cpp           # Camera module operations
├── test_connectivity_orchestrator.cpp # Network orchestration
├── test_detection_pipeline.cpp      # Wildlife detection pipeline
├── test_diagnostics.cpp             # System diagnostics
├── test_environmental_integration.cpp # Environmental sensors
├── test_error_handling.cpp          # Error handling mechanisms
├── test_gps_config.cpp              # GPS configuration
├── test_gps_manager.cpp             # GPS manager
├── test_jpeg_decoder.cpp            # JPEG decoding
├── test_lora_encryption.cpp         # LoRa encryption
├── test_memory_management.cpp       # Memory management
├── test_ota_update.cpp              # OTA update functionality
├── test_power_management.cpp        # Power management
├── test_security_manager.cpp        # Security features
├── test_storage_manager.cpp         # Storage operations
└── test_time_manager.cpp            # Time management
```

## Running Tests Locally

### Prerequisites

1. Install PlatformIO Core:
   ```bash
   pip install platformio
   ```

2. Navigate to the firmware directory:
   ```bash
   cd firmware
   ```

### Running All Tests

Run all tests in the test environment:

```bash
pio test -e test
```

### Running Specific Tests

Run a specific test file:

```bash
pio test -e test -f test_storage_manager
```

Run with verbose output:

```bash
pio test -e test --verbose
```

### Test Environment Configuration

The test environment is configured in `platformio.ini`:

```ini
[env:test]
platform = espressif32@6.4.0
board = esp32cam
framework = arduino
test_framework = unity
test_build_src = yes
build_flags = 
    ${env.build_flags}
    -DUNIT_TEST=1
```

## Test Categories

### 1. Unit Tests (50+ tests)

Unit tests verify individual components in isolation:

- **Camera Module** (test_camera_module.cpp)
  - Initialization and configuration
  - Image capture
  - Error handling
  - Resource cleanup

- **Power Management** (test_power_management.cpp)
  - Battery voltage monitoring
  - Solar charging
  - Power state transitions
  - Low power modes

- **Storage Manager** (test_storage_manager.cpp)
  - File operations
  - SD card and LittleFS support
  - Space management
  - Error handling

- **GPS Manager** (test_gps_manager.cpp)
  - GPS initialization
  - Location data parsing
  - Fix acquisition

- **Time Manager** (test_time_manager.cpp)
  - Time synchronization
  - RTC operations
  - NTP updates
  - Timezone handling

- **Environmental Sensors** (test_environmental_integration.cpp)
  - BME280 temperature/humidity/pressure
  - SGP30 air quality
  - Sensor data validation

### 2. Integration Tests (20+ tests)

Integration tests verify component interactions:

- **Detection Pipeline** (test_detection_pipeline.cpp)
  - End-to-end wildlife detection workflow
  - Image capture → AI processing → storage
  - Metadata generation

- **Network Management** (test_connectivity_orchestrator.cpp)
  - WiFi/cellular failover
  - Connection quality monitoring
  - Data transmission

- **Security System** (test_security_manager.cpp)
  - Encryption/decryption
  - Key management
  - Tamper detection

- **OTA Updates** (test_ota_update.cpp)
  - Update checking
  - Download and verification
  - Rollback capability

### 3. Hardware Tests

Hardware tests require physical ESP32 hardware:

- **Camera Capture** (test_camera_capture.cpp)
  - Physical camera operations
  - Image quality validation
  - Flash operation

- **SD Card Operations**
  - Read/write performance
  - Filesystem integrity
  - Space management

- **Sensor Readings**
  - Environmental sensor accuracy
  - GPS fix acquisition
  - Power measurements

## CI/CD Integration

### GitHub Actions Workflow

Tests are automatically run on every push and pull request via `.github/workflows/ci.yml`:

```yaml
run-tests:
  runs-on: ubuntu-latest
  name: Run Unit Tests
  steps:
    - uses: actions/checkout@v4
    - name: Install PlatformIO
      run: pip install platformio
    - name: Run firmware tests
      run: |
        cd firmware
        pio test -e test --verbose
    - name: Upload test results
      uses: actions/upload-artifact@v3
      with:
        name: test-results
        path: firmware/.pio/build/test/
```

### Test Execution Flow

1. **Validate Project** - Check project structure
2. **Lint Code** - Run static analysis
3. **Run Tests** - Execute unit and integration tests
4. **Build Firmware** - Build for all target environments
5. **Generate Reports** - Create test coverage reports

## Writing New Tests

### Test Template

Use this template for new test files:

```cpp
/**
 * @file test_your_module.cpp
 * @brief Unit tests for YourModule
 * @author ESP32 Wildlife CAM Project
 * @date 2025-10-16
 */

#include <unity.h>
#include "../src/path/to/your_module.h"

YourModule* module = nullptr;

void setUp(void) {
    // Initialize before each test
    module = new YourModule();
}

void tearDown(void) {
    // Cleanup after each test
    if (module) {
        delete module;
        module = nullptr;
    }
}

/**
 * Test description
 * Expected: What should happen
 */
void test_your_feature(void) {
    // Arrange
    bool expected = true;
    
    // Act
    bool result = module->doSomething();
    
    // Assert
    TEST_ASSERT_EQUAL(expected, result);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_your_feature);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
```

### Best Practices

1. **Test One Thing** - Each test should verify a single behavior
2. **Use Descriptive Names** - Test names should clearly describe what they test
3. **AAA Pattern** - Arrange, Act, Assert structure
4. **Independent Tests** - Tests should not depend on each other
5. **Clean Up Resources** - Always clean up in tearDown()
6. **Mock Hardware** - Use mocks for hardware dependencies in unit tests
7. **Test Edge Cases** - Include null checks, boundary conditions, error cases

### Unity Assertions

Common Unity test assertions:

```cpp
// Boolean assertions
TEST_ASSERT_TRUE(condition);
TEST_ASSERT_FALSE(condition);

// Equality assertions
TEST_ASSERT_EQUAL(expected, actual);
TEST_ASSERT_EQUAL_STRING("expected", actual);
TEST_ASSERT_EQUAL_FLOAT(expected, actual);

// Null/Not null assertions
TEST_ASSERT_NULL(pointer);
TEST_ASSERT_NOT_NULL(pointer);

// Comparison assertions
TEST_ASSERT_GREATER_THAN(threshold, actual);
TEST_ASSERT_LESS_THAN(threshold, actual);

// Array assertions
TEST_ASSERT_EQUAL_MEMORY(expected, actual, length);
```

## Test Coverage Goals

### Coverage Targets

- **Core Modules**: 80%+ code coverage
- **Critical Path**: 95%+ code coverage
- **Hardware Abstraction**: 70%+ code coverage
- **Integration Tests**: All major workflows covered

### Current Coverage

Run coverage analysis:

```bash
pio test -e test --coverage
```

### Improving Coverage

1. Identify uncovered code paths
2. Write tests for edge cases
3. Add integration tests for workflows
4. Mock hardware dependencies
5. Test error handling paths

## Hardware-in-the-Loop Testing

### Manual Hardware Tests

Some tests require physical hardware and must be run manually:

#### Camera Test Procedure

1. Flash test firmware to ESP32-CAM
2. Connect serial monitor (115200 baud)
3. Verify camera initialization messages
4. Check captured image quality
5. Test night mode and flash

#### GPS Test Procedure

1. Connect GPS module to ESP32
2. Place device outdoors with clear sky view
3. Wait for GPS fix (2-10 minutes)
4. Verify location accuracy
5. Test movement tracking

#### Power Test Procedure

1. Connect battery and solar panel
2. Monitor voltage readings
3. Test charging states
4. Verify low power modes
5. Measure current consumption

### Test Rig Setup

For automated hardware testing:

1. **Test Rig Components**
   - ESP32-CAM board
   - USB programmer
   - Power supply
   - SD card reader
   - Sensor modules

2. **Automation Tools**
   - pytest for test orchestration
   - pyserial for serial communication
   - GPIO control for power cycling

## Troubleshooting

### Common Issues

#### Tests Not Running

**Problem**: `pio test` shows "SKIPPED"

**Solution**: Ensure test environment is correctly configured in `platformio.ini`

#### Compilation Errors

**Problem**: Tests fail to compile

**Solution**: Check include paths and ensure all dependencies are installed

#### Hardware-Dependent Tests Failing

**Problem**: Tests requiring hardware fail in CI

**Solution**: Add hardware mocks or skip hardware tests in CI environment

#### Memory Issues

**Problem**: Tests crash with out-of-memory errors

**Solution**: Reduce test scope or increase heap size in test configuration

### Debug Mode

Run tests with debug output:

```bash
pio test -e test --verbose -vvv
```

## Contributing

### Adding New Tests

1. Create test file in `firmware/test/`
2. Follow naming convention: `test_<module_name>.cpp`
3. Use the test template provided above
4. Add tests to cover new functionality
5. Run tests locally before committing
6. Update this documentation

### Test Review Checklist

- [ ] Tests follow naming conventions
- [ ] All tests have descriptive names
- [ ] Tests use AAA pattern
- [ ] Edge cases are covered
- [ ] Resources are cleaned up
- [ ] Tests pass locally
- [ ] Documentation updated

## Resources

- [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)
- [PlatformIO Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html)
- [ESP32 Testing Best Practices](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/unit-tests.html)

## Support

For questions or issues:
- Open an issue on GitHub
- Check existing test examples
- Review PlatformIO documentation
- Contact the development team

---

**Last Updated**: 2025-10-16
**Version**: 1.0.0
