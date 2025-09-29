/**
 * @file m5stack_timer_cam.h
 * @brief M5Stack Timer Camera board implementation
 */

#ifndef M5STACK_TIMER_CAM_H
#define M5STACK_TIMER_CAM_H

#include "camera_board.h"

class M5StackTimerCAM : public CameraBoard {
public:
    M5StackTimerCAM();
    virtual ~M5StackTimerCAM() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_M5STACK_TIMER_CAM; }
    const char* getBoardName() const override { return "M5Stack Timer Camera"; }
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
    /**
     * Configure sensor settings for outdoor wildlife photography
     * @param sensor Camera sensor pointer
     */
    void configureWildlifeSettings(sensor_t* sensor);
    
    /**
     * Test camera sensor communication
     * @return True if sensor responds correctly
     */
    bool testSensorCommunication();
    
    /**
     * Setup GPIO pins for camera operation
     */
    void setupCameraPins();
    
    /**
     * Configure power management pins
     */
    void setupPowerPins();
    
    /**
     * Configure battery management for extended field deployment
     */
    void setupBatteryManagement();
};

#endif // M5STACK_TIMER_CAM_H