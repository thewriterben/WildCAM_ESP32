/**
 * @file satellite_config.h
 * @brief Configuration constants and settings for satellite communication
 */

#ifndef SATELLITE_CONFIG_H
#define SATELLITE_CONFIG_H

#include <Arduino.h>

// Hardware configuration
#define SATELLITE_BAUD_RATE 19200
#define SAT_RX_PIN 25
#define SAT_TX_PIN 26
#define SAT_SLEEP_PIN 27
#define SAT_RING_PIN 33

// Timing constants (milliseconds)
#define SERIAL_INIT_DELAY 100
#define SATELLITE_RETRY_COUNT 3
#define SATELLITE_RESPONSE_DELAY 2000
#define SATELLITE_WAKEUP_DELAY 5000
#define COMMUNICATION_SETUP_DELAY 3000
#define NETWORK_RETRY_DELAY 5000
#define LORA_TRANSMISSION_DELAY 1000

// Message size limits
#define SATELLITE_MESSAGE_MAX_LENGTH 160  // Iridium SBD limit
#define SATELLITE_THUMBNAIL_MAX_SIZE 140  // Leave room for headers
#define SWARM_MESSAGE_MAX_LENGTH 192      // Swarm limit

// Daily limits and cost management
#define DEFAULT_DAILY_MESSAGE_LIMIT 50
#define EMERGENCY_DAILY_LIMIT 10
#define IRIDIUM_COST_PER_MESSAGE 0.95    // USD
#define SWARM_COST_PER_MESSAGE 0.05      // USD

// Power management
#define SATELLITE_IDLE_TIMEOUT 300000     // 5 minutes
#define SATELLITE_TRANSMISSION_POWER 2.8  // Watts
#define LOW_BATTERY_SATELLITE_THRESHOLD 20 // Percentage

// Retry logic
#define RETRY_BASE_DELAY 30000            // 30 seconds base delay
#define RETRY_MAX_DELAY 1800000           // 30 minutes max delay
#define RETRY_MULTIPLIER 2                // Exponential backoff multiplier
#define MAX_RETRY_ATTEMPTS 5

// Satellite pass prediction constants
#define IRIDIUM_ORBIT_PERIOD 6000         // ~100 minutes in seconds
#define SWARM_ORBIT_PERIOD 5400           // ~90 minutes in seconds
#define PASS_PREDICTION_WINDOW 7200       // 2 hours prediction window

// Configuration structure for satellite settings
struct SatelliteConfig {
    bool enabled = false;
    SatelliteModule module = MODULE_NONE;
    uint16_t transmissionInterval = 3600;     // Seconds between scheduled transmissions
    uint16_t emergencyInterval = 300;         // Seconds for emergency transmissions
    uint8_t maxDailyMessages = DEFAULT_DAILY_MESSAGE_LIMIT;
    uint8_t emergencyThreshold = 90;          // Battery percentage for emergency mode
    bool costOptimization = true;
    bool autoPassPrediction = true;
    float maxDailyCost = 25.0;                // USD
    bool prioritizeEmergency = true;
    bool enableMessageCompression = true;
    bool useScheduledTransmission = true;
};

// Message priority levels for cost optimization
enum SatelliteMessagePriority {
    SAT_PRIORITY_LOW = 0,        // Send only during optimal windows
    SAT_PRIORITY_NORMAL = 1,     // Send during normal windows
    SAT_PRIORITY_HIGH = 2,       // Send immediately if possible
    SAT_PRIORITY_EMERGENCY = 3   // Send immediately regardless of cost
};

// Transmission window types
enum TransmissionWindow {
    WINDOW_IMMEDIATE = 0,        // Send now
    WINDOW_NEXT_PASS = 1,        // Wait for next satellite pass
    WINDOW_SCHEDULED = 2,        // Wait for scheduled transmission time
    WINDOW_EMERGENCY = 3         // Emergency transmission
};

#endif // SATELLITE_CONFIG_H