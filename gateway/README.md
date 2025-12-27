# WildCAM ESP32 - Raspberry Pi LoRa/MQTT Gateway

This directory contains the gateway software for the WildCAM wildlife monitoring system. The gateway runs on a Raspberry Pi and serves as a central hub for collecting data from ESP32 camera nodes via LoRa mesh network.

## Components

### Core Modules

1. **config.py** - Configuration management
   - LoRa radio settings
   - MQTT broker configuration
   - Database settings
   - API server configuration
   - Cloud sync settings

2. **database.py** - SQLite database for local storage
   - Node tracking and status
   - Wildlife detection events
   - Telemetry data
   - Packet logs
   - Mesh network health
   - Cloud sync queue

3. **lora_gateway.py** - LoRa packet receiver
   - Receives packets from ESP32 nodes
   - Processes wildlife detections
   - Handles telemetry data
   - Compatible with SX127x/SX1262 LoRa modules

4. **mqtt_bridge.py** - MQTT publisher
   - Publishes data to MQTT broker
   - Supports local Mosquitto, AWS IoT, Azure IoT Hub
   - Auto-reconnect functionality
   - TLS/SSL support

5. **api_server.py** - REST API server (Flask)
   - Query node status and detections
   - Retrieve telemetry data
   - Database statistics
   - Health check endpoints
   - Optional API key authentication
   - CORS support for web dashboards

6. **diagnostics.py** - System health monitoring
   - CPU, memory, disk usage monitoring
   - Temperature monitoring (Raspberry Pi)
   - Mesh network health checks
   - LoRa radio status
   - Gateway uptime tracking
   - Diagnostic report generation

7. **cloud_sync.py** - Cloud synchronization
   - Batched uploads to cloud API
   - Exponential backoff retry logic
   - Offline-first operation
   - Sync queue management
   - Configurable sync intervals

## Installation

### Prerequisites

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install Python dependencies
sudo apt install python3 python3-pip python3-dev

# Install system dependencies
sudo apt install sqlite3 mosquitto mosquitto-clients
```

### Python Requirements

```bash
cd gateway
pip3 install -r requirements.txt
```

### Hardware Setup

For LoRa functionality, connect a LoRa module (SX1276/SX1278 or SX1262) to the Raspberry Pi SPI pins:

- **VCC** → 3.3V (Pin 1)
- **GND** → Ground (Pin 6)
- **SCK** → GPIO 11 (Pin 23) - SPI SCLK
- **MISO** → GPIO 9 (Pin 21) - SPI MISO
- **MOSI** → GPIO 10 (Pin 19) - SPI MOSI
- **CS** → GPIO 8 (Pin 24) - SPI CE0
- **RST** → GPIO 25 (Pin 22)
- **DIO0** → GPIO 24 (Pin 18)

## Configuration

### Environment Variables

Create a `.env` file or set environment variables:

```bash
# LoRa Configuration
LORA_FREQUENCY=915.0          # 915 MHz (US), 868 (EU), 433 (Asia)
LORA_SPREADING_FACTOR=7
LORA_BANDWIDTH=125000

# MQTT Configuration
MQTT_ENABLED=true
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883
MQTT_USERNAME=wildcam
MQTT_PASSWORD=secret

# Database Configuration
DB_PATH=gateway_data.db
DB_RETENTION_DAYS=30

# API Configuration
API_ENABLED=true
API_HOST=0.0.0.0
API_PORT=5000
API_KEY=your-secret-api-key

# Cloud Sync Configuration
CLOUD_SYNC_ENABLED=true
CLOUD_SYNC_INTERVAL=300       # 5 minutes
CLOUD_API_URL=https://api.example.com
CLOUD_API_KEY=your-cloud-api-key

# Gateway Identity
GATEWAY_ID=gateway_001
GATEWAY_NAME=WildCAM Gateway
GATEWAY_LOCATION=Conservation Area A
```

## Usage

### Basic Usage

```python
from config import default_config
from database import GatewayDatabase
from lora_gateway import LoRaGateway
from mqtt_bridge import MQTTBridge
from api_server import create_api_server
from diagnostics import GatewayDiagnostics
from cloud_sync import CloudSyncService

# Initialize components
db = GatewayDatabase("gateway_data.db")
lora = LoRaGateway(default_config.lora, db)
mqtt = MQTTBridge(default_config.mqtt, db)
diagnostics = GatewayDiagnostics(default_config, db)
cloud_sync = CloudSyncService(default_config.cloud_sync, db)
api_server = create_api_server(default_config.api, db)

