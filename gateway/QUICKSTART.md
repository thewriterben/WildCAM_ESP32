# WildCAM Gateway - Quick Start Guide

This guide will get you up and running with the WildCAM Gateway in 5 minutes.

## Prerequisites

- Raspberry Pi 3/4 or Zero 2W with Raspberry Pi OS
- LoRa module (SX1276/SX1278 or SX1262) connected via SPI
- Internet connection (for initial setup)

## Quick Installation

```bash
# 1. Clone the repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/gateway

# 2. Run installation script
sudo ./install.sh

# 3. Configure your gateway
sudo nano /opt/wildcam-gateway/.env
# Edit LORA_FREQUENCY and other settings as needed

# 4. Enable and start the service
sudo systemctl enable wildcam-gateway
sudo systemctl start wildcam-gateway

# 5. Check status
sudo systemctl status wildcam-gateway
```

## Verify Installation

### Check Gateway Logs
```bash
sudo journalctl -u wildcam-gateway -f
```

### Access Web Dashboard
Open in browser:
```
http://raspberry-pi-ip:5000/dashboard.html
```

### Test API
```bash
# Health check
curl http://localhost:5000/api/health

# View nodes
curl http://localhost:5000/api/nodes

# View detections
curl http://localhost:5000/api/detections
```

## Testing Without LoRa Hardware

If you don't have LoRa hardware connected yet, you can test with the simulator:

```bash
cd /opt/wildcam-gateway

# Run simulator for 60 seconds
python3 simulate.py --duration 60 --nodes 3

# In another terminal, check the API
curl http://localhost:5000/api/nodes
curl http://localhost:5000/api/detections
```

## Quick Configuration

### Minimum Configuration (.env)
```bash
GATEWAY_ID=gateway_001
LORA_FREQUENCY=915.0    # US=915, EU=868, Asia=433
MQTT_ENABLED=true
MQTT_BROKER_HOST=localhost
API_ENABLED=true
```

### Start Local MQTT Broker
```bash
# Install Mosquitto
sudo apt-get install mosquitto mosquitto-clients

# Subscribe to all topics
mosquitto_sub -t 'wildcam/#' -v
```

## Common Issues

### SPI Not Working
```bash
# Enable SPI interface
sudo raspi-config
# Select: Interface Options -> SPI -> Enable
sudo reboot
```

### LoRa Module Not Detected
- Check wiring connections
- Verify GPIO pin numbers in configuration
- Check that frequency matches your module (915MHz vs 868MHz)

### API Not Accessible
```bash
# Check if service is running
sudo systemctl status wildcam-gateway

# Check firewall
sudo ufw allow 5000/tcp
```

## Next Steps

1. **Connect ESP32 Nodes**: Configure ESP32-CAM nodes with matching LoRa settings
2. **Setup MQTT**: Connect to cloud MQTT broker (AWS IoT, Azure IoT Hub)
3. **Enable Cloud Sync**: Configure cloud API endpoint for data upload
4. **Deploy Dashboard**: Serve dashboard on port 80 with nginx
5. **Monitor System**: Set up alerts and monitoring

## Documentation

- Full documentation: `GATEWAY_SETUP.md`
- API reference: See GATEWAY_SETUP.md "REST API Reference" section
- Configuration options: `.env.example`
- Troubleshooting: GATEWAY_SETUP.md "Troubleshooting" section

## Support

- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Main README: ../README.md

## Example Commands

```bash
# View live gateway stats
watch -n 2 'curl -s http://localhost:5000/api/stats | jq'

# Export detections to JSON
curl http://localhost:5000/api/detections?limit=100 > detections.json

# Check node battery levels
curl -s http://localhost:5000/api/nodes | jq '.nodes[] | {name: .node_name, battery: .battery_level}'

# Monitor MQTT messages
mosquitto_sub -t 'wildcam/#' -F '@Y-@m-@dT@H:@M:@S@z : %t : %p'
```

## Performance Tips

- For 10+ nodes: Increase database retention: `DB_RETENTION_DAYS=60`
- For remote deployments: Enable cloud sync: `CLOUD_SYNC_ENABLED=true`
- For debugging: Set log level: `LOG_LEVEL=DEBUG`
- For production: Disable API debug: `API_DEBUG=false`

---

**You're all set! 🎉** The gateway is now ready to receive data from ESP32-CAM wildlife monitoring nodes.
