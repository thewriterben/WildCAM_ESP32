# 🔧 ESP32 Wildlife CAM - Hardware Requirements & Shopping List

## Overview
This document provides comprehensive hardware requirements for the ESP32 Wildlife CAM project, including shopping lists, compatibility matrices, and procurement guidelines for different deployment scenarios.

## 🎯 Core Hardware Components

### Primary ESP32 Camera Module
| Component | Specification | Quantity | Estimated Cost | Notes |
|-----------|---------------|----------|----------------|-------|
| **AI-Thinker ESP32-CAM** | ESP32-S with OV2640 | 1 | $8-12 | Primary recommendation |
| **ESP32-S3-CAM** | ESP32-S3 with OV2640 | 1 | $15-20 | Alternative for networking |
| **TTGO T-Camera** | ESP32 with OV2640 | 1 | $25-30 | Premium option |

### Camera Sensors (Compatible Options)
| Sensor | Resolution | Field of View | Low Light | Wildlife Suitability | Cost |
|--------|------------|---------------|-----------|---------------------|------|
| **OV2640** | 2MP (1600x1200) | 66° | Good | ⭐⭐⭐⭐ | $3-5 |
| **OV5640** | 5MP (2592x1944) | 62° | Excellent | ⭐⭐⭐⭐⭐ | $8-12 |
| **OV3660** | 3MP (2048x1536) | 68° | Very Good | ⭐⭐⭐⭐ | $6-8 |

### Power System
| Component | Specification | Quantity | Purpose | Cost |
|-----------|---------------|----------|---------|------|
| **Solar Panel** | 6V 1W Mini | 1 | Primary power | $8-15 |
| **LiPo Battery** | 3.7V 2000mAh | 1 | Energy storage | $10-15 |
| **TP4056 Module** | Charging controller | 1 | Battery management | $2-3 |
| **18650 Battery** | 3.7V 3000mAh | 1 | Alternative power | $8-12 |

### Detection & Sensors
| Component | Type | Interface | Range | Purpose | Cost |
|-----------|------|-----------|-------|---------|------|
| **PIR Sensor** | HC-SR501 | Digital | 3-7m | Motion detection | $2-3 |
| **Ultrasonic Sensor** | HC-SR04 | Digital | 2-400cm | Distance detection | $3-5 |
| **BME280** | Environmental | I2C | N/A | Temperature/Humidity | $5-8 |
| **Light Sensor** | LDR/BH1750 | Analog/I2C | N/A | Ambient light | $2-5 |

### Storage & Connectivity
| Component | Specification | Capacity | Speed | Purpose | Cost |
|-----------|---------------|----------|-------|---------|------|
| **MicroSD Card** | Class 10 U1 | 32GB | 10MB/s | Image storage | $8-12 |
| **MicroSD Module** | SPI Interface | N/A | N/A | Card interface | $2-3 |

### Enclosure & Mounting
| Component | Material | Size | Rating | Purpose | Cost |
|-----------|----------|------|--------|---------|------|
| **Waterproof Box** | ABS Plastic | 150x100x70mm | IP65 | Main enclosure | $15-25 |
| **Camera Mount** | Adjustable | Universal | N/A | Positioning | $5-10 |
| **Solar Panel Mount** | Aluminum | Adjustable | IP65 | Panel positioning | $8-15 |

## 📋 Shopping Lists by Deployment Type

### 🏡 Backyard Monitoring Setup
**Total Estimated Cost: $65-95**

**Essential Components:**
- AI-Thinker ESP32-CAM: $10
- PIR Sensor HC-SR501: $3
- MicroSD Card 32GB: $10
- MicroSD Module: $3
- LiPo Battery 2000mAh: $12
- TP4056 Charging Module: $3
- Solar Panel 6V 1W: $12
- Waterproof Enclosure: $20
- Basic mounting hardware: $8

**Optional Additions:**
- BME280 Environmental Sensor: $6
- Light Sensor: $3
- Additional storage (64GB): $15

### 🌲 Trail Monitoring Setup
**Total Estimated Cost: $85-125**

**Enhanced Components:**
- ESP32-S3-CAM (better performance): $18
- OV5640 Camera (higher resolution): $10
- PIR + Ultrasonic sensors: $8
- BME280 + Light sensors: $10
- 18650 Battery 3000mAh: $10
- Larger solar panel setup: $25
- Professional enclosure: $35
- Trail mounting system: $15

### 🏔️ Remote Wilderness Deployment
**Total Estimated Cost: $150-250**

**Professional Components:**
- Multiple ESP32-S3-CAM nodes: $50
- OV5640 + OV3660 cameras: $20
- Full sensor suite: $25
- Dual battery system: $30
- 10W solar panel array: $60
- Weatherproof enclosures: $75
- Professional mounting: $40
- Backup storage systems: $25

## 🔌 Pin Assignment Matrix

### AI-Thinker ESP32-CAM Pin Usage
| GPIO | Primary Function | Secondary Function | Conflict Level |
|------|------------------|-------------------|----------------|
| 0 | XCLK (Camera) | Boot mode | ⚠️ Critical |
| 1 | PIR Sensor | UART TX | ✅ Safe |
| 2 | Status LED | SD Card | ⚠️ Moderate |
| 3 | UART RX | Flash | ⚠️ Critical |
| 4 | Flash LED | Camera | ✅ Safe |
| 12 | SD CS | MTDI | ⚠️ Moderate |
| 13 | Reserved | Camera | ❌ Avoid |
| 14 | SD CLK | Camera | ⚠️ Moderate |
| 15 | SD MOSI | Camera | ⚠️ Moderate |
| 16 | PSRAM | Camera | ❌ Avoid |

