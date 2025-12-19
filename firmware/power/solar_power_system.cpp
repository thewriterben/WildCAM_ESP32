/**
 * @file solar_power_system.cpp
 * @brief Integrated solar power system implementation
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 3.0.0
 */

#include "solar_power_system.h"

// Timing constants
static const unsigned long UPDATE_INTERVAL = 100;           // 100ms
static const unsigned long MODE_EVAL_INTERVAL = 60000;      // 1 minute

// Thresholds for auto mode
static const float LOW_SOC_THRESHOLD = 20.0f;               // Enter emergency below 20%
static const float MEDIUM_SOC_THRESHOLD = 50.0f;            // Battery preserve below 50%
static const float HIGH_SOC_THRESHOLD = 80.0f;              // Maximum harvest above 80%
static const float MIN_SOLAR_POWER = 100.0f;                // Minimum useful solar power (mW)

SolarPowerSystem::SolarPowerSystem(const SolarPowerPins& pins)
    : mppt_controller_(nullptr), battery_manager_(nullptr),
      pins_(pins), operating_mode_(SolarPowerMode::AUTO),
      initialized_(false), charging_enabled_(true),
      total_energy_harvest_(0), cloud_cover_(0), ambient_temperature_(25.0f),
      last_update_(0), last_mode_evaluation_(0) {
}

SolarPowerSystem::~SolarPowerSystem() {
    if (mppt_controller_) {
        delete mppt_controller_;
    }
    if (battery_manager_) {
        delete battery_manager_;
    }
}

bool SolarPowerSystem::begin(const BatteryConfig& battery_config,
                             MPPTAlgorithm mppt_algorithm) {
    Serial.println("[SolarPower] Initializing solar power system...");
    
    // Create MPPT controller
    mppt_controller_ = new MPPTController(
        pins_.solar_voltage,
        pins_.solar_current,
        pins_.battery_voltage,
        pins_.battery_current,
        pins_.mppt_pwm,
        mppt_algorithm
    );
    
    if (!mppt_controller_->begin()) {
        Serial.println("[SolarPower] Failed to initialize MPPT controller");
        return false;
    }
    
    // Create battery manager
    battery_manager_ = new BatteryManager(
        pins_.battery_voltage,
        pins_.battery_current,
        pins_.battery_temp,
        pins_.charge_enable
    );
    
    // Use provided config or default
    BatteryConfig config = battery_config;
    if (config.nominal_capacity == 0) {
        config = BatteryManager::getDefaultConfig(BatteryChemistry::LITHIUM_ION);
    }
    
    if (!battery_manager_->begin(config)) {
        Serial.println("[SolarPower] Failed to initialize battery manager");
        return false;
    }
    
    // Initialize timing
    last_update_ = millis();
    last_mode_evaluation_ = millis();
    
    initialized_ = true;
    
    Serial.println("[SolarPower] Solar power system initialized successfully");
    printStatus();
    
    return true;
}

bool SolarPowerSystem::begin() {
    return begin(BatteryConfig(), MPPTAlgorithm::PERTURB_OBSERVE);
}

void SolarPowerSystem::update() {
    if (!initialized_) {
        return;
    }
    
    unsigned long now = millis();
    
    // Rate limit updates
    if (now - last_update_ < UPDATE_INTERVAL) {
        return;
    }
    last_update_ = now;
    
    // Update MPPT controller
    mppt_controller_->update();
    
    // Update battery manager
    battery_manager_->update();
    
    // Accumulate total energy
    float daily_energy = mppt_controller_->getDailyEnergyHarvest();
    static float last_daily_energy = 0;
    if (daily_energy > last_daily_energy) {
        total_energy_harvest_ += (daily_energy - last_daily_energy);
    } else if (daily_energy < last_daily_energy) {
        // Daily counter was reset
        total_energy_harvest_ += daily_energy;
    }
    last_daily_energy = daily_energy;
    
    // Periodically evaluate operating mode
    if (operating_mode_ == SolarPowerMode::AUTO) {
        if (now - last_mode_evaluation_ >= MODE_EVAL_INTERVAL) {
            evaluateAutoMode();
            last_mode_evaluation_ = now;
        }
    }
    
    // Optimize operation for current mode
    optimizeForMode();
}

