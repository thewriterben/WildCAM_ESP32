/**
 * @file battery_manager.cpp
 * @brief Advanced battery management implementation
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 3.0.0
 */

#include "battery_manager.h"
#include <algorithm>

// ADC Constants
static const int ADC_MAX_VALUE = 4095;
static const float ADC_REF_VOLTAGE = 3.3f;
static const int ADC_SAMPLES = 16;

// Timing constants
static const unsigned long UPDATE_INTERVAL = 100;       // 100ms update interval
static const unsigned long STAGE_MIN_TIME = 5000;       // Minimum time in each stage (5s)
static const unsigned long SOC_UPDATE_INTERVAL = 1000;  // SOC calculation interval (1s)

// Protection thresholds
static const float OVER_VOLTAGE_MARGIN = 0.1f;          // 100mV over max is fault
static const float UNDER_VOLTAGE_MARGIN = 0.2f;         // 200mV under min is fault
static const float CURRENT_FAULT_MARGIN = 1.2f;         // 20% over max current is fault

// SOC estimation constants
static const float SOC_VOLTAGE_WEIGHT = 0.3f;           // Weight for voltage-based SOC
static const float SOC_COULOMB_WEIGHT = 0.7f;           // Weight for coulomb counting

BatteryManager::BatteryManager(int voltage_pin, int current_pin, 
                               int temp_pin, int charge_enable_pin)
    : voltage_pin_(voltage_pin), current_pin_(current_pin),
      temp_pin_(temp_pin), charge_enable_pin_(charge_enable_pin),
      voltage_(0), current_(0), temperature_(25.0f),
      state_of_charge_(0), state_of_health_(100.0f),
      charging_stage_(ChargingStage::IDLE),
      battery_health_(BatteryHealth::UNKNOWN),
      current_fault_(BatteryFault::NONE),
      charging_enabled_(true), balancing_active_(false),
      target_charge_current_(0),
      voltage_calibration_(1.0f), current_calibration_(1.0f), current_offset_(0),
      cycle_count_(0), coulomb_counter_(0), capacity_used_(0),
      last_update_(0), stage_start_time_(0), last_full_charge_time_(0),
      buffer_index_(0) {
    
    // Initialize averaging buffers
    for (int i = 0; i < AVG_BUFFER_SIZE; i++) {
        voltage_buffer_[i] = 0;
        current_buffer_[i] = 0;
    }
}

bool BatteryManager::begin(const BatteryConfig& config) {
    config_ = config;
    
    // Configure pins
    pinMode(voltage_pin_, INPUT);
    if (current_pin_ >= 0) {
        pinMode(current_pin_, INPUT);
    }
    if (temp_pin_ >= 0) {
        pinMode(temp_pin_, INPUT);
    }
    if (charge_enable_pin_ >= 0) {
        pinMode(charge_enable_pin_, OUTPUT);
        digitalWrite(charge_enable_pin_, charging_enabled_ ? HIGH : LOW);
    }
    
    // Configure ADC
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    
    // Initialize timing
    last_update_ = millis();
    stage_start_time_ = millis();
    
    // Take initial readings
    voltage_ = readVoltage();
    current_ = readCurrent();
    temperature_ = readTemperature();
    
    // Initialize SOC based on voltage (rough estimate)
    state_of_charge_ = calculateSOC();
    coulomb_counter_ = (state_of_charge_ / 100.0f) * config_.nominal_capacity;
    
    Serial.println("[Battery] Manager initialized");
    Serial.printf("[Battery] Chemistry: %d, Cells: %d, Capacity: %.0fmAh\n",
                  static_cast<int>(config_.chemistry), 
                  config_.cell_count, 
                  config_.nominal_capacity);
    Serial.printf("[Battery] Initial voltage: %.2fV, SOC: %.1f%%\n", 
                  voltage_, state_of_charge_);
    
    return true;
}

bool BatteryManager::begin() {
    // Use default Li-ion configuration
    return begin(getDefaultConfig(BatteryChemistry::LITHIUM_ION));
}

