/**
 * @file burst_video_example.cpp
 * @brief Example demonstrating burst capture, video recording, and time-lapse features
 * 
 * This example shows how to use the enhanced CameraManager with the new
 * burst capture, video recording, and time-lapse functionality.
 */

#include "../src/camera/camera_manager.h"

CameraManager camera;

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 Wildlife Camera - Burst & Video Example");
    
    // Initialize SD card
    if (!SD_MMC.begin()) {
        Serial.println("SD card initialization failed!");
        return;
    }
    
    // Initialize camera
    if (!camera.initialize()) {
        Serial.println("Camera initialization failed!");
        return;
    }
    
    Serial.println("Camera initialized successfully");
    Serial.println("Starting demonstration...");
    
    // Demonstrate burst capture
    demonstrateBurstCapture();
    
    // Demonstrate video recording
    demonstrateVideoRecording();
    
    // Demonstrate time-lapse
    demonstrateTimeLapse();
    
    // Demonstrate night vision features
    demonstrateNightVision();
}

void loop() {
    // In a real application, you would call camera.updateTimeLapse()
    // periodically if time-lapse is active
    
    if (camera.isTimeLapseActive()) {
        camera.updateTimeLapse();
    }
    
    // Auto-adjust night mode based on light conditions
    camera.autoAdjustNightMode();
    
    delay(1000);
}

void demonstrateBurstCapture() {
    Serial.println("\n=== Burst Capture Demonstration ===");
    
    // Configure burst settings
    CameraManager::BurstConfig burstConfig;
    burstConfig.count = 5;                // Capture 5 images
    burstConfig.intervalMs = 500;         // 500ms between captures
    burstConfig.quality = 10;             // High quality
    burstConfig.saveIndividual = true;    // Save each image separately
    burstConfig.createSequence = true;    // Create metadata file
    
    // Capture burst
    CameraManager::BurstResult result = camera.captureBurst(burstConfig, "/burst");
    
    if (result.success) {
        Serial.printf("Burst capture successful!\n");
        Serial.printf("- Sequence ID: %s\n", result.sequenceId.c_str());
        Serial.printf("- Images captured: %d/%d\n", result.capturedCount, burstConfig.count);
        Serial.printf("- Total time: %d ms\n", result.totalTime);
        Serial.printf("- Total size: %d bytes\n", result.totalSize);
        Serial.printf("- Files:\n");
        
        for (uint8_t i = 0; i < result.capturedCount; i++) {
            Serial.printf("  %d: %s\n", i + 1, result.filenames[i].c_str());
        }
    } else {
        Serial.println("Burst capture failed!");
    }
}

void demonstrateVideoRecording() {
    Serial.println("\n=== Video Recording Demonstration ===");
    
    // Configure video settings
    CameraManager::VideoConfig videoConfig;
    videoConfig.durationSeconds = 5;      // 5 second video
    videoConfig.frameRate = 10;           // 10 FPS
    videoConfig.quality = 12;             // Good quality
    videoConfig.frameSize = FRAMESIZE_VGA; // VGA resolution for video
    
    // Record video
    CameraManager::VideoResult result = camera.recordVideo(videoConfig, "/videos");
    
    if (result.success) {
        Serial.printf("Video recording successful!\n");
        Serial.printf("- Filename: %s\n", result.filename.c_str());
        Serial.printf("- Duration: %d seconds\n", result.duration);
        Serial.printf("- Frames captured: %d\n", result.frameCount);
        Serial.printf("- File size: %d bytes\n", result.fileSize);
        Serial.printf("- Recording time: %d ms\n", result.recordingTime);
    } else {
        Serial.println("Video recording failed!");
    }
}

void demonstrateTimeLapse() {
    Serial.println("\n=== Time-lapse Demonstration ===");
    
    // Configure time-lapse settings
    CameraManager::TimeLapseConfig timelapseConfig;
    timelapseConfig.intervalSeconds = 10;  // Capture every 10 seconds
    timelapseConfig.maxImages = 10;        // Maximum 10 images for demo
    timelapseConfig.autoCleanup = true;    // Enable cleanup
    timelapseConfig.folder = "/timelapse"; // Storage folder
    
    // Start time-lapse
    CameraManager::TimeLapseResult result = camera.startTimeLapse(timelapseConfig);
    
    if (result.success) {
        Serial.printf("Time-lapse started!\n");
        Serial.printf("- Sequence ID: %s\n", result.sequenceId.c_str());
        Serial.printf("- Interval: %d seconds\n", timelapseConfig.intervalSeconds);
        Serial.printf("- Maximum images: %d\n", timelapseConfig.maxImages);
        Serial.printf("- Folder: %s\n", result.folder.c_str());
        
        // In a real application, the time-lapse would continue in the main loop
        // For this demo, we'll simulate running for a short time
        Serial.println("Time-lapse will continue in main loop...");
        Serial.println("Call camera.stopTimeLapse() to stop manually");
    } else {
        Serial.println("Failed to start time-lapse!");
    }
}

void demonstrateNightVision() {
    Serial.println("\n=== Night Vision Demonstration ===");
    
    // Get current light level
    uint8_t lightLevel = camera.getLightLevel();
    Serial.printf("Current light level: %d%%\n", lightLevel);
    
    // Manual IR LED control
    Serial.println("Testing IR LED control...");
    camera.setIRLED(true);
    delay(2000);
    camera.setIRLED(false);
    
    // Demonstrate auto night mode
    Serial.println("Auto night mode adjustment...");
    camera.autoAdjustNightMode();
    
    // Show current camera configuration
    Serial.printf("Camera configuration: %s\n", camera.getConfiguration().c_str());
    
    // Display statistics
    CameraManager::CameraStats stats = camera.getStatistics();
    Serial.printf("Camera statistics:\n");
    Serial.printf("- Total captures: %d\n", stats.totalCaptures);
    Serial.printf("- Successful: %d\n", stats.successfulCaptures);
    Serial.printf("- Failed: %d\n", stats.failedCaptures);
    Serial.printf("- Success rate: %.1f%%\n", stats.successRate * 100);
    Serial.printf("- Average capture time: %d ms\n", stats.averageCaptureTime);
    Serial.printf("- Average image size: %d bytes\n", stats.averageImageSize);
}

// Additional utility functions for wildlife monitoring

void motionTriggeredBurst() {
    // Example of burst capture triggered by motion detection
    Serial.println("Motion detected - triggering burst capture");
    
    CameraManager::BurstConfig config;
    config.count = 3;
    config.intervalMs = 200; // Fast burst for wildlife
    config.createSequence = true;
    
    CameraManager::BurstResult result = camera.captureBurst(config, "/motion");
    
    if (result.success) {
        Serial.printf("Motion burst captured: %d images in %d ms\n", 
                     result.capturedCount, result.totalTime);
    }
}

void scheduleNightTimeRecording() {
    // Example of scheduling video recording for night time
    Serial.println("Scheduling night time video recording");
    
    // Enable night mode and IR
    camera.setNightMode(true);
    camera.setIRLED(true);
    
    // Configure for night recording
    CameraManager::VideoConfig config;
    config.durationSeconds = 10;
    config.frameRate = 5; // Lower frame rate for night
    config.quality = 15;  // Slightly lower quality for better low-light performance
    
    CameraManager::VideoResult result = camera.recordVideo(config, "/night_videos");
    
    if (result.success) {
        Serial.printf("Night video recorded: %s\n", result.filename.c_str());
    }
    
    // Disable IR after recording
    camera.setIRLED(false);
}