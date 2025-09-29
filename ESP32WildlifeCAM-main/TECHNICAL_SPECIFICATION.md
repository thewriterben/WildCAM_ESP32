# 📐 ESP32WildlifeCAM Technical Specification

*Version: 1.0.0*  
*Date: 2025-08-31*  
*Author: @thewriterben*  
*Status: Draft*

## 1. System Overview

### 1.1 Purpose
The ESP32WildlifeCAM system provides autonomous wildlife monitoring through a network of solar-powered camera nodes that detect, photograph, and identify wildlife using edge AI, with data integrity verified through blockchain technology.

### 1.2 Scope
This specification defines the technical requirements, interfaces, and implementation details for all hardware and software components of the ESP32WildlifeCAM system.

### 1.3 System Context
```
┌─────────────────────────────────────────────────────────────┐
│                     Wildlife Habitat                          │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐ │
│  │ Camera  │    │ Camera  │    │ Camera  │    │Coordinator│ │
│  │ Node 1  │◄──►│ Node 2  │◄──►│ Node 3  │◄──►│   Node    │ │
│  └─────────┘    └─────────┘    └─────────┘    └─────────┘ │
│       │              │              │                │       │
│       └──────────────┴──────────────┴────────────────┘      │
│                         LoRa Mesh Network                    │
└─────────────────────────────────────────────────────────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │   Cloud Services      │
                    │  • Data Storage       │
                    │  • Web Interface      │
                    │  • AI Model Updates  │
                    └───────────────────────┘
```

## 2. Hardware Specifications

### 2.1 Supported ESP32 Boards

#### 2.1.1 AI-Thinker ESP32-CAM
```
Specification         Value
─────────────────────────────────────────
Processor            ESP32-D0WDQ6 (dual-core)
Clock Speed          240 MHz
Flash                4 MB
PSRAM                4 MB
GPIO Available       9 pins (with limitations)
Camera Interface     OV2640 (2MP)
Power Consumption    Active: 180mA, Deep Sleep: 10µA
Operating Voltage    5V (3.3V internal)
Dimensions           27mm × 40.5mm × 4.5mm
Operating Temp       -20°C to 70°C
```

#### 2.1.2 ESP32-S3-CAM (Recommended)
```
Specification         Value
─────────────────────────────────────────
Processor            ESP32-S3 (dual-core)
Clock Speed          240 MHz
Flash                8 MB
PSRAM                8 MB
GPIO Available       15 pins
Camera Interface     OV2640/OV5640 (5MP)
Power Consumption    Active: 200mA, Deep Sleep: 7µA
Operating Voltage    5V (3.3V internal)
USB                  Native USB support
AI Acceleration      Vector instructions
Dimensions           32mm × 45mm × 5mm
Operating Temp       -40°C to 85°C
```

### 2.2 Camera Specifications

#### 2.2.1 OV2640 (Standard)
```
Parameter             Value
─────────────────────────────────────────
Resolution           1600×1200 (2MP)
Pixel Size           2.2µm × 2.2µm
Lens FOV             65°
Output Format        JPEG, RGB565, YUV422
Frame Rate           15 fps @ UXGA
Power Consumption    Active: 140mA, Standby: <20µA
Interface            SCCB (I2C compatible)
Operating Voltage    3.3V
```

#### 2.2.2 OV5640 (Enhanced)
```
Parameter             Value
─────────────────────────────────────────
Resolution           2592×1944 (5MP)
Pixel Size           1.4µm × 1.4µm
Lens FOV             72°
Output Format        JPEG, RGB565, YUV422
Frame Rate           15 fps @ 5MP, 30 fps @ 1080p
Power Consumption    Active: 140mA, Standby: <20µA
Auto Features        Auto exposure, white balance, focus
Interface            SCCB (I2C compatible)
Operating Voltage    3.3V
```

### 2.3 Communication Modules

#### 2.3.1 LoRa Module (SX1276/SX1278)
```
Parameter             Value
─────────────────────────────────────────
Frequency            433/868/915 MHz (region-specific)
Modulation           LoRa, FSK, OOK
Sensitivity          -148 dBm
Output Power         +20 dBm (100mW)
Data Rate            0.3-50 kbps
Range                2-15 km (open area)
Current Draw         TX: 120mA, RX: 11mA, Sleep: 1µA
Interface            SPI
Operating Voltage    3.3V
Antenna              50Ω impedance
```

