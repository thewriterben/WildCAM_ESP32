# 📚 ESP32 Wildlife Camera API Reference

## Core Classes

### CameraManager
*Advanced camera management with multiple capture modes*

#### Public Methods
```cpp
class CameraManager {
public:
    // Initialization
    bool initialize();
    bool configure(const CameraConfig& config);
    
    // Image Capture
    bool captureImage(const char* filename = nullptr);
    bool captureBurst(int count, int interval_ms = 500);
    bool startTimeLapse(int interval_s, int duration_s = 0);
    bool stopTimeLapse();
    
    // Video Recording
    bool startVideoRecording(const char* filename = nullptr);
    bool stopVideoRecording();
    bool isRecording();
    
    // Status and Configuration
    CameraStatus getStatus();
    bool setResolution(framesize_t size);
    bool setQuality(int quality); // 0-63, lower = better
    bool enableNightVision(bool enable);
    
    // Advanced Features
    bool enableBurstMode(bool enable);
    bool setImageFormat(pixformat_t format);
    float getLastCaptureTime();
};
```

#### Usage Example
```cpp
CameraManager camera;
CameraConfig config = {
    .resolution = FRAMESIZE_UXGA,
    .quality = 10,
    .enablePSRAM = true
};

if (camera.initialize() && camera.configure(config)) {
    camera.captureImage("wildlife_001.jpg");
}
```

### MotionDetectionManager
*Enhanced motion detection with AI-powered filtering*

#### Public Methods
```cpp
class MotionDetectionManager {
public:
    // Initialization
    bool initialize(int pirPin = 12);
    bool configure(const MotionConfig& config);
    
    // Detection
    bool checkMotion();
    bool isMotionDetected();
    float getConfidence();
    MotionData getLastMotion();
    
    // Advanced Analysis
    bool enableAdvancedAnalysis(bool enable);
    bool setThreshold(float threshold);
    bool setFilteringEnabled(bool enable);
    
    // Callbacks
    void setMotionCallback(void (*callback)(MotionData));
    void enableLogging(bool enable);
};
```

### PowerManager
*Comprehensive power management with solar integration*

#### Public Methods
```cpp
class PowerManager {
public:
    // Initialization
    bool initialize();
    bool configurePowerSaving(PowerMode mode);
    
    // Battery Management
    int getBatteryPercentage();
    float getBatteryVoltage();
    bool isCharging();
    float getChargingCurrent();
    
    // Solar Panel
    float getSolarVoltage();
    float getSolarCurrent();
    float getSolarPower();
    
    // Power Control
    bool enableDeepSleep(uint64_t sleep_time_us);
    bool enableLightSleep(uint64_t sleep_time_us);
    PowerMode getCurrentMode();
    
    // Statistics
    PowerStats getPowerStatistics();
    float getAveragePowerConsumption();
};
```

### StorageManager
*SD card and file system management*

#### Public Methods
```cpp
class StorageManager {
public:
    // Initialization
    static bool initialize();
    static bool isInitialized();
    
    // File Operations
    static bool saveImage(const char* filename, uint8_t* data, size_t length);
    static bool saveVideo(const char* filename, uint8_t* data, size_t length);
    static bool deleteFile(const char* filename);
    static bool fileExists(const char* filename);
    
    // Directory Management
    static bool createDirectory(const char* dirname);
    static std::vector<String> listFiles(const char* path = "/");
    
    // Storage Statistics
    static StorageStats getStatistics();
    static bool hasAdequateSpace(size_t required_bytes);
    static bool performCleanup();
};
```

### WildlifeClassifier
*AI-powered wildlife species classification*

#### Public Methods
```cpp
class WildlifeClassifier {
public:
    // Initialization
    bool initialize();
    bool loadModel(const char* model_path);
    
    // Classification
    ClassificationResult classifyImage(uint8_t* image_data, size_t length);
    std::vector<Species> detectSpecies(uint8_t* image_data);
    float getConfidence();
    
    // Model Management
    bool updateModel(const char* new_model_path);
    ModelInfo getModelInfo();
    
    // Federated Learning
    bool enableFederatedLearning(bool enable);
    bool contributeToFederation(const TrainingData& data);
};
```

## Data Structures

```cpp
struct MotionData {
    unsigned long timestamp;
    bool detected;
    float confidence;
    int sensorValue;
    float temperature;
    float humidity;
    int lightLevel;
    MotionType type;               // PIR, ADVANCED, AI_FILTERED
};
```

### StorageStats
```cpp
struct StorageStats {
    uint64_t totalSpace;           // Total storage in bytes
    uint64_t freeSpace;            // Available space in bytes
    uint64_t usedSpace;            // Used space in bytes
    float usagePercentage;         // Usage as percentage
    int fileCount;                 // Number of files
    unsigned long lastUpdate;      // Last update timestamp
};
```

### ClassificationResult
```cpp
struct ClassificationResult {
    String species;                // Detected species name
    float confidence;              // Classification confidence (0-1)
    BoundingBox bbox;              // Object location
    std::vector<String> attributes; // Additional attributes
    unsigned long timestamp;       // Detection timestamp
};
```

## Web API Endpoints

### Device Status
```http
GET /api/status
```
Response:
```json
{
    "device_id": "ESP32CAM_001",
    "uptime": 3600,
    "battery": 85,
    "storage": {
        "total": 32000000000,
        "free": 25000000000,
        "usage": 21.9
    },
    "last_capture": "2025-01-01T12:00:00Z"
}
```

