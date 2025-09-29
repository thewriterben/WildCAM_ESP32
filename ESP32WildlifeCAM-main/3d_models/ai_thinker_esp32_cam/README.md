# AI-Thinker ESP32-CAM Enclosures

This directory contains 3D printable enclosures specifically designed for the AI-Thinker ESP32-CAM board.

## Enclosure Components

### Main Housing
- **ai_thinker_main_enclosure.stl** - Primary weatherproof housing
  - Dimensions: 85mm x 65mm x 45mm (internal)
  - IP54+ weather protection (with proper assembly)
  - Camera lens opening: 12mm diameter
  - Antenna access port
  - Ventilation channels for heat dissipation

### Access Covers
- **ai_thinker_front_cover.stl** - Transparent front cover for camera
  - UV-resistant clear PETG recommended
  - Sealed camera lens protection
  - Optional IR filter mounting

- **ai_thinker_back_cover.stl** - Hinged access panel
  - Secure latching mechanism
  - Cable routing channels
  - Programming port access

### Mounting System
- **solar_panel_mount_bracket.stl** - Adjustable solar panel mount
  - Fits standard 5V solar panels (80-120mm)
  - Tilt adjustment: 0-45°
  - Secure panel attachment points

- **tree_mounting_strap_clips.stl** - Tree mounting clips
  - Compatible with 25-50mm tree straps
  - Anti-slip design
  - Weather-resistant mounting points

### Ventilation
- **ventilation_grilles.stl** - Condensation prevention grilles
  - Breathable membrane compatible
  - Insect protection
  - Maintains weather seal

## Board Specifications

### AI-Thinker ESP32-CAM Dimensions
- PCB: 40.5mm x 27mm x 4.5mm
- Camera module: 8.5mm x 8.5mm x 5.5mm
- Antenna: Built-in PCB antenna
- Power connector: Micro-USB or external pins

### Pin Access Requirements
```cpp
// Critical pins requiring external access
#define PROGRAMMING_TX_PIN    1    // Serial programming (TX)
#define PROGRAMMING_RX_PIN    3    // Serial programming (RX)  
#define RESET_PIN           N/A    // Reset button (not connected on AI-Thinker)
#define GPIO_FLASH_PIN        0    // Flash button access (GPIO0)
#define EXTERNAL_ANTENNA    N/A    // Optional external antenna (not implemented)
```

## Print Settings

### Recommended Materials
1. **PETG** (Primary choice)
   - Weather resistant
   - UV stable
   - Easy to print
   - Good chemical resistance

2. **ASA** (Premium choice)
   - Excellent UV resistance
   - High temperature resistance
   - Professional appearance
   - Requires enclosed printer

3. **ABS** (Alternative)
   - Good temperature resistance
   - Strong layer adhesion
   - Post-processing friendly
   - Acetone smoothing possible

### Print Settings
```
Layer Height: 0.2mm
Infill: 25% (walls), 100% (mounting points)
Wall Thickness: 1.2mm (3 perimeters)
Top/Bottom Layers: 5 layers
Support: Required for overhangs >45°
Print Speed: 40-60mm/s
```

## Assembly Instructions

### Step 1: Print Components
1. Print main enclosure in weather-resistant material
2. Print covers with appropriate material (clear PETG for front)
3. Print mounting hardware with 100% infill for strength

### Step 2: Hardware Installation
1. Install threaded inserts (M3 x 5mm recommended)
2. Install rubber gasket in lid groove
3. Test fit all components before final assembly

### Step 3: Electronics Assembly
1. Mount ESP32-CAM board using provided mounting posts
2. Route antenna through designated channel
3. Install solar charging circuit if used
4. Connect external sensors through cable glands

### Step 4: Weatherproofing
1. Apply silicone sealant to all cable entry points
2. Install breathable membrane in ventilation ports  
3. Test seal integrity before deployment

> **Detailed weatherproofing guidance:** See `../common/README.md` for comprehensive gasket installation, sealant application, and testing procedures.

## Customization Options

### Size Variants
- **Compact**: Minimal size for basic deployment
- **Extended**: Additional space for sensors and batteries
- **Professional**: Larger enclosure for research applications

### Mounting Options
- Tree strap mounting (included)
- Wall mount bracket (separate file)
- Tripod mounting (standard 1/4" thread)
- Ground stake mount (separate file)

### Environmental Adaptations
- **Desert**: Light colors, enhanced ventilation
- **Tropical**: Extended drainage, anti-fungal treatment
- **Arctic**: Insulated version, heating elements
- **Urban**: Stealth colors, vibration dampening

## Quality Control

### Pre-Deployment Testing
1. **Dimensional Check**: Verify all tolerances
2. **Weather Seal Test**: 24-hour water immersion
3. **Temperature Cycling**: -20°C to 60°C test
4. **UV Exposure**: 100-hour UV chamber test
5. **Drop Test**: 1.5m drop on concrete

### Field Validation
- Monitor for condensation in first 48 hours
- Check seal integrity after 30 days
- Inspect mounting hardware quarterly
- Replace gaskets annually

## Troubleshooting

### Common Issues
1. **Condensation**: Improve ventilation, add desiccant
2. **Loose fit**: Check print tolerances, adjust slicer settings
3. **Water ingress**: Inspect gaskets, reseal cable entries
4. **Mounting failure**: Use stainless steel hardware, apply thread locker

### Print Quality Issues
1. **Warping**: Use heated bed, enclosure, better bed adhesion
2. **Poor overhangs**: Increase support density, optimize angles
3. **Layer adhesion**: Increase temperature, reduce cooling
4. **Stringing**: Optimize retraction settings

## Downloads and Sources

### Open Source Designs
The STL files in this directory are based on open-source designs and community contributions:

- Base enclosure design derived from ESP32-CAM community projects
- Mounting systems adapted from trail camera designs
- Weather sealing targets IP54+ protection with proper gasket installation

### Design Credits
- Original ESP32-CAM enclosure concept: ESP32 community
- Weather sealing design: Industrial enclosure standards
- Mounting systems: Trail camera and wildlife monitoring community
- Solar integration: DIY solar project references

### License
These designs are released under Creative Commons Attribution-ShareAlike 4.0 International License.
You are free to share, adapt, and use commercially with proper attribution.

## Support and Community

### Getting Help
1. Check the troubleshooting section above
2. Review the main project documentation
3. Submit issues to the project repository
4. Join the ESP32 wildlife monitoring community

### Contributing
1. Submit improved designs via pull request
2. Share field test results and improvements
3. Document successful deployments
4. Report any design issues or failures

---

*Last updated: August 2025*
*Design version: 1.0*
*Compatible with: AI-Thinker ESP32-CAM v1.6+*