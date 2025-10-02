# WildCAM ESP32 - Advanced Features API Reference

## Table of Contents

1. [WildlifeClassifier](#wildlifeclassifier)
2. [DangerousSpeciesAlertSystem](#dangerousspeciesalertsystem)
3. [MotionPatternAnalyzer](#motionpatternanalyzer)
4. [LoRaWildlifeAlerts](#lorawildlifealerts)
5. [Data Structures](#data-structures)
6. [Enumerations](#enumerations)

---

## WildlifeClassifier

AI-powered wildlife species classification using TensorFlow Lite.

### Initialization

```cpp
bool initialize()
```
Initialize the wildlife classifier system.

**Returns:** `true` if initialization successful

**Example:**
```cpp
WildlifeClassifier classifier;
if (classifier.initialize()) {
    Serial.println("Classifier ready");
}
```

---

### Classification Methods

#### classifyFrame()

```cpp
ClassificationResult classifyFrame(camera_fb_t* frame)
```
Classify wildlife from camera frame buffer.

**Parameters:**
- `frame` - Camera frame buffer from `esp_camera_fb_get()`

**Returns:** `ClassificationResult` structure

**Example:**
```cpp
camera_fb_t* frame = esp_camera_fb_get();
ClassificationResult result = classifier.classifyFrame(frame);
if (result.isValid) {
    Serial.printf("Species: %s (%.1f%%)\n", 
                 result.speciesName.c_str(), 
                 result.confidence * 100);
}
esp_camera_fb_return(frame);
```

---

#### classifyImage()

```cpp
ClassificationResult classifyImage(const uint8_t* imageData, 
                                   size_t imageSize,
                                   uint16_t width, 
                                   uint16_t height)
```
Classify wildlife from raw image data.

**Parameters:**
- `imageData` - Pointer to image data
- `imageSize` - Size of image data in bytes
- `width` - Image width in pixels
- `height` - Image height in pixels

**Returns:** `ClassificationResult` structure

---

### Configuration Methods

#### setConfidenceThreshold()

```cpp
void setConfidenceThreshold(float threshold)
```
Set minimum confidence threshold for valid detections.

**Parameters:**
- `threshold` - Confidence threshold (0.0 to 1.0)

**Example:**
```cpp
classifier.setConfidenceThreshold(0.75f); // 75% minimum
```

---

#### setEnabled()

```cpp
void setEnabled(bool enable)
```
Enable or disable the classifier.

**Parameters:**
- `enable` - Enable state (true/false)

---

#### resetStatistics()

```cpp
void resetStatistics()
```
Reset classifier statistics counters.

---

### Query Methods

#### isEnabled()

```cpp
bool isEnabled() const
```
Check if classifier is enabled.

**Returns:** `true` if enabled

---

#### getStatistics()

```cpp
ClassifierStats getStatistics() const
```
Get classifier statistics.

**Returns:** `ClassifierStats` structure

**Example:**
```cpp
ClassifierStats stats = classifier.getStatistics();
Serial.printf("Total inferences: %d\n", stats.totalInferences);
Serial.printf("Average confidence: %.1f%%\n", stats.averageConfidence * 100);
```

---

### Static Utility Methods

#### getSpeciesName()

```cpp
static String getSpeciesName(SpeciesType species)
```
Get human-readable species name.

**Parameters:**
- `species` - Species type enumeration

**Returns:** Species name string

---

#### isDangerousSpecies()

```cpp
static bool isDangerousSpecies(SpeciesType species)
```
Check if species is dangerous/predator.

**Parameters:**
- `species` - Species type enumeration

**Returns:** `true` if dangerous

**Example:**
```cpp
if (WildlifeClassifier::isDangerousSpecies(result.species)) {
    Serial.println("⚠️ Dangerous species!");
}
```

---

#### getConfidenceLevelDescription()

```cpp
static String getConfidenceLevelDescription(ConfidenceLevel level)
```
Get description of confidence level.

**Parameters:**
- `level` - Confidence level enumeration

**Returns:** Description string (e.g., "High (75-90%)")

---

## DangerousSpeciesAlertSystem

Real-time alert system for dangerous wildlife detection.

### Initialization

```cpp
bool initialize()
```
Initialize the alert system.

**Returns:** `true` if initialization successful

---

### Alert Processing

#### processClassification()

```cpp
DangerousSpeciesAlert* processClassification(
    const ClassificationResult& result)
```
Process classification result and generate alerts if criteria met.

**Parameters:**
- `result` - Classification result from WildlifeClassifier

**Returns:** Pointer to alert if generated, `nullptr` otherwise

**Example:**
```cpp
ClassificationResult result = classifier.classifyFrame(frame);
DangerousSpeciesAlert* alert = alertSystem.processClassification(result);

if (alert != nullptr) {
    Serial.println(alert->alertMessage);
    if (alert->requiresImmediate) {
        sendEmergencyNotification();
    }
}
```

---

### Configuration Methods

#### setConfidenceThreshold()

```cpp
void setConfidenceThreshold(float threshold)
```
Set minimum confidence threshold for alerts.

**Parameters:**
- `threshold` - Confidence threshold (0.0 to 1.0)

---

#### setMinConsecutiveDetections()

```cpp
void setMinConsecutiveDetections(uint32_t count)
```
Set minimum consecutive detections required for alert.

**Parameters:**
- `count` - Number of consecutive detections (minimum 1)

**Example:**
```cpp
alertSystem.setMinConsecutiveDetections(3); // Require 3 detections
```

---

#### setEnabled()

```cpp
void setEnabled(bool enable)
```
Enable or disable the alert system.

---

#### clearAlertHistory()

```cpp
void clearAlertHistory()
```
Clear alert history and reset counters.

---

### Query Methods

#### getTotalAlerts()

```cpp
uint32_t getTotalAlerts() const
```
Get total number of alerts generated.

---

#### getCriticalAlerts()

```cpp
uint32_t getCriticalAlerts() const
```
Get number of critical priority alerts.

---

#### isEnabled()

```cpp
bool isEnabled() const
```
Check if alert system is enabled.

---

### Static Utility Methods

#### getAlertPriority()

```cpp
static AlertPriority getAlertPriority(SpeciesType species)
```
Get alert priority for species.

**Returns:** `AlertPriority` enumeration

---

#### requiresImmediateAlert()

```cpp
static bool requiresImmediateAlert(SpeciesType species, float confidence)
```
Check if species/confidence requires immediate alert.

**Returns:** `true` if immediate notification needed

---

#### getAlertMessage()

```cpp
static String getAlertMessage(SpeciesType species, float confidence)
```
Generate alert message for species.

**Returns:** Formatted alert message string

---

## MotionPatternAnalyzer

Temporal motion analysis for behavior identification.

### Initialization

```cpp
bool initialize()
```
Initialize motion pattern analyzer.

**Returns:** `true` if initialization successful

---

### Analysis Methods

#### analyzePattern()

```cpp
MotionPatternResult analyzePattern(const MotionResult& motionResult)
```
Analyze motion pattern from detection result.

**Parameters:**
- `motionResult` - Motion detection result

**Returns:** `MotionPatternResult` structure

**Example:**
```cpp
MotionResult motion = motionDetector.detectMotion();
MotionPatternResult pattern = patternAnalyzer.analyzePattern(motion);

if (pattern.isAnimalMotion) {
    Serial.printf("Pattern: %s\n", pattern.description.c_str());
    classifyWildlife(); // Proceed with AI
} else {
    Serial.println("False positive - Skipping");
}
```

---

#### addMotionData()

```cpp
void addMotionData(uint32_t timestamp, 
                   float motionLevel,
                   int16_t centerX, 
                   int16_t centerY,
                   uint16_t motionArea)
```
Manually add motion data to temporal history.

**Parameters:**
- `timestamp` - Time of detection (milliseconds)
- `motionLevel` - Motion intensity (0.0 to 1.0)
- `centerX` - Motion center X coordinate
- `centerY` - Motion center Y coordinate
- `motionArea` - Motion area in pixels

---

#### isFalsePositive()

```cpp
bool isFalsePositive()
```
Check if current motion is likely false positive.

**Returns:** `true` if false positive detected

---

### Configuration Methods

#### setSensitivity()

```cpp
void setSensitivity(float threshold)
```
Set sensitivity threshold for pattern detection.

**Parameters:**
- `threshold` - Sensitivity (0.0 to 1.0)

---

#### setEnabled()

```cpp
void setEnabled(bool enable)
```
Enable or disable temporal analysis.

---

#### clearHistory()

```cpp
void clearHistory()
```
Clear temporal motion history buffer.

---

### Query Methods

#### getTotalAnalyzed()

```cpp
uint32_t getTotalAnalyzed() const
```
Get total patterns analyzed.

---

#### getFalsePositivesDetected()

```cpp
uint32_t getFalsePositivesDetected() const
```
Get number of false positives detected.

---

### Static Utility Methods

#### getPatternDescription()

```cpp
static String getPatternDescription(MotionPattern pattern)
```
Get description of motion pattern.

**Returns:** Pattern description string

---

## LoRaWildlifeAlerts

Wildlife alert distribution via LoRa mesh network.

### Initialization

```cpp
bool init()
```
Initialize LoRa wildlife alert system.

**Returns:** `true` if initialization successful

---

### Alert Transmission

#### sendAlert()

```cpp
bool sendAlert(const DangerousSpeciesAlert& alert)
```
Send wildlife alert via LoRa mesh network.

**Parameters:**
- `alert` - Dangerous species alert to transmit

**Returns:** `true` if alert queued/sent successfully

**Example:**
```cpp
DangerousSpeciesAlert* alert = alertSystem.processClassification(result);
if (alert != nullptr) {
    LoRaWildlifeAlerts::sendAlert(*alert);
}
```

---

#### processIncomingAlert()

```cpp
void processIncomingAlert(const String& message)
```
Process incoming alert from mesh network (called by LoRa receive handler).

**Parameters:**
- `message` - JSON alert message received

---

#### flushPendingAlerts()

```cpp
void flushPendingAlerts()
```
Force transmission of all pending alerts.

---

### Configuration Methods

#### setLocation()

```cpp
void setLocation(float lat, float lon)
```
Set GPS coordinates for this node.

**Parameters:**
- `lat` - Latitude
- `lon` - Longitude

**Example:**
```cpp
LoRaWildlifeAlerts::setLocation(45.5231, -122.6765);
```

---

#### setAlertCallback()

```cpp
void setAlertCallback(void (*callback)(const WildlifeAlertMessage&))
```
Register callback for received alerts.

**Parameters:**
- `callback` - Function pointer to callback

**Example:**
```cpp
void onAlertReceived(const WildlifeAlertMessage& alert) {
    Serial.printf("Alert from node %d: %s\n", 
                 alert.sourceNodeId, 
                 alert.speciesName);
}

LoRaWildlifeAlerts::setAlertCallback(onAlertReceived);
```

---

#### setEnabled()

```cpp
void setEnabled(bool enable)
```
Enable or disable alert system.

---

### Query Methods

#### isEnabled()

```cpp
bool isEnabled()
```
Check if alert system is enabled.

---

#### getStatistics()

```cpp
WildlifeAlertStats getStatistics()
```
Get alert transmission statistics.

**Returns:** `WildlifeAlertStats` structure

---

#### cleanup()

```cpp
void cleanup()
```
Cleanup alert system resources.

---

## Data Structures

### ClassificationResult

```cpp
struct ClassificationResult {
    SpeciesType species;              // Detected species
    float confidence;                  // Confidence score (0.0-1.0)
    ConfidenceLevel confidenceLevel;   // Confidence category
    uint32_t inferenceTime;            // Inference time (ms)
    bool isValid;                      // Valid detection flag
    String speciesName;                // Human-readable name
    uint8_t animalCount;               // Number of animals detected
    float boundingBoxes[10][4];        // Bounding boxes (x,y,w,h)
};
```

---

### DangerousSpeciesAlert

```cpp
struct DangerousSpeciesAlert {
    SpeciesType species;          // Detected species
    String speciesName;           // Species name
    float confidence;             // Detection confidence
    AlertPriority priority;       // Alert priority level
    uint32_t timestamp;           // Detection timestamp
    bool requiresImmediate;       // Immediate action flag
    String alertMessage;          // Formatted message
    uint32_t detectionCount;      // Consecutive detections
    bool alertSent;              // Transmission status
};
```

---

### MotionPatternResult

```cpp
struct MotionPatternResult {
    MotionPattern pattern;           // Identified pattern
    float confidence;                // Pattern confidence
    bool isAnimalMotion;            // Animal motion flag
    bool isEnvironmental;           // Environmental motion flag
    uint32_t analysisTime;          // Analysis time (ms)
    String description;             // Pattern description
    float averageSpeed;             // Average speed (px/frame)
    float directionConsistency;     // Direction consistency
    float rhythmScore;              // Movement rhythm
    uint32_t movementDuration;      // Duration (ms)
};
```

---

### WildlifeAlertMessage

```cpp
struct WildlifeAlertMessage {
    uint32_t alertId;              // Unique alert ID
    uint32_t sourceNodeId;         // Source node ID
    uint32_t timestamp;            // Detection time
    uint8_t speciesType;           // Species enum value
    char speciesName[32];          // Species name
    float confidence;              // Detection confidence
    uint8_t alertPriority;         // Alert priority
    bool requiresImmediate;        // Immediate flag
    float latitude;                // GPS latitude
    float longitude;               // GPS longitude
    uint8_t transmissionPriority;  // Transmission priority
    uint8_t transmissionStatus;    // Status
    uint8_t retryCount;            // Retry count
    uint32_t lastTransmitTime;     // Last transmit time
};
```

---

## Enumerations

### SpeciesType

```cpp
enum class SpeciesType {
    UNKNOWN = 0,
    WHITE_TAILED_DEER = 1,
    BLACK_BEAR = 2,           // Dangerous
    RED_FOX = 3,
    GRAY_WOLF = 4,            // Dangerous
    MOUNTAIN_LION = 5,        // Dangerous
    ELK = 6,
    MOOSE = 7,
    RACCOON = 8,
    COYOTE = 9,               // Dangerous
    BOBCAT = 10,              // Dangerous
    WILD_TURKEY = 11,
    BALD_EAGLE = 12,
    RED_TAILED_HAWK = 13,
    GREAT_BLUE_HERON = 14,
    RABBIT = 15,
    SQUIRREL = 16,
    SKUNK = 17,
    OPOSSUM = 18,
    BEAVER = 19,
    OTTER = 20,
    HUMAN = 50
};
```

---

### AlertPriority

```cpp
enum class AlertPriority {
    LOW = 0,        // Low risk species
    MEDIUM = 1,     // Moderate risk
    HIGH = 2,       // Dangerous predators
    CRITICAL = 3    // Large dangerous animals
};
```

---

### MotionPattern

```cpp
enum class MotionPattern {
    UNKNOWN = 0,
    ANIMAL_WALKING = 1,      // Steady rhythmic
    ANIMAL_RUNNING = 2,      // Fast continuous
    ANIMAL_FEEDING = 3,      // Slow stationary
    ANIMAL_GRAZING = 4,      // Slow with pauses
    ENVIRONMENTAL = 5,       // Wind/vegetation
    BIRD_FLIGHT = 6,         // Fast directional
    FALSE_POSITIVE = 7       // Not actual motion
};
```

---

### ConfidenceLevel

```cpp
enum class ConfidenceLevel {
    VERY_LOW = 0,   // < 40%
    LOW = 1,        // 40-60%
    MEDIUM = 2,     // 60-75%
    HIGH = 3,       // 75-90%
    VERY_HIGH = 4   // > 90%
};
```

---

## Complete Usage Example

```cpp
#include "wildlife_classifier.h"
#include "dangerous_species_alert.h"
#include "motion_pattern_analyzer.h"
#include "lora_wildlife_alerts.h"

// Initialize components
WildlifeClassifier classifier;
DangerousSpeciesAlertSystem alertSystem;
MotionPatternAnalyzer patternAnalyzer;

void setup() {
    Serial.begin(115200);
    
    // Initialize systems
    classifier.initialize();
    classifier.setConfidenceThreshold(0.70f);
    
    alertSystem.initialize();
    alertSystem.setMinConsecutiveDetections(2);
    
    patternAnalyzer.initialize();
    
    LoRaMesh::init();
    LoRaWildlifeAlerts::init();
    LoRaWildlifeAlerts::setLocation(45.5231, -122.6765);
}

void loop() {
    // 1. Detect motion
    MotionResult motion = detectMotion();
    
    if (motion.motionDetected) {
        // 2. Analyze pattern
        MotionPatternResult pattern = patternAnalyzer.analyzePattern(motion);
        
        if (pattern.isAnimalMotion) {
            // 3. Classify species
            camera_fb_t* frame = esp_camera_fb_get();
            ClassificationResult result = classifier.classifyFrame(frame);
            esp_camera_fb_return(frame);
            
            if (result.isValid) {
                // 4. Check for dangerous species
                DangerousSpeciesAlert* alert = 
                    alertSystem.processClassification(result);
                
                if (alert != nullptr) {
                    // 5. Send alert
                    LoRaWildlifeAlerts::sendAlert(*alert);
                }
            }
        }
    }
    
    delay(100);
}
```

---

**Version**: 2.0.0  
**Last Updated**: 2024  
**Status**: Production Ready
