/**
 * @file mppt_controller.cpp
 * @brief Maximum Power Point Tracking controller implementation
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 3.0.0
 * 
 * Implements MPPT algorithms for efficient solar energy harvesting:
 * - Perturb & Observe (P&O) algorithm
 * - Incremental Conductance (IC) algorithm
 * - Constant Voltage (CV) algorithm
 */

#include "mppt_controller.h"
#include <esp_adc_cal.h>

// ADC Constants
static const int ADC_MAX_VALUE = 4095;
static const float ADC_REF_VOLTAGE = 3.3f;
static const int PWM_RESOLUTION = 8;
static const int PWM_MAX_VALUE = 255;
static const int PWM_FREQUENCY = 50000;  // 50kHz PWM for MPPT
static const int PWM_CHANNEL = 0;

// MPPT Constants
static const float MIN_SOLAR_VOLTAGE = 0.5f;        // Minimum voltage to consider charging
static const float MIN_SOLAR_CURRENT = 10.0f;       // Minimum current (mA) to consider charging
static const float PERTURB_STEP_DEFAULT = 0.05f;    // Default perturbation step (V)
static const float PERTURB_STEP_MIN = 0.01f;        // Minimum perturbation step
static const float PERTURB_STEP_MAX = 0.2f;         // Maximum perturbation step
static const float MPP_TOLERANCE = 0.02f;           // 2% tolerance for MPP detection
static const float IC_TOLERANCE = 0.001f;           // Tolerance for IC algorithm
static const float CV_RATIO = 0.76f;                // Open-circuit to MPP voltage ratio

// Energy accounting
static const unsigned long ENERGY_UPDATE_INTERVAL = 1000;  // 1 second
static const unsigned long DAY_MILLISECONDS = 86400000UL;  // 24 hours

MPPTController::MPPTController(int solar_v_pin, int solar_c_pin, int battery_v_pin, 
                               int battery_c_pin, int pwm_pin, MPPTAlgorithm algorithm)
    : solar_voltage_pin_(solar_v_pin), solar_current_pin_(solar_c_pin),
      battery_voltage_pin_(battery_v_pin), battery_current_pin_(battery_c_pin),
      pwm_control_pin_(pwm_pin), algorithm_(algorithm),
      reference_voltage_(0), max_power_voltage_(0), perturb_step_(PERTURB_STEP_DEFAULT),
      efficiency_threshold_(0.85f), previous_power_(0), previous_voltage_(0),
      previous_current_(0), perturb_direction_(true), cloud_factor_(1.0f),
      temperature_coefficient_(1.0f), last_mppt_update_(0), mppt_update_interval_(100),
      daily_energy_counter_(0), energy_reset_time_(0),
      voltage_calibration_factor_(1.0f), current_calibration_factor_(1.0f),
      mppt_enabled_(true), current_duty_cycle_(128), last_energy_update_(0) {
}

bool MPPTController::begin() {
    // Configure ADC pins for input
    pinMode(solar_voltage_pin_, INPUT);
    pinMode(solar_current_pin_, INPUT);
    pinMode(battery_voltage_pin_, INPUT);
    pinMode(battery_current_pin_, INPUT);
    
    // Configure PWM for charge control
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(pwm_control_pin_, PWM_CHANNEL);
    
    // Set initial duty cycle (50%)
    setPWMDutyCycle(128);
    
    // Initialize ADC calibration
    analogReadResolution(12);  // 12-bit resolution
    analogSetAttenuation(ADC_11db);  // Full range
    
    // Record initialization time
    energy_reset_time_ = millis();
    last_mppt_update_ = millis();
    last_energy_update_ = millis();
    
    // Take initial readings
    previous_voltage_ = readSolarVoltage();
    previous_current_ = readSolarCurrent();
    previous_power_ = previous_voltage_ * previous_current_ / 1000.0f;  // mW to W
    
    Serial.println("[MPPT] Controller initialized");
    Serial.printf("[MPPT] Algorithm: %d, PWM pin: %d\n", 
                  static_cast<int>(algorithm_), pwm_control_pin_);
    
    return true;
}

