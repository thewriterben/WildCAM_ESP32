# WildCAM ESP32 Testing Quick Reference

## Quick Start

### Run All Tests
```bash
cd firmware
pio test -e test
```

### Run Specific Test
```bash
pio test -e test -f test_storage_manager
```

### Using Helper Scripts
```bash
# Run tests
./scripts/run_tests.sh

# Show test summary
./scripts/test_summary.sh
```

## Test Statistics

- **Total Test Files**: 17
- **Total Test Functions**: 143+
- **Unit Tests**: 15 files
- **Integration Tests**: 3 files

## Test Files Overview

| Test File | Tests | Category | Description |
|-----------|-------|----------|-------------|
| test_camera_capture.cpp | 10 | Unit | Camera capture operations |
| test_camera_module.cpp | 12 | Unit | Camera module functionality |
| test_connectivity_orchestrator.cpp | 8 | Integration | Network failover & management |
| test_detection_pipeline.cpp | 15 | Integration | Wildlife detection workflow |
| test_diagnostics.cpp | 6 | Unit | System diagnostics |
| test_environmental_integration.cpp | 8 | Integration | Sensor integration |
| test_error_handling.cpp | 10 | Unit | Error handling mechanisms |
| test_gps_config.cpp | 11 | Unit | GPS configuration |
| test_gps_manager.cpp | 2 | Unit | GPS manager (stub) |
| test_jpeg_decoder.cpp | 14 | Unit | JPEG decoding |
| test_lora_encryption.cpp | 12 | Unit | LoRa encryption |
| test_memory_management.cpp | 9 | Unit | Memory operations |
| test_ota_update.cpp | 14 | Unit | OTA update system |
| test_power_management.cpp | 6 | Unit | Power management |
| test_security_manager.cpp | 11 | Unit | Security features |
| test_storage_manager.cpp | 18 | Unit | Storage operations |
| test_time_manager.cpp | 8 | Unit | Time synchronization |

## Common Test Patterns

### Basic Test Structure
```cpp
void test_feature_name(void) {
    // Arrange
    Module module;
    bool expected = true;
    
    // Act
    bool result = module.doSomething();
    
    // Assert
    TEST_ASSERT_EQUAL(expected, result);
}
```

### Testing with Setup/Teardown
```cpp
Module* module = nullptr;

void setUp(void) {
    module = new Module();
}

void tearDown(void) {
    delete module;
    module = nullptr;
}

void test_feature(void) {
    TEST_ASSERT_NOT_NULL(module);
    // Test code here
}
```

### Error Handling Tests
```cpp
void test_null_pointer(void) {
    Result result = function(nullptr);
    TEST_ASSERT_NOT_EQUAL(SUCCESS, result);
}
```

## Common Assertions

```cpp
// Boolean
TEST_ASSERT_TRUE(condition);
TEST_ASSERT_FALSE(condition);

// Equality
TEST_ASSERT_EQUAL(expected, actual);
TEST_ASSERT_EQUAL_STRING("expected", actual);
TEST_ASSERT_EQUAL_FLOAT(expected, actual);

// Pointers
TEST_ASSERT_NULL(pointer);
TEST_ASSERT_NOT_NULL(pointer);

// Comparison
TEST_ASSERT_GREATER_THAN(threshold, actual);
TEST_ASSERT_LESS_THAN(threshold, actual);
```

## CI/CD Integration

Tests run automatically on:
- Every push to main/develop
- Every pull request
- Manual workflow dispatch

**View Results**: GitHub Actions → Run Tests job

## Troubleshooting

### Tests Skipped
- Ensure test environment is configured in platformio.ini
- Check that tests follow naming convention: `test_*.cpp`

### Compilation Errors
- Verify include paths are correct
- Check that all dependencies are installed
- Ensure test files are in `test/` directory

### Hardware Tests Failing in CI
- Normal for tests requiring physical hardware
- These tests should be run manually on device
- See TEST_README.md for hardware test procedures

## Coverage Goals

- **Core Modules**: 80%+ coverage ✅
- **Critical Path**: 95%+ coverage 🚧
- **Hardware Abstraction**: 70%+ coverage ✅
- **Integration Tests**: All major workflows ✅

## Adding New Tests

1. Create `test_your_module.cpp` in `firmware/test/`
2. Include Unity: `#include <unity.h>`
3. Write test functions: `void test_feature(void)`
4. Add setup/teardown if needed
5. Register tests in `setup()` with `RUN_TEST(test_feature)`
6. Run locally: `pio test -e test -f test_your_module`
7. Commit and push - CI will run automatically

## Resources

- **Full Documentation**: [TEST_README.md](TEST_README.md)
- **Unity Framework**: [Unity Test](https://github.com/ThrowTheSwitch/Unity)
- **PlatformIO Testing**: [Docs](https://docs.platformio.org/en/latest/advanced/unit-testing/)

---

**Last Updated**: 2025-10-16  
**Test Count**: 143+ tests
