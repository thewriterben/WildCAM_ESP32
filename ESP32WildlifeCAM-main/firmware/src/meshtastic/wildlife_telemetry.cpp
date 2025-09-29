/**
 * @file wildlife_telemetry.cpp
 * @brief Wildlife-Specific Telemetry System Implementation for ESP32WildlifeCAM
 * 
 * Comprehensive implementation of wildlife monitoring telemetry with
 * mesh network integration and data management capabilities.
 */

#include "wildlife_telemetry.h"
#include "../debug_utils.h"
#include "../config.h"
#include "../sensors/advanced_environmental_sensors.h"
#include "../../src/data/storage_manager.h"
#include <FS.h>
#include <LittleFS.h>
#include <esp_system.h>

// External function from environmental integration
extern AdvancedEnvironmentalData getLatestEnvironmentalData();
extern bool areEnvironmentalSensorsHealthy();

// ===========================
// CONSTRUCTOR/DESTRUCTOR
// ===========================

WildlifeTelemetry::WildlifeTelemetry()
    : initialized_(false)
    , collecting_(false)
    , meshInterface_(nullptr)
    , lastMotionTelemetry_(0)
    , lastEnvironmentalTelemetry_(0)
    , lastPowerTelemetry_(0)
    , lastLocationTelemetry_(0)
    , lastHealthTelemetry_(0)
    , lastMaintenance_(0)
    , totalTransmissions_(0)
    , transmissionErrors_(0)
    , dataCompressionRatio_(0)
    , motionEventCallback_(nullptr)
    , wildlifeDetectionCallback_(nullptr)
    , alertCallback_(nullptr)
    , dataTransmittedCallback_(nullptr)
{
    // Initialize default configuration
    config_ = createDefaultTelemetryConfig();
}

WildlifeTelemetry::~WildlifeTelemetry() {
    cleanup();
}

// ===========================
// INITIALIZATION
// ===========================

bool WildlifeTelemetry::init(MeshInterface* meshInterface) {
    DEBUG_PRINTLN("WildlifeTelemetry: Initializing...");
    
    if (!meshInterface) {
        DEBUG_PRINTLN("WildlifeTelemetry: Invalid mesh interface");
        return false;
    }
    
    meshInterface_ = meshInterface;
    
    // Initialize LittleFS for data storage
    if (!LittleFS.begin()) {
        DEBUG_PRINTLN("WildlifeTelemetry: Failed to initialize LittleFS");
        return false;
    }
    
    // Load configuration if exists
    if (LittleFS.exists("/telemetry_config.json")) {
        loadConfigFromFile("/telemetry_config.json");
    }
    
    // Reserve storage space
    motionEvents_.reserve(config_.maxStoredRecords);
    environmentalData_.reserve(config_.maxStoredRecords);
    powerHistory_.reserve(config_.maxStoredRecords);
    locationHistory_.reserve(config_.maxStoredRecords);
    healthHistory_.reserve(config_.maxStoredRecords);
    captureHistory_.reserve(config_.maxStoredRecords);
    wildlifeDetections_.reserve(config_.maxStoredRecords);
    
    initialized_ = true;
    
    DEBUG_PRINTLN("WildlifeTelemetry: Initialized successfully");
    
    return true;
}

bool WildlifeTelemetry::configure(const TelemetryConfig& config) {
    if (!initialized_) {
        return false;
    }
    
    config_ = config;
    
    // Save configuration
    saveConfigToFile("/telemetry_config.json");
    
    DEBUG_PRINTLN("WildlifeTelemetry: Configuration updated");
    
    return true;
}

void WildlifeTelemetry::cleanup() {
    stopAutomaticCollection();
    
    // Clear data vectors
    motionEvents_.clear();
    environmentalData_.clear();
    powerHistory_.clear();
    locationHistory_.clear();
    healthHistory_.clear();
    captureHistory_.clear();
    wildlifeDetections_.clear();
    
    initialized_ = false;
}

// ===========================
// DATA COLLECTION
// ===========================

bool WildlifeTelemetry::recordMotionEvent(const MotionEvent& event) {
    if (!initialized_ || !isValidMotionEvent(event)) {
        return false;
    }
    
    // Add to storage
    motionEvents_.push_back(event);
    limitStorageSize();
    
    // Call callback if set
    if (motionEventCallback_) {
        motionEventCallback_(event);
    }
    
    // Transmit if enabled
    if (config_.transmitOverMesh) {
        transmitMotionEvent(event);
    }
    
    DEBUG_PRINTF("WildlifeTelemetry: Recorded motion event (confidence: %d%%)\n", 
                 event.confidence);
    
    return true;
}

bool WildlifeTelemetry::recordEnvironmentalData(const EnvironmentalData& data) {
    if (!initialized_ || !isValidEnvironmentalData(data)) {
        return false;
    }
    
    environmentalData_.push_back(data);
    limitStorageSize();
    
    // Transmit if enabled and interval met
    if (config_.transmitOverMesh && 
        millis() - lastEnvironmentalTelemetry_ > config_.environmentalInterval) {
        transmitEnvironmentalData(data);
        lastEnvironmentalTelemetry_ = millis();
    }
    
    return true;
}