#### 2.3.2 Optional Cellular Module (SIM7600)
```
Parameter             Value
─────────────────────────────────────────
Network              LTE-M/NB-IoT/GSM
Bands                Global bands support
Data Rate            LTE: 10Mbps DL/5Mbps UL
Power Consumption    Active: 500mA peak, Idle: 10mA
Interface            UART/USB
Operating Voltage    3.4-4.2V
GPS                  Integrated
```

### 2.4 Power System

#### 2.4.1 Solar Panel Requirements
```
Parameter             Minimum    Recommended
────────────────────────────────────────────
Power Output         5W         10W
Voltage              6V         6V
Current              830mA      1670mA
Efficiency           18%        20%
Dimensions           165×135mm  235×165mm
Weather Rating       IP65       IP67
Operating Temp       -40°C to 85°C
```

#### 2.4.2 Battery Specifications
```
Parameter             Value
─────────────────────────────────────────
Type                 LiPo/Li-ion 18650
Nominal Voltage      3.7V
Capacity             3000-6000mAh
Charge Current       0.5C (1.5-3A)
Discharge Current    2C max
Protection           Over-charge, over-discharge, short circuit
Operating Temp       Charge: 0°C to 45°C, Discharge: -20°C to 60°C
Cycles               500+ @ 80% capacity
```

#### 2.4.3 Power Management IC (TP4056)
```
Parameter             Value
─────────────────────────────────────────
Input Voltage        4.5-5.5V
Charge Voltage       4.2V ±1%
Charge Current       Programmable up to 1A
Efficiency           >85%
Protection           Thermal, reverse polarity
Status Indicators    Charging, Complete
```

### 2.5 Sensors

#### 2.5.1 PIR Motion Sensor (HC-SR501)
```
Parameter             Value
─────────────────────────────────────────
Detection Range      3-7m adjustable
Detection Angle      120°
Trigger Modes        Single/Repeat
Output               3.3V digital
Delay Time           5s-300s adjustable
Operating Voltage    4.5-20V
Current Draw         <50µA standby, <65mA active
```

#### 2.5.2 Environmental Sensor (BME280)
```
Parameter             Value
─────────────────────────────────────────
Temperature Range    -40°C to 85°C
Temperature Accuracy ±1°C
Humidity Range       0-100% RH
Humidity Accuracy    ±3% RH
Pressure Range       300-1100 hPa
Pressure Accuracy    ±1 hPa
Interface            I2C/SPI
Power Consumption    3.6µA @ 1Hz
Operating Voltage    1.8-3.6V
```

### 2.6 Storage

#### 2.6.1 SD Card Requirements
```
Parameter             Minimum    Recommended
────────────────────────────────────────────
Capacity             16GB       64GB
Speed Class          Class 10   UHS-I U3
Write Speed          10MB/s     30MB/s
File System          FAT32      exFAT
Endurance            Standard   High Endurance
Operating Temp       -25°C to 85°C
```

## 3. Software Architecture

### 3.1 System Software Stack
```
┌──────────────────────────────────────────────┐
│            Application Layer                  │
├──────────────────────────────────────────────┤
│  Wildlife Detection │ Web UI │ Data Analytics │
├──────────────────────────────────────────────┤
│            Service Layer                      │
├──────────────────────────────────────────────┤
│  Multi-Board │ OTA │ Blockchain │ Telemetry  │
├──────────────────────────────────────────────┤
│         Hardware Abstraction Layer            │
├──────────────────────────────────────────────┤
│  Camera │ Network │ Storage │ Power │ Sensors │
├──────────────────────────────────────────────┤
│            Platform Layer                     │
├──────────────────────────────────────────────┤
│    ESP-IDF 5.0+ │ FreeRTOS │ Arduino Core    │
└──────────────────────────────────────────────┘
```

### 3.2 Memory Map
```
Address Range         Size    Usage
──────────────────────────────────────────
0x3FFB0000-0x3FFB7FFF  32KB   DRAM0 (Data)
0x3FFB8000-0x3FFFFFFF  160KB  DRAM1 (Heap)
0x40080000-0x400BFFFF  256KB  IRAM (Code)
0x400C0000-0x400C1FFF  8KB    RTC FAST
0x50000000-0x50001FFF  8KB    RTC SLOW

PSRAM (External):
0x3F800000-0x3FBFFFFF  4MB    Frame buffers, AI models
```

