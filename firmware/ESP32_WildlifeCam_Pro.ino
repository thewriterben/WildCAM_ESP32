/**
 * Professional ESP32 Wildlife Camera Firmware
 * With AI inference, multi-sensor support, and cloud connectivity
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <SD_MMC.h>
#include <TensorFlowLite_ESP32.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_MLX90640.h>
#include <RTClib.h>
#include <ArduinoJson.h>

// Pin definitions
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define PIR_PIN         13
#define LORA_SS         5
#define LORA_RST        14
#define LORA_DIO0       2
#define BATTERY_PIN     34

// ML Model
#include "wildlife_model.h"  // Generated from Python script

// Encryption for secure LoRa communications
#include "networking/lora_encryption.h"

class ProfessionalWildlifeCam {
private:
    // Core components
    camera_config_t camera_config;
    Adafruit_MLX90640 thermal_cam;
    RTC_DS3231 rtc;
    LoRaEncryption* lora_encryption;
    
    // ML inference
    tflite::MicroInterpreter* interpreter;
    TfLiteTensor* input;
    TfLiteTensor* output;
    
    // System state
    struct SystemStatus {
        float battery_voltage;
        int sd_free_space_mb;
        int detections_today;
        bool wifi_connected;
        bool lora_connected;
        unsigned long uptime;
    } status;
    
    // Configuration
    struct Config {
        int capture_interval_ms = 30000;
        float motion_threshold = 0.7;
        bool use_ai = true;
        bool use_thermal = false;
        bool night_mode = false;
        String wifi_ssid;
        String wifi_password;
        String api_endpoint;
    } config;
    
public:
    void setup() {
        Serial.begin(115200);
        
        // Initialize RTC
        if (!rtc.begin()) {
            Serial.println("RTC failed");
        }
        
        // Initialize SD card
        if (!SD_MMC.begin()) {
            Serial.println("SD Card Mount Failed");
        }
        
        // Initialize camera
        setupCamera();
        
        // Initialize ML model
        setupMLModel();
        
        // Initialize sensors
        pinMode(PIR_PIN, INPUT);
        
        // Initialize thermal camera if available
        if (config.use_thermal) {
            thermal_cam.begin(MLX90640_I2CADDR_DEFAULT, &Wire);
            thermal_cam.setMode(MLX90640_CHESS);
            thermal_cam.setResolution(MLX90640_ADC_19BIT);
        }
        
        // Initialize LoRa encryption
        lora_encryption = new LoRaEncryption(DEFAULT_LORA_KEY);
        if (!lora_encryption->begin()) {
            Serial.println("LoRa encryption init failed");
        } else {
            Serial.println("✓ LoRa encryption initialized (AES-256)");
        }
        
        // Initialize LoRa
        LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
        if (LoRa.begin(915E6)) {
            Serial.println("LoRa init succeeded");
            status.lora_connected = true;
        }
        
        // Connect WiFi
        connectWiFi();
    }
    
    void setupCamera() {
        camera_config.ledc_channel = LEDC_CHANNEL_0;
        camera_config.ledc_timer = LEDC_TIMER_0;
        camera_config.pin_d0 = 5;
        camera_config.pin_d1 = 18;
        camera_config.pin_d2 = 19;
        camera_config.pin_d3 = 21;
        camera_config.pin_d4 = 36;
        camera_config.pin_d5 = 39;
        camera_config.pin_d6 = 34;
        camera_config.pin_d7 = 35;
        camera_config.pin_xclk = CAM_PIN_XCLK;
        camera_config.pin_pclk = 22;
        camera_config.pin_vsync = 25;
        camera_config.pin_href = 23;
        camera_config.pin_sscb_sda = CAM_PIN_SIOD;
        camera_config.pin_sscb_scl = CAM_PIN_SIOC;
        camera_config.pin_pwdn = CAM_PIN_PWDN;
        camera_config.pin_reset = CAM_PIN_RESET;
        camera_config.xclk_freq_hz = 20000000;
        camera_config.pixel_format = PIXFORMAT_JPEG;
        
        // High quality settings
        camera_config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
        camera_config.jpeg_quality = 10;
        camera_config.fb_count = 2;
        
        esp_err_t err = esp_camera_init(&camera_config);
        if (err != ESP_OK) {
            Serial.printf("Camera init failed: 0x%x", err);
        }
        
        // Camera settings optimization
        sensor_t* s = esp_camera_sensor_get();
        s->set_brightness(s, 0);
        s->set_contrast(s, 0);
        s->set_saturation(s, 0);
        s->set_special_effect(s, 0);
        s->set_whitebal(s, 1);
        s->set_awb_gain(s, 1);
        s->set_wb_mode(s, 0);
        s->set_exposure_ctrl(s, 1);
        s->set_aec2(s, 1);
        s->set_ae_level(s, 0);
        s->set_aec_value(s, 300);
        s->set_gain_ctrl(s, 1);
        s->set_agc_gain(s, 0);
        s->set_gainceiling(s, (gainceiling_t)6);
    }
    
    void setupMLModel() {
        // Initialize TensorFlow Lite
        static tflite::MicroErrorReporter micro_error_reporter;
        static tflite::AllOpsResolver resolver;
        
        const tflite::Model* model = tflite::GetModel(wildlife_model);
        
        // Create interpreter
        static const int tensor_arena_size = 100 * 1024;
        static uint8_t tensor_arena[tensor_arena_size];
        
        static tflite::MicroInterpreter static_interpreter(
            model, resolver, tensor_arena, tensor_arena_size, &micro_error_reporter
        );
        
        interpreter = &static_interpreter;
        interpreter->AllocateTensors();
        
        input = interpreter->input(0);
        output = interpreter->output(0);
    }
    
    void loop() {
        // Update system status
        updateStatus();
        
        // Check triggers
        bool should_capture = false;
        String trigger_reason;
        
        if (digitalRead(PIR_PIN) == HIGH) {
            should_capture = true;
            trigger_reason = "motion";
        }
        
        if (config.use_thermal && checkThermalTrigger()) {
            should_capture = true;
            trigger_reason = "thermal";
        }
        
        if (should_capture) {
            captureAndProcess(trigger_reason);
        }
        
        // Periodic capture
        static unsigned long last_capture = 0;
        if (millis() - last_capture > config.capture_interval_ms) {
            captureAndProcess("scheduled");
            last_capture = millis();
        }
        
        // Handle commands
        handleCommands();
        
        // Power management
        managePower();
        
        delay(100);
    }
    
    void captureAndProcess(String trigger_reason) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            return;
        }
        
        // Run AI inference if enabled
        WildlifeDetection detection;
        if (config.use_ai) {
            detection = runInference(fb);
        }
        
        // Create metadata
        StaticJsonDocument<512> metadata;
        metadata["timestamp"] = rtc.now().unixtime();
        metadata["trigger"] = trigger_reason;
        metadata["battery"] = status.battery_voltage;
        
        if (config.use_ai && detection.confidence > 0.5) {
            metadata["species"] = detection.species;
            metadata["confidence"] = detection.confidence;
            
            // Save image if wildlife detected
            saveImage(fb, metadata);
            
            // Send alert
            sendDetectionAlert(detection, metadata);
        }
        
        esp_camera_fb_return(fb);
        status.detections_today++;
    }
    
    struct WildlifeDetection {
        String species;
        float confidence;
        int bbox[4];
    };
    
    WildlifeDetection runInference(camera_fb_t* fb) {
        WildlifeDetection result;
        
        // Preprocess image for model input
        // ... (image resizing and normalization code)
        
        // Run inference
        interpreter->Invoke();
        
        // Parse output
        float max_score = 0;
        int max_index = 0;
        
        for (int i = 0; i < output->dims->data[1]; i++) {
            float score = output->data.f[i];
            if (score > max_score) {
                max_score = score;
                max_index = i;
            }
        }
        
        // Map index to species name
        const char* species_names[] = {
            "deer", "bear", "fox", "rabbit", "bird", "empty"
        };
        
        result.species = species_names[max_index];
        result.confidence = max_score;
        
        return result;
    }
    
    bool checkThermalTrigger() {
        float mlx_temps[32*24];
        thermal_cam.readPixels(mlx_temps);
        
        // Detect warm bodies
        float ambient_temp = mlx_temps[0];  // Corner pixel as reference
        int warm_pixels = 0;
        
        for (int i = 0; i < 768; i++) {
            if (mlx_temps[i] > ambient_temp + 5.0) {  // 5°C above ambient
                warm_pixels++;
            }
        }
        
        return warm_pixels > 50;  // Significant warm object detected
    }
    
    void sendDetectionAlert(WildlifeDetection& detection, JsonDocument& metadata) {
        if (status.wifi_connected) {
            // Send via WiFi (could also add encryption here)
            HTTPClient http;
            http.begin(config.api_endpoint);
            http.addHeader("Content-Type", "application/json");
            
            String payload;
            serializeJson(metadata, payload);
            http.POST(payload);
            http.end();
            
        } else if (status.lora_connected) {
            // Send via encrypted LoRa
            StaticJsonDocument<256> lora_msg;
            lora_msg["id"] = WiFi.macAddress();
            lora_msg["species"] = detection.species;
            lora_msg["conf"] = detection.confidence;
            lora_msg["time"] = metadata["timestamp"];
            
            String msg;
            serializeJson(lora_msg, msg);
            
            // Encrypt message before sending
            String encrypted_msg;
            if (lora_encryption && lora_encryption->encrypt(msg, encrypted_msg)) {
                Serial.println("Sending encrypted LoRa message");
                
                // Send encrypted message
                LoRa.beginPacket();
                LoRa.print(encrypted_msg);
                LoRa.endPacket();
                
                Serial.printf("✓ Encrypted message sent (%d bytes)\n", encrypted_msg.length());
            } else {
                // Fallback to unencrypted if encryption fails
                Serial.println("⚠️ Encryption failed, sending unencrypted message");
                LoRa.beginPacket();
                LoRa.print(msg);
                LoRa.endPacket();
            }
        }
    }
    
    void managePower() {
        status.battery_voltage = analogRead(BATTERY_PIN) * 2.0 / 4095.0 * 3.3;
        
        if (status.battery_voltage < 3.2) {
            // Critical battery - enter deep sleep
            esp_sleep_enable_timer_wakeup(3600 * 1000000);  // 1 hour
            esp_deep_sleep_start();
        } else if (status.battery_voltage < 3.5) {
            // Low battery - reduce capture frequency
            config.capture_interval_ms = 60000;  // 1 minute
            config.use_ai = false;  // Disable AI to save power
        }
    }
};