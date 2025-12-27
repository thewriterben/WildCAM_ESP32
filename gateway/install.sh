#!/bin/bash
#
# WildCAM Gateway Installation Script for Raspberry Pi
# This script installs and configures the gateway service
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}WildCAM Gateway Installation${NC}"
echo -e "${GREEN}================================${NC}"
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}Please run as root (use sudo)${NC}"
    exit 1
fi

# Detect Raspberry Pi
if ! grep -q "Raspberry Pi" /proc/cpuinfo; then
    echo -e "${YELLOW}Warning: This doesn't appear to be a Raspberry Pi${NC}"
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Update system
echo -e "${GREEN}[1/7] Updating system packages...${NC}"
apt-get update
apt-get upgrade -y

# Install system dependencies
echo -e "${GREEN}[2/7] Installing system dependencies...${NC}"
apt-get install -y \
    python3 \
    python3-pip \
    python3-dev \
    python3-spidev \
    python3-rpi.gpio \
    git \
    sqlite3

# Enable SPI
echo -e "${GREEN}[3/7] Enabling SPI interface...${NC}"
if ! grep -q "^dtparam=spi=on" /boot/config.txt; then
    echo "dtparam=spi=on" >> /boot/config.txt
    echo -e "${YELLOW}SPI enabled. Reboot required.${NC}"
fi

# Create installation directory
INSTALL_DIR="/opt/wildcam-gateway"
echo -e "${GREEN}[4/7] Creating installation directory: ${INSTALL_DIR}${NC}"
mkdir -p ${INSTALL_DIR}
mkdir -p /var/log/wildcam-gateway
mkdir -p /var/lib/wildcam-gateway

# Copy gateway files
echo -e "${GREEN}[5/7] Installing gateway files...${NC}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cp ${SCRIPT_DIR}/*.py ${INSTALL_DIR}/
cp ${SCRIPT_DIR}/requirements.txt ${INSTALL_DIR}/
cp ${SCRIPT_DIR}/README.md ${INSTALL_DIR}/
chmod +x ${INSTALL_DIR}/gateway.py

# Install Python dependencies
echo -e "${GREEN}[6/7] Installing Python dependencies...${NC}"
cd ${INSTALL_DIR}
pip3 install -r requirements.txt

# Install systemd service
echo -e "${GREEN}[7/7] Installing systemd service...${NC}"
cp ${SCRIPT_DIR}/wildcam-gateway.service /etc/systemd/system/
systemctl daemon-reload

# Set permissions
chown -R pi:pi ${INSTALL_DIR}
chown -R pi:pi /var/log/wildcam-gateway
chown -R pi:pi /var/lib/wildcam-gateway

# Create environment file
ENV_FILE="${INSTALL_DIR}/.env"
if [ ! -f ${ENV_FILE} ]; then
    echo -e "${GREEN}Creating default environment configuration...${NC}"
    cat > ${ENV_FILE} << 'EOF'
# WildCAM Gateway Configuration
# Edit this file to customize your gateway settings

# Gateway Identification
GATEWAY_ID=gateway_001
GATEWAY_NAME=WildCAM Gateway
GATEWAY_LOCATION=

# LoRa Configuration
LORA_FREQUENCY=915.0
LORA_BANDWIDTH=125000
LORA_SPREADING_FACTOR=7
LORA_CODING_RATE=5
LORA_TX_POWER=17

# GPIO Pins (BCM numbering)
LORA_CS_PIN=8
LORA_RESET_PIN=25
LORA_DIO0_PIN=24

# MQTT Configuration
MQTT_ENABLED=true
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883
MQTT_USERNAME=
MQTT_PASSWORD=
MQTT_TOPIC_PREFIX=wildcam

# API Configuration
API_ENABLED=true
API_HOST=0.0.0.0
API_PORT=5000
API_DEBUG=false
API_KEY=

# Database Configuration
DB_PATH=/var/lib/wildcam-gateway/gateway_data.db
DB_RETENTION_DAYS=30

# Cloud Sync Configuration
CLOUD_SYNC_ENABLED=false
CLOUD_API_URL=
CLOUD_API_KEY=

# Logging
LOG_LEVEL=INFO
LOG_FILE=/var/log/wildcam-gateway/gateway.log
EOF
    chown pi:pi ${ENV_FILE}
    echo -e "${YELLOW}Edit ${ENV_FILE} to customize your configuration${NC}"
fi

# Installation complete
echo ""
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Installation Complete!${NC}"
echo -e "${GREEN}================================${NC}"
echo ""
echo -e "Gateway installed to: ${INSTALL_DIR}"
echo -e "Configuration file: ${ENV_FILE}"
echo -e "Log directory: /var/log/wildcam-gateway"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo -e "1. Edit configuration: sudo nano ${ENV_FILE}"
echo -e "2. Enable service: sudo systemctl enable wildcam-gateway"
echo -e "3. Start service: sudo systemctl start wildcam-gateway"
echo -e "4. Check status: sudo systemctl status wildcam-gateway"
echo -e "5. View logs: sudo journalctl -u wildcam-gateway -f"
echo ""

if grep -q "SPI enabled" /tmp/install.log 2>/dev/null || ! grep -q "^dtparam=spi=on" /boot/config.txt; then
    echo -e "${YELLOW}IMPORTANT: SPI interface was enabled. Please reboot before starting the gateway.${NC}"
    echo -e "Run: sudo reboot"
    echo ""
fi

echo -e "${GREEN}Gateway API will be available at: http://$(hostname -I | awk '{print $1}'):5000${NC}"
echo ""
