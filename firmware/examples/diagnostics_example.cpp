/**
 * @file diagnostics_example.cpp
 * @brief Example usage of system diagnostics module
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * 
 * This example demonstrates how to use the system diagnostics module
 * to verify hardware functionality at boot time.
 */

#include <Arduino.h>
#include "../src/diagnostics.h"
#include "../utils/logger.h"

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);
    
    Logger::begin();
    Logger::info("WildCAM ESP32 - Diagnostics Example");
    Logger::info("===================================");
    
    // Example 1: Run full system diagnostics
    Logger::info("\nExample 1: Running full system diagnostics");
    Logger::info("------------------------------------------");
    bool allSystemsOK = runSystemDiagnostics();
    
    if (allSystemsOK) {
        Logger::info("✓ All critical systems passed!");
        Logger::info("✓ System ready for field deployment");
    } else {
        Logger::error("✗ Critical system failure detected");
        Logger::error("✗ Review diagnostics.log for details");
        Logger::error("✗ System may not be ready for deployment");
    }
    
    delay(2000);
    
    // Example 2: Run individual diagnostic tests
    Logger::info("\nExample 2: Running individual tests");
    Logger::info("------------------------------------");
    
    // Test camera
    Logger::info("Testing camera...");
    if (testCameraInit()) {
        Logger::info("✓ Camera test passed");
    } else {
        Logger::error("✗ Camera test failed");
    }
    delay(500);
    
    // Test SD card
    Logger::info("Testing SD card...");
    if (testSDCardReadWrite()) {
        Logger::info("✓ SD card test passed");
    } else {
        Logger::error("✗ SD card test failed");
    }
    delay(500);
    
    // Test PIR sensor
    Logger::info("Testing PIR sensor...");
    if (testPIRSensor()) {
        Logger::info("✓ PIR sensor test passed");
    } else {
        Logger::error("✗ PIR sensor test failed");
    }
    delay(500);
    
    // Test power management
    Logger::info("Testing power management...");
    if (testPowerManagement()) {
        Logger::info("✓ Power management test passed");
    } else {
        Logger::error("✗ Power management test failed");
    }
    delay(500);
    
    // Test memory
    Logger::info("Testing memory...");
    if (testMemory()) {
        Logger::info("✓ Memory test passed");
    } else {
        Logger::error("✗ Memory test failed");
    }
    delay(500);
    
    // Example 3: Conditional system operation based on diagnostics
    Logger::info("\nExample 3: Conditional operation");
    Logger::info("----------------------------------");
    
    // Re-run diagnostics for demonstration
    bool systemReady = runSystemDiagnostics();
    
    if (systemReady) {
        Logger::info("✓ Starting normal operation mode");
        // Start camera monitoring, AI detection, etc.
        // createSystemTasks();
    } else {
        Logger::warning("⚠ Entering safe mode due to diagnostic failures");
        // Enter safe mode: reduced functionality, attempt recovery
        // enterSafeMode();
    }
    
    Logger::info("\n===================================");
    Logger::info("Diagnostics example complete");
    Logger::info("Check /diagnostics.log on SD card");
    Logger::info("===================================");
}

void loop() {
    // Example 4: Periodic diagnostics
    static unsigned long lastDiagnostics = 0;
    const unsigned long diagnosticsInterval = 3600000; // 1 hour
    
    if (millis() - lastDiagnostics >= diagnosticsInterval) {
        Logger::info("\nRunning periodic diagnostics...");
        bool systemHealthy = runSystemDiagnostics();
        
        if (!systemHealthy) {
            Logger::warning("Periodic diagnostics detected issues!");
            // Take corrective action: restart, safe mode, alert, etc.
        }
        
        lastDiagnostics = millis();
    }
    
    // Normal operation
    delay(1000);
}

/**
 * Example safe mode implementation
 */
void enterSafeMode() {
    Logger::warning("Entering SAFE MODE");
    Logger::info("Safe mode features:");
    Logger::info("- Reduced power consumption");
    Logger::info("- Basic camera capture only");
    Logger::info("- No AI processing");
    Logger::info("- Periodic diagnostic re-checks");
    
    // Disable non-critical features
    // Reduce camera resolution
    // Disable AI processing
    // Enable power saving
    // Schedule periodic diagnostics
}

/**
 * Example usage in production setup
 */
void productionSetup() {
    Serial.begin(115200);
    Logger::begin();
    
    Logger::info("Production Startup - Running Diagnostics");
    
    // Run diagnostics with timeout
    unsigned long diagnosticStart = millis();
    bool diagnosticsPass = runSystemDiagnostics();
    unsigned long diagnosticDuration = millis() - diagnosticStart;
    
    Logger::info("Diagnostics completed in %lu ms", diagnosticDuration);
    
    // Verify diagnostics completed in reasonable time
    if (diagnosticDuration > 5000) {
        Logger::warning("Diagnostics took longer than expected");
    }
    
    // Take action based on results
    if (diagnosticsPass) {
        // All systems OK - start normal operation
        Logger::info("All systems operational - starting normal operation");
        // Initialize AI, networking, tasks, etc.
    } else {
        // Critical failure - enter safe mode or alert
        Logger::error("Critical system failure - entering safe mode");
        enterSafeMode();
        
        // Optional: Send alert via network if available
        // if (networkAvailable()) {
        //     sendAlert("Critical system failure at boot");
        // }
    }
}