### Capture Image
```http
POST /api/capture
Content-Type: application/json

{
    "filename": "wildlife_001.jpg",
    "quality": 10,
    "enable_flash": false
}
```

### Configuration Management
```http
GET /api/config
PUT /api/config
```

### File Management
```http
GET /api/files
GET /api/files/{filename}
DELETE /api/files/{filename}
```

### Real-time Streaming
```http
GET /stream
```
Returns MJPEG stream for live monitoring.

## Configuration APIs

### WiFi Configuration
```cpp
struct WiFiConfig {
    String ssid;
    String password;
    String hostname = "ESP32WildlifeCam";
    bool enableAP = false;
    String apPassword = "wildcam123";
    int timeout_ms = 30000;
};

bool configureWiFi(const WiFiConfig& config);
```

### Environmental Configuration
```cpp
enum EnvironmentType {
    TROPICAL,
    DESERT,
    ARCTIC,
    TEMPERATE_FOREST
};

struct EnvironmentalConfig {
    EnvironmentType type;
    float temperatureRange[2];     // Min, Max
    float humidityRange[2];        // Min, Max
    int operatingVoltage;          // mV
    bool weatherproofing;
};
```

## Event System

### Event Types
```cpp
enum EventType {
    MOTION_DETECTED,
    CAPTURE_COMPLETED,
    STORAGE_LOW,
    BATTERY_LOW,
    SYSTEM_ERROR,
    NETWORK_CONNECTED,
    CLASSIFICATION_COMPLETED
};
```

### Event Handler
```cpp
class EventManager {
public:
    void registerHandler(EventType type, EventCallback callback);
    void fireEvent(EventType type, const EventData& data);
    void removeHandler(EventType type);
};
```

### Usage Example
```cpp
EventManager events;
events.registerHandler(MOTION_DETECTED, [](const EventData& data) {
    Serial.println("Motion detected at: " + String(data.timestamp));
    camera.captureImage();
});
```

## Error Handling

### Error Codes
```cpp
enum ErrorCode {
    SUCCESS = 0,
    CAMERA_INIT_FAILED = 1001,
    SD_CARD_ERROR = 2001,
    MOTION_SENSOR_ERROR = 3001,
    POWER_SYSTEM_ERROR = 4001,
    NETWORK_ERROR = 5001,
    CLASSIFICATION_ERROR = 6001
};
```

### Error Information
```cpp
struct ErrorInfo {
    ErrorCode code;
    String message;
    unsigned long timestamp;
    String component;
    bool recoverable;
};
```

## Usage Examples

### Complete Wildlife Monitoring Setup
```cpp
#include "camera/camera_manager.h"
#include "detection/motion_detection_manager.h"
#include "power/power_manager.h"
#include "storage/storage_manager.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize storage
    if (!StorageManager::initialize()) {
        Serial.println("Storage initialization failed!");
        return;
    }
    
    // Configure camera
    CameraManager camera;
    CameraConfig camConfig = {
        .resolution = FRAMESIZE_UXGA,
        .quality = 10,
        .enableBurst = true
    };
    
    camera.initialize();
    camera.configure(camConfig);
    
    // Setup motion detection
    MotionDetectionManager motion;
    motion.initialize(12); // PIR on GPIO 12
    motion.setMotionCallback([](MotionData data) {
        if (data.confidence > 0.8) {
            camera.captureBurst(3, 1000); // 3 images, 1s apart
        }
    });
    
    // Configure power management
    PowerManager power;
    power.initialize();
    power.configurePowerSaving(POWER_MODE_EFFICIENT);
    
    Serial.println("Wildlife monitoring system ready!");
}

void loop() {
    // System automatically handles motion detection and captures
    delay(1000);
    
    // Optional: Print status every 10 seconds
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 10000) {
        auto stats = StorageManager::getStatistics();
        Serial.printf("Storage: %.1f%% used, Battery: %d%%\n", 
                     stats.usagePercentage, 
                     PowerManager::getBatteryPercentage());
        lastStatus = millis();
    }
}
```

### Federated Learning Integration
```cpp
#include "ai/wildlife_classifier.h"
#include "ai/federated_learning_system.h"

WildlifeClassifier classifier;
FederatedLearningSystem federation;

void processWildlifeImage(uint8_t* imageData, size_t length) {
    // Classify the image
    auto result = classifier.classifyImage(imageData, length);
    
    // Log detection
    Serial.printf("Detected: %s (confidence: %.2f)\n", 
                 result.species.c_str(), result.confidence);
    
    // Contribute to federated learning if confidence is high
    if (result.confidence > 0.9) {
        TrainingData trainingData = {
            .imageData = imageData,
            .length = length,
            .label = result.species,
            .confidence = result.confidence
        };
        
        federation.contributeTrainingData(trainingData);
    }
}
```

---

## Related Documentation
- [Setup Guide](setup_guide.md) - Complete installation and configuration
- [Hardware Integration](hardware_selection_guide.md) - Hardware setup and wiring
- [Analytics Dashboard](ANALYTICS_DASHBOARD.md) - Web interface and data visualization
- [Troubleshooting](troubleshooting.md) - Common issues and solutions
    int imageHeight;
    float compressionRatio;
};
```

### Configuration Options
*Configuration parameters and their descriptions.*

## Examples

### Basic Usage
```cpp
#include "camera_handler.h"

void setup() {
    CameraHandler::init();
}

void loop() {
    if (motionDetected) {
        CameraHandler::captureImage();
    }
}
```

## Error Codes

*Error codes and their meanings will be documented here.*

---

*This API reference is currently a placeholder and will be expanded with complete documentation.*