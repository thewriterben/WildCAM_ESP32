/**
 * @file lilygo_t_camera_plus.h
 * @brief LilyGO T-Camera Plus board implementation
 */

#ifndef LILYGO_T_CAMERA_PLUS_H
#define LILYGO_T_CAMERA_PLUS_H

#include "camera_board.h"

class LILYGOTCAMERAPLUS : public CameraBoard {
public:
    LILYGOTCAMERAPLUS();
    virtual ~LILYGOTCAMERAPLUS() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_LILYGO_T_CAMERA_PLUS; }
    const char* getBoardName() const override { return "LilyGO T-Camera Plus"; }
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
    void configureWildlifeSettings(sensor_t* sensor);
    bool testSensorCommunication();
    void setupCameraPins();
    void setupPowerPins();
    void setupEnhancedFeatures();
};

#endif // LILYGO_T_CAMERA_PLUS_H