bool WildlifeTelemetry::recordPowerStatus(const PowerStatus& status) {
    if (!initialized_ || !isValidPowerStatus(status)) {
        return false;
    }
    
    powerHistory_.push_back(status);
    limitStorageSize();
    
    // Check for power alerts
    if (status.lowBattery && alertCallback_) {
        alertCallback_("LOW_BATTERY", 
                      "Battery level critical: " + String(status.batteryLevel) + "%");
    }
    
    // Transmit if enabled and interval met
    if (config_.transmitOverMesh && 
        millis() - lastPowerTelemetry_ > config_.powerInterval) {
        transmitPowerStatus(status);
        lastPowerTelemetry_ = millis();
    }
    
    return true;
}

bool WildlifeTelemetry::recordLocationData(const LocationData& location) {
    if (!initialized_ || !isValidLocationData(location)) {
        return false;
    }
    
    locationHistory_.push_back(location);
    limitStorageSize();
    
    // Transmit if enabled and interval met
    if (config_.transmitOverMesh && 
        millis() - lastLocationTelemetry_ > config_.locationInterval) {
        transmitLocationData(location);
        lastLocationTelemetry_ = millis();
    }
    
    return true;
}

bool WildlifeTelemetry::recordDeviceHealth(const DeviceHealth& health) {
    if (!initialized_ || !isValidDeviceHealth(health)) {
        return false;
    }
    
    healthHistory_.push_back(health);
    limitStorageSize();
    
    // Check for health alerts
    if (health.cpuTemperature > 80.0 && alertCallback_) {
        alertCallback_("HIGH_TEMPERATURE", 
                      "CPU temperature high: " + String(health.cpuTemperature) + "°C");
    }
    
    if (health.freeHeap < 10000 && alertCallback_) {
        alertCallback_("LOW_MEMORY", 
                      "Free heap low: " + String(health.freeHeap) + " bytes");
    }
    
    // Transmit if enabled and interval met
    if (config_.transmitOverMesh && 
        millis() - lastHealthTelemetry_ > config_.healthInterval) {
        transmitDeviceHealth(health);
        lastHealthTelemetry_ = millis();
    }
    
    return true;
}

bool WildlifeTelemetry::recordCaptureEvent(const CaptureEvent& event) {
    if (!initialized_ || !isValidCaptureEvent(event)) {
        return false;
    }
    
    captureHistory_.push_back(event);
    limitStorageSize();
    
    // Transmit if enabled
    if (config_.transmitOverMesh) {
        transmitCaptureEvent(event);
    }
    
    DEBUG_PRINTF("WildlifeTelemetry: Recorded capture event (%s)\n", 
                 event.filename.c_str());
    
    return true;
}

bool WildlifeTelemetry::recordWildlifeDetection(const WildlifeDetection& detection) {
    if (!initialized_ || !isValidWildlifeDetection(detection)) {
        return false;
    }
    
    wildlifeDetections_.push_back(detection);
    limitStorageSize();
    
    // Call callback if set
    if (wildlifeDetectionCallback_) {
        wildlifeDetectionCallback_(detection);
    }
    
    // Send alert for endangered species
    if (detection.endangered && alertCallback_) {
        alertCallback_("ENDANGERED_SPECIES", 
                      "Detected: " + detection.species + 
                      " (confidence: " + String(detection.confidence * 100) + "%)");
    }
    
    // Transmit if enabled
    if (config_.transmitOverMesh) {
        transmitWildlifeDetection(detection);
    }
    
    DEBUG_PRINTF("WildlifeTelemetry: Recorded wildlife detection (%s, %.1f%%)\n", 
                 detection.species.c_str(), detection.confidence * 100);
    
    return true;
}

// ===========================
// AUTOMATIC COLLECTION
// ===========================

bool WildlifeTelemetry::startAutomaticCollection() {
    if (!initialized_) {
        return false;
    }
    
    collecting_ = true;
    
    DEBUG_PRINTLN("WildlifeTelemetry: Started automatic collection");
    
    return true;
}

bool WildlifeTelemetry::stopAutomaticCollection() {
    collecting_ = false;
    
    DEBUG_PRINTLN("WildlifeTelemetry: Stopped automatic collection");
    
    return true;
}

bool WildlifeTelemetry::isCollecting() const {
    return collecting_;
}

// ===========================
// DATA TRANSMISSION
// ===========================

bool WildlifeTelemetry::transmitMotionEvent(const MotionEvent& event, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> data = serializeMotionEvent(event);
    
    if (config_.compressionLevel > 0) {
        data = compressData(data);
    }
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_WILDLIFE, 
                                              data, PRIORITY_NORMAL);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_MOTION, success);
    }
    
    return success;
}

