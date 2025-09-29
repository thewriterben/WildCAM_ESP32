# Enclosure Selection Example

This example demonstrates the automatic enclosure selection system for ESP32 Wildlife Camera boards.

## Overview

The enclosure selection system automatically:
- Detects your ESP32 board type
- Recommends appropriate enclosure based on deployment environment
- Generates 3D printing instructions
- Provides assembly guides and bill of materials
- Validates board-enclosure compatibility

## Features Demonstrated

### 1. Board Detection
- Automatic identification of ESP32 board type
- Hardware fingerprinting and capability detection
- Support for multiple board variants

### 2. Environment-Based Selection
- **Temperate Forest**: Standard outdoor deployment
- **Budget Conservation**: Cost-optimized designs
- **Desert**: UV-resistant, high-temperature designs
- **Urban**: Stealth and vibration-resistant designs

### 3. Cost Optimization
- Budget constraint consideration
- Material cost estimation
- Print time calculation
- Assembly complexity rating

### 4. Complete Documentation
- Detailed 3D printing instructions
- Step-by-step assembly guides
- Complete bill of materials
- Environment-specific modifications

## Running the Example

### Hardware Required
- Any supported ESP32 board (AI-Thinker ESP32-CAM recommended)
- USB programming cable
- Serial monitor capability

### Software Setup
1. Open in Arduino IDE or PlatformIO
2. Select appropriate board type
3. Upload and run
4. Open Serial Monitor at 115200 baud

### Expected Output
```
=================================================
ESP32 Wildlife Camera - Enclosure Selection Demo
=================================================

STEP 1: Board Detection
------------------------
Detected Board: AI-Thinker ESP32-CAM

STEP 2: Environment Selection
------------------------------
Scenario 1: Temperate Forest
Budget: $50.00
Recommended Enclosure: AI-Thinker Professional
Material Cost: $18.50
Print Time: 24 hours
IP Rating: IP65
Assembly Complexity: 4/5

STEP 3: 3D Printing Instructions (Scenario 1)
----------------------------------------------
=== 3D PRINTING INSTRUCTIONS ===

Recommended Material: PETG
Alternative: PLA+ with UV coating
Special Notes: Earth tone colors for camouflage

PRINT SETTINGS:
Layer Height: 0.2mm (quality)
Infill: 25-30%
Walls: 3 perimeters (1.2mm)
Support: Required for overhangs
Estimated Print Time: 24 hours
Material Cost: $18.50

STL FILES TO PRINT:
Directory: 3d_models/ai_thinker_esp32_cam/
- ai_thinker_main_enclosure.stl (1x)
- ai_thinker_front_cover.stl (1x, clear PETG)
- ai_thinker_back_cover.stl (1x)
- solar_panel_mount_bracket.stl (1x)
- tree_mounting_strap_clips.stl (4x)
- ventilation_grilles.stl (2x)

Common components (3d_models/common/):
- mounting_hardware.stl (1x, 100% infill)
- cable_management.stl (1x)
- weatherproofing_gaskets.stl (1x, TPU material)

ENVIRONMENT-SPECIFIC NOTES:
- Use earth tone filament colors
- Consider bark texture post-processing
- Standard ventilation adequate

[Additional assembly and BOM information follows...]
```

## Integration with Main Project

### Using the Enclosure Selector
```cpp
#include "enclosure_config.h"

// Detect board and get recommendation
BoardType board = BoardDetector::detectBoardType();
EnclosureType enclosure = EnclosureSelector::getRecommendedEnclosure(
    board, 
    ENV_TEMPERATE_FOREST, 
    50.0  // $50 budget
);

// Get specifications
EnclosureSpecs specs = EnclosureSelector::getEnclosureSpecs(enclosure);

// Generate instructions
String print_guide = EnclosureSelector::getPrintInstructions(enclosure, ENV_TEMPERATE_FOREST);
String assembly_guide = EnclosureSelector::getAssemblyInstructions(enclosure);
String bom = EnclosureSelector::getBillOfMaterials(enclosure);
```

### Web Interface Integration
The enclosure selector can be integrated into web-based setup interfaces:
```cpp
// Web server endpoint for enclosure selection
server.on("/enclosure-select", HTTP_GET, [](AsyncWebServerRequest *request){
    String environment = request->getParam("environment")->value();
    String budget = request->getParam("budget")->value();
    
    DeploymentEnvironment env = parseEnvironment(environment);
    float budget_limit = budget.toFloat();
    
    BoardType board = BoardDetector::detectBoardType();
    EnclosureType recommended = EnclosureSelector::getRecommendedEnclosure(
        board, env, budget_limit
    );
    
    String response = EnclosureSelector::getPrintInstructions(recommended, env);
    request->send(200, "text/plain", response);
});
```

## Customization

### Adding New Environments
1. Add new enum value to `DeploymentEnvironment`
2. Add material recommendations to `environment_materials[]`
3. Implement specific instructions in `formatEnvironmentSpecificInstructions()`

### Adding New Enclosures
1. Create STL files in appropriate directory
2. Add specifications to `enclosure_specs[]`
3. Update compatibility matrix
4. Add support in instruction generators

### Custom Cost Models
Modify cost calculations in `getEnclosureSpecs()` to reflect:
- Local material costs
- Printer operating costs
- Labor time estimates
- Hardware availability

## Troubleshooting

### Board Detection Issues
- Ensure proper board selection in IDE
- Check GPIO accessibility for fingerprinting
- Review board detection debug output

### Enclosure Compatibility
- Verify board dimensions against enclosure specs
- Check mounting hole compatibility
- Consider GPIO pin accessibility

### Print Quality
- Calibrate printer for dimensional accuracy
- Test fit with small parts first
- Adjust settings for material and environment

## Future Enhancements

### Planned Features
- **Visual enclosure previews**: 3D model rendering
- **Custom enclosure generator**: Parametric design tools
- **Community sharing**: Upload and share designs
- **Print farm integration**: Distributed printing network

### Contributing
1. Test with different board types
2. Share successful print settings
3. Document field deployment results
4. Submit improved designs and instructions

---

*This example demonstrates the complete enclosure selection workflow*
*Actual 3D printing requires physical printer and materials*
*Always test electronics before final enclosure assembly*