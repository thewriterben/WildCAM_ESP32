# Hardware Setup Guide - OV5640 5MP Wildlife Camera

Complete hardware assembly and configuration guide for the ESP32 Wildlife Camera system with OV5640 sensor and IR Cut filter.

## Table of Contents

1. [Bill of Materials](#bill-of-materials)
2. [Tools Required](#tools-required)
3. [Board Specifications](#board-specifications)
4. [Pin Connections](#pin-connections)
5. [Assembly Instructions](#assembly-instructions)
6. [Power System Setup](#power-system-setup)
7. [Enclosure Assembly](#enclosure-assembly)
8. [Testing and Verification](#testing-and-verification)
9. [Troubleshooting](#troubleshooting)

## Bill of Materials

### Core Components

| Component | Part Number | Quantity | Description | Supplier |
|-----------|-------------|----------|-------------|----------|
| **Main Board** | T-Camera Plus S3 | 1 | LilyGO ESP32-S3 camera board | LilyGO |
| **Camera Sensor** | OV5640 | 1 | 5MP camera module with auto focus | Multiple |
| **IR Cut Filter** | AP1511B_FBC | 1 | Day/night filter mechanism | Camera suppliers |
| **PIR Sensor** | HC-SR501 | 1 | Passive infrared motion detector | Electronics suppliers |
| **MicroSD Card** | Class 10, 32GB+ | 1 | High-speed storage for images | Multiple |

### Power System

| Component | Specification | Quantity | Description |
|-----------|---------------|----------|-------------|
| **LiPo Battery** | 3.7V, 2000mAh+ | 1 | Lithium polymer rechargeable battery |
| **Solar Panel** | 6V, 2W | 1 | Monocrystalline solar panel |
| **Charging Module** | TP4056 | 1 | LiPo battery charging circuit |
| **Voltage Divider** | 10kΩ, 20kΩ | 2 sets | Battery/solar voltage monitoring |
| **Protection Circuit** | Built-in or separate | 1 | Overcurrent/undervoltage protection |

### Mechanical Components

| Component | Specification | Quantity | Description |
|-----------|---------------|----------|-------------|
| **Weather Enclosure** | IP65 rated | 1 | Waterproof housing for electronics |
| **Cable Glands** | M12, IP68 | 3-4 | Waterproof cable entry points |
| **Mounting Bracket** | Adjustable | 1 | Camera positioning and tree mount |
| **Desiccant Packs** | Silica gel | 2-3 | Moisture absorption |

### Cables and Connectors

| Component | Specification | Quantity | Description |
|-----------|---------------|----------|-------------|
| **FFC Cable** | 24-pin, 15cm | 1 | Camera to board connection |
| **Jumper Wires** | Female-Female | 10 | PIR and power connections |
| **USB-C Cable** | 1m | 1 | Programming and debugging |
| **Extension Cables** | Various lengths | As needed | Remote sensor placement |

## Tools Required

### Basic Tools
- Soldering iron (temperature controlled)
- Solder (60/40 rosin core)
- Wire strippers
- Small Phillips screwdriver set
- Multimeter
- Heat shrink tubing
- Hot air gun or hair dryer

### Specialized Tools
- Crimping tool for JST connectors
- Flux and desoldering braid
- Oscilloscope (for advanced debugging)
- 3D printer (for custom enclosure parts)

## Board Specifications

### LilyGO T-Camera Plus S3 Features

#### ESP32-S3 Specifications
- **CPU**: Dual-core Xtensa LX7, up to 240MHz
- **RAM**: 512KB SRAM + 8MB PSRAM
- **Flash**: 16MB SPI Flash
- **WiFi**: 802.11 b/g/n, 2.4GHz
- **Bluetooth**: Bluetooth 5.0 LE
- **GPIO**: 45 programmable pins

#### Camera Interface
- **Parallel Interface**: 8-bit data bus
- **Clock Speed**: Up to 40MHz pixel clock
- **Supported Formats**: JPEG, YUV422, RGB565
- **Maximum Resolution**: 5MP (2592x1944)
- **Frame Rates**: Up to 30fps at lower resolutions

#### Power Management
- **Input Voltage**: 5V via USB-C or external
- **Logic Voltage**: 3.3V (regulated on-board)
- **Sleep Current**: <10µA in deep sleep
- **Active Current**: 100-300mA depending on operation

## Pin Connections

### T-Camera Plus S3 Pinout

```
                    ┌─────────────────────┐
                    │                     │
             GPIO15 │ XCLK    │ Camera    │ Module
              GPIO4 │ SDA     │ I2C       │ Connector
              GPIO5 │ SCL     │           │
                    │         │           │
             GPIO16 │ D7      │ Camera    │ Parallel
             GPIO17 │ D6      │ Data      │ Data
             GPIO18 │ D5      │ Bus       │ Interface
             GPIO12 │ D4      │           │
             GPIO10 │ D3      │           │
              GPIO8 │ D2      │           │
              GPIO9 │ D1      │           │
             GPIO11 │ D0      │           │
                    │         │           │
              GPIO6 │ VSYNC   │ Camera    │ Sync
              GPIO7 │ HREF    │ Sync      │ Signals
             GPIO13 │ PCLK    │           │
                    │                     │
                    └─────────────────────┘
```

### Primary Pin Assignments

| GPIO | Function | Direction | Description |
|------|----------|-----------|-------------|
| **Camera Interface** |
| 15 | XCLK | Output | External clock to camera (20MHz) |
| 4 | SDA | Bidirectional | I2C data for camera control |
| 5 | SCL | Output | I2C clock for camera control |
| 16 | D7 | Input | Camera data bit 7 (MSB) |
| 17 | D6 | Input | Camera data bit 6 |
| 18 | D5 | Input | Camera data bit 5 |
| 12 | D4 | Input | Camera data bit 4 |
| 10 | D3 | Input | Camera data bit 3 |
| 8 | D2 | Input | Camera data bit 2 |
| 9 | D1 | Input | Camera data bit 1 |
| 11 | D0 | Input | Camera data bit 0 (LSB) |
| 6 | VSYNC | Input | Vertical synchronization |
| 7 | HREF | Input | Horizontal reference |
| 13 | PCLK | Input | Pixel clock |

### Additional Pin Assignments

| GPIO | Function | Direction | Description |
|------|----------|-----------|-------------|
| **Wildlife Monitoring** |
| 16 | IR_CUT | Output | IR Cut filter control (AP1511B_FBC) |
| 17 | PIR_SENSOR | Input | Motion detection sensor |
| **Power Monitoring** |
| 1 | BATTERY_ADC | Input | Battery voltage sensing |
| 2 | SOLAR_ADC | Input | Solar panel voltage sensing |
| **System** |
| 0 | BOOT | Input | Boot mode selection / User button |
| 2 | STATUS_LED | Output | Built-in status LED |

**Note**: GPIO16 and GPIO17 are reassigned from camera data to IR Cut and PIR sensor respectively. This configuration assumes the OV5640 can operate with 6-bit data mode or alternative pin mapping.

## Assembly Instructions

### Step 1: Prepare the Main Board

1. **Inspect the T-Camera Plus S3**
   - Check for physical damage
   - Verify all connectors are intact
   - Test basic functionality with USB connection

2. **Install MicroSD Card**
   - Format as FAT32 with 32KB allocation
   - Insert into board's SD card slot
   - Verify detection with test program

### Step 2: Camera Module Preparation

1. **OV5640 Module Inspection**
   ```
   OV5640 Module Components:
   ┌─────────────────────────┐
   │  ┌─────┐               │
   │  │ Lens│ ○ Focus Motor  │
   │  └─────┘               │
   │    ┌───┐               │
   │    │OV │ ○ LED (if any) │
   │    │5640               │
   │    └───┘               │
   │                        │
   │  FFC Connector (24-pin) │
   └─────────────────────────┘
   ```

2. **Test Auto Focus Mechanism**
   - Gently test focus ring movement
   - Check for smooth operation
   - Verify electrical connections

3. **Lens Cleaning**
   - Use lens cleaning solution and microfiber cloth
   - Remove dust and fingerprints
   - Apply lens cap when not testing

### Step 3: IR Cut Filter Installation

1. **AP1511B_FBC Module Wiring**
   ```
   AP1511B_FBC Connections:
   ┌─────────────────┐
   │     Filter      │
   │   Mechanism     │
   │                 │
   │ VCC  ────────── │ +3.3V
   │ GND  ────────── │ Ground
   │ CTRL ────────── │ GPIO16
   └─────────────────┘
   ```

2. **Control Circuit**
   ```cpp
   // Basic control circuit
   ESP32 GPIO16 ──┬── 1kΩ ──┬── Gate (MOSFET)
                  │         │
                  └── 10kΩ ──┴── GND
   
   +3.3V ──┬── IR Cut Filter VCC
           │
   MOSFET  │── Drain
   Source ──── GND
   ```

3. **Mechanical Installation**
   - Position filter in optical path
   - Ensure clear switching between positions
   - Test mechanical reliability

### Step 4: PIR Sensor Connection

1. **HC-SR501 PIR Sensor Wiring**
   ```
   HC-SR501 Connections:
   ┌─────────────────┐
   │     PIR         │
   │   Sensor        │
   │                 │
   │ VCC  ────────── │ +5V (from external supply)
   │ OUT  ────────── │ GPIO17 (via level shifter)
   │ GND  ────────── │ Ground
   └─────────────────┘
   ```

2. **Level Shifting Circuit** (5V PIR to 3.3V ESP32)
   ```
   PIR OUT (5V) ──┬── 10kΩ ──┬── +3.3V
                  │          │
                  └── MOSFET ──┴── GPIO17
                      │
                      └── GND
   ```

3. **Sensitivity Adjustment**
   - Locate sensitivity potentiometer on PIR module
   - Start with medium setting (center position)
   - Fine-tune during testing phase

### Step 5: Power System Assembly

1. **Battery Connection**
   ```
   Battery Management:
   Solar Panel + ──┬── Blocking Diode ──┬── TP4056 VIN
                   │                    │
   Battery +  ─────┴── TP4056 BAT+ ─────┴── Load Switch + ──┬── ESP32 VIN
                                                            │
   Battery -  ────────── TP4056 BAT- ─────── Load Switch - ──┴── ESP32 GND
                                      │
   Solar Panel - ─────────── TP4056 GND
   ```

2. **Voltage Monitoring**
   ```
   Battery Voltage Divider:
   BAT+ ──┬── 20kΩ ──┬── GPIO1 (ADC)
          │          │
          └── 10kΩ ──┴── GND
   
   Solar Voltage Divider:
   SOLAR+ ──┬── 30kΩ ──┬── GPIO2 (ADC)
            │          │
            └── 10kΩ ──┴── GND
   ```

3. **Protection Circuits**
   - Install fuse or PTC resettable fuse
   - Add reverse polarity protection diode
   - Include undervoltage lockout circuit

### Step 6: System Integration

1. **Cable Management**
   - Use appropriate gauge wire for current loads
   - Implement strain relief at all connections
   - Color code wires for easy identification

2. **Grounding Strategy**
   - Single point ground to minimize noise
   - Keep analog and digital grounds separated initially
   - Use ground plane in PCB design if custom board

3. **EMI/RFI Considerations**
   - Keep high-frequency signals (PCLK) short
   - Use twisted pair for I2C connections
   - Add ferrite cores if noise issues occur

## Power System Setup

### Solar Panel Sizing

#### Power Consumption Analysis
```
Operating Mode Power Consumption:
┌─────────────────┬─────────┬──────────┬──────────────┐
│ Mode            │ Current │ Duration │ Daily Energy │
├─────────────────┼─────────┼──────────┼──────────────┤
│ Deep Sleep      │ 10µA    │ 20 hours │ 0.2mAh       │
│ PIR Monitoring  │ 50mA    │ 3 hours  │ 150mAh       │
│ Image Capture   │ 300mA   │ 1 hour   │ 300mAh       │
│ WiFi Upload     │ 200mA   │ 10 min   │ 33mAh        │
├─────────────────┼─────────┼──────────┼──────────────┤
│ Total Daily     │         │          │ 483mAh       │
└─────────────────┴─────────┴──────────┴──────────────┘
```

#### Solar Panel Requirements
- **Daily Energy**: 483mAh × 3.7V = 1.79Wh
- **Charging Efficiency**: ~70% (including conversion losses)
- **Required Solar Energy**: 1.79Wh ÷ 0.7 = 2.56Wh
- **Peak Sun Hours**: 4-6 hours (depending on location)
- **Panel Power**: 2.56Wh ÷ 4h = 0.64W minimum
- **Recommended Panel**: 2W (includes safety margin and winter compensation)

### Battery Sizing

#### Capacity Requirements
```
Autonomy Analysis:
┌─────────────────┬──────────┬────────────┬─────────────┐
│ Scenario        │ Duration │ Daily Use  │ Capacity    │
├─────────────────┼──────────┼────────────┼─────────────┤
│ Normal Ops      │ 1 day    │ 483mAh     │ 483mAh      │
│ Cloudy Weather  │ 2 days   │ 483mAh     │ 966mAh      │
│ Winter/Emergency│ 3 days   │ 483mAh     │ 1449mAh     │
├─────────────────┼──────────┼────────────┼─────────────┤
│ Recommended     │          │            │ 2000mAh+    │
└─────────────────┴──────────┴────────────┴─────────────┘
```

#### Battery Selection Criteria
- **Chemistry**: LiPo for energy density, Li-ion for longevity
- **Voltage**: 3.7V nominal (3.0-4.2V range)
- **Capacity**: 2000-5000mAh depending on autonomy requirements
- **Temperature Rating**: -20°C to +60°C for outdoor use
- **Protection**: Built-in or external BMS required

### Charging Circuit Configuration

#### TP4056 Module Setup
```
TP4056 Configuration:
┌─────────────────────────────────────┐
│  Solar Input (5-6V)                 │
│         │                           │
│    ┌────▼────┐                      │
│    │ TP4056  │ ← Charge Current     │
│    │ Charger │   Setting Resistor   │
│    └────┬────┘   (1.2kΩ = 1A)       │
│         │                           │
│  Battery Output (3.7V)              │
│         │                           │
│    ┌────▼────┐                      │
│    │ DW01A   │ ← Protection         │
│    │ Battery │   Circuit            │
│    │ Monitor │   (Optional)         │
│    └────┬────┘                      │
│         │                           │
│  Load Output to ESP32               │
└─────────────────────────────────────┘
```

#### Charge Current Setting
```cpp
// Charge current calculation
// I_charge = 1000V / R_prog
// For 500mA charge: R_prog = 1000V / 0.5A = 2kΩ
// For 1A charge: R_prog = 1000V / 1A = 1kΩ

#define CHARGE_CURRENT_MA 500  // Conservative for longevity
#define RPROG_OHMS 2000        // 2kΩ resistor
```

## Enclosure Assembly

### Environmental Protection Requirements

#### IP Rating Specifications
- **IP65 Minimum**: Protection against dust and water jets
- **IP67 Recommended**: Protection against temporary submersion
- **Operating Temperature**: -20°C to +60°C
- **Storage Temperature**: -40°C to +85°C
- **Humidity**: 0-95% non-condensing

### Enclosure Selection

#### Commercial Enclosure Options
```
Recommended Enclosure Sizes:
┌─────────────────┬─────────────┬──────────────┬─────────────┐
│ Application     │ Dimensions  │ Material     │ IP Rating   │
├─────────────────┼─────────────┼──────────────┼─────────────┤
│ Basic System    │ 150x100x75  │ ABS Plastic  │ IP65        │
│ Extended System │ 200x150x100 │ Aluminum     │ IP67        │
│ Solar Integrated│ 250x200x100 │ Fiberglass   │ IP68        │
└─────────────────┴─────────────┴──────────────┴─────────────┘
```

#### Custom 3D Printed Options
```stl
// Basic enclosure dimensions
module wildlife_camera_enclosure() {
    difference() {
        // Main body
        cube([150, 100, 75]);
        
        // Internal cavity
        translate([5, 5, 5])
            cube([140, 90, 65]);
        
        // Camera opening
        translate([75, 50, 70])
            cylinder(d=25, h=10);
        
        // Cable glands
        translate([10, 50, 10])
            cylinder(d=12, h=20);
    }
}
```

### Assembly Process

#### Step 1: Prepare Enclosure
1. **Drill Cable Entry Points**
   - Mark locations for cable glands
   - Use step drill bits for clean holes
   - Install cable glands with gaskets

2. **Mount Internal Components**
   - Create mounting plate for main board
   - Position for easy access to ports
   - Allow space for heat dissipation

#### Step 2: Waterproofing
1. **Gasket Installation**
   - Apply silicone gasket to lid interface
   - Ensure even pressure distribution
   - Test fit before final assembly

2. **Cable Sealing**
   - Use appropriate cable glands for wire gauge
   - Apply thread locker to gland threads
   - Test seal with slight pressure

#### Step 3: Ventilation and Condensation Control
1. **Breather Valve** (if required)
   - Install Gore-Tex vent for pressure equalization
   - Position away from direct weather exposure
   - Size for expected temperature variations

2. **Desiccant Placement**
   - Use indicating silica gel packets
   - Position away from heat sources
   - Plan for periodic replacement

### Thermal Management

#### Heat Dissipation Strategy
```
Thermal Design Considerations:
┌─────────────────┬─────────────┬──────────────┬─────────────┐
│ Component       │ Power (W)   │ Temp Rise    │ Solution    │
├─────────────────┼─────────────┼──────────────┼─────────────┤
│ ESP32-S3        │ 0.5-1.0     │ 10-20°C      │ Airflow     │
│ Camera Module   │ 0.3-0.5     │ 5-10°C       │ Thermal Pad │
│ Charging Circuit│ 0.2-0.5     │ 10-15°C      │ Heat Sink   │
│ Total System    │ 1.0-2.0     │ 15-30°C      │ Ventilation │
└─────────────────┴─────────────┴──────────────┴─────────────┘
```

#### Cooling Solutions
1. **Natural Convection**
   - Position enclosure for upward airflow
   - Use aluminum enclosure for heat conduction
   - Add thermal interface materials

2. **Forced Ventilation** (if needed)
   - Low-power 12V fans for extreme climates
   - Temperature-controlled operation
   - Filtered air intake

## Testing and Verification

### Initial System Test

#### Step 1: Basic Functionality
```bash
# Connect serial monitor
platformio device monitor --baud 115200

# Verify boot sequence
[I] ESP32WildlifeCAM: System starting...
[I] CAMERA: OV5640 sensor detected
[I] IR_CUT: Filter control initialized
[I] PIR: Motion sensor ready
[I] POWER: Battery voltage: 3.87V
[I] POWER: Solar voltage: 5.23V
[I] SYSTEM: All systems operational
```

#### Step 2: Camera Test
```cpp
// Test capture sequence
camera.init(BOARD_LILYGO_T_CAMERA_PLUS);
camera_fb_t* test_image = camera.captureImage();

if (test_image) {
    Serial.printf("Captured: %dx%d, %u bytes\n", 
                  test_image->width, 
                  test_image->height, 
                  test_image->len);
    esp_camera_fb_return(test_image);
}
```

#### Step 3: IR Cut Filter Test
```cpp
// Test day/night switching
camera.setIRCutFilter(IR_CUT_ENABLED);
delay(1000);
camera_fb_t* day_image = camera.captureImage();

camera.setIRCutFilter(IR_CUT_DISABLED);
delay(1000);
camera_fb_t* night_image = camera.captureImage();

// Compare image characteristics
```

### Comprehensive System Validation

#### Power System Test
1. **Battery Charging**
   ```cpp
   // Monitor charging cycle
   for (int i = 0; i < 24; i++) {
       float battery_v = powerManager.getBatteryVoltage();
       float solar_v = powerManager.getSolarVoltage();
       bool charging = powerManager.isCharging();
       
       Serial.printf("Hour %d: Bat=%.2fV, Solar=%.2fV, Charging=%s\n",
                     i, battery_v, solar_v, charging ? "Yes" : "No");
       delay(3600000); // 1 hour
   }
   ```

2. **Deep Sleep Test**
   ```cpp
   // Test wake-up functionality
   esp_sleep_enable_ext0_wakeup(PIR_SENSOR_PIN, HIGH);
   esp_sleep_enable_timer_wakeup(30 * 1000000ULL); // 30 seconds
   
   Serial.println("Entering deep sleep...");
   esp_deep_sleep_start();
   ```

#### Motion Detection Validation
1. **Sensitivity Calibration**
   - Test with known motion sources
   - Adjust PIR sensitivity potentiometer
   - Verify detection range and accuracy

2. **False Positive Testing**
   - Test with environmental conditions
   - Evaluate wind sensitivity
   - Check temperature compensation

#### Image Quality Assessment
1. **Resolution Verification**
   ```cpp
   // Test all supported resolutions
   WildlifeFrameSize sizes[] = {
       WILDLIFE_VGA, WILDLIFE_SVGA, WILDLIFE_HD,
       WILDLIFE_SXGA, WILDLIFE_UXGA, WILDLIFE_FHD,
       WILDLIFE_QSXGA
   };
   
   for (auto size : sizes) {
       camera.setFrameSize(size);
       camera_fb_t* fb = camera.captureImage();
       // Analyze image quality metrics
   }
   ```

2. **Auto Focus Performance**
   ```cpp
   // Test focus accuracy
   for (int distance = 1; distance <= 10; distance++) {
       // Position test target at known distance
       camera.performAutoFocus();
       camera_fb_t* fb = camera.captureImage();
       
       float sharpness = calculateSharpness(fb);
       Serial.printf("Distance %dm: Sharpness = %.2f\n", distance, sharpness);
   }
   ```

### Field Deployment Test

#### Environmental Stress Test
1. **Temperature Cycling**
   - Test operation from -10°C to +50°C
   - Monitor performance degradation
   - Verify automatic compensation

2. **Humidity Testing**
   - Expose to high humidity conditions
   - Check for condensation issues
   - Verify desiccant effectiveness

3. **Vibration/Shock Testing**
   - Simulate wind and animal disturbance
   - Check mounting security
   - Verify camera stability

#### Long-Term Reliability Test
1. **Continuous Operation**
   - Run system for 30+ days
   - Monitor failure modes
   - Collect performance statistics

2. **Component Stress Analysis**
   - Monitor power supply stability
   - Check connector reliability
   - Assess mechanical wear

## Troubleshooting

### Common Hardware Issues

#### Camera Problems
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No image capture | Poor FFC connection | Reseat camera cable |
| Blurry images | Focus mechanism fault | Check auto focus motor |
| Color distortion | Lens contamination | Clean lens carefully |
| Dark images | IR Cut filter stuck | Verify GPIO16 control |

#### Power Issues
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Won't turn on | Battery discharged | Check solar charging |
| Frequent resets | Undervoltage | Verify power supply capacity |
| Not charging | Solar panel fault | Check panel voltage/connections |
| Short runtime | Battery degradation | Test battery capacity |

#### Motion Detection Issues
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No detection | PIR sensor fault | Check power and connections |
| False triggers | High sensitivity | Adjust PIR potentiometer |
| Delayed response | Software debounce | Tune timing parameters |
| Continuous trigger | Electrical noise | Add filtering capacitor |

### Diagnostic Tools

#### Built-in Diagnostics
```cpp
// System health check
void systemDiagnostics() {
    // Camera test
    bool camera_ok = camera.testCamera();
    Serial.printf("Camera: %s\n", camera_ok ? "OK" : "FAULT");
    
    // Power system test
    float vbat = powerManager.getBatteryVoltage();
    float vsolar = powerManager.getSolarVoltage();
    Serial.printf("Power: Bat=%.2fV Solar=%.2fV\n", vbat, vsolar);
    
    // Memory test
    size_t free_heap = esp_get_free_heap_size();
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    Serial.printf("Memory: Heap=%uKB PSRAM=%uKB\n", 
                  free_heap/1024, free_psram/1024);
    
    // Storage test
    uint64_t card_size = SD_MMC.cardSize();
    uint64_t used_space = SD_MMC.usedBytes();
    Serial.printf("Storage: %lluMB used of %lluMB\n", 
                  used_space/(1024*1024), card_size/(1024*1024));
}
```

#### External Test Equipment
1. **Multimeter**: Voltage and current measurements
2. **Oscilloscope**: Signal integrity analysis
3. **Thermal Camera**: Heat distribution monitoring
4. **Function Generator**: Signal injection testing
5. **Network Analyzer**: WiFi performance testing

### Preventive Maintenance

#### Regular Inspection Checklist
- [ ] Camera lens cleanliness
- [ ] Enclosure seal integrity
- [ ] Cable connection security
- [ ] Solar panel efficiency
- [ ] Battery performance
- [ ] SD card health
- [ ] Mounting stability
- [ ] Desiccant condition

#### Replacement Schedule
| Component | Lifespan | Replacement Indicator |
|-----------|----------|----------------------|
| Battery | 2-3 years | Capacity <80% of rated |
| SD Card | 3-5 years | Write errors or slow access |
| Desiccant | 6-12 months | Color change indicator |
| Gaskets | 2-3 years | Visible degradation |
| Solar Panel | 10+ years | Output <90% of rated |

This comprehensive hardware setup guide provides the foundation for building a reliable, professional-grade wildlife monitoring system. Proper assembly, testing, and maintenance will ensure years of dependable operation in challenging field conditions.