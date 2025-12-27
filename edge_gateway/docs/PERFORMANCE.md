# Performance Tuning Guide

Optimize your NVIDIA Jetson Edge Gateway for maximum throughput and efficiency.

## Table of Contents

- [Hardware Optimization](#hardware-optimization)
- [Software Optimization](#software-optimization)
- [Model Optimization](#model-optimization)
- [Camera Stream Optimization](#camera-stream-optimization)
- [Benchmarking](#benchmarking)

## Hardware Optimization

### Power Mode Configuration

**Jetson Orin Nano**
```bash
# Maximum performance mode (15W)
sudo nvpmodel -m 0
sudo jetson_clocks

# Verify current mode
sudo nvpmodel -q

# Monitor power consumption
sudo tegrastats
```

**Jetson Nano**
```bash
# 10W mode (maximum for Nano)
sudo nvpmodel -m 0
sudo jetson_clocks
```

### Storage Optimization

**Use NVMe SSD (Orin Nano)**
- NVMe provides much faster I/O than microSD
- Configure system and application data on NVMe

```bash
# Check disk performance
sudo hdparm -Tt /dev/nvme0n1
```

**Optimize microSD (Jetson Nano)**
```bash
# Use high-quality UHS-1 or faster card
# Minimize writes to SD card
# Use tmpfs for temporary files

# Mount tmpfs for detections (if not syncing to cloud)
sudo mkdir -p /tmp/detections
sudo mount -t tmpfs -o size=2G tmpfs /tmp/detections
```

### Cooling

**Ensure adequate cooling**
- Active cooling (fan) recommended for sustained performance
- Thermal throttling can reduce FPS by 30-50%

```bash
# Monitor temperature
watch -n 1 'cat /sys/devices/virtual/thermal/thermal_zone*/temp'

# Enable fan (if available)
sudo sh -c 'echo 255 > /sys/devices/pwm-fan/target_pwm'
```

## Software Optimization

### JetPack Version

**Use latest stable JetPack**
- JetPack 5.1+ recommended for Orin Nano
- JetPack 4.6+ recommended for Jetson Nano

### Python Optimization

**Use Python 3.8+**
```bash
# Verify Python version
python3 --version

# Install performance tools
pip3 install cython numba
```

### CUDA Optimization

**Configure CUDA properly**
```bash
# Verify CUDA installation
nvcc --version

# Set environment variables
export CUDA_HOME=/usr/local/cuda
export LD_LIBRARY_PATH=$CUDA_HOME/lib64:$LD_LIBRARY_PATH
export PATH=$CUDA_HOME/bin:$PATH
```

## Model Optimization

### Choose Right Model Size

**Model Size vs Performance Trade-off**

| Model | Orin Nano FPS (1080p) | Jetson Nano FPS (720p) | Memory Usage |
|-------|----------------------|------------------------|--------------|
| YOLOv8n | 45-60 | 20-25 | ~1.5 GB |
| YOLOv8s | 25-35 | 12-15 | ~2.5 GB |
| YOLOv8m | 15-20 | 6-8 | ~4 GB |

**Recommendation**: Use YOLOv8n for real-time performance

### TensorRT Optimization

**FP16 Precision** (Recommended)
```bash
/usr/src/tensorrt/bin/trtexec \
    --onnx=yolov8n.onnx \
    --saveEngine=yolov8n_fp16.engine \
    --fp16 \
    --workspace=4096 \
    --verbose
```

**INT8 Quantization** (Best Performance)
```bash
# Requires calibration dataset
/usr/src/tensorrt/bin/trtexec \
    --onnx=yolov8n.onnx \
    --saveEngine=yolov8n_int8.engine \
    --int8 \
    --workspace=4096 \
    --calib=calibration_images/ \
    --verbose
```

**Optimization Tips**:
- Use `--fp16` for 2x speedup over FP32
- Use `--int8` for 4x speedup (with minor accuracy loss)
- Increase `--workspace` for complex models
- Build engine on target device for best performance

### Input Size Optimization

**Resolution vs Speed**

| Input Size | Speed | Accuracy |
|------------|-------|----------|
| 320x320 | Fastest | Lower |
| 416x416 | Fast | Good |
| 640x640 | Balanced | Best |
| 1280x1280 | Slow | Highest |

```yaml
# In config file
detection:
  input_size: [640, 640]  # Balanced
  # or
  input_size: [416, 416]  # Faster, slightly less accurate
```

## Camera Stream Optimization

### Resolution and FPS

**Match camera capabilities**
```yaml
cameras:
  - name: "cam1"
    resolution: [1920, 1080]  # Full HD
    fps: 30  # Maximum for most cameras
    
  # For more cameras, reduce resolution
  - name: "cam2"
    resolution: [1280, 720]  # HD
    fps: 15  # Lower FPS for more concurrent streams
```

**Concurrent Streams**

| Platform | 1080p@30fps | 720p@30fps | 720p@15fps |
|----------|-------------|------------|------------|
| Orin Nano | 4-6 | 8-10 | 12-16 |
| Jetson Nano | 2-3 | 4-5 | 6-8 |

### GStreamer Acceleration

**Use hardware-accelerated GStreamer** (for RTSP cameras)

```python
# In camera config, use GStreamer pipeline
gst_pipeline = (
    f"rtspsrc location={rtsp_url} latency=0 ! "
    "rtph264depay ! h264parse ! "
    "nvv4l2decoder ! nvvidconv ! "
    "video/x-raw, format=BGRx ! "
    "videoconvert ! appsink"
)

cap = cv2.VideoCapture(gst_pipeline, cv2.CAP_GSTREAMER)
```

### Frame Buffering

**Optimize queue sizes**
```python
# In stream_processor.py
CameraStream(..., max_queue_size=10)  # Reduce for low latency
# or
CameraStream(..., max_queue_size=60)  # Increase for stability
```

## Benchmarking

### FPS Measurement

**Test detection speed**
```bash
# Run gateway with debug logging
python3 main.py --config config/gateway_config.yaml

# Monitor FPS in logs
tail -f logs/gateway_*.log | grep "fps"
```

**Get statistics via API**
```bash
# Get detailed stats
curl http://localhost:8000/api/stats | jq .

# Monitor in real-time
watch -n 1 'curl -s http://localhost:8000/api/stats | jq .stream_processor.detector.avg_fps'
```

### System Monitoring

**Monitor resource usage**
```bash
# GPU/CPU/Memory monitoring
sudo tegrastats

# Detailed GPU stats
nvidia-smi

# System resources
htop

# Disk I/O
iotop
```

### Performance Targets

**Expected Performance** (YOLOv8n, FP16)

**Orin Nano (8GB)**
- Single camera (1080p): 45-60 FPS
- 4 cameras (1080p): 25-30 FPS each
- 8 cameras (720p): 15-20 FPS each
- Memory usage: 3-4 GB
- Power consumption: 10-15W

**Jetson Nano (4GB)**
- Single camera (720p): 20-25 FPS
- 2 cameras (720p): 15-18 FPS each
- 4 cameras (480p): 10-12 FPS each
- Memory usage: 2.5-3 GB
- Power consumption: 8-10W

## Configuration Examples

### Maximum Performance (Orin Nano)

```yaml
gateway_id: "orin_max_perf"
log_level: "INFO"

cameras:
  - name: "cam1"
    source: "/dev/video0"
    type: "usb"
    fps: 30
    resolution: [1920, 1080]
    enabled: true
  # Add up to 6 cameras at 1080p

detection:
  model_path: "models/yolov8n_int8_trt.engine"  # INT8 quantized
  confidence_threshold: 0.6
  nms_threshold: 0.45
  input_size: [640, 640]

storage:
  detections_dir: "/mnt/nvme/detections"  # NVMe storage
```

### Maximum Cameras (Jetson Nano)

```yaml
gateway_id: "nano_multi_cam"
log_level: "WARNING"  # Reduce logging overhead

cameras:
  - name: "cam1"
    source: "/dev/video0"
    type: "usb"
    fps: 15  # Reduced FPS
    resolution: [640, 480]  # Lower resolution
    enabled: true
  # Add up to 4 cameras

detection:
  model_path: "models/yolov8n_int8_trt.engine"
  confidence_threshold: 0.7  # Higher threshold = less processing
  input_size: [416, 416]  # Smaller input
```

### Balanced (General Use)

```yaml
cameras:
  - name: "cam1"
    fps: 30
    resolution: [1280, 720]  # HD
    
detection:
  model_path: "models/yolov8n_fp16_trt.engine"
  input_size: [640, 640]
  confidence_threshold: 0.6
```

## Troubleshooting Performance Issues

### Low FPS

1. **Check thermal throttling**
   ```bash
   watch -n 1 'cat /sys/devices/virtual/thermal/thermal_zone*/temp'
   ```
   - Add cooling if temp >70°C

2. **Check power mode**
   ```bash
   sudo nvpmodel -q
   sudo jetson_clocks --show
   ```

3. **Reduce concurrent streams**
   - Disable cameras in config
   - Reduce resolution/FPS

4. **Use smaller model**
   - Switch from YOLOv8s to YOLOv8n
   - Use INT8 quantization

### High Memory Usage

1. **Reduce tensor arena size**
   - Lower `--workspace` when building TensorRT engine

2. **Limit camera queue size**
   ```python
   max_queue_size=10  # Instead of 30
   ```

3. **Disable image saving**
   ```yaml
   cloud:
     upload_images: false
   ```

### High Latency

1. **Reduce queue sizes**
   ```python
   max_queue_size=5  # Minimum buffering
   ```

2. **Use hardware decoding**
   - Enable GStreamer acceleration for RTSP

3. **Optimize sync interval**
   ```yaml
   cloud:
     sync_interval: 600  # Sync less frequently
   ```

## Best Practices

1. **Start with one camera** - Test and optimize before adding more
2. **Use NVMe storage** - Much faster than microSD
3. **Monitor temperatures** - Add cooling if needed
4. **Use FP16 or INT8** - Don't use FP32 models
5. **Match resolution to needs** - Don't use 4K if 1080p suffices
6. **Test on target device** - Build TensorRT engines on Jetson
7. **Update JetPack** - Use latest stable version

## Next Steps

- [Deployment Guide](DEPLOYMENT.md) - Initial setup
- [API Reference](API.md) - API documentation
- [ESP32 Integration](ESP32_PROTOCOL.md) - Field node communication
