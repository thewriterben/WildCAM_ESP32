/**
 * @file freenove_esp32_wrover_cam.h
 * @brief Freenove ESP32-WROVER-CAM board implementation
 */

#ifndef FREENOVE_ESP32_WROVER_CAM_H
#define FREENOVE_ESP32_WROVER_CAM_H

#include "camera_board.h"

class FREENOVESP32WROVERCAM : public CameraBoard {
public:
    FREENOVESP32WROVERCAM();
    virtual ~FREENOVESP32WROVERCAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_FREENOVE_ESP32_WROVER_CAM; }
    const char* getBoardName() const override { return "Freenove ESP32-WROVER-CAM"; }
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
    void setupPSRAMOptimizations();
};

#endif // FREENOVE_ESP32_WROVER_CAM_H