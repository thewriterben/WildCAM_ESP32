/**
 * @file TimeManager.h
 * @brief Time management with NTP sync and DS3231 RTC support for WildCAM ESP32
 * 
 * This module provides comprehensive time keeping functionality including:
 * - NTP time synchronization over WiFi
 * - External DS3231 RTC module support for persistent timekeeping
 * - Fallback to ESP32 internal RTC when external RTC is not available
 * - Automatic time persistence across power cycles
 * - Configurable timezone support
 * 
 * @author WildCAM Project
 * @date 2024
 */

#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <time.h>

/**
 * @brief Time source enumeration
 * 
 * Indicates the current source of system time.
 */
enum TimeSource {
    TIME_SOURCE_NONE = 0,      ///< Time not set
    TIME_SOURCE_NTP,           ///< Time synchronized via NTP
    TIME_SOURCE_RTC_EXTERNAL,  ///< Time from external DS3231 RTC
    TIME_SOURCE_RTC_INTERNAL,  ///< Time from ESP32 internal RTC
    TIME_SOURCE_MANUAL         ///< Time set manually
};

/**
 * @class TimeManager
 * @brief Manages system time with NTP sync and RTC support
 * 
 * This class provides a unified interface for time management on the ESP32.
 * It handles NTP synchronization when WiFi is available and uses an external
 * DS3231 RTC module for persistent timekeeping across power cycles.
 * 
 * Key Features:
 * - NTP synchronization with configurable servers and timezone
 * - DS3231 external RTC support via I2C
 * - Automatic fallback to ESP32 internal RTC
 * - Time persistence across deep sleep and power cycles
 * - ISO 8601 formatted timestamp generation
 * 
 * Example usage:
 * @code
 * TimeManager timeManager;
 * 
 * // Initialize with default settings
 * timeManager.init();
 * 
 * // Sync time via NTP when WiFi is connected
 * if (WiFi.status() == WL_CONNECTED) {
 *     timeManager.syncNTP();
 * }
 * 
 * // Get current timestamp
 * char timestamp[30];
 * timeManager.getTimestamp(timestamp, sizeof(timestamp));
 * Serial.printf("Current time: %s\n", timestamp);
 * 
 * // Get Unix timestamp
 * time_t now = timeManager.getUnixTime();
 * @endcode
 */
class TimeManager {
public:
    /**
     * @brief Default constructor
     * 
     * Initializes the TimeManager with default settings:
     * - NTP server: pool.ntp.org
     * - Timezone: UTC (GMT offset 0)
     * - External RTC not initialized
     */
    TimeManager();
    
    /**
     * @brief Initialize the time management system
     * 
     * Performs the following initialization steps:
     * 1. Attempts to initialize external DS3231 RTC via I2C
     * 2. If external RTC found and has valid time, loads time from RTC
     * 3. Configures ESP32 internal RTC as fallback
     * 4. Sets up timezone configuration
     * 
     * @param gmtOffset_sec GMT offset in seconds (e.g., -18000 for EST)
     * @param daylightOffset_sec Daylight saving offset in seconds (e.g., 3600)
     * @return bool true if initialization successful, false otherwise
     * 
     * @note Call this function once during system startup
     * @note External RTC failure is non-critical - system falls back to internal RTC
     */
    bool init(long gmtOffset_sec = 0, int daylightOffset_sec = 0);
    
    /**
     * @brief Synchronize time with NTP server
     * 
     * Connects to the configured NTP server to synchronize system time.
     * If successful, also updates the external RTC (if available) for
     * persistent timekeeping.
     * 
     * @param ntpServer NTP server address (default: "pool.ntp.org")
     * @param timeoutMs Maximum time to wait for NTP response (default: 10000ms)
     * @return bool true if NTP sync successful, false otherwise
     * 
     * @note Requires WiFi connection to be established
     * @note Automatically updates external RTC after successful sync
     * @note Blocks until sync completes or timeout
     */
    bool syncNTP(const char* ntpServer = "pool.ntp.org", unsigned long timeoutMs = 10000);
    
    /**
     * @brief Check if system time has been set
     * 
     * @return bool true if time is valid (from NTP or RTC), false if using millis() fallback
     */
    bool isTimeSet() const;
    
    /**
     * @brief Get current Unix timestamp
     * 
     * @return time_t Current Unix timestamp (seconds since Jan 1, 1970)
     * @note Returns 0 if time not set
     */
    time_t getUnixTime() const;
    
    /**
     * @brief Get current time as tm structure
     * 
     * @param timeinfo Pointer to tm structure to fill
     * @return bool true if time retrieved successfully, false otherwise
     */
    bool getTime(struct tm* timeinfo) const;
    
