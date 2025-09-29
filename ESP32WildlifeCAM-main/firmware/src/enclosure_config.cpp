/**
 * @file enclosure_config.cpp
 * @brief Implementation of enclosure configuration and selection system
 */

#include "enclosure_config.h"
#include "config.h"

EnclosureType EnclosureSelector::getRecommendedEnclosure(
    BoardType board_type,
    DeploymentEnvironment environment,
    float budget_constraint
) {
    // Find board compatibility
    for (const auto& compat : compatibility_matrix) {
        if (compat.board == board_type) {
            // Check budget constraint
            EnclosureSpecs primary_specs = getEnclosureSpecs(compat.primary_enclosure);
            if (primary_specs.estimated_material_cost <= budget_constraint) {
                return compat.primary_enclosure;
            }
            
            // Try alternative if primary is too expensive
            EnclosureSpecs alt_specs = getEnclosureSpecs(compat.alternative_enclosure);
            if (alt_specs.estimated_material_cost <= budget_constraint) {
                return compat.alternative_enclosure;
            }
        }
    }
    
    // Default to budget enclosure if no specific match
    return ENCLOSURE_BUDGET_UNIVERSAL;
}

EnclosureSpecs EnclosureSelector::getEnclosureSpecs(EnclosureType enclosure_type) {
    if (enclosure_type < sizeof(enclosure_specs) / sizeof(enclosure_specs[0])) {
        return enclosure_specs[enclosure_type];
    }
    
    // Return budget specs as fallback
    return enclosure_specs[ENCLOSURE_BUDGET_UNIVERSAL];
}

bool EnclosureSelector::isCompatible(EnclosureType enclosure_type, BoardType board_type) {
    for (const auto& compat : compatibility_matrix) {
        if (compat.board == board_type) {
            return (compat.primary_enclosure == enclosure_type || 
                    compat.alternative_enclosure == enclosure_type);
        }
    }
    
    // Budget enclosure is universally compatible
    return (enclosure_type == ENCLOSURE_BUDGET_UNIVERSAL);
}

String EnclosureSelector::getPrintInstructions(
    EnclosureType enclosure_type,
    DeploymentEnvironment environment
) {
    EnclosureSpecs specs = getEnclosureSpecs(enclosure_type);
    
    String instructions = "=== 3D PRINTING INSTRUCTIONS ===\n\n";
    
    // Material selection based on environment
    String material = specs.recommended_material;
    for (const auto& env_mat : environment_materials) {
        if (env_mat.environment == environment) {
            material = env_mat.primary_material;
            instructions += "Recommended Material: " + String(env_mat.primary_material) + "\n";
            instructions += "Alternative: " + String(env_mat.alternative_material) + "\n";
            instructions += "Special Notes: " + String(env_mat.special_instructions) + "\n\n";
            break;
        }
    }
    
    // Print settings
    instructions += "PRINT SETTINGS:\n";
    if (enclosure_type == ENCLOSURE_BUDGET_UNIVERSAL) {
        instructions += "Layer Height: 0.3mm (fast printing)\n";
        instructions += "Infill: 15-20%\n";
        instructions += "Walls: 2 perimeters (0.8mm)\n";
        instructions += "Support: " + String(specs.requires_supports ? "Required" : "Not needed") + "\n";
    } else {
        instructions += "Layer Height: 0.2mm (quality)\n";
        instructions += "Infill: 25-30%\n";
        instructions += "Walls: 3 perimeters (1.2mm)\n";
        instructions += "Support: " + String(specs.requires_supports ? "Required for overhangs" : "Not needed") + "\n";
    }
    
    instructions += "Estimated Print Time: " + String(specs.print_time_hours) + " hours\n";
    instructions += "Material Cost: $" + String(specs.estimated_material_cost, 2) + "\n\n";
    
    // Files to print
    instructions += "STL FILES TO PRINT:\n";
    instructions += "Directory: " + specs.stl_directory + "\n";
    
    if (enclosure_type == ENCLOSURE_AI_THINKER_PRO) {
        instructions += "- ai_thinker_main_enclosure.stl (1x)\n";
        instructions += "- ai_thinker_front_cover.stl (1x, clear PETG)\n";
        instructions += "- ai_thinker_back_cover.stl (1x)\n";
        instructions += "- solar_panel_mount_bracket.stl (1x)\n";
        instructions += "- tree_mounting_strap_clips.stl (4x)\n";
        instructions += "- ventilation_grilles.stl (2x)\n";
    } else if (enclosure_type == ENCLOSURE_BUDGET_UNIVERSAL) {
        instructions += "- basic_wildlife_cam_case.stl (1x)\n";
        instructions += "- solar_panel_mount.stl (1x)\n";
        instructions += "- camera_lens_hood.stl (1x)\n";
        instructions += "- battery_compartment.stl (1x)\n";
    }
    
    instructions += "\nCommon components (3d_models/common/):\n";
    instructions += "- mounting_hardware.stl (1x, 100% infill)\n";
    instructions += "- cable_management.stl (1x)\n";
    instructions += "- weatherproofing_gaskets.stl (1x, TPU material)\n\n";
    
    // Environment-specific notes
    instructions += formatEnvironmentSpecificInstructions(environment, specs);
    
    return instructions;
}

