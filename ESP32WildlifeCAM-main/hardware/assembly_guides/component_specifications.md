# ESP32-S3-CAM Component Specifications

*Detailed technical specifications for all ESP32-S3-CAM hardware components*

## STL File Specifications

### ESP32-S3-CAM Enclosure Components

#### ai_thinker_main_enclosure.stl
```
External Dimensions: 90mm x 70mm x 50mm
Internal Dimensions: 85mm x 65mm x 45mm
Wall Thickness: 2.5mm
Material Volume: ~85 cm³
Print Time: 6-8 hours
Weight: ~170g (PETG)
```

**Features:**
- IP65 weatherproof rating with proper assembly
- Camera lens opening: 12mm diameter
- USB-C access port in rear
- ESP32-S3-CAM board mounting posts (45x30mm spacing)
- Enhanced ventilation channels for AI processing heat
- External antenna connector access
- Cable management channels

**Critical Dimensions:**
- Board mounting hole spacing: 42mm x 27mm
- Camera lens opening center: 45mm from left, 35mm from front
- USB-C port center: 45mm from left, 35mm from back
- Mounting post height: 2.5mm above base

#### ai_thinker_front_cover.stl
```
Dimensions: 90mm x 70mm x 3mm
Camera Opening: 12mm diameter
LED Window: 5mm x 5mm
Material Volume: ~15 cm³
Print Time: 1-2 hours
Weight: ~30g (Clear PETG)
```

**Features:**
- UV-resistant clear material recommended
- Sealed camera lens protection
- Status LED visibility window
- Gasket groove for weatherproofing
- Removable lens hood mounting

**Critical Dimensions:**
- Camera opening center: 45mm x 35mm from corner
- LED window position: 75mm x 60mm from corner
- Gasket groove: 2mm deep x 3mm wide

#### ai_thinker_back_cover.stl
```
Dimensions: 90mm x 70mm x 3mm
USB-C Access: 20mm x 10mm
Cable Glands: 2x PG7 (12.5mm)
Ventilation Slots: 3x 8mm x 6mm
Material Volume: ~18 cm³
Print Time: 1-2 hours
Weight: ~35g (PETG/ASA)
```

**Features:**
- USB-C programming port access
- Weatherproof cable gland entries
- Ventilation grilles with membrane support
- Secure latching mechanism
- Service access panel

**Critical Dimensions:**
- USB-C opening: 20mm x 10mm centered
- Cable gland centers: 15mm x 55mm and 15mm x 15mm
- Ventilation slots: 77-85mm from left edge

#### battery_compartment.stl
```
External Dimensions: 80mm x 25mm x 25mm
Internal Dimensions: 75mm x 20mm x 20mm
Battery Type: 18650 Li-ion
Access Hatch: Tool-free design
Material Volume: ~22 cm³
Print Time: 2-3 hours
Weight: ~45g (PETG)
```

**Features:**
- Secure 18650 battery housing
- Positive/negative terminal access
- Weather sealing gasket groove
- Tool-free battery replacement
- Safety contact protection

**Critical Dimensions:**
- Battery cavity: 68mm x 18.5mm x 18.5mm
- Contact spacing: 65mm center-to-center
- Access hatch: 15mm x 25mm opening

#### camera_lens_hood.stl
```
External Diameter: 30mm
Internal Diameter: 12mm
Height: 20mm
UV Protection: 45° hood angle
Material Volume: ~12 cm³
Print Time: 1-2 hours
Weight: ~20g (ASA)
```

**Features:**
- UV protection and glare reduction
- Rain shield with drainage
- Removable snap-fit design
- Anti-reflective internal surface (black recommended)
- Universal camera lens compatibility

**Critical Dimensions:**
- Lens opening: 12mm diameter
- Hood depth: 20mm from mounting face
- Mounting ring: 25mm outer diameter

### Mounting System Components

#### solar_panel_mount_bracket.stl
```
Dimensions: 120mm x 80mm x 15mm
Panel Support: 80-120mm panels
Tilt Range: 0-45° adjustable
Load Capacity: 2kg maximum
Material Volume: ~45 cm³
Print Time: 4-5 hours
Weight: ~90g (PETG/ASA)
```

**Features:**
- Adjustable tilt mechanism
- Standard solar panel mounting holes
- Corrosion-resistant hardware compatibility
- Camera mount integration points
- Wind load resistance up to 60 km/h