    /**
     * @brief Get formatted timestamp string
     * 
     * Generates a timestamp in ISO 8601 format: "YYYY-MM-DDTHH:MM:SSZ"
     * 
     * @param buffer Buffer to write timestamp to
     * @param bufferSize Size of the buffer (minimum 25 bytes recommended)
     * @return char* Pointer to buffer, or NULL on error
     * 
     * @note If time not set, returns millisecond-based fallback timestamp
     */
    char* getTimestamp(char* buffer, size_t bufferSize) const;
    
    /**
     * @brief Get formatted date string
     * 
     * Generates a date string in YYYYMMDD format for directory naming.
     * 
     * @param buffer Buffer to write date to
     * @param bufferSize Size of the buffer (minimum 9 bytes)
     * @return char* Pointer to buffer, or NULL on error
     */
    char* getDateString(char* buffer, size_t bufferSize) const;
    
    /**
     * @brief Get formatted time string
     * 
     * Generates a time string in HHMMSS format for filename generation.
     * 
     * @param buffer Buffer to write time to
     * @param bufferSize Size of the buffer (minimum 7 bytes)
     * @return char* Pointer to buffer, or NULL on error
     */
    char* getTimeString(char* buffer, size_t bufferSize) const;
    
    /**
     * @brief Get the current time source
     * 
     * @return TimeSource Current source of system time
     */
    TimeSource getTimeSource() const;
    
    /**
     * @brief Get human-readable time source name
     * 
     * @return const char* String describing current time source
     */
    const char* getTimeSourceString() const;
    
    /**
     * @brief Save current time to external RTC
     * 
     * Manually saves the current system time to the external DS3231 RTC.
     * Useful after NTP sync or manual time setting.
     * 
     * @return bool true if save successful, false if RTC not available
     */
    bool saveToRTC();
    
    /**
     * @brief Load time from external RTC
     * 
     * Loads time from the external DS3231 RTC into the system.
     * 
     * @return bool true if load successful and time valid, false otherwise
     */
    bool loadFromRTC();
    
    /**
     * @brief Set time manually
     * 
     * @param year Year (e.g., 2024)
     * @param month Month (1-12)
     * @param day Day (1-31)
     * @param hour Hour (0-23)
     * @param minute Minute (0-59)
     * @param second Second (0-59)
     * @return bool true if time set successfully
     */
    bool setTime(int year, int month, int day, int hour, int minute, int second);
    
    /**
     * @brief Set time from Unix timestamp
     * 
     * @param unixTime Unix timestamp (seconds since Jan 1, 1970)
     * @return bool true if time set successfully
     */
    bool setTime(time_t unixTime);
    
    /**
     * @brief Check if external RTC is available
     * 
     * @return bool true if DS3231 RTC is detected and initialized
     */
    bool hasExternalRTC() const;
    
    /**
     * @brief Get temperature from DS3231 RTC
     * 
     * The DS3231 includes a temperature sensor used for its internal
     * temperature compensation. This method returns that temperature.
     * 
     * @return float Temperature in Celsius, or NAN if RTC not available
     */
    float getRTCTemperature() const;
    
    /**
     * @brief Print time status to Serial
     * 
     * Displays current time, time source, and RTC status.
     */
    void printStatus() const;
    
private:
    bool _initialized;           ///< Flag indicating initialization status
    bool _hasExternalRTC;        ///< Flag indicating if DS3231 is available
    TimeSource _timeSource;      ///< Current source of system time
    long _gmtOffset;             ///< GMT offset in seconds
    int _daylightOffset;         ///< Daylight saving offset in seconds
    
    /**
     * @brief Initialize DS3231 RTC module
     * 
     * @return bool true if RTC detected and initialized
     */
    bool initDS3231();
    
    /**
     * @brief Check if DS3231 has lost power (time invalid)
     * 
     * @return bool true if RTC lost power and time needs to be set
     */
    bool isRTCLostPower() const;
    
    /**
     * @brief Write time to DS3231 registers
     * 
     * @param timeinfo Time structure to write
     * @return bool true if write successful
     */
    bool writeDS3231(const struct tm* timeinfo);
    
    /**
     * @brief Read time from DS3231 registers
     * 
     * @param timeinfo Time structure to fill
     * @return bool true if read successful
     */
    bool readDS3231(struct tm* timeinfo) const;
    
    /**
     * @brief Convert BCD to decimal
     * 
     * @param bcd BCD value
     * @return uint8_t Decimal value
     */
    static uint8_t bcdToDec(uint8_t bcd);
    
    /**
     * @brief Convert decimal to BCD
     * 
     * @param dec Decimal value
     * @return uint8_t BCD value
     */
    static uint8_t decToBcd(uint8_t dec);
};

#endif // TIME_MANAGER_H