String EnclosureSelector::getAssemblyInstructions(EnclosureType enclosure_type) {
    EnclosureSpecs specs = getEnclosureSpecs(enclosure_type);
    
    String instructions = "=== ASSEMBLY INSTRUCTIONS ===\n\n";
    
    instructions += "Complexity Level: " + String(specs.assembly_complexity) + "/5\n";
    instructions += "Tool-free Assembly: " + String(specs.tool_free_assembly ? "Yes" : "No") + "\n\n";
    
    if (enclosure_type == ENCLOSURE_BUDGET_UNIVERSAL) {
        instructions += "BUDGET ASSEMBLY STEPS:\n";
        instructions += "1. Test fit all printed parts\n";
        instructions += "2. Install ESP32 board in main case\n";
        instructions += "3. Route cables through side channels\n";
        instructions += "4. Snap-fit lid with gasket\n";
        instructions += "5. Attach solar mount with provided clips\n";
        instructions += "6. Install lens hood with snap-fit\n";
        instructions += "7. Connect battery compartment\n";
        instructions += "8. Test all connections before sealing\n";
    } else if (enclosure_type == ENCLOSURE_AI_THINKER_PRO) {
        instructions += "PROFESSIONAL ASSEMBLY STEPS:\n";
        instructions += "1. Install threaded inserts (M3 x 5mm)\n";
        instructions += "2. Mount ESP32-CAM on internal posts\n";
        instructions += "3. Install ventilation grilles with membrane\n";
        instructions += "4. Route cables through cable glands\n";
        instructions += "5. Apply silicone sealant to joints\n";
        instructions += "6. Install rubber gasket in lid groove\n";
        instructions += "7. Attach front cover with clear lens\n";
        instructions += "8. Mount solar bracket with adjustment mechanism\n";
        instructions += "9. Install tree mounting clips\n";
        instructions += "10. Perform weather seal test\n";
    }
    
    instructions += "\nGENERAL NOTES:\n";
    instructions += "- Test electronics before final assembly\n";
    instructions += "- Use stainless steel hardware for outdoor use\n";
    instructions += "- Apply thread locker to prevent loosening\n";
    instructions += "- Document camera position for future adjustments\n";
    
    return instructions;
}

