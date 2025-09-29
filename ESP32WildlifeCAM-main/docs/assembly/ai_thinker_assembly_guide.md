# AI-Thinker ESP32-CAM Assembly Guide

## Overview
This guide provides step-by-step instructions for assembling the AI-Thinker ESP32-CAM Wildlife Camera enclosure system.

## Required Components

### 3D Printed Parts
- `ai_thinker_main_enclosure.stl` - Main weatherproof housing
- `ai_thinker_front_cover.stl` - Front panel with camera cutout
- `ai_thinker_back_cover.stl` - Back panel with cable access
- `solar_panel_mount_bracket.stl` - Solar panel mounting system
- `tree_mounting_strap_clips.stl` - Tree mounting hardware
- `ventilation_grilles.stl` - Weatherproof ventilation
- `camera_lens_hood.stl` - Lens protection and glare reduction
- `battery_compartment.stl` - Secure battery housing

### Electronic Components
- AI-Thinker ESP32-CAM board
- OV2640 camera module (usually included)
- 18650 Li-ion battery
- Solar panel (6V, 1W minimum)
- PIR motion sensor
- Micro SD card (32GB recommended)

### Hardware
- M3 x 8mm screws (8 pieces)
- M3 x 12mm screws (4 pieces)
- O-ring gaskets for weatherproofing
- Cable glands
- Mounting straps or clamps

## Assembly Steps

### Step 1: Prepare the Main Enclosure
1. Insert O-ring gaskets into groove channels on `ai_thinker_main_enclosure`
2. Test fit all components before final assembly
3. Check that ventilation grilles align properly

### Step 2: Install Electronics
1. Mount ESP32-CAM board in main enclosure
2. Connect PIR sensor to designated GPIO pins
3. Install battery compartment and connect power leads
4. Secure all connections with appropriate strain relief

### Step 3: Camera Assembly
1. Attach camera lens hood to front cover
2. Ensure camera module is properly aligned
3. Test camera functionality before sealing

### Step 4: Solar Panel Installation
1. Mount solar panel to bracket
2. Attach bracket to main enclosure
3. Route solar panel cables through cable glands
4. Connect to charging circuit

### Step 5: Final Assembly
1. Install front and back covers with gaskets
2. Secure with M3 screws
3. Test all functions before deployment
4. Apply weatherproofing compound as needed

## Testing Procedures
- Power-on test
- Camera capture test
- Motion detection test
- Solar charging verification
- Weatherproofing check

## Troubleshooting

### Common Assembly Issues
- **Camera not detected**: Check cable connections and seating
- **Power issues**: Verify battery polarity and connections
- **Moisture ingress**: Inspect gasket placement and cover sealing
- **Mount instability**: Ensure proper strap tension and bracket alignment

### Performance Optimization
- Position camera to avoid direct sunlight
- Orient solar panel for maximum sun exposure
- Place PIR sensor away from heat sources
- Ensure adequate ventilation

## Maintenance Schedule
- Monthly: Battery voltage check
- Quarterly: Clean solar panel and camera lens
- Annually: Replace O-ring gaskets
- As needed: Update firmware

## Safety Considerations
- Always power down before assembly
- Use ESD protection when handling electronics
- Verify weatherproofing before outdoor deployment
- Follow local regulations for wildlife monitoring

---
*For technical support, refer to the troubleshooting documentation or open an issue on GitHub.*