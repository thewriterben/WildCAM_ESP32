# ESP32-S3-CAM Assembly Guide

*Complete assembly instructions for ESP32-S3-CAM wildlife camera system*

## Overview

This guide provides step-by-step instructions for assembling a complete ESP32-S3-CAM wildlife camera system using the 3D printed components in this repository. The assembly process is designed for both beginners and experienced makers.

## Required Components

### 3D Printed Parts
- `ai_thinker_main_enclosure.stl` - Main weatherproof housing
- `ai_thinker_front_cover.stl` - Front panel with camera opening
- `ai_thinker_back_cover.stl` - Back panel with access ports
- `battery_compartment.stl` - Battery housing
- `camera_lens_hood.stl` - Lens protection
- `solar_panel_mount_bracket.stl` - Solar panel mount (optional)
- `tree_mounting_strap_clips.stl` - Tree mounting hardware
- `ventilation_grilles.stl` - Heat management (2 pieces)

### Electronic Components
- **AI Thinker ESP32-S3-CAM board** - Main controller
- **OV2640/OV5640 camera module** - Usually included with board
- **18650 Li-ion battery** (3.7V, 3000mAh minimum)
- **Solar panel** (5V-6V, 1-10W) - Optional but recommended
- **MicroSD card** (32GB Class 10 recommended)
- **PIR motion sensor** (HC-SR501 or similar) - Optional

### Hardware Components
- **M3 x 5mm threaded inserts** (8 pieces)
- **M3 x 12mm stainless steel screws** (6 pieces)
- **M3 x 8mm screws** (4 pieces)
- **PG7 cable glands** (2 pieces)
- **2mm thick rubber gasket** (90x70mm)
- **Breathable membrane** (50x50mm, Gore-Tex or equivalent)
- **Silicone sealant** (clear, outdoor grade)
- **Tree mounting strap** (25-50mm width, 2m length)

### Tools Required
- Soldering iron (for threaded inserts)
- Phillips screwdrivers (PH1, PH2)
- Drill with bits (2.5mm, 6mm, 12.5mm)
- M3 tap and handle
- File or sandpaper (220-400 grit)
- Multimeter
- Heat gun (optional)

## Pre-Assembly Preparation

### 1. Print Quality Check
- Inspect all printed parts for defects
- Remove support material carefully
- Test-fit all components before assembly
- Sand any rough surfaces that will contact gaskets

### 2. Threaded Insert Installation
```
Temperature: 200°C (for PETG/ASA)
Insert Depth: Flush with surface
Cooling: Allow 30 seconds before handling
```

**Main Enclosure Inserts:**
- 4x inserts in mounting posts (for ESP32-S3-CAM)
- 4x inserts around rim (for cover attachment)

**Battery Compartment Inserts:**
- 2x inserts in access panel

### 3. Gasket Preparation
- Cut rubber gasket to fit lid groove (88x68mm)
- Test gasket fit - should compress slightly when lid closed
- Apply thin layer of silicone grease for better sealing

## Main Assembly Process

### Step 1: ESP32-S3-CAM Installation

1. **Board Preparation**
   - Flash firmware before installation
   - Insert MicroSD card
   - Test camera function
   - Note antenna orientation for optimal placement

2. **Mounting in Enclosure**
   ```
   Board Position: Centered, camera facing forward
   Mounting Screws: M3 x 8mm (4 pieces)
   Antenna Route: Through designated channel
   USB-C Access: Aligned with back panel opening
   ```

3. **Camera Lens Alignment**
   - Ensure camera lens aligns with front panel opening
   - Adjust board position if necessary
   - Tighten mounting screws gradually in cross pattern

### Step 2: Power System Assembly

1. **Battery Compartment**
   - Install battery contacts (positive/negative terminals)
   - Route power cables through designated channels
   - Test battery fit and removal mechanism
   - Apply thread locker to compartment screws

2. **Power Wiring**
   ```
   Battery +: ESP32-S3-CAM VIN (via switch)
   Battery -: ESP32-S3-CAM GND
   Solar +: Charging circuit input (optional)
   Solar -: Charging circuit ground (optional)
   ```

### Step 3: Weatherproofing

1. **Cable Gland Installation**
   ```
   Front Gland: Sensor cables (if used)
   Rear Gland: Power/solar cables
   Hole Size: 12.5mm for PG7 glands
   Sealant: Marine-grade silicone around threads
   ```

2. **Gasket Installation**
   - Place gasket in groove around enclosure rim
   - Ensure even compression around entire perimeter
   - Test lid closure - should require moderate pressure

3. **Ventilation Setup**
   - Install breathable membrane behind ventilation grilles
   - Secure grilles with M3 x 8mm screws
   - Ensure membrane is taut but not torn

### Step 4: Final Assembly

1. **Front Cover Installation**
   ```
   Camera Hood: Press-fit or snap-on design
   LED Window: Align with status indicators
   Lens Protection: Ensure clear optical path
   Screws: M3 x 12mm (4 pieces around perimeter)
   ```

2. **Back Cover Installation**
   ```
   USB-C Access: Verify port alignment
   Ventilation: Check grille alignment
   Cable Management: Route cables neatly
   Screws: M3 x 12mm (4 pieces)
   ```

3. **System Testing**
   - Power on test (LED indicators)
   - Camera function test
   - Motion detection test (if equipped)
   - WiFi connectivity test
   - Weatherproof seal test (spray bottle)

## Mounting Options

### Tree Mounting

1. **Strap Clip Installation**
   ```
   Strap Width: 25-50mm compatible
   Installation: Slide strap through clips
   Tree Diameter: 5-25cm optimal
   Height: 2-4m recommended for wildlife
   ```

2. **Positioning Guidelines**
   ```
   Camera Angle: 15-30° downward tilt
   Direction: North-facing to avoid sun glare
   Clearance: 3m minimum detection range
   Access: Consider maintenance accessibility
   ```

## Performance Specifications

### Expected Performance
```
Battery Life: 2-4 weeks (without solar)
Solar Battery Life: Indefinite (with adequate sun)
Operating Temperature: -20°C to +60°C
Weather Rating: IP65 (with proper assembly)
Detection Range: 3-7 meters (PIR dependent)
Image Resolution: Up to 5MP (sensor dependent)
Video Recording: 1080p at 30fps
Storage Capacity: Limited by SD card size
WiFi Range: 50-100m (environment dependent)
```

## Safety Considerations

### Electrical Safety
- Always disconnect power before servicing
- Use proper ESD precautions
- Verify polarity before connections
- Test circuits with multimeter

### Environmental Safety
- Use non-toxic materials only
- Ensure no wildlife entanglement hazards
- Follow local regulations for wildlife monitoring
- Minimize habitat disturbance

---

*This assembly guide ensures professional-grade wildlife camera deployment with the ESP32-S3-CAM platform. Follow all steps carefully for optimal performance and longevity.*

**Document Version**: 1.0  
**Last Updated**: December 2024  
**Compatibility**: ESP32-S3-CAM v1.0+