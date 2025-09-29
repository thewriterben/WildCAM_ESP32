/**
 * @file wildlife_telemetry.h
 * @brief Wildlife-Specific Telemetry System for ESP32WildlifeCAM
 * 
 * Handles collection, processing, and mesh transmission of wildlife monitoring data:
 * - Motion detection events
 * - Environmental sensor readings
 * - Camera capture events
 * - Power and solar charging status
 * - GPS location data
 * - Device health metrics
 */

#ifndef WILDLIFE_TELEMETRY_H
#define WILDLIFE_TELEMETRY_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include "mesh_interface.h"

// ===========================
// TELEMETRY DATA STRUCTURES
// ===========================

// Motion detection event
struct MotionEvent {
    unsigned long timestamp;
    uint16_t confidence;        // Detection confidence (0-100)
    uint16_t duration;          // Motion duration in milliseconds
    uint16_t pixelChanges;      // Number of pixels changed
    int16_t triggerZone;        // Detection zone ID (-1 for global)
    float batteryVoltage;       // Battery voltage at detection
    int16_t temperature;        // Temperature at detection (°C * 10)
    bool photoTaken;            // Whether photo was captured
    String photoFilename;       // Filename of captured photo
};

// Environmental sensor data
struct EnvironmentalData {
    unsigned long timestamp;
    
    // Basic environmental (BME280)
    float temperature;          // Temperature in Celsius
    float humidity;             // Relative humidity percentage
    float pressure;             // Atmospheric pressure in hPa
    uint16_t lightLevel;        // Light level (0-1023) - basic compatibility
    float windSpeed;            // Wind speed in m/s
    uint16_t windDirection;     // Wind direction in degrees
    bool sensorValid;           // Sensor readings validity
    
    // Advanced temperature monitoring (DS18B20)
    float ground_temperature;   // Ground temperature
    float enclosure_temperature; // Enclosure temperature
    float battery_temperature;  // Battery temperature
    
    // Advanced light monitoring (TSL2591)
    float visible_light;        // Visible light in lux
    float infrared_light;       // IR component
    float full_spectrum_light;  // Full spectrum
    
    // Air quality (SGP30)
    uint16_t tvoc_ppb;         // Total VOC in ppb
    uint16_t eco2_ppm;         // Equivalent CO2 in ppm
    
    // Power monitoring (MAX17048)
    float battery_voltage;      // Battery voltage
    float battery_percentage;   // Battery percentage
    float solar_voltage;        // Solar panel voltage
    
    // Derived environmental calculations
    float dew_point;           // Dew point temperature
    float heat_index;          // Heat index
    float vapor_pressure;      // Vapor pressure
    
    // Wildlife/photography indices (0-100%)
    uint8_t wildlife_activity_index;    // Wildlife activity prediction
    uint8_t photography_conditions;     // Photography conditions assessment
    uint8_t comfort_index;             // Overall environmental comfort
    
    // Diagnostics
    uint32_t sensor_errors;    // Sensor error flags
};

// Power system status
struct PowerStatus {
    unsigned long timestamp;
    float batteryVoltage;       // Battery voltage
    float solarVoltage;         // Solar panel voltage
    float chargingCurrent;      // Charging current in mA
    uint8_t batteryLevel;       // Battery level percentage
    bool isCharging;            // Charging status
    bool lowBattery;            // Low battery warning
    uint32_t uptimeSeconds;     // System uptime
    float powerConsumption;     // Current power consumption in mW
};

// GPS location data
struct LocationData {
    unsigned long timestamp;
    double latitude;            // Latitude in decimal degrees
    double longitude;           // Longitude in decimal degrees
    float altitude;             // Altitude in meters
    float accuracy;             // GPS accuracy in meters
    uint8_t satellites;         // Number of satellites
    bool fixValid;              // GPS fix validity
    float speed;                // Speed in m/s
    float heading;              // Heading in degrees
};

// Device health metrics
struct DeviceHealth {
    unsigned long timestamp;
    float cpuTemperature;       // CPU temperature
    uint32_t freeHeap;          // Free heap memory
    uint32_t minFreeHeap;       // Minimum free heap since boot
    uint8_t wifiSignal;         // WiFi signal strength (if enabled)
    uint8_t loraSignal;         // LoRa signal strength
    uint32_t resetReason;       // Last reset reason
    uint32_t errorCount;        // Error counter
    bool sdCardStatus;          // SD card status
    bool cameraStatus;          // Camera status
};

// Camera capture event
struct CaptureEvent {
    unsigned long timestamp;
    String filename;            // Image filename
    uint32_t fileSize;          // File size in bytes
    uint16_t imageWidth;        // Image width
    uint16_t imageHeight;       // Image height
    uint8_t jpegQuality;        // JPEG quality setting
    uint16_t captureTime;       // Capture time in milliseconds
    bool motionTriggered;       // Motion triggered capture
    bool scheduled;             // Scheduled capture
    bool transmitted;           // Transmitted over mesh
};