void BatteryManager::update() {
    unsigned long now = millis();
    
    // Rate limit updates
    if (now - last_update_ < UPDATE_INTERVAL) {
        return;
    }
    
    float dt = (now - last_update_) / 1000.0f;  // Time delta in seconds
    last_update_ = now;
    
    // Read sensors
    voltage_ = readVoltage();
    current_ = readCurrent();
    temperature_ = readTemperature();
    
    // Update averaging buffers
    voltage_buffer_[buffer_index_] = voltage_;
    current_buffer_[buffer_index_] = current_;
    buffer_index_ = (buffer_index_ + 1) % AVG_BUFFER_SIZE;
    
    // Check protection limits
    checkProtection();
    
    // If fault, don't proceed with charging
    if (current_fault_ != BatteryFault::NONE) {
        if (charging_enabled_) {
            setChargingEnabled(false);
        }
        charging_stage_ = ChargingStage::FAULT;
        return;
    }
    
    // Update coulomb counter
    updateCoulombCounter(current_, dt);
    
    // Update state of charge
    state_of_charge_ = calculateSOC();
    
    // Update charging stage
    updateChargingStage();
    
    // Update health assessment periodically
    static unsigned long last_health_check = 0;
    if (now - last_health_check > 60000) {  // Every minute
        battery_health_ = assessHealth();
        last_health_check = now;
    }
}

BatteryStatus BatteryManager::getStatus() const {
    BatteryStatus status;
    
    status.voltage = voltage_;
    status.current = current_;
    status.temperature = temperature_;
    status.state_of_charge = state_of_charge_;
    status.state_of_health = state_of_health_;
    status.capacity_remaining = coulomb_counter_;
    status.time_to_full = getTimeToFull();
    status.time_to_empty = getTimeToEmpty();
    status.cycle_count = cycle_count_;
    status.charging_stage = charging_stage_;
    status.health = battery_health_;
    status.fault = current_fault_;
    status.is_charging = isCharging();
    status.is_balancing = balancing_active_;
    
    return status;
}

float BatteryManager::getStateOfCharge() const {
    return state_of_charge_;
}

float BatteryManager::getVoltage() const {
    return voltage_;
}

float BatteryManager::getCurrent() const {
    return current_;
}

float BatteryManager::getTemperature() const {
    return temperature_;
}

bool BatteryManager::isCharging() const {
    return (current_ > 10.0f) && 
           (charging_stage_ != ChargingStage::IDLE) &&
           (charging_stage_ != ChargingStage::FAULT);
}

void BatteryManager::clearFault() {
    current_fault_ = BatteryFault::NONE;
    charging_stage_ = ChargingStage::IDLE;
    Serial.println("[Battery] Fault cleared");
}

void BatteryManager::setChargingEnabled(bool enable) {
    charging_enabled_ = enable;
    
    if (charge_enable_pin_ >= 0) {
        digitalWrite(charge_enable_pin_, enable ? HIGH : LOW);
    }
    
    if (!enable) {
        charging_stage_ = ChargingStage::IDLE;
    }
    
    Serial.printf("[Battery] Charging %s\n", enable ? "enabled" : "disabled");
}

void BatteryManager::setMaxChargeCurrent(float current_ma) {
    target_charge_current_ = current_ma;
    config_.max_charge_current = current_ma;
}

float BatteryManager::getTimeToFull() const {
    if (!isCharging() || current_ <= 0) {
        return -1;
    }
    
    float remaining_capacity = config_.nominal_capacity - coulomb_counter_;
    if (remaining_capacity <= 0) {
        return 0;
    }
    
    // Time = Capacity / Current, convert to minutes
    return (remaining_capacity / current_) * 60.0f;
}

float BatteryManager::getTimeToEmpty() const {
    if (current_ >= 0 || coulomb_counter_ <= 0) {
        return -1;  // Not discharging or no capacity
    }
    
    float discharge_current = -current_;  // Make positive
    
    // Time = Capacity / Current, convert to minutes
    return (coulomb_counter_ / discharge_current) * 60.0f;
}

void BatteryManager::calibrateVoltage(float known_voltage) {
    float measured = readVoltage();
    if (measured > 0) {
        voltage_calibration_ = known_voltage / measured;
        Serial.printf("[Battery] Voltage calibration: %.3f\n", voltage_calibration_);
    }
}

void BatteryManager::calibrateCurrent(float known_current) {
    float measured = readCurrent();
    if (abs(measured) > 0.1f) {
        current_calibration_ = known_current / measured;
    } else {
        current_offset_ = known_current - measured;
    }
    Serial.printf("[Battery] Current calibration: %.3f, offset: %.1f\n", 
                  current_calibration_, current_offset_);
}

