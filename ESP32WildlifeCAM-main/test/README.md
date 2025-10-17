# WildCAM ESP32 Test Suite

## Overview
Comprehensive test suite for the WildCAM ESP32 wildlife camera system using PlatformIO Test framework with Unity.

## Test Structure

```
test/
├── unit/                    # Unit tests for individual modules
│   ├── test_camera_manager.cpp
│   ├── test_power_manager.cpp
│   ├── test_time_manager.cpp
│   ├── test_storage_manager.cpp
│   ├── test_environmental_sensors.cpp
│   └── test_gps_manager.cpp
├── integration/             # Integration tests for complete workflows
│   ├── test_detection_pipeline.cpp
│   ├── test_network_management.cpp
│   └── test_power_save_mode.cpp
└── mocks/                   # Mock implementations for hardware
    ├── mock_arduino.h
    ├── mock_camera.h
    └── mock_sd.h
```

## Running Tests

### Native Unit Tests (PC)
Run all unit tests on your local machine without hardware:
```bash
pio test -e native
```

Run specific test:
```bash
pio test -e native -f test_camera_manager
```

### Hardware-in-the-Loop Tests
Run tests on actual ESP32-CAM hardware:
```bash
pio test -e test_esp32cam --upload-port /dev/ttyUSB0
```

### CI/CD Tests
Tests run automatically on every commit via GitHub Actions.

## Test Categories

### Unit Tests (60+ tests)
Individual module testing with mocked dependencies:

1. **Camera Manager** (6 tests)
   - Camera initialization
   - Frame capture
   - Configuration validation
   - Multiple capture handling

2. **Power Manager** (10 tests)
   - Battery voltage reading
   - Battery percentage calculation
   - Power state transitions
   - Solar charging detection
   - Power mode management

3. **Time Manager** (9 tests)
   - Timestamp handling
   - Time formatting
   - Time arithmetic
   - Validation
   - Leap year handling

4. **Storage Manager** (10 tests)
   - SD card initialization
   - File operations
   - Directory creation
   - Storage capacity monitoring
   - Low storage detection

5. **Environmental Sensors** (12 tests)
   - Temperature readings
   - Humidity monitoring
   - Pressure sensing
   - Temperature conversion
   - Dew point calculation
   - Altitude calculation
   - Sensor validation

6. **GPS Manager** (12 tests)
   - GPS initialization
   - Coordinate validation
   - Distance calculation
   - Fix quality assessment
   - Speed and altitude readings

### Integration Tests (20+ tests)
Complete workflow testing:

1. **Detection Pipeline** (5 tests)
   - Complete detection workflow
   - Confidence thresholding
   - Multiple detection handling
   - Capture trigger logic

2. **Network Management** (7 tests)
   - WiFi connectivity
   - State transitions
   - Data transmission
   - Signal strength monitoring
   - Connection resilience

3. **Power Save Mode** (8 tests)
   - Sleep mode transitions
   - Wake-up functionality
   - Battery-based triggers
   - Sleep duration validation
   - Power state cycling

## Test Configuration

### platformio.ini Test Environments

```ini
[env:native]
platform = native
test_framework = unity
test_build_src = yes
build_flags = 
    -std=c++11
    -DUNIT_TEST
    -DNATIVE_TEST
lib_deps = 
    throwtheswitch/Unity@^2.5.2

[env:test_esp32cam]
board = esp32cam
test_framework = unity
test_build_src = yes
lib_deps = 
    throwtheswitch/Unity@^2.5.2
```

## Mock Objects

### Available Mocks
- **mock_arduino.h**: Arduino framework functions (millis, digitalWrite, etc.)
- **mock_camera.h**: ESP32 camera interface
- **mock_sd.h**: SD card file system operations

### Using Mocks in Tests

```cpp
#include <unity.h>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
#include "../mocks/mock_camera.h"
MockSerial Serial;
#endif

void test_example() {
    camera_fb_t* fb = esp_camera_fb_get();
    TEST_ASSERT_NOT_NULL(fb);
    esp_camera_fb_return(fb);
}
```

## Writing New Tests

### Unit Test Template

```cpp
#include <unity.h>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
MockSerial Serial;
#endif

void test_feature() {
    // Arrange
    int expected = 42;
    
    // Act
    int result = myFunction();
    
    // Assert
    TEST_ASSERT_EQUAL_INT(expected, result);
}

void setUp(void) {
    // Initialize before each test
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_feature);
    return UNITY_END();
}
```

## Test Assertions

Common Unity assertions:
- `TEST_ASSERT_TRUE(condition)`
- `TEST_ASSERT_FALSE(condition)`
- `TEST_ASSERT_EQUAL_INT(expected, actual)`
- `TEST_ASSERT_EQUAL_FLOAT(expected, actual)`
- `TEST_ASSERT_NOT_NULL(pointer)`
- `TEST_ASSERT_GREATER_THAN(threshold, actual)`
- `TEST_ASSERT_LESS_THAN(threshold, actual)`
- `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)`

## Hardware Testing

Some tests require actual hardware:

### Camera Tests
- Require ESP32-CAM module
- Test actual image capture
- Validate camera configuration

### Sensor Tests  
- Require BME280 sensor
- Test I2C communication
- Validate readings

### GPS Tests
- Require GPS module
- Test UART communication
- Validate fix acquisition

### SD Card Tests
- Require SD card
- Test file I/O
- Validate storage operations

## Test Coverage

Current coverage targets:
- Unit tests: 80%+ coverage of core modules
- Integration tests: All major workflows covered
- Hardware tests: Critical hardware operations validated

## CI/CD Integration

Tests run automatically on:
- Pull requests to main/develop
- Commits to main/develop
- Manual workflow dispatch

Test results are reported in:
- GitHub Actions logs
- Pull request comments
- Build status badges

## Troubleshooting

### Test Compilation Errors
1. Ensure all dependencies are installed: `pio lib install`
2. Clean build: `pio run -t clean`
3. Check mock includes are correct for native tests

### Test Failures
1. Check setUp/tearDown are properly resetting state
2. Verify mock data matches expected values
3. Review test assertions for correctness

### Hardware Test Issues
1. Verify hardware connections
2. Check USB port permissions
3. Ensure correct board selected in platformio.ini

## Best Practices

1. **Test Isolation**: Each test should be independent
2. **Clear Naming**: Use descriptive test names
3. **Single Assertion**: Test one thing per test when possible
4. **Mock External Dependencies**: Use mocks for hardware
5. **Setup/Teardown**: Always clean up after tests
6. **Meaningful Messages**: Use TEST_ASSERT_*_MESSAGE variants

## Future Enhancements

- [ ] Add test coverage reporting with gcov/lcov
- [ ] Implement performance benchmarks
- [ ] Add stress tests for memory/timing
- [ ] Create visual test reports
- [ ] Add mutation testing
- [ ] Implement fuzz testing for robustness
