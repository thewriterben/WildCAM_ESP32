# LoRa Addon Board for ESP32WildlifeCAM

## Overview

This documentation describes the LoRa addon board design for the ESP32WildlifeCAM project. The addon board solves the GPIO pin conflicts between the AI-Thinker ESP32-CAM and LoRa modules by providing alternative pin mappings and board configurations.

## Problem Statement

The AI-Thinker ESP32-CAM has fundamental GPIO conflicts with standard LoRa modules:

- **GPIO 5**: LORA_CS conflicts with Y2_GPIO_NUM (camera data bit 2)
- **GPIO 18**: LORA_SCK conflicts with Y3_GPIO_NUM (camera data bit 3)  
- **GPIO 19**: LORA_MISO conflicts with Y4_GPIO_NUM (camera data bit 4)
- **GPIO 23**: LORA_MOSI conflicts with HREF_GPIO_NUM (camera horizontal reference)
- **GPIO 26**: LORA_DIO0 conflicts with SIOD_GPIO_NUM (camera I2C data)

## Solutions

### Solution 1: Alternative GPIO Mapping (Recommended)

Use alternative GPIO pins available on the ESP32-CAM:

```
LoRa Module Pin    ESP32-CAM GPIO    Function
SCLK              GPIO 12           SPI Clock (alternative)
MISO              GPIO 13           SPI MISO (alternative) 
MOSI              GPIO 15           SPI MOSI (alternative)
CS                GPIO 14           Chip Select (alternative)
RST               GPIO 2            Reset (alternative)
DIO0              GPIO 4            Interrupt (alternative)
DIO1              GPIO 16           Interrupt (optional)
```

**Advantages:**
- No hardware modifications required
- Uses available ESP32-CAM pins
- Compatible with standard LoRa modules

**Disadvantages:**
- Conflicts with SD card functionality
- Requires careful pin management

### Solution 2: ESP32-S3-CAM Upgrade

Switch to ESP32-S3-CAM which has more available GPIO pins:

```
LoRa Module Pin    ESP32-S3 GPIO    Function
SCLK              GPIO 18           SPI Clock (standard)
MISO              GPIO 19           SPI MISO (standard)
MOSI              GPIO 23           SPI MOSI (standard)  
CS                GPIO 5            Chip Select (standard)
RST               GPIO 14           Reset (standard)
DIO0              GPIO 26           Interrupt (standard)
DIO1              GPIO 27           Interrupt
DIO2              GPIO 33           Interrupt
```

**Advantages:**
- No pin conflicts
- Standard LoRa pin mapping
- Better performance and more memory
- Future-proof design

**Disadvantages:**
- Requires hardware upgrade
- Higher cost
- Code compatibility considerations

### Solution 3: Custom Breakout Board

Design a custom breakout board that routes signals appropriately:

**Board Features:**
- ESP32-CAM socket
- LoRa module socket (SX1276/SX1262/SX1280)
- Alternative pin routing
- Power management
- External antenna connector
- Status LEDs

## Supported LoRa Modules

### 1. SX1276/RFM95 (Sub-GHz)
- **Frequency**: 433/868/915 MHz
- **Range**: Up to 15 km (line of sight)
- **Data Rate**: 0.018 - 37.5 kbps
- **Power**: 2-20 dBm
- **Current**: 10-120 mA (transmit), 10 mA (receive)

### 2. SX1262/SX1268 (Sub-GHz)
- **Frequency**: 150-960 MHz  
- **Range**: Up to 20 km (line of sight)
- **Data Rate**: 0.061 - 62.5 kbps
- **Power**: -9 to +22 dBm
- **Current**: 15-124 mA (transmit), 4.2 mA (receive)

### 3. SX1280 (2.4 GHz)
- **Frequency**: 2400-2500 MHz
- **Range**: Up to 12 km (line of sight)  
- **Data Rate**: 0.8 - 203 kbps
- **Power**: -18 to +12.5 dBm
- **Current**: 12.9-16.8 mA (transmit), 3.9 mA (receive)

### 4. E22-900M30S (High Power)
- **Frequency**: 850-930 MHz
- **Range**: Up to 30 km (line of sight)
- **Data Rate**: 0.3 - 19.2 kbps  
- **Power**: 10-30 dBm
- **Current**: 120-900 mA (transmit), 16 mA (receive)

## Regional Configurations

