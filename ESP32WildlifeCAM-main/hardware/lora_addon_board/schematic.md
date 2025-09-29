# LoRa Addon Board Schematic and Wiring Guide

## ESP32-CAM + LoRa Module Connection Diagrams

### Solution 1: Alternative GPIO Mapping (AI-Thinker ESP32-CAM)

```
ESP32-CAM                    LoRa Module (SX1276/RFM95)
=========                    ==========================

Power:
 3.3V  ----+                   VCC (Pin 1)
           |
           +---- [100nF] ----+  [Bypass Capacitor]
           |                 |
           +---- [10µF]  ----+  [Power Filter]
           
 GND   ----------------------  GND (Pin 2)

SPI Interface:
 GPIO12 ---[1kΩ]------------- SCK  (Pin 3)  [SPI Clock]
 GPIO13 ---[1kΩ]------------- MISO (Pin 4)  [SPI Data Out]  
 GPIO15 ---[1kΩ]------------- MOSI (Pin 5)  [SPI Data In]
 GPIO14 ---[1kΩ]------------- CS   (Pin 6)  [Chip Select]

Control:
 GPIO2  ---[1kΩ]------------- RST  (Pin 7)  [Reset]
 GPIO4  ---[1kΩ]------------- DIO0 (Pin 8)  [Interrupt]
 GPIO16 ---[1kΩ]------------- DIO1 (Pin 9)  [Optional]

Antenna:
                               ANT  (Pin 10) [Antenna Connection]

Legend:
---     = Wire Connection
[1kΩ]   = Series Resistor (protection)
[100nF] = Ceramic Capacitor  
[10µF]  = Electrolytic Capacitor
```

### Solution 2: ESP32-S3-CAM Standard Mapping

```
ESP32-S3-CAM                 LoRa Module (SX1276/RFM95)
============                 ==========================

Power:
 3.3V  ----+                   VCC (Pin 1)
           |
           +---- [100nF] ----+  [Bypass Capacitor]
           |                 |
           +---- [10µF]  ----+  [Power Filter]
           
 GND   ----------------------  GND (Pin 2)

SPI Interface (Standard):
 GPIO18 ---[1kΩ]------------- SCK  (Pin 3)  [SPI Clock]
 GPIO19 ---[1kΩ]------------- MISO (Pin 4)  [SPI Data Out]
 GPIO23 ---[1kΩ]------------- MOSI (Pin 5)  [SPI Data In]
 GPIO5  ---[1kΩ]------------- CS   (Pin 6)  [Chip Select]

Control:
 GPIO14 ---[1kΩ]------------- RST  (Pin 7)  [Reset]
 GPIO26 ---[1kΩ]------------- DIO0 (Pin 8)  [Interrupt]
 GPIO27 ---[1kΩ]------------- DIO1 (Pin 9)  [Optional]
 GPIO33 ---[1kΩ]------------- DIO2 (Pin 10) [Optional]

Antenna:
                               ANT  (Pin 11) [Antenna Connection]
```

## Detailed Component Layouts

### LoRa Module Pinouts

#### SX1276/RFM95 Module
```
      ┌─────────────────┐
   1  │ VCC       GND   │ 2
   3  │ SCK       DIO5  │ 4
   5  │ MOSI      DIO4  │ 6  
   7  │ RST       DIO3  │ 8
   9  │ DIO1      DIO2  │ 10
  11  │ DIO0      CS    │ 12
  13  │ MISO      ANT   │ 14
      └─────────────────┘
      
Pin Functions:
1  - VCC: 3.3V Power Supply
2  - GND: Ground
3  - SCK: SPI Clock Input
4  - DIO5: Digital I/O 5 (optional)
5  - MOSI: SPI Data Input
6  - DIO4: Digital I/O 4 (optional)
7  - RST: Reset Input (active low)
8  - DIO3: Digital I/O 3 (optional) 
9  - DIO1: Digital I/O 1 (optional)
10 - DIO2: Digital I/O 2 (optional)
11 - DIO0: Digital I/O 0 (interrupt)
12 - CS: SPI Chip Select (active low)
13 - MISO: SPI Data Output
14 - ANT: Antenna Connection
```

