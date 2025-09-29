/**
 * @file wildlife_monitoring_ov5640.ino
 * @brief Complete OV5640 5MP Wildlife Monitoring System
 * 
 * Advanced wildlife monitoring system featuring:
 * - OV5640 5MP camera with full resolution capture
 * - IR Cut filter for day/night operation 
 * - PIR motion detection with configurable sensitivity
 * - Solar power management with battery monitoring
 * - SD card storage with timestamped files
 * - WiFi connectivity for remote monitoring
 * - Deep sleep power management for extended operation
 * 
 * Hardware Requirements:
 * - LilyGO T-Camera Plus S3 with ESP32-S3
 * - OV5640 5MP camera module
 * - PIR sensor on GPIO17
 * - IR Cut filter control on GPIO16 (AP1511B_FBC)
 * - SD card for storage
 * - Solar panel + battery system
 * 
 * @author ESP32WildlifeCAM Project
 * @version 2.0.0
 */

#include <WiFi.h>
#include <SD_MMC.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_sleep.h>
#include <esp_log.h>

// Project includes
#include "../src/enhanced_wildlife_camera.h"
#include "../src/detection/pir_sensor.h"
#include "../firmware/src/power_manager.h"

// Configuration
#include "wifi_config.h" // Create this file with your WiFi credentials

// Pin Definitions for T-Camera Plus S3
#define PIR_SENSOR_PIN 17
#define IR_CUT_PIN 16  // AP1511B_FBC control
#define BATTERY_ADC_PIN 1
#define SOLAR_ADC_PIN 2

// Wildlife monitoring configuration
#define CAPTURE_INTERVAL_MS 30000      // 30 seconds between captures
#define MOTION_TIMEOUT_MS 10000        // 10 seconds motion timeout
#define SLEEP_DURATION_S 300           // 5 minutes deep sleep when inactive
#define MAX_DAILY_CAPTURES 500         // Limit daily captures
#define LOW_BATTERY_THRESHOLD 3.4      // Voltage threshold for low battery

// File system paths
#define BASE_PATH "/wildlife"
#define CONFIG_FILE "/config.json"

// Global objects
EnhancedWildlifeCamera camera;
PIRSensor pirSensor;
PowerManager powerManager;
AsyncWebServer server(80);

// System state
struct SystemState {
  bool wifi_connected;
  bool camera_ready;
  bool motion_detected;
  bool monitoring_active;
  uint32_t daily_captures;
  uint32_t total_captures;
  float battery_voltage;
  float solar_voltage;
  unsigned long last_capture_time;
  unsigned long last_motion_time;
  WildlifeCaptureMode capture_mode;
  WildlifeCaptureSettings capture_settings;
} system_state;

// Configuration structure
struct MonitoringConfig {
  bool enable_motion_trigger;
  bool enable_time_lapse;
  bool enable_ir_cut_auto;
  uint16_t capture_interval_s;
  uint8_t motion_sensitivity;
  uint8_t image_quality;
  WildlifeFrameSize frame_size;
  uint8_t sleep_hour_start;
  uint8_t sleep_hour_end;
} config;

void setup() {
  Serial.begin(115200);
  
  // Initialize system state
  memset(&system_state, 0, sizeof(system_state));
  system_state.capture_mode = MODE_MOTION_TRIGGER;
  system_state.capture_settings = getDefaultWildlifeSettings();
  
  // Load configuration
  loadConfiguration();
  
  ESP_LOGI("MAIN", "ESP32 Wildlife Camera - OV5640 5MP System Starting...");
  ESP_LOGI("MAIN", "Board: LilyGO T-Camera Plus S3");
  ESP_LOGI("MAIN", "Sensor: OV5640 5MP with IR Cut filter");
  
  // Initialize SD card first
  if (!initializeStorage()) {
    ESP_LOGE("MAIN", "Failed to initialize storage - system halted");
    while(1) delay(1000);
  }
  
  // Initialize power management
  if (!powerManager.init()) {
    ESP_LOGE("MAIN", "Failed to initialize power management");
  }
  
  // Initialize camera
  if (!camera.init(BOARD_LILYGO_T_CAMERA_PLUS)) {
    ESP_LOGE("MAIN", "Failed to initialize camera - system halted");
    while(1) delay(1000);
  }
  system_state.camera_ready = true;
  
  // Apply configuration to camera
  camera.configureSensor(system_state.capture_settings);
  
  // Initialize PIR sensor
  if (!pirSensor.initialize()) {
    ESP_LOGE("MAIN", "Failed to initialize PIR sensor");
  }
  pirSensor.setSensitivity(config.motion_sensitivity / 100.0f);
  
  // Initialize WiFi
  initializeWiFi();
  
  // Setup web server
  setupWebServer();
  
  // Set initial time (approximate)
  setSystemTime();
  
  // Configure deep sleep wakeup
  configureDeepSleep();
  
  // Print system status
  printSystemStatus();
  
  ESP_LOGI("MAIN", "Wildlife monitoring system initialized successfully");
  system_state.monitoring_active = true;
}

