/**
 * @file adaptive_sleep_scheduler.cpp
 * @brief Adaptive sleep scheduler implementation
 * @author WildCAM ESP32 Team
 * @date 2025-12-19
 * @version 1.0.0
 */

#include "adaptive_sleep_scheduler.h"
#include <algorithm>
#include <cmath>

// Default timing constants
static const uint32_t MIN_SLEEP_DURATION_MS = 5000;         // 5 seconds minimum
static const uint32_t MAX_SLEEP_DURATION_MS = 3600000;      // 1 hour maximum
static const uint32_t DEFAULT_SLEEP_DURATION_MS = 300000;   // 5 minutes default
static const uint32_t PATTERN_ANALYSIS_INTERVAL = 60000;    // Analyze every minute

// Activity thresholds
static const float HIGH_ACTIVITY_THRESHOLD = 0.7f;          // High activity probability
static const float MEDIUM_ACTIVITY_THRESHOLD = 0.3f;        // Medium activity probability
static const float LOW_ACTIVITY_THRESHOLD = 0.1f;           // Low activity probability

// Sleep duration multipliers for aggressiveness levels
static const float CONSERVATIVE_MULTIPLIER = 0.5f;
static const float BALANCED_MULTIPLIER = 1.0f;
static const float AGGRESSIVE_MULTIPLIER = 2.0f;
static const float ULTRA_AGGRESSIVE_MULTIPLIER = 4.0f;

// Battery-based multipliers
static const float BATTERY_LOW_MULTIPLIER = 1.5f;
static const float BATTERY_CRITICAL_MULTIPLIER = 3.0f;

AdaptiveSleepScheduler::AdaptiveSleepScheduler()
    : initialized_(false), current_hour_(12), current_minute_(0),
      last_recorded_hour_(255), last_update_(0), current_day_index_(0),
      today_motion_count_(0), battery_percentage_(100.0f),
      effective_aggressiveness_(SleepAggressiveness::BALANCED),
      last_pattern_analysis_(0), patterns_dirty_(true) {
    
    // Initialize pattern arrays to zero
    for (int d = 0; d < PATTERN_HISTORY_DAYS; d++) {
        for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
            hourly_patterns_[d][h] = 0;
        }
    }
    
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        hourly_averages_[h] = 0.0f;
        hourly_variance_[h] = 0.0f;
    }
}

bool AdaptiveSleepScheduler::begin(const SchedulerConfig& config) {
    config_ = config;
    effective_aggressiveness_ = config.aggressiveness;
    
    // Validate configuration
    if (config_.min_sleep_duration_ms >= config_.max_sleep_duration_ms) {
        config_.min_sleep_duration_ms = MIN_SLEEP_DURATION_MS;
        config_.max_sleep_duration_ms = MAX_SLEEP_DURATION_MS;
    }
    
    if (config_.default_sleep_duration_ms == 0) {
        config_.default_sleep_duration_ms = DEFAULT_SLEEP_DURATION_MS;
    }
    
    last_update_ = millis();
    last_pattern_analysis_ = millis();
    patterns_dirty_ = true;
    
    initialized_ = true;
    
    Serial.println("[AdaptiveSleep] Scheduler initialized");
    Serial.printf("[AdaptiveSleep] Aggressiveness: %d, Sleep range: %lu-%lu ms\n",
                  static_cast<int>(config_.aggressiveness),
                  config_.min_sleep_duration_ms,
                  config_.max_sleep_duration_ms);
    
    return true;
}

bool AdaptiveSleepScheduler::begin() {
    return begin(getDefaultConfig());
}

void AdaptiveSleepScheduler::update() {
    if (!initialized_) return;
    
    unsigned long now = millis();
    
    // Check if hour has changed
    if (current_hour_ != last_recorded_hour_) {
        // Hour changed - check if day changed
        if (last_recorded_hour_ != 255 && current_hour_ < last_recorded_hour_) {
            startNewDay();
        }
        last_recorded_hour_ = current_hour_;
    }
    
    // Periodic pattern analysis
    if (now - last_pattern_analysis_ >= PATTERN_ANALYSIS_INTERVAL || patterns_dirty_) {
        analyzePatterns();
        last_pattern_analysis_ = now;
        patterns_dirty_ = false;
    }
    
    // Update effective aggressiveness based on battery
    updateEffectiveAggressiveness();
    
    last_update_ = now;
}

