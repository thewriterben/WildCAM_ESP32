/**
 * Environmental Adapter - Field Testing and Environmental Optimization
 * 
 * Adapts camera behavior and network operations to real-world environmental conditions.
 * Implements weather resistance, seasonal optimization, and terrain adaptation.
 * 
 * Features:
 * - Weather-aware operation modes
 * - Seasonal wildlife pattern adaptation
 * - Terrain and RF optimization
 * - Environmental sensor integration
 * - Predictive adaptation based on weather forecasts
 */

#ifndef ENVIRONMENTAL_ADAPTER_H
#define ENVIRONMENTAL_ADAPTER_H

#include <Arduino.h>
#include <vector>
#include <map>

// Environmental Conditions
enum WeatherCondition {
    WEATHER_CLEAR,
    WEATHER_CLOUDY,
    WEATHER_RAIN,
    WEATHER_SNOW,
    WEATHER_FOG,
    WEATHER_STORM,
    WEATHER_EXTREME_HEAT,
    WEATHER_EXTREME_COLD
};

enum SeasonType {
    SEASON_SPRING,
    SEASON_SUMMER,
    SEASON_AUTUMN,
    SEASON_WINTER
};

enum TerrainType {
    TERRAIN_FLAT,
    TERRAIN_HILLY,
    TERRAIN_MOUNTAINOUS,
    TERRAIN_FOREST,
    TERRAIN_DESERT,
    TERRAIN_WETLAND,
    TERRAIN_URBAN
};

// Environmental Measurement Structure
struct EnvironmentalData {
    float temperature;          // Celsius
    float humidity;            // Percentage
    float pressure;            // hPa
    float lightLevel;          // Lux
    float windSpeed;           // m/s
    float windDirection;       // Degrees
    float precipitation;       // mm/h
    float uvIndex;             // UV index
    float soilMoisture;        // Percentage
    float batteryTemperature;  // Celsius for thermal management
    uint32_t timestamp;
    
    EnvironmentalData() : 
        temperature(20.0), humidity(50.0), pressure(1013.25), lightLevel(1000.0),
        windSpeed(0.0), windDirection(0.0), precipitation(0.0), uvIndex(0.0),
        soilMoisture(50.0), batteryTemperature(20.0), timestamp(0) {}
};

// Adaptation Configuration
struct AdaptationConfig {
    // Temperature thresholds
    float maxOperatingTemp;     // Maximum safe operating temperature
    float minOperatingTemp;     // Minimum safe operating temperature
    float thermalThrottle;      // Temperature to start throttling
    
    // Weather adaptations
    bool weatherAwareMode;      // Enable weather-based adaptations
    bool stormProtection;       // Enhanced protection during storms
    float rainSensitivity;      // Sensitivity to rain events
    float windResistance;       // Wind speed threshold for adaptations
    
    // Seasonal settings
    bool seasonalOptimization;  // Enable seasonal adaptations
    SeasonType currentSeason;   // Current season setting
    bool migrationTracking;     // Track seasonal animal migration
    
    // Power adaptations
    bool thermalPowerMgmt;      // Temperature-based power management
    bool weatherPowerSaving;    // Power saving during bad weather
    float solarEfficiencyFactor; // Solar panel efficiency adjustment
    
    AdaptationConfig() : 
        maxOperatingTemp(60.0), minOperatingTemp(-20.0), thermalThrottle(45.0),
        weatherAwareMode(true), stormProtection(true), rainSensitivity(0.5),
        windResistance(10.0), seasonalOptimization(true), currentSeason(SEASON_SPRING),
        migrationTracking(true), thermalPowerMgmt(true), weatherPowerSaving(true),
        solarEfficiencyFactor(1.0) {}
};

// Adaptation Response
struct AdaptationResponse {
    bool powerReduction;        // Reduce power consumption
    bool frequencyReduction;    // Reduce operation frequency
    bool qualityReduction;      // Reduce image/processing quality
    bool networkOptimization;   // Optimize network behavior
    bool sensorRecalibration;   // Recalibrate sensors
    bool thermalShutdown;       // Emergency thermal shutdown
    float adaptationStrength;   // Strength of adaptation (0-1)
    String reasoning;           // Explanation of adaptation
    
    AdaptationResponse() : 
        powerReduction(false), frequencyReduction(false), qualityReduction(false),
        networkOptimization(false), sensorRecalibration(false), thermalShutdown(false),
        adaptationStrength(0.0), reasoning("") {}
};

// Seasonal Pattern Data
struct SeasonalPattern {
    SeasonType season;
    std::vector<String> activeSpecies;
    std::vector<int> activityHours;
    float expectedActivityLevel;
    String behaviorChanges;
    bool migrationPeriod;
    
    SeasonalPattern() : 
        season(SEASON_SPRING), expectedActivityLevel(1.0), behaviorChanges(""),
        migrationPeriod(false) {}
};

// Weather Forecast Data
struct WeatherForecast {
    WeatherCondition condition;
    float temperature;
    float humidity;
    float precipitation;
    float windSpeed;
    uint32_t timestamp;
    uint32_t duration;          // Forecast duration in seconds
    float confidence;           // Forecast confidence (0-1)
    
    WeatherForecast() : 
        condition(WEATHER_CLEAR), temperature(20.0), humidity(50.0),
        precipitation(0.0), windSpeed(0.0), timestamp(0), duration(3600),
        confidence(0.8) {}
};

/**
 * Environmental Adapter Class
 * 
 * Provides comprehensive environmental adaptation for wildlife cameras
 */
class EnvironmentalAdapter {
public:
    EnvironmentalAdapter();
    ~EnvironmentalAdapter();
    
