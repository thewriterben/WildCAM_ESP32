/**
 * @file firebeetle_esp32_cam.h
 * @brief DFRobot FireBeetle ESP32-CAM board implementation
 */

#ifndef FIREBEETLE_ESP32_CAM_H
#define FIREBEETLE_ESP32_CAM_H

#include "camera_board.h"

class FIREBEETLEESP32CAM : public CameraBoard {
public:
    FIREBEETLEESP32CAM();
    virtual ~FIREBEETLEESP32CAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_FIREBEETLE_ESP32_CAM; }
    const char* getBoardName() const override { return "DFRobot FireBeetle ESP32-CAM"; }
    bool isSupported() const override { return true; }
    
    // Hardware initialization
    bool init() override;
    bool detectSensor() override;
    SensorType getSensorType() const override { return sensor_type; }
    
    // Configuration
    GPIOMap getGPIOMap() const override;
    CameraConfig getCameraConfig() const override;
    PowerProfile getPowerProfile() const override;
    
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
    void setupLowPowerMode();
};

#endif // FIREBEETLE_ESP32_CAM_H