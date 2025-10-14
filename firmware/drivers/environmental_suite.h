/**
 * @file environmental_suite.h
 * @brief Environmental sensors interface
 */

#ifndef ENVIRONMENTAL_SUITE_H
#define ENVIRONMENTAL_SUITE_H

#include <Arduino.h>

/**
 * @class EnvironmentalSuite
 * @brief Environmental sensors management
 */
class EnvironmentalSuite {
public:
    EnvironmentalSuite() {}
    ~EnvironmentalSuite() {}
    
    bool begin() {
        // Initialize sensors
        return true;
    }
    
    float getTemperature() {
        return 25.0f;
    }
};

#endif // ENVIRONMENTAL_SUITE_H
