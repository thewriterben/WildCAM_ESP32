#include "time_manager.h"
#include "logger.h"

// Static member definitions
bool TimeManager::initialized = false;
TimeManager::SyncStatus TimeManager::syncStatus = SyncStatus::NOT_SYNCED;
int8_t TimeManager::timezoneOffset = 0;
uint32_t TimeManager::bootTime = 0;
TimeManager::ScheduleEntry TimeManager::schedules[10];
uint8_t TimeManager::scheduleCount = 0;
uint32_t TimeManager::lastScheduleCheck = 0;

bool TimeManager::initialize(int8_t timezone) {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing time manager...");

    timezoneOffset = timezone;
    bootTime = millis();
    scheduleCount = 0;
    lastScheduleCheck = 0;

    // Set a default time if no valid time is available
    if (!isTimeValid()) {
        // Set default time: January 1, 2025, 12:00:00
        setTime(2025, 1, 1, 12, 0, 0);
        syncStatus = SyncStatus::MANUAL_SET;
        LOG_WARNING("Time set to default value - please sync with NTP or RTC");
    }

    initialized = true;
    LOG_INFO("Time manager initialized");
    LOG_INFO("Current time: " + getFormattedTime());

    return true;
}

bool TimeManager::setTime(uint16_t year, uint8_t month, uint8_t day, 
                         uint8_t hour, uint8_t minute, uint8_t second) {
    struct tm timeinfo = {};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;

    time_t timestamp = mktime(&timeinfo);
    if (timestamp == -1) {
        LOG_ERROR("Invalid time provided");
        return false;
    }

    struct timeval tv = { timestamp, 0 };
    if (settimeofday(&tv, NULL) != 0) {
        LOG_ERROR("Failed to set system time");
        return false;
    }

    syncStatus = SyncStatus::MANUAL_SET;
    LOG_INFO("Time set manually: " + getFormattedTime());

    return true;
}

time_t TimeManager::getCurrentTimestamp() {
    time_t now;
    time(&now);
    return now;
}

String TimeManager::getFormattedTime(const String& format) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char buffer[64];
    strftime(buffer, sizeof(buffer), format.c_str(), &timeinfo);
    
    return String(buffer);
}

String TimeManager::getDateFolderName() {
    return getFormattedTime("%Y_%m_%d");
}

String TimeManager::getFilenameTimestamp() {
    return getFormattedTime("%Y%m%d_%H%M%S");
}

bool TimeManager::isWithinActiveHours() {
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
        return true; // Default to active if time is invalid
    }

    int currentHour = timeinfo.tm_hour;

    if (ACTIVE_START_HOUR <= ACTIVE_END_HOUR) {
        return (currentHour >= ACTIVE_START_HOUR && currentHour < ACTIVE_END_HOUR);
    } else {
        // Handles case where active period crosses midnight
        return (currentHour >= ACTIVE_START_HOUR || currentHour < ACTIVE_END_HOUR);
    }
}

bool TimeManager::isNightTime() {
    uint8_t sunriseHour = getSunriseHour();
    uint8_t sunsetHour = getSunsetHour();
    
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
        return false;
    }

    int currentHour = timeinfo.tm_hour;
    return (currentHour < sunriseHour || currentHour >= sunsetHour);
}

bool TimeManager::addScheduledTask(uint8_t hour, uint8_t minute, uint8_t dayOfWeek,
                                  void (*callback)(), const String& description) {
    if (scheduleCount >= 10 || callback == nullptr) {
        LOG_ERROR("Cannot add scheduled task: maximum reached or invalid callback");
        return false;
    }

    ScheduleEntry& entry = schedules[scheduleCount];
    entry.hour = hour;
    entry.minute = minute;
    entry.dayOfWeek = dayOfWeek;
    entry.enabled = true;
    entry.description = description;
    entry.callback = callback;

    scheduleCount++;

    LOG_INFO("Scheduled task added: " + description + 
             " at " + String(hour) + ":" + String(minute));

    return true;
}

void TimeManager::processScheduledTasks() {
    if (!initialized || scheduleCount == 0) {
        return;
    }

    uint32_t now = millis();
    
    // Check scheduled tasks every minute
    if (now - lastScheduleCheck < 60000) {
        return;
    }

    lastScheduleCheck = now;

    for (uint8_t i = 0; i < scheduleCount; i++) {
        if (schedules[i].enabled && shouldRunScheduledTask(schedules[i])) {
            LOG_INFO("Executing scheduled task: " + schedules[i].description);
            schedules[i].callback();
        }
    }
}