#### SX1262/SX1268 Module  
```
      ┌─────────────────┐
   1  │ VCC       GND   │ 2
   3  │ SCK       BUSY  │ 4
   5  │ MOSI      DIO3  │ 6
   7  │ RST       DIO2  │ 8
   9  │ DIO1      CS    │ 10
  11  │ MISO      ANT   │ 12
      └─────────────────┘

Pin Functions:
1  - VCC: 3.3V Power Supply
2  - GND: Ground  
3  - SCK: SPI Clock Input
4  - BUSY: Busy Status Output
5  - MOSI: SPI Data Input
6  - DIO3: Digital I/O 3
7  - RST: Reset Input (active low)
8  - DIO2: Digital I/O 2
9  - DIO1: Digital I/O 1 (interrupt)
10 - CS: SPI Chip Select (active low) 
11 - MISO: SPI Data Output
12 - ANT: Antenna Connection
```

### ESP32-CAM Pinout Reference

```
                    ESP32-CAM Module
        ┌─────────────────────────────────────┐
        │  ┌─────┐                            │
     1  │  │ USB │ IO0                    3V3 │ 32
     2  │  │ CON │ GND                    GND │ 31  
     3  │  └─────┘ IO12                  IO1  │ 30
     4  │         IO13                  IO3  │ 29
     5  │         IO15                  IO21 │ 28
     6  │         IO14                  IO19 │ 27
     7  │         IO2                   IO18 │ 26
     8  │         IO4                   IO5  │ 25  
     9  │  [CAMERA]                     IO17 │ 24
    10  │  MODULE                       IO16 │ 23
    11  │         GND                   IO22 │ 22
    12  │         IO33                  IO23 │ 21
    13  │         IO32                  IO25 │ 20
    14  │         IO35                  IO26 │ 19
    15  │         IO34                  IO27 │ 18
    16  │         IO39                  3V3  │ 17
        └─────────────────────────────────────┘

Available GPIO Pins (AI-Thinker ESP32-CAM):
- GPIO2, GPIO4, GPIO12, GPIO13, GPIO14, GPIO15, GPIO16
- Note: GPIO0 used for programming/boot
- Note: GPIO1, GPIO3 used for UART
- Note: GPIO5, GPIO18, GPIO19, GPIO21, GPIO22, GPIO23, GPIO25, GPIO26, GPIO27 used by camera
```

## Power Supply Design

### Basic Power Circuit
```
Solar Panel    Charge Controller    Battery    ESP32-CAM + LoRa
┌─────────┐    ┌───────────────┐   ┌──────┐   ┌─────────────────┐
│   6V    │───→│    MPPT       │───│ 3.7V │───│ 3.3V Regulator  │
│   5W    │    │  Controller   │   │ LiPo │   │    ESP32-CAM    │
└─────────┘    └───────────────┘   └──────┘   │    LoRa Module  │
                                             └─────────────────┘

Components:
- Solar Panel: 6V 5W minimum
- Charge Controller: TP4056 or similar
- Battery: 3.7V 3000mAh LiPo
- Regulator: AMS1117-3.3 or ESP32-CAM built-in
```

### Advanced Power Circuit with Monitoring
```
Solar Panel
┌─────────┐    
│   12V   │    Battery Monitor
│   10W   │    ┌─────────────┐    Battery Pack
└────┬────┘    │ Voltage     │    ┌─────────────┐
     │         │ Current     │    │ 3S LiPo     │
     │    ┌────│ Sensor      │    │ 11.1V       │
     │    │    └─────────────┘    │ 5000mAh     │
     │    │                       └──────┬──────┘
     │    │    MPPT Controller           │
     └────┼───→┌─────────────┐           │
          │    │ Boost/Buck  │           │
          │    │ Converter   │           │
          │    └──────┬──────┘           │
          │           │                  │
          │           │   Power Switch   │
          │           │   ┌───────────┐  │
          │           └───│ MOSFET    │  │
          │               │ Control   │  │
          │               └─────┬─────┘  │
          │                     │        │
          │    ESP32 System     │        │
          └────────────────────→┼────────┘
                               │
                    ┌─────────────────┐
                    │ ESP32-CAM       │
                    │ LoRa Module     │
                    │ Sensors         │
                    └─────────────────┘
```

## Antenna Design and Selection

### Wire Antenna Calculations

#### Quarter Wave Antenna Length
```
Length (mm) = 71250 / Frequency (MHz)

Examples:
- 433 MHz: 164.5 mm
- 868 MHz: 82.1 mm  
- 915 MHz: 77.8 mm
- 920 MHz: 77.4 mm
```

