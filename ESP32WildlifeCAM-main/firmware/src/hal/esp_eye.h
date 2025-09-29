/**
 * @file esp_eye.h
 * @brief ESP-EYE board implementation
 */

#ifndef ESP_EYE_H
#define ESP_EYE_H

#include "camera_board.h"

class ESPEYE : public CameraBoard {
public:
    ESPEYE();
    virtual ~ESPEYE() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_ESP_EYE; }
    const char* getBoardName() const override { return "ESP-EYE"; }
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
     * Configure sensor settings optimized for ESP-EYE capabilities
     * @param sensor Camera sensor pointer
     */
    void configureEspEyeSettings(sensor_t* sensor);
    
    /**
     * Test camera sensor communication
     * @return True if sensor responds correctly
     */
    bool testSensorCommunication();
    
    /**
     * Setup GPIO pins for ESP-EYE camera operation
     */
    void setupCameraPins();
    
    /**
     * Configure ESP-EYE specific power management
     */
    void setupPowerManagement();
    
    /**
     * Detect specific sensor type for ESP-EYE
     */
    SensorType detectSpecificSensor();
};

#endif // ESP_EYE_H