## ⚡ Power Consumption Analysis

### Current Draw by Component
| Component | Active (mA) | Sleep (μA) | Duty Cycle | Avg. Current |
|-----------|-------------|------------|------------|--------------|
| ESP32-CAM | 240 | 10 | 5% | 12.5mA |
| OV2640 Camera | 80 | 0 | 2% | 1.6mA |
| PIR Sensor | 50 | 50 | 100% | 50μA |
| SD Card | 100 | 1 | 1% | 1mA |
| Status LEDs | 20 | 0 | 10% | 2mA |
| **Total** | | | | **~17mA** |

### Battery Life Calculations
| Battery | Capacity | Est. Runtime | Solar Charging |
|---------|----------|--------------|----------------|
| 2000mAh LiPo | 2000mAh | 5-7 days | +3-5 days |
| 3000mAh 18650 | 3000mAh | 7-10 days | +5-8 days |
| Dual 18650 | 6000mAh | 15-20 days | +10-15 days |

## 🛠️ Assembly Prerequisites

### Required Tools
- **Soldering Kit**: Iron, solder, flux, wick
- **Wire Strippers**: 22-28 AWG capacity
- **Multimeter**: For continuity and voltage testing
- **Heat Shrink Tubing**: Various sizes
- **Drill & Bits**: For enclosure mounting
- **Screwdriver Set**: Phillips and flathead
- **Hot Glue Gun**: For cable management

### Optional Professional Tools
- **Oscilloscope**: For signal debugging
- **Logic Analyzer**: For protocol analysis
- **Power Supply**: Variable DC for testing
- **Function Generator**: For testing
- **3D Printer**: For custom mounts

## 📦 Procurement Guidelines

### Recommended Suppliers
| Category | Primary Source | Alternative | Notes |
|----------|---------------|-------------|-------|
| **ESP32 Modules** | SparkFun, Adafruit | AliExpress | Quality vs. cost trade-off |
| **Sensors** | Digi-Key, Mouser | Amazon | Genuine parts important |
| **Power Components** | Battery Junction | Local electronics | Safety critical |
| **Enclosures** | Pelican, Hammond | Generic waterproof | IP rating verification |

### Quality Verification Checklist
- [ ] ESP32 module has genuine Espressif chips
- [ ] Camera sensors have proper model markings
- [ ] Battery cells have protection circuits
- [ ] Solar panels include bypass diodes
- [ ] Enclosures meet IP65 rating minimum
- [ ] All connectors are corrosion resistant

## 🔍 Compatibility Matrix

### ESP32 Board Compatibility
| Board Type | Camera Support | GPIO Available | PSRAM | Recommended Use |
|------------|----------------|----------------|-------|-----------------|
| **AI-Thinker** | ✅ OV2640 | Limited | 4MB | Standard deployment |
| **ESP32-S3-CAM** | ✅ Multiple | Abundant | 8MB | Advanced features |
| **TTGO T-Camera** | ✅ OV2640 | Moderate | 4MB | Development/testing |
| **M5Stack Timer** | ✅ OV2640 | Limited | 4MB | Compact deployment |

### Sensor Interface Compatibility
| Sensor Type | Interface | ESP32 Support | GPIO Requirements |
|-------------|-----------|---------------|-------------------|
| **PIR Motion** | Digital | ✅ Native | 1 GPIO |
| **Ultrasonic** | Digital | ✅ Native | 2 GPIO |
| **BME280** | I2C | ✅ Native | 2 GPIO (shared) |
| **Light (LDR)** | Analog | ✅ Native | 1 ADC |
| **Light (BH1750)** | I2C | ✅ Native | 2 GPIO (shared) |

## 📋 Hardware Validation Checklist

### Pre-Assembly Validation
- [ ] All components received and inventoried
- [ ] ESP32 module tested with basic sketch
- [ ] Camera module produces clear images
- [ ] Power system components tested individually
- [ ] Sensors respond to stimuli
- [ ] Enclosure fits all components with room for airflow

### Post-Assembly Validation
- [ ] System powers on reliably
- [ ] Camera captures and stores images
- [ ] Motion detection triggers consistently
- [ ] Solar charging system functions
- [ ] All sensors report reasonable values
- [ ] System survives basic weather simulation
- [ ] Power consumption within expected range

## 🔧 Troubleshooting Common Issues

### Power Problems
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| No power-on | Dead battery | Check charging system |
| Intermittent power | Loose connections | Secure all power wiring |
| Fast battery drain | Component fault | Measure individual currents |

### Camera Issues
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| No image capture | GPIO conflict | Verify pin assignments |
| Poor image quality | Lens focus/dirt | Clean and adjust lens |
| Corrupted images | SD card issue | Test with known good card |

### Sensor Problems
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| No motion detection | PIR sensitivity | Adjust sensitivity pot |
| False triggers | Vibration/heat | Relocate or shield sensor |
| Sensor offline | I2C address conflict | Scan I2C bus for devices |

## 📖 Next Steps

After procuring hardware, proceed to:
1. **[ASSEMBLY_INSTRUCTIONS.md](ASSEMBLY_INSTRUCTIONS.md)** - Step-by-step assembly guide
2. **[WORKBENCH_SETUP.md](WORKBENCH_SETUP.md)** - Prepare development environment
3. **[TEST_SCENARIOS.md](TEST_SCENARIOS.md)** - Validate hardware functionality

---

**Document Version**: 1.0  
**Last Updated**: 2025-09-01  
**Compatibility**: ESP32 Wildlife CAM v2.0+