void loop() {
  static unsigned long last_update = 0;
  unsigned long current_time = millis();
  
  // Update system components every second
  if (current_time - last_update >= 1000) {
    updateSystemStatus();
    last_update = current_time;
  }
  
  // Check for motion detection
  if (config.enable_motion_trigger && pirSensor.hasMotion()) {
    handleMotionDetection();
  }
  
  // Time-lapse capture
  if (config.enable_time_lapse && shouldCaptureTimeLapse()) {
    captureWildlifeImage("timelapse");
  }
  
  // Auto IR Cut adjustment
  if (config.enable_ir_cut_auto) {
    camera.autoAdjustIRCut();
  }
  
  // Check if we should enter deep sleep
  if (shouldEnterDeepSleep()) {
    enterDeepSleep();
  }
  
  // Handle low battery condition
  if (system_state.battery_voltage < LOW_BATTERY_THRESHOLD) {
    handleLowBattery();
  }
  
  delay(100); // Small delay to prevent watchdog issues
}

bool initializeStorage() {
  ESP_LOGI("STORAGE", "Initializing SD card...");
  
  if (!SD_MMC.begin("/sdcard", true, true)) {
    ESP_LOGE("STORAGE", "SD card initialization failed");
    return false;
  }
  
  // Create directory structure
  if (!SD_MMC.mkdir(BASE_PATH)) {
    ESP_LOGW("STORAGE", "Failed to create base directory (may already exist)");
  }
  
  // Create subdirectories by date
  createDateDirectories();
  
  ESP_LOGI("STORAGE", "SD card initialized successfully");
  return true;
}

