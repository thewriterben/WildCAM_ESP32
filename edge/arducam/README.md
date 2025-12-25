# Arducam Pivistation 5 (IMX500) Integration

## Overview

This module integrates the Arducam Pivistation 5, powered by the Sony IMX500 smart sensor, as an edge AI node for real-time wildlife monitoring in the WildCAM network. The IMX500 features an on-sensor Neural Processing Unit (NPU) that enables truly distributed AI inference, bridging the capability gap between ultra-low-power ESP32 sensors and high-performance Jetson gateways.

## Key Features

### Hardware Capabilities
- **On-Sensor NPU**: Sony IMX500 with dedicated neural processing unit
- **Real-Time Performance**: 30fps @ 640x640 resolution
- **High Resolution**: 12MP still image capture for detailed documentation
- **Edge Intelligence**: AI runs directly on camera sensor, minimal latency

### AI & Detection
- **Object Detection**: Real-time wildlife detection with bounding boxes
- **Species Classification**: Multi-class species identification
- **Pose Estimation**: Optional animal pose tracking (experimental)
- **Multi-Object Tracking**: Track multiple animals across frames
- **Custom Models**: Deploy your own ONNX models (via RPK conversion)

### Communication Protocols
- **LoRa**: Long-range, low-bandwidth metadata transmission
- **WiFi**: High-bandwidth local networking
- **MQTT**: Standard IoT messaging protocol
- **REST API**: Integration with existing web services
- **Satellite**: Ultra-remote deployments (Iridium, Swarm, etc.)

### Smart Features
- **Metadata-Only Mode**: Bandwidth-efficient detection result transmission
- **Store-and-Forward**: Offline operation with local storage
- **Event Triggering**: Capture high-res images only for target species
- **Power Management**: Configurable sleep modes for battery operation
- **Multi-Protocol Failover**: Automatic fallback to alternative protocols

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    IMX500 Edge Node                          │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌──────────────┐      ┌────────────────────────────────┐  │
│  │ Sony IMX500  │──────│  On-Sensor NPU                 │  │
│  │ 12MP Sensor  │      │  - Object Detection            │  │
│  └──────────────┘      │  - Classification              │  │
│         │              │  - 30fps @ 640x640             │  │
│         │              └────────────────────────────────┘  │
│         ▼                                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │         Wildlife Detector Module                      │  │
│  │  - Event filtering                                    │  │
│  │  - Confidence thresholding                            │  │
│  │  - Species targeting                                  │  │
│  │  - Multi-object tracking                              │  │
│  └──────────────────────────────────────────────────────┘  │
│         │                                                    │
│         ▼                                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │      Communication Layer (Multi-Protocol)             │  │
│  │  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────────┐   │  │
│  │  │ LoRa │ │ WiFi │ │ MQTT │ │ REST │ │Satellite │   │  │
│  │  └──────┘ └──────┘ └──────┘ └──────┘ └──────────┘   │  │
│  └──────────────────────────────────────────────────────┘  │
│         │                                                    │
│         ▼                                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │          Store-and-Forward Storage                    │  │
│  │  - Local detection cache                              │  │
│  │  - High-res image buffer                              │  │
│  │  - Automatic sync when online                         │  │
│  └──────────────────────────────────────────────────────┘  │
│                                                               │
└─────────────────────────────────────────────────────────────┘
                          │
                          ▼
        ┌─────────────────────────────────────┐
        │    WildCAM Gateway/Cloud             │
        │  - Pi Gateway                        │
        │  - Jetson Edge Computer              │
        │  - Cloud Platform                    │
        └─────────────────────────────────────┘
```

## Installation

### Prerequisites

1. **Hardware**:
   - Arducam Pivistation 5 (Raspberry Pi with IMX500 camera)
   - Optional: LoRa radio module (RFM95/96)
   - Optional: GPS module for location tagging
   - Power supply (USB-C or battery with solar)

2. **Software**:
   - Raspberry Pi OS (Bookworm or later)
   - Python 3.9+
   - Arducam camera drivers and SDK

### Setup Instructions

1. **Install Arducam drivers**:
```bash
# Follow Arducam's official installation guide
curl -s https://raw.githubusercontent.com/ArduCAM/Arducam-Pivariety-V4L2-Driver/master/install_pivariety_pkgs.sh | bash
```

2. **Install Python dependencies**:
```bash
pip install -r requirements.txt
```

3. **Install WildCAM edge module**:
```bash
cd /home/runner/work/WildCAM_ESP32/WildCAM_ESP32
pip install -e .  # Install in development mode
```

4. **Verify installation**:
```bash
python -c "from edge.arducam import IMX500WildlifeDetector; print('Installation successful!')"
```

## Quick Start

### 1. Basic Detection

```python
from edge.arducam import IMX500WildlifeDetector, IMX500Config
from edge.arducam.config import CommunicationProtocol

