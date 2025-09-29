# Satellite Communication Module Documentation

## Overview

The ESP32 Wildlife Camera satellite communication module enables remote data transmission and monitoring in areas without cellular or WiFi coverage. It supports multiple satellite communication platforms including Iridium, Swarm, and RockBLOCK modules.

## Supported Satellite Modules

### 1. Iridium 9603N / RockBLOCK
- **Cost**: ~$0.95 per message
- **Message Size**: 160 bytes maximum
- **Coverage**: Global
- **Power**: ~2.8W during transmission
- **Best For**: Emergency alerts and critical status updates

### 2. Swarm M138
- **Cost**: ~$0.05 per message
- **Message Size**: 192 bytes maximum
- **Coverage**: Global (expanding)
- **Power**: ~1.5W during transmission
- **Best For**: Regular monitoring and data collection

## Hardware Wiring

### Iridium 9603N / RockBLOCK Wiring
```
ESP32-CAM    →    Iridium Module
GPIO 25      →    RX
GPIO 26      →    TX
GPIO 27      →    SLEEP
GPIO 33      →    RING
3.3V         →    VCC
GND          →    GND
```

### Swarm M138 Wiring
```
ESP32-CAM    →    Swarm Module
GPIO 25      →    RX
GPIO 26      →    TX
GPIO 27      →    SLEEP/ENABLE
3.3V         →    VCC
GND          →    GND
```

## Configuration

### Basic Configuration
```cpp
#include "satellite_comm.h"
#include "satellite_config.h"

SatelliteComm satComm;
SatelliteConfig config;

void setup() {
    // Configure satellite module
    config.enabled = true;
    config.module = MODULE_IRIDIUM;  // or MODULE_SWARM
    config.transmissionInterval = 3600;  // 1 hour
    config.maxDailyMessages = 24;
    config.maxDailyCost = 25.0;  // USD
    config.emergencyThreshold = 90;  // Battery %
    config.costOptimization = true;
    
    satComm.configure(config);
    satComm.initialize(config.module);
}
```

### Advanced Configuration Options

#### Transmission Intervals
- `transmissionInterval`: Regular transmission interval in seconds (default: 3600)
- `emergencyInterval`: Emergency transmission interval in seconds (default: 300)

#### Cost Management
- `maxDailyMessages`: Maximum messages per day (default: 50)
- `maxDailyCost`: Maximum daily cost in USD (default: $25.00)
- `costOptimization`: Enable cost-aware transmission scheduling

#### Power Management
- `emergencyThreshold`: Battery percentage for emergency mode (default: 90%)
- `useScheduledTransmission`: Use satellite pass prediction for optimal timing

## Usage Examples

### Sending Wildlife Alerts
```cpp
// Send high-priority wildlife detection alert
satComm.sendWildlifeAlert("SPECIES:DEER,COUNT:3,CONFIDENCE:0.85");

// Send emergency alert for system issues
satComm.sendEmergencyAlert("LOW_BATTERY:15%,SOLAR_FAULT");
```

### Sending Thumbnails
```cpp
// Capture and send compressed thumbnail
uint8_t thumbnailData[140];
size_t thumbnailSize = createThumbnail(imageData, thumbnailData);
satComm.sendThumbnail(thumbnailData, thumbnailSize);
```

### Status Reporting
```cpp
// Send automated status report
satComm.sendStatusReport();  // Includes battery, message count, signal quality
```

### Receiving Configuration Updates
```cpp
// Check for incoming configuration updates
if (satComm.receiveConfigUpdate()) {
    Serial.println("Configuration updated remotely");
}
```

## Message Priorities

1. **EMERGENCY**: Immediate transmission regardless of cost
2. **HIGH**: Transmit during next available satellite pass
3. **NORMAL**: Transmit during optimal cost/timing windows
4. **LOW**: Transmit only during optimal conditions

## Power Consumption Optimization

### Scheduled Transmission Windows
The module automatically predicts satellite passes and schedules transmissions during optimal windows to minimize power consumption and costs.

### Low Power Mode
When battery levels drop below the configured threshold:
- Transmission intervals are doubled
- Only emergency and high-priority messages are sent immediately
- Normal/low priority messages are queued for better conditions

