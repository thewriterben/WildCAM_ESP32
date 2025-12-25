# NVIDIA Jetson Edge Gateway - Deployment Guide

Complete guide for deploying the WildCAM Edge Gateway on NVIDIA Jetson hardware (Orin Nano or Jetson Nano).

## Table of Contents

- [Prerequisites](#prerequisites)
- [Hardware Setup](#hardware-setup)
- [Software Installation](#software-installation)
- [Model Preparation](#model-preparation)
- [Configuration](#configuration)
- [Running the Gateway](#running-the-gateway)
- [Systemd Service](#systemd-service)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware Requirements

**Minimum (Jetson Nano 4GB)**
- NVIDIA Jetson Nano 4GB
- 64GB+ microSD card (Class 10 or UHS-1)
- 5V 4A power supply
- USB camera or IP camera
- Network connectivity (Ethernet or WiFi)

**Recommended (Jetson Orin Nano)**
- NVIDIA Jetson Orin Nano 8GB
- 128GB+ NVMe SSD
- 5V 5A power supply
- Multiple USB cameras or IP cameras
- Gigabit Ethernet

### Software Requirements

- JetPack 5.0+ (includes CUDA, cuDNN, TensorRT)
- Python 3.8+
- Git

## Hardware Setup

### 1. Jetson Initial Setup

1. **Flash JetPack**
   - Download NVIDIA SDK Manager on host PC
   - Flash JetPack 5.0+ to Jetson
   - Complete initial setup wizard

2. **Configure Power Mode** (Orin Nano)
   ```bash
   # Set maximum performance mode
   sudo nvpmodel -m 0
   sudo jetson_clocks
   ```

3. **Verify CUDA and TensorRT**
   ```bash
   # Check CUDA version
   nvcc --version
   
   # Check TensorRT
   dpkg -l | grep TensorRT
   ```

### 2. Camera Setup

**USB Cameras**
```bash
# List video devices
ls -l /dev/video*

# Test camera with OpenCV
python3 -c "import cv2; cap = cv2.VideoCapture(0); print('Camera OK' if cap.isOpened() else 'Camera FAIL')"
```

**IP/RTSP Cameras**
```bash
# Test RTSP stream
ffmpeg -i "rtsp://192.168.1.100:554/stream" -frames:v 1 test.jpg
```

### 3. LoRa Gateway (Optional)

If using LoRa for ESP32 communication:

```bash
# Check USB serial devices
ls -l /dev/ttyUSB*

# Add user to dialout group
sudo usermod -a -G dialout $USER

# Test serial port
python3 -c "import serial; s = serial.Serial('/dev/ttyUSB0', 115200); print('LoRa OK')"
```

## Software Installation

### 1. Clone Repository

```bash
cd ~
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/edge_gateway
```

### 2. Install System Dependencies

```bash
# Update system
sudo apt-get update
sudo apt-get upgrade -y

# Install Python dependencies
sudo apt-get install -y python3-pip python3-dev

# Install OpenCV dependencies
sudo apt-get install -y libopencv-dev python3-opencv

# Install system tools
sudo apt-get install -y git curl wget htop
```

### 3. Install Python Packages

```bash
# Upgrade pip
pip3 install --upgrade pip

# Install requirements
pip3 install -r requirements.txt

# Install PyCUDA (for TensorRT)
pip3 install pycuda
```

**Note**: TensorRT is pre-installed with JetPack and should be automatically available to Python.

## Model Preparation

### Option 1: Download Pre-trained Model

```bash
# Create models directory
mkdir -p models

# Download pre-trained YOLOv8n wildlife model (TensorRT engine)
# Replace URL with actual model location
wget -O models/yolov8n_wildlife_trt.engine \
    https://example.com/models/yolov8n_wildlife_trt.engine

# Download class labels
wget -O models/class_labels.json \
    https://example.com/models/class_labels.json
```

### Option 2: Convert ONNX to TensorRT

If you have a YOLOv8 ONNX model:

```bash
# Install trtexec (included with TensorRT)
/usr/src/tensorrt/bin/trtexec \
    --onnx=yolov8n.onnx \
    --saveEngine=models/yolov8n_wildlife_trt.engine \
    --fp16 \
    --workspace=4096

# For INT8 quantization (better performance)
/usr/src/tensorrt/bin/trtexec \
    --onnx=yolov8n.onnx \
    --saveEngine=models/yolov8n_wildlife_int8_trt.engine \
    --int8 \
    --workspace=4096 \
    --calib=calibration_images/
```

## Configuration

### 1. Create Configuration File

```bash
# Generate example configuration
python3 main.py --create-example-config

# Copy and edit configuration
mkdir -p config
cp config/gateway_config.example.yaml config/gateway_config.yaml
nano config/gateway_config.yaml
```

### 2. Configure Cameras

Edit `config/gateway_config.yaml`:

```yaml
cameras:
  - name: "cam_front"
    source: "/dev/video0"  # USB camera
    type: "usb"
    fps: 30
    resolution: [1920, 1080]
    enabled: true
    
  - name: "cam_trail"
    source: "rtsp://192.168.1.100:554/stream"  # IP camera
    type: "rtsp"
    fps: 30
    resolution: [1920, 1080]
    enabled: true
```

### 3. Configure Cloud Sync

```yaml
cloud:
  api_url: "https://api.wildcam.example.com"
  api_key: "your-api-key-here"  # Get from cloud backend
  sync_interval: 300  # Sync every 5 minutes
  upload_images: true
```

### 4. Configure ESP32 Integration

```yaml
esp32:
  http_port: 8080
  websocket_port: 8081
  lora_gateway: "/dev/ttyUSB0"  # Set to null if not using LoRa
  lora_baudrate: 115200
```

## Running the Gateway

### Manual Start

```bash
# Run gateway
python3 main.py --config config/gateway_config.yaml
```

### Test Detection

```bash
# In another terminal, test with sample image
curl -X POST http://localhost:8000/api/health

# Check status
curl http://localhost:8000/api/status

# View recent detections
curl http://localhost:8000/api/detections/recent
```

## Systemd Service

### 1. Create Service File

```bash
sudo nano /etc/systemd/system/wildcam-gateway.service
```

Add the following content:

```ini
[Unit]
Description=WildCAM Jetson Edge Gateway
After=network.target

[Service]
Type=simple
User=YOUR_USERNAME
WorkingDirectory=/home/YOUR_USERNAME/WildCAM_ESP32/edge_gateway
Environment="PYTHONUNBUFFERED=1"
ExecStart=/usr/bin/python3 /home/YOUR_USERNAME/WildCAM_ESP32/edge_gateway/main.py --config /home/YOUR_USERNAME/WildCAM_ESP32/edge_gateway/config/gateway_config.yaml
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Replace `YOUR_USERNAME` with your actual username.

### 2. Enable and Start Service

```bash
# Reload systemd
sudo systemctl daemon-reload

# Enable service to start on boot
sudo systemctl enable wildcam-gateway

# Start service
sudo systemctl start wildcam-gateway

# Check status
sudo systemctl status wildcam-gateway

# View logs
sudo journalctl -u wildcam-gateway -f
```

### 3. Service Management

```bash
# Stop service
sudo systemctl stop wildcam-gateway

# Restart service
sudo systemctl restart wildcam-gateway

# Disable service
sudo systemctl disable wildcam-gateway
```

## Troubleshooting

### Camera Issues

**Camera not detected**
```bash
# Check camera permissions
ls -l /dev/video0

# Add user to video group
sudo usermod -a -G video $USER

# Reboot
sudo reboot
```

**Low FPS**
```bash
# Check camera capabilities
v4l2-ctl --device=/dev/video0 --list-formats-ext

# Reduce resolution or lower FPS in config
```

### TensorRT Issues

**Model loading fails**
```bash
# Check TensorRT version compatibility
dpkg -l | grep tensorrt

# Rebuild TensorRT engine for your platform
/usr/src/tensorrt/bin/trtexec --onnx=model.onnx --saveEngine=model.engine
```

### Memory Issues

**Out of memory errors**
```bash
# Check memory usage
free -h

# Reduce number of cameras
# Or reduce model batch size
# Or use smaller YOLOv8n instead of YOLOv8s
```

**Swap configuration** (Jetson Nano)
```bash
# Increase swap space
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab
```

### Network Issues

**Cloud sync failing**
```bash
# Test cloud connectivity
curl https://api.wildcam.example.com/api/health

# Check API key
# View sync queue database
sqlite3 data/sync_queue.db "SELECT * FROM sync_queue LIMIT 10;"
```

**ESP32 nodes not connecting**
```bash
# Check firewall
sudo ufw status
sudo ufw allow 8080/tcp
sudo ufw allow 8081/tcp

# Test from ESP32
curl http://JETSON_IP:8080/api/health
```

### Performance Tuning

**Maximize Jetson performance**
```bash
# Set performance mode
sudo nvpmodel -m 0
sudo jetson_clocks

# Monitor GPU/CPU usage
sudo tegrastats
```

**Optimize camera streams**
```yaml
# In config file, reduce resolution for more cameras
cameras:
  - resolution: [1280, 720]  # Instead of [1920, 1080]
    fps: 15  # Instead of 30
```

### Logs and Debugging

**Enable debug logging**
```yaml
# In config file
log_level: "DEBUG"
```

**View logs**
```bash
# Application logs
tail -f logs/gateway_*.log

# System service logs
sudo journalctl -u wildcam-gateway -f --since "10 minutes ago"
```

## Next Steps

- [Performance Tuning Guide](PERFORMANCE.md) - Optimize for maximum throughput
- [API Reference](API.md) - REST and WebSocket API documentation
- [ESP32 Integration](ESP32_PROTOCOL.md) - Configure ESP32 field nodes

## Support

For issues and questions:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Documentation: https://github.com/thewriterben/WildCAM_ESP32/tree/main/edge_gateway/docs
