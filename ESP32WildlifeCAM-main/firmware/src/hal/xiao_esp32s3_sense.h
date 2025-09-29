/**
 * @file xiao_esp32s3_sense.h
 * @brief Seeed Studio XIAO ESP32S3 Sense board implementation
 */

#ifndef XIAO_ESP32S3_SENSE_H
#define XIAO_ESP32S3_SENSE_H

#include "camera_board.h"

class XIAOESP32S3SENSE : public CameraBoard {
public:
    XIAOESP32S3SENSE();
    virtual ~XIAOESP32S3SENSE() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_XIAO_ESP32S3_SENSE; }
    const char* getBoardName() const override { return "Seeed Studio XIAO ESP32S3 Sense"; }
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
    void setupMicrophone();
};

#endif // XIAO_ESP32S3_SENSE_H