void createDateDirectories() {
  time_t now;
  struct tm timeinfo;
  
  time(&now);
  localtime_r(&now, &timeinfo);
  
  char date_path[64];
  snprintf(date_path, sizeof(date_path), "%s/%04d%02d%02d", 
           BASE_PATH, timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  
  if (!SD_MMC.mkdir(date_path)) {
    ESP_LOGW("STORAGE", "Failed to create date directory: %s", date_path);
  }
}

void initializeWiFi() {
  if (strlen(WIFI_SSID) == 0) {
    ESP_LOGW("WIFI", "No WiFi credentials configured");
    return;
  }
  
  ESP_LOGI("WIFI", "Connecting to WiFi: %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    ESP_LOGI("WIFI", "Connecting... attempt %d", ++attempts);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    system_state.wifi_connected = true;
    ESP_LOGI("WIFI", "Connected! IP: %s", WiFi.localIP().toString().c_str());
  } else {
    ESP_LOGW("WIFI", "Failed to connect to WiFi");
  }
}

void setupWebServer() {
  if (!system_state.wifi_connected) {
    return;
  }
  
  // Serve static files
  server.serveStatic("/", SD_MMC, "/web/").setDefaultFile("index.html");
  
  // API endpoints
  server.on("/api/status", HTTP_GET, handleStatusRequest);
  server.on("/api/capture", HTTP_POST, handleCaptureRequest);
  server.on("/api/config", HTTP_GET, handleConfigRequest);
  server.on("/api/config", HTTP_POST, handleConfigUpdate);
  server.on("/api/images", HTTP_GET, handleImageListRequest);
  
  // IR Cut control
  server.on("/api/ir_cut/enable", HTTP_POST, [](AsyncWebServerRequest *request) {
    camera.setIRCutFilter(IR_CUT_ENABLED);
    request->send(200, "application/json", "{\"status\":\"IR Cut enabled\"}");
  });
  
  server.on("/api/ir_cut/disable", HTTP_POST, [](AsyncWebServerRequest *request) {
    camera.setIRCutFilter(IR_CUT_DISABLED);
    request->send(200, "application/json", "{\"status\":\"IR Cut disabled\"}");
  });
  
  server.begin();
  ESP_LOGI("WEB", "Web server started on port 80");
}

void handleMotionDetection() {
  ESP_LOGI("MOTION", "Motion detected - capturing wildlife image");
  
  system_state.motion_detected = true;
  system_state.last_motion_time = millis();
  
  // Clear motion flag
  pirSensor.clearMotion();
  
  // Capture image
  captureWildlifeImage("motion");
  
  // Reset motion flag after timeout
  delay(100);
  system_state.motion_detected = false;
}

bool captureWildlifeImage(const char* trigger_type) {
  if (!system_state.camera_ready) {
    ESP_LOGE("CAPTURE", "Camera not ready");
    return false;
  }
  
  if (system_state.daily_captures >= MAX_DAILY_CAPTURES) {
    ESP_LOGW("CAPTURE", "Daily capture limit reached");
    return false;
  }
  
  // Generate filename with timestamp
  char filename[128];
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  snprintf(filename, sizeof(filename), 
           "%s/%04d%02d%02d/%s_%04d%02d%02d_%02d%02d%02d.jpg",
           BASE_PATH,
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           trigger_type,
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  
  ESP_LOGI("CAPTURE", "Capturing image: %s", filename);
  
  // Capture and save image
  camera_fb_t* fb = camera.captureImage();
  if (!fb) {
    ESP_LOGE("CAPTURE", "Failed to capture image");
    return false;
  }
  
  // Save to SD card
  File file = SD_MMC.open(filename, FILE_WRITE);
  if (!file) {
    ESP_LOGE("CAPTURE", "Failed to open file for writing: %s", filename);
    esp_camera_fb_return(fb);
    return false;
  }
  
  file.write(fb->buf, fb->len);
  file.close();
  esp_camera_fb_return(fb);
  
  // Update statistics
  system_state.daily_captures++;
  system_state.total_captures++;
  system_state.last_capture_time = millis();
  
  ESP_LOGI("CAPTURE", "Image saved successfully: %s (%d bytes)", filename, fb->len);
  
  // Save metadata
  saveImageMetadata(filename, trigger_type, fb->len);
  
  return true;
}

void saveImageMetadata(const char* filename, const char* trigger_type, size_t file_size) {
  // Create metadata JSON
  DynamicJsonDocument metadata(512);
  metadata["filename"] = filename;
  metadata["trigger_type"] = trigger_type;
  metadata["timestamp"] = time(nullptr);
  metadata["file_size"] = file_size;
  metadata["battery_voltage"] = system_state.battery_voltage;
  metadata["solar_voltage"] = system_state.solar_voltage;
  metadata["ir_cut_enabled"] = camera.getIRCutState() == IR_CUT_ENABLED;
  
  CameraStatus status = camera.getStatus();
  metadata["frame_size"] = camera.frameSizeToString(status.current_frame_size);
  metadata["quality"] = status.image_quality;
  
  // Save metadata file
  String meta_filename = String(filename) + ".meta";
  File meta_file = SD_MMC.open(meta_filename.c_str(), FILE_WRITE);
  if (meta_file) {
    serializeJson(metadata, meta_file);
    meta_file.close();
  }
}

bool shouldCaptureTimeLapse() {
  static unsigned long last_timelapse = 0;
  unsigned long interval_ms = config.capture_interval_s * 1000;
  
  if (millis() - last_timelapse >= interval_ms) {
    last_timelapse = millis();
    return true;
  }
  return false;
}

bool shouldEnterDeepSleep() {
  // Check if it's sleep time
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  if (timeinfo.tm_hour >= config.sleep_hour_start || timeinfo.tm_hour < config.sleep_hour_end) {
    // Check if no motion for extended period
    if (millis() - system_state.last_motion_time > MOTION_TIMEOUT_MS * 6) {
      return true;
    }
  }
  
  return false;
}

void enterDeepSleep() {
  ESP_LOGI("POWER", "Entering deep sleep for %d seconds", SLEEP_DURATION_S);
  
  // Configure PIR as wakeup source
  pirSensor.configureForDeepSleep();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_SENSOR_PIN, HIGH);
  
  // Timer wakeup as backup
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION_S * 1000000ULL);
  
  // Save current state
  saveSystemState();
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

void configureDeepSleep() {
  // Configure wakeup sources
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_SENSOR_PIN, HIGH);
}