**Critical Dimensions:**
- Panel mounting holes: 100mm x 60mm spacing
- Hinge pin diameter: 6mm
- Camera mount spacing: 100mm centers

#### solar_panel_mount.stl
```
Base Dimensions: 100mm x 60mm x 20mm
Fixed Angle: 30° tilt
Panel Size: Up to 100W (500mm x 300mm)
Material Volume: ~35 cm³
Print Time: 3-4 hours
Weight: ~70g (PETG)
```

**Features:**
- Fixed 30° angle for optimal solar collection
- Lightweight simplified design
- 1/4" tripod screw compatibility
- Integrated cable management
- Universal panel mounting

**Critical Dimensions:**
- Base mounting hole: 6mm diameter
- Panel mounting surface: 30° from horizontal
- Support strut angle: 60° from base

#### tree_mounting_strap_clips.stl
```
Dimensions: 50mm x 30mm x 15mm (per clip)
Strap Width: 25-50mm compatible
Tree Diameter: 5-25cm optimal
Grip Mechanism: Anti-slip teeth
Material Volume: ~18 cm³ (per clip)
Print Time: 2 hours (both clips)
Weight: ~35g (PETG/ASA)
```

**Features:**
- Compatible with standard tree straps
- Anti-slip design prevents movement
- Weather-resistant mounting points
- No bark damage design
- Adjustable tension mechanism

**Critical Dimensions:**
- Strap channel: 25mm wide x 10mm deep
- Anti-slip teeth: 2mm height, 45° angle
- Mount boss: 25mm x 15mm x 5mm

### Accessory Components

#### ventilation_grilles.stl
```
Dimensions: 60mm x 40mm x 8mm (set of 2)
Slot Configuration: 4 slots x 44mm length
Airflow Area: 528 mm² per grille
Membrane Support: Gore-Tex compatible
Material Volume: ~8 cm³ (per grille)
Print Time: 1 hour (both grilles)
Weight: ~15g (PETG)
```

**Features:**
- Insect protection (2mm maximum opening)
- Moisture management design
- Breathable membrane mounting
- Airflow optimization for AI processing heat
- Debris filtering capability

**Critical Dimensions:**
- Slot width: 3mm
- Slot spacing: 3mm (center-to-center: 6mm)
- Membrane recess: 1mm deep

#### basic_wildlife_cam_case.stl
```
External Dimensions: 70mm x 50mm x 35mm
Internal Dimensions: 65mm x 45mm x 30mm
Wall Thickness: 2.5mm
Camera Opening: 20mm x 15mm
Material Volume: ~50 cm³
Print Time: 4-5 hours
Weight: ~100g (PLA/PETG)
```

**Features:**
- Simple rectangular design
- Cost-optimized material usage
- Basic weather protection (IP54)
- Easy assembly and maintenance
- Universal mounting lugs

**Critical Dimensions:**
- Board cavity: 45mm x 30mm x 3mm
- Camera opening: 20mm x 15mm rectangular
- Mounting lugs: 5mm x 10mm x 5mm

## Material Properties

### Recommended Materials

#### PETG (Polyethylene Terephthalate Glycol)
```
Glass Transition: 80°C
Operating Range: -20°C to +60°C
UV Resistance: Good (with additives)
Chemical Resistance: Excellent
Flexibility: Moderate
Cost: Medium
```

**Applications:**
- Main enclosures
- Battery compartments
- Clear front covers

#### ASA (Acrylonitrile Styrene Acrylate)
```
Glass Transition: 110°C
Operating Range: -40°C to +85°C
UV Resistance: Excellent
Chemical Resistance: Good
Flexibility: Low
Cost: High
```

**Applications:**
- Solar panel mounts
- Lens hoods
- Long-term outdoor components

#### PLA (Polylactic Acid)
```
Glass Transition: 60°C
Operating Range: 0°C to +40°C
UV Resistance: Poor
Chemical Resistance: Fair
Flexibility: Low
Cost: Low
```

**Applications:**
- Prototyping
- Basic enclosures (indoor/mild climate)
- Educational demonstrations

### Print Settings by Material

#### PETG Settings
```
Nozzle Temperature: 230-250°C
Bed Temperature: 70-80°C
Print Speed: 40-60 mm/s
Retraction: 3-5mm at 40 mm/s
Cooling: 30-50% after first layer
```

#### ASA Settings
```
Nozzle Temperature: 250-270°C
Bed Temperature: 90-110°C
Print Speed: 30-50 mm/s
Retraction: 2-4mm at 40 mm/s
Cooling: 10-30% maximum
Enclosure: Recommended
```

