# 🚀 WildCAM ESP32 v2.0 Deployment Guide

## Overview
This guide covers deployment of the advanced ESP32 WildCAM v2.0 system from development to production field deployment.

## Pre-Deployment Checklist

### Hardware Preparation
- [ ] ESP32-CAM with 8MB PSRAM tested
- [ ] Camera module (OV2640/OV5640) functional
- [ ] PIR motion sensor calibrated
- [ ] Environmental sensors (BME280) verified
- [ ] Solar panel and MPPT controller tested
- [ ] Battery system (18650 Li-ion) charged
- [ ] LoRa module configured for local frequency
- [ ] ATECC608A security chip initialized
- [ ] Enclosure weatherproofing validated

### Software Prerequisites
- [ ] PlatformIO installed and configured
- [ ] Python 3.8+ for backend services
- [ ] Node.js 16+ for frontend applications
- [ ] InfluxDB time-series database setup
- [ ] Docker for containerized deployment

## Phase 1: Firmware Deployment

### 1.1 Build Configuration
```bash
# Navigate to firmware directory
cd firmware/

# Select appropriate configuration
# For standard deployment:
pio run -e esp32cam_advanced

# For power-optimized remote deployment:
pio run -e esp32cam_lowpower

# For mesh network nodes:
pio run -e esp32cam_mesh

# For satellite-enabled deployment:
pio run -e esp32cam_satellite
```

### 1.2 Configuration Settings
Edit `firmware/config.h` for your deployment:
```cpp
// Network configuration
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
#define LORA_FREQUENCY 915E6  // Adjust for your region

// Power management
#define BATTERY_MIN_VOLTAGE 3.0
#define SOLAR_MIN_VOLTAGE 4.5

// AI detection thresholds
#define CONFIDENCE_THRESHOLD 0.7
#define DETECTION_COOLDOWN_MS 5000
```

### 1.3 Firmware Upload
```bash
# Connect ESP32 via USB
# Upload firmware
pio run -e esp32cam_advanced --target upload

# Monitor serial output
pio device monitor
```

## Phase 2: Backend Services Deployment

### 2.1 InfluxDB Setup
```bash
# Using Docker
docker run -d \
  --name influxdb \
  -p 8086:8086 \
  -v influxdb-storage:/var/lib/influxdb2 \
  -e DOCKER_INFLUXDB_INIT_MODE=setup \
  -e DOCKER_INFLUXDB_INIT_USERNAME=admin \
  -e DOCKER_INFLUXDB_INIT_PASSWORD=wildcam2024 \
  -e DOCKER_INFLUXDB_INIT_ORG=wildlife \
  -e DOCKER_INFLUXDB_INIT_BUCKET=camera_data \
  influxdb:2.7
```

### 2.2 API Server Deployment
```bash
# Install dependencies
cd backend/
pip install -r requirements.txt

# Set environment variables
export INFLUXDB_URL=http://localhost:8086
export INFLUXDB_TOKEN=your_influxdb_token
export INFLUXDB_ORG=wildlife
export INFLUXDB_BUCKET=camera_data

# Start API server
python api/wildlife_api.py
```

### 2.3 Docker Compose Deployment
```yaml
# docker-compose.yml
version: '3.8'
services:
  influxdb:
    image: influxdb:2.7
    ports:
      - "8086:8086"
    environment:
      - DOCKER_INFLUXDB_INIT_MODE=setup
      - DOCKER_INFLUXDB_INIT_USERNAME=admin
      - DOCKER_INFLUXDB_INIT_PASSWORD=wildcam2024
      - DOCKER_INFLUXDB_INIT_ORG=wildlife
      - DOCKER_INFLUXDB_INIT_BUCKET=camera_data
    volumes:
      - influxdb-storage:/var/lib/influxdb2

  wildcam-api:
    build: ./backend
    ports:
      - "5000:5000"
    depends_on:
      - influxdb
    environment:
      - INFLUXDB_URL=http://influxdb:8086
      - INFLUXDB_TOKEN=${INFLUXDB_TOKEN}

  wildcam-dashboard:
    build: ./frontend/dashboard
    ports:
      - "3000:3000"
    depends_on:
      - wildcam-api

volumes:
  influxdb-storage:
```

## Phase 3: Frontend Deployment

### 3.1 React Dashboard
```bash
cd frontend/dashboard/

# Install dependencies
npm install

# Build for production
npm run build

# Serve with nginx or Apache
# Copy build/ contents to web server
```

### 3.2 Mobile App Deployment
```bash
cd frontend/mobile/

# Install dependencies
npm install

# Android build
npx react-native run-android --variant=release

# iOS build (requires Xcode)
npx react-native run-ios --configuration Release
```

## Phase 4: Field Deployment

### 4.1 Site Preparation
1. **Location Selection**
   - Wildlife corridor or feeding area
   - Clear line of sight for LoRa communication
   - Solar panel sun exposure (6+ hours daily)
   - Accessible for maintenance

2. **Mounting Setup**
   - Install weatherproof enclosure 2-3m above ground
   - Angle solar panel for optimal sun exposure
   - Secure against wind and animal interference
   - Install PIR sensor with clear detection zone

