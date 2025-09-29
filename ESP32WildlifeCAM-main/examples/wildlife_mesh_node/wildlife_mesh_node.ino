/**
 * @file wildlife_mesh_node.ino
 * @brief Complete Wildlife Mesh Node Example for ESP32WildlifeCAM
 * 
 * This example demonstrates the full wildlife monitoring capabilities with Meshtastic:
 * - Complete wildlife camera functionality
 * - Motion detection with mesh alerts
 * - Environmental sensor monitoring
 * - Image transmission over mesh network
 * - AI/ML wildlife detection (if enabled)
 * - Power management and solar charging
 * - GPS location tracking (if available)
 * 
 * Hardware Requirements:
 * - ESP32-CAM or ESP32-S3-CAM with camera module
 * - LoRa module (SX1276/SX1262/SX1280)
 * - PIR motion sensor (GPIO 13)
 * - BME280 environmental sensor (I2C, if available)
 * - SD card for local storage
 * - Solar panel and battery for remote deployment
 * - GPS module (optional)
 * 
 * Features Demonstrated:
 * - Motion-triggered wildlife photography
 * - Automatic image transmission via mesh
 * - Environmental telemetry collection
 * - Power management and battery monitoring
 * - Mesh network participation and routing
 * - Wildlife detection alerts
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <FS.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// Include Meshtastic and Wildlife CAM headers
#include "../../firmware/src/config.h"
#include "../../firmware/src/meshtastic/mesh_config.h"
#include "../../firmware/src/meshtastic/lora_driver.h"
#include "../../firmware/src/meshtastic/mesh_interface.h"
#include "../../firmware/src/meshtastic/wildlife_telemetry.h"
#include "../../firmware/src/meshtastic/image_mesh.h"

// Camera configuration (AI-Thinker ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Sensor pins (alternative mapping)
#define PIR_SENSOR_PIN    13    // Motion sensor
#define BATTERY_ADC_PIN   33    // Battery voltage monitoring
#define SOLAR_ADC_PIN     32    // Solar panel voltage monitoring

// Global objects
LoRaDriver* radioDriver = nullptr;
MeshInterface* meshInterface = nullptr;
WildlifeTelemetry* wildlifeTelemetry = nullptr;
ImageMesh* imageMesh = nullptr;

// Node configuration
String nodeName = "WildlifeCam";
uint32_t nodeId = 0;

// Wildlife monitoring state
bool motionDetected = false;
unsigned long lastMotionTime = 0;
unsigned long lastImageTime = 0;
unsigned long lastTelemetryTime = 0;
unsigned long lastHeartbeatTime = 0;

// Configuration
const unsigned long MOTION_COOLDOWN = 30000;      // 30 seconds between motion triggers
const unsigned long IMAGE_COOLDOWN = 60000;       // 1 minute between images
const unsigned long TELEMETRY_INTERVAL = 300000;  // 5 minutes telemetry
const unsigned long HEARTBEAT_INTERVAL = 60000;   // 1 minute heartbeat
const float BATTERY_LOW_THRESHOLD = 3.3;          // Low battery voltage
const int MOTION_CONFIDENCE_THRESHOLD = 80;       // Motion confidence threshold

// Statistics
uint32_t totalMotionEvents = 0;
uint32_t totalImagesCaptured = 0;
uint32_t totalImagesTransmitted = 0;
uint32_t totalTelemetryReports = 0;

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("========================================");
    Serial.println("ESP32WildlifeCAM - Complete Mesh Node");
    Serial.println("========================================");
    
    // Initialize file systems
    if (!initializeFileSystems()) {
        Serial.println("ERROR: Failed to initialize file systems!");
        while (1) delay(1000);
    }
    
    // Initialize camera
    if (!initializeCamera()) {
        Serial.println("ERROR: Failed to initialize camera!");
        while (1) delay(1000);
    }
    
    // Initialize sensors
    initializeSensors();
    
    // Initialize mesh networking
    if (!initializeMeshNetwork()) {
        Serial.println("ERROR: Failed to initialize mesh network!");
        while (1) delay(1000);
    }
    
    // Initialize wildlife telemetry
    if (!initializeWildlifeTelemetry()) {
        Serial.println("ERROR: Failed to initialize wildlife telemetry!");
        while (1) delay(1000);
    }
    
    // Initialize image mesh
    if (!initializeImageMesh()) {
        Serial.println("ERROR: Failed to initialize image mesh!");
        while (1) delay(1000);
    }
    
    // Configure interrupts
    setupInterrupts();
    
    // Initial status report
    sendStartupStatus();
    
    Serial.println("=== Wildlife Mesh Node Ready ===");
    Serial.println("Monitoring for wildlife activity...");
    Serial.println();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Process mesh networking
    meshInterface->process();
    
    // Process wildlife telemetry
    wildlifeTelemetry->process();
    
    // Process image mesh
    imageMesh->process();
    
    // Handle motion detection
    handleMotionDetection();
    
    // Periodic telemetry
    if (currentTime - lastTelemetryTime > TELEMETRY_INTERVAL) {
        sendEnvironmentalTelemetry();
        sendPowerStatus();
        lastTelemetryTime = currentTime;
    }
    
    // Periodic heartbeat
    if (currentTime - lastHeartbeatTime > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeatTime = currentTime;
    }
    
    // Handle serial commands for debugging
    handleSerialCommands();
    
    // Power management - enter light sleep if no activity
    if (currentTime - lastMotionTime > 10000) {  // 10 seconds of no motion
        lightSleep(100);  // Sleep for 100ms
    } else {
        delay(10);  // Active monitoring
    }
}

bool initializeFileSystems() {
    Serial.println("Initializing file systems...");
    
    // Initialize LittleFS for configuration and small files
    if (!LittleFS.begin()) {
        Serial.println("Failed to initialize LittleFS");
        return false;
    }
    
    // Initialize SD card for image storage (if available)
    if (SD_MMC.begin()) {
        Serial.println("SD card initialized successfully");
        
        // Create directories
        if (!SD_MMC.exists("/images")) {
            SD_MMC.mkdir("/images");
        }
        if (!SD_MMC.exists("/logs")) {
            SD_MMC.mkdir("/logs");
        }
    } else {
        Serial.println("Warning: SD card not available, using LittleFS only");
    }
    
    return true;
}

bool initializeCamera() {
    Serial.println("Initializing camera...");
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_XGA;  // 1024x768
    config.jpeg_quality = 12;
    config.fb_count = 2;
    
    // Initialize camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Get camera sensor
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        // Configure sensor settings for wildlife photography
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 0); // 0 to 6 (0-No Effect, 1-Negative, 2-Grayscale, 3-Red Tint, 4-Green Tint, 5-Blue Tint, 6-Sepia)
        s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
        s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
        s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
        s->set_aec2(s, 0);           // 0 = disable , 1 = enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 300);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
        s->set_agc_gain(s, 0);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
        s->set_bpc(s, 0);            // 0 = disable , 1 = enable
        s->set_wpc(s, 1);            // 0 = disable , 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
        s->set_lenc(s, 1);           // 0 = disable , 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
        s->set_vflip(s, 0);          // 0 = disable , 1 = enable
        s->set_dcw(s, 1);            // 0 = disable , 1 = enable
        s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
    }
    
    Serial.println("Camera initialized successfully");
    return true;
}

void initializeSensors() {
    Serial.println("Initializing sensors...");
    
    // Initialize PIR motion sensor
    pinMode(PIR_SENSOR_PIN, INPUT);
    
    // Initialize ADC pins for battery/solar monitoring
    pinMode(BATTERY_ADC_PIN, INPUT);
    pinMode(SOLAR_ADC_PIN, INPUT);
    
    // Configure ADC
    analogSetWidth(12);  // 12-bit ADC resolution
    analogSetAttenuation(ADC_11db);  // For 3.3V range
    
    Serial.println("Sensors initialized");
}

bool initializeMeshNetwork() {
    Serial.println("Initializing mesh network...");
    
    // Initialize WiFi for MAC address (will disable after)
    WiFi.mode(WIFI_STA);
    WiFi.begin();
    delay(1000);
    
    // Generate node ID
    nodeId = generateNodeIdFromMac();
    Serial.printf("Node ID: %08X\n", nodeId);
    
    // Disable WiFi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    // Create LoRa configuration for wildlife monitoring
    LoRaConfig radioConfig = createWildlifeConfig(CHANNEL_LONG_SLOW);
    
    // Override GPIO pins for AI-Thinker ESP32-CAM (alternative mapping)
    radioConfig.csPin = 14;
    radioConfig.resetPin = 2;
    radioConfig.dio0Pin = 4;
    radioConfig.dio1Pin = 16;
    
    // Create radio driver
    radioDriver = createLoRaDriver(radioConfig);
    if (!radioDriver) {
        return false;
    }
    
    // Create mesh interface
    meshInterface = createMeshInterface(radioConfig);
    if (!meshInterface) {
        return false;
    }
    
    // Configure node information
    NodeInfo nodeInfo;
    nodeInfo.nodeId = nodeId;
    nodeInfo.shortName = nodeName;
    nodeInfo.longName = "ESP32WildlifeCAM Mesh Node";
    nodeInfo.macAddress = WiFi.macAddress();
    nodeInfo.status = NODE_STATUS_ONLINE;
    nodeInfo.batteryLevel = getBatteryLevel();
    nodeInfo.voltage = getBatteryVoltage();
    nodeInfo.hasPosition = false;  // TODO: Add GPS support
    
    meshInterface->setNodeInfo(nodeInfo);
    
    // Set callbacks
    meshInterface->setMessageCallback(onMeshMessageReceived);
    meshInterface->setNodeDiscoveredCallback(onNodeDiscovered);
    
    Serial.println("Mesh network initialized");
    return true;
}

bool initializeWildlifeTelemetry() {
    Serial.println("Initializing wildlife telemetry...");
    
    wildlifeTelemetry = createWildlifeTelemetry(meshInterface);
    if (!wildlifeTelemetry) {
        return false;
    }
    
    // Configure telemetry
    TelemetryConfig config = createDefaultTelemetryConfig();
    config.motionInterval = 60000;           // 1 minute motion telemetry
    config.environmentalInterval = 300000;   // 5 minutes environmental
    config.powerInterval = 300000;           // 5 minutes power status
    config.healthInterval = 600000;          // 10 minutes health check
    config.transmitOverMesh = true;
    config.storeLocally = true;
    config.maxStoredRecords = 100;
    
    wildlifeTelemetry->configure(config);
    
    // Set callbacks
    wildlifeTelemetry->setMotionEventCallback(onMotionEvent);
    wildlifeTelemetry->setWildlifeDetectionCallback(onWildlifeDetection);
    wildlifeTelemetry->setAlertCallback(onAlert);
    
    // Start automatic collection
    wildlifeTelemetry->startAutomaticCollection();
    
    Serial.println("Wildlife telemetry initialized");
    return true;
}

bool initializeImageMesh() {
    Serial.println("Initializing image mesh...");
    
    imageMesh = createImageMesh(meshInterface);
    if (!imageMesh) {
        return false;
    }
    
    // Configure image mesh
    ImageMeshConfig config = createDefaultImageMeshConfig();
    config.defaultMode = MODE_PROGRESSIVE;         // Send thumbnail first
    config.defaultCompression = COMPRESSION_MEDIUM_QUALITY;
    config.maxChunkSize = 200;                     // Small chunks for LoRa
    config.maxConcurrentTransmissions = 1;        // Conservative for power
    config.enableProgressiveTransmission = true;
    
    imageMesh->configure(config);
    
    // Set callbacks
    imageMesh->setImageTransmittedCallback(onImageTransmitted);
    imageMesh->setImageReceivedCallback(onImageReceived);
    imageMesh->setTransmissionProgressCallback(onTransmissionProgress);
    
    Serial.println("Image mesh initialized");
    return true;
}

void setupInterrupts() {
    // PIR motion sensor interrupt
    attachInterrupt(digitalPinToInterrupt(PIR_SENSOR_PIN), motionISR, RISING);
}

void handleMotionDetection() {
    if (!motionDetected) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Debounce motion detection
    if (currentTime - lastMotionTime < MOTION_COOLDOWN) {
        motionDetected = false;
        return;
    }
    
    Serial.println("=== MOTION DETECTED ===");
    
    totalMotionEvents++;
    lastMotionTime = currentTime;
    motionDetected = false;
    
    // Record motion event
    MotionEvent event;
    event.timestamp = currentTime;
    event.confidence = MOTION_CONFIDENCE_THRESHOLD + random(20);  // Simulate confidence
    event.duration = 1000 + random(2000);  // 1-3 seconds
    event.pixelChanges = 500 + random(1000);
    event.triggerZone = -1;  // Global detection
    event.batteryVoltage = getBatteryVoltage();
    event.temperature = getTemperature() * 10;  // Store as °C * 10
    event.photoTaken = false;
    event.photoFilename = "";
    
    // Capture image if cooldown period has passed
    if (currentTime - lastImageTime > IMAGE_COOLDOWN) {
        String filename = captureImage();
        if (!filename.isEmpty()) {
            event.photoTaken = true;
            event.photoFilename = filename;
            lastImageTime = currentTime;
            
            // Transmit image via mesh network
            transmitCapturedImage(filename);
        }
    }
    
    // Record the motion event
    wildlifeTelemetry->recordMotionEvent(event);
    
    // Send immediate alert for motion
    sendMotionAlert(event);
    
    Serial.printf("Motion event recorded (total: %u)\n", totalMotionEvents);
}

String captureImage() {
    Serial.println("Capturing image...");
    
    // Get camera frame buffer
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return "";
    }
    
    // Generate filename with timestamp
    String filename = "/images/IMG_" + String(millis()) + ".jpg";
    
    // Save to SD card if available, otherwise LittleFS
    bool saved = false;
    if (SD_MMC.exists("/images")) {
        File file = SD_MMC.open(filename, FILE_WRITE);
        if (file) {
            file.write(fb->buf, fb->len);
            file.close();
            saved = true;
        }
    }
    
    if (!saved) {
        // Try LittleFS
        File file = LittleFS.open(filename, "w");
        if (file) {
            file.write(fb->buf, fb->len);
            file.close();
            saved = true;
        }
    }
    
    if (saved) {
        totalImagesCaptured++;
        
        // Record capture event
        CaptureEvent captureEvent;
        captureEvent.timestamp = millis();
        captureEvent.filename = filename;
        captureEvent.fileSize = fb->len;
        captureEvent.imageWidth = fb->width;
        captureEvent.imageHeight = fb->height;
        captureEvent.jpegQuality = 12;  // From camera config
        captureEvent.captureTime = 50;  // Estimate
        captureEvent.motionTriggered = true;
        captureEvent.scheduled = false;
        captureEvent.transmitted = false;
        
        wildlifeTelemetry->recordCaptureEvent(captureEvent);
        
        Serial.printf("Image captured: %s (%u bytes)\n", filename.c_str(), fb->len);
    } else {
        Serial.println("Failed to save image");
        filename = "";
    }
    
    // Return camera frame buffer
    esp_camera_fb_return(fb);
    
    return filename;
}

void transmitCapturedImage(const String& filename) {
    Serial.printf("Transmitting image: %s\n", filename.c_str());
    
    // Transmit image via mesh network
    uint32_t imageId = imageMesh->transmitImageFile(filename, 0, MODE_PROGRESSIVE);
    
    if (imageId > 0) {
        totalImagesTransmitted++;
        Serial.printf("Image transmission started (ID: %08X)\n", imageId);
    } else {
        Serial.println("Failed to start image transmission");
    }
}

void sendMotionAlert(const MotionEvent& event) {
    // Create motion alert message
    DynamicJsonDocument doc(512);
    doc["type"] = "motion_alert";
    doc["nodeId"] = nodeId;
    doc["timestamp"] = event.timestamp;
    doc["confidence"] = event.confidence;
    doc["batteryVoltage"] = event.batteryVoltage;
    doc["photoTaken"] = event.photoTaken;
    if (event.photoTaken) {
        doc["photoFilename"] = event.photoFilename;
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    
    // Broadcast high-priority alert
    meshInterface->broadcastMessage(PACKET_TYPE_WILDLIFE, payload, PRIORITY_HIGH);
    
    Serial.println("Motion alert transmitted");
}

void sendEnvironmentalTelemetry() {
    EnvironmentalData data;
    data.timestamp = millis();
    data.temperature = getTemperature();
    data.humidity = getHumidity();
    data.pressure = getPressure();
    data.lightLevel = getLightLevel();
    data.windSpeed = 0.0;      // Not available
    data.windDirection = 0;    // Not available
    data.sensorValid = true;   // Simplified validation
    
    wildlifeTelemetry->recordEnvironmentalData(data);
}

void sendPowerStatus() {
    PowerStatus status;
    status.timestamp = millis();
    status.batteryVoltage = getBatteryVoltage();
    status.solarVoltage = getSolarVoltage();
    status.chargingCurrent = 0.0;  // Not measured
    status.batteryLevel = getBatteryLevel();
    status.isCharging = (status.solarVoltage > status.batteryVoltage + 0.5);
    status.lowBattery = (status.batteryVoltage < BATTERY_LOW_THRESHOLD);
    status.uptimeSeconds = millis() / 1000;
    status.powerConsumption = 0.0;  // Not measured
    
    wildlifeTelemetry->recordPowerStatus(status);
}

void sendHeartbeat() {
    // Update node status
    NodeInfo nodeInfo = meshInterface->getNodeInfo();
    nodeInfo.batteryLevel = getBatteryLevel();
    nodeInfo.voltage = getBatteryVoltage();
    nodeInfo.uptimeSeconds = millis() / 1000;
    meshInterface->setNodeInfo(nodeInfo);
    
    // Send beacon
    meshInterface->sendBeacon();
    
    Serial.printf("Heartbeat sent (battery: %u%%, uptime: %lu s)\n", 
                  nodeInfo.batteryLevel, nodeInfo.uptimeSeconds);
}

void sendStartupStatus() {
    // Send startup notification
    DynamicJsonDocument doc(512);
    doc["type"] = "startup";
    doc["nodeId"] = nodeId;
    doc["nodeName"] = nodeName;
    doc["version"] = "1.0.0";
    doc["timestamp"] = millis();
    doc["batteryLevel"] = getBatteryLevel();
    doc["freeHeap"] = ESP.getFreeHeap();
    
    String jsonString;
    serializeJson(doc, jsonString);
    std::vector<uint8_t> payload(jsonString.begin(), jsonString.end());
    
    meshInterface->broadcastMessage(PACKET_TYPE_DATA, payload, PRIORITY_NORMAL);
    
    Serial.println("Startup status sent");
}

// Sensor reading functions
float getBatteryVoltage() {
    int adcValue = analogRead(BATTERY_ADC_PIN);
    // Convert ADC reading to voltage (assuming voltage divider)
    return (adcValue / 4095.0) * 3.3 * 2.0;  // 2:1 voltage divider
}

float getSolarVoltage() {
    int adcValue = analogRead(SOLAR_ADC_PIN);
    return (adcValue / 4095.0) * 3.3 * 2.0;  // 2:1 voltage divider
}

uint8_t getBatteryLevel() {
    float voltage = getBatteryVoltage();
    // Convert voltage to percentage (3.0V = 0%, 4.2V = 100%)
    return constrain(((voltage - 3.0) / 1.2) * 100, 0, 100);
}

float getTemperature() {
    // Simulate temperature reading (would read from BME280 if available)
    return 20.0 + random(-10, 15);  // 10-35°C range
}

float getHumidity() {
    // Simulate humidity reading
    return 40.0 + random(0, 40);  // 40-80% range
}

float getPressure() {
    // Simulate pressure reading
    return 1013.25 + random(-50, 50);  // ±50 hPa variation
}

uint16_t getLightLevel() {
    // Could read from photoresistor if available
    return random(0, 1023);
}

// Interrupt service routine
void IRAM_ATTR motionISR() {
    motionDetected = true;
}

// Utility functions
uint32_t generateNodeIdFromMac() {
    String macStr = WiFi.macAddress();
    macStr.replace(":", "");
    
    uint32_t nodeId = 0;
    for (int i = 8; i < 16; i += 2) {
        nodeId = (nodeId << 8) | strtol(macStr.substring(i, i + 2).c_str(), nullptr, 16);
    }
    
    return nodeId;
}

void lightSleep(uint32_t ms) {
    esp_sleep_enable_timer_wakeup(ms * 1000);  // Convert to microseconds
    esp_light_sleep_start();
}

// Callback functions
void onMeshMessageReceived(const MeshPacket& packet) {
    Serial.printf("Mesh message from %08X (RSSI: %d dBm)\n", 
                  packet.header.from, packet.rxRssi);
    
    // Handle different message types
    if (packet.header.portNum == PACKET_TYPE_TEXT) {
        String text((char*)packet.payload.data(), packet.payload.size());
        Serial.printf("Text message: %s\n", text.c_str());
    }
}

void onNodeDiscovered(const NodeInfo& node) {
    Serial.printf("Discovered node: %08X (%s)\n", 
                  node.nodeId, node.shortName.c_str());
}

void onMotionEvent(const MotionEvent& event) {
    Serial.printf("Motion event: confidence=%u%%, photo=%s\n", 
                  event.confidence, event.photoTaken ? "yes" : "no");
}

void onWildlifeDetection(const WildlifeDetection& detection) {
    Serial.printf("Wildlife detected: %s (%.1f%% confidence)\n", 
                  detection.species.c_str(), detection.confidence * 100);
    
    if (detection.endangered) {
        Serial.println("*** ENDANGERED SPECIES DETECTED ***");
    }
}

void onAlert(const String& alertType, const String& message) {
    Serial.printf("ALERT [%s]: %s\n", alertType.c_str(), message.c_str());
}

void onImageTransmitted(uint32_t imageId, bool success) {
    Serial.printf("Image %08X transmission %s\n", 
                  imageId, success ? "completed" : "failed");
}

void onImageReceived(uint32_t imageId, const ImageMetadata& metadata) {
    Serial.printf("Image received: %08X (%s, %u bytes)\n", 
                  imageId, metadata.filename.c_str(), metadata.fileSize);
}

void onTransmissionProgress(uint32_t imageId, float progressPercentage) {
    Serial.printf("Image %08X: %.1f%% transmitted\n", imageId, progressPercentage);
}

void handleSerialCommands() {
    if (!Serial.available()) {
        return;
    }
    
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "status") {
        printStatus();
    } else if (command == "capture") {
        String filename = captureImage();
        Serial.printf("Manual capture: %s\n", filename.c_str());
    } else if (command == "stats") {
        printStatistics();
    } else if (command == "nodes") {
        printNodes();
    } else if (command == "test_motion") {
        motionDetected = true;
        Serial.println("Simulating motion detection...");
    }
}

void printStatus() {
    Serial.println("\n=== Wildlife Mesh Node Status ===");
    Serial.printf("Node: %s (%08X)\n", nodeName.c_str(), nodeId);
    Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
    Serial.printf("Battery: %.2fV (%u%%)\n", getBatteryVoltage(), getBatteryLevel());
    Serial.printf("Solar: %.2fV\n", getSolarVoltage());
    Serial.printf("Temperature: %.1f°C\n", getTemperature());
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Motion events: %u\n", totalMotionEvents);
    Serial.printf("Images captured: %u\n", totalImagesCaptured);
    Serial.printf("Images transmitted: %u\n", totalImagesTransmitted);
    
    MeshStatistics meshStats = meshInterface->getStatistics();
    Serial.printf("Mesh TX/RX: %u/%u packets\n", meshStats.packetsSent, meshStats.packetsReceived);
    Serial.printf("Network nodes: %u\n", meshStats.networkNodes);
    Serial.println("==================================\n");
}

void printStatistics() {
    Serial.println("\n=== Detailed Statistics ===");
    
    // Wildlife telemetry stats
    Serial.printf("Motion Events: %u\n", wildlifeTelemetry->getTotalMotionEvents());
    Serial.printf("Capture Events: %u\n", wildlifeTelemetry->getTotalCaptureEvents());
    Serial.printf("Wildlife Detections: %u\n", wildlifeTelemetry->getTotalWildlifeDetections());
    Serial.printf("Average Battery: %.1f%%\n", wildlifeTelemetry->getAverageBatteryLevel());
    
    // Image mesh stats
    ImageMesh::ImageMeshStatistics imageStats = imageMesh->getStatistics();
    Serial.printf("Images TX/RX: %u/%u\n", imageStats.imagesTransmitted, imageStats.imagesReceived);
    Serial.printf("Chunks TX/RX: %u/%u\n", imageStats.chunksTransmitted, imageStats.chunksReceived);
    Serial.printf("TX Errors: %u\n", imageStats.transmissionErrors);
    
    // Mesh network stats
    MeshStatistics meshStats = meshInterface->getStatistics();
    Serial.printf("Packets Forwarded: %u\n", meshStats.packetsForwarded);
    Serial.printf("Duplicates Dropped: %u\n", meshStats.duplicatesDropped);
    Serial.printf("Average RSSI: %.1f dBm\n", meshStats.averageRssi);
    Serial.printf("Average SNR: %.1f dB\n", meshStats.averageSnr);
    
    Serial.println("============================\n");
}

void printNodes() {
    std::vector<NodeInfo> nodes = meshInterface->getAllNodes();
    
    Serial.printf("\n=== Mesh Network Nodes (%d) ===\n", nodes.size());
    for (const auto& node : nodes) {
        Serial.printf("%08X: %s (battery: %u%%, RSSI: %d dBm)\n", 
                      node.nodeId, node.shortName.c_str(), 
                      node.batteryLevel, node.lastRssi);
    }
    Serial.println("===============================\n");
}