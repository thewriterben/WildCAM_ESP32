/**
 * @file enclosure_config.h
 * @brief Enclosure configuration and selection system
 * 
 * Integrates with board detection to provide appropriate enclosure recommendations
 * and 3D printing instructions based on detected hardware.
 */

#ifndef ENCLOSURE_CONFIG_H
#define ENCLOSURE_CONFIG_H

#include "../hal/camera_board.h"

/**
 * @brief Enclosure type definitions
 */
enum EnclosureType {
    ENCLOSURE_BUDGET_UNIVERSAL = 0,     // Universal budget enclosure
    ENCLOSURE_AI_THINKER_PRO,           // AI-Thinker professional
    ENCLOSURE_ESP32_S3_STANDARD,        // ESP32-S3-CAM standard (future)
    ENCLOSURE_ESP32_S3_AI,              // ESP32-S3-CAM AI optimized (future)
    ENCLOSURE_CUSTOM                    // User-defined custom enclosure
};

/**
 * @brief Deployment environment types
 */
enum DeploymentEnvironment {
    ENV_TEMPERATE_FOREST = 0,           // Moderate climate, tree mounting
    ENV_DESERT,                         // High UV, temperature extremes
    ENV_TROPICAL,                       // High humidity, heavy rain
    ENV_ARCTIC,                         // Sub-zero temperatures
    ENV_URBAN,                          // Stealth, vibration, pollution
    ENV_COASTAL                         // Salt spray, high winds
};

/**
 * @brief Enclosure specifications structure
 */
struct EnclosureSpecs {
    // Physical dimensions (internal space in mm)
    uint16_t internal_width;
    uint16_t internal_height;
    uint16_t internal_depth;
    
    // Environmental protection
    String ip_rating;                   // IP54, IP65, etc.
    float temp_min;                     // Minimum operating temperature (°C)
    float temp_max;                     // Maximum operating temperature (°C)
    bool uv_resistant;                  // UV protection rating
    
    // Features
    bool solar_mount_included;          // Solar panel mounting
    bool tree_mount_included;           // Tree mounting hardware
    bool ventilation_system;            // Condensation prevention
    bool tool_free_assembly;            // No tools required
    
    // Material requirements
    String recommended_material;        // PETG, ASA, PLA+, etc.
    String alternative_material;        // Alternative if primary unavailable
    bool requires_supports;             // 3D printing supports needed
    
    // Cost and complexity
    float estimated_material_cost;      // USD cost estimate
    uint16_t print_time_hours;          // Estimated print time
    uint8_t assembly_complexity;        // 1-5 scale (1=simple, 5=complex)
    
    // File references
    String stl_directory;               // Path to STL files
    String documentation_file;          // README.md path
};

/**
 * @brief Enclosure recommendation system
 */
class EnclosureSelector {
public:
    /**
     * @brief Get recommended enclosure for detected board
     * @param board_type Detected board type
     * @param environment Deployment environment
     * @param budget_constraint Maximum budget in USD
     * @return Recommended enclosure type
     */
    static EnclosureType getRecommendedEnclosure(
        BoardType board_type,
        DeploymentEnvironment environment,
        float budget_constraint = 50.0
    );
    
    /**
     * @brief Get enclosure specifications
     * @param enclosure_type Type of enclosure
     * @return Detailed specifications
     */
    static EnclosureSpecs getEnclosureSpecs(EnclosureType enclosure_type);
    
    /**
     * @brief Check if enclosure is compatible with board
     * @param enclosure_type Enclosure to check
     * @param board_type Board type to validate
     * @return True if compatible
     */
    static bool isCompatible(EnclosureType enclosure_type, BoardType board_type);
    
    /**
     * @brief Get 3D printing instructions
     * @param enclosure_type Enclosure type
     * @param environment Deployment environment
     * @return Formatted printing instructions
     */
    static String getPrintInstructions(
        EnclosureType enclosure_type,
        DeploymentEnvironment environment
    );
    
    /**
     * @brief Get assembly instructions
     * @param enclosure_type Enclosure type
     * @return Step-by-step assembly guide
     */
    static String getAssemblyInstructions(EnclosureType enclosure_type);
    
    /**
     * @brief Get bill of materials
     * @param enclosure_type Enclosure type
     * @return List of required hardware and materials
     */
    static String getBillOfMaterials(EnclosureType enclosure_type);

private:
    static const EnclosureSpecs enclosure_specs[];
    static String formatEnvironmentSpecificInstructions(
        DeploymentEnvironment environment,
        const EnclosureSpecs& specs
    );
};