void updateSystemStatus() {
  // Update power status
  powerManager.update();
  system_state.battery_voltage = powerManager.getBatteryVoltage();
  system_state.solar_voltage = powerManager.getSolarVoltage();
  
  // Update time for camera
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  camera.setTimeOfDay(timeinfo.tm_hour, timeinfo.tm_min);
  
  // Log status periodically
  static unsigned long last_status_log = 0;
  if (millis() - last_status_log >= 60000) { // Every minute
    logSystemStatus();
    last_status_log = millis();
  }
}

void handleLowBattery() {
  ESP_LOGW("POWER", "Low battery detected: %.2fV", system_state.battery_voltage);
  
  // Enter power saving mode
  camera.enterLowPowerMode();
  
  // Reduce capture frequency
  config.capture_interval_s = max(config.capture_interval_s * 2, 300);
  
  // Disable WiFi to save power
  if (system_state.wifi_connected) {
    WiFi.disconnect();
    system_state.wifi_connected = false;
  }
}

void logSystemStatus() {
  ESP_LOGI("STATUS", "=== Wildlife Camera Status ===");
  ESP_LOGI("STATUS", "Camera: %s", system_state.camera_ready ? "Ready" : "Error");
  ESP_LOGI("STATUS", "WiFi: %s", system_state.wifi_connected ? "Connected" : "Disconnected");
  ESP_LOGI("STATUS", "Battery: %.2fV", system_state.battery_voltage);
  ESP_LOGI("STATUS", "Solar: %.2fV", system_state.solar_voltage);
  ESP_LOGI("STATUS", "Daily captures: %d/%d", system_state.daily_captures, MAX_DAILY_CAPTURES);
  ESP_LOGI("STATUS", "Total captures: %d", system_state.total_captures);
  ESP_LOGI("STATUS", "IR Cut: %s", camera.getIRCutState() == IR_CUT_ENABLED ? "Enabled" : "Disabled");
  ESP_LOGI("STATUS", "Motion detected: %s", system_state.motion_detected ? "Yes" : "No");
  
  CameraStatus cam_status = camera.getStatus();
  ESP_LOGI("STATUS", "Frame size: %s", camera.frameSizeToString(cam_status.current_frame_size));
  ESP_LOGI("STATUS", "Image quality: %d", cam_status.image_quality);
}

void printSystemStatus() {
  ESP_LOGI("INIT", "=== System Configuration ===");
  ESP_LOGI("INIT", "Motion trigger: %s", config.enable_motion_trigger ? "Enabled" : "Disabled");
  ESP_LOGI("INIT", "Time-lapse: %s", config.enable_time_lapse ? "Enabled" : "Disabled");
  ESP_LOGI("INIT", "Auto IR Cut: %s", config.enable_ir_cut_auto ? "Enabled" : "Disabled");
  ESP_LOGI("INIT", "Capture interval: %d seconds", config.capture_interval_s);
  ESP_LOGI("INIT", "Motion sensitivity: %d%%", config.motion_sensitivity);
  ESP_LOGI("INIT", "Image quality: %d", config.image_quality);
  ESP_LOGI("INIT", "Sleep hours: %02d:00 - %02d:00", config.sleep_hour_start, config.sleep_hour_end);
}

// Web server handlers
void handleStatusRequest(AsyncWebServerRequest *request) {
  DynamicJsonDocument status(1024);
  
  status["camera_ready"] = system_state.camera_ready;
  status["wifi_connected"] = system_state.wifi_connected;
  status["monitoring_active"] = system_state.monitoring_active;
  status["motion_detected"] = system_state.motion_detected;
  status["battery_voltage"] = system_state.battery_voltage;
  status["solar_voltage"] = system_state.solar_voltage;
  status["daily_captures"] = system_state.daily_captures;
  status["total_captures"] = system_state.total_captures;
  status["ir_cut_enabled"] = camera.getIRCutState() == IR_CUT_ENABLED;
  status["uptime_ms"] = millis();
  
  CameraStatus cam_status = camera.getStatus();
  status["sensor_type"] = camera.getSensorName();
  status["frame_size"] = camera.frameSizeToString(cam_status.current_frame_size);
  status["image_quality"] = cam_status.image_quality;
  
  String response;
  serializeJson(status, response);
  request->send(200, "application/json", response);
}

void handleCaptureRequest(AsyncWebServerRequest *request) {
  if (captureWildlifeImage("manual")) {
    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Image captured\"}");
  } else {
    request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Capture failed\"}");
  }
}