# Create configuration
config = IMX500Config(
    device_id="my_imx500_001",
    protocol=CommunicationProtocol.MQTT,
    mqtt_broker="localhost",
    detection_threshold=0.6,
    target_species=["deer", "bear", "wolf"]
)

# Initialize detector
detector = IMX500WildlifeDetector(config)

# Start detection
if detector.start():
    print("Detector running!")
    try:
        while True:
            status = detector.get_status()
            print(f"Detections: {status['detection_count']}")
            time.sleep(10)
    except KeyboardInterrupt:
        detector.stop()
```

### 2. Using Preset Configurations

```python
from edge.arducam import IMX500WildlifeDetector
from edge.arducam.config import PRESET_CONFIGS

# Load preset configuration
config = PRESET_CONFIGS["tropical_rainforest"]
config.device_id = "rainforest_node_001"

# Start detector
detector = IMX500WildlifeDetector(config)
detector.start()
```

### 3. Loading Configuration from File

```python
import json
from edge.arducam import IMX500WildlifeDetector, IMX500Config

# Load from JSON file
with open("edge/arducam/configs/arctic_deployment.json", "r") as f:
    config_dict = json.load(f)

# Create config from dictionary
config = IMX500Config.from_dict(config_dict["device_config"])

# Start detector
detector = IMX500WildlifeDetector(config)
detector.start()
```

## Model Deployment

### Converting ONNX Models to RPK

The IMX500 requires models in RPK (Arducam's proprietary format). Use the provided converter:

```python
from edge.arducam.model_deployment import ModelConverter

converter = ModelConverter()

# Convert ONNX to RPK
success = converter.convert_onnx_to_rpk(
    onnx_model_path="models/wildlife_detector.onnx",
    output_path="models/wildlife_detector.rpk",
    input_shape=(1, 3, 640, 640),
    quantize=True,  # INT8 quantization for performance
    optimization_level=3
)

if success:
    print("Model converted successfully!")
```

### Deploying Models to Device

```python
from edge.arducam.model_deployment import ModelDeployer

deployer = ModelDeployer(device_path="/home/pi/models")

# Deploy model
deployer.deploy_model(
    rpk_path="models/wildlife_detector.rpk",
    model_name="wildlife_v1",
    backup_existing=True
)

# List deployed models
models = deployer.list_deployed_models()
for name, info in models.items():
    print(f"{name}: {info['size_mb']:.2f} MB")
```

### Training Custom Models

1. **Prepare dataset** using wildlife images
2. **Train model** (YOLOv8, Faster R-CNN, etc.) to ONNX format
3. **Convert to RPK** using the converter
4. **Deploy to device**
5. **Update detector** configuration

Example workflow:
```bash
# Train with your dataset (example using YOLOv8)
python train_model.py --data wildlife.yaml --model yolov8n.pt --epochs 100

# Export to ONNX
python export_onnx.py --weights best.pt --img-size 640

# Convert to RPK
python -m edge.arducam.model_deployment convert \
    --input best.onnx \
    --output wildlife_detector.rpk \
    --quantize

# Deploy to device
python -m edge.arducam.model_deployment deploy \
    --model wildlife_detector.rpk \
    --name wildlife_v2
