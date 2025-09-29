/**
 * @file espressif_esp32_cam_mb.h
 * @brief Espressif ESP32-CAM-MB board implementation
 */

#ifndef ESPRESSIF_ESP32_CAM_MB_H
#define ESPRESSIF_ESP32_CAM_MB_H

#include "camera_board.h"

class ESPRESSIF_ESP32CAMMB : public CameraBoard {
public:
    ESPRESSIF_ESP32CAMMB();
    virtual ~ESPRESSIF_ESP32CAMMB() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_ESPRESSIF_ESP32_CAM_MB; }
    const char* getBoardName() const override { return "Espressif ESP32-CAM-MB"; }
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
    void setupOfficialFeatures();
};

#endif // ESPRESSIF_ESP32_CAM_MB_H