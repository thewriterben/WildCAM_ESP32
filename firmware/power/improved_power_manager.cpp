/**
 * @file improved_power_manager.cpp
 * @brief Improved power management implementation
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 1.0.0
 */

#include "improved_power_manager.h"
#include <esp_sleep.h>

// Timing constants
static const unsigned long UPDATE_INTERVAL = 100;       // 100ms update interval
static const unsigned long MODE_EVAL_INTERVAL = 60000;  // 1 minute mode evaluation

// Default configuration values
static const float DEFAULT_LOW_BATTERY = 20.0f;
static const float DEFAULT_CRITICAL_BATTERY = 10.0f;
static const float DEFAULT_FULL_CHARGE = 95.0f;
static const uint32_t DEFAULT_MIN_SLEEP = 5000;         // 5 seconds
static const uint32_t DEFAULT_MAX_SLEEP = 3600000;      // 1 hour
static const uint32_t DEFAULT_SLEEP = 300000;           // 5 minutes

ImprovedPowerManager::ImprovedPowerManager()
    : solar_system_(nullptr), sleep_scheduler_(nullptr),
      initialized_(false), operating_mode_(PowerSystemMode::NORMAL),
      last_update_(0), last_mode_evaluation_(0),
      current_hour_(12), current_minute_(0) {
}

ImprovedPowerManager::~ImprovedPowerManager() {
    if (solar_system_) {
        delete solar_system_;
        solar_system_ = nullptr;
    }
    if (sleep_scheduler_) {
        delete sleep_scheduler_;
        sleep_scheduler_ = nullptr;
    }
}

bool ImprovedPowerManager::begin(const ImprovedPowerPins& pins, const ImprovedPowerConfig& config) {
    pins_ = pins;
    config_ = config;
    
    Serial.println("[ImprovedPower] Initializing improved power management...");
    
    if (!initializeComponents()) {
        Serial.println("[ImprovedPower] Failed to initialize components");
        return false;
    }
    
    // Initialize timing
    last_update_ = millis();
    last_mode_evaluation_ = millis();
    
    initialized_ = true;
    
    Serial.println("[ImprovedPower] Power management initialized successfully");
    printStatus();
    
    return true;
}

bool ImprovedPowerManager::begin(const ImprovedPowerPins& pins) {
    return begin(pins, getDefaultConfig());
}

void ImprovedPowerManager::update() {
    if (!initialized_) return;
    
    unsigned long now = millis();
    
    // Rate limit updates
    if (now - last_update_ < UPDATE_INTERVAL) {
        return;
    }
    last_update_ = now;
    
    // Update solar power system
    if (solar_system_) {
        solar_system_->update();
    }
    
    // Update sleep scheduler with current battery level
    if (sleep_scheduler_) {
        sleep_scheduler_->setCurrentTime(current_hour_, current_minute_);
        
        if (solar_system_) {
            sleep_scheduler_->updateBatteryLevel(solar_system_->getBatterySOC());
        }
        
        sleep_scheduler_->update();
    }
    
    // Periodic mode evaluation
    if (config_.enable_auto_mode_switching) {
        if (now - last_mode_evaluation_ >= MODE_EVAL_INTERVAL) {
            evaluateOperatingMode();
            last_mode_evaluation_ = now;
        }
    }
}

