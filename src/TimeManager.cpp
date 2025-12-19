/**
 * @file TimeManager.cpp
 * @brief Implementation of time management with NTP sync and DS3231 RTC support
 * 
 * This file implements the TimeManager class providing comprehensive time
 * keeping functionality for the WildCAM ESP32 wildlife camera system.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#include "TimeManager.h"
#include "config.h"
#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <sys/time.h>

// DS3231 I2C address
#define DS3231_ADDRESS 0x68

// DS3231 register addresses
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DAY        0x03
#define DS3231_REG_DATE       0x04
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06
#define DS3231_REG_CONTROL    0x0E
#define DS3231_REG_STATUS     0x0F
#define DS3231_REG_TEMP_MSB   0x11
#define DS3231_REG_TEMP_LSB   0x12

// Status register bits
#define DS3231_STATUS_OSF     0x80  // Oscillator Stop Flag

// PROGMEM strings for Serial output
static const char TAG_INIT[] PROGMEM = "Initializing Time Manager...";
static const char TAG_NTP_SYNC[] PROGMEM = "Synchronizing time with NTP server: %s\n";
static const char TAG_NTP_SUCCESS[] PROGMEM = "NTP sync successful";
static const char TAG_NTP_FAILED[] PROGMEM = "NTP sync failed after %lu ms timeout\n";
static const char TAG_RTC_FOUND[] PROGMEM = "External RTC (DS3231) detected";
static const char TAG_RTC_NOT_FOUND[] PROGMEM = "External RTC not found - using internal RTC";
static const char TAG_RTC_LOST_POWER[] PROGMEM = "RTC lost power - time needs to be set";
static const char TAG_RTC_LOADED[] PROGMEM = "Time loaded from external RTC";
static const char TAG_RTC_SAVED[] PROGMEM = "Time saved to external RTC";
static const char TAG_TIME_SET[] PROGMEM = "System time set: %04d-%02d-%02d %02d:%02d:%02d\n";
static const char TAG_TIME_STATUS[] PROGMEM = "=== Time Status ===";
static const char TAG_CURRENT_TIME[] PROGMEM = "Current time: %s\n";
static const char TAG_TIME_SOURCE[] PROGMEM = "Time source: %s\n";
static const char TAG_RTC_STATUS[] PROGMEM = "External RTC: %s\n";
static const char TAG_RTC_TEMP[] PROGMEM = "RTC Temperature: %.2f C\n";
static const char TAG_SEPARATOR[] PROGMEM = "===================";

TimeManager::TimeManager() 
    : _initialized(false),
      _hasExternalRTC(false),
      _timeSource(TIME_SOURCE_NONE),
      _gmtOffset(0),
      _daylightOffset(0) {
}

bool TimeManager::init(long gmtOffset_sec, int daylightOffset_sec) {
    Serial.println(FPSTR(TAG_INIT));
    
    _gmtOffset = gmtOffset_sec;
    _daylightOffset = daylightOffset_sec;
    
    // Initialize I2C for RTC communication
    // Use default I2C pins (SDA=21, SCL=22 on most ESP32 boards)
    // For AI-Thinker ESP32-CAM, may need to use GPIO 14/15 if default pins are used by camera
    Wire.begin();
    
    // Try to initialize external DS3231 RTC
    _hasExternalRTC = initDS3231();
    
    if (_hasExternalRTC) {
        Serial.println(FPSTR(TAG_RTC_FOUND));
        
        // Check if RTC lost power (oscillator stopped)
        if (isRTCLostPower()) {
            Serial.println(FPSTR(TAG_RTC_LOST_POWER));
            _timeSource = TIME_SOURCE_NONE;
        } else {
            // Load time from external RTC
            if (loadFromRTC()) {
                Serial.println(FPSTR(TAG_RTC_LOADED));
                _timeSource = TIME_SOURCE_RTC_EXTERNAL;
            }
        }
    } else {
        Serial.println(FPSTR(TAG_RTC_NOT_FOUND));
        // Will use ESP32 internal RTC (loses time on power loss)
        _timeSource = TIME_SOURCE_RTC_INTERNAL;
    }
    
    // Configure timezone
    configTime(_gmtOffset, _daylightOffset, "");  // Empty NTP server - will sync later
    
    _initialized = true;
    return true;
}

bool TimeManager::syncNTP(const char* ntpServer, unsigned long timeoutMs) {
    if (!_initialized) {
        return false;
    }
    
    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    Serial.printf_P(TAG_NTP_SYNC, ntpServer);
    
    // Configure time with NTP server
    configTime(_gmtOffset, _daylightOffset, ntpServer);
    
    // Wait for time to be set
    struct tm timeinfo;
    unsigned long startTime = millis();
    
    while (!getLocalTime(&timeinfo, 1000)) {
        if (millis() - startTime > timeoutMs) {
            Serial.printf_P(TAG_NTP_FAILED, timeoutMs);
            return false;
        }
        delay(100);
    }
    
    Serial.println(FPSTR(TAG_NTP_SUCCESS));
    _timeSource = TIME_SOURCE_NTP;
    
    // Save to external RTC if available
    if (_hasExternalRTC) {
        saveToRTC();
    }
    
    // Print the synced time
    char timestamp[30];
    getTimestamp(timestamp, sizeof(timestamp));
    Serial.printf_P(TAG_CURRENT_TIME, timestamp);
    
    return true;
}

bool TimeManager::isTimeSet() const {
    return _timeSource != TIME_SOURCE_NONE;
}

time_t TimeManager::getUnixTime() const {
    time_t now;
    time(&now);
    return now;
}

bool TimeManager::getTime(struct tm* timeinfo) const {
    if (!timeinfo) {
        return false;
    }
    return getLocalTime(timeinfo);
}

char* TimeManager::getTimestamp(char* buffer, size_t bufferSize) const {
    if (!buffer || bufferSize < 25) {
        return nullptr;
    }
    
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        // ISO 8601 format: YYYY-MM-DDTHH:MM:SSZ
        snprintf(buffer, bufferSize, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);
    } else {
        // Fallback to millis-based timestamp
        unsigned long ms = millis();
        unsigned long seconds = ms / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        unsigned long days = hours / 24;
        
        hours = hours % 24;
        minutes = minutes % 60;
        seconds = seconds % 60;
        
        snprintf(buffer, bufferSize, "%lud_%02lu:%02lu:%02lu", 
                 days, hours, minutes, seconds);
    }
    
    return buffer;
}

char* TimeManager::getDateString(char* buffer, size_t bufferSize) const {
    if (!buffer || bufferSize < 9) {
        return nullptr;
    }
    
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        snprintf(buffer, bufferSize, "%04d%02d%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday);
    } else {
        // Fallback to day counter
        unsigned long days = millis() / (1000UL * 60 * 60 * 24);
        snprintf(buffer, bufferSize, "day_%05lu", days);
    }
    
    return buffer;
}

char* TimeManager::getTimeString(char* buffer, size_t bufferSize) const {
    if (!buffer || bufferSize < 7) {
        return nullptr;
    }
    
    struct tm timeinfo;
    
    if (getLocalTime(&timeinfo)) {
        snprintf(buffer, bufferSize, "%02d%02d%02d",
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);
    } else {
        // Fallback to milliseconds
        unsigned long ms = millis();
        snprintf(buffer, bufferSize, "%06lu", ms % 1000000UL);
    }
    
    return buffer;
}

TimeSource TimeManager::getTimeSource() const {
    return _timeSource;
}

const char* TimeManager::getTimeSourceString() const {
    switch (_timeSource) {
        case TIME_SOURCE_NTP:          return "NTP";
        case TIME_SOURCE_RTC_EXTERNAL: return "External RTC (DS3231)";
        case TIME_SOURCE_RTC_INTERNAL: return "Internal RTC";
        case TIME_SOURCE_MANUAL:       return "Manual";
        default:                       return "Not Set";
    }
}

bool TimeManager::saveToRTC() {
    if (!_hasExternalRTC) {
        return false;
    }
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return false;
    }
    
    if (writeDS3231(&timeinfo)) {
        Serial.println(FPSTR(TAG_RTC_SAVED));
        return true;
    }
    
    return false;
}

bool TimeManager::loadFromRTC() {
    if (!_hasExternalRTC) {
        return false;
    }
    
    struct tm timeinfo;
    if (!readDS3231(&timeinfo)) {
        return false;
    }
    
    // Convert to time_t and set system time
    time_t rtcTime = mktime(&timeinfo);
    if (rtcTime == -1) {
        return false;
    }
    
    // Set system time
    struct timeval tv;
    tv.tv_sec = rtcTime;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
    
    return true;
}

bool TimeManager::setTime(int year, int month, int day, int hour, int minute, int second) {
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(timeinfo));
    
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    
    time_t t = mktime(&timeinfo);
    if (t == -1) {
        return false;
    }
    
    return setTime(t);
}

bool TimeManager::setTime(time_t unixTime) {
    struct timeval tv;
    tv.tv_sec = unixTime;
    tv.tv_usec = 0;
    
    if (settimeofday(&tv, nullptr) != 0) {
        return false;
    }
    
    _timeSource = TIME_SOURCE_MANUAL;
    
    // Print confirmation
    struct tm timeinfo;
    localtime_r(&unixTime, &timeinfo);
    Serial.printf_P(TAG_TIME_SET,
                   timeinfo.tm_year + 1900,
                   timeinfo.tm_mon + 1,
                   timeinfo.tm_mday,
                   timeinfo.tm_hour,
                   timeinfo.tm_min,
                   timeinfo.tm_sec);
    
    // Save to external RTC if available
    if (_hasExternalRTC) {
        saveToRTC();
    }
    
    return true;
}

bool TimeManager::hasExternalRTC() const {
    return _hasExternalRTC;
}

float TimeManager::getRTCTemperature() const {
    if (!_hasExternalRTC) {
        return NAN;
    }
    
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_TEMP_MSB);
    if (Wire.endTransmission() != 0) {
        return NAN;
    }
    
    Wire.requestFrom(DS3231_ADDRESS, 2);
    if (Wire.available() < 2) {
        return NAN;
    }
    
    int8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    
    // Temperature is in 0.25°C increments
    // MSB is signed integer part, upper 2 bits of LSB are fractional
    float temp = (float)msb + ((lsb >> 6) * 0.25f);
    
    return temp;
}

void TimeManager::printStatus() const {
    Serial.println(FPSTR(TAG_TIME_STATUS));
    
    // Current time
    char timestamp[30];
    if (getTimestamp(timestamp, sizeof(timestamp))) {
        Serial.printf_P(TAG_CURRENT_TIME, timestamp);
    }
    
    // Time source
    Serial.printf_P(TAG_TIME_SOURCE, getTimeSourceString());
    
    // External RTC status
    Serial.printf_P(TAG_RTC_STATUS, _hasExternalRTC ? "Available" : "Not found");
    
    // RTC temperature if available
    if (_hasExternalRTC) {
        float temp = getRTCTemperature();
        if (!isnan(temp)) {
            Serial.printf_P(TAG_RTC_TEMP, temp);
        }
    }
    
    Serial.println(FPSTR(TAG_SEPARATOR));
}

// Private methods

bool TimeManager::initDS3231() {
    // Try to communicate with DS3231 at I2C address 0x68
    Wire.beginTransmission(DS3231_ADDRESS);
    uint8_t error = Wire.endTransmission();
    
    if (error != 0) {
        return false;  // Device not found
    }
    
    // Read the status register to verify communication
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_STATUS);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(DS3231_ADDRESS, 1);
    if (Wire.available() < 1) {
        return false;
    }
    
    // Successfully communicated with DS3231
    Wire.read();  // Discard the status byte for now
    
    return true;
}

bool TimeManager::isRTCLostPower() const {
    if (!_hasExternalRTC) {
        return true;
    }
    
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_STATUS);
    if (Wire.endTransmission() != 0) {
        return true;
    }
    
    Wire.requestFrom(DS3231_ADDRESS, 1);
    if (Wire.available() < 1) {
        return true;
    }
    
    uint8_t status = Wire.read();
    
    // Check Oscillator Stop Flag (OSF) bit
    // If set, oscillator stopped and time is invalid
    return (status & DS3231_STATUS_OSF) != 0;
}

bool TimeManager::writeDS3231(const struct tm* timeinfo) {
    if (!_hasExternalRTC || !timeinfo) {
        return false;
    }
    
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_SECONDS);
    Wire.write(decToBcd(timeinfo->tm_sec));
    Wire.write(decToBcd(timeinfo->tm_min));
    Wire.write(decToBcd(timeinfo->tm_hour));  // 24-hour format
    Wire.write(decToBcd(timeinfo->tm_wday + 1));  // Day of week (1-7)
    Wire.write(decToBcd(timeinfo->tm_mday));
    Wire.write(decToBcd(timeinfo->tm_mon + 1));  // Month (1-12)
    Wire.write(decToBcd((timeinfo->tm_year + 1900) % 100));  // Year (00-99)
    
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    // Clear the Oscillator Stop Flag
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_STATUS);
    Wire.write(0x00);  // Clear all status flags
    Wire.endTransmission();
    
    return true;
}

bool TimeManager::readDS3231(struct tm* timeinfo) const {
    if (!_hasExternalRTC || !timeinfo) {
        return false;
    }
    
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(DS3231_REG_SECONDS);
    if (Wire.endTransmission() != 0) {
        return false;
    }
    
    Wire.requestFrom(DS3231_ADDRESS, 7);
    if (Wire.available() < 7) {
        return false;
    }
    
    timeinfo->tm_sec = bcdToDec(Wire.read() & 0x7F);
    timeinfo->tm_min = bcdToDec(Wire.read());
    timeinfo->tm_hour = bcdToDec(Wire.read() & 0x3F);  // 24-hour format
    timeinfo->tm_wday = bcdToDec(Wire.read()) - 1;  // Day of week (0-6)
    timeinfo->tm_mday = bcdToDec(Wire.read());
    uint8_t monthRaw = Wire.read();
    timeinfo->tm_mon = bcdToDec(monthRaw & 0x1F) - 1;  // Month (0-11)
    uint8_t yearRaw = Wire.read();
    
    // Handle century bit if present in month register
    int century = (monthRaw & 0x80) ? 100 : 0;
    timeinfo->tm_year = bcdToDec(yearRaw) + century;  // Years since 1900
    
    // If year seems too low, assume 2000s
    if (timeinfo->tm_year < 100) {
        timeinfo->tm_year += 100;  // Assume 2000-2099
    }
    
    timeinfo->tm_isdst = -1;  // Unknown daylight saving
    
    return true;
}

uint8_t TimeManager::bcdToDec(uint8_t bcd) {
    return ((bcd / 16) * 10) + (bcd % 16);
}

uint8_t TimeManager::decToBcd(uint8_t dec) {
    return ((dec / 10) * 16) + (dec % 10);
}