### 4.2 Network Configuration
```bash
# Configure mesh network
# Set each node with unique ID
#define MESH_NODE_ID 1
#define MESH_MASTER_NODE false

# Configure LoRa parameters
#define LORA_FREQUENCY 915E6    # US: 915MHz, EU: 868MHz
#define LORA_SPREADING_FACTOR 7
#define LORA_TX_POWER 20        # Max power for range
```

### 4.3 Power System Setup
1. **Battery Installation**
   - Install 18650 Li-ion batteries (3000mAh minimum)
   - Configure battery protection circuit
   - Test voltage levels (3.7V nominal)

2. **Solar Panel Connection**
   - Connect 5W+ mono-crystalline panel
   - Verify MPPT controller operation
   - Check charging voltage (4.2V for Li-ion)

3. **Power Validation**
   ```bash
   # Monitor power status via serial
   # Expected readings:
   # Battery: 3.7-4.2V
   # Solar: 5-18V (depending on conditions)
   # Charging: true (during daylight)
   # Daily harvest: 10-50Wh
   ```

### 4.4 Communication Testing
```bash
# Test LoRa communication
# Send test message every 30 seconds
# Verify message reception at base station
# Check RSSI values (-80 to -120 dBm acceptable)

# Test WiFi fallback
# Verify connection to backup WiFi
# Test API data transmission
```

## Phase 5: System Validation

### 5.1 AI Detection Testing
1. **Species Recognition**
   - Test with known wildlife species
   - Verify 95%+ accuracy rate
   - Check inference time <200ms
   - Validate confidence scoring

2. **False Positive Testing**
   - Test with moving vegetation
   - Verify <5% false positive rate
   - Adjust PIR sensitivity if needed

### 5.2 Power System Validation
```bash
# Monitor power consumption
# Typical readings:
# Idle: 50-100mA
# Camera active: 200-300mA
# AI processing: 400-500mA
# LoRa transmission: 100-150mA

# Battery life calculation
# 3000mAh battery / 100mA average = 30 hours
# With solar charging: 6+ months continuous
```

### 5.3 Data Pipeline Testing
1. **Real-time Detection**
   - Verify detections appear in dashboard <5 seconds
   - Check WebSocket connectivity
   - Test mobile push notifications

2. **Data Storage**
   - Verify InfluxDB data ingestion
   - Check image storage and retrieval
   - Test SQLite metadata accuracy

## Phase 6: Production Monitoring

### 6.1 Health Monitoring
- **System Metrics**
  - Battery voltage trending
  - Solar charging efficiency
  - Network connectivity uptime
  - AI processing performance
  - Memory usage and stability

### 6.2 Maintenance Schedule
- **Weekly**: Remote system health check
- **Monthly**: Battery voltage validation
- **Quarterly**: Firmware update deployment
- **Bi-annually**: Physical site inspection
- **Annually**: Complete system overhaul

### 6.3 Alert Configuration
```python
# Configure monitoring alerts
alerts = {
    'battery_low': {'threshold': 3.2, 'severity': 'critical'},
    'no_solar_charge': {'duration': '24h', 'severity': 'warning'},
    'communication_loss': {'duration': '1h', 'severity': 'critical'},
    'ai_processing_failure': {'count': 10, 'severity': 'warning'},
    'memory_usage_high': {'threshold': 90, 'severity': 'warning'}
}
```

## Troubleshooting Guide

### Common Issues

#### Firmware Upload Fails
```bash
# Reset ESP32 to bootloader mode
# Hold GPIO0 button while pressing reset
# Try lower upload speed
pio run -e esp32cam_advanced --target upload --upload-port /dev/ttyUSB0
```

#### AI Detection Not Working
```bash
# Check PSRAM availability
# Expected: 8MB PSRAM found
# Verify model loading
# Check confidence threshold settings
```

#### Power Issues
```bash
# Check battery voltage: 3.0-4.2V
# Verify solar panel voltage: 5-18V
# Test MPPT controller PWM output
# Check for loose connections
```

#### Network Connectivity
```bash
# WiFi: Check SSID and password
# LoRa: Verify frequency and spreading factor
# Mesh: Check node ID conflicts
# API: Test endpoint connectivity
```

## Security Considerations

### Production Security
1. **Change Default Passwords**
   - WiFi credentials
   - API authentication tokens
   - Database passwords

2. **Enable Encryption**
   - HTTPS for API endpoints
   - TLS for database connections
   - AES-256 for data storage

3. **Access Control**
   - Firewall rules
   - VPN access for management
   - Role-based API permissions

### Update Management
```bash
# Secure OTA updates
# Use signed firmware images
# Implement rollback capability
# Test updates in staging environment
```

## Support and Maintenance

For deployment support:
- **Documentation**: Check project wiki
- **Issues**: Submit GitHub issue
- **Community**: Join discussions forum
- **Professional**: Contact support team

---

**Deployment completed successfully? 🎉**

Your WildCAM ESP32 v2.0 system is now ready for professional wildlife monitoring!