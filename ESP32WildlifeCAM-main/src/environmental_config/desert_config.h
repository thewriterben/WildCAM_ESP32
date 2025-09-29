/**
 * @file desert_config.h
 * @brief Desert Climate Configuration for ESP32 Wildlife Camera
 * 
 * Provides optimized settings for desert environment deployment including
 * thermal management, UV protection, dust ingress prevention, and power optimization.
 */

#ifndef DESERT_CONFIG_H
#define DESERT_CONFIG_H

#include "../../firmware/src/enclosure_config.h"
#include "../../firmware/src/sensors/environmental_integration.h"

// Desert environment constants
#define DESERT_TEMP_MIN -10.0           // °C - minimum desert temperature
#define DESERT_TEMP_MAX 60.0            // °C - maximum desert temperature
#define DESERT_HUMIDITY_MIN 5.0         // % - minimum desert humidity
#define DESERT_HUMIDITY_MAX 30.0        // % - maximum desert humidity
#define DESERT_UV_INDEX_MAX 15          // Extreme UV levels

// Desert-specific thresholds
#define DESERT_THERMAL_STRESS_TEMP 45.0     // °C - activate thermal protection
#define DESERT_CRITICAL_TEMP 55.0           // °C - emergency throttling
#define DESERT_SHUTDOWN_TEMP 60.0           // °C - emergency shutdown
#define DESERT_DUST_STORM_WIND_SPEED 50.0   // km/h - dust storm threshold
#define DESERT_SOLAR_DERATING_TEMP 40.0     // °C - start solar panel derating

// Desert hardware configuration
#define DESERT_THERMAL_SENSOR_PIN 34        // GPIO pin for thermal monitoring
#define DESERT_FAN_CONTROL_PIN 32           // GPIO pin for cooling fan
#define DESERT_DUST_SENSOR_PIN 35           // GPIO pin for dust detection
#define DESERT_UV_SENSOR_PIN 36             // GPIO pin for UV monitoring
#define DESERT_HEATER_PIN 33                // GPIO pin for cold weather heating

/**
 * @brief Desert environment configuration structure
 */
struct DesertConfig {
    // Temperature management
    float temperature_min = DESERT_TEMP_MIN;
    float temperature_max = DESERT_TEMP_MAX;
    float thermal_stress_threshold = DESERT_THERMAL_STRESS_TEMP;
    float critical_temperature = DESERT_CRITICAL_TEMP;
    float shutdown_temperature = DESERT_SHUTDOWN_TEMP;
    
    // Thermal protection settings
    bool thermal_protection_enabled = true;
    bool active_cooling_enabled = true;
    uint32_t thermal_check_interval = 30000;    // Check every 30 seconds
    uint32_t cooling_activation_delay = 10000;  // 10 second delay before cooling
    
    // Dust protection
    bool dust_protection_enabled = true;
    bool positive_pressure_enabled = true;
    uint32_t dust_check_interval = 300000;      // Check every 5 minutes
    float dust_storm_threshold = DESERT_DUST_STORM_WIND_SPEED;
    
    // UV protection
    bool uv_monitoring_enabled = true;
    float uv_index_threshold = 10.0;            // High UV warning threshold
    bool uv_protective_mode = true;             // Reduce exposure during peak UV
    
    // Power management adaptations
    bool thermal_power_scaling = true;
    float power_reduction_temp = 40.0;          // °C - start reducing power
    bool solar_derating_compensation = true;
    float solar_derating_temp = DESERT_SOLAR_DERATING_TEMP;
    
    // Camera adaptations
    bool thermal_imaging_compensation = true;
    bool heat_shimmer_reduction = true;
    bool contrast_enhancement = true;
    uint32_t lens_cleaning_interval = 86400000; // Daily lens check
    
    // Cold weather adaptations
    bool cold_weather_heating = true;
    float heating_activation_temp = 0.0;        // °C - activate heating
    bool battery_heating_enabled = true;
    
    // Communication adaptations
    bool atmospheric_compensation = true;
    uint32_t transmission_retry_interval = 60000; // 1 minute between retries
    uint8_t max_retries_dust_storm = 10;
    
    DesertConfig() = default;
};

/**
 * @brief Desert climate manager class
 */
class DesertClimateManager {
public:
    /**
     * @brief Initialize desert climate management
     * @return True if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Update desert climate management
     * Called regularly to monitor and respond to desert conditions
     */
    void update();
    
    /**
     * @brief Get current desert configuration
     * @return Current configuration structure
     */
    DesertConfig getConfig() const { return config_; }
    
    /**
     * @brief Set desert configuration
     * @param config New configuration to apply
     */
    void setConfig(const DesertConfig& config);
    
    /**
     * @brief Check if thermal protection is needed
     * @return True if thermal protection should be activated
     */
    bool shouldActivateThermalProtection() const;
    
    /**
     * @brief Activate cooling system
     * @param force_activation True to force activation regardless of thresholds
     */
    void activateCooling(bool force_activation = false);
    
