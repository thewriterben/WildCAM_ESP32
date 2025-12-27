# WildCAM Raspberry Pi LoRa/MQTT Edge Gateway

Complete gateway solution for aggregating ESP32-CAM wildlife monitoring nodes over LoRa mesh network, with MQTT bridge and cloud synchronization.

## Overview

The WildCAM Gateway is a Raspberry Pi-based edge computing platform that:
- **Receives LoRa packets** from distributed ESP32-CAM wildlife monitoring nodes
- **Stores data locally** in SQLite database with automatic rotation and retention
- **Bridges to MQTT** for integration with local/cloud brokers (Mosquitto, AWS IoT, Azure IoT Hub)
- **Exposes REST API** for monitoring, health checks, and data retrieval
- **Syncs to cloud** with robust offline-first queue and retry logic
- **Provides diagnostics** for field deployment and troubleshooting

## Features

### LoRa Mesh Gateway (`lora_gateway.py`)
- Compatible with SX127x and SX1262 LoRa modules
- Receives and processes multiple packet types (beacons, wildlife events, telemetry, data)
- Automatic node discovery and tracking
- Real-time RSSI and SNR monitoring
- Thread-safe packet processing

### Local Database (`database.py`)
- SQLite storage for nodes, detections, telemetry, and packets
- Automatic data rotation based on retention policy
- Indexed queries for fast retrieval
- Database statistics and health monitoring
- Backup support

### MQTT Bridge (`mqtt_bridge.py`)
- Publishes detections, telemetry, and health data to MQTT topics
- Supports TLS/SSL for secure connections
- Compatible with AWS IoT Core, Azure IoT Hub, and standard MQTT brokers
- Automatic reconnection with configurable retry
- QoS and message retention support

### REST API (`api_server.py`)
- Query nodes, detections, telemetry, and statistics
- Health check and status endpoints
- Optional API key authentication
- CORS support for web dashboards
- Batch data retrieval with pagination

### Cloud Synchronization (`cloud_sync.py`)
- Batched uploads to cloud API
- Exponential backoff retry logic (60s to 1 hour)
- Persistent sync queue in database
- Offline-first operation
- Failed upload tracking and retry

### Diagnostics (`diagnostics.py`)
- System resource monitoring (CPU, memory, disk, temperature)
- Mesh network health tracking
- LoRa radio status monitoring
- Gateway uptime and statistics
- Diagnostic reports (JSON and text formats)

## Hardware Requirements

### Required Components
- **Raspberry Pi 3/4/Zero 2W** - Gateway controller
- **LoRa Module** - SX127x (SX1276/SX1278) or SX1262 compatible
- **MicroSD Card** - 16GB+ for OS and data storage
- **Power Supply** - 5V 2.5A+ recommended

### LoRa Module Wiring (SPI)
```
LoRa Module    Raspberry Pi GPIO
-----------    -----------------
VCC            3.3V (Pin 1 or 17)
GND            GND (Pin 6, 9, 14, 20, 25, 30, 34, 39)
MISO           GPIO 9 (MISO, Pin 21)
MOSI           GPIO 10 (MOSI, Pin 19)
SCK            GPIO 11 (SCLK, Pin 23)
NSS/CS         GPIO 8 (CE0, Pin 24) - configurable
RESET          GPIO 25 (Pin 22) - configurable
DIO0           GPIO 24 (Pin 18) - configurable
```

## Installation

### Quick Install (Recommended)
```bash
# Clone repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/gateway

# Run installation script
sudo ./install.sh
```

The installation script will:
1. Update system packages
2. Install Python dependencies
3. Enable SPI interface
4. Create installation directory (`/opt/wildcam-gateway`)
5. Install systemd service
6. Create default configuration

### Manual Installation
```bash
# Install system dependencies
sudo apt-get update
sudo apt-get install -y python3 python3-pip python3-dev python3-spidev python3-rpi.gpio

# Enable SPI
sudo raspi-config
# Navigate to: Interface Options -> SPI -> Enable

# Install Python packages
pip3 install -r requirements.txt

# Create configuration
cp .env.example .env
nano .env  # Edit configuration
```

## Configuration

Configuration is managed via environment variables or `.env` file.

