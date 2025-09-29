/**
 * @file environmental_variant_manager.h
 * @brief Environmental Variant Manager for ESP32 Wildlife Camera
 * 
 * Centralized management system for all environmental variants including
 * tropical, desert, arctic, and temperate forest configurations.
 */

#ifndef ENVIRONMENTAL_VARIANT_MANAGER_H
#define ENVIRONMENTAL_VARIANT_MANAGER_H

#include "tropical_config.h"
#include "desert_config.h"
// Note: arctic_config.h and temperate_forest_config.h would be included when implemented
#include "../firmware/src/enclosure_config.h"

/**
 * @brief Environmental variant types
 */
enum EnvironmentalVariant {
    VARIANT_TROPICAL = 0,           // Enhanced waterproofing, mold resistance
    VARIANT_DESERT,                 // Heat resistance, UV protection, dust protection
    VARIANT_ARCTIC,                 // Cold resistance, heating, insulation
    VARIANT_TEMPERATE_FOREST,       // Camouflage, stealth, seasonal adaptation
    VARIANT_URBAN,                  // Stealth, vibration resistance, pollution protection
    VARIANT_COASTAL,                // Salt resistance, wind protection, corrosion resistance
    VARIANT_AUTO_DETECT             // Automatic variant selection based on conditions
};

/**
 * @brief Environmental conditions structure for variant selection
 */
struct EnvironmentalConditions {
    float temperature_min = 0.0;           // °C - minimum expected temperature
    float temperature_max = 30.0;          // °C - maximum expected temperature
    float humidity_avg = 60.0;             // % - average humidity
    float precipitation_annual = 500.0;    // mm - annual precipitation
    float uv_index_max = 8.0;             // Maximum UV index
    float wind_speed_max = 50.0;          // km/h - maximum wind speed
    bool saltwater_exposure = false;       // Saltwater/coastal environment
    bool urban_environment = false;        // Urban/city environment
    bool forest_canopy = false;           // Forest canopy cover
    String climate_description = "";       // Human-readable climate description
};

/**
 * @brief Variant capabilities structure
 */
struct VariantCapabilities {
    float temp_range_min = -10.0;         // °C - minimum operating temperature
    float temp_range_max = 50.0;          // °C - maximum operating temperature
    bool waterproof_enhanced = false;      // Enhanced waterproofing capability
    bool dust_protection = false;          // Dust ingress protection
    bool uv_resistance = false;           // UV radiation resistance
    bool cold_weather_heating = false;     // Cold weather heating system
    bool mold_resistance = false;          // Mold and fungus resistance
    bool camouflage_optimized = false;     // Camouflage and stealth features
    bool corrosion_resistance = false;     // Corrosion protection
    uint8_t maintenance_frequency = 30;    // Days between maintenance
    String deployment_complexity = "Medium"; // Deployment complexity level
};

/**
 * @brief Environmental variant manager class
 */
class EnvironmentalVariantManager {
public:
    /**
     * @brief Initialize environmental variant manager
     * @return True if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Set current environmental variant
     * @param variant Variant type to activate
     * @return True if variant successfully activated
     */
    bool setVariant(EnvironmentalVariant variant);
    
    /**
     * @brief Get current environmental variant
     * @return Currently active variant
     */
    EnvironmentalVariant getCurrentVariant() const { return current_variant_; }
    
    /**
     * @brief Auto-detect optimal variant based on conditions
     * @param conditions Environmental conditions at deployment site
     * @return Recommended variant type
     */
    EnvironmentalVariant autoDetectVariant(const EnvironmentalConditions& conditions);
    
    /**
     * @brief Get variant capabilities
     * @param variant Variant type to query
     * @return Capabilities structure for the variant
     */
    VariantCapabilities getVariantCapabilities(EnvironmentalVariant variant);
    
    /**
     * @brief Update environmental management
     * Called regularly to manage current variant
     */
    void update();
    
    /**
     * @brief Get deployment recommendations
     * @param variant Variant type
     * @param location Deployment location
     * @return Deployment recommendations
     */
    String getDeploymentRecommendations(EnvironmentalVariant variant, const String& location);
    
    /**
     * @brief Generate maintenance schedule
     * @param variant Variant type
     * @param duration_days Deployment duration in days
     * @return Maintenance schedule
     */
    String generateMaintenanceSchedule(EnvironmentalVariant variant, uint32_t duration_days);
    
    /**
     * @brief Get STL file list for variant
     * @param variant Variant type
     * @return List of required STL files
     */
    std::vector<String> getSTLFileList(EnvironmentalVariant variant);
    
    /**
     * @brief Get hardware requirements
     * @param variant Variant type
     * @return Hardware requirements list
     */
    String getHardwareRequirements(EnvironmentalVariant variant);
    
    /**
     * @brief Calculate environmental health score
     * @return Health score (0-100, 100 = excellent)
     */
    uint8_t getEnvironmentalHealth();
    