ImprovedPowerStatus ImprovedPowerManager::getStatus() const {
    ImprovedPowerStatus status;
    memset(&status, 0, sizeof(status));
    
    if (!initialized_) return status;
    
    // Get solar/battery status
    if (solar_system_) {
        PowerSystemStatus sys_status = solar_system_->getStatus();
        
        // Battery metrics
        status.battery_voltage = sys_status.battery_voltage;
        status.battery_current = sys_status.battery_current;
        status.battery_soc = sys_status.battery_soc;
        status.battery_soh = sys_status.battery_soh;
        status.battery_temperature = sys_status.battery_temperature;
        status.battery_health = sys_status.battery_health;
        status.charging_stage = sys_status.charging_stage;
        
        BatteryManager* bm = const_cast<SolarPowerSystem*>(solar_system_)->getBatteryManager();
        if (bm) {
            status.cycle_count = bm->getCycleCount();
        }
        
        // Solar metrics
        status.solar_voltage = sys_status.solar_voltage;
        status.solar_current = sys_status.solar_current;
        status.solar_power = sys_status.solar_power;
        status.solar_available = sys_status.solar_available;
        status.daily_energy_harvest = sys_status.daily_energy_harvest;
        status.charge_efficiency = sys_status.charge_efficiency;
        status.cloud_cover_estimate = sys_status.cloud_cover;
        
        // Time estimates
        status.time_to_full_min = sys_status.time_to_full;
        status.time_to_empty_min = sys_status.time_to_empty;
        
        // Fault status
        status.has_fault = sys_status.has_fault;
        status.battery_fault = sys_status.battery_fault;
    }
    
    // Sleep scheduler metrics
    if (sleep_scheduler_) {
        status.recommended_sleep_ms = const_cast<AdaptiveSleepScheduler*>(sleep_scheduler_)
                                      ->getRecommendedSleepDuration();
        status.is_active_time = const_cast<AdaptiveSleepScheduler*>(sleep_scheduler_)
                                ->isCurrentlyActiveTime();
        status.activity_score = const_cast<AdaptiveSleepScheduler*>(sleep_scheduler_)
                                ->getActivityScore();
    }
    
    status.operating_mode = operating_mode_;
    
    return status;
}

float ImprovedPowerManager::getBatteryVoltage() const {
    if (solar_system_) {
        return solar_system_->getBatteryVoltage();
    }
    return 0.0f;
}

float ImprovedPowerManager::getBatterySOC() const {
    if (solar_system_) {
        return solar_system_->getBatterySOC();
    }
    return 0.0f;
}

float ImprovedPowerManager::getBatterySOH() const {
    if (solar_system_) {
        BatteryManager* bm = const_cast<SolarPowerSystem*>(solar_system_)->getBatteryManager();
        if (bm) {
            return bm->getStateOfHealth();
        }
    }
    return 100.0f;
}

BatteryHealth ImprovedPowerManager::getBatteryHealth() const {
    if (solar_system_) {
        BatteryManager* bm = const_cast<SolarPowerSystem*>(solar_system_)->getBatteryManager();
        if (bm) {
            return bm->getHealth();
        }
    }
    return BatteryHealth::UNKNOWN;
}

bool ImprovedPowerManager::isCharging() const {
    if (solar_system_) {
        return solar_system_->isCharging();
    }
    return false;
}

float ImprovedPowerManager::getSolarPower() const {
    if (solar_system_) {
        return solar_system_->getSolarPower();
    }
    return 0.0f;
}

bool ImprovedPowerManager::isSolarAvailable() const {
    if (solar_system_) {
        return solar_system_->isSolarAvailable();
    }
    return false;
}

float ImprovedPowerManager::getDailyEnergyHarvest() const {
    if (solar_system_) {
        return solar_system_->getDailyEnergyHarvest();
    }
    return 0.0f;
}

void ImprovedPowerManager::setOperatingMode(PowerSystemMode mode) {
    if (operating_mode_ == mode) return;
    
    operating_mode_ = mode;
    
    const char* mode_names[] = {
        "Normal", "Power Save", "Solar Priority", 
        "Battery Preserve", "Emergency", "Maintenance"
    };
    Serial.printf("[ImprovedPower] Operating mode changed to: %s\n",
                  mode_names[static_cast<int>(mode)]);
    
    applyModeOptimizations();
}

uint32_t ImprovedPowerManager::getRecommendedSleepDuration() {
    if (!initialized_) {
        return config_.default_sleep_duration_ms;
    }
    
    uint32_t duration = config_.default_sleep_duration_ms;
    
    // Get sleep scheduler recommendation
    if (sleep_scheduler_) {
        duration = sleep_scheduler_->getRecommendedSleepDuration();
    }
    
    // Override based on operating mode
    switch (operating_mode_) {
        case PowerSystemMode::EMERGENCY:
            // Maximize sleep in emergency
            duration = config_.max_sleep_duration_ms;
            break;
            
        case PowerSystemMode::POWER_SAVE:
            // Increase sleep duration
            duration = static_cast<uint32_t>(duration * 1.5f);
            break;
            
        case PowerSystemMode::SOLAR_PRIORITY:
            // During solar hours, may stay awake more
            if (isSolarAvailable()) {
                duration = static_cast<uint32_t>(duration * 0.5f);
            }
            break;
            
        case PowerSystemMode::MAINTENANCE:
            // Minimal sleep during maintenance
            duration = config_.min_sleep_duration_ms;
            break;
            
        default:
            // Use scheduler recommendation
            break;
    }
    
    // Clamp to configured range
    if (duration < config_.min_sleep_duration_ms) {
        duration = config_.min_sleep_duration_ms;
    }
    if (duration > config_.max_sleep_duration_ms) {
        duration = config_.max_sleep_duration_ms;
    }
    
    return duration;
}

