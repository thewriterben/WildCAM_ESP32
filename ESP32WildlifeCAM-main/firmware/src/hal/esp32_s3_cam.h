/**
 * @file esp32_s3_cam.h
 * @brief ESP32-S3-CAM board implementation
 */

#ifndef ESP32_S3_CAM_H
#define ESP32_S3_CAM_H

#include "camera_board.h"

class ESP32S3CAM : public CameraBoard {
public:
    ESP32S3CAM();
    virtual ~ESP32S3CAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_ESP32_S3_CAM; }
    const char* getBoardName() const override { return "ESP32-S3-CAM"; }
    bool isSupported() const override { return true; }
    
    // Hardware initialization
    bool init() override;
    bool detectSensor() override;
    SensorType getSensorType() const override { return sensor_type; }
    
    // Configuration
    GPIOMap getGPIOMap() const override;
    CameraConfig getCameraConfig() const override;
    PowerProfile getPowerProfile() const override;
    DisplayProfile getDisplayProfile() const override;
    
    // Camera operations
    bool configureSensor(sensor_t* sensor) override;
    bool setupPins() override;
    void flashLED(bool state) override;
    
    // Power management
    void enterSleepMode() override;
    void exitSleepMode() override;
    float getBatteryVoltage() override;
    
    // Chip information
    bool hasPSRAM() const override;
    uint32_t getChipId() const override;
    const char* getChipModel() const override;

private:
    /**
     * Configure sensor settings optimized for ESP32-S3 capabilities
     * @param sensor Camera sensor pointer
     */
    void configureS3OptimizedSettings(sensor_t* sensor);
    
    /**
     * Test camera sensor communication with multiple sensor types
     * @return True if sensor responds correctly
     */
    bool testSensorCommunication();
    
    /**
     * Setup GPIO pins for ESP32-S3 camera operation
     */
    void setupCameraPins();
    
    /**
     * Configure ESP32-S3 specific power management
     */
    void setupPowerManagement();
    
    /**
     * Detect specific sensor type (OV2640, OV5640, etc.)
     */
    SensorType detectSpecificSensor();
};

#endif // ESP32_S3_CAM_H