uint32_t TimeManager::getTimeUntilNextActive() {
    if (isWithinActiveHours()) {
        return 0; // Already in active period
    }

    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
        return 3600; // Default to 1 hour if time is invalid
    }

    int currentHour = timeinfo.tm_hour;
    int currentMinute = timeinfo.tm_min;
    int currentTotalMinutes = currentHour * 60 + currentMinute;

    int activeStartMinutes = ACTIVE_START_HOUR * 60;
    int activeEndMinutes = ACTIVE_END_HOUR * 60;

    int minutesUntilActive;

    if (ACTIVE_START_HOUR <= ACTIVE_END_HOUR) {
        // Normal case: active period doesn't cross midnight
        if (currentTotalMinutes < activeStartMinutes) {
            minutesUntilActive = activeStartMinutes - currentTotalMinutes;
        } else {
            // After active period, wait until next day
            minutesUntilActive = (24 * 60) - currentTotalMinutes + activeStartMinutes;
        }
    } else {
        // Active period crosses midnight
        if (currentTotalMinutes >= activeEndMinutes && currentTotalMinutes < activeStartMinutes) {
            minutesUntilActive = activeStartMinutes - currentTotalMinutes;
        } else {
            minutesUntilActive = 0; // Should be active
        }
    }

    return minutesUntilActive * 60; // Convert to seconds
}

uint8_t TimeManager::getSunriseHour() {
    // Simplified sunrise calculation
    // In a real implementation, this would consider:
    // - Geographic location (latitude/longitude)
    // - Day of year
    // - Equation of time
    
    uint16_t dayOfYear = getDayOfYear();
    
    // Simple approximation: sunrise varies from 5 AM to 8 AM throughout the year
    float sunriseFloat = 6.5f + 1.5f * sin(2 * PI * (dayOfYear - 81) / 365.0f);
    
    return (uint8_t)constrain(sunriseFloat, 5, 8);
}

uint8_t TimeManager::getSunsetHour() {
    // Simplified sunset calculation
    uint16_t dayOfYear = getDayOfYear();
    
    // Simple approximation: sunset varies from 17 PM to 20 PM throughout the year
    float sunsetFloat = 18.5f + 1.5f * sin(2 * PI * (dayOfYear - 81) / 365.0f);
    
    return (uint8_t)constrain(sunsetFloat, 17, 20);
}

uint16_t TimeManager::getDayOfYear() {
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
        return 1;
    }
    
    return timeinfo.tm_yday + 1; // tm_yday is 0-based
}

bool TimeManager::isTimeValid() {
    time_t now;
    time(&now);
    
    // Check if time is reasonable (after 2020 and before 2030)
    return (now > 1577836800 && now < 1893456000);
}

uint32_t TimeManager::getUptime() {
    return (millis() - bootTime) / 1000;
}

void TimeManager::resetDailyCounters() {
    LOG_INFO("Daily counters reset at midnight");
    // This would be called by the main application
}

bool TimeManager::shouldRunScheduledTask(const ScheduleEntry& entry) {
    struct tm timeinfo;
    if (!getCurrentTime(timeinfo)) {
        return false;
    }

    // Check if time matches
    if (timeinfo.tm_hour != entry.hour || timeinfo.tm_min != entry.minute) {
        return false;
    }

    // Check day of week (255 means daily)
    if (entry.dayOfWeek != 255 && timeinfo.tm_wday != entry.dayOfWeek) {
        return false;
    }

    // Additional check to prevent running the same task multiple times in the same minute
    static uint32_t lastTaskTime = 0;
    static uint8_t lastTaskIndex = 255;
    
    uint32_t currentMinuteStamp = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    uint8_t currentTaskIndex = &entry - schedules; // Calculate index
    
    if (lastTaskTime == currentMinuteStamp && lastTaskIndex == currentTaskIndex) {
        return false; // Already ran this task in this minute
    }
    
    lastTaskTime = currentMinuteStamp;
    lastTaskIndex = currentTaskIndex;
    
    return true;
}

bool TimeManager::getCurrentTime(struct tm& timeinfo) {
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    return isTimeValid();
}