bool WildlifeTelemetry::transmitEnvironmentalData(const EnvironmentalData& data, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> payload = serializeEnvironmentalData(data);
    
    if (config_.compressionLevel > 0) {
        payload = compressData(payload);
    }
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_TELEMETRY, 
                                              payload, PRIORITY_BACKGROUND);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_ENVIRONMENTAL, success);
    }
    
    return success;
}

bool WildlifeTelemetry::transmitPowerStatus(const PowerStatus& status, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> data = serializePowerStatus(status);
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_TELEMETRY, 
                                              data, PRIORITY_NORMAL);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_POWER, success);
    }
    
    return success;
}

bool WildlifeTelemetry::transmitLocationData(const LocationData& location, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> data = serializeLocationData(location);
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_POSITION, 
                                              data, PRIORITY_NORMAL);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_LOCATION, success);
    }
    
    return success;
}

bool WildlifeTelemetry::transmitDeviceHealth(const DeviceHealth& health, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> data = serializeDeviceHealth(health);
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_TELEMETRY, 
                                              data, PRIORITY_BACKGROUND);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_HEALTH, success);
    }
    
    return success;
}

bool WildlifeTelemetry::transmitCaptureEvent(const CaptureEvent& event, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> data = serializeCaptureEvent(event);
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_WILDLIFE, 
                                              data, PRIORITY_HIGH);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_CAPTURE, success);
    }
    
    return success;
}

bool WildlifeTelemetry::transmitWildlifeDetection(const WildlifeDetection& detection, uint32_t destination) {
    if (!meshInterface_) {
        return false;
    }
    
    std::vector<uint8_t> data = serializeWildlifeDetection(detection);
    
    RoutingPriority priority = detection.endangered ? PRIORITY_HIGH : PRIORITY_NORMAL;
    
    bool success = meshInterface_->sendMessage(destination, PACKET_TYPE_WILDLIFE, 
                                              data, priority);
    
    if (success) {
        totalTransmissions_++;
    } else {
        transmissionErrors_++;
    }
    
    if (dataTransmittedCallback_) {
        dataTransmittedCallback_(TELEMETRY_WILDLIFE, success);
    }
    
    return success;
}

// ===========================
// DATA RETRIEVAL
// ===========================

std::vector<MotionEvent> WildlifeTelemetry::getMotionEvents(unsigned long since) const {
    std::vector<MotionEvent> result;
    
    for (const auto& event : motionEvents_) {
        if (event.timestamp >= since) {
            result.push_back(event);
        }
    }
    
    return result;
}

std::vector<EnvironmentalData> WildlifeTelemetry::getEnvironmentalData(unsigned long since) const {
    std::vector<EnvironmentalData> result;
    
    for (const auto& data : environmentalData_) {
        if (data.timestamp >= since) {
            result.push_back(data);
        }
    }
    
    return result;
}

std::vector<PowerStatus> WildlifeTelemetry::getPowerHistory(unsigned long since) const {
    std::vector<PowerStatus> result;
    
    for (const auto& status : powerHistory_) {
        if (status.timestamp >= since) {
            result.push_back(status);
        }
    }
    
    return result;
}

std::vector<CaptureEvent> WildlifeTelemetry::getCaptureHistory(unsigned long since) const {
    std::vector<CaptureEvent> result;
    
    for (const auto& event : captureHistory_) {
        if (event.timestamp >= since) {
            result.push_back(event);
        }
    }
    
    return result;
}

std::vector<WildlifeDetection> WildlifeTelemetry::getWildlifeDetections(unsigned long since) const {
    std::vector<WildlifeDetection> result;
    
    for (const auto& detection : wildlifeDetections_) {
        if (detection.timestamp >= since) {
            result.push_back(detection);
        }
    }
    
    return result;
}

// ===========================
// PROCESSING AND MAINTENANCE
// ===========================

void WildlifeTelemetry::process() {
    if (!initialized_ || !collecting_) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Collect automatic telemetry data
    if (config_.environmentalEnabled && 
        currentTime - lastEnvironmentalTelemetry_ > config_.environmentalInterval) {
        collectEnvironmentalData();
    }
    
    if (config_.powerEnabled && 
        currentTime - lastPowerTelemetry_ > config_.powerInterval) {
        collectPowerStatus();
    }
    
    if (config_.locationEnabled && 
        currentTime - lastLocationTelemetry_ > config_.locationInterval) {
        collectLocationData();
    }
    
    if (config_.healthEnabled && 
        currentTime - lastHealthTelemetry_ > config_.healthInterval) {
        collectDeviceHealth();
    }
    
    // Perform maintenance
    if (currentTime - lastMaintenance_ > 300000) {  // Every 5 minutes
        performMaintenance();
        lastMaintenance_ = currentTime;
    }
}

void WildlifeTelemetry::performMaintenance() {
    // Check for alerts
    checkForAlerts();
    
    // Optimize storage
    optimizeStorage();
    
    // Update statistics
    // Statistics are updated in real-time during data recording
}

