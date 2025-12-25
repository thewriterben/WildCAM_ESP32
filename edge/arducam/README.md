# Arducam Pivistation 5 (Sony IMX500) Integration

Mid-tier edge AI platform for the WildCAM wildlife monitoring network with on-sensor neural network processing.

## Overview

The Arducam Pivistation 5 with Sony IMX500 sensor provides real-time AI inference directly on the camera sensor, enabling:

- **30fps real-time detection** with <10ms latency
- **Wildlife species classification** (20+ species)
- **Behavior analysis** (running, walking, resting, feeding)
- **Metadata-only transmission** for bandwidth efficiency
- **High-resolution capture** (12.3MP) on detection
- **Solar-powered operation** for remote deployment
- **LoRa/WiFi/MQTT connectivity** for mesh networking

## Hardware Requirements

### Required Components

| Component | Specification | Estimated Cost |
|-----------|---------------|----------------|
| Arducam Pivistation 5 Kit | Complete kit with IMX500 camera module | $299 |
| Raspberry Pi CM5 | Compute Module 5 (included in kit) | Included |
| MicroSD Card | Class 10, 32GB+ recommended | $10-15 |
| Power Supply | 5V 3A USB-C or solar panel + battery | $15-150 |

### Optional Components

| Component | Purpose | Cost |
|-----------|---------|------|
| LoRa Module (SX1276/SX1278) | Mesh networking with ESP32 nodes | $10-20 |
| Solar Panel (20W+) | Off-grid power | $40-80 |
| LiPo Battery (10,000mAh+) | Energy storage | $30-60 |
| Weatherproof Enclosure | Outdoor deployment | $25-50 |

## Software Prerequisites

### Operating System
- Raspberry Pi OS (64-bit) Bookworm or later
- Kernel 6.1+ with IMX500 support

### System Packages
```bash
sudo apt update
sudo apt install -y python3-pip python3-picamera2 python3-opencv
sudo apt install -y libatlas-base-dev libopenjp2-7
```

### Python Version
- Python 3.9 or later

## Installation

### 1. Clone Repository

```bash
cd /opt
sudo git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/edge/arducam
```

### 2. Install Python Dependencies

```bash
sudo pip3 install -r requirements.txt
```

### 3. Configure System

Edit configuration file:
```bash
sudo cp config/production.yaml /etc/wildcam/arducam.yaml
sudo nano /etc/wildcam/arducam.yaml
```

Key settings to configure:
- `device.node_id` - Unique identifier for this node
- `inference.model_path` - Path to deployed RPK model
- `capture.trigger_species` - Species that trigger high-res capture
- `lora.enabled` - Enable LoRa transmission
- `mqtt.broker` - MQTT broker address

### 4. Deploy AI Model

Download pre-trained wildlife detection model:
```bash
sudo mkdir -p /usr/share/imx500-models
cd /usr/share/imx500-models

# Download model (example - replace with actual URL)
sudo wget https://example.com/wildlife_detector.rpk
```

Or convert custom ONNX model:
```bash
python3 -c "from utils.model_converter import convert_onnx_to_rpk; \
            convert_onnx_to_rpk('model.onnx', '/usr/share/imx500-models/wildlife_detector.rpk')"
```

## Configuration Guide

### Inference Settings

```yaml
inference:
  model_path: "/usr/share/imx500-models/wildlife_detector.rpk"
  confidence_threshold: 0.5  # Minimum confidence for detections
  inference_fps: 30          # Target frame rate
```

**Tuning Tips:**
- Lower `confidence_threshold` (0.3-0.4) for more detections, higher false positives
- Higher `confidence_threshold` (0.6-0.8) for fewer, more confident detections
- Reduce `inference_fps` to save power in solar deployments

### Capture Settings

```yaml
capture:
  enabled: true
  trigger_species:  # Only these species trigger capture
    - "bear"
    - "wolf"
    - "mountain_lion"
  capture_threshold: 0.6  # Higher threshold than detection
  save_path: "/data/captures"
  max_storage_gb: 32
  cooldown_seconds: 30  # Prevent rapid-fire captures
```

### LoRa Configuration

```yaml
lora:
  enabled: true
  frequency: 915.0      # US: 915.0, EU: 868.0
  spreading_factor: 10  # 7-12 (higher = longer range, slower)
  tx_power: 17          # dBm (max ~20)
```

**Range vs. Speed Trade-offs:**
- SF7: ~2km range, fast transmission
- SF10: ~5km range, moderate speed
- SF12: ~10km range, slow transmission

### MQTT Configuration

```yaml
mqtt:
  enabled: true
  broker: "localhost"  # or cloud broker
  port: 1883
  username: ""         # Optional authentication
  password: ""
  topic_prefix: "wildcam"
```

**Cloud Broker Examples:**
- AWS IoT: `mqtt.region.amazonaws.com:8883` (requires TLS)
- Azure IoT Hub: `hub-name.azure-devices.net:8883` (requires TLS)

## Running the Service

### Manual Start

```bash
cd /opt/WildCAM_ESP32/edge/arducam
python3 -m services.main -c /etc/wildcam/arducam.yaml
```

### Systemd Service (Recommended)

Create service file:
```bash
sudo nano /etc/systemd/system/wildcam-arducam.service
```

Add content:
```ini
[Unit]
Description=WildCAM Arducam Wildlife Monitoring Service
After=network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/opt/WildCAM_ESP32/edge/arducam
ExecStart=/usr/bin/python3 -m services.main -c /etc/wildcam/arducam.yaml
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl daemon-reload
sudo systemctl enable wildcam-arducam
sudo systemctl start wildcam-arducam
```

