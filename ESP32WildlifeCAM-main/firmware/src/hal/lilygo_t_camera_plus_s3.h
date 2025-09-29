/**
 * @file lilygo_t_camera_plus_s3.h
 * @brief LilyGO T-Camera Plus S3 OV5640 V1.1 board implementation
 */

#ifndef LILYGO_T_CAMERA_PLUS_S3_H
#define LILYGO_T_CAMERA_PLUS_S3_H

#include "camera_board.h"

class LILYGOTCAMERAPLASS3 : public CameraBoard {
public:
    LILYGOTCAMERAPLASS3();
    virtual ~LILYGOTCAMERAPLASS3() = default;
    
    // Board identification
    BoardType getBoardType() const override { return BOARD_LILYGO_T_CAMERA_PLUS_S3; }
    const char* getBoardName() const override { return "LilyGO T-Camera Plus S3 OV5640 V1.1"; }
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
    void configureOV5640Settings(sensor_t* sensor);
    bool testOV5640Communication();
    void setupCameraPins();
    void setupPowerPins();
    void setupDisplayPins();
    void setupS3Features();
    bool validateS3Hardware();
};

#endif // LILYGO_T_CAMERA_PLUS_S3_H