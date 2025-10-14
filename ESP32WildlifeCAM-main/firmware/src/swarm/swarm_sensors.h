/**
 * @file swarm_sensors.h
 * @brief Sensor integration and data fusion for swarm members
 * 
 * Provides:
 * - Multi-spectral imaging
 * - Thermal imaging
 * - Acoustic sensors
 * - Environmental sensors
 * - LiDAR integration
 * - Chemical sensors
 * - Sensor data fusion
 */

#ifndef SWARM_SENSORS_H
#define SWARM_SENSORS_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "swarm_coordinator.h"

// Sensor types
enum SensorType {
    SENSOR_CAMERA_RGB = 0,
    SENSOR_CAMERA_THERMAL = 1,
    SENSOR_CAMERA_MULTISPECTRAL = 2,
    SENSOR_MICROPHONE = 3,
    SENSOR_LIDAR = 4,
    SENSOR_RADAR = 5,
    SENSOR_TEMPERATURE = 6,
    SENSOR_HUMIDITY = 7,
    SENSOR_PRESSURE = 8,
    SENSOR_AIR_QUALITY = 9,
    SENSOR_MAGNETOMETER = 10,
    SENSOR_CHEMICAL = 11,
    SENSOR_ULTRASONIC = 12
};

// Detection type
enum DetectionType {
    DETECTION_WILDLIFE = 0,
    DETECTION_HUMAN = 1,
    DETECTION_VEHICLE = 2,
    DETECTION_FIRE = 3,
    DETECTION_WATER = 4,
    DETECTION_UNKNOWN = 5
};

// Environmental threat level
enum ThreatLevel {
    THREAT_NONE = 0,
    THREAT_LOW = 1,
    THREAT_MEDIUM = 2,
    THREAT_HIGH = 3,
    THREAT_CRITICAL = 4
};

// Image spectrum
struct ImageData {
    int memberId;
    SensorType sensorType;
    Position3D location;
    unsigned long timestamp;
    int width;
    int height;
    std::vector<uint8_t> data;
    float confidence;
    String metadata;
};

// Thermal detection
struct ThermalDetection {
    int detectionId;
    int memberId;
    Position3D location;
    float temperature;      // Celsius
    float temperatureMin;
    float temperatureMax;
    DetectionType type;
    float confidence;
    unsigned long timestamp;
};

// Acoustic detection
struct AcousticDetection {
    int detectionId;
    int memberId;
    Position3D location;
    float frequency;        // Hz
    float amplitude;        // dB
    float duration;         // seconds
    String species;         // If identified
    float confidence;
    unsigned long timestamp;
};

// LiDAR point cloud
struct LiDARData {
    int memberId;
    Position3D sensorPosition;
    std::vector<Position3D> points;
    float resolution;       // meters
    float range;           // meters
    unsigned long timestamp;
};

// Environmental reading
struct EnvironmentalData {
    int memberId;
    Position3D location;
    float temperature;      // Celsius
    float humidity;         // Percentage
    float pressure;         // hPa
    float airQuality;       // AQI
    float co2Level;         // ppm
    float vocLevel;         // ppb
    unsigned long timestamp;
};

// Chemical detection
struct ChemicalDetection {
    int detectionId;
    int memberId;
    Position3D location;
    String compound;
    float concentration;    // ppm
    ThreatLevel threat;
    unsigned long timestamp;
};

// Fused detection result
struct FusedDetection {
    int detectionId;
    DetectionType type;
    Position3D location;
    float confidence;
    std::vector<int> contributingMembers;
    std::vector<SensorType> sensors;
    String species;
    float size;            // meters
    float temperature;     // Celsius (if available)
    float speed;           // m/s
    unsigned long timestamp;
};

// Sensor health
struct SensorHealth {
    SensorType type;
    bool isOperational;
    float accuracy;
    float lastCalibration;
    int errorCount;
    String status;
};

// Sensor statistics
struct SensorStats {
    int totalDetections;
    int confirmedDetections;
    int falsePositives;
    float averageConfidence;
    float coverageArea;
    std::map<DetectionType, int> detectionsByType;
    unsigned long operationTime;
};

class SwarmSensors {
public:
    SwarmSensors();
    ~SwarmSensors();
    
    // ===========================
    // INITIALIZATION
    // ===========================
    bool init();
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // ===========================
    // SENSOR REGISTRATION
    // ===========================
    bool registerSensor(int memberId, SensorType type);
    bool unregisterSensor(int memberId, SensorType type);
    std::vector<SensorType> getAvailableSensors(int memberId) const;
    bool hasSensor(int memberId, SensorType type) const;
    
    // ===========================
    // IMAGING SENSORS
    // ===========================
    bool captureImage(int memberId, SensorType type);
    ImageData getLatestImage(int memberId, SensorType type) const;
    bool processMultispectral(int memberId);
    std::vector<Position3D> detectObjectsInImage(const ImageData& image);
    
    // ===========================
    // THERMAL IMAGING
    // ===========================
    bool enableThermalImaging(int memberId, bool enable);
    ThermalDetection detectThermalSignature(int memberId);
    std::vector<ThermalDetection> getAllThermalDetections() const;
    bool isHeatSignatureAnimal(const ThermalDetection& detection) const;
    float estimateAnimalSize(const ThermalDetection& detection) const;
    
