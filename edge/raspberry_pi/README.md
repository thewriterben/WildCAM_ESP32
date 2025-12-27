# Raspberry Pi LoRa Gateway - Setup Guide

Complete setup and deployment guide for the WildCAM Raspberry Pi LoRa Gateway.

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Prerequisites](#software-prerequisites)
- [LoRa Module Setup](#lora-module-setup)
- [Installation](#installation)
- [Configuration](#configuration)
- [Running the Service](#running-the-service)
- [MQTT Setup](#mqtt-setup)
- [Monitoring and Maintenance](#monitoring-and-maintenance)
- [Troubleshooting](#troubleshooting)

## Hardware Requirements

### Raspberry Pi

**Supported Models:**
- Raspberry Pi 3 Model B+ (minimum)
- Raspberry Pi 4 Model B (recommended)
- Raspberry Pi Zero 2 W (compact deployments)

**Recommended Specs:**
- 2GB RAM minimum, 4GB+ recommended
- 32GB SD Card (Class 10)
- Power supply with 5V 3A

### LoRa Module

**Supported LoRa Modules:**

| Module | Frequency | Range | Interface |
|--------|-----------|-------|-----------|
| RFM95W | 915 MHz | 2-5 km | SPI |
| RFM96W | 433 MHz | 3-10 km | SPI |
| SX1276 | 868 MHz | 2-5 km | SPI |
| SX1262 | 915 MHz | 5-15 km | SPI |

**Wiring Diagram (SX1276/RFM95):**

```
LoRa Module    Raspberry Pi
-----------    -------------
VCC       ->   3.3V (Pin 1)
GND       ->   GND (Pin 6)
MISO      ->   GPIO 9 (Pin 21)
MOSI      ->   GPIO 10 (Pin 19)
SCK       ->   GPIO 11 (Pin 23)
NSS/CS    ->   GPIO 8 (Pin 24)
RESET     ->   GPIO 25 (Pin 22)
DIO0      ->   GPIO 24 (Pin 18)
```

### Optional Components

- External antenna (improves range by 3-5x)
- GPS module (for gateway location tracking)
- Real-time clock (RTC) module (for offline time keeping)
- Weatherproof enclosure for outdoor deployment

## Software Prerequisites

### Raspberry Pi OS

```bash
# Check OS version
cat /etc/os-release

# Update system
sudo apt update
sudo apt upgrade -y
```

**Recommended**: Raspberry Pi OS Lite (64-bit) for headless operation.

### Enable SPI

LoRa modules use SPI interface which must be enabled:

```bash
# Enable SPI
sudo raspi-config
# Interface Options -> SPI -> Enable

# Verify SPI enabled
ls /dev/spidev*
# Should show: /dev/spidev0.0 /dev/spidev0.1

# Add user to SPI group
sudo usermod -a -G spi,gpio $USER

# Reboot
sudo reboot
```

### Python Environment

```bash
# Install Python and pip
sudo apt install -y python3 python3-pip python3-venv

# Install system dependencies
sudo apt install -y \
    python3-dev \
    libgpiod2 \
    gcc \
    g++ \
    make

# Create virtual environment (optional)
python3 -m venv ~/wildcam-env
source ~/wildcam-env/bin/activate
```

## LoRa Module Setup

### Install LoRa Library

For **SX1276/RFM95** modules:

```bash
# Install pySX127x
pip3 install pyLoRa
pip3 install RPi.GPIO spidev
```

For **SX1262** modules:

```bash
# Install SX126x library
pip3 install sx126x
```

### Test LoRa Module

```python
# test_lora.py
from SX127x.LoRa import LoRa
from SX127x.board_config import BOARD

BOARD.setup()

lora = LoRa()
lora.set_mode(MODE.STDBY)
lora.set_freq(915.0)  # MHz
lora.set_pa_config(pa_select=1)

print("LoRa module initialized successfully!")
print(f"Frequency: {lora.get_freq()} MHz")

BOARD.teardown()
```

```bash
python3 test_lora.py
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
cd raspberry_pi/
pip3 install -r requirements.txt
```

### 3. Create Directories

```bash
# Create data directories
sudo mkdir -p /data/lora
sudo mkdir -p /var/log/wildcam
sudo mkdir -p /etc/wildcam/rpi

# Set permissions
sudo chown -R $USER:$USER /data /var/log/wildcam
```

## Configuration

### 1. Copy Configuration Files

```bash
# Copy configuration
sudo cp -r config/* /etc/wildcam/rpi/

# Or create symbolic link
sudo ln -s $(pwd)/config /etc/wildcam/rpi
```

### 2. Configure LoRa Settings

Edit `/etc/wildcam/rpi/production.yaml`:

```yaml
lora:
  frequency: 915.0  # MHz (915 for US, 868 for EU, 433 for Asia)
  spreading_factor: 7  # 7-12 (higher = longer range, slower)
  bandwidth: 125000  # Hz (125kHz standard)
  coding_rate: 5  # 5-8
  tx_power: 17  # dBm (adjust for regulatory limits)
  use_sx1262: false  # Set true for SX1262 modules
  db_path: /data/lora/gateway.db
```

**Regional Frequency Settings:**

| Region | Frequency | Notes |
|--------|-----------|-------|
| North America | 915.0 MHz | ISM band |
| Europe | 868.0 MHz | ISM band |
| Asia | 433.0 MHz | ISM band |
| Australia | 915.0 MHz | ISM band |

### 3. Configure MQTT

Edit MQTT section in `/etc/wildcam/rpi/production.yaml`:

```yaml
mqtt:
  broker_host: localhost  # Or cloud MQTT broker
  broker_port: 1883
  username: ${MQTT_USERNAME}
  password: ${MQTT_PASSWORD}
  client_id: wildcam_rpi_gateway
  use_tls: false
  base_topic: wildcam/lora
```

### 4. Environment Variables

```bash
# Create .env file
cat > /etc/wildcam/rpi/.env << EOF
MQTT_USERNAME=wildcam
MQTT_PASSWORD=your_secure_password
EOF

# Secure permissions
chmod 600 /etc/wildcam/rpi/.env
```

## Running the Service

### Development Mode

```bash
cd ~/WildCAM_ESP32/edge

# Run with development config
python3 -m edge.raspberry_pi.gateway.main \
    --config /etc/wildcam/rpi \
    --env development
```

### Production Mode

#### Systemd Service (Recommended)

Create `/etc/systemd/system/wildcam-rpi-gateway.service`:

```ini
[Unit]
Description=WildCAM Raspberry Pi LoRa Gateway
After=network.target

[Service]
Type=simple
User=wildcam
WorkingDirectory=/home/wildcam/WildCAM_ESP32/edge
Environment="PYTHONUNBUFFERED=1"
ExecStart=/usr/bin/python3 -m edge.raspberry_pi.gateway.main --config /etc/wildcam/rpi --env production
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable wildcam-rpi-gateway
sudo systemctl start wildcam-rpi-gateway

# Check status
sudo systemctl status wildcam-rpi-gateway

# View logs
sudo journalctl -u wildcam-rpi-gateway -f
```

#### Docker Deployment

```bash
cd ~/WildCAM_ESP32/edge

# Build image
docker build -t wildcam-rpi -f raspberry_pi/Dockerfile ..

# Run container (with GPIO access)
docker run -d \
    --name wildcam-rpi-gateway \
    --privileged \
    --restart unless-stopped \
    -p 5000:5000 \
    -v /data:/data \
    -v /etc/wildcam/rpi:/etc/wildcam/rpi:ro \
    -v /dev/gpiomem:/dev/gpiomem \
    -v /dev/spidev0.0:/dev/spidev0.0 \
    -e MQTT_USERNAME=wildcam \
    -e MQTT_PASSWORD=your_password \
    wildcam-rpi

# View logs
docker logs -f wildcam-rpi-gateway
```

## MQTT Setup

### Local MQTT Broker (Mosquitto)

Install and configure Mosquitto:

```bash
# Install Mosquitto
sudo apt install -y mosquitto mosquitto-clients

# Enable and start
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Configure authentication
sudo mosquitto_passwd -c /etc/mosquitto/passwd wildcam

# Edit /etc/mosquitto/mosquitto.conf
sudo nano /etc/mosquitto/mosquitto.conf
```

Add to `mosquitto.conf`:

```
listener 1883
allow_anonymous false
password_file /etc/mosquitto/passwd
```

Restart Mosquitto:

```bash
sudo systemctl restart mosquitto
```

### Cloud MQTT (AWS IoT)

For AWS IoT Core integration:

```yaml
mqtt:
  broker_host: xxxxx.iot.us-east-1.amazonaws.com
  broker_port: 8883
  use_tls: true
  cert_path: /etc/wildcam/certs/
  # Place certificates in cert_path:
  # - AmazonRootCA1.pem
  # - certificate.pem.crt
  # - private.pem.key
```

### Test MQTT

```bash
# Subscribe to all topics
mosquitto_sub -h localhost -u wildcam -P your_password -t '#' -v

# Publish test message
mosquitto_pub -h localhost -u wildcam -P your_password \
    -t 'wildcam/lora/test' -m 'Hello from gateway'
```

## Monitoring and Maintenance

### Health Checks

```bash
# Check API health
curl http://localhost:5000/health

# Get mesh nodes
curl http://localhost:5000/api/nodes

# Get recent detections
curl http://localhost:5000/api/detections?limit=10

# Get gateway stats
curl http://localhost:5000/api/stats
```

### Log Monitoring

```bash
# Real-time logs
tail -f /var/log/wildcam/rpi-gateway.log

# Search for errors
grep ERROR /var/log/wildcam/rpi-gateway.log

# With systemd
journalctl -u wildcam-rpi-gateway -f
```

### Database Maintenance

```bash
# Check database size
du -h /data/lora/gateway.db

# Count packets
sqlite3 /data/lora/gateway.db "SELECT COUNT(*) FROM packets;"

# Clean old packets (30+ days)
sqlite3 /data/lora/gateway.db \
    "DELETE FROM packets WHERE timestamp < datetime('now', '-30 days');"

# Vacuum database
sqlite3 /data/lora/gateway.db "VACUUM;"
```

### Performance Monitoring

```bash
# CPU and memory
htop

# System resources
vcgencmd measure_temp  # Temperature
vcgencmd measure_volts  # Voltage
vcgencmd get_mem arm    # Memory split

# Network traffic
sudo iftop -i wlan0
```

## Troubleshooting

### Issue: LoRa Module Not Detected

```bash
# Check SPI enabled
ls /dev/spidev*

# Check GPIO permissions
groups | grep gpio

# Test SPI communication
ls -l /dev/spidev0.0
# Should show crw-rw----

# Re-enable SPI
sudo raspi-config
# Interface Options -> SPI -> Enable
sudo reboot
```

### Issue: No Packets Received

```bash
# Check LoRa configuration matches ESP32
# Frequency, spreading factor, bandwidth must match

# Test with LoRa receiver script
python3 test_lora_receive.py

# Check antenna connection
# Verify antenna is properly connected to module

# Verify ESP32 nodes are transmitting
# Check ESP32 serial output for transmission confirmation
```

### Issue: MQTT Connection Failed

```bash
# Test MQTT broker
mosquitto_sub -h localhost -t test

# Check Mosquitto running
sudo systemctl status mosquitto

# View Mosquitto logs
sudo tail -f /var/log/mosquitto/mosquitto.log

# Test credentials
mosquitto_pub -h localhost -u wildcam -P password -t test -m test
```

### Issue: High Packet Loss

```bash
# Increase LoRa spreading factor for better reliability
# Edit /etc/wildcam/rpi/production.yaml
# lora.spreading_factor: 9  # Increase from 7

# Reduce transmission distance
# Add repeater nodes between ESP32 and gateway

# Check for interference
# Use spectrum analyzer or try different frequency
```

### Issue: Database Growing Too Large

```bash
# Enable automatic cleanup
# Edit /etc/wildcam/rpi/production.yaml
retention:
  packets_days: 7      # Keep only 7 days
  detections_days: 30
  telemetry_days: 14

# Manual cleanup
sqlite3 /data/lora/gateway.db << EOF
DELETE FROM packets WHERE timestamp < datetime('now', '-7 days');
DELETE FROM telemetry WHERE timestamp < datetime('now', '-14 days');
VACUUM;
EOF
```

## Advanced Configuration

### Range Optimization

For maximum range:

```yaml
lora:
  frequency: 915.0
  spreading_factor: 12  # Maximum range
  bandwidth: 125000     # Narrower = better sensitivity
  coding_rate: 8        # Maximum error correction
  tx_power: 20          # Maximum (check regulations)
```

**Note**: Higher spreading factor = longer range but slower data rate.

### Multi-Gateway Setup

Deploy multiple gateways for coverage:

```yaml
# Gateway 1
mqtt:
  client_id: wildcam_gateway_001
  base_topic: wildcam/lora/gw001

# Gateway 2
mqtt:
  client_id: wildcam_gateway_002
  base_topic: wildcam/lora/gw002
```

### GPS Integration

Add GPS for gateway location:

```bash
# Install GPS daemon
sudo apt install -y gpsd gpsd-clients

# Configure GPS
sudo nano /etc/default/gpsd
# Add: DEVICES="/dev/ttyAMA0"

# Test GPS
cgps -s
```

## Performance Benchmarks

| Configuration | Packets/min | Range | Power |
|--------------|-------------|-------|-------|
| SF7, BW125 | 120 | 2 km | 1.5W |
| SF9, BW125 | 60 | 5 km | 1.5W |
| SF12, BW125 | 15 | 10 km | 1.5W |

## Support

For additional help:
- Main Documentation: [Edge Computing README](../README.md)
- Architecture Guide: [../../docs/EDGE_COMPUTING_ARCHITECTURE.md](../../docs/EDGE_COMPUTING_ARCHITECTURE.md)
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