// ===========================
// AUTOMATIC DATA COLLECTION
// ===========================

bool WildlifeTelemetry::collectEnvironmentalData() {
    EnvironmentalData data;
    data.timestamp = getCurrentTimestamp();
    
    // Get real environmental data from advanced sensors
    AdvancedEnvironmentalData advancedData = getLatestEnvironmentalData();
    
    // Map advanced environmental data to telemetry structure
    // Basic environmental (BME280)
    data.temperature = advancedData.temperature;
    data.humidity = advancedData.humidity;
    data.pressure = advancedData.pressure;
    data.lightLevel = (uint16_t)constrain(advancedData.visible_light, 0, 1023); // Backward compatibility
    
    // Wind data would come from dedicated wind sensors (not implemented yet)
    data.windSpeed = 0.0;           // Would read from wind sensor
    data.windDirection = 0;         // Would read from wind sensor
    
    // Advanced temperature monitoring (DS18B20)
    data.ground_temperature = advancedData.ground_temperature;
    data.enclosure_temperature = advancedData.enclosure_temperature;
    data.battery_temperature = advancedData.battery_temperature;
    
    // Advanced light monitoring (TSL2591)
    data.visible_light = advancedData.visible_light;
    data.infrared_light = advancedData.infrared_light;
    data.full_spectrum_light = advancedData.full_spectrum_light;
    
    // Air quality (SGP30)
    data.tvoc_ppb = advancedData.tvoc_ppb;
    data.eco2_ppm = advancedData.eco2_ppm;
    
    // Power monitoring (MAX17048)
    data.battery_voltage = advancedData.battery_voltage;
    data.battery_percentage = advancedData.battery_percentage;
    data.solar_voltage = advancedData.solar_voltage;
    
    // Derived environmental calculations
    data.dew_point = advancedData.dew_point;
    data.heat_index = advancedData.heat_index;
    data.vapor_pressure = advancedData.vapor_pressure;
    
    // Wildlife/photography indices
    data.wildlife_activity_index = advancedData.wildlife_activity_index;
    data.photography_conditions = advancedData.photography_conditions;
    data.comfort_index = advancedData.comfort_index;
    
    // Diagnostics
    data.sensor_errors = advancedData.sensor_errors;
    
    // Sensor validity based on actual sensor health
    data.sensorValid = areEnvironmentalSensorsHealthy() && 
                      (advancedData.timestamp > 0) &&
                      (advancedData.bme280_valid);
    
    DEBUG_PRINTF("Environmental data collected: T=%.1f°C, H=%.1f%%, P=%.1fhPa, Light=%.1flux, Activity=%d%%, Photo=%d%%, Valid=%s\n",
                data.temperature, data.humidity, data.pressure, data.visible_light,
                data.wildlife_activity_index, data.photography_conditions,
                data.sensorValid ? "YES" : "NO");
    
    return recordEnvironmentalData(data);
}

bool WildlifeTelemetry::collectPowerStatus() {
    PowerStatus status;
    status.timestamp = getCurrentTimestamp();
    
    // TODO: Read from actual power management system
    // For now, use placeholder values
    status.batteryVoltage = 3.8;    // Would read from ADC
    status.solarVoltage = 0.0;      // Would read from solar panel ADC
    status.chargingCurrent = 0.0;   // Would read from current sensor
    status.batteryLevel = 75;       // Would calculate from voltage
    status.isCharging = false;      // Would determine from current
    status.lowBattery = status.batteryLevel < 20;
    status.uptimeSeconds = millis() / 1000;
    status.powerConsumption = 0.0;  // Would calculate from current consumption
    
    return recordPowerStatus(status);
}

bool WildlifeTelemetry::collectLocationData() {
    if (!GPS_MESH_ENABLED) {
        return false;
    }
    
    LocationData location;
    location.timestamp = getCurrentTimestamp();
    
    // TODO: Read from actual GPS module
    // For now, use placeholder values
    location.latitude = 0.0;
    location.longitude = 0.0;
    location.altitude = 0.0;
    location.accuracy = 0.0;
    location.satellites = 0;
    location.fixValid = false;
    location.speed = 0.0;
    location.heading = 0.0;
    
    return recordLocationData(location);
}

bool WildlifeTelemetry::collectDeviceHealth() {
    DeviceHealth health;
    health.timestamp = getCurrentTimestamp();
    
    // Collect actual system health data
    health.cpuTemperature = 45.0;   // TODO: Read from temperature sensor
    health.freeHeap = ESP.getFreeHeap();
    health.minFreeHeap = ESP.getMinFreeHeap();
    health.wifiSignal = 0;          // WiFi disabled in config
    health.loraSignal = 80;         // TODO: Get from LoRa driver
    health.resetReason = ESP.getResetReason();
    health.errorCount = 0;          // TODO: Implement error tracking
    health.sdCardStatus = StorageManager::initialize(); // Check SD card status
    health.cameraStatus = true;     // TODO: Check camera status
    
    return recordDeviceHealth(health);
}