void ImprovedPowerManager::recordMotionEvent(unsigned long timestamp) {
    if (sleep_scheduler_) {
        sleep_scheduler_->recordMotionEvent(timestamp);
    }
}

bool ImprovedPowerManager::isActiveTime() const {
    if (sleep_scheduler_) {
        return const_cast<AdaptiveSleepScheduler*>(sleep_scheduler_)->isCurrentlyActiveTime();
    }
    return true;  // Default to active if no scheduler
}

float ImprovedPowerManager::getActivityScore() const {
    if (sleep_scheduler_) {
        return const_cast<AdaptiveSleepScheduler*>(sleep_scheduler_)->getActivityScore();
    }
    return 0.5f;  // Default moderate activity
}

void ImprovedPowerManager::enterDeepSleep(uint32_t duration_ms) {
    if (duration_ms == 0) {
        duration_ms = getRecommendedSleepDuration();
    }
    
    prepareForDeepSleep(duration_ms);
    
    Serial.printf("[ImprovedPower] Entering deep sleep for %lu ms\n", duration_ms);
    Serial.flush();
    
    // Configure timer wakeup
    esp_sleep_enable_timer_wakeup(duration_ms * 1000ULL);  // Convert to microseconds
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void ImprovedPowerManager::prepareForDeepSleep(uint32_t duration_ms) {
    Serial.println("[ImprovedPower] Preparing for deep sleep...");
    
    // Disable charging during sleep if in emergency mode
    if (operating_mode_ == PowerSystemMode::EMERGENCY) {
        setChargingEnabled(false);
    }
    
    // Log current status
    ImprovedPowerStatus status = getStatus();
    Serial.printf("[ImprovedPower] Pre-sleep status: SOC=%.1f%%, Solar=%.1fmW, Mode=%d\n",
                  status.battery_soc, status.solar_power, 
                  static_cast<int>(status.operating_mode));
}

void ImprovedPowerManager::setCurrentTime(uint8_t hour, uint8_t minute) {
    current_hour_ = (hour < 24) ? hour : 0;
    current_minute_ = (minute < 60) ? minute : 0;
    
    if (sleep_scheduler_) {
        sleep_scheduler_->setCurrentTime(current_hour_, current_minute_);
    }
}

void ImprovedPowerManager::updateWeatherConditions(int cloud_cover, float temperature) {
    if (solar_system_) {
        solar_system_->updateWeatherConditions(cloud_cover, temperature);
    }
}

void ImprovedPowerManager::setChargingEnabled(bool enable) {
    if (solar_system_) {
        solar_system_->setChargingEnabled(enable);
    }
}

bool ImprovedPowerManager::isChargingEnabled() const {
    if (solar_system_) {
        return solar_system_->isChargingEnabled();
    }
    return false;
}

bool ImprovedPowerManager::hasFault() const {
    if (solar_system_) {
        return solar_system_->hasFault();
    }
    return false;
}

void ImprovedPowerManager::clearFault() {
    if (solar_system_) {
        solar_system_->clearFault();
    }
}

void ImprovedPowerManager::calibrateBatteryVoltage(float known_voltage) {
    if (solar_system_) {
        solar_system_->calibrateBatteryVoltage(known_voltage);
    }
}

void ImprovedPowerManager::calibrateSolarSensors(float voltage_factor, float current_factor) {
    if (solar_system_) {
        MPPTController* mppt = solar_system_->getMPPTController();
        if (mppt) {
            mppt->calibrateSensors(voltage_factor, current_factor);
        }
    }
}

void ImprovedPowerManager::resetMotionPatterns() {
    if (sleep_scheduler_) {
        sleep_scheduler_->resetPatterns();
    }
}

void ImprovedPowerManager::resetDailyEnergy() {
    if (solar_system_) {
        solar_system_->resetDailyEnergy();
    }
}

void ImprovedPowerManager::printStatus() const {
    ImprovedPowerStatus status = getStatus();
    
    Serial.println("\n============ Improved Power Status ============");
    Serial.printf("Operating Mode: %d\n", static_cast<int>(status.operating_mode));
    Serial.println("--- Battery ---");
    Serial.printf("  Voltage: %.2fV  Current: %.0fmA\n", 
                  status.battery_voltage, status.battery_current);
    Serial.printf("  SOC: %.1f%%  SOH: %.1f%%  Health: %d\n",
                  status.battery_soc, status.battery_soh, 
                  static_cast<int>(status.battery_health));
    Serial.printf("  Temperature: %.1f°C  Cycles: %d\n",
                  status.battery_temperature, status.cycle_count);
    Serial.printf("  Charging Stage: %d\n", static_cast<int>(status.charging_stage));
    Serial.println("--- Solar ---");
    Serial.printf("  Voltage: %.2fV  Current: %.0fmA  Power: %.1fmW\n",
                  status.solar_voltage, status.solar_current, status.solar_power);
    Serial.printf("  Available: %s  Daily Harvest: %.2fWh\n",
                  status.solar_available ? "Yes" : "No", status.daily_energy_harvest);
    Serial.printf("  Efficiency: %.1f%%  Cloud Cover: %d%%\n",
                  status.charge_efficiency, status.cloud_cover_estimate);
    Serial.println("--- Scheduling ---");
    Serial.printf("  Recommended Sleep: %lu ms\n", status.recommended_sleep_ms);
    Serial.printf("  Active Time: %s  Activity Score: %.2f\n",
                  status.is_active_time ? "Yes" : "No", status.activity_score);
    Serial.println("--- Time Estimates ---");
    Serial.printf("  Time to Full: %.1f min  Time to Empty: %.1f min\n",
                  status.time_to_full_min, status.time_to_empty_min);
    Serial.printf("  Fault: %s\n", status.has_fault ? "YES" : "No");
    Serial.println("================================================\n");
}

void ImprovedPowerManager::printDailySchedule() const {
    if (sleep_scheduler_) {
        const_cast<AdaptiveSleepScheduler*>(sleep_scheduler_)->printDailyPattern();
    }
}

ImprovedPowerConfig ImprovedPowerManager::getDefaultConfig() {
    ImprovedPowerConfig config;
    
    config.low_battery_threshold = DEFAULT_LOW_BATTERY;
    config.critical_battery_threshold = DEFAULT_CRITICAL_BATTERY;
    config.full_charge_threshold = DEFAULT_FULL_CHARGE;
    
    config.min_sleep_duration_ms = DEFAULT_MIN_SLEEP;
    config.max_sleep_duration_ms = DEFAULT_MAX_SLEEP;
    config.default_sleep_duration_ms = DEFAULT_SLEEP;
    
    config.enable_mppt = true;
    config.mppt_algorithm = MPPTAlgorithm::PERTURB_OBSERVE;
    
    config.battery_chemistry = BatteryChemistry::LITHIUM_ION;
    config.battery_cell_count = 1;
    config.battery_capacity_mah = 2000.0f;
    
    config.enable_adaptive_sleep = true;
    config.enable_auto_mode_switching = true;
    config.sleep_aggressiveness = SleepAggressiveness::BALANCED;
    
    return config;
}

bool ImprovedPowerManager::initializeComponents() {
    // Create solar power system pins structure
    SolarPowerPins solar_pins;
    solar_pins.solar_voltage = pins_.solar_voltage_pin;
    solar_pins.solar_current = pins_.solar_current_pin;
    solar_pins.battery_voltage = pins_.battery_voltage_pin;
    solar_pins.battery_current = pins_.battery_current_pin;
    solar_pins.battery_temp = pins_.battery_temp_pin;
    solar_pins.mppt_pwm = pins_.mppt_pwm_pin;
    solar_pins.charge_enable = pins_.charge_enable_pin;
    
    // Create solar power system
    solar_system_ = new SolarPowerSystem(solar_pins);
    
    // Create battery configuration
    BatteryConfig battery_config = BatteryManager::getDefaultConfig(config_.battery_chemistry);
    battery_config.cell_count = config_.battery_cell_count;
    battery_config.nominal_capacity = config_.battery_capacity_mah;
    
    // Initialize solar system
    if (!solar_system_->begin(battery_config, config_.mppt_algorithm)) {
        Serial.println("[ImprovedPower] Failed to initialize solar power system");
        return false;
    }
    
    // Create and initialize sleep scheduler
    if (config_.enable_adaptive_sleep) {
        sleep_scheduler_ = new AdaptiveSleepScheduler();
        
        SchedulerConfig sched_config = AdaptiveSleepScheduler::getDefaultConfig();
        sched_config.aggressiveness = config_.sleep_aggressiveness;
        sched_config.min_sleep_duration_ms = config_.min_sleep_duration_ms;
        sched_config.max_sleep_duration_ms = config_.max_sleep_duration_ms;
        sched_config.default_sleep_duration_ms = config_.default_sleep_duration_ms;
        sched_config.battery_low_threshold = config_.low_battery_threshold;
        sched_config.battery_critical_threshold = config_.critical_battery_threshold;
        
        if (!sleep_scheduler_->begin(sched_config)) {
            Serial.println("[ImprovedPower] Failed to initialize sleep scheduler");
            return false;
        }
    }
    
    return true;
}

void ImprovedPowerManager::evaluateOperatingMode() {
    if (!config_.enable_auto_mode_switching) return;
    
    PowerSystemMode optimal = determineOptimalMode();
    
    if (optimal != operating_mode_) {
        setOperatingMode(optimal);
    }
}

void ImprovedPowerManager::applyModeOptimizations() {
    if (!solar_system_) return;
    
    switch (operating_mode_) {
        case PowerSystemMode::NORMAL:
            solar_system_->setMode(SolarPowerMode::BALANCED);
            if (sleep_scheduler_) {
                sleep_scheduler_->setAggressiveness(SleepAggressiveness::BALANCED);
            }
            break;
            
        case PowerSystemMode::POWER_SAVE:
            solar_system_->setMode(SolarPowerMode::BATTERY_PRESERVE);
            if (sleep_scheduler_) {
                sleep_scheduler_->setAggressiveness(SleepAggressiveness::AGGRESSIVE);
            }
            break;
            
        case PowerSystemMode::SOLAR_PRIORITY:
            solar_system_->setMode(SolarPowerMode::MAXIMUM_HARVEST);
            if (sleep_scheduler_) {
                sleep_scheduler_->setAggressiveness(SleepAggressiveness::CONSERVATIVE);
            }
            break;
            
        case PowerSystemMode::BATTERY_PRESERVE:
            solar_system_->setMode(SolarPowerMode::BATTERY_PRESERVE);
            if (sleep_scheduler_) {
                sleep_scheduler_->setAggressiveness(SleepAggressiveness::AGGRESSIVE);
            }
            break;
            
        case PowerSystemMode::EMERGENCY:
            solar_system_->setMode(SolarPowerMode::EMERGENCY);
            if (sleep_scheduler_) {
                sleep_scheduler_->setAggressiveness(SleepAggressiveness::ULTRA_AGGRESSIVE);
            }
            break;
            
        case PowerSystemMode::MAINTENANCE:
            solar_system_->setMode(SolarPowerMode::AUTO);
            if (sleep_scheduler_) {
                sleep_scheduler_->setAggressiveness(SleepAggressiveness::CONSERVATIVE);
            }
            break;
    }
}

PowerSystemMode ImprovedPowerManager::determineOptimalMode() const {
    float soc = getBatterySOC();
    bool solar_available = isSolarAvailable();
    bool charging = isCharging();
    
    // Emergency mode for critically low battery
    if (soc < config_.critical_battery_threshold) {
        return PowerSystemMode::EMERGENCY;
    }
    
    // Power save for low battery
    if (soc < config_.low_battery_threshold) {
        return PowerSystemMode::POWER_SAVE;
    }
    
    // Solar priority when battery is good and solar available
    if (solar_available && soc < config_.full_charge_threshold) {
        return PowerSystemMode::SOLAR_PRIORITY;
    }
    
    // Battery preserve when full and solar available
    if (soc >= config_.full_charge_threshold && solar_available) {
        return PowerSystemMode::BATTERY_PRESERVE;
    }
    
    // Default to normal operation
    return PowerSystemMode::NORMAL;
}

void ImprovedPowerManager::initializeDefaultConfig() {
    config_ = getDefaultConfig();
}
