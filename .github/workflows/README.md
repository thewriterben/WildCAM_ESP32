# GitHub Actions Workflows

This directory contains automated workflows for the WildCAM ESP32 project.

## Build Firmware Releases (`build-firmware-releases.yml`)

Automatically builds firmware binaries for all supported ESP32-CAM boards when a new version tag is pushed.

### Supported Boards

- **AI-Thinker ESP32-CAM** (`esp32cam`)
- **ESP32-S3-CAM** (`esp32s3cam`)
- **TTGO T-Camera** (`ttgo-t-camera`)

### Triggers

#### Automatic: Version Tags
The workflow automatically runs when you push a version tag:

```bash
git tag v1.0.0
git push origin v1.0.0
```

Tag format: `v*.*.*` (e.g., `v1.0.0`, `v2.1.3`, `v1.0.0-beta`)

#### Manual: Workflow Dispatch
You can also trigger a build manually from the GitHub Actions tab:

1. Go to **Actions** → **Build Firmware Releases**
2. Click **Run workflow**
3. Enter a version string (e.g., `dev`, `test`, `v1.0.0-rc1`)

### Artifacts

Each build produces release artifacts for each board, retained for **90 days**:

- `wildcam_{board}_{version}.bin` - Main firmware binary
- `bootloader.bin` - ESP32 bootloader (if available)
- `partitions.bin` - Partition table (if available)
- `build_info.txt` - Build information and flash instructions

### Download Artifacts

1. Go to the **Actions** tab
2. Click on the workflow run
3. Scroll down to **Artifacts** section
4. Download the firmware package for your board

Example artifact names:
- `firmware-esp32cam-v1.0.0`
- `firmware-esp32s3cam-v1.0.0`
- `firmware-ttgo-t-camera-v1.0.0`

### Flashing Instructions

Instructions are included in each artifact's `build_info.txt` file.

#### Using esptool.py:
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash \
  -z 0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 wildcam_esp32cam_v1.0.0.bin
```

#### Using PlatformIO:
```bash
cd ESP32WildlifeCAM-main/WildCAM_ESP32
pio run -e esp32cam -t upload
```

### Build Process

The workflow performs the following steps for each board:

1. **Checkout** - Clones the repository with submodules
2. **Setup** - Installs Python 3.11 and PlatformIO
3. **Cache** - Caches PlatformIO packages for faster builds
4. **Build** - Compiles firmware using `pio run -e {board}`
5. **Package** - Prepares release artifacts with version naming
6. **Upload** - Stores artifacts in GitHub for download

### Build Time

Typical build times:
- First build: ~5-10 minutes (downloading dependencies)
- Cached builds: ~2-3 minutes per board
- Total workflow: ~6-9 minutes for all 3 boards (parallel)

### Troubleshooting

#### Build Fails
- Check that the board environment exists in `platformio.ini`
- Verify all dependencies are specified correctly
- Review build logs in the Actions tab

#### Artifacts Not Generated
- Ensure the build step completed successfully
- Check that the `.pio/build/{board}/` directory exists
- Verify file paths in the "Prepare release artifacts" step

#### Version Not Showing
- For tagged releases: Ensure tag format is `v*.*.*`
- For manual builds: The version is taken from the workflow input

### Related Workflows

- **ci.yml** - Continuous integration for all commits
- **security-scan.yml** - Security scanning
- **infrastructure-tests.yml** - Infrastructure validation

For more information, see [WORKFLOW_SUMMARY.md](../../WORKFLOW_SUMMARY.md)
