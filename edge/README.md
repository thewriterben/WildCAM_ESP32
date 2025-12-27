# WildCAM Edge Computing Platform

Advanced edge computing capabilities for the WildCAM wildlife monitoring platform, featuring NVIDIA Jetson and Raspberry Pi gateway support for multi-tier AI processing and cloud connectivity.

## Overview

The WildCAM Edge Computing Platform creates a three-tier architecture:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        CLOUD TIER                                        │
│   AWS/Azure Storage  │  ML Model Training  │  Research Dashboard         │
└───────────────────────────────────┬─────────────────────────────────────┘
                                    │ WiFi/Cellular/Satellite
┌───────────────────────────────────┼─────────────────────────────────────┐
│              EDGE GATEWAY TIER (NVIDIA Jetson / Raspberry Pi)            │
│   • Advanced YOLO inference (real-time)                                  │
│   • Multi-camera aggregation                                             │
│   • Video analytics & behavior tracking                                  │
│   • Local database & caching                                             │
│   • Federated learning coordinator                                       │
│   • Cloud sync & uplink management                                       │
└───────────────────────────────────┬─────────────────────────────────────┘
                                    │ LoRa Mesh / WiFi
┌───────────────────────────────────┼─────────────────────────────────────┐
│                    SENSOR TIER (ESP32-CAM Nodes)                         │
│   • Motion detection  • Image capture  • Basic TFLite  • LoRa TX         │
└─────────────────────────────────────────────────────────────────────────┘
```

## Quick Start

### Prerequisites

- **For Jetson**: NVIDIA Jetson Nano/Xavier/Orin with JetPack 5.0+
- **For Raspberry Pi**: Raspberry Pi 3B+/4 with Raspberry Pi OS
- Python 3.8+
- Docker (optional)

### Installation

#### Option 1: Docker Deployment (Recommended)

```bash
# Clone repository
cd edge/

# Configure environment
cp .env.example .env
# Edit .env with your settings

# Deploy all services
docker-compose up -d

# View logs
docker-compose logs -f
```

#### Option 2: Native Installation

**Jetson Gateway:**
```bash
cd edge/jetson/

# Install dependencies
pip3 install -r requirements.txt

# Download YOLO model
mkdir -p /models
# Place your YOLOv8 model in /models/yolov8-wildlife.engine

# Run service
python3 -m edge.jetson.services.main --config /etc/wildcam/jetson --env production
```

**Raspberry Pi Gateway:**
```bash
cd edge/raspberry_pi/

# Install dependencies
pip3 install -r requirements.txt

# Install LoRa driver (for SX127x)
pip3 install pyLoRa

# Run service
python3 -m edge.raspberry_pi.gateway.main --config /etc/wildcam/rpi --env production
```

## Components

### NVIDIA Jetson Edge Gateway

**Features:**
- ✅ TensorRT-optimized YOLOv8 wildlife detection (>20 FPS)
- ✅ Multi-camera support (up to 8 simultaneous streams)
- ✅ Object tracking with behavior analysis
- ✅ Async cloud synchronization with offline queue
- ✅ HTTP API and WebSocket server for ESP32 nodes
- ✅ Local SQLite database with automatic cleanup

**Key Files:**
- `jetson/inference/wildlife_detector.py` - YOLO detector with TensorRT
- `jetson/services/cloud_sync.py` - Cloud sync with retry queue
- `jetson/services/esp32_protocol.py` - ESP32 communication handler
- `jetson/services/main.py` - Main service orchestrator
- `jetson/config/` - Configuration files

**API Endpoints:**
- `POST /api/detection` - Receive detection from ESP32
- `POST /api/image` - Receive image from ESP32
- `POST /api/telemetry` - Receive telemetry from ESP32
- `POST /api/heartbeat` - Node heartbeat
- `GET /api/nodes` - List all connected nodes
- `GET /api/nodes/{id}/config` - Get node configuration

### Raspberry Pi LoRa Gateway

**Features:**
- ✅ SX127x/SX1262 LoRa radio support
- ✅ Compatible with ESP32 mesh protocol
- ✅ MQTT bridge (local/cloud brokers)
- ✅ REST API for monitoring
- ✅ SQLite database for packets/detections/telemetry
- ✅ Automatic node tracking and health monitoring

**Key Files:**
- `raspberry_pi/gateway/lora_gateway.py` - LoRa radio driver and mesh protocol
- `raspberry_pi/gateway/mqtt_bridge.py` - MQTT bridge with auto-reconnect
- `raspberry_pi/gateway/rest_api.py` - REST API server
- `raspberry_pi/config/` - Configuration files

**API Endpoints:**
- `GET /health` - Health check
- `GET /api/nodes` - List mesh nodes
- `GET /api/detections` - Recent detections
- `GET /api/telemetry` - Recent telemetry
- `GET /api/stats` - Gateway statistics
- `GET /api/config` - Current configuration

### Shared Components

**Protocols:**
- ESP32 message protocol definitions
- Cloud API client with async support
- Common data structures

**Utilities:**
- Logging configuration
- Database helpers (async SQLite)
- YAML configuration loader

## Configuration

### Jetson Configuration

Edit `edge/jetson/config/production.yaml`:

```yaml
detector:
  model_path: /models/yolov8-wildlife.engine
  confidence_threshold: 0.6
  use_tensorrt: true
  max_cameras: 8

