# Build Verification Script

## Overview

The `verify_build.py` script provides comprehensive build verification for the WildCAM ESP32 firmware project. It enforces code quality and consistency before commits.

## Features

### 1. Header/CPP File Consistency
- Scans for all `.h` header files in the firmware directory
- Verifies each header has a corresponding `.cpp` implementation file
- Automatically detects header-only files (templates, inline functions, enums, etc.)
- Reports any headers missing implementations

### 2. Undefined Extern Variables
- Parses the entire codebase for `extern` variable declarations
- Verifies all externs have matching definitions
- Reports any undefined extern variables with file locations

### 3. Multi-Board Build Verification
Compiles firmware for multiple ESP32 board types:
- **AI-Thinker ESP32-CAM** (4MB flash)
- **ESP32-S3-CAM** (8MB flash)
- **ESP-EYE/TTGO T-Camera** (4MB flash)

### 4. Compilation Error and Warning Reporting
- Captures all compilation errors and warnings
- Displays up to 10 errors and 20 warnings per board
- Shows clear build success/failure status

### 5. Binary Size Verification
- Checks firmware binary size after build
- Compares against board flash capacity
- **Fails** if binary exceeds 95% of flash size
- **Warns** if binary exceeds 85% of flash size

### 6. TODO Documentation Verification
- Scans codebase for all TODO comments
- Checks if TODOs are documented in `TODO.md`
- Lists all undocumented TODOs with file locations

## Requirements

- Python 3.6 or higher
- PlatformIO CLI (for builds)

### Installing PlatformIO

```bash
pip install platformio
```

Or via system package manager:
```bash
# Ubuntu/Debian
sudo apt install python3-platformio

# macOS
brew install platformio
```

## Usage

### Basic Usage (All Checks + All Boards)

```bash
python scripts/verify_build.py
```

### Skip Build (Code Quality Checks Only)

```bash
python scripts/verify_build.py --skip-build
```

This is useful for:
- Quick pre-commit checks
- CI environments without hardware platform support
- Testing code quality without waiting for compilation

### Build Specific Boards

```bash
# Build for single board
python scripts/verify_build.py --boards esp32cam_advanced

# Build for multiple specific boards
python scripts/verify_build.py --boards esp32cam_advanced esp32s3cam_ai
```

### Custom Firmware Path

```bash
python scripts/verify_build.py --firmware-path /path/to/firmware
```

### Help

```bash
python scripts/verify_build.py --help
```

## Exit Codes

- **0**: All checks passed
- **1**: One or more checks failed

## Output Format

The script provides clear, colored output with:
- ✓ Success indicators
- ✗ Failure indicators
- Detailed issue listings
- Build statistics (time, size, warnings)
- Summary report

## Example Output

```
======================================================================
WildCAM ESP32 Build Verification
======================================================================

1. Checking header/cpp file consistency...
   ✓ Header/CPP Consistency: PASSED

2. Checking for undefined extern variables...
   ✗ Undefined Externs: FAILED (2 issue(s))
     - Undefined extern 'g_config' at core/config.h:45

3. Checking TODO documentation...
   ✓ TODO Documentation: PASSED

4. Building firmware for multiple boards...

   Building for AI-Thinker ESP32-CAM...
   ✓ SUCCESS - AI-Thinker ESP32-CAM
      Build time: 42.3s
      Binary size: 1,234,567 bytes (30.2% of 4,194,304 bytes)
      Warnings: 3

======================================================================
VERIFICATION SUMMARY
======================================================================
Header/CPP Issues: 0
Undefined Externs: 2
Undocumented TODOs: 0
Build Results: 3/3 successful

✗ VERIFICATION FAILED
```

## Integration

### Pre-Commit Hook

Add to `.git/hooks/pre-commit`:

```bash
#!/bin/bash
python scripts/verify_build.py --skip-build
if [ $? -ne 0 ]; then
    echo "Build verification failed. Commit aborted."
    exit 1
fi
```

### CI/CD Integration

#### GitHub Actions

```yaml
- name: Verify Build
  run: |
    pip install platformio
    python scripts/verify_build.py
```

#### GitLab CI

```yaml
verify:
  script:
    - pip install platformio
    - python scripts/verify_build.py
```

## Known Issues

- **Network Errors**: In sandboxed/offline environments, PlatformIO may fail to download platform dependencies. Use `--skip-build` in these cases.
- **Long Build Times**: First build can take 5-10 minutes per board as PlatformIO downloads dependencies.
- **False Positives**: Some header-only detection may miss edge cases. Please report these.

## Troubleshooting

### "PlatformIO CLI not found"

Install PlatformIO:
```bash
pip install platformio
```

### "Could not find firmware directory"

Ensure you're running from the repository root, or specify the path:
```bash
python scripts/verify_build.py --firmware-path ./firmware
```

### Build Timeout

Increase timeout in the script or run individual board builds:
```bash
python scripts/verify_build.py --boards esp32cam_advanced
```

## Contributing

When adding new boards, update `BOARD_CONFIGS` in `verify_build.py`:

```python
BOARD_CONFIGS = {
    'new_board': {
        'name': 'New Board Name',
        'flash_size': 4 * 1024 * 1024,  # bytes
        'env_name': 'platformio_env_name'
    }
}
```

## License

Part of the WildCAM ESP32 project.