// Wildlife detection result (AI/ML)
struct WildlifeDetection {
    unsigned long timestamp;
    String species;             // Detected species
    float confidence;           // Detection confidence (0-1)
    uint16_t boundingBoxX;      // Bounding box coordinates
    uint16_t boundingBoxY;
    uint16_t boundingBoxW;
    uint16_t boundingBoxH;
    uint8_t animalCount;        // Number of animals detected
    String behavior;            // Detected behavior
    bool endangered;            // Endangered species flag
    String photoFilename;       // Associated photo
};

// Telemetry packet types
enum TelemetryType {
    TELEMETRY_MOTION = 0,
    TELEMETRY_ENVIRONMENTAL = 1,
    TELEMETRY_POWER = 2,
    TELEMETRY_LOCATION = 3,
    TELEMETRY_HEALTH = 4,
    TELEMETRY_CAPTURE = 5,
    TELEMETRY_WILDLIFE = 6,
    TELEMETRY_STATUS = 7,
    TELEMETRY_ALERT = 8
};

// Telemetry configuration
struct TelemetryConfig {
    bool motionEnabled;
    bool environmentalEnabled;
    bool powerEnabled;
    bool locationEnabled;
    bool healthEnabled;
    bool captureEnabled;
    bool wildlifeEnabled;
    
    uint32_t motionInterval;        // Motion telemetry interval
    uint32_t environmentalInterval; // Environmental telemetry interval
    uint32_t powerInterval;         // Power telemetry interval
    uint32_t locationInterval;      // Location telemetry interval
    uint32_t healthInterval;        // Health telemetry interval
    
    bool transmitOverMesh;          // Transmit over mesh network
    bool storeLocally;              // Store data locally
    uint32_t maxStoredRecords;      // Maximum stored records per type
    
    uint8_t compressionLevel;       // Data compression level
    bool encryptData;               // Encrypt telemetry data
};

// ===========================
// WILDLIFE TELEMETRY CLASS
// ===========================

class WildlifeTelemetry {
public:
    WildlifeTelemetry();
    ~WildlifeTelemetry();
    
    // Initialization and configuration
    bool init(MeshInterface* meshInterface);
    bool configure(const TelemetryConfig& config);
    void cleanup();
    
    // Data collection methods
    bool recordMotionEvent(const MotionEvent& event);
    bool recordEnvironmentalData(const EnvironmentalData& data);
    bool recordPowerStatus(const PowerStatus& status);
    bool recordLocationData(const LocationData& location);
    bool recordDeviceHealth(const DeviceHealth& health);
    bool recordCaptureEvent(const CaptureEvent& event);
    bool recordWildlifeDetection(const WildlifeDetection& detection);
    
    // Automatic data collection
    bool startAutomaticCollection();
    bool stopAutomaticCollection();
    bool isCollecting() const;
    
    // Data retrieval
    std::vector<MotionEvent> getMotionEvents(unsigned long since = 0) const;
    std::vector<EnvironmentalData> getEnvironmentalData(unsigned long since = 0) const;
    std::vector<PowerStatus> getPowerHistory(unsigned long since = 0) const;
    std::vector<LocationData> getLocationHistory(unsigned long since = 0) const;
    std::vector<DeviceHealth> getHealthHistory(unsigned long since = 0) const;
    std::vector<CaptureEvent> getCaptureHistory(unsigned long since = 0) const;
    std::vector<WildlifeDetection> getWildlifeDetections(unsigned long since = 0) const;
    
    // Data transmission
    bool transmitTelemetry(TelemetryType type, uint32_t destination = 0);
    bool transmitMotionEvent(const MotionEvent& event, uint32_t destination = 0);
    bool transmitEnvironmentalData(const EnvironmentalData& data, uint32_t destination = 0);
    bool transmitPowerStatus(const PowerStatus& status, uint32_t destination = 0);
    bool transmitLocationData(const LocationData& location, uint32_t destination = 0);
    bool transmitDeviceHealth(const DeviceHealth& health, uint32_t destination = 0);
    bool transmitCaptureEvent(const CaptureEvent& event, uint32_t destination = 0);
    bool transmitWildlifeDetection(const WildlifeDetection& detection, uint32_t destination = 0);
    
    // Data analysis and alerts
    bool checkForAlerts();
    bool sendAlert(const String& alertType, const String& message, uint32_t destination = 0);
    
    // Status and statistics
    uint32_t getTotalMotionEvents() const;
    uint32_t getTotalCaptureEvents() const;
    uint32_t getTotalWildlifeDetections() const;
    float getAverageBatteryLevel() const;
    float getAverageTemperature() const;
    String getLastSpeciesDetected() const;
    
    // Data management
    bool exportTelemetryData(const String& filename);
    bool importTelemetryData(const String& filename);
    bool clearOldData(unsigned long olderThan);
    bool optimizeStorage();
    
