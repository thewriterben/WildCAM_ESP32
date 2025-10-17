# Test Quick Reference Guide

## Quick Start

### Run All Tests
```bash
cd ESP32WildlifeCAM-main
pio test -e native
```

### Run Specific Test
```bash
pio test -e native -f test_camera_manager
pio test -e native -f test_power_manager
```

### Run Tests with Verbose Output
```bash
pio test -e native -v
```

## Test Organization

```
test/
├── unit/                 # Independent module tests
├── integration/          # Workflow and system tests
└── mocks/               # Hardware simulation
```

## Available Tests

### Unit Tests
- `test_camera_manager` - Camera operations (6 tests)
- `test_power_manager` - Power and battery (10 tests)
- `test_time_manager` - Time handling (9 tests)
- `test_storage_manager` - SD card ops (10 tests)
- `test_environmental_sensors` - Sensors (12 tests)
- `test_gps_manager` - GPS operations (12 tests)

### Integration Tests
- `test_detection_pipeline` - Complete detection (5 tests)
- `test_network_management` - WiFi and network (7 tests)
- `test_power_save_mode` - Sleep modes (8 tests)

## Common Commands

```bash
# List all tests
pio test --list-tests

# Run without uploading (native only)
pio test -e native --without-uploading

# Run single test file
pio test -e native -f test_camera_manager

# Clean test builds
pio test -e native -c

# Generate coverage report (requires lcov)
./scripts/generate_coverage.sh
```

## Writing Tests

### Basic Test Structure
```cpp
#include <unity.h>

void test_my_feature() {
    // Arrange
    int input = 42;
    
    // Act
    int result = myFunction(input);
    
    // Assert
    TEST_ASSERT_EQUAL_INT(42, result);
}

void setUp(void) {}
void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_my_feature);
    return UNITY_END();
}
```

### Using Mocks
```cpp
#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
#include "../mocks/mock_camera.h"
MockSerial Serial;
#endif
```

## Common Assertions

```cpp
TEST_ASSERT_TRUE(condition)
TEST_ASSERT_FALSE(condition)
TEST_ASSERT_EQUAL_INT(expected, actual)
TEST_ASSERT_EQUAL_FLOAT(expected, actual)
TEST_ASSERT_NOT_NULL(pointer)
TEST_ASSERT_NULL(pointer)
TEST_ASSERT_GREATER_THAN(threshold, actual)
TEST_ASSERT_LESS_THAN(threshold, actual)
TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)
```

## Debugging Failed Tests

1. **Check test output**: Look for specific assertion failures
2. **Run with verbose**: Use `-v` flag for detailed output
3. **Isolate test**: Run single test file with `-f` flag
4. **Check setUp/tearDown**: Ensure clean state between tests
5. **Verify mocks**: Check mock data matches expectations

## CI/CD Integration

Tests run automatically on:
- Push to main/develop
- Pull requests
- Manual trigger

View results in:
- Actions tab on GitHub
- PR checks
- Workflow logs

## Coverage Reports

Generate locally:
```bash
./scripts/generate_coverage.sh
```

View report:
```bash
open coverage/html/index.html
```

## Hardware Testing

For hardware-in-the-loop tests:
```bash
# Connect ESP32-CAM via USB
pio test -e test_esp32cam --upload-port /dev/ttyUSB0

# List available ports
pio device list
```

## Troubleshooting

### "Platform not installed"
```bash
pio platform install native
```

### "Test not found"
- Check test file is in `test/unit/` or `test/integration/`
- Verify filename matches pattern: `test_*.cpp`

### Compilation errors
- Ensure mocks are included for NATIVE_TEST
- Check include paths in platformio.ini

### Tests pass locally but fail in CI
- Check for platform-specific code
- Verify all dependencies in platformio.ini
- Review CI logs for specific errors

## Best Practices

1. ✅ Write tests for new features
2. ✅ Keep tests independent
3. ✅ Use descriptive test names
4. ✅ Test edge cases
5. ✅ Mock hardware dependencies
6. ✅ Clean up in tearDown()
7. ✅ One assertion per test (when possible)
8. ✅ Use meaningful assertion messages

## Resources

- [Unity Testing Framework](https://github.com/ThrowTheSwitch/Unity)
- [PlatformIO Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html)
- [Test Documentation](test/README.md)
