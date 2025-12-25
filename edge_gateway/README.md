# NVIDIA Jetson Edge AI Gateway for WildCAM

High-performance edge computing gateway for advanced wildlife monitoring with real-time YOLO-based species detection, multi-camera video aggregation, and seamless cloud synchronization.

## Features

- **Real-time Object Detection**: TensorRT-optimized YOLOv8 inference (>20fps per camera on Orin Nano)
- **Multi-Camera Support**: Simultaneous processing of up to 8 camera streams (USB, RTSP, IP)
- **Cloud Synchronization**: Robust offline queue/retry logic with existing WildCAM cloud API
- **ESP32 Integration**: HTTP/WebSocket and LoRa relay interface for field nodes
- **Intelligent Triggers**: Motion-based and vision-based recording triggers
- **Telemetry Aggregation**: Local caching and batch sync for ESP32 field data

## Architecture

```
edge_gateway/
├── inference/          # YOLO detection and TensorRT optimization
├── camera/            # Multi-stream camera processing
├── sync/              # Cloud synchronization services
├── protocols/         # ESP32 communication handlers
├── api/               # REST/WebSocket API
├── config/            # Configuration management
├── models/            # Pre-trained YOLO models
├── docs/              # Documentation
└── tests/             # Unit and integration tests
```

## Quick Start

### Prerequisites

- NVIDIA Jetson (Orin Nano or Jetson Nano)
- JetPack 5.0+ installed
- Python 3.8+
- CUDA Toolkit and TensorRT (included in JetPack)

### Installation

```bash
# Install system dependencies
sudo apt-get update
sudo apt-get install python3-pip libopencv-dev

# Install Python dependencies
cd edge_gateway
pip3 install -r requirements.txt

# Download pre-trained YOLOv8 model
python3 scripts/download_models.py
```

### Configuration

Edit `config/gateway_config.yaml`:

```yaml
cameras:
  - name: "cam_1"
    source: "/dev/video0"  # USB camera
    type: "usb"
  - name: "cam_2"
    source: "rtsp://192.168.1.100:554/stream"
    type: "rtsp"

detection:
  model_path: "models/yolov8n_wildlife_trt.engine"
  confidence_threshold: 0.6
  classes: ["deer", "bear", "fox", "rabbit", "bird"]

cloud:
  api_url: "https://api.wildcam.example.com"
  sync_interval: 300  # seconds
  offline_queue_size: 1000

esp32:
  http_port: 8080
  websocket_port: 8081
  lora_gateway: "/dev/ttyUSB0"
```

### Running the Gateway

```bash
# Start the gateway service
python3 main.py --config config/gateway_config.yaml

# Or use systemd service
sudo systemctl start wildcam-gateway
```

## Documentation

- [Deployment Guide](docs/DEPLOYMENT.md) - Step-by-step setup for Jetson hardware
- [Performance Tuning](docs/PERFORMANCE.md) - Optimization for maximum throughput
- [API Reference](docs/API.md) - REST and WebSocket endpoints
- [ESP32 Integration](docs/ESP32_PROTOCOL.md) - Communication protocol details

## Performance Benchmarks

### Orin Nano (8GB)
- YOLOv8n: 45-60 FPS per camera (1080p)
- YOLOv8s: 25-35 FPS per camera (1080p)
- Max concurrent cameras: 8 (YOLOv8n)

### Jetson Nano (4GB)
- YOLOv8n: 20-25 FPS per camera (720p)
- Max concurrent cameras: 4 (YOLOv8n)

## License

MIT License - See main project LICENSE file
