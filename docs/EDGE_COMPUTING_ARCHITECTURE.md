# WildCAM Edge Computing Architecture

Comprehensive technical documentation for the WildCAM Edge Computing Platform.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [System Components](#system-components)
3. [Data Flow](#data-flow)
4. [Communication Protocols](#communication-protocols)
5. [Database Schema](#database-schema)
6. [API Specifications](#api-specifications)
7. [Deployment Patterns](#deployment-patterns)
8. [Security Architecture](#security-architecture)
9. [Performance Optimization](#performance-optimization)
10. [Scaling Strategies](#scaling-strategies)

## Architecture Overview

### Three-Tier Architecture

The WildCAM Edge Computing Platform implements a hierarchical three-tier architecture designed for wildlife monitoring in remote environments:

```
┌────────────────────────────────────────────────────────────────────┐
│                         CLOUD TIER                                  │
│  ┌──────────────┐  ┌─────────────┐  ┌──────────────────────┐       │
│  │ AWS S3/Azure │  │ ML Training │  │ Research Dashboard   │       │
│  │ Blob Storage │  │ Pipeline    │  │ & Analytics          │       │
│  └──────┬───────┘  └──────┬──────┘  └──────────┬───────────┘       │
│         │                 │                     │                   │
│         └─────────────────┴─────────────────────┘                   │
│                           │ HTTPS/MQTT                              │
└───────────────────────────┼─────────────────────────────────────────┘
                            │
┌───────────────────────────┼─────────────────────────────────────────┐
│                    EDGE GATEWAY TIER                                │
│                            │                                         │
│  ┌────────────────────────┴──────────────────────────┐              │
│  │                                                    │              │
│  │  ┌──────────────────────┐  ┌──────────────────────┴───┐          │
│  │  │ NVIDIA Jetson        │  │ Raspberry Pi LoRa        │          │
│  │  │ ┌──────────────────┐ │  │ Gateway                  │          │
│  │  │ │ YOLO Detector    │ │  │ ┌────────────────────┐   │          │
│  │  │ │ (TensorRT)       │ │  │ │ LoRa Receiver      │   │          │
│  │  │ └──────────────────┘ │  │ │ (SX127x/SX1262)    │   │          │
│  │  │ ┌──────────────────┐ │  │ └────────────────────┘   │          │
│  │  │ │ Cloud Sync       │ │  │ ┌────────────────────┐   │          │
│  │  │ │ (Offline Queue)  │ │  │ │ MQTT Bridge        │   │          │
│  │  │ └──────────────────┘ │  │ │ (Local/Cloud)      │   │          │
│  │  │ ┌──────────────────┐ │  │ └────────────────────┘   │          │
│  │  │ │ ESP32 Protocol   │ │  │ ┌────────────────────┐   │          │
│  │  │ │ (HTTP/WebSocket) │ │  │ │ REST API           │   │          │
│  │  │ └──────────────────┘ │  │ └────────────────────┘   │          │
│  │  │ ┌──────────────────┐ │  │ ┌────────────────────┐   │          │
│  │  │ │ Local Database   │ │  │ │ Local Database     │   │          │
│  │  │ │ (SQLite)         │ │  │ │ (SQLite)           │   │          │
│  │  │ └──────────────────┘ │  │ └────────────────────┘   │          │
│  │  └──────────────────────┘  └──────────────────────────┘          │
│  │         │ HTTP/WS                    │ LoRa 915MHz               │
│  └─────────┴────────────────────────────┴───────────────────────────┘
│                            │                                         │
└────────────────────────────┼─────────────────────────────────────────┘
                             │
┌────────────────────────────┼─────────────────────────────────────────┐
│                      SENSOR TIER                                     │
│                            │                                         │
│  ┌─────────────────────────┴────────────────────────┐               │
│  │                                                   │               │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │               │
│  │  │ ESP32-CAM│  │ ESP32-CAM│  │ ESP32-CAM│  ...  │               │
│  │  │ Node 001 │  │ Node 002 │  │ Node 003 │       │               │
│  │  │          │  │          │  │          │       │               │
│  │  │ • PIR    │  │ • Camera │  │ • TFLite │       │               │
│  │  │ • Camera │  │ • LoRa TX│  │ • LoRa   │       │               │
│  │  │ • TFLite │  │ • Battery│  │ • Sensors│       │               │
│  │  └──────────┘  └──────────┘  └──────────┘       │               │
│  └───────────────────────────────────────────────────┘               │
└──────────────────────────────────────────────────────────────────────┘
```

### Design Principles

1. **Offline-First**: Operate independently without cloud connectivity
2. **Edge Intelligence**: Process data locally to reduce bandwidth
3. **Resilient**: Automatic recovery from failures
4. **Scalable**: Support 1-100+ nodes per gateway
5. **Efficient**: Optimize for power and bandwidth constraints

## System Components

### 1. NVIDIA Jetson Edge Gateway

**Purpose**: Advanced AI processing and multi-camera aggregation

**Hardware:**
- NVIDIA Jetson (Nano/Xavier/Orin)
- 4-8GB RAM
- 64-256GB storage
- WiFi/Ethernet connectivity
- Optional: USB/CSI cameras

**Software Stack:**
- JetPack 5.0+ (CUDA, TensorRT, cuDNN)
- Python 3.8+
- YOLOv8 with TensorRT optimization
- aiohttp for async HTTP/WebSocket
- SQLite for local storage

**Key Features:**
- Real-time YOLO inference (20-80 FPS)
- Multi-camera stream processing (4-8 cameras)
- Object tracking with DeepSORT/ByteTrack
- Behavior analysis (running, walking, feeding, resting)
- Offline queue with exponential backoff retry
- HTTP REST API for ESP32 nodes
- WebSocket server for real-time updates

**Resource Requirements:**
- CPU: 25-50% (4 cameras, YOLOv8n)
- GPU: 60-80% (TensorRT inference)
- Memory: 2-4 GB
- Disk: 10-100 GB (depends on recording)
- Power: 10-20W

### 2. Raspberry Pi LoRa Gateway

**Purpose**: LoRa mesh network gateway and MQTT bridge

**Hardware:**
- Raspberry Pi 3B+/4
- 2-4GB RAM
- 32-64GB SD card
- SX1276/SX1262 LoRa module
- External antenna (recommended)

**Software Stack:**
- Raspberry Pi OS Lite
- Python 3.8+
- pyLoRa/SX126x driver
- asyncio-mqtt for MQTT
- Flask for REST API
- SQLite for local storage

**Key Features:**
- LoRa packet reception (SF7-SF12)
- Mesh protocol compatible with ESP32
- MQTT bridge (local/cloud)
- REST API for monitoring
- Node health tracking
- Automatic packet logging

**Resource Requirements:**
- CPU: 15-25%
- Memory: 200-300 MB
- Disk: 1-10 GB (packet logs)
- Power: 1.5-2W

### 3. ESP32-CAM Sensor Nodes

**Purpose**: Wildlife detection and image capture

**Hardware:**
- ESP32-CAM with OV2640 camera
- PIR motion sensor
- LoRa module (RFM95/SX1276)
- 3.7V LiPo battery + solar panel

**Software:**
- WildCAM ESP32 firmware (v3.1.0+)
- TensorFlow Lite for edge inference
- LoRa mesh networking
- Deep sleep power management

**Communication:**
- LoRa to Raspberry Pi gateway
- WiFi/HTTP to Jetson gateway (when available)
- Mesh routing for multi-hop

## Data Flow

### Detection Workflow

```
┌───────────────┐
│ ESP32 Detects │
│ Motion (PIR)  │
└───────┬───────┘
        │
        ▼
┌───────────────┐
│ Capture Image │
│ OV2640 Camera │
└───────┬───────┘
        │
        ▼
┌───────────────┐
│ TFLite        │
│ Classification│
└───────┬───────┘
        │
        ├─────────────────────────┐
        │                         │
        ▼                         ▼
┌───────────────┐        ┌───────────────┐
│ Send via LoRa │        │ Send via WiFi │
│ to RPI Gateway│        │ to Jetson     │
└───────┬───────┘        └───────┬───────┘
        │                         │
        ▼                         ▼
┌───────────────┐        ┌───────────────┐
│ RPI: Store in │        │ Jetson: YOLO  │
│ DB + Publish  │        │ Re-inference  │
│ to MQTT       │        │ + Tracking    │
└───────┬───────┘        └───────┬───────┘
        │                         │
        └────────┬────────────────┘
                 │
                 ▼
        ┌────────────────┐
        │ Cloud Sync     │
        │ (when online)  │
        └────────┬───────┘
                 │
                 ▼
        ┌────────────────┐
        │ Cloud Storage  │
        │ AWS S3 / Azure │
        └────────────────┘
```

### Offline Operation

When cloud connectivity is unavailable:

1. **Jetson Gateway**:
   - Queue detections in SQLite
   - Continue local processing
   - Store images locally
   - Retry sync with exponential backoff

2. **Raspberry Pi Gateway**:
   - Store LoRa packets in SQLite
   - Continue MQTT publishing (if local broker)
   - Buffer data for cloud MQTT

3. **ESP32 Nodes**:
   - Store on SD card
   - Transmit via LoRa mesh
   - Retry transmission if ACK not received

### Sync Priority

Data synced to cloud in priority order:

1. **Critical** (immediate): Endangered species detections
2. **High** (5 min): New species first detection
3. **Medium** (1 hour): Regular wildlife images
4. **Low** (when available): Telemetry, test images

## Communication Protocols

### LoRa Mesh Protocol

**Packet Structure** (compatible with ESP32 MeshManager):

```
┌─────────┬──────┬──────┬──────┬─────────┬───────┬─────────┐
│ Version │ Type │ From │  To  │ Seq Num │ Flags │ Payload │
│ (1B)    │ (1B) │ (4B) │ (4B) │  (2B)   │ (1B)  │ (var)   │
└─────────┴──────┴──────┴──────┴─────────┴───────┴─────────┘
```

**Message Types:**
- 0x01: BEACON (discovery)
- 0x02: DATA (generic)
- 0x03: ACK (acknowledgment)
- 0x04: ROUTING (mesh routing)
- 0x05: WILDLIFE (detection alert)
- 0x06: IMAGE (image chunk)
- 0x07: TELEMETRY (sensor data)
- 0x08: EMERGENCY (urgent alert)

**LoRa Parameters:**

| Parameter | Value | Notes |
|-----------|-------|-------|
| Frequency | 915.0 MHz | US (adjust per region) |
| Bandwidth | 125 kHz | Standard |
| Spreading Factor | 7-12 | Higher = longer range |
| Coding Rate | 4/5 | Error correction |
| TX Power | 17 dBm | Regulatory limit |

### HTTP API (Jetson)

**Base URL**: `http://jetson-gateway:8080`

**Endpoints:**

```
POST /api/detection
POST /api/image
POST /api/telemetry
POST /api/heartbeat
GET  /api/nodes
GET  /api/nodes/{id}/config
```

**Detection Format:**

```json
{
  "node_id": 1001,
  "species": "deer",
  "confidence": 0.85,
  "latitude": 45.5231,
  "longitude": -122.6765,
  "timestamp": "2024-12-25T12:00:00Z",
  "image_size": 153600,
  "has_image": true
}
```

### WebSocket (Jetson)

**URL**: `ws://jetson-gateway:8081/ws`

**Real-time Events:**
- New detections
- Node status changes
- System alerts

**Message Format:**

```json
{
  "event": "detection",
  "data": {
    "node_id": 1001,
    "species": "deer",
    "confidence": 0.85,
    "timestamp": "2024-12-25T12:00:00Z"
  }
}
```

### MQTT Topics (Raspberry Pi)

**Base Topic**: `wildcam/lora`

**Topic Structure:**

```
wildcam/lora/
├── detections/{node_id}    # Wildlife detections
├── telemetry/{node_id}     # Sensor telemetry
├── packets/{node_id}       # Raw LoRa packets
└── gateway/
    ├── status              # Gateway health
    └── stats               # Statistics
```

**QoS Levels:**
- Detections: QoS 1 (at least once)
- Telemetry: QoS 0 (at most once)
- Status: QoS 1 with retain

## Database Schema

### Jetson Gateway (SQLite)

**sync_queue table:**

```sql
CREATE TABLE sync_queue (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    item_type TEXT NOT NULL,        -- detection, telemetry, image, video
    payload TEXT NOT NULL,           -- JSON data
    created_at TEXT NOT NULL,
    retry_count INTEGER DEFAULT 0,
    next_retry TEXT NOT NULL,
    status TEXT DEFAULT 'pending'   -- pending, syncing, synced, failed
);
```

### Raspberry Pi Gateway (SQLite)

**packets table:**

```sql
CREATE TABLE packets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id INTEGER NOT NULL,
    packet_type INTEGER NOT NULL,
    sequence INTEGER NOT NULL,
    payload BLOB,
    rssi INTEGER,
    snr REAL,
    timestamp TEXT NOT NULL
);
```

**detections table:**

```sql
CREATE TABLE detections (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id INTEGER NOT NULL,
    species TEXT,
    confidence REAL,
    timestamp TEXT NOT NULL,
    latitude REAL,
    longitude REAL
);
```

**telemetry table:**

```sql
CREATE TABLE telemetry (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    node_id INTEGER NOT NULL,
    battery_level INTEGER,
    temperature REAL,
    humidity REAL,
    pressure REAL,
    rssi INTEGER,
    snr REAL,
    timestamp TEXT NOT NULL
);
```

## API Specifications

See individual README files:
- [Jetson API](../edge/jetson/README.md#api-endpoints)
- [Raspberry Pi API](../edge/raspberry_pi/README.md#api-endpoints)

## Deployment Patterns

### Pattern 1: Single Jetson Gateway

**Use Case**: Small deployment, WiFi-connected ESP32 nodes

```
ESP32 Nodes (WiFi)
     │
     ▼
Jetson Gateway ──► Cloud
```

**Pros**: Simple, high performance
**Cons**: Requires WiFi coverage

### Pattern 2: LoRa Mesh with Raspberry Pi

**Use Case**: Remote deployment, no WiFi

```
ESP32 Nodes (LoRa)
     │
     ▼
Raspberry Pi Gateway ──► MQTT Broker ──► Cloud
```

**Pros**: Long range, low power
**Cons**: Lower throughput

### Pattern 3: Hybrid (Recommended)

**Use Case**: Large deployment, mixed connectivity

```
ESP32 Nodes (LoRa) ──► Raspberry Pi ──► MQTT
     │                      │
     ▼                      ▼
ESP32 Nodes (WiFi) ──► Jetson ──────► Cloud
```

**Pros**: Flexibility, redundancy
**Cons**: More complex

## Security Architecture

### Authentication

- **API Keys**: Cloud API access
- **mTLS**: Device certificates for cloud MQTT
- **JWT**: Optional ESP32 authentication

### Encryption

- **In Transit**: TLS 1.3 for cloud communication
- **At Rest**: Optional SQLite encryption (SQLCipher)
- **LoRa**: AES-256 for mesh packets

### Access Control

- Read-only REST API endpoints
- Admin endpoints require authentication
- Principle of least privilege

## Performance Optimization

### Jetson Optimization

1. **TensorRT Optimization**:
   - FP16 precision (2x speedup)
   - INT8 quantization (4x speedup)
   - Dynamic batching

2. **Multi-threading**:
   - Async I/O with aiohttp
   - Separate threads for inference, sync, API

3. **Memory Management**:
   - Limit queue sizes
   - Periodic database cleanup

### Raspberry Pi Optimization

1. **LoRa Configuration**:
   - Optimal spreading factor for range vs. throughput
   - Bandwidth selection

2. **Database**:
   - Indexes on node_id, timestamp
   - Regular VACUUM operations

3. **MQTT**:
   - Persistent sessions
   - Connection pooling

## Scaling Strategies

### Horizontal Scaling

- Deploy multiple gateways for coverage
- Load balance ESP32 nodes across gateways
- Aggregate data at cloud level

### Vertical Scaling

- Upgrade to more powerful Jetson (Xavier, Orin)
- Increase Raspberry Pi count for LoRa capacity

### Cloud Scaling

- Auto-scaling for cloud processing
- CDN for image delivery
- Time-series database for analytics

---

**Version**: 1.0.0  
**Last Updated**: 2024-12-25  
**Maintained By**: WildCAM Edge Computing Team