Monitor logs:
```bash
sudo journalctl -u wildcam-arducam -f
```

## Model Deployment

### Using Pre-trained Models

Download from model repository:
```bash
cd /usr/share/imx500-models
sudo wget https://models.wildcam.io/imx500/wildlife_detector_v1.rpk
```

### Converting Custom Models

1. Train ONNX model (YOLOv5, YOLOv8, etc.)
2. Convert to RPK format:

```python
from edge.arducam.utils.model_converter import convert_onnx_to_rpk

convert_onnx_to_rpk(
    onnx_path="models/custom_wildlife.onnx",
    output_path="/usr/share/imx500-models/custom.rpk",
    quantization="int8",
    input_shape=(1, 3, 640, 640)
)
```

### Model Requirements

- Input: RGB image, 640x640 pixels
- Output: Bounding boxes + class probabilities
- Format: ONNX 1.12+
- Quantization: INT8 recommended for IMX500

## Integration with WildCAM Network

### LoRa Mesh Protocol

The Arducam node is compatible with WildCAM ESP32 LoRa mesh protocol:

```
Payload Format (10 bytes):
| species_id (1) | confidence (1) | bbox_x (2) | bbox_y (2) | bbox_w (2) | bbox_h (2) |
```

ESP32 nodes can receive and relay these messages.

### MQTT Topics

Published topics:
- `wildcam/{node_id}/detections` - Detection events
- `wildcam/{node_id}/telemetry` - Device health and stats
- `wildcam/{node_id}/images` - Image capture metadata
- `wildcam/{node_id}/status` - Online/offline status

### Raspberry Pi Gateway Integration

The Arducam node can act as:
1. **Standalone detector** - Direct cloud publishing
2. **Mid-tier gateway** - Aggregate data from ESP32 nodes
3. **Edge processor** - Run complex inference for mesh network

See [Gateway Integration Guide](../../gateway/README.md) for details.

## Power Management

### Solar Sizing

For 24/7 operation:
- **Solar Panel:** 20W minimum, 40W recommended
- **Battery:** 10,000mAh (37Wh) minimum
- **Daily Consumption:** ~20-30Wh (varies with inference load)

### Power-Saving Tips

1. Reduce inference FPS during low-activity periods
2. Disable high-res capture on low battery
3. Use LoRa instead of WiFi when possible
4. Implement sleep schedules (night mode)

Configuration example:
```yaml
power:
  solar_powered: true
  battery_low_threshold: 20
  reduce_inference_fps: true  # Auto-reduce FPS on low battery
```

## Troubleshooting

### Camera Not Detected

```bash
# Check IMX500 device
vcgencmd get_camera

# Should show: supported=1 detected=1

# If not detected:
sudo raspi-config  # Enable camera interface
sudo reboot
```

### Model Loading Fails

Check model file:
```bash
ls -lh /usr/share/imx500-models/
# Verify RPK file exists and is reasonable size (>1MB)
```

### LoRa Not Working

Check SPI interface:
```bash
ls /dev/spidev0.0
# Should exist

# Enable SPI if missing:
sudo raspi-config  # Interface Options -> SPI -> Enable
```

### MQTT Connection Fails

Test broker connectivity:
```bash
mosquitto_pub -h localhost -t test -m "hello"
```

### Low Detection Rate

Adjust thresholds:
```yaml
inference:
  confidence_threshold: 0.3  # Lower = more detections
```

## Performance Benchmarks

Expected performance on Raspberry Pi CM5:

| Metric | Value |
|--------|-------|
| Inference FPS | 30 fps |
| Inference Latency | 5-8 ms |
| Detection Rate | 0.5-2% (wildlife present) |
| High-res Capture Time | 200-300 ms |
| LoRa Transmission Time | 100-500 ms (depends on SF) |
| MQTT Publish Latency | 10-50 ms |
| Power Consumption | 3-5W (active inference) |

## Advanced Features

### Custom Callbacks

Register custom detection handlers:

```python
from edge.arducam.inference import IMX500WildlifeDetector

detector = IMX500WildlifeDetector(config)

def my_callback(detection):
    print(f"Detected: {detection.species}")
    # Custom processing here

detector.register_detection_callback(my_callback)
detector.start()
```

### Behavior Analysis

Enable behavior detection:
```yaml
inference:
  enable_behavior_analysis: true
```

Detections will include activity state (requires compatible model).

### Multi-Camera Setup

Run multiple instances for multi-camera arrays:
```bash
# Terminal 1
python3 -m services.main -c config/camera1.yaml

# Terminal 2
python3 -m services.main -c config/camera2.yaml
```

## Support and Resources

- **Documentation:** https://docs.wildcam.io
- **Model Repository:** https://models.wildcam.io
- **Community Forum:** https://community.wildcam.io
- **Issue Tracker:** https://github.com/thewriterben/WildCAM_ESP32/issues

## References

- [Arducam Pivistation 5 Product Page](https://www.arducam.com/arducam-for-raspberry-pi-ai-camera-kit-imx500-all-in-one-pivistation-5-ai-camera-kit.html)
- [Sony IMX500 Documentation](https://www.raspberrypi.com/documentation/accessories/ai-camera.html)
- [IMX500 Wiki](https://docs.arducam.com/AI-Camera-Solutions/Quick-Start-Guide/IMX500-CM5-AI-Camera-Kit/)
- [WildCAM Project Roadmap](../../PROJECT_ROADMAP.md)

## License

MIT License - See [LICENSE](../../LICENSE) for details