void MPPTController::update() {
    if (!mppt_enabled_) {
        return;
    }
    
    unsigned long now = millis();
    
    // Check if it's time for MPPT update
    if (now - last_mppt_update_ < mppt_update_interval_) {
        return;
    }
    last_mppt_update_ = now;
    
    // Check daylight conditions
    if (!isDaylight()) {
        // No solar input, disable charging
        setPWMDutyCycle(0);
        return;
    }
    
    // Execute selected MPPT algorithm
    switch (algorithm_) {
        case MPPTAlgorithm::PERTURB_OBSERVE:
            executeP_O_Algorithm();
            break;
        case MPPTAlgorithm::INCREMENTAL_CONDUCTANCE:
            executeIC_Algorithm();
            break;
        case MPPTAlgorithm::CONSTANT_VOLTAGE:
            executeCV_Algorithm();
            break;
    }
    
    // Update energy accounting
    float current_power = readSolarVoltage() * readSolarCurrent() / 1000.0f;
    updateEnergyAccounting(current_power);
    
    // Check for daily reset
    if (now - energy_reset_time_ >= DAY_MILLISECONDS) {
        resetDailyEnergy();
    }
}

SolarPowerStatus MPPTController::getStatus() {
    SolarPowerStatus status;
    
    status.solar_voltage = readSolarVoltage();
    status.solar_current = readSolarCurrent();
    status.solar_power = status.solar_voltage * status.solar_current;  // mW
    status.battery_voltage = readBatteryVoltage();
    status.battery_current = readBatteryCurrent();
    status.daily_energy_harvest = daily_energy_counter_;
    status.is_daylight = isDaylight();
    status.cloud_cover_estimate = estimateCloudCover();
    
    // Calculate charging efficiency
    if (status.solar_power > 0) {
        float battery_power = status.battery_voltage * status.battery_current;
        status.charge_efficiency = (battery_power / status.solar_power) * 100.0f;
        if (status.charge_efficiency > 100.0f) {
            status.charge_efficiency = 100.0f;  // Cap at 100%
        }
    } else {
        status.charge_efficiency = 0.0f;
    }
    
    // Determine charging status
    status.is_charging = (status.solar_current > MIN_SOLAR_CURRENT) && 
                        (status.battery_current > 0) &&
                        status.is_daylight;
    
    return status;
}

void MPPTController::updateWeatherConditions(int cloud_cover, float temperature) {
    // Cloud factor: reduces expected power based on cloud cover
    cloud_factor_ = (100.0f - static_cast<float>(cloud_cover)) / 100.0f;
    
    // Temperature coefficient: adjust for solar panel temperature effects
    // Typical silicon PV: -0.4% per °C above 25°C
    float temp_delta = temperature - 25.0f;
    temperature_coefficient_ = 1.0f - (temp_delta * 0.004f);
    
    // Clamp coefficient to reasonable range
    if (temperature_coefficient_ < 0.5f) temperature_coefficient_ = 0.5f;
    if (temperature_coefficient_ > 1.1f) temperature_coefficient_ = 1.1f;
    
    // Adjust MPPT update interval based on conditions
    // Faster updates in variable conditions, slower in stable conditions
    if (cloud_cover > 50) {
        mppt_update_interval_ = 50;   // Fast tracking in cloudy conditions
    } else if (cloud_cover > 20) {
        mppt_update_interval_ = 100;  // Normal tracking
    } else {
        mppt_update_interval_ = 200;  // Slow tracking in clear conditions
    }
}

void MPPTController::calibrateSensors(float v_factor, float c_factor) {
    voltage_calibration_factor_ = v_factor;
    current_calibration_factor_ = c_factor;
    
    Serial.printf("[MPPT] Calibration updated - V: %.3f, I: %.3f\n", 
                  v_factor, c_factor);
}

void MPPTController::resetDailyEnergy() {
    daily_energy_counter_ = 0;
    energy_reset_time_ = millis();
    Serial.println("[MPPT] Daily energy counter reset");
}

bool MPPTController::isAtMaxPowerPoint() const {
    // Read current power
    float voltage = const_cast<MPPTController*>(this)->readSolarVoltage();
    float current = const_cast<MPPTController*>(this)->readSolarCurrent();
    float current_power = voltage * current / 1000.0f;
    
    // Check if we're within tolerance of max power
    if (previous_power_ > 0) {
        float power_diff = abs(current_power - previous_power_) / previous_power_;
        return power_diff < MPP_TOLERANCE;
    }
    
    return false;
}

float MPPTController::getChargingEfficiency() const {
    float solar_power = const_cast<MPPTController*>(this)->readSolarVoltage() * 
                       const_cast<MPPTController*>(this)->readSolarCurrent();
    float battery_power = const_cast<MPPTController*>(this)->readBatteryVoltage() * 
                         const_cast<MPPTController*>(this)->readBatteryCurrent();
    
    if (solar_power > 0) {
        float efficiency = (battery_power / solar_power) * 100.0f;
        return (efficiency > 100.0f) ? 100.0f : efficiency;
    }
    
    return 0.0f;
}