### United States (915 MHz)
```cpp
#define LORA_REGION REGION_US915
#define LORA_FREQUENCY 915E6
#define LORA_BANDWIDTH 125E3
#define LORA_MAX_POWER 30      // 30 dBm max
#define LORA_DUTY_CYCLE_LIMIT 100  // No limit
```

### Europe (868 MHz)
```cpp
#define LORA_REGION REGION_EU868  
#define LORA_FREQUENCY 868E6
#define LORA_BANDWIDTH 125E3
#define LORA_MAX_POWER 14      // 14 dBm max
#define LORA_DUTY_CYCLE_LIMIT 1    // 1% duty cycle
```

### Europe (433 MHz)
```cpp
#define LORA_REGION REGION_EU433
#define LORA_FREQUENCY 433E6
#define LORA_BANDWIDTH 125E3  
#define LORA_MAX_POWER 10      // 10 dBm max
#define LORA_DUTY_CYCLE_LIMIT 10   // 10% duty cycle
```

### Australia (915 MHz)
```cpp
#define LORA_REGION REGION_AU915
#define LORA_FREQUENCY 915E6
#define LORA_BANDWIDTH 125E3
#define LORA_MAX_POWER 30      // 30 dBm max  
#define LORA_DUTY_CYCLE_LIMIT 100  // No limit
```

### China (470 MHz)
```cpp
#define LORA_REGION REGION_CN470
#define LORA_FREQUENCY 470E6
#define LORA_BANDWIDTH 125E3
#define LORA_MAX_POWER 17      // 17 dBm max
#define LORA_DUTY_CYCLE_LIMIT 100  // No limit
```

### Japan (920 MHz)
```cpp
#define LORA_REGION REGION_JP920
#define LORA_FREQUENCY 920E6  
#define LORA_BANDWIDTH 125E3
#define LORA_MAX_POWER 13      // 13 dBm max
#define LORA_DUTY_CYCLE_LIMIT 100  // No limit
```

## Power Considerations

### Power Consumption by Mode

| Mode | SX1276 | SX1262 | SX1280 | E22-900M30S |
|------|--------|--------|--------|-------------|
| Sleep | 0.2 µA | 0.16 µA | 0.25 µA | 2 µA |
| Standby | 1.5 µA | 0.8 µA | 1.25 µA | 5 µA |
| Receive | 10.3 mA | 4.2 mA | 3.9 mA | 16 mA |
| Transmit (max) | 120 mA | 124 mA | 16.8 mA | 900 mA |

### Solar Power Requirements

For continuous operation with solar power:

- **Minimum Solar Panel**: 5W (cloudy conditions)
- **Recommended Solar Panel**: 10-20W (reliable operation)
- **Battery Capacity**: 3000-5000 mAh (3+ days autonomy)
- **Charge Controller**: MPPT preferred for efficiency

### Power Management Strategy

1. **Deep Sleep**: Use ESP32 deep sleep between captures
2. **LoRa Sleep**: Put LoRa module in sleep mode when not transmitting
3. **Adaptive Power**: Reduce TX power based on signal quality
4. **Duty Cycle**: Respect regional duty cycle limitations
5. **Solar Optimization**: Transmit during peak solar hours when possible

## Antenna Selection

### Antenna Types

1. **Wire Antenna**: Simple, low cost, omnidirectional
   - Length: λ/4 (e.g., 8.2 cm for 915 MHz)
   - Gain: 2-3 dBi
   - Cost: $1-5

2. **PCB Antenna**: Compact, integrated design
   - Gain: 1-2 dBi  
   - Cost: Integrated
   - Range: Limited

3. **External Whip**: Better performance, weatherproof
   - Gain: 3-5 dBi
   - Cost: $10-30
   - Range: Good

4. **Yagi/Directional**: Maximum range, directional
   - Gain: 6-15 dBi
   - Cost: $20-100
   - Range: Excellent (line of sight)

### Antenna Placement

- **Height**: Higher is better (line of sight)
- **Orientation**: Vertical for omnidirectional coverage
- **Ground Plane**: Required for optimal performance
- **Weatherproofing**: Essential for outdoor deployment
- **Lightning Protection**: Consider surge protectors

## Installation Guide

### Hardware Assembly

1. **Prepare ESP32-CAM**:
   - Flash with ESP32WildlifeCAM firmware
   - Configure alternative GPIO pins
   - Test camera functionality

2. **Connect LoRa Module**:
   - Wire according to chosen GPIO mapping
   - Use short, twisted wires to minimize interference
   - Add bypass capacitors (100nF ceramic + 10µF electrolytic)

