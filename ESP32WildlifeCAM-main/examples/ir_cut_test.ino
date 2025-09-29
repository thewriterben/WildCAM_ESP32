/**
 * @file ir_cut_test.ino
 * @brief IR Cut Filter Testing Utility
 * 
 * Test utility for the IR Cut filter control system on the T-Camera Plus S3.
 * Provides manual and automatic testing of the AP1511B_FBC IR Cut filter
 * with visual feedback and performance measurements.
 * 
 * Features:
 * - Manual IR Cut filter control
 * - Automatic day/night cycling test
 * - Image capture comparison (day vs night mode)
 * - Filter response time measurement
 * - Serial command interface
 * - Visual LED feedback
 * 
 * Hardware Requirements:
 * - LilyGO T-Camera Plus S3
 * - OV5640 camera module
 * - IR Cut filter on GPIO16 (AP1511B_FBC)
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

#include <Arduino.h>
#include <esp_log.h>
#include <esp_camera.h>
#include <driver/gpio.h>
#include <SD_MMC.h>

// Project includes
#include "../src/enhanced_wildlife_camera.h"

// Pin definitions
#define IR_CUT_PIN 16          // AP1511B_FBC control pin
#define STATUS_LED_PIN 2       // Built-in LED for status indication
#define TEST_BUTTON_PIN 0      // Boot button for manual testing

// Test configuration
#define FILTER_SETTLE_TIME_MS 100   // Time for filter to mechanically settle
#define TEST_CYCLE_INTERVAL_MS 5000 // Interval between automatic tests
#define IMAGE_COMPARISON_DELAY_MS 500 // Delay between comparison images

// Global objects
EnhancedWildlifeCamera camera;

// Test state
struct TestState {
  bool auto_test_running;
  bool camera_initialized;
  bool sd_card_available;
  uint32_t test_cycle_count;
  uint32_t filter_switch_count;
  unsigned long last_auto_test;
  IRCutState current_ir_state;
} test_state;

void setup() {
  Serial.begin(115200);
  delay(2000); // Allow serial monitor to connect
  
  ESP_LOGI("IR_CUT_TEST", "=================================");
  ESP_LOGI("IR_CUT_TEST", "IR Cut Filter Test Utility v1.0");
  ESP_LOGI("IR_CUT_TEST", "LilyGO T-Camera Plus S3");
  ESP_LOGI("IR_CUT_TEST", "=================================");
  
  // Initialize test state
  memset(&test_state, 0, sizeof(test_state));
  test_state.current_ir_state = IR_CUT_ENABLED;
  
  // Initialize hardware
  if (!initializeHardware()) {
    ESP_LOGE("IR_CUT_TEST", "Hardware initialization failed!");
    while(1) {
      blinkErrorPattern();
      delay(1000);
    }
  }
  
  // Initialize camera
  if (!camera.init(BOARD_LILYGO_T_CAMERA_PLUS)) {
    ESP_LOGE("IR_CUT_TEST", "Camera initialization failed!");
    test_state.camera_initialized = false;
  } else {
    test_state.camera_initialized = true;
    ESP_LOGI("IR_CUT_TEST", "Camera initialized successfully");
  }
  
  // Initialize SD card for image storage
  if (SD_MMC.begin("/sdcard", true, true)) {
    test_state.sd_card_available = true;
    ESP_LOGI("IR_CUT_TEST", "SD card available for test images");
    
    // Create test directory
    if (!SD_MMC.mkdir("/ir_cut_tests")) {
      ESP_LOGW("IR_CUT_TEST", "Test directory may already exist");
    }
  } else {
    ESP_LOGW("IR_CUT_TEST", "SD card not available - images will not be saved");
  }
  
  // Print test menu
  printTestMenu();
  
  // Start with IR Cut enabled (day mode)
  setIRCutFilter(IR_CUT_ENABLED);
  
  ESP_LOGI("IR_CUT_TEST", "IR Cut Filter Test ready!");
  digitalWrite(STATUS_LED_PIN, HIGH); // Solid LED indicates ready
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    handleSerialCommand();
  }
  
  // Handle button press for manual toggle
  static bool last_button_state = HIGH;
  bool current_button_state = digitalRead(TEST_BUTTON_PIN);
  
  if (last_button_state == HIGH && current_button_state == LOW) {
    delay(50); // Debounce
    if (digitalRead(TEST_BUTTON_PIN) == LOW) {
      toggleIRCutManual();
    }
  }
  last_button_state = current_button_state;
  
  // Automatic test cycle
  if (test_state.auto_test_running) {
    runAutomaticTest();
  }
  
  delay(10);
}

bool initializeHardware() {
  ESP_LOGI("INIT", "Initializing hardware...");
  
  // Configure status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
  
  // Configure test button
  pinMode(TEST_BUTTON_PIN, INPUT_PULLUP);
  
  // Configure IR Cut control pin
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << IR_CUT_PIN);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  
  esp_err_t ret = gpio_config(&io_conf);
  if (ret != ESP_OK) {
    ESP_LOGE("INIT", "Failed to configure IR Cut GPIO: %s", esp_err_to_name(ret));
    return false;
  }
  
  ESP_LOGI("INIT", "Hardware initialized successfully");
  return true;
}

void setIRCutFilter(IRCutState state) {
  ESP_LOGI("IR_CUT", "Setting IR Cut filter: %s", 
           state == IR_CUT_ENABLED ? "ENABLED (Day Mode)" : "DISABLED (Night Mode)");
  
  // Measure response time
  unsigned long start_time = micros();
  
  // Set GPIO state
  gpio_set_level((gpio_num_t)IR_CUT_PIN, state == IR_CUT_ENABLED ? 1 : 0);
  
  // Update camera object if available
  if (test_state.camera_initialized) {
    camera.setIRCutFilter(state);
  }
  
  // Wait for mechanical settling
  delay(FILTER_SETTLE_TIME_MS);
  
  unsigned long response_time = micros() - start_time;
  
  test_state.current_ir_state = state;
  test_state.filter_switch_count++;
  
  ESP_LOGI("IR_CUT", "Filter switched in %lu microseconds", response_time);
  
  // Provide visual feedback
  if (state == IR_CUT_ENABLED) {
    // Quick double blink for day mode
    blinkStatusLED(2, 100);
  } else {
    // Single long blink for night mode
    blinkStatusLED(1, 300);
  }
}

void toggleIRCutManual() {
  IRCutState new_state = (test_state.current_ir_state == IR_CUT_ENABLED) ? 
                         IR_CUT_DISABLED : IR_CUT_ENABLED;
  
  ESP_LOGI("MANUAL", "Manual IR Cut toggle requested");
  setIRCutFilter(new_state);
  
  // Capture comparison image if camera is available
  if (test_state.camera_initialized) {
    captureTestImage("manual_toggle");
  }
}

void runAutomaticTest() {
  unsigned long current_time = millis();
  
  if (current_time - test_state.last_auto_test >= TEST_CYCLE_INTERVAL_MS) {
    test_state.last_auto_test = current_time;
    test_state.test_cycle_count++;
    
    ESP_LOGI("AUTO_TEST", "=== Automatic Test Cycle #%d ===", test_state.test_cycle_count);
    
    // Toggle IR Cut filter
    IRCutState new_state = (test_state.current_ir_state == IR_CUT_ENABLED) ? 
                           IR_CUT_DISABLED : IR_CUT_ENABLED;
    setIRCutFilter(new_state);
    
    // Capture test image
    if (test_state.camera_initialized) {
      char test_name[64];
      snprintf(test_name, sizeof(test_name), "auto_cycle_%d_%s", 
               test_state.test_cycle_count,
               new_state == IR_CUT_ENABLED ? "day" : "night");
      captureTestImage(test_name);
    }
    
    // Print test statistics
    printTestStatistics();
  }
}

bool captureTestImage(const char* test_name) {
  if (!test_state.camera_initialized) {
    ESP_LOGW("CAPTURE", "Camera not available for image capture");
    return false;
  }
  
  ESP_LOGI("CAPTURE", "Capturing test image: %s", test_name);
  
  // Wait for any mechanical settling
  delay(IMAGE_COMPARISON_DELAY_MS);
  
  // Capture image
  camera_fb_t* fb = camera.captureImage();
  if (!fb) {
    ESP_LOGE("CAPTURE", "Failed to capture image");
    return false;
  }
  
  ESP_LOGI("CAPTURE", "Image captured: %dx%d, %u bytes", fb->width, fb->height, fb->len);
  
  // Save to SD card if available
  if (test_state.sd_card_available) {
    char filename[128];
    snprintf(filename, sizeof(filename), "/ir_cut_tests/%s_%lu.jpg", test_name, millis());
    
    File file = SD_MMC.open(filename, FILE_WRITE);
    if (file) {
      file.write(fb->buf, fb->len);
      file.close();
      ESP_LOGI("CAPTURE", "Image saved: %s", filename);
      
      // Save metadata
      saveImageMetadata(filename, fb);
    } else {
      ESP_LOGE("CAPTURE", "Failed to save image to SD card");
    }
  }
  
  // Analyze image (basic brightness calculation)
  analyzeImageBrightness(fb);
  
  esp_camera_fb_return(fb);
  return true;
}

void saveImageMetadata(const char* filename, camera_fb_t* fb) {
  char meta_filename[128];
  snprintf(meta_filename, sizeof(meta_filename), "%s.meta", filename);
  
  File meta_file = SD_MMC.open(meta_filename, FILE_WRITE);
  if (meta_file) {
    meta_file.printf("{\n");
    meta_file.printf("  \"filename\": \"%s\",\n", filename);
    meta_file.printf("  \"timestamp\": %lu,\n", millis());
    meta_file.printf("  \"ir_cut_state\": \"%s\",\n", 
                     test_state.current_ir_state == IR_CUT_ENABLED ? "enabled" : "disabled");
    meta_file.printf("  \"width\": %d,\n", fb->width);
    meta_file.printf("  \"height\": %d,\n", fb->height);
    meta_file.printf("  \"file_size\": %u,\n", fb->len);
    meta_file.printf("  \"pixel_format\": \"%s\",\n", 
                     fb->format == PIXFORMAT_JPEG ? "JPEG" : "RAW");
    meta_file.printf("  \"test_cycle\": %d,\n", test_state.test_cycle_count);
    meta_file.printf("  \"filter_switches\": %d\n", test_state.filter_switch_count);
    meta_file.printf("}\n");
    meta_file.close();
  }
}

void analyzeImageBrightness(camera_fb_t* fb) {
  if (fb->format != PIXFORMAT_JPEG) {
    // Calculate average brightness for RAW images
    uint32_t total_brightness = 0;
    uint32_t pixel_count = fb->width * fb->height;
    
    for (uint32_t i = 0; i < fb->len && i < pixel_count; i++) {
      total_brightness += fb->buf[i];
    }
    
    float avg_brightness = (float)total_brightness / pixel_count;
    ESP_LOGI("ANALYSIS", "Average brightness: %.2f (IR Cut %s)", 
             avg_brightness, 
             test_state.current_ir_state == IR_CUT_ENABLED ? "enabled" : "disabled");
  } else {
    ESP_LOGI("ANALYSIS", "JPEG format - brightness analysis skipped");
  }
}

void handleSerialCommand() {
  String command = Serial.readStringUntil('\n');
  command.trim();
  command.toLowerCase();
  
  ESP_LOGI("COMMAND", "Received: %s", command.c_str());
  
  if (command == "help" || command == "h") {
    printTestMenu();
  }
  else if (command == "enable" || command == "e") {
    setIRCutFilter(IR_CUT_ENABLED);
  }
  else if (command == "disable" || command == "d") {
    setIRCutFilter(IR_CUT_DISABLED);
  }
  else if (command == "toggle" || command == "t") {
    toggleIRCutManual();
  }
  else if (command == "capture" || command == "c") {
    if (test_state.camera_initialized) {
      captureTestImage("manual_capture");
    } else {
      ESP_LOGW("COMMAND", "Camera not available");
    }
  }
  else if (command == "compare" || command == "comp") {
    runComparisonTest();
  }
  else if (command == "auto" || command == "a") {
    test_state.auto_test_running = !test_state.auto_test_running;
    ESP_LOGI("COMMAND", "Automatic testing %s", 
             test_state.auto_test_running ? "STARTED" : "STOPPED");
  }
  else if (command == "status" || command == "s") {
    printTestStatistics();
  }
  else if (command == "reset" || command == "r") {
    resetTestStatistics();
  }
  else if (command == "quick") {
    runQuickTest();
  }
  else {
    ESP_LOGW("COMMAND", "Unknown command: %s", command.c_str());
    ESP_LOGI("COMMAND", "Type 'help' for available commands");
  }
}

void runComparisonTest() {
  ESP_LOGI("COMPARISON", "Running day/night comparison test...");
  
  if (!test_state.camera_initialized) {
    ESP_LOGW("COMPARISON", "Camera not available for comparison test");
    return;
  }
  
  // Capture day mode image
  setIRCutFilter(IR_CUT_ENABLED);
  delay(500); // Allow time for settling and exposure adjustment
  captureTestImage("comparison_day");
  
  // Capture night mode image
  setIRCutFilter(IR_CUT_DISABLED);
  delay(500); // Allow time for settling and exposure adjustment
  captureTestImage("comparison_night");
  
  ESP_LOGI("COMPARISON", "Day/night comparison test completed");
}

void runQuickTest() {
  ESP_LOGI("QUICK", "Running quick IR Cut response test...");
  
  unsigned long start_time = millis();
  
  // Rapid switching test
  for (int i = 0; i < 5; i++) {
    setIRCutFilter(IR_CUT_ENABLED);
    delay(200);
    setIRCutFilter(IR_CUT_DISABLED);
    delay(200);
  }
  
  // Return to day mode
  setIRCutFilter(IR_CUT_ENABLED);
  
  unsigned long total_time = millis() - start_time;
  ESP_LOGI("QUICK", "Quick test completed in %lu ms", total_time);
}

void printTestMenu() {
  Serial.println();
  Serial.println("=== IR Cut Filter Test Commands ===");
  Serial.println("h, help      - Show this menu");
  Serial.println("e, enable    - Enable IR Cut filter (day mode)");
  Serial.println("d, disable   - Disable IR Cut filter (night mode)");
  Serial.println("t, toggle    - Toggle IR Cut filter state");
  Serial.println("c, capture   - Capture test image");
  Serial.println("comp, compare - Run day/night comparison test");
  Serial.println("a, auto      - Start/stop automatic testing");
  Serial.println("s, status    - Show test statistics");
  Serial.println("r, reset     - Reset test counters");
  Serial.println("quick        - Run quick response test");
  Serial.println();
  Serial.println("Hardware Controls:");
  Serial.println("Boot button  - Manual IR Cut toggle");
  Serial.println("Status LED   - Visual feedback");
  Serial.println();
}

void printTestStatistics() {
  Serial.println();
  Serial.println("=== Test Statistics ===");
  Serial.printf("Camera initialized: %s\n", test_state.camera_initialized ? "Yes" : "No");
  Serial.printf("SD card available: %s\n", test_state.sd_card_available ? "Yes" : "No");
  Serial.printf("Current IR Cut state: %s\n", 
                test_state.current_ir_state == IR_CUT_ENABLED ? "Enabled (Day)" : "Disabled (Night)");
  Serial.printf("Test cycles completed: %d\n", test_state.test_cycle_count);
  Serial.printf("Filter switches: %d\n", test_state.filter_switch_count);
  Serial.printf("Auto testing: %s\n", test_state.auto_test_running ? "Running" : "Stopped");
  Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
  
  if (test_state.camera_initialized) {
    CameraStatus cam_status = camera.getStatus();
    Serial.printf("Camera sensor: %s\n", camera.getSensorName());
    Serial.printf("Total captures: %d\n", cam_status.total_captures);
    Serial.printf("Failed captures: %d\n", cam_status.failed_captures);
  }
  Serial.println();
}

void resetTestStatistics() {
  test_state.test_cycle_count = 0;
  test_state.filter_switch_count = 0;
  test_state.last_auto_test = 0;
  
  ESP_LOGI("RESET", "Test statistics reset");
}

void blinkStatusLED(int count, int duration_ms) {
  for (int i = 0; i < count; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(duration_ms);
    digitalWrite(STATUS_LED_PIN, LOW);
    if (i < count - 1) {
      delay(duration_ms / 2);
    }
  }
}

void blinkErrorPattern() {
  // SOS pattern for critical errors
  // ... --- ...
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
  }
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(300);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
  }
  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(100);
  }
}