String EnclosureSelector::getBillOfMaterials(EnclosureType enclosure_type) {
    String bom = "=== BILL OF MATERIALS ===\n\n";
    
    bom += "3D PRINTED PARTS:\n";
    bom += "Filament cost: $" + String(getEnclosureSpecs(enclosure_type).estimated_material_cost, 2) + "\n\n";
    
    if (enclosure_type == ENCLOSURE_BUDGET_UNIVERSAL) {
        bom += "HARDWARE (Budget Version):\n";
        bom += "- M3 x 12mm screws (4x) - $0.50\n";
        bom += "- M3 nuts (4x) - $0.25\n";
        bom += "- Rubber gasket strip (200mm) - $1.00\n";
        bom += "- Cable ties (10x) - $0.50\n";
        bom += "- Silicone sealant tube - $2.00\n";
        bom += "Total hardware: $4.25\n";
    } else if (enclosure_type == ENCLOSURE_AI_THINKER_PRO) {
        bom += "HARDWARE (Professional Version):\n";
        bom += "- M3 x 5mm threaded inserts (8x) - $2.00\n";
        bom += "- M3 x 12mm stainless screws (8x) - $1.50\n";
        bom += "- M3 x 8mm screws (4x) - $0.75\n";
        bom += "- PG7 cable glands (2x) - $3.00\n";
        bom += "- Rubber gasket O-ring - $1.50\n";
        bom += "- Breathable membrane (50x50mm) - $2.00\n";
        bom += "- Thread locker (blue) - $3.00\n";
        bom += "- Silicone sealant (premium) - $4.00\n";
        bom += "- Desiccant packs (2x) - $1.00\n";
        bom += "Total hardware: $18.75\n";
    }
    
    bom += "\nOPTIONAL ACCESSORIES:\n";
    bom += "- Tree mounting straps (25mm x 2m) - $5.00\n";
    bom += "- Solar panel (5V 2W) - $12.00\n";
    bom += "- 18650 battery holder - $3.00\n";
    bom += "- External antenna (SMA) - $8.00\n";
    bom += "- PIR motion sensor - $4.00\n";
    
    bom += "\nTOOLS REQUIRED:\n";
    if (enclosure_type == ENCLOSURE_BUDGET_UNIVERSAL) {
        bom += "- Phillips screwdriver\n";
        bom += "- Drill with 3mm bit (or heated nail)\n";
        bom += "- File or sandpaper\n";
    } else {
        bom += "- Soldering iron (for threaded inserts)\n";
        bom += "- Phillips screwdriver set\n";
        bom += "- Drill with multiple bits\n";
        bom += "- File and sandpaper set\n";
        bom += "- Thread tapping set (M3)\n";
    }
    
    return bom;
}

String EnclosureSelector::formatEnvironmentSpecificInstructions(
    DeploymentEnvironment environment,
    const EnclosureSpecs& specs
) {
    String instructions = "ENVIRONMENT-SPECIFIC NOTES:\n";
    
    switch (environment) {
        case ENV_TEMPERATE_FOREST:
            instructions += "- Use earth tone filament colors\n";
            instructions += "- Consider bark texture post-processing\n";
            instructions += "- Standard ventilation adequate\n";
            break;
            
        case ENV_DESERT:
            instructions += "- Use light colored filament (white/tan)\n";
            instructions += "- Print extra ventilation grilles\n";
            instructions += "- Apply UV coating even to ASA prints\n";
            instructions += "- Consider reflective tape on top surfaces\n";
            break;
            
        case ENV_TROPICAL:
            instructions += "- Print drainage channels in bottom\n";
            instructions += "- Use anti-fungal filament treatment\n";
            instructions += "- Double gasket sealing recommended\n";
            instructions += "- Include extra desiccant compartments\n";
            break;
            
        case ENV_ARCTIC:
            instructions += "- Print thicker walls (4 perimeters)\n";
            instructions += "- Consider insulation layer\n";
            instructions += "- Use ABS or ASA material only\n";
            instructions += "- Design heating element mounting\n";
            break;
            
        case ENV_URBAN:
            instructions += "- Use gray/beige camouflage colors\n";
            instructions += "- Add vibration dampening pads\n";
            instructions += "- Consider theft-deterrent design\n";
            instructions += "- Print utility box appearance\n";
            break;
            
        case ENV_COASTAL:
            instructions += "- Use only stainless steel hardware\n";
            instructions += "- Apply corrosion-resistant coating\n";
            instructions += "- Seal all metal-plastic interfaces\n";
            instructions += "- Design for high wind resistance\n";
            break;
    }
    
    instructions += "\n";
    return instructions;
}