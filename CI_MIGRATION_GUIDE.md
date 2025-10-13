# CI/CD Pipeline Migration Guide

## Overview

The CI/CD pipeline has been migrated from web application workflows (backend/frontend) to ESP32-specific workflows optimized for embedded development with PlatformIO.

## Changes Made

### 1. CI Workflow (`ci.yml`)

**Replaced:**
- Backend (Python/PostgreSQL/Redis) testing
- Frontend (Node.js/React) testing
- Docker image building and deployment
- Web application deployment stages

**New ESP32 CI includes:**
- **Project Structure Validation**: Verifies platformio.ini, source directories, and documentation
- **Code Linting**: Uses cppcheck for C++ code quality checks
- **Multi-Board Builds**: Compiles firmware for multiple ESP32 variants:
  - esp32cam (AI-Thinker ESP32-CAM)
  - esp32s3cam (ESP32-S3 based cameras)
  - ttgo-t-camera (TTGO T-Camera)
  - esp32c3cam (ESP32-C3 low-power variant)
  - esp32c6cam (ESP32-C6 with WiFi 6)
  - esp32s2cam (ESP32-S2 single-core)
- **Example Validation**: Builds example projects
- **Validation Scripts**: Runs project-specific validation scripts
- **Documentation Check**: Verifies documentation completeness

### 2. Security Scan Workflow (`security-scan.yml`)

**Replaced:**
- Backend/frontend dependency scanning (npm/pip)
- Web container security scanning
- Kubernetes manifest security checks
- Web-specific license compliance

**New ESP32 Security includes:**
- **Secret Detection**: TruffleHog for hardcoded credentials
- **Hardcoded Credentials Check**: Scans for WiFi passwords, API keys, IP addresses
- **Embedded Security Analysis**: 
  - Checks for unsafe C functions (strcpy, strcat, sprintf, gets)
  - Buffer overflow risk detection
  - Security feature verification (flash encryption, secure boot)
- **Library Dependency Scanning**: PlatformIO library security
- **Code Quality Security**: Enhanced cppcheck with security focus
- **Filesystem Scanning**: Trivy for filesystem vulnerabilities
- **License Compliance**: Embedded-appropriate license checking
- **OTA Security**: Verifies OTA update security measures

### 3. Infrastructure Tests Workflow (`infrastructure-tests.yml`)

**Replaced:**
- Kubernetes manifest validation
- Docker Compose configuration testing
- Nginx configuration validation
- Prometheus/Grafana monitoring config
- Web application integration tests
- Load testing with Apache Bench

**New ESP32 Infrastructure includes:**
- **PlatformIO Configuration Validation**: Syntax and environment checks
- **Board Definition Validation**: Verifies all board targets exist
- **Library Compatibility Check**: Tests library dependencies
- **Build Environment Testing**: Tests compilation for each board
- **Project Structure Validation**: Checks required directories and files
- **Partition Table Validation**: Validates ESP32 partition CSV files
- **Integration Compile Test**: Full compilation and firmware size checks

## Workflow Triggers

All workflows trigger on:
- Push to `main` and `develop` branches
- Pull requests to `main` and `develop` branches

Security scan also runs:
- Daily at 2 AM UTC (cron schedule)

## Artifacts

### Build Artifacts
- Firmware binaries for each board variant (`.bin` files)
- Retention: 30 days

### Security Reports
- Trivy scan results (uploaded to GitHub Security tab)
- TruffleHog secret detection results

## How to Use

### Running Locally

```bash
# Install PlatformIO
pip install platformio

# Validate project
cd ESP32WildlifeCAM-main
pio project config

# Build for specific board
pio run -e esp32cam

# Build for all boards
pio run

# Run validation scripts
bash scripts/quick_validate.sh
bash scripts/validate_critical_components.sh
```

### Continuous Integration

The workflows automatically run on all pull requests and pushes. Check the Actions tab in GitHub to see results.

### Firmware Artifacts

After successful builds, firmware binaries are available:
1. Go to Actions tab
2. Select the workflow run
3. Download artifacts (firmware-esp32cam, firmware-esp32s3cam, etc.)

## Board Support Matrix

| Board | Environment | Status | Notes |
|-------|-------------|--------|-------|
| AI-Thinker ESP32-CAM | esp32cam | ✅ Tested | Primary development board |
| ESP32-S3 CAM | esp32s3cam | ✅ Tested | USB CDC support |
| TTGO T-Camera | ttgo-t-camera | ✅ Tested | With display |
| ESP32-C3 CAM | esp32c3cam | ⚠️ Limited | RISC-V core, may need hardware |
| ESP32-C6 CAM | esp32c6cam | ⚠️ Limited | WiFi 6, may need hardware |
| ESP32-S2 CAM | esp32s2cam | ⚠️ Limited | Single core, may need hardware |

## Troubleshooting

### Build Failures

If builds fail:
1. Check platformio.ini syntax
2. Verify library dependencies are available
3. Check for board-specific issues
4. Review build logs in GitHub Actions

### Security Scan Failures

If security scans fail:
1. Review hardcoded credentials warnings
2. Check for unsafe C functions
3. Update library versions if vulnerabilities found
4. Review TruffleHog reports for secrets

### Missing Dependencies

If PlatformIO reports missing dependencies:
1. Ensure library versions in platformio.ini are available
2. Check platform version (espressif32@6.4.0)
3. Verify internet connectivity for library downloads

## Migration Benefits

1. **Appropriate Tooling**: Uses ESP32-specific tools (PlatformIO, cppcheck)
2. **Multi-Board Support**: Tests across all ESP32 variants
3. **Embedded Security**: Focuses on embedded-specific security concerns
4. **Faster Builds**: No unnecessary web application builds
5. **Better Validation**: ESP32-appropriate project validation
6. **Clear Artifacts**: Firmware binaries ready for deployment

## Next Steps

Consider adding:
- Hardware-in-the-loop (HIL) testing with actual ESP32 boards
- OTA update testing
- Power consumption profiling
- Flash memory usage analysis
- RTOS task validation
- Memory leak detection

## Support

For issues with the CI/CD pipeline:
1. Check workflow logs in GitHub Actions
2. Review this guide
3. Consult CONTRIBUTING.md
4. Open an issue with CI/CD label
