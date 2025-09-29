/**
 * @file esp32_cam.h
 * @brief AI-Thinker ESP32-CAM board implementation
 */

#ifndef ESP32_CAM_H
#define ESP32_CAM_H

#include "camera_board.h"

class ESP32CAM : public CameraBoard {
public:
    ESP32CAM();
    virtual ~ESP32CAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_AI_THINKER_ESP32_CAM; }
    const char* getBoardName() const override { return "AI-Thinker ESP32-CAM"; }
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
     * Configure sensor settings for outdoor wildlife photography
     * @param sensor Camera sensor pointer
     */
    void configureWildlifeSettings(sensor_t* sensor);
    
    /**
     * Test camera sensor communication
     * @return True if sensor responds correctly
     */
    bool testSensorCommunication();
    
    /**
     * Setup GPIO pins for camera operation
     */
    void setupCameraPins();
    
    /**
     * Configure power management pins
     */
    void setupPowerPins();
};

#endif // ESP32_CAM_H