```

## Configuration

### Configuration Options

The `IMX500Config` class provides extensive configuration options:

#### Device Settings
- `device_id`: Unique identifier for this edge node
- `location`: Human-readable location
- `deployment_name`: Deployment identifier

#### Camera Settings
- `resolution`: Detection resolution (default: 640x640)
- `frame_rate`: Target FPS (1-30)
- `detection_threshold`: Confidence threshold (0.0-1.0)

#### Communication
- `protocol`: Primary protocol (LoRa, WiFi, MQTT, REST, Satellite)
- `fallback_protocols`: Backup protocols for resilience
- MQTT, LoRa, REST, Satellite-specific settings

#### Detection
- `detection_mode`: Continuous, motion-triggered, scheduled, or event-based
- `metadata_only`: Send only detection results (no images)
- `capture_high_res_on_detect`: Trigger 12MP capture for target species
- `target_species`: List of species to monitor
- `min_detection_interval_sec`: Minimum time between reports

#### Storage
- `store_and_forward`: Enable offline operation
- `max_storage_mb`: Maximum local storage
- `storage_path`: Local storage directory
- `sync_interval_minutes`: Sync frequency when online

#### Power Management
- `power_save_enabled`: Enable power saving
- `sleep_duration_sec`: Sleep time between captures
- `low_battery_threshold`: Low battery voltage threshold

### Preset Configurations

Four preset configurations are provided:

1. **Tropical Rainforest** (`tropical_rainforest`)
   - LoRa primary, WiFi fallback
   - Target species: jaguar, tapir, macaw, howler monkey
   - Metadata-only with high-res triggers
   - Power saving enabled

2. **Arctic Deployment** (`arctic_deployment`)
   - Satellite primary, LoRa fallback
   - Target species: polar bear, arctic fox, caribou
   - Motion-triggered mode
   - Extended power saving

3. **Research Station** (`research_station`)
   - WiFi primary, MQTT fallback
   - All species, continuous detection
   - Full images with pose estimation
   - Cloud sync enabled

4. **Low Power Remote** (`low_power_remote`)
   - LoRa only
   - Motion-triggered, aggressive power saving
   - Metadata-only, no high-res capture
   - Maximum battery life

## Communication Protocols

### LoRa

Optimized for long-range, low-bandwidth wildlife monitoring:

```python
config = IMX500Config(
    protocol=CommunicationProtocol.LORA,
    lora_frequency=915.0,  # or 868.0 for EU
    lora_spreading_factor=7,  # 7-12 (higher = longer range, lower speed)
    lora_bandwidth=125.0,  # kHz
    lora_tx_power=14,  # dBm
    metadata_only=True  # Essential for LoRa bandwidth
)
```

**LoRa Best Practices**:
- Use metadata-only mode
- Increase spreading factor for longer range (reduces data rate)
- Target 1-5 detections per minute max
- Compress species names to codes

### MQTT

Standard IoT protocol for local networks:

```python
config = IMX500Config(
    protocol=CommunicationProtocol.MQTT,
    mqtt_broker="gateway.local",
    mqtt_port=1883,
    mqtt_topic_prefix="wildcam/imx500",
    mqtt_use_tls=True,  # Enable for production
    mqtt_username="wildcam",
    mqtt_password="your_password"
)
```

**MQTT Topics**:
- `wildcam/imx500/{device_id}/detections` - Detection events
- `wildcam/imx500/{device_id}/status` - Device status
- `wildcam/imx500/{device_id}/images` - High-res images (if enabled)

### REST API

Direct HTTP(S) integration:

```python
config = IMX500Config(
    protocol=CommunicationProtocol.REST,
    rest_api_endpoint="https://api.wildcam.org/detections",
    rest_api_token="your_api_token"
)
```

### Satellite

For ultra-remote deployments:

```python
config = IMX500Config(
    protocol=CommunicationProtocol.SATELLITE,
    metadata_only=True,  # Critical - satellite is expensive
    min_detection_interval_sec=300,  # 5 minutes minimum
    store_and_forward=True  # Queue messages for batch transmission
)
```

**Satellite Considerations**:
- Extremely limited bandwidth (~100 bytes per message)
- High cost per message
- Use for critical alerts only
- Batch detections when possible

## Integration with WildCAM Platform

### With Pi Gateway

```python
# IMX500 node sends to Pi gateway via LoRa
config = IMX500Config(
    protocol=CommunicationProtocol.LORA,
    fallback_protocols=[CommunicationProtocol.WIFI],
    metadata_only=True
)
```

### With Jetson Edge Computer

```python
# IMX500 sends to Jetson via WiFi/MQTT
config = IMX500Config(
    protocol=CommunicationProtocol.MQTT,
    mqtt_broker="jetson.local",
    metadata_only=False,  # Can send full images to Jetson
    capture_high_res_on_detect=True
)
```

### With Cloud Platform

```python
# Direct cloud upload via REST API
config = IMX500Config(
    protocol=CommunicationProtocol.REST,
    rest_api_endpoint="https://api.wildcam.cloud/v1/detections",
    rest_api_token="your_cloud_token",
    cloud_sync_enabled=True,
    cloud_provider="aws",
    cloud_bucket="wildcam-detections"
)
```

## Field Deployment Guide

### 1. Pre-Deployment Checklist

- [ ] Hardware assembled and tested
- [ ] Model deployed and validated
- [ ] Configuration tested in lab
- [ ] Power system verified (battery, solar)
- [ ] Communication tested at deployment site
- [ ] Weatherproof enclosure prepared
- [ ] Mounting hardware ready
- [ ] GPS coordinates recorded

### 2. Deployment Steps

1. **Site Selection**
   - Clear view of wildlife corridor
   - Stable mounting location
   - LoRa/WiFi line of sight to gateway (if applicable)
   - Solar panel orientation (if using solar)

2. **Physical Installation**
   - Mount camera at wildlife eye level (varies by species)
   - Angle camera slightly downward
   - Secure all cables and connections
   - Test motion trigger range

3. **System Configuration**
   - Start detector in test mode
   - Verify detections with manual triggers
   - Confirm data transmission to gateway
   - Check battery and solar charging

4. **Finalization**
   - Seal enclosure against weather
   - Activate store-and-forward mode
   - Enable power saving if needed
   - Record deployment details

### 3. Monitoring and Maintenance

```python
# Remote status check
status = detector.get_status()
print(f"Detections: {status['detection_count']}")
print(f"Queue size: {status['queue_size']}")
print(f"Protocols: {status['active_protocols']}")
```

**Recommended Maintenance Schedule**:
- Weekly: Remote status check
- Monthly: Battery health check
- Quarterly: Physical inspection
- Annually: Model update and recalibration

## Performance Optimization

### Latency Optimization

For minimum latency:
```python
config = IMX500Config(
    frame_rate=30,  # Maximum
    detection_mode=DetectionMode.CONTINUOUS,
    min_detection_interval_sec=1,
    protocol=CommunicationProtocol.WIFI  # Fastest
)
```

Expected latency: **<250ms** from detection to transmission

### Battery Life Optimization

For maximum battery life:
```python
config = IMX500Config(
    detection_mode=DetectionMode.MOTION_TRIGGERED,
    power_save_enabled=True,
    sleep_duration_sec=600,  # 10 minutes
    metadata_only=True,
    capture_high_res_on_detect=False,
    protocol=CommunicationProtocol.LORA,
    lora_spreading_factor=12  # Max range, low power
)
```

Expected battery life: **30-60 days** on 10,000mAh battery with solar assist

### Bandwidth Optimization

For low-bandwidth networks:
```python
config = IMX500Config(
    metadata_only=True,
    min_detection_interval_sec=30,
    protocol=CommunicationProtocol.LORA,
    target_species=["rare_species"],  # Filter detections
)
```

Data usage: **~50-100 bytes per detection**

## Troubleshooting

### Camera Not Initializing

```bash
# Check camera detection
libcamera-hello --list-cameras