PowerSystemStatus SolarPowerSystem::getStatus() const {
    PowerSystemStatus status;
    
    if (!initialized_) {
        memset(&status, 0, sizeof(status));
        return status;
    }
    
    // Get MPPT status
    SolarPowerStatus solar_status = mppt_controller_->getStatus();
    status.solar_voltage = solar_status.solar_voltage;
    status.solar_current = solar_status.solar_current;
    status.solar_power = solar_status.solar_power;
    status.solar_available = solar_status.is_daylight;
    status.cloud_cover = solar_status.cloud_cover_estimate;
    status.charge_efficiency = solar_status.charge_efficiency;
    status.daily_energy_harvest = solar_status.daily_energy_harvest;
    
    // Get battery status
    BatteryStatus battery_status = battery_manager_->getStatus();
    status.battery_voltage = battery_status.voltage;
    status.battery_current = battery_status.current;
    status.battery_soc = battery_status.state_of_charge;
    status.battery_soh = battery_status.state_of_health;
    status.battery_temperature = battery_status.temperature;
    status.charging_stage = battery_status.charging_stage;
    status.battery_health = battery_status.health;
    status.battery_charging = battery_status.is_charging;
    status.time_to_full = battery_status.time_to_full;
    status.time_to_empty = battery_status.time_to_empty;
    status.battery_fault = battery_status.fault;
    
    // System status
    status.total_energy_harvest = total_energy_harvest_;
    status.operating_mode = operating_mode_;
    status.mppt_algorithm = MPPTAlgorithm::PERTURB_OBSERVE;  // Would need getter
    status.has_fault = hasFault();
    
    return status;
}

void SolarPowerSystem::setMode(SolarPowerMode mode) {
    operating_mode_ = mode;
    
    const char* mode_names[] = {"AUTO", "MAX_HARVEST", "BATTERY_PRESERVE", "BALANCED", "EMERGENCY"};
    Serial.printf("[SolarPower] Mode set to: %s\n", mode_names[static_cast<int>(mode)]);
    
    // Apply mode-specific settings
    optimizeForMode();
}

void SolarPowerSystem::setMPPTAlgorithm(MPPTAlgorithm algorithm) {
    if (mppt_controller_) {
        mppt_controller_->setAlgorithm(algorithm);
        
        const char* algo_names[] = {"P&O", "Inc Conductance", "Constant Voltage"};
        Serial.printf("[SolarPower] MPPT algorithm set to: %s\n", 
                     algo_names[static_cast<int>(algorithm)]);
    }
}

void SolarPowerSystem::setChargingEnabled(bool enable) {
    charging_enabled_ = enable;
    
    if (mppt_controller_) {
        mppt_controller_->setMPPTEnabled(enable);
    }
    if (battery_manager_) {
        battery_manager_->setChargingEnabled(enable);
    }
    
    Serial.printf("[SolarPower] Charging %s\n", enable ? "enabled" : "disabled");
}

bool SolarPowerSystem::isChargingEnabled() const {
    return charging_enabled_;
}

float SolarPowerSystem::getSolarPower() const {
    if (mppt_controller_) {
        SolarPowerStatus status = mppt_controller_->getStatus();
        return status.solar_power;
    }
    return 0;
}

float SolarPowerSystem::getBatterySOC() const {
    if (battery_manager_) {
        return battery_manager_->getStateOfCharge();
    }
    return 0;
}

float SolarPowerSystem::getBatteryVoltage() const {
    if (battery_manager_) {
        return battery_manager_->getVoltage();
    }
    return 0;
}

bool SolarPowerSystem::isSolarAvailable() const {
    if (mppt_controller_) {
        SolarPowerStatus status = mppt_controller_->getStatus();
        return status.is_daylight && status.solar_power > MIN_SOLAR_POWER;
    }
    return false;
}

bool SolarPowerSystem::isCharging() const {
    if (battery_manager_) {
        return battery_manager_->isCharging();
    }
    return false;
}

float SolarPowerSystem::getDailyEnergyHarvest() const {
    if (mppt_controller_) {
        return mppt_controller_->getDailyEnergyHarvest();
    }
    return 0;
}

void SolarPowerSystem::resetDailyEnergy() {
    if (mppt_controller_) {
        mppt_controller_->resetDailyEnergy();
    }
}

bool SolarPowerSystem::hasFault() const {
    if (battery_manager_) {
        return battery_manager_->hasFault();
    }
    return false;
}

void SolarPowerSystem::clearFault() {
    if (battery_manager_) {
        battery_manager_->clearFault();
    }
}

void SolarPowerSystem::updateWeatherConditions(int cloud_cover, float temperature) {
    cloud_cover_ = cloud_cover;
    ambient_temperature_ = temperature;
    
    if (mppt_controller_) {
        mppt_controller_->updateWeatherConditions(cloud_cover, temperature);
    }
}

void SolarPowerSystem::calibrateSolarVoltage(float known_voltage) {
    if (mppt_controller_) {
        // Calculate calibration factor
        SolarPowerStatus status = mppt_controller_->getStatus();
        if (status.solar_voltage > 0) {
            float factor = known_voltage / status.solar_voltage;
            mppt_controller_->calibrateSensors(factor, 1.0f);
        }
    }
}

void SolarPowerSystem::calibrateSolarCurrent(float known_current) {
    if (mppt_controller_) {
        SolarPowerStatus status = mppt_controller_->getStatus();
        if (status.solar_current > 0) {
            float factor = known_current / status.solar_current;
            mppt_controller_->calibrateSensors(1.0f, factor);
        }
    }
}

void SolarPowerSystem::calibrateBatteryVoltage(float known_voltage) {
    if (battery_manager_) {
        battery_manager_->calibrateVoltage(known_voltage);
    }
}

