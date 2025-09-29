/**
 * @file ttgo_t_camera.h
 * @brief TTGO T-Camera board implementation
 */

#ifndef TTGO_T_CAMERA_H
#define TTGO_T_CAMERA_H

#include "camera_board.h"

class TTGOTCAMERA : public CameraBoard {
public:
    TTGOTCAMERA();
    virtual ~TTGOTCAMERA() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_TTGO_T_CAMERA; }
    const char* getBoardName() const override { return "TTGO T-Camera"; }
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
    void setupDisplayInterface();
};

#endif // TTGO_T_CAMERA_H