#### Simple Wire Antenna Construction
```
                    Coax Cable
    ┌──────────────────────────────┐
    │                              │
    │  Center Conductor (Signal)   │ ────→ Wire Element
    │  ├─────────────────────────┐ │       (1/4 wavelength)
    │  │                         │ │           │
    │  │    Outer Shield (GND)   │ │ ──────────┘
    │  └─────────────────────────┘ │
    │                              │
    └──────────────────────────────┘
            To LoRa Module
```

### PCB Antenna Design (Meandered)
```
PCB Antenna Pattern (Top Layer):
                                    Feed Point
    ┌─────────────────────────────────┐ ←
    │                                 │
    └──┐                         ┌────┘
       │                         │
    ┌──┘                         └────┐
    │                                 │
    └──┐                         ┌────┘
       │                         │
    ┌──┘                         └────┐
    │                                 │
    └─────────────────────────────────┘
    
Ground Plane (Bottom Layer):
    ┌─────────────────────────────────┐
    │ █████████████████████████████████│
    │ ███████████████   ███████████████│ ← Keepout Area
    │ █████████████████████████████████│
    └─────────────────────────────────┘
```

## PCB Layout Guidelines

### Layer Stack-up (4-layer recommended)
```
Layer 1: Component/Signal (Top)
         ┌─────────────────────────┐
         │ Components, Routing     │
         ├─────────────────────────┤
Layer 2: Ground Plane
         │ ████████████████████████│
         ├─────────────────────────┤  
Layer 3: Power Plane (+3.3V)
         │ ████████████████████████│
         ├─────────────────────────┤
Layer 4: Signal/Shield (Bottom)  
         │ Routing, Shield         │
         └─────────────────────────┘
```

### Component Placement
```
                PCB Layout (Top View)
    ┌─────────────────────────────────────────┐
    │                                         │
    │  ESP32-CAM                              │
    │  ┌─────────┐    LoRa Module             │
    │  │         │    ┌─────────┐             │
    │  │         │    │         │    Antenna  │
    │  │         │    │         │    ┌──────┐ │
    │  │    U1   │    │   U2    │────│  A1  │ │
    │  │         │    │         │    └──────┘ │
    │  └─────────┘    └─────────┘             │
    │                                         │
    │  Power Management                       │
    │  ┌─────────┐  ┌─────────┐              │
    │  │   U3    │  │   U4    │   Status     │
    │  │ Charge  │  │ Buck/   │   LEDs       │
    │  │ Control │  │ Boost   │   ┌─┐ ┌─┐    │
    │  └─────────┘  └─────────┘   │1│ │2│    │
    │                             └─┘ └─┘    │
    │                                         │
    │  Connectors                             │
    │  ┌─────────┐  ┌─────────┐  ┌─────────┐ │
    │  │ Solar   │  │ Battery │  │  Prog   │ │
    │  │   J1    │  │   J2    │  │   J3    │ │
    │  └─────────┘  └─────────┘  └─────────┘ │
    └─────────────────────────────────────────┘
```

### Routing Guidelines

1. **High-Speed Signals**:
   - Keep SPI traces short (<50mm)
   - Match trace impedance (50Ω single-ended)
   - Avoid vias on high-speed signals
   - Use ground planes for return paths

2. **Power Distribution**:
   - Use thick traces for power (>0.5mm)
   - Add multiple vias for power/ground
   - Place bypass capacitors close to ICs
   - Separate analog and digital supplies

3. **RF Considerations**:
   - Keep antenna feed line short
   - Use coplanar waveguide for impedance control
   - Add test points for tuning
   - Maintain ground plane continuity

## Assembly Instructions

### Step 1: Prepare Components
```
Required Tools:
- Soldering iron (temperature controlled)
- Solder (0.6mm rosin core)
- Flux paste
- Tweezers
- Multimeter
- Oscilloscope (optional)

Components Checklist:
□ ESP32-CAM module
□ LoRa module (SX1276/SX1262/etc.)
□ Resistors (1kΩ, 10kΩ)
□ Capacitors (100nF, 10µF, 100µF)
□ Connectors (JST, USB, etc.)
□ PCB or breadboard
□ Wire (30AWG for signals, 22AWG for power)
```

### Step 2: Power System Assembly
```
1. Install charge controller circuit:
   - Solder TP4056 module
   - Add protection diode (1N4007)
   - Connect battery connector
   - Test charging function

2. Install voltage regulator:
   - Solder AMS1117-3.3 (if needed)
   - Add input/output capacitors
   - Test 3.3V output under load

3. Add monitoring circuit:
   - Voltage divider for battery monitoring
   - Current shunt for power measurement
   - Connect to ESP32 ADC pins
```