### 3.3 Task Allocation
```
Task Name            Priority  Stack   Core  Function
────────────────────────────────────────────────────
Camera Capture       5         4096    0     Image acquisition
Motion Detection     4         2048    0     PIR/Image monitoring
Network Manager      3         3072    1     LoRa/WiFi handling
AI Inference         3         8192    1     Wildlife detection
Storage Manager      2         2048    0     SD card operations
Power Manager        2         1024    0     Battery/solar monitoring
Web Server          1         4096    1     HTTP/WebSocket server
Maintenance         1         2048    0     Logging, cleanup
```

### 3.4 Communication Protocols

#### 3.4.1 LoRa Message Format
```
┌────┬─────┬─────┬──────┬─────────┬──────────┬─────┐
│PRE │SYNC │TYPE │LENGTH│ PAYLOAD │ CHECKSUM │ END │
├────┼─────┼─────┼──────┼─────────┼──────────┼─────┤
│0xAA│0x55 │ 1B  │  2B  │ 0-250B  │   2B     │0xFF │
└────┴─────┴─────┴──────┴─────────┴──────────┴─────┘

Message Types:
0x01 - Discovery Beacon
0x02 - Node Registration
0x03 - Image Metadata
0x04 - Image Data Chunk
0x05 - Command/Control
0x06 - Telemetry Data
0x07 - Acknowledgment
0x08 - Error Report
```

#### 3.4.2 Image Metadata Structure
```c
typedef struct {
    uint32_t timestamp;      // Unix timestamp
    uint32_t node_id;        // Unique node identifier
    uint16_t image_id;       // Sequential image number
    uint16_t width;          // Image width in pixels
    uint16_t height;         // Image height in pixels
    uint32_t size;           // Image size in bytes
    uint8_t  quality;        // JPEG quality (1-100)
    uint8_t  trigger_type;   // PIR/Scheduled/Manual
    float    confidence;     // AI detection confidence
    uint8_t  species_id;     // Detected species ID
    float    battery_voltage;// Current battery level
    float    temperature;    // Ambient temperature
    uint8_t  hash[32];      // SHA-256 hash
} ImageMetadata;
```

### 3.5 AI Model Specifications

#### 3.5.1 Wildlife Detection Model
```
Architecture:        MobileNetV2 (modified)
Input Size:          224×224×3 (RGB)
Output Classes:      20 common wildlife species
Model Size:          2.5MB (quantized)
Inference Time:      <500ms @ 240MHz
Accuracy:            87% top-1, 95% top-3
Memory Required:     4MB (including buffers)
```

#### 3.5.2 Species Classes
```
ID   Common Name          Scientific Name
──────────────────────────────────────────
00   No Animal           -
01   White-tailed Deer   Odocoileus virginianus
02   Black Bear          Ursus americanus
03   Gray Wolf           Canis lupus
04   Red Fox             Vulpes vulpes
05   Raccoon             Procyon lotor
06   Wild Turkey         Meleagris gallopavo
07   Coyote              Canis latrans
08   Bobcat              Lynx rufus
09   Elk                 Cervus canadensis
10   Moose               Alces alces
11   Mountain Lion       Puma concolor
12   Wild Boar           Sus scrofa
13   Bald Eagle          Haliaeetus leucocephalus
14   Great Blue Heron    Ardea herodias
15   Beaver              Castor canadensis
16   River Otter         Lontra canadensis
17   Striped Skunk       Mephitis mephitis
18   Opossum             Didelphis virginiana
19   Unknown Animal      -
```

### 3.6 Data Storage Schema

#### 3.6.1 SD Card Directory Structure
```
/WILDLIFE/
├── CONFIG/
│   ├── node.json         # Node configuration
│   ├── network.json      # Network settings
│   └── schedule.json     # Capture schedule
├── IMAGES/
│   ├── 2025/
│   │   ├── 08/
│   │   │   ├── 31/
│   │   │   │   ├── IMG_00001.jpg
│   │   │   │   ├── IMG_00001.json
│   │   │   │   └── ...
├── LOGS/
│   ├── system.log        # System events
│   ├── error.log         # Error logs
│   └── network.log       # Network activity
├── MODELS/
│   └── wildlife_v1.tflite # AI model
└── BLOCKCHAIN/
    └── chain.db          # Local blockchain
```