#### PLA Settings
```
Nozzle Temperature: 200-220°C
Bed Temperature: 50-60°C
Print Speed: 50-80 mm/s
Retraction: 4-6mm at 60 mm/s
Cooling: 100% after first layer
```

## Hardware Specifications

### Fasteners and Hardware

#### Threaded Inserts
```
Type: M3 x 5mm heat-set inserts
Material: Brass
Installation: 200°C soldering iron
Torque Rating: 1.5 Nm maximum
Quantity Required: 16 total (various components)
```

#### Screws
```
M3 x 8mm (4): ESP32-S3-CAM board mounting
M3 x 12mm (8): Cover attachment
M3 x 16mm (4): Heavy-duty mounting applications
Material: 316 stainless steel (marine grade)
Head Type: Phillips pan head
```

#### Gaskets and Seals
```
Material: EPDM rubber (outdoor grade)
Thickness: 2mm standard
Hardness: Shore A 70
Temperature Range: -40°C to +120°C
UV Resistance: 10+ years outdoor exposure
```

### Electronic Component Compatibility

#### ESP32-S3-CAM Variants
```
AI Thinker ESP32-S3-CAM: Primary target
Freenove ESP32-S3-WROOM: Compatible
TTGO T-Camera S3: Requires minor modifications
Generic ESP32-S3-CAM: Usually compatible
```

#### Camera Modules
```
OV2640: 2MP, standard compatibility
OV3660: 3MP, direct replacement
OV5640: 5MP, requires firmware support
Custom modules: Check mounting compatibility
```

#### Power Requirements
```
Operating Voltage: 3.3V-5V
Current Consumption: 
  - Deep Sleep: <10μA
  - WiFi Active: 150-200mA
  - Camera Active: 200-300mA
  - AI Processing: 400-500mA peak
Battery Life: 2-4 weeks (3000mAh, typical usage)
```

## Performance Specifications

### Environmental Ratings

#### Weather Resistance
```
Rating: IP65 (with proper assembly)
Test Standards: IEC 60529
Water Protection: Jets from any direction
Dust Protection: Complete protection
Temperature Cycling: -20°C to +60°C
Humidity: 0-95% RH non-condensing
```

#### Mechanical Properties
```
Impact Resistance: IK08 (5 Joules)
Vibration: 10-500 Hz, 2g acceleration
UV Exposure: 1000 hours minimum (ASA)
Chemical Resistance: pH 4-10 solutions
Thermal Expansion: <0.1% over operating range
```

### Operational Specifications

#### Camera Performance
```
Resolution: Up to 5MP (sensor dependent)
Frame Rate: 30fps at 1080p
Detection Range: 3-7m (PIR sensor)
Field of View: 60-120° (lens dependent)
Night Vision: IR illumination compatible
Storage: MicroSD up to 128GB
```

#### Connectivity
```
WiFi Range: 50-100m (open field)
Bluetooth: 10m typical range
Antenna Options: PCB trace or external
Data Transfer: HTTP/FTP/MQTT protocols
Remote Access: Web interface, mobile app
```

#### Power Management
```
Solar Charging: 5-6V, 1-10W panels
Battery Monitoring: Voltage and current
Low Power Modes: Deep sleep <10μA
Charging Control: MPPT or linear
Battery Protection: Over/under voltage
```

## Quality Control Standards

### Dimensional Tolerances
```
Critical Interfaces: ±0.1mm
General Dimensions: ±0.2mm
Hole Diameters: +0.1/-0.0mm
Thread Fits: ISO 2768-m standard
Surface Finish: Ra 3.2μm maximum
```

### Testing Requirements
```
Fit Testing: All mating parts
Pressure Testing: 1.5x operating pressure
Temperature Testing: -25°C to +65°C
UV Testing: 500 hours minimum exposure
Vibration Testing: 10g for 2 hours
```

### Acceptance Criteria
```
Dimensional: Within specified tolerances
Surface Quality: No cracks, voids, or delamination
Fit and Function: Smooth operation of all mechanisms
Weather Sealing: No leakage at 1 bar pressure
Mounting Security: No loosening after vibration test
```

---

*These specifications ensure consistent, professional-grade manufacturing of ESP32-S3-CAM wildlife camera components. All measurements and requirements are based on field testing and real-world deployment experience.*

**Document Version**: 1.0  
**Last Updated**: December 2024  
**Standards Compliance**: IEC 60529, ISO 2768-m