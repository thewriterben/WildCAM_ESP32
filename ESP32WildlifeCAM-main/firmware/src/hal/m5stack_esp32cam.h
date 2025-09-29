/**
 * @file m5stack_esp32cam.h
 * @brief M5Stack ESP32CAM board implementation
 */

#ifndef M5STACK_ESP32CAM_H
#define M5STACK_ESP32CAM_H

#include "camera_board.h"

class M5STACKESP32CAM : public CameraBoard {
public:
    M5STACKESP32CAM();
    virtual ~M5STACKESP32CAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_M5STACK_ESP32CAM; }
    const char* getBoardName() const override { return "M5Stack ESP32CAM"; }
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
    void setupStackableInterface();
};

#endif // M5STACK_ESP32CAM_H