### Step 3: Digital Circuit Assembly  
```
1. Install ESP32-CAM:
   - Socket or direct solder
   - Test all GPIO connections
   - Verify camera functionality

2. Install LoRa module:
   - Use socket for easy replacement
   - Add bypass capacitors nearby
   - Test SPI communication

3. Make interconnections:
   - Follow wiring diagram exactly
   - Use color-coded wires
   - Add test points for debugging
```

### Step 4: RF Circuit Assembly
```
1. Install antenna connector:
   - Use SMA or U.FL connector
   - Maintain 50Ω impedance
   - Keep connections short

2. Install antenna:
   - Connect to LoRa module
   - Test VSWR < 2:1
   - Verify frequency response

3. RF shielding (optional):
   - Add copper tape shield
   - Connect shield to ground
   - Test for reduced interference
```

## Testing and Validation

### Functional Tests

1. **Power System Test**:
   ```
   □ Solar charging works
   □ Battery protection active
   □ 3.3V regulation stable
   □ Current consumption reasonable
   ```

2. **Communication Test**:
   ```
   □ ESP32-CAM boots properly
   □ LoRa module detected
   □ SPI communication working
   □ Camera captures images
   ```

3. **RF Performance Test**:
   ```
   □ Antenna VSWR < 2:1
   □ Transmission power correct
   □ Reception sensitivity good
   □ Range meets expectations
   ```

### Performance Measurements

1. **Power Consumption**:
   ```
   Mode                Current (mA)
   Deep Sleep         0.1 - 1
   LoRa RX            15 - 20
   LoRa TX (20dBm)    120 - 150
   Camera Active      200 - 300
   Peak (TX+Camera)   350 - 450
   ```

2. **RF Performance**:
   ```
   Parameter          Typical Value
   TX Power           +20 dBm
   RX Sensitivity     -137 dBm
   Frequency Error    < ±20 ppm
   VSWR               < 1.5:1
   ```

3. **Range Testing**:
   ```
   Environment        Range (km)
   Open Field         10 - 15
   Suburban           3 - 5
   Urban              1 - 2
   Forest             2 - 4
   Indoor             0.1 - 0.3
   ```

## Troubleshooting Guide

### Common Problems and Solutions

1. **No LoRa Communication**:
   - Check power supply voltage (3.3V ±5%)
   - Verify SPI wiring (MOSI/MISO not swapped)
   - Test with simple ping program
   - Check antenna connection

2. **Poor Range Performance**:
   - Measure antenna VSWR
   - Check TX power setting
   - Verify line-of-sight path
   - Test different antennas

3. **High Power Consumption**:
   - Check for stuck TX mode
   - Verify sleep mode operation
   - Look for ground loops
   - Test individual components

4. **Intermittent Operation**:
   - Check solder joints
   - Test under temperature variation
   - Look for power supply noise
   - Verify timing relationships

### Debug Procedures

1. **Signal Integrity Check**:
   ```
   Use oscilloscope to verify:
   □ SPI clock signal clean
   □ Data signals have proper levels
   □ No ringing or overshoot
   □ Ground bounce minimal
   ```

2. **Power Quality Check**:
   ```
   Measure with multimeter/scope:
   □ 3.3V rail regulation
   □ Ripple < 50mV
   □ Load regulation good
   □ Transient response stable
   ```

3. **RF Performance Check**:
   ```
   Use spectrum analyzer/VNA:
   □ Carrier frequency accurate
   □ Spurious emissions low
   □ Antenna match good
   □ Modulation clean
   ```

## Maintenance and Upgrades

### Routine Maintenance
- Clean solar panel monthly
- Check battery voltage quarterly  
- Inspect antenna connections annually
- Update firmware as available

### Upgrade Paths
- Higher capacity battery
- More efficient solar panel
- Better antenna system
- Additional sensors
- Mesh network expansion

## Safety Considerations

### Electrical Safety
- Use proper ESD precautions
- Verify polarity before powering
- Add fuse protection
- Use weatherproof enclosures

### RF Safety  
- Follow local power limits
- Use proper antenna impedance
- Avoid transmitting near people
- Monitor duty cycle limits

### Environmental
- Use UV-resistant materials
- Waterproof all connections
- Consider lightning protection
- Plan for temperature extremes