### Essential Settings

```bash
# Gateway identification
GATEWAY_ID=gateway_001
GATEWAY_NAME=WildCAM Gateway

# LoRa frequency (MHz): 915 (US), 868 (EU), 433 (Asia)
LORA_FREQUENCY=915.0

# MQTT broker
MQTT_ENABLED=true
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883

# API server
API_ENABLED=true
API_PORT=5000

# Cloud sync
CLOUD_SYNC_ENABLED=true
CLOUD_API_URL=https://api.wildcam.example.com/v1
CLOUD_API_KEY=your-api-key
```

See `.env.example` for complete configuration options.

### MQTT Broker Setup

#### Local Mosquitto
```bash
# Install Mosquitto
sudo apt-get install mosquitto mosquitto-clients

# Start service
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Test
mosquitto_sub -t 'wildcam/#' -v
```

#### AWS IoT Core
```bash
MQTT_ENABLED=true
MQTT_BROKER_HOST=your-endpoint.iot.us-east-1.amazonaws.com
MQTT_BROKER_PORT=8883
MQTT_USE_TLS=true
MQTT_CA_CERT=/path/to/AmazonRootCA1.pem
MQTT_CLIENT_CERT=/path/to/certificate.pem.crt
MQTT_CLIENT_KEY=/path/to/private.pem.key
```

#### Azure IoT Hub
```bash
MQTT_ENABLED=true
MQTT_BROKER_HOST=your-hub.azure-devices.net
MQTT_BROKER_PORT=8883
MQTT_USERNAME=your-hub.azure-devices.net/device-id/?api-version=2021-04-12
MQTT_USE_TLS=true
```

## Running the Gateway

### Using Systemd (Production)
```bash
# Enable automatic startup
sudo systemctl enable wildcam-gateway

# Start service
sudo systemctl start wildcam-gateway

# Check status
sudo systemctl status wildcam-gateway

# View logs
sudo journalctl -u wildcam-gateway -f

# Stop service
sudo systemctl stop wildcam-gateway
```

### Manual Run (Development/Testing)
```bash
# Run directly
python3 gateway.py

# Run with custom config
python3 gateway.py --config /path/to/config

# Run in background
nohup python3 gateway.py > gateway.log 2>&1 &
```

### Using Docker
```bash
# Build and start
docker-compose up -d

# View logs
docker-compose logs -f gateway

# Stop
docker-compose down
```

## REST API Reference

### Base URL
```
http://gateway-ip:5000/api
```

### Health Check
```bash
GET /api/health
```
Returns gateway health status and system information.

### Node Management
```bash
# Get all nodes
GET /api/nodes

# Get specific node
GET /api/nodes/{node_id}

# Get node statistics
GET /api/nodes/stats
```

### Wildlife Detections
```bash
# Get all detections
GET /api/detections?limit=100&offset=0

# Get node detections
GET /api/detections/{node_id}?limit=50

# Get unsynced detections
GET /api/detections/unsynced
```

### Telemetry Data
```bash
# Get all telemetry
GET /api/telemetry?limit=100

# Get node telemetry
GET /api/telemetry/{node_id}
```

### Gateway Statistics
```bash
# Get statistics
GET /api/stats

# Get diagnostics
GET /api/diagnostics
```

### Example API Calls
```bash
# Get health status
curl http://localhost:5000/api/health

# Get all nodes
curl http://localhost:5000/api/nodes

# Get recent detections
curl http://localhost:5000/api/detections?limit=10

# Get gateway statistics
curl http://localhost:5000/api/stats
```

## MQTT Topics

The gateway publishes data to the following topics:

```
wildcam/detections          - Wildlife detection events
wildcam/telemetry           - Environmental telemetry
wildcam/health              - Gateway health status
wildcam/gateway/status      - Gateway online/offline status
wildcam/nodes/{node_id}     - Individual node updates
```

### Subscribing to Topics
```bash
# Subscribe to all topics
mosquitto_sub -t 'wildcam/#' -v

# Subscribe to detections only
mosquitto_sub -t 'wildcam/detections' -v

# Subscribe to specific node
mosquitto_sub -t 'wildcam/nodes/0x12345678' -v
```

## Diagnostics and Monitoring