void AdaptiveSleepScheduler::recordMotionEvent(unsigned long timestamp) {
    if (!initialized_) return;
    
    // Use current hour if timestamp not provided
    uint8_t hour = current_hour_;
    
    // Increment pattern counter for current day and hour
    if (hour < TIME_SLOTS_PER_DAY) {
        hourly_patterns_[current_day_index_][hour]++;
        today_motion_count_++;
        patterns_dirty_ = true;
        
        Serial.printf("[AdaptiveSleep] Motion recorded at hour %d (total today: %d)\n",
                      hour, today_motion_count_);
    }
}

uint32_t AdaptiveSleepScheduler::getRecommendedSleepDuration() {
    return getRecommendedSleepDuration(current_hour_);
}

uint32_t AdaptiveSleepScheduler::getRecommendedSleepDuration(uint8_t hour) {
    if (!initialized_ || hour >= TIME_SLOTS_PER_DAY) {
        return config_.default_sleep_duration_ms;
    }
    
    // Get statistics for this time slot
    MotionSlotStats stats = getSlotStatistics(hour);
    
    // Calculate base sleep duration
    uint32_t base_duration = calculateSleepDuration(stats);
    
    // Apply battery multiplier
    uint32_t adjusted_duration = applyBatteryMultiplier(base_duration);
    
    // Clamp to configured range
    if (adjusted_duration < config_.min_sleep_duration_ms) {
        adjusted_duration = config_.min_sleep_duration_ms;
    }
    if (adjusted_duration > config_.max_sleep_duration_ms) {
        adjusted_duration = config_.max_sleep_duration_ms;
    }
    
    return adjusted_duration;
}

void AdaptiveSleepScheduler::getDailySchedule(SleepScheduleEntry* schedule) {
    if (!initialized_ || schedule == nullptr) return;
    
    for (uint8_t hour = 0; hour < TIME_SLOTS_PER_DAY; hour++) {
        MotionSlotStats stats = getSlotStatistics(hour);
        
        schedule[hour].hour = hour;
        schedule[hour].sleep_duration_ms = getRecommendedSleepDuration(hour);
        schedule[hour].detection_probability = stats.activity_probability;
        schedule[hour].allow_extended_sleep = (stats.activity_probability < LOW_ACTIVITY_THRESHOLD);
        
        // Set wake interval based on activity probability
        if (stats.activity_probability > HIGH_ACTIVITY_THRESHOLD) {
            schedule[hour].wake_interval_ms = config_.min_sleep_duration_ms;
        } else if (stats.activity_probability > MEDIUM_ACTIVITY_THRESHOLD) {
            schedule[hour].wake_interval_ms = config_.default_sleep_duration_ms / 2;
        } else {
            schedule[hour].wake_interval_ms = config_.default_sleep_duration_ms;
        }
    }
}

MotionSlotStats AdaptiveSleepScheduler::getSlotStatistics(uint8_t hour) {
    MotionSlotStats stats = {0, 0, 0, 0, 0.0f};
    
    if (hour >= TIME_SLOTS_PER_DAY) return stats;
    
    uint32_t total = 0;
    uint8_t active_days = 0;
    uint8_t consecutive_idle = 0;
    bool found_activity = false;
    
    // Analyze historical data for this hour
    for (int d = 0; d < PATTERN_HISTORY_DAYS; d++) {
        int day_idx = (current_day_index_ - d + PATTERN_HISTORY_DAYS) % PATTERN_HISTORY_DAYS;
        uint16_t events = hourly_patterns_[day_idx][hour];
        
        total += events;
        
        if (events > 0) {
            active_days++;
            found_activity = true;
            consecutive_idle = 0;
        } else if (found_activity) {
            consecutive_idle++;
        }
    }
    
    stats.total_events = total;
    stats.active_days = active_days;
    stats.consecutive_idle = consecutive_idle;
    
    if (active_days > 0) {
        stats.avg_events = total / active_days;
    }
    
    // Calculate activity probability
    stats.activity_probability = calculateActivityProbability(hour);
    
    return stats;
}

