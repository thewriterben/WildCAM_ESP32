/**
 * @file esp32_s3_eye.h
 * @brief ESP32-S3-EYE board implementation
 */

#ifndef ESP32_S3_EYE_H
#define ESP32_S3_EYE_H

#include "camera_board.h"

class ESP32S3EYE : public CameraBoard {
public:
    ESP32S3EYE();
    virtual ~ESP32S3EYE() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_ESP32_S3_EYE; }
    const char* getBoardName() const override { return "ESP32-S3-EYE"; }
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
    void setupAIAcceleration();
};

#endif // ESP32_S3_EYE_H