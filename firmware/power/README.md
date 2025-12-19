# MPPT Solar Charging and Advanced Battery Management

## Overview

This module provides comprehensive solar power management for the WildCAM ESP32 wildlife camera system. It includes:

- **MPPT (Maximum Power Point Tracking)** solar charge controller
- **Advanced battery management** with multi-stage charging
- **Integrated solar power system** combining both for optimal operation

## Features

### MPPT Controller (`mppt_controller.h/cpp`)

The MPPT controller implements three tracking algorithms for efficient solar energy harvesting:

1. **Perturb & Observe (P&O)** - Simple, robust algorithm suitable for most conditions
2. **Incremental Conductance (IC)** - Better tracking in rapidly changing conditions
3. **Constant Voltage (CV)** - Simple implementation for stable solar conditions

Key features:
- Adaptive step size for faster MPP tracking
- Weather-adaptive operation (cloud cover, temperature compensation)
- Daily energy accounting and tracking
- Real-time efficiency monitoring
- PWM-based charge control

### Battery Manager (`battery_manager.h/cpp`)

Advanced battery management supporting multiple chemistries:

- **Lithium-Ion (Li-ion)** - Standard 3.7V cells
- **Lithium Polymer (LiPo)** - Lightweight option
- **LiFePO4** - Long-life, temperature-tolerant
- **Lead-Acid** - Traditional 12V systems
- **NiMH** - Rechargeable alkaline alternative

Charging stages:
1. **Precharge** - For deeply discharged batteries
2. **Bulk** - Constant current charging
3. **Absorption** - Constant voltage, tapering current
4. **Float** - Maintenance charging

Protection features:
- Over-voltage protection
- Under-voltage protection
- Over-current protection
- Temperature protection (high and low)
- Cell imbalance detection

### Solar Power System (`solar_power_system.h/cpp`)

Unified interface integrating MPPT and battery management with:

- **AUTO mode** - Automatic optimization based on conditions
- **MAXIMUM_HARVEST** - Prioritize solar energy collection
- **BATTERY_PRESERVE** - Prioritize battery longevity
- **BALANCED** - Balance between harvest and preservation
- **EMERGENCY** - Low power conservation mode

## Pin Configuration

### Required Pins
| Function | Default Pin | Notes |
|----------|-------------|-------|
| Solar Voltage | GPIO 34 | ADC input with voltage divider |
| Solar Current | GPIO 35 | Current sense amplifier output |
| Battery Voltage | GPIO 32 | ADC input with voltage divider |
| MPPT PWM | GPIO 25 | PWM output to charge controller |

### Optional Pins
| Function | Default Pin | Notes |
|----------|-------------|-------|
| Battery Current | GPIO 33 | Current sense amplifier |
| Battery Temperature | GPIO 36 | NTC thermistor |
| Charge Enable | GPIO 26 | Digital output |

## Hardware Setup

### Voltage Dividers
Use 2:1 voltage dividers to scale battery/solar voltages to the ESP32's 0-3.3V ADC range:

```
Solar Panel     R1 (100k)
  +  ──────────┬──────────> To ADC
               │
              R2 (100k)
               │
              GND
```

### Current Sensing
Use a Hall-effect current sensor (e.g., ACS712) or shunt resistor with amplifier:

```
Solar +  ──────[ACS712]──────> Load
                  │
                  └──> To ADC (center point at Vcc/2)
```

### Temperature Sensing
Use 10K NTC thermistor with 10K pullup:

```
3.3V ────┬──────────> To ADC
         │
        10K (pullup)
         │
     ┌───┴───┐
     │  NTC  │
     │  10K  │
     └───┬───┘
         │
        GND
```

## Usage Examples

### Basic MPPT Controller

```cpp
#include "mppt_controller.h"

MPPTController mppt(34, 35, 32, 33, 25, MPPTAlgorithm::PERTURB_OBSERVE);

void setup() {
    Serial.begin(115200);
    mppt.begin();
}

void loop() {
    mppt.update();
    
    SolarPowerStatus status = mppt.getStatus();
    Serial.printf("Solar: %.2fV %.0fmA %.0fmW\n",
                  status.solar_voltage,
                  status.solar_current,
                  status.solar_power);
    
    delay(1000);
}
```

### Advanced Battery Management

```cpp
#include "battery_manager.h"

BatteryManager battery(32, 33, 36, 26);

void setup() {
    Serial.begin(115200);
    
    // Use LiFePO4 configuration
    BatteryConfig config = BatteryManager::getDefaultConfig(BatteryChemistry::LIFEPO4);
    config.cell_count = 4;  // 4S pack (12.8V nominal)
    config.nominal_capacity = 10000.0f;  // 10Ah
    
    battery.begin(config);
}

void loop() {
    battery.update();
    
    BatteryStatus status = battery.getStatus();
    Serial.printf("Battery: %.2fV %.0fmA SOC:%.1f%% SOH:%.1f%%\n",
                  status.voltage,
                  status.current,
                  status.state_of_charge,
                  status.state_of_health);
    
    if (battery.hasFault()) {
        Serial.printf("FAULT: %d\n", static_cast<int>(battery.getFault()));
    }
    
    delay(1000);
}
```

