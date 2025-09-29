# Tropical Climate Variant STL Files

## Overview
This directory contains STL files specifically designed for tropical climate deployments with enhanced waterproofing, mold resistance, and corrosion protection.

## STL File Descriptions

### 1. tropical_main_enclosure.stl
**Purpose**: Primary weatherproof housing with enhanced tropical features
**Dimensions**: 95mm x 75mm x 55mm (internal)
**Features**:
- 3.5mm wall thickness for enhanced durability
- Integrated drainage channels at base
- Multiple cable entry points with marine-grade gland compatibility
- Reinforcement ribs for impact resistance
- Mounting posts for ESP32-CAM boards

**Print Settings**:
- Layer Height: 0.2mm
- Infill: 25%
- Wall Count: 4 (1.6mm walls)
- Support: Required for overhangs
- Material: ASA or PETG with anti-fungal additives

### 2. tropical_drainage_system.stl
**Purpose**: Integrated drainage and water management system
**Features**:
- One-way drainage valves
- Overflow channels for extreme precipitation
- Debris filtering screens
- Anti-backflow design

**Print Settings**:
- Layer Height: 0.15mm (fine detail for drainage channels)
- Infill: 100% (solid for water pressure resistance)
- Support: Minimal, designed for easy printing
- Material: ASA (chemical resistance for marine environments)

### 3. tropical_ventilation_assembly.stl
**Purpose**: Active ventilation system for humidity control
**Features**:
- Gore-Tex membrane mounting points
- 12V micro-fan mounting bracket
- Intake and exhaust ports
- Filter mounting system

**Print Settings**:
- Layer Height: 0.2mm
- Infill: 30%
- Support: Required for fan mounting bracket
- Material: ASA (temperature resistance for fan operation)

### 4. tropical_lens_protection.stl
**Purpose**: Enhanced lens shield with hydrophobic features
**Features**:
- Extended rain hood design
- Impact-resistant polycarbonate shield mounting
- Drainage channels around lens perimeter
- Anti-fog heating element mounting

**Print Settings**:
- Layer Height: 0.15mm (smooth surface for optical clarity)
- Infill: 20%
- Support: Minimal (designed with printing in mind)
- Material: Clear PETG or ASA

### 5. tropical_mounting_brackets.stl
**Purpose**: Corrosion-resistant mounting system
**Features**:
- 316 stainless steel hardware compatibility
- Multiple mounting angles (0°, 15°, 30°, 45°)
- Anti-twist locking mechanisms
- Vibration dampening features

**Print Settings**:
- Layer Height: 0.2mm
- Infill: 50% (high strength for mounting loads)
- Support: Required for mounting holes
- Material: ASA (long-term outdoor durability)

### 6. tropical_cable_glands.stl
**Purpose**: Marine-grade waterproof cable entry system
**Features**:
- IP68 rated seal design
- Multiple cable diameter compatibility (4-12mm)
- Strain relief integration
- Corrosion-resistant threading

**Print Settings**:
- Layer Height: 0.15mm (precise threading)
- Infill: 100% (pressure resistance)
- Support: None (designed for vertical printing)
- Material: ASA (chemical resistance)

### 7. tropical_gasket_profiles.stl
**Purpose**: Dual-seal gasket system templates
**Features**:
- Primary and secondary seal channels
- Marine silicone compatibility
- Pressure equalization passages
- Easy replacement design

**Print Settings**:
- Layer Height: 0.1mm (precision gasket channels)
- Infill: 100% (structural integrity)
- Support: None
- Material: ASA (dimensional stability)

## Assembly Order
1. Print all components with specified settings
2. Install tropical_drainage_system into main enclosure
3. Mount tropical_ventilation_assembly with fan and filters
4. Install tropical_cable_glands with marine sealant
5. Attach tropical_lens_protection with hydrophobic coating
6. Mount assembly using tropical_mounting_brackets
7. Install tropical_gasket_profiles with marine silicone

## Post-Processing Requirements

