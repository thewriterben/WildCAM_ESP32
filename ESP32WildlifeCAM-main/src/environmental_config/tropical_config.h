/**
 * @file tropical_config.h
 * @brief Tropical Climate Configuration for ESP32 Wildlife Camera
 * 
 * Provides optimized settings for tropical environment deployment including
 * enhanced humidity control, mold prevention, and corrosion protection.
 */

#ifndef TROPICAL_CONFIG_H
#define TROPICAL_CONFIG_H

#include "../../firmware/src/enclosure_config.h"
#include "../../firmware/src/sensors/environmental_integration.h"

// Tropical environment constants
#define TROPICAL_TEMP_MIN 18.0          // °C - minimum tropical temperature
#define TROPICAL_TEMP_MAX 42.0          // °C - maximum tropical temperature
#define TROPICAL_HUMIDITY_MIN 80.0      // % - minimum tropical humidity
#define TROPICAL_HUMIDITY_MAX 100.0     // % - maximum tropical humidity
#define TROPICAL_PRECIPITATION_MAX 300  // mm/hr - extreme precipitation rate

// Tropical-specific thresholds
#define TROPICAL_MOLD_HUMIDITY_THRESHOLD 95.0  // % - activate mold prevention
#define TROPICAL_VENTILATION_THRESHOLD 90.0    // % - activate ventilation
#define TROPICAL_DRAINAGE_THRESHOLD 10.0       // mm/hr - activate drainage
#define TROPICAL_THERMAL_STRESS_TEMP 38.0      // °C - reduce power consumption

// Tropical hardware configuration
#define TROPICAL_VENTILATION_PIN 32          // GPIO pin for ventilation fan
#define TROPICAL_LENS_HEATER_PIN 33          // GPIO pin for lens heating
#define TROPICAL_HUMIDITY_SENSOR_PIN 34      // GPIO pin for internal humidity
#define TROPICAL_DRAINAGE_VALVE_PIN 35       // GPIO pin for drainage control

/**
 * @brief Tropical environment configuration structure
 */
struct TropicalConfig {
    // Environmental thresholds
    float temperature_min = TROPICAL_TEMP_MIN;
    float temperature_max = TROPICAL_TEMP_MAX;
    float humidity_threshold = TROPICAL_MOLD_HUMIDITY_THRESHOLD;
    float precipitation_threshold = TROPICAL_DRAINAGE_THRESHOLD;
    
    // Mold prevention settings
    bool mold_prevention_enabled = true;
    uint32_t mold_check_interval = 3600000;     // Check every hour
    bool uv_sterilization_enabled = false;      // Optional UV-C sterilization
    uint32_t ventilation_duration = 900000;     // 15 minutes ventilation cycle
    
    // Waterproofing and drainage
    bool enhanced_drainage = true;
    bool pressure_equalization = true;
    bool flood_detection = true;
    float flood_detection_threshold = 50.0;     // mm water level
    
    // Corrosion protection
    bool corrosion_monitoring = true;
    uint32_t corrosion_check_interval = 86400000; // Daily check
    bool sacrificial_anode_monitoring = true;
    
    // Power management adaptations
    bool thermal_power_reduction = true;
    float thermal_reduction_threshold = TROPICAL_THERMAL_STRESS_TEMP;
    bool humidity_power_optimization = true;
    
    // Camera adaptations
    bool lens_heating_enabled = true;
    float lens_heating_threshold = 5.0;         // °C temperature differential
    bool hydrophobic_coating_monitoring = true;
    uint32_t lens_cleaning_reminder = 604800000; // Weekly cleaning reminder
    
    // Communication adaptations
    bool atmospheric_interference_compensation = true;
    uint32_t transmission_retry_delay = 30000;   // 30 seconds between retries
    uint8_t max_transmission_retries = 5;
    
    TropicalConfig() = default;
};

/**
 * @brief Tropical climate manager class
 */
class TropicalClimateManager {
public:
    /**
     * @brief Initialize tropical climate management
     * @return True if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Update tropical climate management
     * Called regularly to monitor and respond to tropical conditions
     */
    void update();
    
