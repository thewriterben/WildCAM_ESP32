# 3D Reconstruction Quick Start Guide

Get started with 3D wildlife capture in 5 minutes!

## Prerequisites

- **3+ ESP32-CAM boards** with multi-board firmware
- **Python 3.7+** on your computer
- **SD cards** in each camera board
- **Network connectivity** between boards (LoRa or WiFi)

## Step 1: Hardware Setup (5 minutes)

Position cameras in a semi-circle around your subject area:

```
Camera 1     Camera 2     Camera 3
    \           |           /
     \          |          /
      \         |         /
       \        |        /
        \       |       /
              SUBJECT
```

**Spacing:** 30-45° between cameras  
**Distance:** 2-5 meters from subject  
**Height:** Same level for simplicity

## Step 2: Configure Each Board (2 minutes)

In `config.h` on each board:

```cpp
// Board 1
#define MULTIBOARD_ENABLED true
#define MULTIBOARD_NODE_ID 1
#define MULTIBOARD_PREFERRED_ROLE ROLE_NODE

// Board 2
#define MULTIBOARD_NODE_ID 2
// ... (increment for each board)
```

Flash firmware to all boards.

## Step 3: Record Camera Positions (3 minutes)

Measure camera positions relative to subject center:

```
Camera 1: (-2.0, 0.0, 1.5) facing 45°
Camera 2: ( 0.0, 0.0, 2.0) facing 0°
Camera 3: ( 2.0, 0.0, 1.5) facing 315°
```

## Step 4: Upload Coordinator Sketch (5 minutes)

Copy and modify `examples/3d_capture_example.cpp`:

```cpp
const Camera3DConfig CAMERA_ARRAY[] = {
    // Node, X,    Y,    Z,    Pitch, Yaw,   Roll
    {1,     -2.0,  0.0,  1.5,  0.0,   45.0,  0.0},
    {2,      0.0,  0.0,  2.0,  0.0,    0.0,  0.0},
    {3,      2.0,  0.0,  1.5,  0.0,  315.0,  0.0},
};
```

Upload to coordinator board (Node 100).

## Step 5: Capture! (1 minute)

Open Serial Monitor on coordinator, send command `1` to trigger a capture.

Wait 10 seconds for all cameras to capture and save.

## Step 6: Collect Images (5 minutes)

1. Remove SD cards from all boards
2. Copy `/3d_captures/` folder to your computer
3. Combine all images into one directory

## Step 7: Reconstruct (1 command!)

```bash
cd /path/to/WildCAM_ESP32
python3 tools/3d_reconstruction_aggregator.py \
    --session session_0 \
    --input /path/to/combined/3d_captures \
    --output ./my_3d_model

cd my_3d_model
./run_meshroom.sh
```

## What You Get

```
my_3d_model/
├── images/              # Your synchronized images
├── metadata/            # Camera calibration
├── run_meshroom.sh     # Ready-to-run reconstruction
├── run_colmap.sh       # Alternative reconstruction
└── README.md           # Detailed instructions
```

## Troubleshooting

**No images captured?**
- Check SD card is formatted (FAT32)
- Verify network connectivity
- Check serial monitor for errors

**Images not synchronized?**
- Increase `sync_delay_ms` in code
- Check network latency
- Verify all boards received tasks

**Poor 3D model?**
- Add more cameras (5-8 recommended)
- Improve lighting (avoid harsh shadows)
- Ensure subject was stationary
- Check camera overlap

## Next Steps

- Read the [Full 3D Reconstruction Guide](3D_RECONSTRUCTION_GUIDE.md)
- Try different [camera array configurations](3D_RECONSTRUCTION_GUIDE.md#multi-camera-setup)
- Experiment with [time-lapse 3D captures](3D_RECONSTRUCTION_GUIDE.md#automated-capture)

## Need Help?

- **Documentation:** See `docs/3D_RECONSTRUCTION_GUIDE.md`
- **Examples:** See `examples/3d_capture_example.cpp`
- **Issues:** https://github.com/thewriterben/WildCAM_ESP32/issues

---

**Total Time:** ~20 minutes from start to 3D model!