    // Initialization and configuration
    bool init(const AdaptationConfig& config);
    void cleanup();
    bool configure(const AdaptationConfig& config);
    AdaptationConfig getConfiguration() const { return config_; }
    
    // Environmental monitoring
    bool updateEnvironmentalData(const EnvironmentalData& data);
    EnvironmentalData getCurrentData() const { return currentData_; }
    bool readSensors();
    bool calibrateSensors();
    
    // Weather adaptation
    AdaptationResponse adaptToWeather(WeatherCondition condition);
    bool setWeatherCondition(WeatherCondition condition);
    WeatherCondition detectWeatherCondition() const;
    bool isExtremeWeather() const;
    
    // Seasonal adaptation
    bool updateSeasonalSettings(SeasonType season);
    SeasonalPattern getSeasonalPattern(SeasonType season) const;
    bool optimizeForSeason();
    bool trackMigrationPatterns();
    
    // Temperature management
    bool performThermalManagement();
    bool checkTemperatureThresholds() const;
    float getThermalDerating() const;
    bool requiresCooling() const;
    
    // Terrain optimization
    bool adaptToTerrain(TerrainType terrain);
    bool optimizeRFPropagation();
    bool adjustForTopography();
    
    // Power optimization based on environment
    bool optimizePowerForEnvironment();
    float calculateSolarEfficiency() const;
    bool adjustForBatteryTemperature();
    
    // Predictive adaptations
    bool integrateWeatherForecast(const WeatherForecast& forecast);
    bool predictEnvironmentalChanges();
    bool preemptiveAdaptation();
    
    // Camera and imaging adaptations
    bool adaptCameraSettings();
    bool adjustForLightConditions();
    bool compensateForWeather();
    bool optimizeImageQuality();
    
    // Network adaptations
    bool adaptNetworkBehavior();
    bool optimizeForConnectivity();
    bool adjustTransmissionPower();
    
    // Data collection and analysis
    std::vector<EnvironmentalData> getHistoricalData(uint32_t hours = 24) const;
    String generateEnvironmentalReport() const;
    bool exportEnvironmentalData(const String& filename) const;
    
    // Utility functions
    String getWeatherDescription(WeatherCondition condition) const;
    String getSeasonDescription(SeasonType season) const;
    String getTerrainDescription(TerrainType terrain) const;
    bool isOperationalConditions() const;
    float getEnvironmentalStress() const;
    
    // Event callbacks
    typedef void (*EnvironmentChangeCallback)(const EnvironmentalData& data);
    typedef void (*AdaptationCallback)(const AdaptationResponse& response);
    typedef void (*WeatherAlertCallback)(WeatherCondition condition, bool severe);
    
    void setEnvironmentChangeCallback(EnvironmentChangeCallback callback) { environmentCallback_ = callback; }
    void setAdaptationCallback(AdaptationCallback callback) { adaptationCallback_ = callback; }
    void setWeatherAlertCallback(WeatherAlertCallback callback) { weatherCallback_ = callback; }

private:
    // Core configuration and state
    AdaptationConfig config_;
    EnvironmentalData currentData_;
    std::vector<EnvironmentalData> dataHistory_;
    bool initialized_;
    
    // Current conditions
    WeatherCondition currentWeather_;
    SeasonType currentSeason_;
    TerrainType terrainType_;
    
    // Adaptation state
    AdaptationResponse currentAdaptation_;
    uint32_t lastAdaptationTime_;
    std::map<String, float> adaptationHistory_;
    
    // Forecasting
    std::vector<WeatherForecast> weatherForecasts_;
    uint32_t lastForecastUpdate_;
    
    // Seasonal patterns (could be loaded from external source)
    std::map<SeasonType, SeasonalPattern> seasonalPatterns_;
    
    // Callbacks
    EnvironmentChangeCallback environmentCallback_;
    AdaptationCallback adaptationCallback_;
    WeatherAlertCallback weatherCallback_;
    
    // Internal methods
    bool initializeSensors();
    bool loadSeasonalPatterns();
    AdaptationResponse calculateAdaptationResponse() const;
    float calculateEnvironmentalScore() const;
    bool shouldAdapt() const;
    
    // Weather analysis
    WeatherCondition analyzeWeatherFromSensors() const;
    bool detectSevereWeather() const;
    float calculateWeatherImpact() const;
    
    // Thermal management
    bool checkThermalLimits() const;
    float calculateThermalDerating() const;
    bool activateThermalProtection();
    
    // Seasonal optimization
    bool applySeasonalSettings(SeasonType season);
    std::vector<String> getSeasonalSpecies(SeasonType season) const;
    bool adjustForMigrationPeriod();
    
    // Power calculations
    float calculateTemperatureEfficiency() const;
    float calculateWeatherPowerFactor() const;
    bool needsWeatherPowerSaving() const;
    
    // Notification helpers
    void notifyEnvironmentChange(const EnvironmentalData& data);
    void notifyAdaptation(const AdaptationResponse& response);
    void notifyWeatherAlert(WeatherCondition condition, bool severe);
    
    // Data management
    void addToHistory(const EnvironmentalData& data);
    void pruneOldData();
    EnvironmentalData calculateAverageData(uint32_t hours) const;
};

// Global environmental adapter instance
extern EnvironmentalAdapter* g_environmentalAdapter;

// Utility functions for easy integration
bool initializeEnvironmentalAdapter(const AdaptationConfig& config);
bool updateEnvironment();
AdaptationResponse adaptToCurrentConditions();
bool isWeatherSuitable();
void cleanupEnvironmentalAdapter();

// Quick access functions
float getCurrentTemperature();
float getCurrentHumidity();
WeatherCondition getCurrentWeather();
bool isExtremeConditions();
String getEnvironmentalStatus();

#endif // ENVIRONMENTAL_ADAPTER_H