    /**
     * @brief Check dust storm conditions
     * @return True if dust storm conditions detected
     */
    bool detectDustStorm() const;
    
    /**
     * @brief Activate dust protection measures
     */
    void activateDustProtection();
    
    /**
     * @brief Get thermal health status
     * @return Thermal health score (0-100, 100 = excellent)
     */
    uint8_t getThermalHealth() const;
    
    /**
     * @brief Handle emergency desert conditions
     * @param emergency_type Type of emergency detected
     */
    void handleEmergency(const String& emergency_type);
    
    /**
     * @brief Optimize power for desert conditions
     * @return True if power optimization applied
     */
    bool optimizeDesertPower();
    
    /**
     * @brief Check UV exposure levels
     * @return Current UV index reading
     */
    float getCurrentUVIndex() const;
    
private:
    DesertConfig config_;
    bool initialized_ = false;
    uint32_t last_thermal_check_ = 0;
    uint32_t last_dust_check_ = 0;
    uint32_t cooling_start_time_ = 0;
    bool cooling_active_ = false;
    bool heating_active_ = false;
    bool dust_protection_active_ = false;
    
    // Monitoring functions
    float getCurrentTemperature() const;
    float getWindSpeed() const;
    float getDustLevel() const;
    float getUVIntensity() const;
    bool detectThermalStress() const;
    bool detectExtremeHeat() const;
    bool detectFreezingConditions() const;
    
    // Control functions
    void controlCoolingFan(bool enable);
    void controlHeating(bool enable);
    void controlDustSealing(bool enable);
    void performThermalProtection();
    void performDustStormProtection();
    void performColdWeatherProtection();
    
    // Monitoring and logging
    void logDesertEnvironmentalData();
    void sendDesertAlert(const String& alert_type, const String& message);
    void updatePowerManagement();
};

/**
 * @brief Desert species detection optimizations
 */
struct DesertSpeciesConfig {
    // Common desert species
    std::vector<String> target_species = {
        "desert_bighorn_sheep", "mule_deer", "javelina", "mountain_lion",
        "bobcat", "coyote", "gray_fox", "ringtail", "jackrabbit",
        "cottontail", "kangaroo_rat", "desert_tortoise", "chuckwalla",
        "gila_monster", "roadrunner", "gambel_quail", "cactus_wren"
    };
    
    // Activity patterns
    bool crepuscular_optimization = true;       // Enhanced dawn/dusk detection
    bool midday_reduction = true;               // Reduced monitoring during peak heat
    float heat_avoidance_factor = 0.8;         // Reduced activity during hot periods
    
    // Environmental adaptations
    bool camouflage_enhancement = true;         // Enhanced detection for desert coloration
    bool water_source_monitoring = true;       // Increased sensitivity near water
    bool shade_seeking_behavior = true;        // Monitor shaded areas more intensively
    
    // Seasonal adaptations
    bool migration_tracking = true;             // Track seasonal movements
    bool breeding_season_optimization = true;  // Optimize for breeding behaviors
    uint32_t summer_dormancy_period = 14400000; // 4-hour midday rest period
};

/**
 * @brief Desert power optimization structure
 */
struct DesertPowerConfig {
    // Solar panel optimization
    bool dust_compensation = true;
    float dust_derating_factor = 0.7;          // 30% reduction during dust storms
    bool temperature_derating = true;
    float temp_coefficient = -0.004;           // %/°C power loss
    
    // Battery management
    bool thermal_protection = true;
    float battery_max_temp = 50.0;             // °C - maximum safe battery temperature
    float battery_min_temp = -10.0;            // °C - minimum safe battery temperature
    bool battery_heating = true;
    
    // Load management
    bool thermal_throttling = true;
    float throttling_start_temp = 45.0;        // °C - start reducing load
    float emergency_shutdown_temp = 60.0;      // °C - emergency shutdown
};

/**
 * @brief Global desert configuration instance
 */
extern DesertClimateManager g_desertManager;

/**
 * @brief Initialize desert climate configuration
 * @return True if successful
 */
bool initializeDesertConfig();

/**
 * @brief Update desert climate management
 * Should be called from main loop
 */
void updateDesertManagement();

/**
 * @brief Get desert deployment recommendations
 * @param location Geographic location for deployment
 * @return Deployment recommendations specific to desert climate
 */
String getDesertDeploymentRecommendations(const String& location);

/**
 * @brief Generate desert maintenance schedule
 * @param deployment_duration Expected deployment duration in days
 * @return Customized maintenance schedule for desert conditions
 */
String generateDesertMaintenanceSchedule(uint32_t deployment_duration);

/**
 * @brief Calculate thermal derating for components
 * @param ambient_temp Current ambient temperature
 * @return Derating factor (0.0-1.0)
 */
float calculateThermalDerating(float ambient_temp);

/**
 * @brief Estimate dust accumulation rate
 * @param wind_speed Current wind speed
 * @param humidity Current humidity level
 * @return Dust accumulation rate (arbitrary units)
 */
float estimateDustAccumulation(float wind_speed, float humidity);

#endif // DESERT_CONFIG_H