#### 3.6.2 Image Metadata JSON
```json
{
  "version": "1.0",
  "image": {
    "id": "IMG_00001",
    "timestamp": "2025-08-31T17:28:39Z",
    "node_id": "CAM_NODE_001",
    "location": {
      "lat": 45.1234,
      "lon": -122.5678,
      "alt": 125.5
    }
  },
  "capture": {
    "trigger": "motion",
    "settings": {
      "resolution": "1600x1200",
      "quality": 85,
      "exposure": "auto",
      "gain": 0
    }
  },
  "detection": {
    "species": "White-tailed Deer",
    "confidence": 0.923,
    "bbox": [120, 80, 450, 380],
    "count": 2
  },
  "environment": {
    "temperature": 22.5,
    "humidity": 65.2,
    "pressure": 1013.25,
    "light": "daylight"
  },
  "system": {
    "battery": 3.85,
    "solar": 5.12,
    "rssi": -72,
    "free_space": 28672
  },
  "blockchain": {
    "hash": "a3f5c8d9e2b1...",
    "block": 1234,
    "previous": "b7e4a2c6f9d3..."
  }
}
```

## 4. Network Specifications

### 4.1 LoRa Network Parameters
```
Parameter             Value
─────────────────────────────────────────
Frequency            915 MHz (US), 868 MHz (EU)
Bandwidth            125 kHz
Spreading Factor     7-12 (adaptive)
Coding Rate          4/5
Preamble Length      8 symbols
Sync Word            0x34 (private network)
TX Power             14-20 dBm (adaptive)
Air Time Limit       <1% duty cycle
Max Packet Size      255 bytes
Network Topology     Mesh (multi-hop)
```

### 4.2 WiFi Configuration
```
Mode:                Station/AP switchable
Security:            WPA2-PSK
SSID (AP Mode):      "WILDLIFE_CAM_XXXX"
Channel:             Auto (1-11)
TX Power:            20 dBm max
IP Assignment:       DHCP client/server
Web Server Port:     80 (HTTP), 443 (HTTPS)
WebSocket Port:      8080
mDNS:               wildlife-cam-XXXX.local
```

### 4.3 Network Protocols

#### 4.3.1 Node Discovery Protocol
```
1. New node broadcasts discovery beacon
2. Coordinator responds with network info
3. Node sends registration request
4. Coordinator assigns node ID and routes
5. Node acknowledges and joins network
6. Periodic heartbeat maintains connection
```

#### 4.3.2 Data Transmission Flow
```
1. Motion detected → Capture image
2. Run AI inference → Generate metadata
3. Create blockchain entry
4. Queue for transmission
5. Find best route to coordinator
6. Transmit in chunks with ACK
7. Coordinator aggregates data
8. Forward to cloud if available
```

## 5. Security Specifications

### 5.1 Encryption Standards
```
Layer                Algorithm         Key Size
────────────────────────────────────────────────
Storage              AES-256-CBC       256 bits
Network (LoRa)       AES-128-CTR       128 bits
Network (WiFi)       WPA2-PSK          256 bits
Web Interface        TLS 1.3           2048 bits
Firmware Updates     RSA + SHA-256     2048 bits
```

### 5.2 Authentication
```
Component            Method              Storage
────────────────────────────────────────────────
Node Identity        UUID + Secret       eFuse
Web Access          Username/Password    Encrypted NVS
API Access          JWT Tokens          RAM (temporary)
OTA Updates         RSA Signature       Flash
```

### 5.3 Secure Boot Configuration
```
1. Enable Flash Encryption
2. Enable Secure Boot V2
3. Burn eFuses for production
4. Disable JTAG/UART download
5. Set read protection on flash
```

## 6. Performance Requirements

### 6.1 Timing Constraints
```
Operation                    Max Time    Typical
────────────────────────────────────────────────
Motion to Capture Start      100ms       50ms
Image Capture               1000ms      750ms
AI Inference                 500ms      350ms
Metadata Generation          50ms        20ms
Storage Write               500ms       300ms
Network Transmission        5000ms     2000ms
Total Motion to Storage     <3s         <2s
```

