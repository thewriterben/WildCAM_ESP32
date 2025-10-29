#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>

class PowerManager {
private:
    int batteryPin;
    float batteryVoltage;
    bool lowPowerMode;
    const float BATTERY_MIN = 3.3;
    const float BATTERY_MAX = 4.2;

public:
    PowerManager();
    
    bool init(int batteryMonitorPin = 35);
    float getBatteryVoltage();
    int getBatteryPercentage();
    bool isLowBattery();
    void enterDeepSleep(uint64_t sleepTimeSeconds);
    void configureWakeOnMotion(int pirPin);
    void configureWakeOnTimer(uint64_t seconds);
    void printPowerStatus();
};

#endif // POWER_MANAGER_H
