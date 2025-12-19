/**
 * @file adaptive_sleep_scheduler.h
 * @brief Adaptive sleep scheduler based on motion patterns for power optimization
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 1.0.0
 * 
 * This module analyzes historical motion patterns to intelligently schedule
 * deep sleep periods, maximizing battery life while minimizing missed detections.
 * 
 * Features:
 * - Time-of-day activity pattern learning
 * - Motion frequency analysis
 * - Adaptive sleep duration adjustment
 * - Battery-aware scheduling
 * - Seasonal pattern adaptation
 */

#ifndef ADAPTIVE_SLEEP_SCHEDULER_H
#define ADAPTIVE_SLEEP_SCHEDULER_H

#include <Arduino.h>

/**
 * @brief Number of time slots per day (hourly granularity)
 */
static const int TIME_SLOTS_PER_DAY = 24;

/**
 * @brief Number of days to track for pattern analysis
 */
static const int PATTERN_HISTORY_DAYS = 7;

/**
 * @brief Sleep schedule aggressiveness level
 */
enum class SleepAggressiveness {
    CONSERVATIVE,   // Minimize missed detections, shorter sleep periods
    BALANCED,       // Balance between power saving and detection
    AGGRESSIVE,     // Maximize power saving, may miss some activity
    ULTRA_AGGRESSIVE // Maximum power saving, for critical battery situations
};

/**
 * @brief Motion pattern statistics for a time slot
 */
struct MotionSlotStats {
    uint16_t total_events;      // Total motion events in this slot historically
    uint16_t avg_events;        // Average events per occurrence
    uint8_t active_days;        // Number of days with activity in this slot
    uint8_t consecutive_idle;   // Consecutive days without activity
    float activity_probability; // Probability of activity (0.0-1.0)
};

/**
 * @brief Sleep schedule entry
 */
struct SleepScheduleEntry {
    uint8_t hour;               // Hour of day (0-23)
    uint32_t sleep_duration_ms; // Recommended sleep duration in milliseconds
    uint32_t wake_interval_ms;  // Wake interval for quick checks
    float detection_probability;// Expected detection probability
    bool allow_extended_sleep;  // Whether extended sleep is safe
};

/**
 * @brief Daily motion summary
 */
struct DailyMotionSummary {
    uint32_t date;              // Date as days since epoch
    uint16_t events_per_hour[TIME_SLOTS_PER_DAY];  // Events per hour
    uint16_t total_events;      // Total events for the day
    uint8_t peak_hour;          // Hour with most activity
    uint8_t quiet_hours_count;  // Number of hours with no activity
};

/**
 * @brief Scheduler configuration
 */
struct SchedulerConfig {
    SleepAggressiveness aggressiveness;
    uint32_t min_sleep_duration_ms;      // Minimum sleep duration
    uint32_t max_sleep_duration_ms;      // Maximum sleep duration
    uint32_t default_sleep_duration_ms;  // Default when no pattern data
    float battery_low_threshold;         // Battery % to trigger aggressive mode
    float battery_critical_threshold;    // Battery % for ultra-aggressive mode
    bool enable_seasonal_adjustment;     // Adjust for seasonal patterns
    bool enable_weather_adjustment;      // Adjust based on weather conditions
};

/**
 * @brief Adaptive sleep scheduler class
 */
class AdaptiveSleepScheduler {
public:
    /**
     * @brief Constructor
     */
    AdaptiveSleepScheduler();

    /**
     * @brief Initialize the scheduler
     * @param config Scheduler configuration
     * @return true if initialization successful
     */
    bool begin(const SchedulerConfig& config = SchedulerConfig());

    /**
     * @brief Initialize with default configuration
     * @return true if initialization successful
     */
    bool begin();

    /**
     * @brief Update scheduler state - call regularly
     */
    void update();

    /**
     * @brief Record a motion detection event
     * @param timestamp Timestamp of the event (millis since boot or epoch)
     */
    void recordMotionEvent(unsigned long timestamp = 0);

    /**
     * @brief Get recommended sleep duration for current time
     * @return Sleep duration in milliseconds
     */
    uint32_t getRecommendedSleepDuration();

    /**
     * @brief Get recommended sleep duration for specific hour
     * @param hour Hour of day (0-23)
     * @return Sleep duration in milliseconds
     */
    uint32_t getRecommendedSleepDuration(uint8_t hour);

    /**
     * @brief Get full schedule for the day
     * @param schedule Array to fill with schedule entries (size TIME_SLOTS_PER_DAY)
     */
    void getDailySchedule(SleepScheduleEntry* schedule);