cloud:
  api_url: https://api.wildcam.example.com
  api_key: YOUR_API_KEY
  sync_interval: 60

esp32:
  http_port: 8080
  ws_port: 8081
```

### Raspberry Pi Configuration

Edit `edge/raspberry_pi/config/production.yaml`:

```yaml
lora:
  frequency: 915.0  # 915 MHz for US, 868 MHz for EU
  spreading_factor: 7
  bandwidth: 125000

mqtt:
  broker_host: localhost
  broker_port: 1883
  base_topic: wildcam/lora

api:
  host: 0.0.0.0
  port: 5000
```

## Usage Examples

### ESP32 Sending Detection to Jetson

```cpp
// ESP32 code
HTTPClient http;
http.begin("http://jetson-gateway:8080/api/detection");
http.addHeader("Content-Type", "application/json");

String payload = "{\"node_id\":1001,\"species\":\"deer\",\"confidence\":0.85}";
int httpCode = http.POST(payload);
```

### Querying Raspberry Pi Gateway

```bash
# Get all mesh nodes
curl http://rpi-gateway:5000/api/nodes

# Get recent detections
curl http://rpi-gateway:5000/api/detections?limit=10

# Get gateway stats
curl http://rpi-gateway:5000/api/stats
```

### Subscribing to MQTT Topics

```python
import paho.mqtt.client as mqtt

def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}, Payload: {msg.payload}")

client = mqtt.Client()
client.on_message = on_message
client.connect("mqtt-broker", 1883)

# Subscribe to all detection topics
client.subscribe("wildcam/lora/detections/#")
client.loop_forever()
```

## Monitoring

### Docker Health Checks

```bash
# Check service health
docker-compose ps

# View specific service logs
docker-compose logs -f jetson-gateway
docker-compose logs -f rpi-gateway
```

### Service Statistics

**Jetson:**
```bash
curl http://localhost:8080/api/stats
```

**Raspberry Pi:**
```bash
curl http://localhost:5000/api/stats
```

## Troubleshooting

### Jetson Gateway Issues

**Issue: Model not loading**
```bash
# Check model file exists
ls -lh /models/yolov8-wildlife.engine

# Check TensorRT installation
python3 -c "import tensorrt; print(tensorrt.__version__)"

# View logs
tail -f /var/log/wildcam/jetson-edge.log
```

**Issue: ESP32 connection failures**
```bash
# Check network connectivity
ping esp32-node-ip

# Verify API server running
curl http://localhost:8080/health
```

### Raspberry Pi Gateway Issues

**Issue: LoRa radio not detected**
```bash
# Check SPI enabled
ls -l /dev/spidev0.0

# Check GPIO permissions
groups | grep gpio

# Enable SPI if needed
sudo raspi-config
# Interface Options -> SPI -> Enable
```

**Issue: MQTT connection failures**
```bash
# Test MQTT broker
mosquitto_sub -h localhost -t '#' -v

# Check MQTT service
sudo systemctl status mosquitto

# View gateway logs
tail -f /var/log/wildcam/rpi-gateway.log
```

## Performance Benchmarks

### NVIDIA Jetson Nano

| Model | Resolution | FPS | Power |
|-------|-----------|-----|-------|
| YOLOv8n TensorRT | 640x640 | 25 | 10W |
| YOLOv8s TensorRT | 640x640 | 18 | 12W |
| YOLOv8m TensorRT | 640x640 | 10 | 15W |

### NVIDIA Jetson Xavier NX

| Model | Resolution | FPS | Power |
|-------|-----------|-----|-------|
| YOLOv8n TensorRT | 640x640 | 60 | 15W |
| YOLOv8s TensorRT | 640x640 | 45 | 18W |
| YOLOv8m TensorRT | 640x640 | 30 | 20W |

### Raspberry Pi 4 LoRa Gateway

| Metric | Value |
|--------|-------|
| LoRa Packets/sec | 10-50 |
| MQTT Publish/sec | 100+ |
| API Requests/sec | 50+ |
| Memory Usage | ~200 MB |
| CPU Usage | 15-25% |

## Security

### Authentication

- **API Keys**: Required for cloud API access
- **TLS/SSL**: Enabled for MQTT connections to cloud brokers
- **JWT**: Optional for ESP32 node authentication

### Data Privacy

- Local processing minimizes cloud data transfer
- Encryption for sensitive detection data
- Configurable data retention policies

## Contributing

See main repository [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

## License

See main repository [LICENSE](../LICENSE).

## Support

For detailed setup guides:
- [Jetson Setup Guide](jetson/README.md)
- [Raspberry Pi Setup Guide](raspberry_pi/README.md)
- [Edge Architecture Documentation](../docs/EDGE_COMPUTING_ARCHITECTURE.md)

For issues and questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Discussions: https://github.com/thewriterben/WildCAM_ESP32/discussions
