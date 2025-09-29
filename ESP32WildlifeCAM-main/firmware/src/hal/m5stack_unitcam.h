/**
 * @file m5stack_unitcam.h
 * @brief M5Stack UnitCAM board implementation
 */

#ifndef M5STACK_UNITCAM_H
#define M5STACK_UNITCAM_H

#include "camera_board.h"

class M5STACKUNITCAM : public CameraBoard {
public:
    M5STACKUNITCAM();
    virtual ~M5STACKUNITCAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_M5STACK_UNITCAM; }
    const char* getBoardName() const override { return "M5Stack UnitCAM"; }
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
    void setupTinyFormFactor();
};

#endif // M5STACK_UNITCAM_H