void BatteryManager::startBalancing() {
    balancing_active_ = true;
    Serial.println("[Battery] Cell balancing started");
}

void BatteryManager::stopBalancing() {
    balancing_active_ = false;
    Serial.println("[Battery] Cell balancing stopped");
}

void BatteryManager::setConfiguration(const BatteryConfig& config) {
    config_ = config;
    Serial.println("[Battery] Configuration updated");
}

BatteryConfig BatteryManager::getDefaultConfig(BatteryChemistry chemistry) {
    BatteryConfig config;
    config.chemistry = chemistry;
    config.cell_count = 1;
    config.temp_compensation = -3.0f;  // -3mV/°C typical
    
    switch (chemistry) {
        case BatteryChemistry::LITHIUM_ION:
        case BatteryChemistry::LITHIUM_POLYMER:
            config.nominal_capacity = 2000.0f;      // 2000mAh typical
            config.charge_voltage = 4.2f;           // 4.2V per cell
            config.float_voltage = 4.1f;            // 4.1V float
            config.discharge_cutoff = 3.0f;         // 3.0V cutoff
            config.max_charge_current = 1000.0f;    // 1C charging
            config.max_discharge_current = 2000.0f; // 2C discharge
            config.precharge_threshold = 3.0f;      // Below 3V needs precharge
            config.precharge_current = 100.0f;      // 0.05C precharge
            config.absorption_current_threshold = 100.0f; // 0.05C end current
            config.max_temperature = 45.0f;
            config.min_temperature = 0.0f;
            break;
            
        case BatteryChemistry::LIFEPO4:
            config.nominal_capacity = 3000.0f;
            config.charge_voltage = 3.65f;
            config.float_voltage = 3.4f;
            config.discharge_cutoff = 2.5f;
            config.max_charge_current = 3000.0f;    // 1C
            config.max_discharge_current = 9000.0f; // 3C
            config.precharge_threshold = 2.5f;
            config.precharge_current = 150.0f;
            config.absorption_current_threshold = 150.0f;
            config.max_temperature = 55.0f;
            config.min_temperature = -20.0f;
            break;
            
        case BatteryChemistry::LEAD_ACID:
            config.cell_count = 6;                  // 12V battery
            config.nominal_capacity = 7000.0f;      // 7Ah typical
            config.charge_voltage = 2.45f;          // Per cell
            config.float_voltage = 2.3f;
            config.discharge_cutoff = 1.75f;
            config.max_charge_current = 700.0f;     // 0.1C
            config.max_discharge_current = 3500.0f; // 0.5C
            config.precharge_threshold = 1.8f;
            config.precharge_current = 70.0f;
            config.absorption_current_threshold = 70.0f;
            config.max_temperature = 40.0f;
            config.min_temperature = -10.0f;
            break;
            
        case BatteryChemistry::NIMH:
            config.nominal_capacity = 2500.0f;
            config.charge_voltage = 1.45f;          // Per cell
            config.float_voltage = 1.35f;
            config.discharge_cutoff = 1.0f;
            config.max_charge_current = 500.0f;     // 0.2C safe
            config.max_discharge_current = 2500.0f;
            config.precharge_threshold = 1.0f;
            config.precharge_current = 50.0f;
            config.absorption_current_threshold = 50.0f;
            config.max_temperature = 45.0f;
            config.min_temperature = 0.0f;
            break;
    }
    
    return config;
}

