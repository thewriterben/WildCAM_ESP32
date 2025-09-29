# STL Files Directory

**📌 Navigation**: [🏠 Main README](../README.md) | [📚 Documentation Index](../docs/README.md) | [🚀 Quick Start](../QUICK_START.md) | [🤝 Contributing](../CONTRIBUTING.md)

This directory contains template STL files for 3D printing wildlife camera enclosures and components. These files serve as foundational templates that can be customized for specific deployment needs.

## Directory Structure

### AI-Thinker ESP32-CAM Components
- `ai_thinker_main_enclosure.stl` - Main weatherproof housing for AI-Thinker ESP32-CAM
- `ai_thinker_front_cover.stl` - Front panel with camera cutout and lens protection
- `ai_thinker_back_cover.stl` - Back panel with cable management and access ports

### ESP32-S3-CAM Components  
- `esp32_s3_main_enclosure.stl` - Main housing optimized for ESP32-S3-CAM dimensions
- `esp32_s3_front_cover.stl` - Front panel with enhanced camera sensor support
- `esp32_s3_back_cover.stl` - Back panel with USB-C access and improved ventilation

### Universal Components
- `solar_panel_mount_bracket.stl` - Adjustable solar panel mounting system
- `tree_mounting_strap_clips.stl` - Secure tree mounting hardware
- `ventilation_grilles.stl` - Weatherproof ventilation components
- `camera_lens_hood.stl` - Lens protection and glare reduction

### Mounting Hardware
- `battery_compartment.stl` - Secure battery housing with weather sealing
- `cable_management_clips.stl` - Organization and protection for cables
- `mounting_base_plate.stl` - Universal mounting interface

## File Format and Specifications

All STL files are provided in ASCII format for maximum compatibility and are designed with the following specifications:

### Print Settings
- **Layer Height**: 0.2mm (recommended)
- **Infill**: 20-30% for structural components, 15% for covers
- **Print Speed**: 50mm/s for optimal quality
- **Support Material**: Required for overhangs >45°

### Material Recommendations
- **Primary Material**: PETG or ABS for weather resistance
- **Alternative**: PLA+ for indoor or protected installations
- **UV Protection**: UV-resistant filament recommended for outdoor use

### Post-Processing
- **Weatherproofing**: Apply silicone coating to all joint surfaces
- **Hardware**: Use stainless steel screws and waterproof gaskets
- **Testing**: Verify fit before final assembly

## Customization Guidelines

These template files can be modified to accommodate:

1. **Different Camera Modules**: Adjust mounting holes and openings
2. **Additional Sensors**: Add cutouts for PIR sensors, temperature probes, etc.
3. **Alternative Power Systems**: Modify for different battery configurations
4. **Environmental Adaptations**: Scale components for extreme weather conditions

## File Status

**Template Status**: These are foundational template files that provide the basic structure and mounting points. The complete detailed STL files with full geometry are available in the `3d_models/` directory.

For production-ready STL files with complete geometry and tested dimensions, please refer to:
- `../3d_models/ai_thinker_esp32_cam/` - Complete AI-Thinker components
- `../3d_models/esp32_s3_cam/` - Complete ESP32-S3-CAM components  
- `../3d_models/budget_enclosures/` - Simplified single-piece designs
- `../3d_models/common/` - Universal mounting hardware

## Integration with Existing Files

This template directory complements the existing comprehensive 3D model library by providing:
1. **Simplified Templates**: Easy-to-modify baseline designs
2. **Rapid Prototyping**: Quick customization for specific needs  
3. **Educational Resources**: Clear examples for community contributions
4. **Version Control**: Template tracking for design evolution

## Community Contributions

We welcome community contributions of:
- Modified templates for specific use cases
- Regional adaptations for different climates
- Improved designs based on field testing
- Documentation of successful modifications

## License

These STL template files are released under the same open-source license as the main project. See the main LICENSE file for details.

## Support

For questions about STL file usage, customization, or printing recommendations, please:
1. Check the main documentation in `docs/`
2. Review existing issues in the project repository
3. Create a new issue with the "3D-printing" label

---

*Note: This directory provides foundational templates. For immediate use, the complete STL files in the `3d_models/` directory are recommended.*