// Predefined enclosure specifications
const EnclosureSpecs EnclosureSelector::enclosure_specs[] = {
    // ENCLOSURE_BUDGET_UNIVERSAL
    {
        .internal_width = 70,
        .internal_height = 55,
        .internal_depth = 35,
        .ip_rating = "IP54",
        .temp_min = -5.0,
        .temp_max = 40.0,
        .uv_resistant = false,  // Requires UV coating
        .solar_mount_included = true,
        .tree_mount_included = true,
        .ventilation_system = false,
        .tool_free_assembly = true,
        .recommended_material = "PLA+",
        .alternative_material = "PETG",
        .requires_supports = false,
        .estimated_material_cost = 6.25,
        .print_time_hours = 12,
        .assembly_complexity = 2,
        .stl_directory = "3d_models/budget_enclosures/",
        .documentation_file = "3d_models/budget_enclosures/README.md"
    },
    
    // ENCLOSURE_AI_THINKER_PRO
    {
        .internal_width = 85,
        .internal_height = 65,
        .internal_depth = 45,
        .ip_rating = "IP65",
        .temp_min = -20.0,
        .temp_max = 60.0,
        .uv_resistant = true,
        .solar_mount_included = true,
        .tree_mount_included = true,
        .ventilation_system = true,
        .tool_free_assembly = false,
        .recommended_material = "PETG",
        .alternative_material = "ASA",
        .requires_supports = true,
        .estimated_material_cost = 18.50,
        .print_time_hours = 24,
        .assembly_complexity = 4,
        .stl_directory = "3d_models/ai_thinker_esp32_cam/",
        .documentation_file = "3d_models/ai_thinker_esp32_cam/README.md"
    },
    
    // ENCLOSURE_ESP32_S3_STANDARD (Future)
    {
        .internal_width = 90,
        .internal_height = 70,
        .internal_depth = 50,
        .ip_rating = "IP65",
        .temp_min = -20.0,
        .temp_max = 65.0,
        .uv_resistant = true,
        .solar_mount_included = true,
        .tree_mount_included = true,
        .ventilation_system = true,
        .tool_free_assembly = false,
        .recommended_material = "ASA",
        .alternative_material = "PETG",
        .requires_supports = true,
        .estimated_material_cost = 22.00,
        .print_time_hours = 28,
        .assembly_complexity = 4,
        .stl_directory = "3d_models/esp32_s3_cam/",
        .documentation_file = "3d_models/esp32_s3_cam/README.md"
    }
};

/**
 * @brief Environment-specific material recommendations
 */
struct EnvironmentMaterial {
    DeploymentEnvironment environment;
    const char* primary_material;
    const char* alternative_material;
    const char* special_instructions;
};

const EnvironmentMaterial environment_materials[] = {
    {
        ENV_TEMPERATE_FOREST,
        "PETG",
        "PLA+ with UV coating",
        "Earth tone colors for camouflage"
    },
    {
        ENV_DESERT,
        "ASA",
        "PETG with UV coating",
        "Light colors, enhanced ventilation required"
    },
    {
        ENV_TROPICAL,
        "ASA",
        "PETG",
        "Anti-fungal treatment, extra drainage"
    },
    {
        ENV_ARCTIC,
        "ABS",
        "ASA",
        "Insulated version, heating elements"
    },
    {
        ENV_URBAN,
        "PETG",
        "ABS",
        "Gray/beige colors, vibration dampening"
    },
    {
        ENV_COASTAL,
        "ASA",
        "PETG",
        "Corrosion-resistant hardware required"
    }
};

/**
 * @brief Board-specific enclosure compatibility matrix
 */
struct BoardEnclosureCompatibility {
    BoardType board;
    EnclosureType primary_enclosure;
    EnclosureType alternative_enclosure;
    bool requires_modification;
    const char* modification_notes;
};

const BoardEnclosureCompatibility compatibility_matrix[] = {
    {
        BOARD_AI_THINKER_ESP32_CAM,
        ENCLOSURE_AI_THINKER_PRO,
        ENCLOSURE_BUDGET_UNIVERSAL,
        false,
        "Perfect fit, all features supported"
    },
    {
        BOARD_ESP32_S3_CAM,
        ENCLOSURE_ESP32_S3_STANDARD,
        ENCLOSURE_BUDGET_UNIVERSAL,
        true,
        "Budget enclosure requires mounting post modification"
    },
    {
        BOARD_ESP_EYE,
        ENCLOSURE_BUDGET_UNIVERSAL,
        ENCLOSURE_AI_THINKER_PRO,
        true,
        "Display requires front panel modification"
    }
};

#endif // ENCLOSURE_CONFIG_H