bool AdaptiveSleepScheduler::isCurrentlyActiveTime() {
    return isActiveTime(current_hour_);
}

bool AdaptiveSleepScheduler::isActiveTime(uint8_t hour) {
    if (hour >= TIME_SLOTS_PER_DAY) return false;
    
    float probability = calculateActivityProbability(hour);
    return probability > MEDIUM_ACTIVITY_THRESHOLD;
}

void AdaptiveSleepScheduler::setAggressiveness(SleepAggressiveness level) {
    config_.aggressiveness = level;
    updateEffectiveAggressiveness();
    
    const char* level_names[] = {"Conservative", "Balanced", "Aggressive", "Ultra-Aggressive"};
    Serial.printf("[AdaptiveSleep] Aggressiveness set to: %s\n", 
                  level_names[static_cast<int>(level)]);
}

void AdaptiveSleepScheduler::updateBatteryLevel(float battery_percentage) {
    battery_percentage_ = battery_percentage;
    updateEffectiveAggressiveness();
}

void AdaptiveSleepScheduler::setCurrentTime(uint8_t hour, uint8_t minute) {
    if (hour < 24) current_hour_ = hour;
    if (minute < 60) current_minute_ = minute;
}

void AdaptiveSleepScheduler::setConfiguration(const SchedulerConfig& config) {
    config_ = config;
    updateEffectiveAggressiveness();
    patterns_dirty_ = true;
    
    Serial.println("[AdaptiveSleep] Configuration updated");
}

void AdaptiveSleepScheduler::resetPatterns() {
    for (int d = 0; d < PATTERN_HISTORY_DAYS; d++) {
        for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
            hourly_patterns_[d][h] = 0;
        }
    }
    
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        hourly_averages_[h] = 0.0f;
        hourly_variance_[h] = 0.0f;
    }
    
    today_motion_count_ = 0;
    current_day_index_ = 0;
    patterns_dirty_ = true;
    
    Serial.println("[AdaptiveSleep] Patterns reset");
}

uint8_t AdaptiveSleepScheduler::getPeakActivityHour() const {
    uint8_t peak_hour = 0;
    float max_average = 0.0f;
    
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        if (hourly_averages_[h] > max_average) {
            max_average = hourly_averages_[h];
            peak_hour = h;
        }
    }
    
    return peak_hour;
}

int AdaptiveSleepScheduler::getQuietHours(uint8_t* hours, int max_hours) {
    if (hours == nullptr || max_hours <= 0) return 0;
    
    // Find hours with lowest activity probability
    float probabilities[TIME_SLOTS_PER_DAY];
    uint8_t indices[TIME_SLOTS_PER_DAY];
    
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        probabilities[h] = calculateActivityProbability(h);
        indices[h] = h;
    }
    
    // Simple bubble sort for small array
    for (int i = 0; i < TIME_SLOTS_PER_DAY - 1; i++) {
        for (int j = 0; j < TIME_SLOTS_PER_DAY - i - 1; j++) {
            if (probabilities[j] > probabilities[j + 1]) {
                float temp_p = probabilities[j];
                probabilities[j] = probabilities[j + 1];
                probabilities[j + 1] = temp_p;
                
                uint8_t temp_i = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp_i;
            }
        }
    }
    
    // Return quietest hours
    int count = (max_hours < TIME_SLOTS_PER_DAY) ? max_hours : TIME_SLOTS_PER_DAY;
    for (int i = 0; i < count; i++) {
        hours[i] = indices[i];
    }
    
    return count;
}

float AdaptiveSleepScheduler::getActivityScore() {
    // Calculate current activity score based on recent patterns
    float current_prob = calculateActivityProbability(current_hour_);
    
    // Weight by time of day factor (animals often more active dawn/dusk)
    float time_factor = 1.0f;
    if ((current_hour_ >= 5 && current_hour_ <= 8) ||
        (current_hour_ >= 17 && current_hour_ <= 20)) {
        time_factor = 1.2f;  // Crepuscular bonus
    }
    
    // Weight by recent activity
    float recent_factor = 1.0f;
    if (today_motion_count_ > 10) {
        recent_factor = 1.3f;  // High activity day
    }
    
    float score = current_prob * time_factor * recent_factor;
    if (score > 1.0f) score = 1.0f;
    
    return score;
}