void SolarPowerSystem::printStatus() const {
    PowerSystemStatus status = getStatus();
    
    Serial.println("\n========== Solar Power System Status ==========");
    Serial.printf("Solar:  V=%.2fV  I=%.0fmA  P=%.0fmW  Available=%s\n",
                  status.solar_voltage, status.solar_current, status.solar_power,
                  status.solar_available ? "Yes" : "No");
    Serial.printf("Battery: V=%.2fV  I=%.0fmA  SOC=%.1f%%  SOH=%.1f%%\n",
                  status.battery_voltage, status.battery_current,
                  status.battery_soc, status.battery_soh);
    Serial.printf("Charging: %s  Stage=%d  Efficiency=%.1f%%\n",
                  status.battery_charging ? "Yes" : "No",
                  static_cast<int>(status.charging_stage),
                  status.charge_efficiency);
    Serial.printf("Energy: Daily=%.2fWh  Total=%.2fWh\n",
                  status.daily_energy_harvest, status.total_energy_harvest);
    Serial.printf("Mode: %d  Fault: %s\n",
                  static_cast<int>(status.operating_mode),
                  status.has_fault ? "YES" : "No");
    Serial.println("===============================================\n");
}

void SolarPowerSystem::optimizeForMode() {
    if (!initialized_) return;
    
    switch (operating_mode_) {
        case SolarPowerMode::MAXIMUM_HARVEST:
            // Use P&O for maximum tracking, fast updates
            mppt_controller_->setWeatherAdaptation(true);
            break;
            
        case SolarPowerMode::BATTERY_PRESERVE:
            // Use CV for gentler charging, slower updates
            mppt_controller_->setAlgorithm(MPPTAlgorithm::CONSTANT_VOLTAGE);
            mppt_controller_->setWeatherAdaptation(false);
            break;
            
        case SolarPowerMode::BALANCED:
            // Use IC for balanced operation
            mppt_controller_->setAlgorithm(MPPTAlgorithm::INCREMENTAL_CONDUCTANCE);
            mppt_controller_->setWeatherAdaptation(true);
            break;
            
        case SolarPowerMode::EMERGENCY:
            // Minimize power consumption
            mppt_controller_->setMPPTEnabled(false);
            break;
            
        case SolarPowerMode::AUTO:
            // Handled by evaluateAutoMode()
            break;
    }
}

void SolarPowerSystem::evaluateAutoMode() {
    SolarPowerMode optimal_mode = determineOptimalMode();
    
    // Only log if mode would change
    static SolarPowerMode last_optimal = SolarPowerMode::AUTO;
    if (optimal_mode != last_optimal) {
        const char* mode_names[] = {"AUTO", "MAX_HARVEST", "BATTERY_PRESERVE", "BALANCED", "EMERGENCY"};
        Serial.printf("[SolarPower] Auto mode: selecting %s\n", 
                     mode_names[static_cast<int>(optimal_mode)]);
        last_optimal = optimal_mode;
    }
    
    // Apply optimal settings (but don't change operating_mode_ since we're in AUTO)
    switch (optimal_mode) {
        case SolarPowerMode::EMERGENCY:
            mppt_controller_->setMPPTEnabled(false);
            break;
            
        case SolarPowerMode::BATTERY_PRESERVE:
            mppt_controller_->setMPPTEnabled(true);
            mppt_controller_->setAlgorithm(MPPTAlgorithm::CONSTANT_VOLTAGE);
            break;
            
        case SolarPowerMode::MAXIMUM_HARVEST:
            mppt_controller_->setMPPTEnabled(true);
            mppt_controller_->setAlgorithm(MPPTAlgorithm::PERTURB_OBSERVE);
            mppt_controller_->setWeatherAdaptation(true);
            break;
            
        case SolarPowerMode::BALANCED:
        default:
            mppt_controller_->setMPPTEnabled(true);
            mppt_controller_->setAlgorithm(MPPTAlgorithm::INCREMENTAL_CONDUCTANCE);
            break;
    }
}

SolarPowerMode SolarPowerSystem::determineOptimalMode() const {
    float soc = battery_manager_->getStateOfCharge();
    bool solar_available = isSolarAvailable();
    
    // Emergency mode if battery critically low
    if (soc < LOW_SOC_THRESHOLD) {
        return SolarPowerMode::EMERGENCY;
    }
    
    // No solar - just maintain
    if (!solar_available) {
        return SolarPowerMode::BALANCED;
    }
    
    // Battery low - prioritize charging
    if (soc < MEDIUM_SOC_THRESHOLD) {
        return SolarPowerMode::MAXIMUM_HARVEST;
    }
    
    // Battery high - gentle charging
    if (soc > HIGH_SOC_THRESHOLD) {
        return SolarPowerMode::BATTERY_PRESERVE;
    }
    
    // Default balanced operation
    return SolarPowerMode::BALANCED;
}