    /**
     * @brief Handle environmental emergency
     * @param emergency_type Type of emergency
     * @param severity Severity level (1-5)
     */
    void handleEnvironmentalEmergency(const String& emergency_type, uint8_t severity);
    
    /**
     * @brief Get performance metrics
     * @return Performance metrics for current variant
     */
    String getPerformanceMetrics();
    
private:
    EnvironmentalVariant current_variant_ = VARIANT_AUTO_DETECT;
    bool initialized_ = false;
    uint32_t last_update_ = 0;
    
    // Variant-specific managers
    TropicalClimateManager* tropical_manager_ = nullptr;
    DesertClimateManager* desert_manager_ = nullptr;
    // Additional managers would be added for other variants
    
    // Helper functions
    float calculateVariantScore(EnvironmentalVariant variant, const EnvironmentalConditions& conditions);
    bool activateVariantManager(EnvironmentalVariant variant);
    void deactivateCurrentManager();
    String getVariantName(EnvironmentalVariant variant);
    void logVariantChange(EnvironmentalVariant old_variant, EnvironmentalVariant new_variant);
};

/**
 * @brief Global environmental variant manager instance
 */
extern EnvironmentalVariantManager g_environmentalVariantManager;

/**
 * @brief Initialize environmental variant system
 * @return True if successful
 */
bool initializeEnvironmentalVariants();

/**
 * @brief Update environmental variant management
 * Should be called from main loop
 */
void updateEnvironmentalVariants();

/**
 * @brief Auto-configure based on geographic location
 * @param latitude Deployment latitude
 * @param longitude Deployment longitude
 * @return Recommended variant based on geographic location
 */
EnvironmentalVariant autoConfigureByLocation(float latitude, float longitude);

/**
 * @brief Get variant comparison matrix
 * @return Comparison of all variants and their capabilities
 */
String getVariantComparisonMatrix();

/**
 * @brief Calculate deployment cost estimate
 * @param variant Variant type
 * @param duration_days Deployment duration
 * @return Cost estimate in USD
 */
float calculateDeploymentCost(EnvironmentalVariant variant, uint32_t duration_days);

/**
 * @brief Validate variant compatibility with hardware
 * @param variant Variant type
 * @param board_type ESP32 board type
 * @return True if compatible
 */
bool validateVariantCompatibility(EnvironmentalVariant variant, BoardType board_type);

/**
 * @brief Get field testing recommendations
 * @param variant Variant type
 * @return Field testing procedures
 */
String getFieldTestingRecommendations(EnvironmentalVariant variant);

/**
 * @brief Export variant configuration
 * @param variant Variant type
 * @return JSON configuration string
 */
String exportVariantConfiguration(EnvironmentalVariant variant);

/**
 * @brief Import variant configuration
 * @param config_json JSON configuration string
 * @return True if configuration successfully imported
 */
bool importVariantConfiguration(const String& config_json);

/**
 * @brief Environmental conditions presets for common locations
 */
namespace EnvironmentalPresets {
    extern const EnvironmentalConditions AMAZON_RAINFOREST;
    extern const EnvironmentalConditions MOJAVE_DESERT;
    extern const EnvironmentalConditions ALASKAN_TUNDRA;
    extern const EnvironmentalConditions TEMPERATE_DECIDUOUS;
    extern const EnvironmentalConditions COASTAL_MEDITERRANEAN;
    extern const EnvironmentalConditions URBAN_TEMPERATE;
}

/**
 * @brief Utility functions for environmental calculations
 */
namespace EnvironmentalUtils {
    /**
     * @brief Calculate heat index from temperature and humidity
     * @param temperature Temperature in °C
     * @param humidity Relative humidity in %
     * @return Heat index in °C
     */
    float calculateHeatIndex(float temperature, float humidity);
    
    /**
     * @brief Calculate wind chill from temperature and wind speed
     * @param temperature Temperature in °C
     * @param wind_speed Wind speed in km/h
     * @return Wind chill in °C
     */
    float calculateWindChill(float temperature, float wind_speed);
    
    /**
     * @brief Estimate UV exposure based on conditions
     * @param uv_index UV index
     * @param cloud_cover Cloud cover percentage
     * @param altitude Altitude in meters
     * @return Estimated UV exposure level
     */
    float estimateUVExposure(float uv_index, float cloud_cover, float altitude);
    
    /**
     * @brief Calculate condensation risk
     * @param temperature Current temperature in °C
     * @param humidity Relative humidity in %
     * @return Condensation risk (0.0-1.0)
     */
    float calculateCondensationRisk(float temperature, float humidity);
    
    /**
     * @brief Estimate battery life based on environmental conditions
     * @param base_capacity_ah Base battery capacity in Ah
     * @param temperature Average temperature in °C
     * @param variant Environmental variant
     * @return Estimated battery life in hours
     */
    uint32_t estimateBatteryLife(float base_capacity_ah, float temperature, EnvironmentalVariant variant);
}

#endif // ENVIRONMENTAL_VARIANT_MANAGER_H