    // Configuration management
    TelemetryConfig getConfig() const;
    bool setConfig(const TelemetryConfig& config);
    bool saveConfigToFile(const String& filename);
    bool loadConfigFromFile(const String& filename);
    
    // Processing and maintenance
    void process();                 // Call regularly in main loop
    void performMaintenance();      // Periodic maintenance
    
    // Event callbacks
    typedef void (*MotionEventCallback)(const MotionEvent& event);
    typedef void (*WildlifeDetectionCallback)(const WildlifeDetection& detection);
    typedef void (*AlertCallback)(const String& alertType, const String& message);
    typedef void (*DataTransmittedCallback)(TelemetryType type, bool success);
    
    void setMotionEventCallback(MotionEventCallback callback);
    void setWildlifeDetectionCallback(WildlifeDetectionCallback callback);
    void setAlertCallback(AlertCallback callback);
    void setDataTransmittedCallback(DataTransmittedCallback callback);

private:
    // Internal state
    bool initialized_;
    bool collecting_;
    MeshInterface* meshInterface_;
    TelemetryConfig config_;
    
    // Data storage
    std::vector<MotionEvent> motionEvents_;
    std::vector<EnvironmentalData> environmentalData_;
    std::vector<PowerStatus> powerHistory_;
    std::vector<LocationData> locationHistory_;
    std::vector<DeviceHealth> healthHistory_;
    std::vector<CaptureEvent> captureHistory_;
    std::vector<WildlifeDetection> wildlifeDetections_;
    
    // Timing
    unsigned long lastMotionTelemetry_;
    unsigned long lastEnvironmentalTelemetry_;
    unsigned long lastPowerTelemetry_;
    unsigned long lastLocationTelemetry_;
    unsigned long lastHealthTelemetry_;
    unsigned long lastMaintenance_;
    
    // Statistics
    uint32_t totalTransmissions_;
    uint32_t transmissionErrors_;
    uint32_t dataCompressionRatio_;
    
    // Callbacks
    MotionEventCallback motionEventCallback_;
    WildlifeDetectionCallback wildlifeDetectionCallback_;
    AlertCallback alertCallback_;
    DataTransmittedCallback dataTransmittedCallback_;
    
    // Internal methods
    bool collectEnvironmentalData();
    bool collectPowerStatus();
    bool collectLocationData();
    bool collectDeviceHealth();
    
    // Data serialization
    std::vector<uint8_t> serializeMotionEvent(const MotionEvent& event);
    std::vector<uint8_t> serializeEnvironmentalData(const EnvironmentalData& data);
    std::vector<uint8_t> serializePowerStatus(const PowerStatus& status);
    std::vector<uint8_t> serializeLocationData(const LocationData& location);
    std::vector<uint8_t> serializeDeviceHealth(const DeviceHealth& health);
    std::vector<uint8_t> serializeCaptureEvent(const CaptureEvent& event);
    std::vector<uint8_t> serializeWildlifeDetection(const WildlifeDetection& detection);
    
    // Data compression
    std::vector<uint8_t> compressData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> decompressData(const std::vector<uint8_t>& compressedData);
    
    // Storage management
    void limitStorageSize();
    void removeOldestRecords(TelemetryType type, size_t maxRecords);
    
    // Alert detection
    bool checkBatteryAlert();
    bool checkTemperatureAlert();
    bool checkMotionAlert();
    bool checkSystemHealthAlert();
    
    // Utility methods
    unsigned long getCurrentTimestamp() const;
    String formatTimestamp(unsigned long timestamp) const;
    float calculateBatteryTrend() const;
    float calculateTemperatureTrend() const;
};

// ===========================
// UTILITY FUNCTIONS
// ===========================

// Factory function
WildlifeTelemetry* createWildlifeTelemetry(MeshInterface* meshInterface);

// Configuration helpers
TelemetryConfig createDefaultTelemetryConfig();
TelemetryConfig createLowPowerTelemetryConfig();
TelemetryConfig createResearchTelemetryConfig();

// Data analysis utilities
float calculateAverageValue(const std::vector<float>& values, size_t maxSamples = 0);
float calculateTrend(const std::vector<float>& values, const std::vector<unsigned long>& timestamps);
bool detectAnomaly(float value, const std::vector<float>& historicalValues, float threshold = 2.0);

// Time utilities
String formatDuration(unsigned long milliseconds);
String formatDataRate(uint32_t bytesPerSecond);
unsigned long parseTimestamp(const String& timestampStr);

// Validation functions
bool isValidMotionEvent(const MotionEvent& event);
bool isValidEnvironmentalData(const EnvironmentalData& data);
bool isValidPowerStatus(const PowerStatus& status);
bool isValidLocationData(const LocationData& location);
bool isValidDeviceHealth(const DeviceHealth& health);
bool isValidCaptureEvent(const CaptureEvent& event);
bool isValidWildlifeDetection(const WildlifeDetection& detection);

#endif // WILDLIFE_TELEMETRY_H