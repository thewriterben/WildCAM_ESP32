# 3D Models - ESP32 Wildlife Camera

This directory contains 3D printable enclosures and mounting hardware for the ESP32 Wildlife Camera system, supporting multiple board types and deployment scenarios.

## Overview

The 3D models are organized by board type and use case, providing complete enclosure solutions from budget-friendly conservation projects to professional wildlife monitoring installations.

## File Structure

```
3d_models/
├── README.md                           # This file
├── ai_thinker_esp32_cam/              # AI-Thinker ESP32-CAM specific enclosures
│   ├── README.md                      # Detailed AI-Thinker documentation
│   ├── ai_thinker_main_enclosure.stl  # Primary weatherproof housing
│   ├── ai_thinker_front_cover.stl     # Transparent camera cover
│   ├── ai_thinker_back_cover.stl      # Access panel with hinges
│   ├── solar_panel_mount_bracket.stl  # Adjustable solar mount
│   ├── tree_mounting_strap_clips.stl  # Tree mounting system
│   └── ventilation_grilles.stl        # Condensation prevention
├── budget_enclosures/                 # Cost-effective universal designs
│   ├── README.md                      # Budget project guide
│   ├── basic_wildlife_cam_case.stl    # Universal ESP32 housing
│   ├── solar_panel_mount.stl          # Simple fixed-angle mount
│   ├── camera_lens_hood.stl           # Lens protection
│   └── battery_compartment.stl        # 18650 battery housing
├── esp32_s3_cam/                      # ESP32-S3-CAM future support
│   └── README.md                      # Planned development guide
└── common/                            # Shared components
    ├── mounting_hardware.stl          # Universal mounting brackets
    ├── cable_management.stl           # Cable routing solutions
    └── weatherproofing_gaskets.stl    # Rubber gasket profiles
```

## Board Compatibility Matrix

| Board Type | Enclosure Directory | Status | Notes |
|------------|-------------------|---------|-------|
| AI-Thinker ESP32-CAM | `ai_thinker_esp32_cam/` | ✅ Complete | Primary support, full feature set |
| ESP32-S3-CAM | `esp32_s3_cam/` | 🔄 Planned | Future development |
| Generic ESP32 | `budget_enclosures/` | ✅ Available | Universal compatibility |
| ESP-EYE | `budget_enclosures/` | ⚠️ Compatible | Use universal designs |
| M5Stack Timer Camera | `budget_enclosures/` | ⚠️ Compatible | May require modifications |

## Design Categories

### Professional Deployment (`ai_thinker_esp32_cam/`)
- **Target**: Research installations, long-term monitoring
- **Features**: IP65 weatherproofing, modular design, professional mounting
- **Materials**: PETG/ASA for durability
- **Cost**: $15-25 per unit

### Budget Conservation (`budget_enclosures/`)
- **Target**: Educational projects, community conservation
- **Features**: Simple assembly, minimal material usage
- **Materials**: PLA with UV coating acceptable
- **Cost**: Under $10 per unit

### Future Platforms (`esp32_s3_cam/`)
- **Target**: Next-generation ESP32-S3 boards
- **Features**: Enhanced processing, AI integration
- **Status**: Development planned for 2025

## Printing Guidelines

### Material Selection by Use Case

**Long-term Outdoor (1+ years):**
- **ASA**: Best UV resistance, requires enclosed printer
- **PETG**: Good balance of printability and durability
- **ABS**: Strong and temperature resistant

**Short-term/Budget (6-12 months):**
- **PLA+**: Enhanced PLA with better outdoor performance
- **PLA with UV coating**: Most economical option

**Prototyping/Testing:**
- **PLA**: Fastest and easiest for initial testing

### Optimized Print Settings

```
Professional Enclosures:
- Layer Height: 0.2mm
- Infill: 25-30%
- Walls: 3 perimeters (1.2mm)
- Top/Bottom: 5 layers
- Support: Required for overhangs >45°

Budget Enclosures:
- Layer Height: 0.3mm (faster)
- Infill: 15-20%
- Walls: 2 perimeters (0.8mm)
- Top/Bottom: 3 layers
- Support: Minimized by design
```

## Assembly Instructions

### Enclosure Assembly
1. Print main body and lid components
2. Install camera module in camera mount
3. Route cables through designated openings
4. Secure electronics with mounting screws
5. Apply weatherproof seal to lid

### Mounting System
1. Select appropriate mounting bracket for deployment location
2. Print mounting hardware with 100% infill for strength
3. Use stainless steel bolts for outdoor installation
4. Apply thread locker to prevent loosening

## Weatherproofing

- Use silicone sealant around cable entry points
- Install rubber gasket in lid groove
- Ensure proper drainage for condensation
- Consider desiccant packets for humid environments

## Customization

The 3D models are designed to be easily customizable:
- Modify mounting points for different hardware
- Adjust enclosure size for additional components
- Add ventilation features if needed
- Customize mounting brackets for specific installations

## Print Testing

Before final deployment, test print with:
1. Dimensional accuracy verification
2. Fit test with actual hardware
3. Weather seal effectiveness
4. Mounting system strength

## Post-Processing

- Sand contact surfaces for smooth fit
- Drill out mounting holes to final size
- Apply UV-resistant coating if using PLA
- Test assembly before field deployment

---

*Detailed 3D printing guides, assembly instructions, and model files are available in the respective subdirectories: `ai_thinker_esp32_cam/` and `budget_enclosures/`.*