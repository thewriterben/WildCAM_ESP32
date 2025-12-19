/**
 * @file SensorManager.h
 * @brief Additional Sensor Management for WildCAM ESP32
 * 
 * This class provides support for environmental sensors including:
 * - BME280: Temperature, humidity, and pressure sensing
 * - GPS: Location tagging for wildlife images (optional)
 * - Light sensor: Day/night detection using BH1750 or analog sensor
 * 
 * @author WildCAM Project
 * @date 2024
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

//==============================================================================
// SENSOR DATA STRUCTURES
//==============================================================================

/**
 * @brief Environmental data from BME280 sensor
 */
struct EnvironmentalData {
    float temperature;      ///< Temperature in Celsius
    float humidity;         ///< Relative humidity percentage
    float pressure;         ///< Atmospheric pressure in hPa
    float altitude;         ///< Calculated altitude in meters
    bool valid;             ///< Data validity flag
    unsigned long timestamp; ///< Reading timestamp
};

/**
 * @brief GPS location data
 */
struct GPSData {
    float latitude;         ///< Latitude in decimal degrees
    float longitude;        ///< Longitude in decimal degrees
    float altitude;         ///< Altitude in meters
    int satellites;         ///< Number of satellites
    float hdop;             ///< Horizontal dilution of precision
    bool hasFix;            ///< GPS fix status
    unsigned long timestamp; ///< Reading timestamp
};

/**
 * @brief Light sensor data
 */
struct LightData {
    float lux;              ///< Light level in lux
    bool isDaytime;         ///< Day/night classification
    bool valid;             ///< Data validity flag
    unsigned long timestamp; ///< Reading timestamp
};

/**
 * @brief Day/night detection thresholds
 */
enum class LightCondition {
    NIGHT,          ///< Very dark (< 10 lux)
    TWILIGHT,       ///< Dim light (10-100 lux)
    CLOUDY,         ///< Overcast daylight (100-1000 lux)
    DAYLIGHT,       ///< Normal daylight (1000-10000 lux)
    BRIGHT_SUN      ///< Direct sunlight (> 10000 lux)
};

//==============================================================================
// SENSOR MANAGER CLASS
//==============================================================================

/**
 * @class SensorManager
 * @brief Manages BME280, GPS, and light sensors for wildlife monitoring
 * 
 * This class provides a unified interface for reading environmental data
 * from multiple sensors. All sensors are optional and can be enabled/disabled
 * independently.
 * 
 * Example usage:
 * @code
 * SensorManager sensors;
 * if (sensors.init()) {
 *     EnvironmentalData env = sensors.readEnvironmental();
 *     GPSData gps = sensors.readGPS();
 *     LightData light = sensors.readLight();
 *     
 *     if (light.isDaytime) {
 *         // Capture image in daylight mode
 *     }
 * }
 * @endcode
 */
class SensorManager {
public:
    /**
     * @brief Default constructor
     */
    SensorManager();
    
    /**
     * @brief Destructor
     */
    ~SensorManager();
    
    //==========================================================================
    // INITIALIZATION
    //==========================================================================
    
    /**
     * @brief Initialize all enabled sensors
     * @return true if at least one sensor initialized successfully
     */
    bool init();
    
    /**
     * @brief Initialize I2C bus
     * @param sda I2C SDA pin
     * @param scl I2C SCL pin
     * @return true if I2C initialized successfully
     */
    bool initI2C(int sda, int scl);
    
    /**
     * @brief Initialize BME280 sensor
     * @param address I2C address (default: 0x76)
     * @return true if BME280 initialized successfully
     */
    bool initBME280(uint8_t address = 0x76);
    
    /**
     * @brief Initialize GPS module
     * @param rxPin GPS RX pin
     * @param txPin GPS TX pin
     * @param baudRate Serial baud rate (default: 9600)
     * @return true if GPS initialized successfully
     */
    bool initGPS(int rxPin, int txPin, int baudRate = 9600);
    
    /**
     * @brief Initialize light sensor
     * @param pin Analog pin for light sensor (use -1 for I2C BH1750)
     * @param address I2C address for BH1750 (default: 0x23)
     * @return true if light sensor initialized successfully
     */
    bool initLightSensor(int pin = -1, uint8_t address = 0x23);
    
    //==========================================================================
    // DATA READING
    //==========================================================================
    
    /**
     * @brief Read environmental data from BME280
     * @return EnvironmentalData struct with current readings
     */
    EnvironmentalData readEnvironmental();
    
    /**
     * @brief Read GPS location data
     * @return GPSData struct with current location
     */
    GPSData readGPS();
    
    /**
     * @brief Read light level
     * @return LightData struct with current light level
     */
    LightData readLight();
    
    /**
     * @brief Update GPS data (call regularly in main loop)
     * @return true if new GPS data available
     */
    bool updateGPS();
    
    //==========================================================================
    // UTILITY FUNCTIONS
    //==========================================================================
    
    /**
     * @brief Get current light condition
     * @return LightCondition enum value
     */
    LightCondition getLightCondition();
    
    /**
     * @brief Get formatted location string
     * @return String in format "lat,lon,alt" or "No GPS"
     */
    String getLocationString();
    
    /**
     * @brief Print all sensor status to Serial
     */
    void printStatus();
    
    /**
     * @brief Check if BME280 is available
     * @return true if BME280 is initialized and working
     */
    bool isBME280Available();
    
    /**
     * @brief Check if GPS is available
     * @return true if GPS is initialized
     */
    bool isGPSAvailable();
    
    /**
     * @brief Check if GPS has a valid fix
     * @return true if GPS has a fix
     */
    bool hasGPSFix();
    
    /**
     * @brief Check if light sensor is available
     * @return true if light sensor is initialized
     */
    bool isLightSensorAvailable();
    
    /**
     * @brief Set daytime threshold for light sensor
     * @param luxThreshold Lux value above which is considered daytime
     */
    void setDaytimeThreshold(float luxThreshold);
    
    /**
     * @brief Set sea level pressure for altitude calculation
     * @param pressure Sea level pressure in hPa (default: 1013.25)
     */
    void setSeaLevelPressure(float pressure);

private:
    // Sensor status flags
    bool _bme280Initialized;
    bool _gpsInitialized;
    bool _lightSensorInitialized;
    bool _i2cInitialized;
    
    // BME280 configuration
    uint8_t _bme280Address;
    float _seaLevelPressure;
    
    // GPS configuration
    int _gpsRxPin;
    int _gpsTxPin;
    
    // Light sensor configuration
    int _lightSensorPin;       // -1 for I2C (BH1750)
    uint8_t _lightSensorAddress;
    float _daytimeThreshold;
    
    // Cached data
    EnvironmentalData _lastEnvData;
    GPSData _lastGPSData;
    LightData _lastLightData;
    
    // Private helper methods
    bool readBME280Raw(float* temp, float* hum, float* pres);
    float readLightAnalog();
    float readLightBH1750();
    bool scanI2CDevice(uint8_t address);
};

#endif // SENSOR_MANAGER_H