    // ===========================
    // ACOUSTIC SENSORS
    // ===========================
    bool enableAcousticMonitoring(int memberId, bool enable);
    AcousticDetection detectSound(int memberId);
    std::vector<AcousticDetection> getAllAcousticDetections() const;
    String identifySpeciesBySound(const AcousticDetection& detection);
    Position3D triangulateSound(const std::vector<AcousticDetection>& detections);
    
    // ===========================
    // LIDAR
    // ===========================
    bool enableLiDAR(int memberId, bool enable);
    LiDARData getLiDARScan(int memberId);
    std::vector<Obstacle> extractObstacles(const LiDARData& data);
    float calculateTerrainElevation(const LiDARData& data, float x, float y);
    bool create3DMap(const std::vector<LiDARData>& scans);
    
    // ===========================
    // ENVIRONMENTAL SENSORS
    // ===========================
    bool updateEnvironmentalData(int memberId, const EnvironmentalData& data);
    EnvironmentalData getEnvironmentalData(int memberId) const;
    float getAverageTemperature(const Position3D& area, float radius) const;
    bool detectWeatherChange() const;
    ThreatLevel assessEnvironmentalThreat() const;
    
    // ===========================
    // CHEMICAL SENSORS
    // ===========================
    bool enableChemicalSensors(int memberId, bool enable);
    ChemicalDetection detectChemical(int memberId);
    std::vector<ChemicalDetection> getAllChemicalDetections() const;
    bool detectPollution(const Position3D& area, float radius) const;
    ThreatLevel assessChemicalThreat() const;
    
    // ===========================
    // DATA FUSION
    // ===========================
    FusedDetection fuseDetections(const std::vector<int>& memberIds,
                                  const Position3D& targetArea);
    std::vector<FusedDetection> getAllFusedDetections() const;
    bool correlateDetections(int detectionA, int detectionB) const;
    float calculateDetectionConfidence(const FusedDetection& detection) const;
    
    // ===========================
    // MULTI-MODAL SENSING
    // ===========================
    bool enableMultiModalSensing(bool enable);
    FusedDetection detectWildlife(const Position3D& area, float radius);
    bool verifyDetection(int detectionId);
    std::vector<FusedDetection> trackMovingObject(int trackId);
    
    // ===========================
    // WILDLIFE DETECTION
    // ===========================
    bool startWildlifeMonitoring(const Position3D& area, float radius);
    std::vector<FusedDetection> detectWildlifeInArea(const Position3D& area, 
                                                     float radius) const;
    String identifySpecies(const FusedDetection& detection) const;
    bool estimateBehavior(const FusedDetection& detection, String& behavior) const;
    int countAnimalsInArea(const Position3D& area, float radius) const;
    
    // ===========================
    // THREAT DETECTION
    // ===========================
    bool detectHumanIntrusion(const Position3D& area, float radius);
    bool detectVehicle(const Position3D& area, float radius);
    bool detectFire(const Position3D& area, float radius);
    ThreatLevel assessOverallThreat() const;
    std::vector<Position3D> identifyThreatLocations() const;
    
    // ===========================
    // SENSOR HEALTH & CALIBRATION
    // ===========================
    bool calibrateSensor(int memberId, SensorType type);
    SensorHealth getSensorHealth(int memberId, SensorType type) const;
    bool performSelfTest(int memberId);
    bool replaceFaultySensor(int memberId, SensorType type);
    
    // ===========================
    // DATA MANAGEMENT
    // ===========================
    bool storeSensorData(int memberId, const String& data);
    String retrieveSensorData(int memberId, unsigned long startTime, 
                             unsigned long endTime) const;
    bool compressSensorData(int memberId);
    bool transmitSensorData(int memberId, int targetId);
    
    // ===========================
    // STATISTICS & MONITORING
    // ===========================
    SensorStats getStatistics() const;
    void logSensorEvent(const String& event, const String& details = "");
    
private:
    // State
    bool initialized_;
    bool multiModalEnabled_;
    
    // Sensor registration
    std::map<int, std::vector<SensorType>> memberSensors_;
    std::map<int, std::map<SensorType, SensorHealth>> sensorHealth_;
    
    // Detections
    std::vector<ThermalDetection> thermalDetections_;
    std::vector<AcousticDetection> acousticDetections_;
    std::vector<ChemicalDetection> chemicalDetections_;
    std::vector<FusedDetection> fusedDetections_;
    
    // Environmental data
    std::map<int, EnvironmentalData> environmentalData_;
    
    // LiDAR data
    std::map<int, LiDARData> lidarData_;
    
    // Statistics
    SensorStats stats_;
    int nextDetectionId_;
    
    // Private helper methods
    float calculateDistance(const Position3D& a, const Position3D& b) const;
    bool isInRange(const Position3D& position, const Position3D& center, 
                   float radius) const;
    float calculateSensorFusion(const std::vector<float>& confidences) const;
    bool validateDetection(const FusedDetection& detection) const;
    void updateStatistics();
};

// Global sensors instance
extern SwarmSensors* g_swarmSensors;

// Utility functions
bool initializeSwarmSensors();
void processSwarmSensors();
SensorStats getSensorStatistics();
void cleanupSwarmSensors();

#endif // SWARM_SENSORS_H
