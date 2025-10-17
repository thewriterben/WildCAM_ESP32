/**
 * @file power_save_mode_example.cpp
 * @brief Example demonstrating power save mode behavior
 * @author WildCAM ESP32 Team
 * @date 2025-10-16
 * @version 1.0.0
 * 
 * This example demonstrates how the power save mode activates and deactivates
 * based on battery voltage levels.
 */

#include <Arduino.h>

// Simulated system state
struct SystemState {
    bool power_save_mode = false;
    float battery_voltage = 3.7f;
    bool network_connected = true;
};

SystemState system_state;
uint32_t deep_sleep_duration = 300; // seconds

// Mock functions
void setCpuFrequencyMhz(uint8_t freq) {
    Serial.printf("CPU frequency set to %d MHz\n", freq);
}

void WiFi_disconnect() {
    Serial.println("WiFi disconnected");
}

void WiFi_mode_off() {
    Serial.println("WiFi mode set to OFF");
}

/**
 * @brief Simulate power management behavior
 */
void simulatePowerManagement() {
    Serial.println("\n=== Power Save Mode Demonstration ===\n");
    
    // Scenario 1: Normal operation
    Serial.println("Scenario 1: Normal operation (Battery: 3.7V)");
    system_state.battery_voltage = 3.7f;
    Serial.printf("Battery: %.2fV, Power Save: %s\n", 
                  system_state.battery_voltage,
                  system_state.power_save_mode ? "ON" : "OFF");
    Serial.printf("CPU: 240MHz, Sleep: %ds, WiFi: %s\n\n",
                  deep_sleep_duration,
                  system_state.network_connected ? "ON" : "OFF");
    
    // Scenario 2: Battery drops below 3.0V - enter power save mode
    Serial.println("Scenario 2: Battery drops (Battery: 2.8V)");
    system_state.battery_voltage = 2.8f;
    
    if (system_state.battery_voltage < 3.0f && !system_state.power_save_mode) {
        Serial.printf("⚠️  ENTERING POWER SAVE MODE (battery: %.2fV)\n", 
                      system_state.battery_voltage);
        
        // Reduce CPU frequency
        setCpuFrequencyMhz(80);
        
        // Increase sleep duration
        deep_sleep_duration = 600;
        Serial.printf("Deep sleep duration increased to %d seconds\n", deep_sleep_duration);
        
        // Disable WiFi
        if (system_state.network_connected) {
            WiFi_disconnect();
            WiFi_mode_off();
            system_state.network_connected = false;
        }
        
        system_state.power_save_mode = true;
        Serial.println("✓ Power save mode activated\n");
    }
    
    Serial.printf("Battery: %.2fV, Power Save: %s\n", 
                  system_state.battery_voltage,
                  system_state.power_save_mode ? "ON" : "OFF");
    Serial.printf("CPU: 80MHz, Sleep: %ds, WiFi: %s\n\n",
                  deep_sleep_duration,
                  system_state.network_connected ? "ON" : "OFF");
    
    // Scenario 3: Battery still low - no action
    Serial.println("Scenario 3: Battery still low (Battery: 3.2V)");
    system_state.battery_voltage = 3.2f;
    Serial.println("Battery recovering but below exit threshold (3.4V)");
    Serial.printf("Battery: %.2fV, Power Save: %s (no change)\n\n", 
                  system_state.battery_voltage,
                  system_state.power_save_mode ? "ON" : "OFF");
    
    // Scenario 4: Battery recovers above 3.4V - exit power save mode
    Serial.println("Scenario 4: Battery recovered (Battery: 3.5V)");
    system_state.battery_voltage = 3.5f;
    
    if (system_state.battery_voltage > 3.4f && system_state.power_save_mode) {
        Serial.printf("✓ EXITING POWER SAVE MODE (battery: %.2fV)\n", 
                      system_state.battery_voltage);
        
        // Restore CPU frequency
        setCpuFrequencyMhz(240);
        
        // Restore sleep duration
        deep_sleep_duration = 300;
        Serial.printf("Deep sleep duration restored to %d seconds\n", deep_sleep_duration);
        
        system_state.power_save_mode = false;
        Serial.println("✓ Normal operation resumed\n");
    }
    
    Serial.printf("Battery: %.2fV, Power Save: %s\n", 
                  system_state.battery_voltage,
                  system_state.power_save_mode ? "ON" : "OFF");
    Serial.printf("CPU: 240MHz, Sleep: %ds\n\n",
                  deep_sleep_duration);
    
    Serial.println("=== Demonstration Complete ===\n");
    
    // Show power savings summary
    Serial.println("Power Savings in Power Save Mode:");
    Serial.println("- CPU frequency: 240MHz → 80MHz (67% reduction)");
    Serial.println("- Active time: 50% reduction (300s → 600s sleep)");
    Serial.println("- WiFi: Disabled (significant power saving)");
    Serial.println("- Estimated total power reduction: >40%");
    Serial.println("- Estimated battery life extension: >50%");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\nPower Save Mode Example");
    Serial.println("=======================\n");
    
    Serial.println("This example demonstrates the power save mode behavior:");
    Serial.println("- Enter power save when battery < 3.0V");
    Serial.println("- Exit power save when battery > 3.4V");
    Serial.println("- Hysteresis gap: 0.4V (prevents oscillation)\n");
    
    simulatePowerManagement();
}

void loop() {
    // Nothing here - example runs once
    delay(1000);
}