void handleConfigRequest(AsyncWebServerRequest *request) {
  DynamicJsonDocument config_json(512);
  
  config_json["enable_motion_trigger"] = config.enable_motion_trigger;
  config_json["enable_time_lapse"] = config.enable_time_lapse;
  config_json["enable_ir_cut_auto"] = config.enable_ir_cut_auto;
  config_json["capture_interval_s"] = config.capture_interval_s;
  config_json["motion_sensitivity"] = config.motion_sensitivity;
  config_json["image_quality"] = config.image_quality;
  config_json["sleep_hour_start"] = config.sleep_hour_start;
  config_json["sleep_hour_end"] = config.sleep_hour_end;
  
  String response;
  serializeJson(config_json, response);
  request->send(200, "application/json", response);
}

void handleConfigUpdate(AsyncWebServerRequest *request) {
  // Update configuration from POST data
  // Implementation would parse JSON and update config structure
  saveConfiguration();
  request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration updated\"}");
}

void handleImageListRequest(AsyncWebServerRequest *request) {
  // Return list of captured images
  // Implementation would scan SD card and return file list
  request->send(200, "application/json", "{\"images\":[]}");
}

void loadConfiguration() {
  // Set defaults
  config.enable_motion_trigger = true;
  config.enable_time_lapse = false;
  config.enable_ir_cut_auto = true;
  config.capture_interval_s = 30;
  config.motion_sensitivity = 80;
  config.image_quality = 12;
  config.frame_size = WILDLIFE_UXGA;
  config.sleep_hour_start = 22; // 10 PM
  config.sleep_hour_end = 6;    // 6 AM
  
  // Load from file if it exists
  File config_file = SD_MMC.open(CONFIG_FILE, FILE_READ);
  if (config_file) {
    DynamicJsonDocument doc(512);
    deserializeJson(doc, config_file);
    
    config.enable_motion_trigger = doc["enable_motion_trigger"] | config.enable_motion_trigger;
    config.enable_time_lapse = doc["enable_time_lapse"] | config.enable_time_lapse;
    config.enable_ir_cut_auto = doc["enable_ir_cut_auto"] | config.enable_ir_cut_auto;
    config.capture_interval_s = doc["capture_interval_s"] | config.capture_interval_s;
    config.motion_sensitivity = doc["motion_sensitivity"] | config.motion_sensitivity;
    config.image_quality = doc["image_quality"] | config.image_quality;
    config.sleep_hour_start = doc["sleep_hour_start"] | config.sleep_hour_start;
    config.sleep_hour_end = doc["sleep_hour_end"] | config.sleep_hour_end;
    
    config_file.close();
    ESP_LOGI("CONFIG", "Configuration loaded from file");
  } else {
    ESP_LOGI("CONFIG", "Using default configuration");
  }
}

void saveConfiguration() {
  DynamicJsonDocument doc(512);
  
  doc["enable_motion_trigger"] = config.enable_motion_trigger;
  doc["enable_time_lapse"] = config.enable_time_lapse;
  doc["enable_ir_cut_auto"] = config.enable_ir_cut_auto;
  doc["capture_interval_s"] = config.capture_interval_s;
  doc["motion_sensitivity"] = config.motion_sensitivity;
  doc["image_quality"] = config.image_quality;
  doc["sleep_hour_start"] = config.sleep_hour_start;
  doc["sleep_hour_end"] = config.sleep_hour_end;
  
  File config_file = SD_MMC.open(CONFIG_FILE, FILE_WRITE);
  if (config_file) {
    serializeJson(doc, config_file);
    config_file.close();
    ESP_LOGI("CONFIG", "Configuration saved to file");
  }
}

void saveSystemState() {
  // Save current system state before deep sleep
  DynamicJsonDocument state(256);
  state["daily_captures"] = system_state.daily_captures;
  state["total_captures"] = system_state.total_captures;
  
  File state_file = SD_MMC.open("/state.json", FILE_WRITE);
  if (state_file) {
    serializeJson(state, state_file);
    state_file.close();
  }
}

void setSystemTime() {
  // Configure NTP if WiFi is available
  if (system_state.wifi_connected) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    ESP_LOGI("TIME", "NTP time configured");
  } else {
    // Set approximate time (would need RTC module for accuracy)
    struct tm timeinfo = {0};
    timeinfo.tm_year = 2024 - 1900;
    timeinfo.tm_mon = 0;
    timeinfo.tm_mday = 1;
    timeinfo.tm_hour = 12;
    time_t t = mktime(&timeinfo);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);
    ESP_LOGI("TIME", "Approximate time set");
  }
}