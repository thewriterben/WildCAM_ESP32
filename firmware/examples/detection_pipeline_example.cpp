/**
 * @file detection_pipeline_example.cpp
 * @brief Example demonstrating the wildlife detection pipeline
 * 
 * This example shows how the detection pipeline automatically processes
 * wildlife detections and saves images with metadata.
 */

#include <Arduino.h>

// The detection pipeline is integrated into the main firmware
// This example demonstrates the flow and how to verify it's working

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=================================================");
    Serial.println("Wildlife Detection Pipeline Example");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("The detection pipeline is integrated into the");
    Serial.println("main firmware and runs automatically when:");
    Serial.println("  1. AI system is initialized");
    Serial.println("  2. Camera captures frames");
    Serial.println("  3. Wildlife is detected by YOLO");
    Serial.println();
    
    Serial.println("What happens on each detection:");
    Serial.println("  Step 1: Generate unique filename");
    Serial.println("          Format: YYYYMMDD_HHMMSS_species.jpg");
    Serial.println("          Example: 20251016_143052_deer.jpg");
    Serial.println();
    
    Serial.println("  Step 2: Save image to SD card");
    Serial.println("          Path: /wildlife/images/");
    Serial.println("          Uses: StorageManager class");
    Serial.println();
    
    Serial.println("  Step 3: Log the operation");
    Serial.println("          Format: Image saved: filename (size bytes)");
    Serial.println();
    
    Serial.println("  Step 4: Handle errors gracefully");
    Serial.println("          Logs errors but continues detection");
    Serial.println();
    
    Serial.println("  Step 5: Save metadata (optional)");
    Serial.println("          Format: JSON with detection details");
    Serial.println("          Fields: filename, species, confidence,");
    Serial.println("                  timestamp, bounding_box, class_id");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println("Detection Pipeline Configuration");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("Storage Paths:");
    Serial.println("  Images:   /wildlife/images/");
    Serial.println("  Metadata: Same directory as images");
    Serial.println();
    
    Serial.println("Filename Format:");
    Serial.println("  With RTC:  YYYYMMDD_HHMMSS_species.jpg");
    Serial.println("  Fallback:  timestamp_species.jpg");
    Serial.println();
    
    Serial.println("Metadata Format (JSON):");
    Serial.println("  {");
    Serial.println("    \"filename\": \"20251016_143052_deer.jpg\",");
    Serial.println("    \"species\": \"deer\",");
    Serial.println("    \"confidence\": 0.895,");
    Serial.println("    \"timestamp\": 3456789,");
    Serial.println("    \"bounding_box\": {");
    Serial.println("      \"x\": 0.450,");
    Serial.println("      \"y\": 0.320,");
    Serial.println("      \"width\": 0.280,");
    Serial.println("      \"height\": 0.450");
    Serial.println("    },");
    Serial.println("    \"class_id\": 1");
    Serial.println("  }");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println("Monitoring Detection Events");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("Watch the serial monitor for these messages:");
    Serial.println();
    Serial.println("  [INFO] Wildlife detected: deer (confidence: 0.89)");
    Serial.println("  [INFO] Processing wildlife detection: 20251016_143052_deer.jpg");
    Serial.println("  [INFO] Image saved successfully: 20251016_143052_deer.jpg (size: 15234 bytes)");
    Serial.println("  [INFO] Metadata saved: 20251016_143052_deer.json");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println("Error Handling Examples");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("If storage is unavailable:");
    Serial.println("  [ERROR] Storage not ready, cannot save detection image");
    Serial.println("  [INFO]  (continuing detection loop)");
    Serial.println();
    
    Serial.println("If image save fails:");
    Serial.println("  [ERROR] Failed to save image: SD card write error");
    Serial.println("  [INFO]  (continuing detection loop)");
    Serial.println();
    
    Serial.println("If metadata save fails:");
    Serial.println("  [WARN]  Failed to save metadata for 20251016_143052_deer.jpg");
    Serial.println("  [INFO]  (continuing operation - metadata is optional)");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println("Testing the Pipeline");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("To test the detection pipeline:");
    Serial.println();
    Serial.println("1. Hardware Setup:");
    Serial.println("   - Insert formatted SD card (FAT32)");
    Serial.println("   - Ensure camera is connected");
    Serial.println("   - Power on the system");
    Serial.println();
    
    Serial.println("2. Verify Initialization:");
    Serial.println("   - Check for 'Storage system initialized' message");
    Serial.println("   - Check for 'AI detection system ready' message");
    Serial.println("   - Check for 'Camera system initialized' message");
    Serial.println();
    
    Serial.println("3. Trigger Detection:");
    Serial.println("   - Place wildlife in camera view");
    Serial.println("   - OR use test images if available");
    Serial.println("   - Watch serial monitor for detection messages");
    Serial.println();
    
    Serial.println("4. Verify Results:");
    Serial.println("   - Check SD card for /wildlife/images/ directory");
    Serial.println("   - Look for .jpg and .json files");
    Serial.println("   - Verify timestamps in filenames");
    Serial.println();
    
    Serial.println("5. Run Unit Tests:");
    Serial.println("   - Command: pio test -e test");
    Serial.println("   - Tests verify all pipeline functions");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println("Performance Characteristics");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("Processing Time per Detection:");
    Serial.println("  - Filename generation: ~1ms");
    Serial.println("  - Image save: ~30-80ms (depends on size)");
    Serial.println("  - Metadata save: ~5-10ms");
    Serial.println("  - Total: ~50-100ms per detection");
    Serial.println();
    
    Serial.println("Storage Requirements:");
    Serial.println("  - Image size: ~15-30KB (JPEG, 320x240)");
    Serial.println("  - Metadata size: ~200 bytes (JSON)");
    Serial.println("  - Hourly rate: ~100MB @ 10 detections/hour");
    Serial.println();
    
    Serial.println("Memory Usage:");
    Serial.println("  - Filename buffer: 64 bytes");
    Serial.println("  - Metadata buffer: 512 bytes");
    Serial.println("  - Minimal heap impact during processing");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println("Troubleshooting Guide");
    Serial.println("=================================================");
    Serial.println();
    
    Serial.println("Problem: No images saving");
    Serial.println("Solutions:");
    Serial.println("  1. Check SD card is inserted and formatted");
    Serial.println("  2. Verify 'Storage system initialized' message");
    Serial.println("  3. Check SD card has free space");
    Serial.println("  4. Try reformatting SD card (FAT32)");
    Serial.println();
    
    Serial.println("Problem: Timestamps are wrong");
    Serial.println("Solutions:");
    Serial.println("  1. Configure NTP server in WiFi settings");
    Serial.println("  2. Install and configure RTC module");
    Serial.println("  3. System will use millis() as fallback");
    Serial.println();
    
    Serial.println("Problem: Detection not triggering pipeline");
    Serial.println("Solutions:");
    Serial.println("  1. Verify AI system initialized");
    Serial.println("  2. Check detection confidence threshold");
    Serial.println("  3. Ensure wildlife in camera field of view");
    Serial.println("  4. Review AI model is loaded correctly");
    Serial.println();
    
    Serial.println("=================================================");
    Serial.println();
    Serial.println("For more information, see DETECTION_PIPELINE.md");
    Serial.println();
    Serial.println("=================================================");
}

void loop() {
    // Main firmware handles detection pipeline automatically
    // This example is for documentation purposes only
    delay(1000);
}