void AdaptiveSleepScheduler::printStatus() {
    Serial.println("\n========== Adaptive Sleep Scheduler Status ==========");
    Serial.printf("Current Time: %02d:%02d\n", current_hour_, current_minute_);
    Serial.printf("Battery Level: %.1f%%\n", battery_percentage_);
    Serial.printf("Today's Motion Events: %d\n", today_motion_count_);
    Serial.printf("Aggressiveness: %d (Effective: %d)\n", 
                  static_cast<int>(config_.aggressiveness),
                  static_cast<int>(effective_aggressiveness_));
    Serial.printf("Recommended Sleep: %lu ms\n", getRecommendedSleepDuration());
    Serial.printf("Current Activity Score: %.2f\n", getActivityScore());
    Serial.printf("Is Active Time: %s\n", isCurrentlyActiveTime() ? "Yes" : "No");
    Serial.printf("Peak Activity Hour: %02d:00\n", getPeakActivityHour());
    Serial.println("=====================================================\n");
}

void AdaptiveSleepScheduler::printDailyPattern() {
    Serial.println("\n===== Daily Activity Pattern =====");
    Serial.println("Hour | Avg Events | Probability");
    Serial.println("-----|------------|------------");
    
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        float prob = calculateActivityProbability(h);
        Serial.printf(" %02d  |    %6.1f  |    %.2f\n", h, hourly_averages_[h], prob);
    }
    
    Serial.println("==================================\n");
}

SchedulerConfig AdaptiveSleepScheduler::getDefaultConfig() {
    SchedulerConfig config;
    config.aggressiveness = SleepAggressiveness::BALANCED;
    config.min_sleep_duration_ms = MIN_SLEEP_DURATION_MS;
    config.max_sleep_duration_ms = MAX_SLEEP_DURATION_MS;
    config.default_sleep_duration_ms = DEFAULT_SLEEP_DURATION_MS;
    config.battery_low_threshold = 30.0f;
    config.battery_critical_threshold = 15.0f;
    config.enable_seasonal_adjustment = false;
    config.enable_weather_adjustment = false;
    return config;
}

void AdaptiveSleepScheduler::analyzePatterns() {
    // Calculate averages and variance for each hour
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        float sum = 0.0f;
        float sum_sq = 0.0f;
        
        for (int d = 0; d < PATTERN_HISTORY_DAYS; d++) {
            float val = static_cast<float>(hourly_patterns_[d][h]);
            sum += val;
            sum_sq += val * val;
        }
        
        float avg = sum / PATTERN_HISTORY_DAYS;
        float variance = (sum_sq / PATTERN_HISTORY_DAYS) - (avg * avg);
        
        hourly_averages_[h] = avg;
        hourly_variance_[h] = variance;
    }
}

uint32_t AdaptiveSleepScheduler::calculateSleepDuration(const MotionSlotStats& stats) {
    // Base calculation: inverse relationship with activity probability
    float probability = stats.activity_probability;
    
    // High activity = short sleep, low activity = long sleep
    uint32_t base_duration;
    
    if (probability > HIGH_ACTIVITY_THRESHOLD) {
        // Very active time - minimal sleep
        base_duration = config_.min_sleep_duration_ms;
    } else if (probability > MEDIUM_ACTIVITY_THRESHOLD) {
        // Moderately active - short sleep
        float factor = 1.0f - ((probability - MEDIUM_ACTIVITY_THRESHOLD) / 
                               (HIGH_ACTIVITY_THRESHOLD - MEDIUM_ACTIVITY_THRESHOLD));
        base_duration = config_.min_sleep_duration_ms + 
                       static_cast<uint32_t>((config_.default_sleep_duration_ms - 
                                             config_.min_sleep_duration_ms) * factor);
    } else if (probability > LOW_ACTIVITY_THRESHOLD) {
        // Low activity - longer sleep
        float factor = 1.0f - ((probability - LOW_ACTIVITY_THRESHOLD) / 
                               (MEDIUM_ACTIVITY_THRESHOLD - LOW_ACTIVITY_THRESHOLD));
        base_duration = config_.default_sleep_duration_ms + 
                       static_cast<uint32_t>((config_.max_sleep_duration_ms - 
                                             config_.default_sleep_duration_ms) * factor * 0.5f);
    } else {
        // Very low/no activity - extended sleep
        base_duration = config_.max_sleep_duration_ms * 0.75f;
    }
    
    // Apply aggressiveness multiplier
    float multiplier = BALANCED_MULTIPLIER;
    switch (effective_aggressiveness_) {
        case SleepAggressiveness::CONSERVATIVE:
            multiplier = CONSERVATIVE_MULTIPLIER;
            break;
        case SleepAggressiveness::BALANCED:
            multiplier = BALANCED_MULTIPLIER;
            break;
        case SleepAggressiveness::AGGRESSIVE:
            multiplier = AGGRESSIVE_MULTIPLIER;
            break;
        case SleepAggressiveness::ULTRA_AGGRESSIVE:
            multiplier = ULTRA_AGGRESSIVE_MULTIPLIER;
            break;
    }
    
    return static_cast<uint32_t>(base_duration * multiplier);
}