3. **Install Antenna**:
   - Connect to LoRa module antenna pin
   - Ensure proper impedance matching (50Ω)
   - Weatherproof all connections

4. **Power System**:
   - Connect solar panel to charge controller
   - Connect battery to charge controller
   - Connect ESP32-CAM to regulated power output

### Software Configuration

1. **Set Region**:
   ```cpp
   #define LORA_REGION REGION_US915  // Change for your region
   ```

2. **Configure GPIO Pins**:
   ```cpp
   #define LORA_SPI_SCLK   12
   #define LORA_SPI_MISO   13  
   #define LORA_SPI_MOSI   15
   #define LORA_CS         14
   #define LORA_RST        2
   #define LORA_DIO0       4
   ```

3. **Set Channel Preset**:
   ```cpp
   #define MESH_CHANNEL_PRESET CHANNEL_LONG_SLOW  // Maximum range
   ```

4. **Configure Node ID**:
   ```cpp
   #define MESH_NODE_ID_AUTO true  // Auto-generate from MAC
   ```

### Range Testing

1. **Line of Sight Test**:
   - Start with nodes 100m apart
   - Gradually increase distance
   - Note signal strength (RSSI/SNR)
   - Test different antenna orientations

2. **Obstacle Testing**:
   - Test through trees, buildings, terrain
   - Note signal degradation
   - Adjust power levels accordingly

3. **Network Testing**:
   - Deploy multiple nodes
   - Test mesh routing functionality  
   - Verify message delivery
   - Monitor packet loss rates

## Troubleshooting

### Common Issues

1. **No LoRa Communication**:
   - Check wiring connections
   - Verify GPIO pin configuration
   - Test with different spreading factors
   - Check antenna connection

2. **Short Range**:
   - Increase TX power (within legal limits)
   - Improve antenna placement
   - Check for interference sources
   - Verify antenna tuning

3. **High Power Consumption**:
   - Enable sleep modes
   - Reduce TX power
   - Optimize transmission intervals
   - Check for stuck transmit mode

4. **Mesh Routing Issues**:
   - Verify node IDs are unique
   - Check hop limits
   - Monitor routing table updates
   - Test with fewer nodes first

### Debug Commands

1. **Radio Status**:
   ```cpp
   radioDriver->printConfiguration();
   radioDriver->printStatistics();
   ```

2. **Mesh Status**:
   ```cpp
   meshInterface->getStatistics();
   meshInterface->getNetworkTopology();
   ```

3. **Signal Quality**:
   ```cpp
   int rssi = radioDriver->getRssi();
   float snr = radioDriver->getSnr();
   ```

## Legal Considerations

### Regulatory Compliance

1. **Frequency Bands**: Use only authorized frequencies in your region
2. **Power Limits**: Respect maximum power limitations
3. **Duty Cycle**: Follow duty cycle restrictions (EU)
4. **Licensing**: Some regions require amateur radio licenses
5. **Type Approval**: Commercial deployments may need certification

### Best Practices

1. **Good Neighbor Policy**: Minimize interference
2. **Emergency Services**: Don't interfere with emergency frequencies
3. **Documentation**: Keep records of deployments
4. **Monitoring**: Be aware of local spectrum usage
5. **Updates**: Stay informed of regulatory changes

## Bill of Materials

### Basic Kit (Alternative GPIO)
- ESP32-CAM module: $8
- SX1276/RFM95 LoRa module: $12
- Wire antenna: $3
- Prototype board: $5
- Connectors and wire: $5
- **Total**: ~$33

### Professional Kit (ESP32-S3)
- ESP32-S3-CAM module: $15
- SX1262 LoRa module: $18
- External whip antenna: $25
- Custom PCB: $15
- Enclosure: $20
- **Total**: ~$93

### High-Power Kit (Long Range)
- ESP32-S3-CAM module: $15
- E22-900M30S module: $35
- Yagi antenna: $60
- RF amplifier: $40
- Power supply upgrade: $25
- **Total**: ~$175

## Future Enhancements

1. **Mesh Topology Optimization**: Dynamic routing algorithms
2. **AI Integration**: Intelligent power management
3. **Multi-Band Operation**: Frequency hopping capabilities
4. **Satellite Integration**: Backup communication via satellite
5. **Edge Computing**: Local image processing and analysis

## Support and Resources

- **Documentation**: See `/docs/` directory
- **Examples**: See `/examples/` directory  
- **Community**: GitHub discussions and issues
- **Updates**: Check releases for firmware updates
- **Hardware**: PCB files and schematics available