### System Health
```bash
# Get system diagnostics
curl http://localhost:5000/api/diagnostics

# Check gateway logs
sudo journalctl -u wildcam-gateway -n 100

# Monitor database size
ls -lh /var/lib/wildcam-gateway/gateway_data.db

# Check disk space
df -h /var/lib/wildcam-gateway
```

### Network Health
```bash
# View mesh network status
curl http://localhost:5000/api/stats | jq

# Monitor packet reception
watch -n 5 'curl -s http://localhost:5000/api/stats | jq .lora_stats'

# Check MQTT connection
curl http://localhost:5000/api/stats | jq .mqtt_stats
```

### Performance Tuning
```bash
# Adjust log level for debugging
export LOG_LEVEL=DEBUG

# Increase database retention
export DB_RETENTION_DAYS=60

# Adjust cloud sync interval
export CLOUD_SYNC_INTERVAL=600  # 10 minutes
```

## Troubleshooting

### LoRa Not Receiving
1. Check SPI is enabled: `ls /dev/spidev*`
2. Verify wiring connections
3. Check LoRa frequency matches ESP32 nodes
4. Verify spreading factor and bandwidth settings
5. Check antenna connection

### MQTT Connection Failed
1. Verify broker is running: `sudo systemctl status mosquitto`
2. Check broker host and port configuration
3. Test connection: `mosquitto_pub -t test -m "hello"`
4. Check firewall rules
5. Verify TLS certificates if using secure connection

### High CPU Usage
1. Reduce log level from DEBUG to INFO
2. Increase database cleanup interval
3. Disable unnecessary services
4. Check for packet flooding

### Database Growing Too Large
1. Reduce retention period: `DB_RETENTION_DAYS`
2. Enable automatic cleanup
3. Increase cleanup frequency
4. Consider external storage

## Integration with ESP32 Nodes

### ESP32 Configuration
Ensure ESP32-CAM nodes have matching LoRa settings:

```cpp
// config.h on ESP32
#define LORA_ENABLED true
#define LORA_FREQUENCY 915.0E6    // Match gateway
#define LORA_BANDWIDTH 125E3
#define LORA_SPREADING_FACTOR 7
#define LORA_CODING_RATE 5
```

### Testing Communication
1. Start gateway with DEBUG logging
2. Deploy ESP32 node in range (start with 50m)
3. Watch for beacon packets in gateway logs
4. Verify node appears in API: `curl http://localhost:5000/api/nodes`

## Performance Benchmarks

- **Packet Reception**: >1000 packets/day tested
- **Concurrent Nodes**: 10+ nodes simultaneously tracked
- **API Response Time**: <50ms average
- **Database Queries**: <100ms for 10,000 records
- **MQTT Latency**: <10ms local, <100ms cloud

## File Structure

```
gateway/
├── gateway.py              # Main entry point
├── config.py              # Configuration management
├── database.py            # SQLite database layer
├── lora_gateway.py        # LoRa packet receiver
├── mqtt_bridge.py         # MQTT publisher
├── api_server.py          # REST API server
├── cloud_sync.py          # Cloud synchronization
├── diagnostics.py         # System diagnostics
├── requirements.txt       # Python dependencies
├── install.sh             # Installation script
├── wildcam-gateway.service # Systemd service
├── docker-compose.yml     # Docker deployment
├── Dockerfile             # Docker image
├── mosquitto.conf         # MQTT broker config
├── .env.example           # Configuration template
└── README.md              # This file
```

## Security Considerations

1. **API Authentication**: Set `API_KEY` environment variable
2. **MQTT Security**: Use TLS/SSL for production deployments
3. **Firewall**: Restrict access to API port (5000) and MQTT port (1883)
4. **Database Backup**: Enable automatic backups
5. **Update Regularly**: Keep system and packages updated

## Support and Contributing

- **Issues**: https://github.com/thewriterben/WildCAM_ESP32/issues
- **Documentation**: See main repository README
- **Contributing**: Follow CONTRIBUTING.md guidelines

## License

MIT License - See LICENSE file in main repository

## Acknowledgments

Built for the WildCAM ESP32 wildlife monitoring project. Compatible with ESP32-CAM nodes running the WildCAM firmware.