# Verify IMX500 driver
dmesg | grep imx500

# Check permissions
sudo usermod -a -G video $USER
```

### No Detections

1. Check model is loaded: `detector.config.model_path`
2. Lower detection threshold: `config.detection_threshold = 0.3`
3. Verify target species list matches model classes
4. Test with continuous mode: `config.detection_mode = DetectionMode.CONTINUOUS`

### Communication Failures

1. Check protocol handler initialized: `detector.comm_handlers`
2. Verify network connectivity (WiFi/LoRa)
3. Check MQTT broker reachability: `ping mqtt_broker`
4. Enable store-and-forward: `config.store_and_forward = True`

### High Power Consumption

1. Enable power save mode: `config.power_save_enabled = True`
2. Switch to motion-triggered: `config.detection_mode = DetectionMode.MOTION_TRIGGERED`
3. Reduce frame rate: `config.frame_rate = 10`
4. Increase sleep duration: `config.sleep_duration_sec = 300`

## API Reference

### IMX500WildlifeDetector

```python
class IMX500WildlifeDetector:
    def __init__(self, config: IMX500Config)
    def initialize_camera(self) -> bool
    def initialize_communication(self) -> bool
    def start(self) -> bool
    def stop(self)
    def get_status(self) -> Dict
    def update_model(self, model_path: str) -> bool
```

### IMX500Config

```python
@dataclass
class IMX500Config:
    # Core settings
    device_id: str
    resolution: tuple
    frame_rate: int
    detection_threshold: float
    
    # Communication
    protocol: CommunicationProtocol
    fallback_protocols: List[CommunicationProtocol]
    
    # Detection
    detection_mode: DetectionMode
    target_species: List[str]
    
    # Methods
    def to_dict(self) -> Dict
    def from_dict(cls, config_dict: Dict) -> "IMX500Config"
    def validate(self) -> bool
```

## Examples

See the `examples/` directory for complete examples:
- `basic_detection.py` - Simple detection setup
- `multi_protocol.py` - Multi-protocol failover
- `model_update.py` - Dynamic model updates
- `field_deployment.py` - Complete field deployment script

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](../../CONTRIBUTING.md) for guidelines.

## License

This module is part of the WildCAM_ESP32 project and is licensed under the MIT License. See [LICENSE](../../LICENSE) for details.

## Support

- **Documentation**: [WildCAM Docs](https://wildcam.readthedocs.io)
- **Issues**: [GitHub Issues](https://github.com/thewriterben/WildCAM_ESP32/issues)
- **Community**: [WildCAM Forum](https://community.wildcam.org)

## Acknowledgments

- Arducam for the Pivistation 5 platform
- Sony for the IMX500 smart sensor
- WildCAM community for testing and feedback

---

**Version**: 1.0.0  
**Last Updated**: 2025-12-25  
**Maintainer**: WildCAM Team