### Integrated Solar Power System

```cpp
#include "solar_power_system.h"

SolarPowerPins pins = {
    .solar_voltage = 34,
    .solar_current = 35,
    .battery_voltage = 32,
    .battery_current = 33,
    .battery_temp = 36,
    .mppt_pwm = 25,
    .charge_enable = 26
};

SolarPowerSystem solarSystem(pins);

void setup() {
    Serial.begin(115200);
    
    BatteryConfig config = BatteryManager::getDefaultConfig(BatteryChemistry::LITHIUM_ION);
    solarSystem.begin(config, MPPTAlgorithm::PERTURB_OBSERVE);
    
    // Use automatic mode for hands-off operation
    solarSystem.setMode(SolarPowerMode::AUTO);
}

void loop() {
    solarSystem.update();
    
    // Print status every 10 seconds
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 10000) {
        solarSystem.printStatus();
        lastPrint = millis();
    }
    
    delay(100);
}
```

## API Reference

### MPPTController

```cpp
bool begin();                                    // Initialize controller
void update();                                   // Main update loop (call every 100ms)
SolarPowerStatus getStatus();                    // Get current status
void setAlgorithm(MPPTAlgorithm algorithm);      // Set MPPT algorithm
void setMPPTEnabled(bool enabled);               // Enable/disable MPPT
void updateWeatherConditions(int cloud, float temp); // Update weather for optimization
void calibrateSensors(float v_factor, float c_factor); // Calibrate sensors
float getDailyEnergyHarvest();                   // Get daily energy in Wh
void resetDailyEnergy();                         // Reset daily counter
bool isAtMaxPowerPoint();                        // Check if at MPP
float getChargingEfficiency();                   // Get efficiency percentage
```

### BatteryManager

```cpp
bool begin(const BatteryConfig& config);         // Initialize with configuration
bool begin();                                    // Initialize with default Li-ion config
void update();                                   // Main update loop (call every 100ms)
BatteryStatus getStatus();                       // Get comprehensive status
float getStateOfCharge();                        // Get SOC (0-100%)
float getVoltage();                              // Get voltage in V
float getCurrent();                              // Get current in mA
float getTemperature();                          // Get temperature in °C
ChargingStage getChargingStage();                // Get current charging stage
BatteryHealth getHealth();                       // Get health assessment
void setChargingEnabled(bool enable);            // Enable/disable charging
void setMaxChargeCurrent(float current_ma);      // Set max charge current
void calibrateVoltage(float known_voltage);      // Calibrate voltage sensor
void calibrateCurrent(float known_current);      // Calibrate current sensor
void clearFault();                               // Clear fault condition
static BatteryConfig getDefaultConfig(BatteryChemistry chemistry);
```

### SolarPowerSystem

```cpp
bool begin(const BatteryConfig& config, MPPTAlgorithm algorithm);
bool begin();                                    // Initialize with defaults
void update();                                   // Main update loop
PowerSystemStatus getStatus();                   // Get comprehensive status
void setMode(SolarPowerMode mode);               // Set operating mode
SolarPowerMode getMode();                        // Get current mode
void setMPPTAlgorithm(MPPTAlgorithm algorithm);  // Change MPPT algorithm
void setChargingEnabled(bool enable);            // Enable/disable charging
float getSolarPower();                           // Get solar power in mW
float getBatterySOC();                           // Get battery SOC
bool isSolarAvailable();                         // Check solar availability
bool isCharging();                               // Check if charging
float getDailyEnergyHarvest();                   // Get daily energy in Wh
void updateWeatherConditions(int cloud, float temp); // Update weather
void printStatus();                              // Print status to Serial
```

## Testing

Run the unit tests with PlatformIO:

```bash
cd firmware
pio test -e test
```

Test files:
- `test/test_mppt_controller.cpp` - MPPT controller tests
- `test/test_battery_manager.cpp` - Battery manager tests

## Performance Specifications

### MPPT Controller
- Tracking efficiency: >95% under stable conditions
- Update rate: 10-200 Hz (adaptive)
- Supported solar voltage: 6-24V (with appropriate voltage divider)

### Battery Manager
- SOC accuracy: ±5% with coulomb counting
- Charge efficiency: >90% typical
- Protection response time: <100ms

## License

Copyright (c) 2025 WildCAM ESP32 Team. Licensed under MIT License.
