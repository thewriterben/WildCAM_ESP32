/**
 * @file system_manager.h
 * @brief Main system coordination and management
 * @author thewriterben
 * @date 2025-08-31
 */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <memory>
#include "../hardware/board_detector.h"
#include "../firmware/include/power/power_manager.h"
#include "../src/detection/motion_coordinator.h"

/**
 * @brief Main system manager class that coordinates all subsystems
 * 
 * Handles initialization, configuration, and runtime management of:
 * - Hardware detection and configuration
 * - Camera subsystem
 * - Storage management (SD card, LittleFS)
 * - Power management
 * - Network connectivity (WiFi, LoRa when available)
 * - Sensor monitoring
 * - Error handling and safe mode
 */
class SystemManager {
public:
    enum SystemState {
        STATE_UNINITIALIZED,
        STATE_INITIALIZING,
        STATE_RUNNING,
        STATE_ERROR,
        STATE_SAFE_MODE
    };

    SystemManager(BoardDetector::BoardType board);
    ~SystemManager();
    
    bool initialize();
    void update();
    
    // System state management
    SystemState getState() const { return m_state; }
    bool isInitialized() const { return m_initialized; }
    
    // Component status
    bool isCameraReady() const { return m_cameraReady; }
    bool isStorageReady() const { return m_storageReady; }
    bool isNetworkReady() const { return m_networkReady; }
    
    // Safe mode and error handling
    void enterSafeMode();
    const char* getLastError() const { return m_lastError; }
    
private:
    BoardDetector::BoardType m_boardType;
    BoardDetector::PinConfig m_pinConfig;
    
    SystemState m_state;
    bool m_initialized;
    unsigned long m_lastUpdate;
    
    // Component status flags
    bool m_cameraReady;
    bool m_storageReady;
    bool m_networkReady;
    bool m_sensorsReady;
    
    // Enhanced motion detection
    std::unique_ptr<MotionCoordinator> m_motionCoordinator;
    MotionCoordinator::EnvironmentalConditions m_environmentalConditions;
    
    // Error tracking
    char m_lastError[128];
    int m_errorCount;
    
    // Initialization methods
    bool initializeHardware();
    bool initializeCamera();
    bool initializeStorage();
    bool initializeSensors();
    bool initializePowerManagement();
    bool initializeNetwork();
    bool initializeTasks();
    bool initializeMotionDetection();
    
    // Motion detection handlers
    void handleMotionDetected(const MotionCoordinator::CoordinatorResult& result);
    void updateEnvironmentalConditions();
    
    // Helper methods
    void setError(const char* error);
    bool validateHardwareConfiguration();
    void printSystemInfo();
    void performSystemHealthChecks();
    void updateSystemTelemetry();
    void checkPowerConditions();
    
    // FreeRTOS task functions
    static void systemMonitorTask(void* parameter);
    static void powerManagementTask(void* parameter);
    static void sensorMonitorTask(void* parameter);
    static void motionDetectionTask(void* parameter);
    static void networkCommTask(void* parameter);
};

#endif // SYSTEM_MANAGER_H