    /**
     * @brief Get current tropical configuration
     * @return Current configuration structure
     */
    TropicalConfig getConfig() const { return config_; }
    
    /**
     * @brief Set tropical configuration
     * @param config New configuration to apply
     */
    void setConfig(const TropicalConfig& config);
    
    /**
     * @brief Check if mold prevention is needed
     * @return True if mold prevention should be activated
     */
    bool shouldActivateMoldPrevention() const;
    
    /**
     * @brief Activate ventilation system
     * @param duration_ms Duration to run ventilation (0 = use default)
     */
    void activateVentilation(uint32_t duration_ms = 0);
    
    /**
     * @brief Check drainage system
     * @return True if drainage is functioning properly
     */
    bool checkDrainageSystem();
    
    /**
     * @brief Activate lens heating
     * @param enable True to enable, false to disable
     */
    void activateLensHeating(bool enable);
    
    /**
     * @brief Get environmental health status
     * @return Health score (0-100, 100 = excellent)
     */
    uint8_t getEnvironmentalHealth() const;
    
    /**
     * @brief Handle emergency tropical conditions
     * @param emergency_type Type of emergency detected
     */
    void handleEmergency(const String& emergency_type);
    
private:
    TropicalConfig config_;
    bool initialized_ = false;
    uint32_t last_mold_check_ = 0;
    uint32_t last_corrosion_check_ = 0;
    uint32_t ventilation_start_time_ = 0;
    bool ventilation_active_ = false;
    bool lens_heating_active_ = false;
    
    // Internal monitoring functions
    float getCurrentHumidity() const;
    float getCurrentTemperature() const;
    float getPrecipitationRate() const;
    bool detectMoldRisk() const;
    bool detectCorrosionRisk() const;
    bool detectFloodConditions() const;
    
    // Control functions
    void controlVentilationFan(bool enable);
    void controlLensHeater(bool enable);
    void controlDrainageValve(bool open);
    void performMoldPrevention();
    void performCorrosionPrevention();
    
    // Monitoring and logging
    void logEnvironmentalData();
    void sendEnvironmentalAlert(const String& alert_type, const String& message);
};

/**
 * @brief Tropical species detection optimizations
 */
struct TropicalSpeciesConfig {
    // Common tropical species
    std::vector<String> target_species = {
        "jaguar", "ocelot", "margay", "tapir", "peccary",
        "howler_monkey", "spider_monkey", "capuchin_monkey",
        "toucan", "macaw", "quetzal", "hummingbird",
        "boa_constrictor", "fer_de_lance", "iguana"
    };
    
    // Activity patterns
    bool crepuscular_enhanced = true;    // Enhanced dawn/dusk detection
    bool nocturnal_mode = true;          // Night vision optimization
    float canopy_light_compensation = 0.8; // Adjust for low light under canopy
    
    // Environmental motion filtering
    bool rain_motion_filter = true;      // Filter rain-induced motion
    bool wind_motion_filter = true;      // Filter wind-induced vegetation motion
    float vegetation_motion_threshold = 0.3; // Motion sensitivity adjustment
    
    // Behavioral adaptations
    bool migration_tracking = true;      // Track seasonal movements
    bool feeding_pattern_optimization = true; // Optimize for feeding times
    uint32_t rainy_season_adaptation = 7200000; // 2-hour activity windows
};

/**
 * @brief Global tropical configuration instance
 */
extern TropicalClimateManager g_tropicalManager;

/**
 * @brief Initialize tropical climate configuration
 * @return True if successful
 */
bool initializeTropicalConfig();

/**
 * @brief Update tropical climate management
 * Should be called from main loop
 */
void updateTropicalManagement();

/**
 * @brief Get tropical deployment recommendations
 * @param location Geographic location for deployment
 * @return Deployment recommendations specific to tropical climate
 */
String getTropicalDeploymentRecommendations(const String& location);

/**
 * @brief Generate tropical maintenance schedule
 * @param deployment_duration Expected deployment duration in days
 * @return Customized maintenance schedule for tropical conditions
 */
String generateTropicalMaintenanceSchedule(uint32_t deployment_duration);

#endif // TROPICAL_CONFIG_H