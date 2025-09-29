# Common 3D Printing Components

This directory contains shared components that work across multiple ESP32 board types and enclosure designs.

## Universal Mounting Hardware

### mounting_hardware.stl
Universal mounting bracket system compatible with multiple enclosure types:
- Standard tripod mount (1/4"-20 thread)
- Wall mounting holes (M4 spacing)
- Tree strap attachment points
- Adjustable positioning joints

### Design Features
```
Material: PETG or ABS (100% infill for strength)
Mounting options:
  - Tripod: Standard 1/4" camera thread
  - Wall: M4 bolts, 50mm spacing
  - Tree: Strap slots for 25-50mm straps
  - Pole: Clamp mechanism for 20-60mm poles
```

## Cable Management

### cable_management.stl
Standardized cable routing and strain relief system:
- Multiple cable diameters (3-8mm)
- Waterproof cable gland integration
- Strain relief protection
- Organized cable routing channels

### Specifications
```
Cable gland sizes: PG7, PG9, PG11
Compatible cables:
  - USB: Mini, Micro, Type-C
  - Power: 2.1mm barrel, screw terminals
  - Antenna: SMA, RP-SMA connectors
  - Sensor: JST, Dupont, I2C
```

## Weatherproofing Components

### weatherproofing_gaskets.stl
Printable rubber gasket profiles for different sealing requirements:
- Lid seals (round and rectangular)
- Cable entry seals
- Lens protection seals
- Ventilation membrane frames

### Material Notes
```
Gasket printing:
  - Use flexible filament (TPU, TPE)
  - Shore A hardness: 85-95
  - Print orientation: Flat on bed
  - No supports required
  
Alternative gaskets:
  - Commercial rubber strips
  - Silicone sealant application
  - Weather stripping material
```

## Print Settings for Shared Components

### Mounting Hardware
```
Material: PETG/ABS
Layer Height: 0.2mm
Infill: 100% (critical for strength)
Walls: 4 perimeters minimum
Support: Required for threaded holes
Post-processing: Tap threads, test fit
```

### Cable Management
```
Material: PETG (chemical resistance)
Layer Height: 0.15mm (smooth cable contact)
Infill: 25%
Walls: 3 perimeters
Support: Minimal bridging design
```

### Gaskets
```
Material: TPU (flexible)
Layer Height: 0.1mm (smooth surface)
Infill: 100%
Speed: 20mm/s (flexible material)
Support: None (flat design)
```

## Assembly Integration

### Mounting Points
All enclosure designs include standardized mounting points:
- M3 threaded inserts (recommended)
- M3 through-holes (alternative)
- 1/4" tripod thread compatibility
- Universal bracket attachment

### Cable Routing
Consistent cable management across designs:
- Bottom entry (preferred for weather protection)
- Side entry (easier access)
- Multiple cable support
- Strain relief integration

### Gasket Installation
Standard gasket groove profiles:
- 2mm groove depth
- 3mm groove width
- Compatible with printed and commercial gaskets
- Compression fit design

## Customization Guide

### Scaling Components
```bash
# For different board sizes
Scale factor = New_board_size / Reference_board_size

# Example: Larger board 1.2x size
Scale mounting hardware by 1.2x
Keep cable management same size
Scale gaskets proportionally
```

### Material Substitutions
```
Mounting Hardware:
  PETG → ABS: Same settings, better chemicals
  PETG → Nylon: +20°C temp, slower speed
  PETG → PLA: Temporary use only

Cable Management:
  PETG → TPU: Flexible, harder to print
  PETG → ASA: Better UV resistance
  
Gaskets:
  TPU → Silicone sealant: Better seal, messier
  TPU → Commercial gasket: Professional option
```

## Quality Control

### Test Procedures
1. **Dimensional Accuracy**
   - Measure mounting hole spacing
   - Verify thread pitch and depth
   - Check gasket compression fit

2. **Strength Testing**
   - Load test mounting points (5kg minimum)
   - Torque test threaded connections
   - Drop test assembled units

3. **Weather Resistance**
   - Water spray test (IP54 minimum)
   - Temperature cycling (-10°C to 50°C)
   - UV exposure test (100 hours)

### Common Issues and Solutions
```
Loose mounting threads:
  → Use thread locker
  → Print at 90% scale, tap to size
  → Insert threaded metal inserts

Cable gland leaks:
  → Check gasket compression
  → Apply thread sealant
  → Verify cable diameter match

Gasket failure:
  → Check print orientation
  → Increase wall thickness
  → Consider commercial alternative
```

## Maintenance Schedule

### Quarterly Inspection
- [ ] Check mounting hardware for looseness
- [ ] Inspect gaskets for wear or damage
- [ ] Verify cable strain relief effectiveness
- [ ] Test weatherproofing seal integrity

### Annual Replacement
- [ ] Replace flexible gaskets
- [ ] Re-apply thread locker to hardware
- [ ] Update to improved component designs
- [ ] Document performance and issues

## Design Evolution

### Version History
- **v1.0**: Initial shared component designs
- **v1.1**: Improved weather sealing (planned)
- **v1.2**: Enhanced mounting options (planned)
- **v2.0**: Next-gen board integration (planned)

### Community Contributions
Submit improvements via GitHub:
1. Fork repository
2. Modify STL files or create new variants
3. Test with actual hardware
4. Document improvements and test results
5. Submit pull request with changes

### Future Enhancements
- **Smart mounting**: Motorized positioning
- **Modular cables**: Quick-disconnect systems
- **Advanced sealing**: Multi-stage weather protection
- **Tool-free assembly**: Snap-fit connections

---

*Common components are designed for maximum compatibility*
*Test thoroughly before field deployment*
*Report issues and improvements to help the community*