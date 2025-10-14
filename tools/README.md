# WildCAM ESP32 Tools

This directory contains utility scripts and tools for working with the WildCAM ESP32 system.

## Available Tools

### 3D Reconstruction Aggregator

**File:** `3d_reconstruction_aggregator.py`

Aggregates images from multiple synchronized cameras and prepares them for 3D reconstruction using external tools like Meshroom or COLMAP.

**Purpose:**
- Collect images from multiple camera SD cards
- Parse camera position and orientation metadata
- Validate synchronization timing
- Export camera calibration data
- Generate reconstruction scripts

**Usage:**

```bash
# List available capture sessions
python3 3d_reconstruction_aggregator.py --list --input /path/to/sd_captures

# Process a specific session
python3 3d_reconstruction_aggregator.py \
    --session session_001 \
    --input /path/to/sd_captures \
    --output ./reconstruction_output
```

**Dependencies:**
```bash
pip install pillow numpy  # Optional but recommended
```

**Output:**
- Synchronized images with standardized naming
- Camera calibration JSON with positions/orientations
- Ready-to-run Meshroom and COLMAP scripts
- README with complete workflow instructions

**Documentation:** See [3D Reconstruction Guide](../ESP32WildlifeCAM-main/docs/3D_RECONSTRUCTION_GUIDE.md)

---

### Edge Deployment Tool

**File:** `edge_deployment.py`

Deploy and manage ML models on ESP32 edge devices.

---

### MegaDetector Integration

**File:** `megadetector_integration.py`

Integration with Microsoft's MegaDetector for wildlife detection.

---

### YOLO Wildlife

**File:** `yolo_wildlife.py`

YOLO-based wildlife detection and classification.

---

### TFLite System Validator

**File:** `validate_tflite_system.py`

Validate TensorFlow Lite models for ESP32 deployment.

---

## Contributing

When adding new tools:
1. Include a docstring at the top of the file explaining purpose and usage
2. Add command-line help with `--help` flag
3. Update this README with tool description
4. Add dependencies to requirements if needed
5. Include examples in the tool's help text

## Requirements

Create a virtual environment for development:

```bash
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install -r requirements.txt  # If exists
```

## Support

For issues with tools:
- Check tool's `--help` output for usage
- See documentation in `../ESP32WildlifeCAM-main/docs/`
- Open issue at https://github.com/thewriterben/WildCAM_ESP32/issues
