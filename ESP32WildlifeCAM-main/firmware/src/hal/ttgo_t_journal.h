/**
 * @file ttgo_t_journal.h
 * @brief TTGO T-Journal board implementation
 */

#ifndef TTGO_T_JOURNAL_H
#define TTGO_T_JOURNAL_H

#include "camera_board.h"

class TTGOTJOURNAL : public CameraBoard {
public:
    TTGOTJOURNAL();
    virtual ~TTGOTJOURNAL() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_TTGO_T_JOURNAL; }
    const char* getBoardName() const override { return "TTGO T-Journal"; }
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
    void setupEPaperDisplay();
};

#endif // TTGO_T_JOURNAL_H