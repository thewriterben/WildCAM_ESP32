# ESP32 Wildlife CAM - Hardware Test Suite

This directory contains hardware-specific test cases for validating ESP32 Wildlife CAM components and assemblies.

## Test Categories

### Component Tests
- **Power System**: Battery, charging, solar panel validation
- **Camera System**: Image capture, quality, configuration
- **Sensor System**: PIR, environmental sensors, calibration
- **Storage System**: SD card, file operations, integrity
- **Communication**: WiFi, LoRa, data transmission

### Integration Tests
- **System Boot**: Initialization sequence validation
- **Component Interaction**: Cross-component functionality
- **Error Handling**: Fault tolerance and recovery
- **Performance**: Resource utilization and optimization

## Test Execution

```bash
# Run all hardware tests
./scripts/hardware_test.sh --component all

# Run specific component tests
./scripts/hardware_test.sh --component camera
./scripts/hardware_test.sh --component power
./scripts/hardware_test.sh --component sensors

# Run with simulation for offline testing
./scripts/hardware_test.sh --component all --simulation
```

## Test Results

Test results are stored in `test_results/hardware_tests/` with timestamped directories for each test run.

## Hardware Requirements

- ESP32-CAM development board
- Complete sensor suite (PIR, BME280, etc.)
- Power system (battery, charging, solar panel)
- SD card for storage testing
- USB connection for programming and monitoring

## Safety Notes

- Follow ESD precautions when handling components
- Verify power supply voltages before connecting
- Use appropriate current limiting for component protection
- Monitor temperature during extended testing