// ===========================
// DATA SERIALIZATION
// ===========================

std::vector<uint8_t> WildlifeTelemetry::serializeMotionEvent(const MotionEvent& event) {
    DynamicJsonDocument doc(512);
    doc["type"] = "motion";
    doc["timestamp"] = event.timestamp;
    doc["confidence"] = event.confidence;
    doc["duration"] = event.duration;
    doc["pixelChanges"] = event.pixelChanges;
    doc["triggerZone"] = event.triggerZone;
    doc["batteryVoltage"] = event.batteryVoltage;
    doc["temperature"] = event.temperature;
    doc["photoTaken"] = event.photoTaken;
    doc["photoFilename"] = event.photoFilename;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

std::vector<uint8_t> WildlifeTelemetry::serializeEnvironmentalData(const EnvironmentalData& data) {
    DynamicJsonDocument doc(1024); // Increased size for additional fields
    doc["type"] = "environmental";
    doc["timestamp"] = data.timestamp;
    
    // Basic environmental (BME280)
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["pressure"] = data.pressure;
    doc["lightLevel"] = data.lightLevel; // Backward compatibility
    doc["windSpeed"] = data.windSpeed;
    doc["windDirection"] = data.windDirection;
    doc["sensorValid"] = data.sensorValid;
    
    // Advanced temperature monitoring
    doc["groundTemp"] = data.ground_temperature;
    doc["enclosureTemp"] = data.enclosure_temperature;
    doc["batteryTemp"] = data.battery_temperature;
    
    // Advanced light monitoring
    doc["visibleLight"] = data.visible_light;
    doc["infraredLight"] = data.infrared_light;
    doc["fullSpectrumLight"] = data.full_spectrum_light;
    
    // Air quality
    doc["tvocPpb"] = data.tvoc_ppb;
    doc["eco2Ppm"] = data.eco2_ppm;
    
    // Power monitoring
    doc["batteryVoltage"] = data.battery_voltage;
    doc["batteryPercentage"] = data.battery_percentage;
    doc["solarVoltage"] = data.solar_voltage;
    
    // Derived calculations
    doc["dewPoint"] = data.dew_point;
    doc["heatIndex"] = data.heat_index;
    doc["vaporPressure"] = data.vapor_pressure;
    
    // Wildlife/photography indices
    doc["wildlifeActivity"] = data.wildlife_activity_index;
    doc["photoConditions"] = data.photography_conditions;
    doc["comfortIndex"] = data.comfort_index;
    
    // Diagnostics
    doc["sensorErrors"] = data.sensor_errors;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

std::vector<uint8_t> WildlifeTelemetry::serializePowerStatus(const PowerStatus& status) {
    DynamicJsonDocument doc(512);
    doc["type"] = "power";
    doc["timestamp"] = status.timestamp;
    doc["batteryVoltage"] = status.batteryVoltage;
    doc["solarVoltage"] = status.solarVoltage;
    doc["chargingCurrent"] = status.chargingCurrent;
    doc["batteryLevel"] = status.batteryLevel;
    doc["isCharging"] = status.isCharging;
    doc["lowBattery"] = status.lowBattery;
    doc["uptimeSeconds"] = status.uptimeSeconds;
    doc["powerConsumption"] = status.powerConsumption;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

std::vector<uint8_t> WildlifeTelemetry::serializeLocationData(const LocationData& location) {
    DynamicJsonDocument doc(512);
    doc["type"] = "location";
    doc["timestamp"] = location.timestamp;
    doc["latitude"] = location.latitude;
    doc["longitude"] = location.longitude;
    doc["altitude"] = location.altitude;
    doc["accuracy"] = location.accuracy;
    doc["satellites"] = location.satellites;
    doc["fixValid"] = location.fixValid;
    doc["speed"] = location.speed;
    doc["heading"] = location.heading;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

std::vector<uint8_t> WildlifeTelemetry::serializeDeviceHealth(const DeviceHealth& health) {
    DynamicJsonDocument doc(512);
    doc["type"] = "health";
    doc["timestamp"] = health.timestamp;
    doc["cpuTemperature"] = health.cpuTemperature;
    doc["freeHeap"] = health.freeHeap;
    doc["minFreeHeap"] = health.minFreeHeap;
    doc["wifiSignal"] = health.wifiSignal;
    doc["loraSignal"] = health.loraSignal;
    doc["resetReason"] = health.resetReason;
    doc["errorCount"] = health.errorCount;
    doc["sdCardStatus"] = health.sdCardStatus;
    doc["cameraStatus"] = health.cameraStatus;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

std::vector<uint8_t> WildlifeTelemetry::serializeCaptureEvent(const CaptureEvent& event) {
    DynamicJsonDocument doc(512);
    doc["type"] = "capture";
    doc["timestamp"] = event.timestamp;
    doc["filename"] = event.filename;
    doc["fileSize"] = event.fileSize;
    doc["imageWidth"] = event.imageWidth;
    doc["imageHeight"] = event.imageHeight;
    doc["jpegQuality"] = event.jpegQuality;
    doc["captureTime"] = event.captureTime;
    doc["motionTriggered"] = event.motionTriggered;
    doc["scheduled"] = event.scheduled;
    doc["transmitted"] = event.transmitted;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

std::vector<uint8_t> WildlifeTelemetry::serializeWildlifeDetection(const WildlifeDetection& detection) {
    DynamicJsonDocument doc(512);
    doc["type"] = "wildlife";
    doc["timestamp"] = detection.timestamp;
    doc["species"] = detection.species;
    doc["confidence"] = detection.confidence;
    doc["boundingBoxX"] = detection.boundingBoxX;
    doc["boundingBoxY"] = detection.boundingBoxY;
    doc["boundingBoxW"] = detection.boundingBoxW;
    doc["boundingBoxH"] = detection.boundingBoxH;
    doc["animalCount"] = detection.animalCount;
    doc["behavior"] = detection.behavior;
    doc["endangered"] = detection.endangered;
    doc["photoFilename"] = detection.photoFilename;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    return std::vector<uint8_t>(jsonString.begin(), jsonString.end());
}

// ===========================
// STORAGE MANAGEMENT
// ===========================

void WildlifeTelemetry::limitStorageSize() {
    if (motionEvents_.size() > config_.maxStoredRecords) {
        motionEvents_.erase(motionEvents_.begin());
    }
    if (environmentalData_.size() > config_.maxStoredRecords) {
        environmentalData_.erase(environmentalData_.begin());
    }
    if (powerHistory_.size() > config_.maxStoredRecords) {
        powerHistory_.erase(powerHistory_.begin());
    }
    if (locationHistory_.size() > config_.maxStoredRecords) {
        locationHistory_.erase(locationHistory_.begin());
    }
    if (healthHistory_.size() > config_.maxStoredRecords) {
        healthHistory_.erase(healthHistory_.begin());
    }
    if (captureHistory_.size() > config_.maxStoredRecords) {
        captureHistory_.erase(captureHistory_.begin());
    }
    if (wildlifeDetections_.size() > config_.maxStoredRecords) {
        wildlifeDetections_.erase(wildlifeDetections_.begin());
    }
}

bool WildlifeTelemetry::optimizeStorage() {
    // Remove old data older than 7 days
    unsigned long sevenDaysAgo = getCurrentTimestamp() - (7 * 24 * 60 * 60 * 1000);
    return clearOldData(sevenDaysAgo);
}

bool WildlifeTelemetry::clearOldData(unsigned long olderThan) {
    // Remove old motion events
    motionEvents_.erase(
        std::remove_if(motionEvents_.begin(), motionEvents_.end(),
                      [olderThan](const MotionEvent& event) {
                          return event.timestamp < olderThan;
                      }),
        motionEvents_.end());
    
    // Remove old environmental data
    environmentalData_.erase(
        std::remove_if(environmentalData_.begin(), environmentalData_.end(),
                      [olderThan](const EnvironmentalData& data) {
                          return data.timestamp < olderThan;
                      }),
        environmentalData_.end());
    
    // Similar cleanup for other data types...
    
    return true;
}

// ===========================
// ALERT DETECTION
// ===========================

bool WildlifeTelemetry::checkForAlerts() {
    bool alertSent = false;
    
    alertSent |= checkBatteryAlert();
    alertSent |= checkTemperatureAlert();
    alertSent |= checkSystemHealthAlert();
    
    return alertSent;
}

bool WildlifeTelemetry::checkBatteryAlert() {
    if (powerHistory_.empty()) {
        return false;
    }
    
    const PowerStatus& latest = powerHistory_.back();
    if (latest.lowBattery && alertCallback_) {
        alertCallback_("LOW_BATTERY", 
                      "Battery level: " + String(latest.batteryLevel) + "%");
        return true;
    }
    
    return false;
}

bool WildlifeTelemetry::checkTemperatureAlert() {
    if (healthHistory_.empty()) {
        return false;
    }
    
    const DeviceHealth& latest = healthHistory_.back();
    if (latest.cpuTemperature > 85.0 && alertCallback_) {
        alertCallback_("HIGH_TEMPERATURE", 
                      "CPU temperature: " + String(latest.cpuTemperature) + "°C");
        return true;
    }
    
    return false;
}

bool WildlifeTelemetry::checkSystemHealthAlert() {
    if (healthHistory_.empty()) {
        return false;
    }
    
    const DeviceHealth& latest = healthHistory_.back();
    if (latest.freeHeap < 5000 && alertCallback_) {
        alertCallback_("LOW_MEMORY", 
                      "Free heap: " + String(latest.freeHeap) + " bytes");
        return true;
    }
    
    return false;
}

// ===========================
// STATISTICS
// ===========================

uint32_t WildlifeTelemetry::getTotalMotionEvents() const {
    return motionEvents_.size();
}

uint32_t WildlifeTelemetry::getTotalCaptureEvents() const {
    return captureHistory_.size();
}

uint32_t WildlifeTelemetry::getTotalWildlifeDetections() const {
    return wildlifeDetections_.size();
}

float WildlifeTelemetry::getAverageBatteryLevel() const {
    if (powerHistory_.empty()) {
        return 0.0;
    }
    
    float sum = 0.0;
    for (const auto& status : powerHistory_) {
        sum += status.batteryLevel;
    }
    
    return sum / powerHistory_.size();
}

float WildlifeTelemetry::getAverageTemperature() const {
    if (environmentalData_.empty()) {
        return 0.0;
    }
    
    float sum = 0.0;
    for (const auto& data : environmentalData_) {
        sum += data.temperature;
    }
    
    return sum / environmentalData_.size();
}

String WildlifeTelemetry::getLastSpeciesDetected() const {
    if (wildlifeDetections_.empty()) {
        return "None";
    }
    
    return wildlifeDetections_.back().species;
}

// ===========================
// CONFIGURATION MANAGEMENT
// ===========================

TelemetryConfig WildlifeTelemetry::getConfig() const {
    return config_;
}

bool WildlifeTelemetry::setConfig(const TelemetryConfig& config) {
    return configure(config);
}

bool WildlifeTelemetry::saveConfigToFile(const String& filename) {
    File file = LittleFS.open(filename, "w");
    if (!file) {
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    doc["motionEnabled"] = config_.motionEnabled;
    doc["environmentalEnabled"] = config_.environmentalEnabled;
    doc["powerEnabled"] = config_.powerEnabled;
    doc["locationEnabled"] = config_.locationEnabled;
    doc["healthEnabled"] = config_.healthEnabled;
    doc["captureEnabled"] = config_.captureEnabled;
    doc["wildlifeEnabled"] = config_.wildlifeEnabled;
    doc["motionInterval"] = config_.motionInterval;
    doc["environmentalInterval"] = config_.environmentalInterval;
    doc["powerInterval"] = config_.powerInterval;
    doc["locationInterval"] = config_.locationInterval;
    doc["healthInterval"] = config_.healthInterval;
    doc["transmitOverMesh"] = config_.transmitOverMesh;
    doc["storeLocally"] = config_.storeLocally;
    doc["maxStoredRecords"] = config_.maxStoredRecords;
    doc["compressionLevel"] = config_.compressionLevel;
    doc["encryptData"] = config_.encryptData;
    
    serializeJson(doc, file);
    file.close();
    
    return true;
}

bool WildlifeTelemetry::loadConfigFromFile(const String& filename) {
    File file = LittleFS.open(filename, "r");
    if (!file) {
        return false;
    }
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false;
    }
    
    config_.motionEnabled = doc["motionEnabled"];
    config_.environmentalEnabled = doc["environmentalEnabled"];
    config_.powerEnabled = doc["powerEnabled"];
    config_.locationEnabled = doc["locationEnabled"];
    config_.healthEnabled = doc["healthEnabled"];
    config_.captureEnabled = doc["captureEnabled"];
    config_.wildlifeEnabled = doc["wildlifeEnabled"];
    config_.motionInterval = doc["motionInterval"];
    config_.environmentalInterval = doc["environmentalInterval"];
    config_.powerInterval = doc["powerInterval"];
    config_.locationInterval = doc["locationInterval"];
    config_.healthInterval = doc["healthInterval"];
    config_.transmitOverMesh = doc["transmitOverMesh"];
    config_.storeLocally = doc["storeLocally"];
    config_.maxStoredRecords = doc["maxStoredRecords"];
    config_.compressionLevel = doc["compressionLevel"];
    config_.encryptData = doc["encryptData"];
    
    return true;
}

// ===========================
// CALLBACK MANAGEMENT
// ===========================

void WildlifeTelemetry::setMotionEventCallback(MotionEventCallback callback) {
    motionEventCallback_ = callback;
}

void WildlifeTelemetry::setWildlifeDetectionCallback(WildlifeDetectionCallback callback) {
    wildlifeDetectionCallback_ = callback;
}

void WildlifeTelemetry::setAlertCallback(AlertCallback callback) {
    alertCallback_ = callback;
}

void WildlifeTelemetry::setDataTransmittedCallback(DataTransmittedCallback callback) {
    dataTransmittedCallback_ = callback;
}

// ===========================
// UTILITY METHODS
// ===========================

unsigned long WildlifeTelemetry::getCurrentTimestamp() const {
    return millis();
}

// ===========================
// UTILITY FUNCTIONS
// ===========================

WildlifeTelemetry* createWildlifeTelemetry(MeshInterface* meshInterface) {
    WildlifeTelemetry* telemetry = new WildlifeTelemetry();
    if (telemetry->init(meshInterface)) {
        return telemetry;
    } else {
        delete telemetry;
        return nullptr;
    }
}

TelemetryConfig createDefaultTelemetryConfig() {
    TelemetryConfig config;
    config.motionEnabled = true;
    config.environmentalEnabled = true;
    config.powerEnabled = true;
    config.locationEnabled = GPS_MESH_ENABLED;
    config.healthEnabled = true;
    config.captureEnabled = true;
    config.wildlifeEnabled = true;
    
    config.motionInterval = WILDLIFE_TELEMETRY_INTERVAL;
    config.environmentalInterval = ENV_SENSOR_INTERVAL;
    config.powerInterval = BATTERY_STATUS_INTERVAL;
    config.locationInterval = GPS_UPDATE_INTERVAL;
    config.healthInterval = MESH_DIAGNOSTICS_INTERVAL;
    
    config.transmitOverMesh = true;
    config.storeLocally = true;
    config.maxStoredRecords = 100;
    
    config.compressionLevel = 0;
    config.encryptData = MESH_ENCRYPTION_ENABLED;
    
    return config;
}

TelemetryConfig createLowPowerTelemetryConfig() {
    TelemetryConfig config = createDefaultTelemetryConfig();
    
    // Reduce transmission frequency for power saving
    config.motionInterval = 600000;         // 10 minutes
    config.environmentalInterval = 1800000; // 30 minutes
    config.powerInterval = 900000;          // 15 minutes
    config.healthInterval = 3600000;        // 60 minutes
    
    config.maxStoredRecords = 50;           // Reduce storage
    config.compressionLevel = 3;            // Enable compression
    
    return config;
}

TelemetryConfig createResearchTelemetryConfig() {
    TelemetryConfig config = createDefaultTelemetryConfig();
    
    // Increase data collection for research
    config.motionInterval = 60000;          // 1 minute
    config.environmentalInterval = 30000;   // 30 seconds
    config.powerInterval = 120000;          // 2 minutes
    config.healthInterval = 300000;         // 5 minutes
    
    config.maxStoredRecords = 500;          // Increase storage
    config.compressionLevel = 0;            // No compression for accuracy
    
    return config;
}

// ===========================
// VALIDATION FUNCTIONS
// ===========================

bool isValidMotionEvent(const MotionEvent& event) {
    return event.timestamp > 0 && 
           event.confidence <= 100 &&
           event.duration > 0;
}

bool isValidEnvironmentalData(const EnvironmentalData& data) {
    return data.timestamp > 0 &&
           // Basic environmental validation
           data.temperature >= -50.0 && data.temperature <= 85.0 &&
           data.humidity >= 0.0 && data.humidity <= 100.0 &&
           data.pressure >= 800.0 && data.pressure <= 1200.0 &&
           data.lightLevel <= 1023 &&
           // Advanced temperature validation
           data.ground_temperature >= -50.0 && data.ground_temperature <= 85.0 &&
           data.enclosure_temperature >= -50.0 && data.enclosure_temperature <= 85.0 &&
           data.battery_temperature >= -50.0 && data.battery_temperature <= 85.0 &&
           // Light validation (reasonable lux values)
           data.visible_light >= 0.0 && data.visible_light <= 100000.0 &&
           data.infrared_light >= 0.0 && data.infrared_light <= 100000.0 &&
           data.full_spectrum_light >= 0.0 && data.full_spectrum_light <= 100000.0 &&
           // Air quality validation
           data.tvoc_ppb <= 60000 && // SGP30 max range
           data.eco2_ppm >= 400 && data.eco2_ppm <= 60000 && // SGP30 range
           // Battery validation
           data.battery_voltage >= 0.0 && data.battery_voltage <= 5.0 &&
           data.battery_percentage <= 100 &&
           data.solar_voltage >= 0.0 && data.solar_voltage <= 25.0 &&
           // Index validation (0-100%)
           data.wildlife_activity_index <= 100 &&
           data.photography_conditions <= 100 &&
           data.comfort_index <= 100;
}

bool isValidPowerStatus(const PowerStatus& status) {
    return status.timestamp > 0 &&
           status.batteryVoltage >= 0.0 && status.batteryVoltage <= 5.0 &&
           status.batteryLevel <= 100;
}

bool isValidLocationData(const LocationData& location) {
    return location.timestamp > 0 &&
           location.latitude >= -90.0 && location.latitude <= 90.0 &&
           location.longitude >= -180.0 && location.longitude <= 180.0;
}

bool isValidDeviceHealth(const DeviceHealth& health) {
    return health.timestamp > 0 &&
           health.cpuTemperature >= -50.0 && health.cpuTemperature <= 150.0;
}

bool isValidCaptureEvent(const CaptureEvent& event) {
    return event.timestamp > 0 &&
           !event.filename.isEmpty() &&
           event.fileSize > 0;
}

bool isValidWildlifeDetection(const WildlifeDetection& detection) {
    return detection.timestamp > 0 &&
           !detection.species.isEmpty() &&
           detection.confidence >= 0.0 && detection.confidence <= 1.0;
}