# NVIDIA Jetson Edge Gateway - Setup Guide

Complete setup and deployment guide for the WildCAM Jetson Edge Gateway.

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Prerequisites](#software-prerequisites)
- [Installation](#installation)
- [Configuration](#configuration)
- [Model Deployment](#model-deployment)
- [Running the Service](#running-the-service)
- [Monitoring and Maintenance](#monitoring-and-maintenance)
- [Troubleshooting](#troubleshooting)

## Hardware Requirements

### Minimum Configuration

- **Platform**: NVIDIA Jetson Nano (4GB)
- **Storage**: 64GB SD Card (Class 10)
- **Power**: 5V 4A power supply
- **Network**: WiFi or Ethernet
- **Optional**: USB Camera or CSI camera

### Recommended Configuration

- **Platform**: NVIDIA Jetson Xavier NX or Orin Nano
- **Storage**: 128GB NVMe SSD
- **Power**: Appropriate power supply for platform
- **Network**: Gigabit Ethernet + WiFi
- **Cameras**: Up to 4 IP cameras or CSI cameras

### Tested Platforms

| Platform | Performance | Max Cameras | Notes |
|----------|------------|-------------|-------|
| Jetson Nano 2GB | 10-15 FPS | 2 | Entry level |
| Jetson Nano 4GB | 20-25 FPS | 4 | Recommended minimum |
| Jetson Xavier NX | 40-60 FPS | 6 | Excellent performance |
| Jetson Orin Nano | 60-80 FPS | 8 | Best performance |

## Software Prerequisites

### JetPack SDK

Install NVIDIA JetPack (includes CUDA, TensorRT, cuDNN):

```bash
# Check current JetPack version
sudo apt-cache show nvidia-jetpack

# Update to latest JetPack (if needed)
sudo apt update
sudo apt upgrade
```

**Required versions:**
- JetPack 5.0+ (for Orin)
- JetPack 4.6+ (for Nano/Xavier)

### Python Environment

```bash
# Update system Python
sudo apt update
sudo apt install -y python3-pip python3-dev

# Install virtualenv (optional but recommended)
pip3 install virtualenv

# Create virtual environment
python3 -m venv ~/wildcam-env
source ~/wildcam-env/bin/activate
```

### System Dependencies

```bash
# Install OpenCV dependencies
sudo apt install -y \
    libopencv-dev \
    python3-opencv \
    libopenblas-dev \
    libjpeg-dev \
    zlib1g-dev

# Install video codec libraries
sudo apt install -y \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev
```

## Installation

### 1. Clone Repository

```bash
cd ~
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/edge
```

### 2. Install Python Dependencies

```bash
cd jetson/
pip3 install -r requirements.txt
```

**Note**: TensorRT and PyTorch are pre-installed with JetPack. If using a virtual environment, you may need to link system packages:

```bash
# Link TensorRT
ln -s /usr/lib/python3.8/dist-packages/tensorrt ~/wildcam-env/lib/python3.8/site-packages/

# Link PyTorch
ln -s /usr/lib/python3.8/dist-packages/torch ~/wildcam-env/lib/python3.8/site-packages/
```

### 3. Create Directories

```bash
# Create data directories
sudo mkdir -p /data/sync_queue /data/recordings
sudo mkdir -p /models
sudo mkdir -p /var/log/wildcam
sudo mkdir -p /etc/wildcam/jetson

# Set permissions
sudo chown -R $USER:$USER /data /models /var/log/wildcam
```

## Configuration

### 1. Copy Configuration Files

```bash
# Copy configuration to /etc/wildcam
sudo cp -r config/* /etc/wildcam/jetson/

# Or create symbolic link
sudo ln -s $(pwd)/config /etc/wildcam/jetson
```

### 2. Configure Environment Variables

```bash
# Create .env file
cat > /etc/wildcam/jetson/.env << EOF
CLOUD_API_URL=https://api.wildcam.example.com
CLOUD_API_KEY=your_api_key_here
EOF
```

### 3. Edit Configuration

Edit `/etc/wildcam/jetson/production.yaml`:

```yaml
# Adjust settings for your deployment
detector:
  model_path: /models/yolov8-wildlife.engine
  confidence_threshold: 0.6
  max_cameras: 4  # Adjust based on hardware

cloud:
  api_url: ${CLOUD_API_URL}
  api_key: ${CLOUD_API_KEY}
  sync_interval: 60

esp32:
  http_port: 8080
  ws_port: 8081
  node_timeout: 300

cameras:
  - id: cam01
    source: rtsp://192.168.1.101:8554/stream
    enabled: true
  # Add your cameras
```

## Model Deployment

### 1. Download Pre-trained Model

```bash
# Download from WildCAM model repository
wget https://models.wildcam.example.com/yolov8-wildlife.onnx -O /models/yolov8-wildlife.onnx
```

### 2. Convert to TensorRT (Recommended)

```bash
# Install model conversion tools
pip3 install onnx

# Convert ONNX to TensorRT
/usr/src/tensorrt/bin/trtexec \
    --onnx=/models/yolov8-wildlife.onnx \
    --saveEngine=/models/yolov8-wildlife.engine \
    --fp16 \
    --workspace=4096

# Verify engine
ls -lh /models/yolov8-wildlife.engine
```

**Optimization options:**
- `--fp16`: Use FP16 precision (2x faster, minimal accuracy loss)
- `--int8`: Use INT8 precision (4x faster, requires calibration)
- `--workspace=4096`: Memory allocation (MB)

### 3. Custom Model Training

To deploy your own model:

```bash
# Train YOLOv8 model
# See: docs/ML_WORKFLOW.md

# Export to ONNX
yolo export model=path/to/best.pt format=onnx

# Convert to TensorRT (as above)
```

## Running the Service

### Development Mode

```bash
cd ~/WildCAM_ESP32/edge

# Run with development config
python3 -m edge.jetson.services.main \
    --config /etc/wildcam/jetson \
    --env development
```

### Production Mode

#### Systemd Service (Recommended)

Create `/etc/systemd/system/wildcam-jetson.service`:

```ini
[Unit]
Description=WildCAM Jetson Edge Gateway
After=network.target

[Service]
Type=simple
User=wildcam
WorkingDirectory=/home/wildcam/WildCAM_ESP32/edge
Environment="PYTHONUNBUFFERED=1"
ExecStart=/usr/bin/python3 -m edge.jetson.services.main --config /etc/wildcam/jetson --env production
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable wildcam-jetson
sudo systemctl start wildcam-jetson

# Check status
sudo systemctl status wildcam-jetson

# View logs
sudo journalctl -u wildcam-jetson -f
```

#### Docker Deployment

```bash
cd ~/WildCAM_ESP32/edge

# Build image
docker build -t wildcam-jetson -f jetson/Dockerfile ..

# Run container
docker run -d \
    --name wildcam-jetson \
    --runtime nvidia \
    --restart unless-stopped \
    -p 8080:8080 \
    -p 8081:8081 \
    -v /data:/data \
    -v /models:/models \
    -v /etc/wildcam/jetson:/etc/wildcam/jetson:ro \
    -e CLOUD_API_URL=https://api.wildcam.example.com \
    -e CLOUD_API_KEY=your_key \
    wildcam-jetson

# View logs
docker logs -f wildcam-jetson
```

## Monitoring and Maintenance

### Health Checks

```bash
# Check API health
curl http://localhost:8080/health

# Get system stats
curl http://localhost:8080/api/stats

# List connected nodes
curl http://localhost:8080/api/nodes
```

### Log Monitoring

```bash
# Real-time logs
tail -f /var/log/wildcam/jetson-edge.log

# Search for errors
grep ERROR /var/log/wildcam/jetson-edge.log

# Monitor with journalctl (if using systemd)
journalctl -u wildcam-jetson -f --since "1 hour ago"
```

### Performance Monitoring

```bash
# GPU utilization
sudo tegrastats

# Continuous monitoring
watch -n 1 'sudo tegrastats | grep GR3D'

# Install jtop for comprehensive monitoring
sudo pip3 install -U jetson-stats
sudo jtop
```

### Database Maintenance

```bash
# Check sync queue size
sqlite3 /data/sync_queue.db "SELECT COUNT(*) FROM sync_queue WHERE status='pending';"

# Clean old synced items
sqlite3 /data/sync_queue.db "DELETE FROM sync_queue WHERE status='synced' AND created_at < datetime('now', '-7 days');"

# Vacuum database
sqlite3 /data/sync_queue.db "VACUUM;"
```

## Troubleshooting

### Issue: High CPU/GPU Usage

```bash
# Check running processes
top -b -n 1 | head -20

# Reduce camera count in config
# Edit /etc/wildcam/jetson/production.yaml
# Set max_cameras to lower value

# Use lower resolution model
# Use YOLOv8n instead of YOLOv8s/m
```

### Issue: Model Inference Slow

```bash
# Verify TensorRT engine
ls -lh /models/*.engine

# Check GPU clocks
sudo jetson_clocks  # Set to max performance

# Benchmark model
/usr/src/tensorrt/bin/trtexec \
    --loadEngine=/models/yolov8-wildlife.engine \
    --iterations=100
```

### Issue: ESP32 Connections Dropped

```bash
# Check network connectivity
ping esp32-node-ip

# Verify ports open
sudo netstat -tulpn | grep -E '8080|8081'

# Increase node timeout in config
# Edit /etc/wildcam/jetson/production.yaml
# esp32.node_timeout: 600  # 10 minutes
```

### Issue: Cloud Sync Failures

```bash
# Check internet connectivity
ping api.wildcam.example.com

# Verify API credentials
curl -H "Authorization: Bearer $CLOUD_API_KEY" \
    https://api.wildcam.example.com/health

# Check sync queue
sqlite3 /data/sync_queue.db "SELECT status, COUNT(*) FROM sync_queue GROUP BY status;"

# Retry failed items
# They will auto-retry with exponential backoff
```

### Issue: Out of Disk Space

```bash
# Check disk usage
df -h /data

# Clean old recordings
find /data/recordings -name "*.mp4" -mtime +30 -delete

# Clean sync queue
sqlite3 /data/sync_queue.db "DELETE FROM sync_queue WHERE status='synced' AND created_at < datetime('now', '-7 days');"
```

## Performance Optimization

### 1. Enable Maximum Performance Mode

```bash
# Set Jetson to max performance
sudo jetson_clocks

# Make permanent
sudo systemctl enable jetson_clocks
```

### 2. Optimize TensorRT Engine

```bash
# Use FP16 for 2x speedup
trtexec --onnx=model.onnx --fp16 --saveEngine=model.engine

# Enable DLA (Deep Learning Accelerator) on Xavier
trtexec --onnx=model.onnx --useDLACore=0 --saveEngine=model.engine
```

### 3. Camera Resolution Optimization

Reduce camera resolution in RTSP streams to improve throughput:

```yaml
cameras:
  - id: cam01
    source: rtsp://camera:8554/stream?resolution=640x480
```

## Advanced Features

### Multi-Camera Synchronized Capture

Configure time-synchronized multi-camera capture:

```yaml
cameras:
  - id: cam01
    source: rtsp://192.168.1.101:8554/stream
    sync_group: front
  - id: cam02
    source: rtsp://192.168.1.102:8554/stream
    sync_group: front
```

### Behavior Analysis

Enable behavior tracking and analysis:

```yaml
detector:
  enable_tracking: true
  enable_behavior_analysis: true
  behavior_history_frames: 30
```

## Support

For additional help:
- Main Documentation: [Edge Computing README](../README.md)
- Architecture Guide: [../../docs/EDGE_COMPUTING_ARCHITECTURE.md](../../docs/EDGE_COMPUTING_ARCHITECTURE.md)
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