# Start services
lora.initialize()
lora.start()

mqtt.initialize()
mqtt.start()
mqtt.connect()

cloud_sync.start()

# Run API server (blocking)
api_server.run()
```

### Using the Example Gateway

```bash
# Run the complete gateway application
python3 example_gateway.py
```

### Generating Diagnostic Reports

```python
from diagnostics import GatewayDiagnostics

diagnostics = GatewayDiagnostics(config, db)

# Generate text report
report = diagnostics.generate_diagnostic_report(format='text')
print(report)

# Generate JSON report
report = diagnostics.generate_diagnostic_report(format='json')
```

## API Endpoints

### Health and Status

- `GET /api/health` - Health check
- `GET /api/status` - Gateway status
- `GET /api/stats` - Database statistics
- `GET /api/stats/summary` - Comprehensive summary

### Nodes

- `GET /api/nodes` - List all nodes
- `GET /api/nodes/<node_id>` - Get node details
- `GET /api/nodes/<node_id>/telemetry` - Node telemetry
- `GET /api/nodes/<node_id>/detections` - Node detections

### Detections

- `GET /api/detections` - List detections
- `GET /api/detections/<id>` - Get detection details
- `GET /api/detections/batch` - Batch retrieval for sync

### Telemetry

- `GET /api/telemetry` - List telemetry data
- `GET /api/telemetry/batch` - Batch retrieval

### Query Parameters

- `limit` - Maximum results (default: 100, max: 1000)
- `offset` - Pagination offset (default: 0)
- `node_id` - Filter by node ID
- `unsynced_only` - Only unsynced detections (true/false)
- `active_only` - Only active nodes (true/false)

### Authentication

Include API key in header or query parameter:

```bash
# Header
curl -H "X-API-Key: your-api-key" http://localhost:5000/api/nodes

# Query parameter
curl http://localhost:5000/api/nodes?api_key=your-api-key
```

## Database Schema

### Tables

- **nodes** - ESP32 node information and status
- **detections** - Wildlife detection events
- **telemetry** - Environmental and health data
- **packets** - Raw LoRa packet logs
- **mesh_health** - Network health snapshots
- **sync_queue** - Cloud synchronization queue

## Cloud Synchronization

The cloud sync service operates in offline-first mode:

1. Collects detections and telemetry locally
2. Batches data for efficient transmission
3. Uploads to cloud API at configured intervals
4. Retries failed uploads with exponential backoff
5. Maintains sync queue for resilience

### Retry Logic

- Initial retry delay: 60 seconds
- Exponential backoff: delay × 2^(attempt)
- Maximum delay: 1 hour
- Maximum attempts: 3 (configurable)

## Monitoring and Diagnostics

### System Health Checks

- CPU usage monitoring
- Memory usage monitoring
- Disk space monitoring
- Temperature monitoring (Raspberry Pi)
- Database health

### Mesh Network Monitoring

- Active node count
- Packet statistics
- Signal strength (RSSI/SNR)
- Network uptime

### LoRa Radio Status

- Radio operational status
- Packet reception rate
- Error rate
- Success rate

## Troubleshooting

### LoRa Not Working

1. Check SPI is enabled: `sudo raspi-config` → Interface Options → SPI
2. Verify wiring connections
3. Check LoRa module compatibility (SX127x or SX1262)
4. Review frequency settings for your region

### MQTT Connection Issues

1. Verify Mosquitto is running: `sudo systemctl status mosquitto`
2. Check broker host and port
3. Verify credentials if authentication enabled
4. Check firewall rules

### Database Issues

1. Check disk space: `df -h`
2. Verify database file permissions
3. Run cleanup: `db.cleanup_old_data(retention_days=30)`

### Cloud Sync Failures

1. Check internet connectivity
2. Verify cloud API URL and credentials
3. Review retry queue: `cloud_sync.get_retry_queue_status()`
4. Clear failed items: `cloud_sync.clear_failed_items()`

## Performance Tuning

### Database Optimization

- Set appropriate retention period
- Enable regular backups
- Monitor database size
- Use indexes for queries

### Cloud Sync Optimization

- Adjust sync interval based on bandwidth
- Configure appropriate batch size
- Monitor retry queue size

### API Server Optimization

- Use pagination for large datasets
- Enable caching for static data
- Configure appropriate CORS settings

## License

See main project LICENSE file.

## Contributing

See main project CONTRIBUTING.md file.

## Support

For issues and questions, please open an issue on the GitHub repository.