void AdaptiveSleepScheduler::updateEffectiveAggressiveness() {
    // Start with configured aggressiveness
    effective_aggressiveness_ = config_.aggressiveness;
    
    // Increase aggressiveness if battery is low
    if (battery_percentage_ < config_.battery_critical_threshold) {
        effective_aggressiveness_ = SleepAggressiveness::ULTRA_AGGRESSIVE;
    } else if (battery_percentage_ < config_.battery_low_threshold) {
        // Bump up one level
        int current = static_cast<int>(config_.aggressiveness);
        int max_level = static_cast<int>(SleepAggressiveness::ULTRA_AGGRESSIVE);
        if (current < max_level) {
            effective_aggressiveness_ = static_cast<SleepAggressiveness>(current + 1);
        }
    }
}

void AdaptiveSleepScheduler::startNewDay() {
    Serial.printf("[AdaptiveSleep] New day started, yesterday had %d events\n", 
                  today_motion_count_);
    
    // Move to next day slot
    current_day_index_ = (current_day_index_ + 1) % PATTERN_HISTORY_DAYS;
    
    // Clear the new day's data
    for (int h = 0; h < TIME_SLOTS_PER_DAY; h++) {
        hourly_patterns_[current_day_index_][h] = 0;
    }
    
    today_motion_count_ = 0;
    patterns_dirty_ = true;
}

uint32_t AdaptiveSleepScheduler::applyBatteryMultiplier(uint32_t base_duration) {
    float multiplier = 1.0f;
    
    if (battery_percentage_ < config_.battery_critical_threshold) {
        multiplier = BATTERY_CRITICAL_MULTIPLIER;
    } else if (battery_percentage_ < config_.battery_low_threshold) {
        multiplier = BATTERY_LOW_MULTIPLIER;
    }
    
    return static_cast<uint32_t>(base_duration * multiplier);
}

float AdaptiveSleepScheduler::calculateActivityProbability(uint8_t hour) {
    if (hour >= TIME_SLOTS_PER_DAY) return 0.0f;
    
    // Count days with activity in this hour
    int active_days = 0;
    for (int d = 0; d < PATTERN_HISTORY_DAYS; d++) {
        if (hourly_patterns_[d][hour] > 0) {
            active_days++;
        }
    }
    
    // Base probability from historical frequency
    float base_prob = static_cast<float>(active_days) / PATTERN_HISTORY_DAYS;
    
    // Adjust based on average events (more events = higher confidence)
    float avg = hourly_averages_[hour];
    float event_factor = 1.0f;
    if (avg > 5.0f) {
        event_factor = 1.2f;  // Strong pattern
    } else if (avg > 1.0f) {
        event_factor = 1.1f;  // Moderate pattern
    }
    
    float probability = base_prob * event_factor;
    if (probability > 1.0f) probability = 1.0f;
    
    return probability;
}

void AdaptiveSleepScheduler::initializeDefaultConfig() {
    config_ = getDefaultConfig();
}