### Sleep Mode Management
```cpp
// Enter sleep mode to conserve power
satComm.enterSleepMode();

// Wake up for transmission
satComm.exitSleepMode();
```

## Data Compression

### Automatic Message Compression
Messages are automatically compressed to fit within satellite message limits:
- Removes unnecessary spaces
- Uses abbreviations for common terms
- Optimizes for satellite transmission constraints

### Thumbnail Optimization
```cpp
// Optimize image data for satellite transmission
uint8_t optimizedData[160];
size_t optimizedSize = dataCompression.optimizeForSatellite(
    imageData, imageSize, optimizedData, sizeof(optimizedData)
);
```

## Fallback Mechanisms

### Local Storage Buffering
When satellite communication is unavailable:
- Messages are stored locally with priority levels
- Automatic retry with exponential backoff
- Transmission during next available window

### Retry Logic
- Base delay: 30 seconds
- Exponential backoff with 2x multiplier
- Maximum delay: 30 minutes
- Maximum attempts: 5

## Cost Considerations

### Daily Cost Monitoring
```cpp
float todayCost = satComm.getTodayCost();
bool withinBudget = satComm.isWithinCostLimit();
```

### Message Costs (Approximate)
- **Iridium/RockBLOCK**: $0.95 per message
- **Swarm**: $0.05 per message

### Cost Optimization Tips
1. Enable message compression
2. Use scheduled transmission windows
3. Set appropriate daily message limits
4. Prioritize emergency messages only when necessary
5. Use thumbnail summaries instead of full images

## Troubleshooting

### Common Issues

#### No Satellite Signal
- Check antenna orientation (should point skyward)
- Ensure clear view of sky (no buildings/trees blocking)
- Verify module wiring and power supply
- Check signal quality: `satComm.checkSignalQuality()`

#### High Costs
- Review daily message limits
- Enable cost optimization
- Use lower priority for routine messages
- Implement message compression

#### Power Issues
- Monitor battery levels during transmission
- Use scheduled transmission windows
- Enable low power mode for battery conservation
- Check solar panel functionality

### Debug Commands
```cpp
// Check satellite availability
bool available = satComm.checkSatelliteAvailability();

// Get signal quality (0-5 scale)
int signal = satComm.checkSignalQuality();

// Check stored message count
size_t stored = satComm.getStoredMessageCount();

// Get cost information
float cost = satComm.getTodayCost();
```

## Environmental Considerations

### Operating Conditions
- Temperature: -40°C to +85°C
- Humidity: 0-100% RH
- Altitude: Sea level to 50,000 feet
- Weather: Fully waterproof enclosure recommended

### Antenna Placement
- Mount antenna with clear sky view
- Avoid metallic obstructions
- Consider seasonal foliage changes
- Use appropriate RF cable length

## Integration with Wildlife Camera System

### Main Loop Integration
```cpp
void loop() {
    // Regular camera operations
    handleMotionDetection();
    
    // Periodic satellite communication
    if (millis() - lastSatCheck > config.transmissionInterval * 1000) {
        satComm.sendStoredMessages();
        lastSatCheck = millis();
    }
    
    // Emergency battery alert
    if (batteryLevel < config.emergencyThreshold && !emergencyAlertSent) {
        satComm.sendEmergencyAlert("LOW_BATTERY:" + String(batteryLevel) + "%");
        emergencyAlertSent = true;
    }
}
```

## Future Enhancements

1. **Enhanced Pass Prediction**: Integration with real-time satellite orbital data
2. **Mesh Integration**: Satellite as backup for LoRa mesh networks
3. **Image Compression**: Advanced JPEG compression for larger thumbnails
4. **Bi-directional Communication**: Full command and control capabilities
5. **Multi-Module Support**: Automatic failover between different satellite providers

## Support and Resources

- **Iridium Documentation**: [Iridium Developer Resources](https://www.iridium.com/)
- **Swarm Documentation**: [Swarm Developer Portal](https://swarm.space/)
- **RockBLOCK Documentation**: [Rock7 Mobile](https://www.rock7.com/)
- **ESP32 Wildlife Camera Project**: [GitHub Repository](https://github.com/thewriterben/ESP32WildlifeCAM)