    /**
     * @brief Get motion statistics for a time slot
     * @param hour Hour of day (0-23)
     * @return Motion statistics for the slot
     */
    MotionSlotStats getSlotStatistics(uint8_t hour);

    /**
     * @brief Check if current time is typically active
     * @return true if current hour typically has motion activity
     */
    bool isCurrentlyActiveTime();

    /**
     * @brief Check if specific hour is typically active
     * @param hour Hour to check (0-23)
     * @return true if hour typically has motion activity
     */
    bool isActiveTime(uint8_t hour);

    /**
     * @brief Set aggressiveness level
     * @param level Aggressiveness level
     */
    void setAggressiveness(SleepAggressiveness level);

    /**
     * @brief Get current aggressiveness level
     * @return Current aggressiveness level
     */
    SleepAggressiveness getAggressiveness() const { return config_.aggressiveness; }

    /**
     * @brief Update battery level for adaptive scheduling
     * @param battery_percentage Current battery percentage (0-100)
     */
    void updateBatteryLevel(float battery_percentage);

    /**
     * @brief Set current time (hour and minute)
     * @param hour Current hour (0-23)
     * @param minute Current minute (0-59)
     */
    void setCurrentTime(uint8_t hour, uint8_t minute);

    /**
     * @brief Get current configuration
     * @return Current scheduler configuration
     */
    SchedulerConfig getConfiguration() const { return config_; }

    /**
     * @brief Set configuration
     * @param config New configuration
     */
    void setConfiguration(const SchedulerConfig& config);

    /**
     * @brief Reset all learned patterns
     */
    void resetPatterns();

    /**
     * @brief Get total motion events today
     * @return Number of motion events today
     */
    uint16_t getTodayMotionCount() const { return today_motion_count_; }

    /**
     * @brief Get peak activity hour
     * @return Hour with highest average activity (0-23)
     */
    uint8_t getPeakActivityHour() const;

    /**
     * @brief Get quietest hours for extended sleep
     * @param hours Array to fill with quiet hours
     * @param max_hours Maximum hours to return
     * @return Number of quiet hours found
     */
    int getQuietHours(uint8_t* hours, int max_hours);

    /**
     * @brief Calculate activity score for current conditions
     * @return Activity score (0.0-1.0, higher = more active)
     */
    float getActivityScore();

    /**
     * @brief Print scheduler status to Serial
     */
    void printStatus();

    /**
     * @brief Print daily pattern to Serial
     */
    void printDailyPattern();

    /**
     * @brief Get default configuration
     * @return Default SchedulerConfig
     */
    static SchedulerConfig getDefaultConfig();

private:
    // Configuration
    SchedulerConfig config_;
    bool initialized_;

    // Time tracking
    uint8_t current_hour_;
    uint8_t current_minute_;
    uint8_t last_recorded_hour_;
    unsigned long last_update_;

    // Pattern data (hourly bins for the week)
    uint16_t hourly_patterns_[PATTERN_HISTORY_DAYS][TIME_SLOTS_PER_DAY];
    uint8_t current_day_index_;
    uint16_t today_motion_count_;

    // Long-term averages
    float hourly_averages_[TIME_SLOTS_PER_DAY];
    float hourly_variance_[TIME_SLOTS_PER_DAY];

    // Battery state
    float battery_percentage_;
    SleepAggressiveness effective_aggressiveness_;

    // Cached calculations
    unsigned long last_pattern_analysis_;
    bool patterns_dirty_;

    /**
     * @brief Analyze patterns and update statistics
     */
    void analyzePatterns();

    /**
     * @brief Calculate sleep duration based on slot statistics
     * @param stats Slot statistics
     * @return Recommended sleep duration in milliseconds
     */
    uint32_t calculateSleepDuration(const MotionSlotStats& stats);

    /**
     * @brief Update effective aggressiveness based on battery
     */
    void updateEffectiveAggressiveness();

    /**
     * @brief Start a new day in pattern tracking
     */
    void startNewDay();

    /**
     * @brief Apply battery multiplier to sleep duration
     * @param base_duration Base sleep duration
     * @return Adjusted sleep duration
     */
    uint32_t applyBatteryMultiplier(uint32_t base_duration);

    /**
     * @brief Calculate activity probability for a slot
     * @param hour Hour to calculate for
     * @return Probability (0.0-1.0)
     */
    float calculateActivityProbability(uint8_t hour);

    /**
     * @brief Initialize default configuration
     */
    void initializeDefaultConfig();
};

#endif // ADAPTIVE_SLEEP_SCHEDULER_H
