# WildCAM ESP32 Integration Test

Comprehensive integration test suite for the WildCAM ESP32 wildlife camera system.

## Overview

This test script (`test_integration.py`) validates the complete functionality of the WildCAM ESP32 system through automated integration tests.

## Test Scenarios

### 1. System Initialization Test
- Connects to ESP32 via serial
- Verifies all modules initialize successfully
- Checks for "System Ready" message
- Validates initialization sequence of:
  - Power Manager
  - Camera Manager
  - Storage Manager
  - Motion Detector
  - Web Server

### 2. Motion Detection Test
- Simulates motion by triggering PIR sensor (manual action required)
- Verifies "Motion detected!" message appears
- Verifies image capture is triggered
- Validates motion detection workflow

### 3. Image Capture Test
- Triggers motion event
- Waits for capture to complete
- Verifies image file is created on SD card
- Verifies image file size is reasonable (> 10KB)

### 4. Metadata Test
- After image capture, verifies JSON metadata file exists
- Parses JSON and verifies required fields:
  - timestamp
  - battery_voltage
  - battery_percent
  - image_size
  - image_count

### 5. Web Server Test
- Connects to ESP32 WiFi (IP auto-detected or specified)
- Makes GET request to `/status` endpoint
- Verifies JSON response with system info
- Makes GET request to `/latest` endpoint
- Verifies image is returned

### 6. Power Management Test
- Monitors serial output for battery voltage readings
- Verifies deep sleep configuration
- Verifies wake-up sources are configured
- Validates power management features

## Requirements

### Hardware Requirements
- ESP32-CAM board with WildCAM firmware
- PIR motion sensor connected to GPIO 13
- SD card inserted and formatted
- USB-to-Serial adapter or direct USB connection
- WiFi access point (for web server tests)

### Software Requirements
- Python 3.7 or higher
- Required Python packages:
  - `pyserial` - For serial communication
  - `requests` - For HTTP requests to web server

Install dependencies (if needed):
```bash
pip install pyserial requests
```

## Usage

### Basic Usage

Run all tests with default settings:
```bash
python3 test_integration.py
```

### Custom Serial Port

Linux/macOS:
```bash
python3 test_integration.py --port /dev/ttyUSB0
```

Windows:
```bash
python3 test_integration.py --port COM3
```

### Specify ESP32 IP Address

If the ESP32 IP is not auto-detected, specify it manually:
```bash
python3 test_integration.py --ip 192.168.1.100
```

### Custom Timeout

Increase timeout for slower operations:
```bash
python3 test_integration.py --timeout 60
```

### Complete Example

```bash
python3 test_integration.py --port /dev/ttyUSB0 --ip 192.168.4.1 --baud 115200 --timeout 45
```

## Command-Line Options

| Option | Short | Description | Default |
|--------|-------|-------------|---------|
| `--port` | `-p` | Serial port to connect to ESP32 | `/dev/ttyUSB0` |
| `--baud` | `-b` | Serial baud rate | `115200` |
| `--ip` | | ESP32 IP address for web server tests | Auto-detected |
| `--timeout` | `-t` | Default timeout for operations (seconds) | `30` |

## Test Execution Flow

1. **Connect to Serial**: Establishes serial connection with ESP32
2. **Reset ESP32**: Triggers a reset to observe initialization
3. **System Init**: Verifies all modules initialize correctly
4. **Motion Test**: Prompts user to trigger motion sensor
5. **Image Capture**: Validates image capture and storage
6. **Metadata**: Checks metadata file creation and content
7. **Web Server**: Tests HTTP endpoints (if WiFi connected)
8. **Power Mgmt**: Verifies power management features
9. **Generate Report**: Creates detailed test report

## Manual Actions Required

During the **Motion Detection Test**, you will see:
```
*** MANUAL ACTION REQUIRED ***
Please trigger the PIR motion sensor now...
(You have 30 seconds to trigger motion)
```

At this point, physically trigger the PIR sensor by:
- Moving your hand in front of the sensor
- Walking in front of the camera
- Using a test motion source

## Output

### Console Output

The script provides color-coded console output:
- **Blue [INFO]**: Informational messages
- **Green [SUCCESS]**: Successful operations
- **Yellow [WARNING]**: Warnings and non-critical issues
- **Red [ERROR]**: Errors and failures
- **Purple [TEST]**: Test headers and results
- **Cyan [>]**: Serial output from ESP32