void MPPTController::setMPPTEnabled(bool enabled) {
    mppt_enabled_ = enabled;
    
    if (!enabled) {
        // Disable PWM output when MPPT is disabled
        setPWMDutyCycle(0);
    }
    
    Serial.printf("[MPPT] MPPT %s\n", enabled ? "enabled" : "disabled");
}

float MPPTController::readSolarVoltage() {
    // Read ADC value with averaging for stability
    uint32_t sum = 0;
    const int samples = 8;
    
    for (int i = 0; i < samples; i++) {
        sum += analogRead(solar_voltage_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / samples;
    
    // Convert to voltage (assuming voltage divider for higher voltages)
    // Typical solar panel: 6V-24V, needs voltage divider to fit ADC range
    float voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE * voltage_calibration_factor_;
    
    // Apply weather compensation
    voltage *= cloud_factor_ * temperature_coefficient_;
    
    return voltage;
}

float MPPTController::readSolarCurrent() {
    // Read ADC value with averaging
    uint32_t sum = 0;
    const int samples = 8;
    
    for (int i = 0; i < samples; i++) {
        sum += analogRead(solar_current_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / samples;
    
    // Convert to current (mA)
    // Assuming current sense resistor or Hall sensor
    float current = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE * 
                   current_calibration_factor_ * 1000.0f;  // Convert to mA
    
    return current;
}

float MPPTController::readBatteryVoltage() {
    // Read ADC value with averaging
    uint32_t sum = 0;
    const int samples = 8;
    
    for (int i = 0; i < samples; i++) {
        sum += analogRead(battery_voltage_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / samples;
    
    // Convert to voltage
    float voltage = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE * 2.0f;  // Assuming 2:1 divider
    
    return voltage;
}

float MPPTController::readBatteryCurrent() {
    // Read ADC value with averaging
    uint32_t sum = 0;
    const int samples = 8;
    
    for (int i = 0; i < samples; i++) {
        sum += analogRead(battery_current_pin_);
    }
    
    float adc_value = static_cast<float>(sum) / samples;
    
    // Convert to current (mA)
    float current = (adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE * 1000.0f;
    
    return current;
}

void MPPTController::setPWMDutyCycle(int duty_cycle) {
    // Clamp duty cycle to valid range
    if (duty_cycle < 0) duty_cycle = 0;
    if (duty_cycle > PWM_MAX_VALUE) duty_cycle = PWM_MAX_VALUE;
    
    current_duty_cycle_ = duty_cycle;
    ledcWrite(PWM_CHANNEL, duty_cycle);
}

void MPPTController::executeP_O_Algorithm() {
    // Read current values
    float voltage = readSolarVoltage();
    float current = readSolarCurrent();
    float power = voltage * current / 1000.0f;  // Convert to Watts
    
    // Calculate power change
    float delta_power = power - previous_power_;
    float delta_voltage = voltage - previous_voltage_;
    
    // Adaptive step size based on power change
    if (abs(delta_power) > 0.1f) {
        // Large power change - use larger steps
        perturb_step_ = PERTURB_STEP_MAX;
    } else if (abs(delta_power) < 0.01f) {
        // Small power change - use smaller steps (near MPP)
        perturb_step_ = PERTURB_STEP_MIN;
    } else {
        // Medium power change - use default step
        perturb_step_ = PERTURB_STEP_DEFAULT;
    }
    
    // P&O Algorithm decision logic
    if (delta_power > 0) {
        // Power increased - continue in same direction
        if (delta_voltage > 0) {
            // Voltage increased and power increased - increase voltage more
            perturb_direction_ = true;
        } else {
            // Voltage decreased and power increased - decrease voltage more
            perturb_direction_ = false;
        }
    } else {
        // Power decreased - reverse direction
        if (delta_voltage > 0) {
            // Voltage increased but power decreased - decrease voltage
            perturb_direction_ = false;
        } else {
            // Voltage decreased but power decreased - increase voltage
            perturb_direction_ = true;
        }
    }
    
    // Apply perturbation by adjusting duty cycle
    int duty_change = static_cast<int>(perturb_step_ * 10);  // Convert voltage step to duty cycle
    
    if (perturb_direction_) {
        current_duty_cycle_ += duty_change;
    } else {
        current_duty_cycle_ -= duty_change;
    }
    
    // Apply new duty cycle
    setPWMDutyCycle(current_duty_cycle_);
    
    // Store values for next iteration
    previous_power_ = power;
    previous_voltage_ = voltage;
    previous_current_ = current;
}

void MPPTController::executeIC_Algorithm() {
    // Read current values
    float voltage = readSolarVoltage();
    float current = readSolarCurrent();
    
    // Calculate changes
    float delta_voltage = voltage - previous_voltage_;
    float delta_current = current - previous_current_;
    
    // Avoid division by zero
    if (abs(delta_voltage) < 0.001f) {
        delta_voltage = 0.001f;
    }
    if (voltage < 0.001f) {
        voltage = 0.001f;
    }
    
    // Calculate conductance and incremental conductance
    float conductance = current / voltage;
    float incremental_conductance = delta_current / delta_voltage;
    
    // IC Algorithm decision logic
    // At MPP: dI/dV = -I/V
    // Left of MPP: dI/dV > -I/V
    // Right of MPP: dI/dV < -I/V
    
    if (abs(incremental_conductance + conductance) < IC_TOLERANCE) {
        // At MPP - maintain current duty cycle
    } else if (incremental_conductance > -conductance) {
        // Left of MPP - increase voltage (decrease duty cycle)
        current_duty_cycle_ -= 1;
    } else {
        // Right of MPP - decrease voltage (increase duty cycle)
        current_duty_cycle_ += 1;
    }
    
    // Apply new duty cycle
    setPWMDutyCycle(current_duty_cycle_);
    
    // Store values for next iteration
    previous_voltage_ = voltage;
    previous_current_ = current;
    previous_power_ = voltage * current / 1000.0f;
}

void MPPTController::executeCV_Algorithm() {
    // Read current voltage
    float voltage = readSolarVoltage();
    
    // Static variables for non-blocking Voc measurement
    static unsigned long last_voc_check = 0;
    static unsigned long voc_settle_start = 0;
    static bool voc_settling = false;
    unsigned long now = millis();
    
    // Non-blocking Voc measurement state machine
    if (voc_settling) {
        // Wait for voltage to settle (10ms)
        if (now - voc_settle_start >= 10) {
            float voc = readSolarVoltage();
            reference_voltage_ = voc * CV_RATIO;  // Target voltage is ~76% of Voc
            max_power_voltage_ = reference_voltage_;
            
            last_voc_check = now;
            voc_settling = false;
            Serial.printf("[MPPT] CV: Voc=%.2fV, Vmpp=%.2fV\n", voc, reference_voltage_);
        }
        return;  // Don't update duty cycle while settling
    }
    
    // Periodically check open-circuit voltage (every 5 minutes)
    if (reference_voltage_ == 0 || (now - last_voc_check > 300000)) {
        // Temporarily disable charging to measure Voc
        setPWMDutyCycle(0);
        voc_settle_start = now;
        voc_settling = true;
        return;  // Wait for next update to read Voc
    }
    
    // Simple proportional control to maintain reference voltage
    float error = reference_voltage_ - voltage;
    int duty_adjustment = static_cast<int>(error * 10);  // P-controller gain
    
    current_duty_cycle_ += duty_adjustment;
    setPWMDutyCycle(current_duty_cycle_);
    
    // Store values
    previous_voltage_ = voltage;
    previous_current_ = readSolarCurrent();
    previous_power_ = previous_voltage_ * previous_current_ / 1000.0f;
}

void MPPTController::updateEnergyAccounting(float power) {
    unsigned long now = millis();
    unsigned long elapsed = now - last_energy_update_;
    
    if (elapsed >= ENERGY_UPDATE_INTERVAL) {
        // Calculate energy in Wh: Power (W) * Time (h)
        float hours = static_cast<float>(elapsed) / 3600000.0f;  // ms to hours
        float energy = power * hours;
        
        daily_energy_counter_ += energy;
        last_energy_update_ = now;
    }
}

bool MPPTController::isDaylight() {
    float voltage = readSolarVoltage();
    float current = readSolarCurrent();
    
    // Consider it daylight if there's meaningful solar input
    return (voltage > MIN_SOLAR_VOLTAGE) && (current > MIN_SOLAR_CURRENT);
}

int MPPTController::estimateCloudCover() {
    // Compare current power to expected max power
    float current_power = readSolarVoltage() * readSolarCurrent();
    
    // Estimate expected max power based on time of day and panel specs
    // This is a simplified estimation
    float expected_max = 5000.0f;  // 5W typical small panel max
    
    if (expected_max > 0 && current_power > 0) {
        float ratio = current_power / expected_max;
        if (ratio > 1.0f) ratio = 1.0f;
        
        // Convert ratio to cloud cover percentage
        int cloud_cover = static_cast<int>((1.0f - ratio) * 100);
        return cloud_cover;
    }
    
    return 100;  // Assume full cloud cover if no power
}