### Surface Treatment
- Sand all mating surfaces to 400 grit
- Apply primer suitable for marine environments
- Apply hydrophobic nano-coating to exterior surfaces
- Install marine-grade silicone gaskets

### Hardware Installation
- Use only 316 stainless steel fasteners
- Apply marine-grade thread locker
- Install sacrificial zinc anodes if electrical system present
- Apply dielectric grease to electrical connections

## Material Recommendations

### Primary Materials
1. **ASA (Acrylonitrile Styrene Acrylate)**
   - Excellent UV resistance
   - Chemical resistance to salt spray
   - Temperature stability (-40°C to +80°C)
   - Recommended brands: eSUN ASA+, SUNLU ASA

2. **PETG with Additives**
   - Anti-microbial additives for mold resistance
   - Chemical resistance
   - Easy printing characteristics
   - Lower cost alternative to ASA

### Coating Requirements
- **Hydrophobic Coating**: Rain-X or similar nano-ceramic coating
- **Anti-Fungal Treatment**: Copper-based antimicrobial additives
- **Marine Primer**: Corrosion-resistant primer for metal contact areas

## Testing and Validation

### Pressure Testing
1. Submerge completed assembly to 1.5 bar (15m depth)
2. Hold pressure for 24 hours
3. Check for leaks using leak detection fluid
4. Verify drainage system operation under pressure

### Environmental Testing
1. **Salt Spray Test**: 96 hours ASTM B117
2. **UV Exposure**: 200 hours accelerated weathering
3. **Thermal Cycling**: -10°C to +50°C, 20 cycles
4. **Humidity Test**: 95% RH at 40°C for 168 hours

### Functional Testing
1. Install ESP32-CAM and verify operation
2. Test camera clarity through lens protection
3. Verify ventilation system airflow
4. Check drainage under simulated heavy rain

## Maintenance Schedule

### Weekly
- Visual inspection for cracks or damage
- Clear drainage channels of debris
- Check mounting system tightness

### Monthly
- Deep clean all surfaces
- Inspect gaskets for wear
- Test drainage system flow rate
- Check lens protection clarity

### Quarterly
- Replace ventilation filters
- Refresh hydrophobic coating
- Replace desiccant packets
- Comprehensive pressure test

### Annually
- Complete disassembly and cleaning
- Replace all gaskets and seals
- Hardware inspection and replacement
- Reapply protective coatings

## Troubleshooting

### Common Issues
1. **Moisture Ingress**: Check gasket installation and drainage system
2. **Mold Growth**: Verify ventilation system operation and anti-fungal treatment
3. **Corrosion**: Inspect hardware and sacrificial anodes
4. **Lens Fogging**: Check heating element operation and gasket seals

### Performance Optimization
- Adjust ventilation fan speed based on humidity levels
- Optimize drainage channel angles for local precipitation patterns
- Customize mounting angles for optimal solar panel exposure
- Adapt cable routing for specific sensor configurations

## Customization Options

### Site-Specific Modifications
- Adjust mounting bracket angles for tree installation
- Modify cable gland count for additional sensors
- Scale enclosure size for larger battery systems
- Add solar panel integration points

### Hardware Adaptations
- ESP32-S3-CAM compatibility modifications
- Additional sensor mounting points
- LoRa antenna integration
- Satellite communication hardware support

## Quality Control Checklist

Before deployment, verify:
- [ ] All components printed within tolerance (±0.1mm)
- [ ] Surface finish suitable for gasket sealing
- [ ] Mounting hardware properly torqued
- [ ] Drainage system flows freely
- [ ] Ventilation system operates correctly
- [ ] Camera lens protection clear and sealed
- [ ] Pressure test passed (1.5 bar, 24 hours)
- [ ] Environmental protection coatings applied
- [ ] Assembly documentation complete

## Support and Updates

For technical support or design improvements:
1. Check project GitHub issues for known problems
2. Review field deployment reports for real-world performance
3. Submit improvement suggestions with test data
4. Participate in community testing programs

---

*These STL files are optimized for tropical climate deployment and have been tested in rainforest environments. Always perform site-specific testing before permanent installation.*