### Test Report

At the end of the test run, a comprehensive report is displayed showing:
- Total tests run
- Number passed/failed
- Pass rate percentage
- Individual test results with durations
- Detailed messages for each test

### JSON Report File

A detailed JSON report is saved to:
```
test_report_YYYYMMDD_HHMMSS.json
```

Example report structure:
```json
{
  "test_suite": "WildCAM ESP32 Integration Tests",
  "start_time": "2024-10-29T10:30:00",
  "end_time": "2024-10-29T10:35:00",
  "serial_port": "/dev/ttyUSB0",
  "total_tests": 6,
  "passed": 5,
  "failed": 1,
  "results": [
    {
      "name": "System Initialization",
      "passed": true,
      "message": "System initialized successfully",
      "duration": 15.3,
      "timestamp": "2024-10-29T10:30:15"
    }
  ]
}
```

## Exit Codes

- `0`: All tests passed
- `1`: One or more tests failed or error occurred
- `130`: Test interrupted by user (Ctrl+C)

## Troubleshooting

### Serial Connection Issues

**Problem**: Cannot connect to serial port
```
[ERROR] Failed to connect to serial port: [Errno 13] Permission denied
```

**Solution**:
- Linux: Add user to dialout group: `sudo usermod -a -G dialout $USER`
- Check port name: `ls /dev/tty*` or `ls /dev/cu.*` (macOS)
- Verify ESP32 is connected and drivers installed

### No Serial Output

**Problem**: Connected but no output from ESP32

**Solution**:
- Verify baud rate matches ESP32 firmware (115200)
- Try resetting ESP32 (press RESET button)
- Check USB cable and connection
- Verify firmware is flashed correctly

### Web Server Tests Fail

**Problem**: Cannot connect to web server

**Solution**:
- Verify ESP32 is connected to WiFi (check serial output)
- Ensure ESP32 IP address is correct
- Check firewall settings
- Verify WiFi credentials in config.h are correct
- Battery might be low (web server disabled in low power mode)

### Motion Not Detected

**Problem**: Motion detection test times out

**Solution**:
- Verify PIR sensor is connected to GPIO 13
- Check PIR sensor power (usually 5V)
- Ensure sensor is working (LED should blink on motion)
- Trigger motion within 30-second timeout window
- Check sensor cooldown period hasn't been exceeded

### Image Size Too Small

**Problem**: Image capture test fails due to small file size

**Solution**:
- Check camera initialization in serial output
- Verify SD card is inserted and working
- Check camera module connection (ribbon cable)
- Ensure adequate lighting for camera

## Best Practices

1. **Fresh Start**: Reset ESP32 before running tests for consistent results
2. **Serial Monitor**: Keep serial monitor output visible during tests
3. **Timing**: Be ready to trigger motion when prompted
4. **Environment**: Test in environment similar to deployment
5. **Multiple Runs**: Run tests multiple times to verify consistency
6. **Log Review**: Review JSON report for detailed analysis

## Integration with CI/CD

This test can be integrated into CI/CD pipelines with hardware-in-the-loop testing:

```bash
# Example CI/CD script
python3 test_integration.py --port $SERIAL_PORT --timeout 60
if [ $? -eq 0 ]; then
    echo "Integration tests passed"
else
    echo "Integration tests failed"
    exit 1
fi
```

## Known Limitations

1. **Motion Trigger**: Requires manual intervention to trigger PIR sensor
2. **SD Card Access**: Cannot directly verify files on SD card (relies on serial output)
3. **WiFi Dependency**: Web server tests require WiFi connection
4. **Hardware Required**: Cannot run in pure simulation mode (requires actual ESP32)
5. **Serial Port**: Requires physical serial connection to ESP32

## Future Enhancements

Planned improvements:
- Automated motion triggering via GPIO control
- Direct SD card verification via card reader
- Performance benchmarking
- Long-duration stress testing
- Multi-device parallel testing
- Automated firmware flashing

## Support

For issues or questions:
1. Check ESP32 serial output for errors
2. Review JSON test report for detailed failure information
3. Verify hardware connections and configuration
4. Check GitHub issues for known problems

## License

Part of the WildCAM ESP32 project.
