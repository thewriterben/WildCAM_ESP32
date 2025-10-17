# Testing Infrastructure

## Overview
The WildCAM ESP32 project now includes a comprehensive test suite with 79 individual test cases covering all core modules and major system workflows.

## Quick Start
```bash
# Run all tests
cd ESP32WildlifeCAM-main
pio test -e native

# Run specific test module
pio test -e native -f test_camera_manager
```

## Test Suite Components

### Unit Tests (59 tests)
- **Camera Manager** - Image capture and configuration
- **Power Manager** - Battery and power state management  
- **Time Manager** - Timestamp and RTC operations
- **Storage Manager** - SD card and file operations
- **Environmental Sensors** - Temperature, humidity, pressure
- **GPS Manager** - Location tracking and calculations

### Integration Tests (20 tests)
- **Detection Pipeline** - Complete capture-to-detection workflow
- **Network Management** - WiFi connectivity and data transmission
- **Power Save Mode** - Sleep modes and wake-up functionality

### Mock Objects
Hardware abstraction for native testing:
- Arduino framework mocks
- ESP32 camera interface mocks
- SD card filesystem mocks

## Documentation
- 📖 [Comprehensive Test Guide](test/README.md)
- ⚡ [Quick Reference](test/QUICK_REFERENCE.md)
- 📊 [Implementation Summary](test/TEST_IMPLEMENTATION_SUMMARY.md)

## CI/CD Integration
Tests run automatically on:
- Every push to main/develop branches
- All pull requests
- Manual workflow triggers

View test results in the [Actions](../../actions) tab.

## Development Workflow

### Adding New Tests
1. Create test file in `test/unit/` or `test/integration/`
2. Use Unity framework assertions
3. Include mocks for native testing
4. Run locally before pushing

### Running Tests Locally
```bash
# All tests
pio test -e native

# Specific test
pio test -e native -f test_power_manager

# With verbose output
pio test -e native -v

# Generate coverage
./scripts/generate_coverage.sh
```

## Test Statistics
- **Test Files**: 9 (6 unit + 3 integration)
- **Test Cases**: 79 individual tests
- **Core Modules**: 100% covered
- **Mock Objects**: 3 hardware abstractions
- **Documentation**: Complete with examples

## Benefits
- ✅ Early bug detection
- ✅ Regression prevention
- ✅ Safe refactoring
- ✅ Automated quality checks
- ✅ Clear module specifications

## Next Steps
- Monitor CI/CD test results
- Expand hardware-in-the-loop tests
- Add performance benchmarks
- Implement coverage tracking
- Add mutation testing

---
For detailed information, see the [test directory](test/).