float BatteryManager::readVoltage() {
    uint32_t sum = 0;
    
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(voltage_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / ADC_SAMPLES;
    
    // Convert ADC value to voltage
    // Assuming 2:1 voltage divider for Li-ion range
    float voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE * 2.0f;
    
    // Apply calibration
    voltage *= voltage_calibration_;
    
    return voltage;
}

float BatteryManager::readCurrent() {
    if (current_pin_ < 0) {
        return 0;  // No current sensor
    }
    
    uint32_t sum = 0;
    
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(current_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / ADC_SAMPLES;
    
    // Convert to current (assuming current sense amplifier)
    // Center point at 1.65V (half of 3.3V), sensitivity ~100mV/A
    float voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
    float current = (voltage - 1.65f) * 1000.0f;  // Convert to mA
    
    // Apply calibration
    current = current * current_calibration_ + current_offset_;
    
    return current;
}

float BatteryManager::readTemperature() {
    if (temp_pin_ < 0) {
        return 25.0f;  // Default room temperature if no sensor
    }
    
    uint32_t sum = 0;
    
    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(temp_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / ADC_SAMPLES;
    
    // Convert to temperature (assuming NTC thermistor with voltage divider)
    // This is a simplified calculation - actual formula depends on thermistor
    float voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
    
    // Simple linear approximation for 10K NTC
    float temperature = 25.0f + (1.65f - voltage) * 50.0f;
    
    return temperature;
}

float BatteryManager::calculateSOC() {
    // Hybrid SOC calculation combining voltage and coulomb counting
    
    // Voltage-based SOC estimation
    float cell_voltage = voltage_ / config_.cell_count;
    float voltage_range = config_.charge_voltage - config_.discharge_cutoff;
    float voltage_soc = ((cell_voltage - config_.discharge_cutoff) / voltage_range) * 100.0f;
    
    // Clamp voltage SOC
    if (voltage_soc < 0) voltage_soc = 0;
    if (voltage_soc > 100) voltage_soc = 100;
    
    // Coulomb counting SOC
    float coulomb_soc = (coulomb_counter_ / config_.nominal_capacity) * 100.0f;
    if (coulomb_soc < 0) coulomb_soc = 0;
    if (coulomb_soc > 100) coulomb_soc = 100;
    
    // Weight-averaged SOC
    // Use more coulomb counting weight when current flow is significant
    float coulomb_weight = SOC_COULOMB_WEIGHT;
    float voltage_weight = SOC_VOLTAGE_WEIGHT;
    
    // At rest (low current), rely more on voltage
    if (abs(current_) < 50.0f) {
        coulomb_weight = 0.3f;
        voltage_weight = 0.7f;
    }
    
    float soc = (voltage_soc * voltage_weight) + (coulomb_soc * coulomb_weight);
    
    return soc;
}

void BatteryManager::updateChargingStage() {
    if (!charging_enabled_) {
        charging_stage_ = ChargingStage::IDLE;
        return;
    }
    
    unsigned long now = millis();
    float cell_voltage = voltage_ / config_.cell_count;
    float target_voltage = getTemperatureCompensatedVoltage(config_.charge_voltage);
    float float_voltage = getTemperatureCompensatedVoltage(config_.float_voltage);
    
    // State machine for charging stages
    switch (charging_stage_) {
        case ChargingStage::IDLE:
            // Check if we should start charging
            if (current_ > 10.0f && cell_voltage < target_voltage) {
                if (cell_voltage < config_.precharge_threshold) {
                    charging_stage_ = ChargingStage::PRECHARGE;
                } else {
                    charging_stage_ = ChargingStage::BULK;
                }
                stage_start_time_ = now;
            }
            break;
            
        case ChargingStage::PRECHARGE:
            // Exit precharge when voltage rises above threshold
            if (cell_voltage >= config_.precharge_threshold) {
                charging_stage_ = ChargingStage::BULK;
                stage_start_time_ = now;
                Serial.println("[Battery] Precharge complete, entering bulk charge");
            }
            break;
            
        case ChargingStage::BULK:
            // Exit bulk when voltage reaches target
            if (cell_voltage >= target_voltage) {
                charging_stage_ = ChargingStage::ABSORPTION;
                stage_start_time_ = now;
                Serial.println("[Battery] Bulk charge complete, entering absorption");
            }
            break;
            
        case ChargingStage::ABSORPTION:
            // Exit absorption when current drops below threshold
            if (current_ < config_.absorption_current_threshold) {
                charging_stage_ = ChargingStage::FLOAT;
                stage_start_time_ = now;
                last_full_charge_time_ = now;
                
                // Increment cycle count
                cycle_count_++;
                
                // Reset coulomb counter to full capacity
                coulomb_counter_ = config_.nominal_capacity;
                
                Serial.println("[Battery] Absorption complete, entering float charge");
                Serial.printf("[Battery] Cycle count: %d\n", cycle_count_);
            }
            break;
            
        case ChargingStage::FLOAT:
            // Maintain float voltage
            // Drop back to bulk if voltage drops significantly
            if (cell_voltage < float_voltage - 0.1f && current_ > config_.absorption_current_threshold) {
                charging_stage_ = ChargingStage::BULK;
                stage_start_time_ = now;
            }
            break;
            
        case ChargingStage::FAULT:
            // Stay in fault until cleared
            break;
            
        default:
            charging_stage_ = ChargingStage::IDLE;
            break;
    }
}

void BatteryManager::checkProtection() {
    float cell_voltage = voltage_ / config_.cell_count;
    
    // Over-voltage protection
    if (cell_voltage > config_.charge_voltage + OVER_VOLTAGE_MARGIN) {
        current_fault_ = BatteryFault::OVER_VOLTAGE;
        Serial.printf("[Battery] FAULT: Over-voltage %.2fV\n", cell_voltage);
        return;
    }
    
    // Under-voltage protection
    if (cell_voltage < config_.discharge_cutoff - UNDER_VOLTAGE_MARGIN) {
        current_fault_ = BatteryFault::UNDER_VOLTAGE;
        Serial.printf("[Battery] FAULT: Under-voltage %.2fV\n", cell_voltage);
        return;
    }
    
    // Over-current protection (charging)
    if (current_ > config_.max_charge_current * CURRENT_FAULT_MARGIN) {
        current_fault_ = BatteryFault::OVER_CURRENT;
        Serial.printf("[Battery] FAULT: Over-current %.0fmA\n", current_);
        return;
    }
    
    // Over-current protection (discharging)
    if (current_ < -config_.max_discharge_current * CURRENT_FAULT_MARGIN) {
        current_fault_ = BatteryFault::OVER_CURRENT;
        Serial.printf("[Battery] FAULT: Over-discharge current %.0fmA\n", -current_);
        return;
    }
    
    // Temperature protection
    if (temperature_ > config_.max_temperature) {
        current_fault_ = BatteryFault::OVER_TEMPERATURE;
        Serial.printf("[Battery] FAULT: Over-temperature %.1f°C\n", temperature_);
        return;
    }
    
    if (temperature_ < config_.min_temperature && current_ > 0) {
        // Only fault on low temp during charging
        current_fault_ = BatteryFault::UNDER_TEMPERATURE;
        Serial.printf("[Battery] FAULT: Under-temperature %.1f°C\n", temperature_);
        return;
    }
}

BatteryHealth BatteryManager::assessHealth() {
    // Estimate state of health based on:
    // 1. Cycle count
    // 2. Capacity fade
    // 3. Internal resistance (if measurable)
    
    // Typical Li-ion: 300-500 cycles to 80% capacity
    float cycle_health = 100.0f - (cycle_count_ / 5.0f);  // ~500 cycles to 0%
    if (cycle_health < 0) cycle_health = 0;
    
    // Capacity-based health
    float capacity_health = (coulomb_counter_ / config_.nominal_capacity) * 100.0f;
    
    // Weighted average
    state_of_health_ = (cycle_health * 0.5f) + (capacity_health * 0.5f);
    
    // Determine health category
    if (state_of_health_ > 90) return BatteryHealth::EXCELLENT;
    if (state_of_health_ > 70) return BatteryHealth::GOOD;
    if (state_of_health_ > 50) return BatteryHealth::FAIR;
    if (state_of_health_ > 30) return BatteryHealth::POOR;
    return BatteryHealth::REPLACE;
}

void BatteryManager::updateCoulombCounter(float current, float dt) {
    // Coulomb counting: integrate current over time
    // Current is in mA, dt is in seconds
    // mAh = mA * hours = mA * (seconds / 3600)
    
    float mah_delta = current * (dt / 3600.0f);
    coulomb_counter_ += mah_delta;
    
    // Clamp to valid range
    if (coulomb_counter_ < 0) {
        coulomb_counter_ = 0;
    }
    if (coulomb_counter_ > config_.nominal_capacity * 1.1f) {
        coulomb_counter_ = config_.nominal_capacity;
    }
}

float BatteryManager::getTemperatureCompensatedVoltage(float base_voltage) {
    // Temperature compensation for charging voltage
    // Typically -3mV/°C per cell for Li-ion
    float temp_delta = temperature_ - 25.0f;  // Reference is 25°C
    float compensation = config_.temp_compensation * temp_delta / 1000.0f;  // Convert mV to V
    
    return base_voltage + compensation;
}
