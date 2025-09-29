#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include "../include/config.h"

/**
 * @brief Time management and scheduling utilities
 * 
 * Provides time synchronization, scheduling, and time-based
 * operations for the wildlife camera system.
 */
class TimeManager {
public:
    /**
     * @brief Time synchronization status
     */
    enum class SyncStatus {
        NOT_SYNCED = 0,
        NTP_SYNCED = 1,
        RTC_SYNCED = 2,
        MANUAL_SET = 3
    };

    /**
     * @brief Schedule entry for automated operations
     */
    struct ScheduleEntry {
        uint8_t hour;           // 0-23
        uint8_t minute;         // 0-59
        uint8_t dayOfWeek;      // 0-6 (0=Sunday), 255=daily
        bool enabled;
        String description;
        void (*callback)();
    };

    /**
     * @brief Initialize time manager
     * @param timezone Timezone offset in hours
     * @return true if initialization successful
     */
    static bool initialize(int8_t timezone = 0);

    /**
     * @brief Set system time manually
     * @param year Year (e.g., 2025)
     * @param month Month (1-12)
     * @param day Day (1-31)
     * @param hour Hour (0-23)
     * @param minute Minute (0-59)
     * @param second Second (0-59)
     * @return true if successful
     */
    static bool setTime(uint16_t year, uint8_t month, uint8_t day, 
                       uint8_t hour, uint8_t minute, uint8_t second);

    /**
     * @brief Get current timestamp
     * @return Unix timestamp
     */
    static time_t getCurrentTimestamp();

    /**
     * @brief Get formatted date/time string
     * @param format Format string (strftime compatible)
     * @return Formatted date/time
     */
    static String getFormattedTime(const String& format = "%Y-%m-%d %H:%M:%S");

    /**
     * @brief Get current date folder name
     * @return Date folder name (YYYY_MM_DD format)
     */
    static String getDateFolderName();

    /**
     * @brief Get filename timestamp
     * @return Timestamp for filename (YYYYMMDD_HHMMSS)
     */
    static String getFilenameTimestamp();

    /**
     * @brief Check if current time is within active hours
     * @return true if within active hours
     */
    static bool isWithinActiveHours();

    /**
     * @brief Check if it's currently night time
     * @return true if night time
     */
    static bool isNightTime();

    /**
     * @brief Add scheduled task
     * @param hour Hour to execute (0-23)
     * @param minute Minute to execute (0-59)
     * @param dayOfWeek Day of week (0-6, 255 for daily)
     * @param callback Function to call
     * @param description Task description
     * @return true if task added successfully
     */
    static bool addScheduledTask(uint8_t hour, uint8_t minute, uint8_t dayOfWeek,
                               void (*callback)(), const String& description);

    /**
     * @brief Process scheduled tasks
     * Call this regularly to check for scheduled tasks
     */
    static void processScheduledTasks();

    /**
     * @brief Get time synchronization status
     * @return Current sync status
     */
    static SyncStatus getSyncStatus() { return syncStatus; }

    /**
     * @brief Calculate time until next active period
     * @return Seconds until next active period
     */
    static uint32_t getTimeUntilNextActive();

    /**
     * @brief Calculate sunrise time (simplified)
     * @return Hour of sunrise (0-23)
     */
    static uint8_t getSunriseHour();

    /**
     * @brief Calculate sunset time (simplified)
     * @return Hour of sunset (0-23)
     */
    static uint8_t getSunsetHour();

    /**
     * @brief Get day of year
     * @return Day of year (1-366)
     */
    static uint16_t getDayOfYear();

    /**
     * @brief Check if time is valid
     * @return true if system time is valid
     */
    static bool isTimeValid();

    /**
     * @brief Get uptime in seconds
     * @return System uptime
     */
    static uint32_t getUptime();

    /**
     * @brief Reset daily counters (call at midnight)
     */
    static void resetDailyCounters();

private:
    static bool initialized;
    static SyncStatus syncStatus;
    static int8_t timezoneOffset;
    static uint32_t bootTime;
    static ScheduleEntry schedules[10]; // Maximum 10 scheduled tasks
    static uint8_t scheduleCount;
    static uint32_t lastScheduleCheck;

    /**
     * @brief Check if a scheduled task should run
     * @param entry Schedule entry to check
     * @return true if task should run now
     */
    static bool shouldRunScheduledTask(const ScheduleEntry& entry);

    /**
     * @brief Get current time structure
     * @param timeinfo Output time structure
     * @return true if successful
     */
    static bool getCurrentTime(struct tm& timeinfo);
};

#endif // TIME_MANAGER_H