### 6.2 Resource Utilization
```
Resource             Limit      Target    Max
──────────────────────────────────────────────
CPU Usage            100%       <70%      90%
RAM Usage            320KB      <224KB    288KB
PSRAM Usage          4MB        <3MB      3.5MB
Flash Writes/Day     100        <50       80
Power Consumption    500mW      <300mW    400mW
Network Bandwidth    50kbps     <20kbps   40kbps
```

### 6.3 Reliability Targets
```
Metric                      Target
─────────────────────────────────────
System Uptime               >99%
Image Capture Success       >95%
Network Delivery Rate       >95%
False Positive Rate         <5%
Mean Time Between Failures  >180 days
Data Integrity              100%
```

## 7. Environmental Specifications

### 7.1 Operating Conditions
```
Parameter            Min        Typical    Max
──────────────────────────────────────────────
Temperature          -20°C      20°C       50°C
Humidity             10%        50%        95%
Altitude             0m         500m       3000m
Wind Speed           0km/h      20km/h     80km/h
Solar Irradiance     100W/m²    500W/m²    1000W/m²
```

### 7.2 Enclosure Requirements
```
Specification        Requirement
─────────────────────────────────────
Ingress Protection   IP66 or better
Impact Resistance    IK08
UV Resistance        UV stabilized materials
Cable Entry          Sealed glands
Mounting             Universal bracket
Dimensions           <200×150×100mm
Weight               <1kg
Color                Camouflage/Natural
```

### 7.3 Compliance Standards
```
Standard             Region     Requirement
──────────────────────────────────────────
FCC Part 15          USA        Class B
CE RED 2014/53/EU    Europe     Compliance
IC RSS-247           Canada     Compliance
RoHS 2011/65/EU      Global     Compliant
IP66 IEC 60529       Global     Certified
MIL-STD-810G         Military   Method 514.6
```

## 8. Testing Specifications

### 8.1 Unit Test Coverage
```
Module               Target    Priority
───────────────────────────────────────
Camera Handler       90%       Critical
Motion Detector      85%       Critical
Network Protocol     90%       Critical
AI Inference         80%       High
Power Manager        85%       High
Storage Manager      80%       Medium
Web Interface        70%       Medium
Blockchain           75%       Low
```

### 8.2 Integration Test Scenarios
```
1. Power-on to First Image
2. Multi-node Network Formation
3. Continuous 24-hour Operation
4. Network Failure Recovery
5. Storage Full Handling
6. Low Battery Operation
7. Firmware Update Process
8. Environmental Stress Test
```

### 8.3 Field Test Requirements
```
Duration:            30 days minimum
Locations:           3 different habitats
Weather Conditions:  Include rain, snow, heat
Wildlife Activity:   High, medium, low areas
Network Coverage:    Full, partial, none
Data Collection:     Daily retrieval
Success Criteria:    >90% uptime, >85% detection
```

## 9. Maintenance Specifications

### 9.1 Preventive Maintenance Schedule
```
Interval    Task
─────────────────────────────────────────
Weekly      Remote health check
Monthly     Data retrieval and backup
Quarterly   Physical inspection
Annually    Battery replacement
Annually    Weatherproofing check
As Needed   Firmware updates
As Needed   Lens cleaning
```

### 9.2 Diagnostic Features
```
Feature              Implementation
─────────────────────────────────────────
Self-Test            Power-on diagnostics
Health Monitoring    Continuous telemetry
Error Logging        Persistent storage
Remote Diagnostics   Web interface
LED Indicators       Status codes
Watchdog Timer       Auto-recovery
```

### 9.3 Troubleshooting Matrix
```
Symptom              Possible Cause         Action
──────────────────────────────────────────────────
No Images            Motion sensor fault    Check PIR/settings
Poor Image Quality   Dirty lens            Clean lens
Network Loss         Range/interference     Check antenna/move
Rapid Battery Drain  Solar panel issue     Check connections
System Resets        Software bug          Update firmware
Missing Detections   AI model issue        Retrain/update model
```

## 10. Appendices

### A. GPIO Pin Assignments
[Detailed pin mappings for each board variant]

### B. API Reference
[Complete API documentation]

### C. Error Codes
[Comprehensive error code listing]

### D. Glossary
[Technical terms and abbreviations]

---

*This specification is subject to change. Version control and change history maintained in Git.*

*© 2025 ESP32WildlifeCAM Project - Technical Specification v1.0.0*