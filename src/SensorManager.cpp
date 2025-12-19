/**
 * @file SensorManager.cpp
 * @brief Additional Sensor Management implementation for WildCAM ESP32
 * 
 * Implements support for BME280, GPS, and light sensors.
 * 
 * @author WildCAM Project
 * @date 2024
 */

#include "SensorManager.h"
#include "config.h"

// BME280 register addresses
#define BME280_REG_CHIP_ID      0xD0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7
#define BME280_REG_CALIB_START  0x88
#define BME280_REG_CALIB_HUM    0xE1

// BME280 chip ID
#define BME280_CHIP_ID          0x60

// BH1750 commands
#define BH1750_POWER_ON         0x01
#define BH1750_CONTINUOUS_HIGH  0x10

// Default configuration
#define DEFAULT_SEA_LEVEL_PRESSURE 1013.25
#define DEFAULT_DAYTIME_THRESHOLD  100.0

// GPS parsing
#ifdef SENSOR_GPS_ENABLED
#include <TinyGPS++.h>
static TinyGPSPlus gps;
static HardwareSerial* gpsSerial = nullptr;
#endif

// BME280 calibration data storage
static struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
    int32_t t_fine;
} bme280_calib;

//==============================================================================
// CONSTRUCTOR / DESTRUCTOR
//==============================================================================

SensorManager::SensorManager()
    : _bme280Initialized(false),
      _gpsInitialized(false),
      _lightSensorInitialized(false),
      _i2cInitialized(false),
      _bme280Address(0x76),
      _seaLevelPressure(DEFAULT_SEA_LEVEL_PRESSURE),
      _gpsRxPin(-1),
      _gpsTxPin(-1),
      _lightSensorPin(-1),
      _lightSensorAddress(0x23),
      _daytimeThreshold(DEFAULT_DAYTIME_THRESHOLD)
{
    // Initialize cached data
    memset(&_lastEnvData, 0, sizeof(_lastEnvData));
    memset(&_lastGPSData, 0, sizeof(_lastGPSData));
    memset(&_lastLightData, 0, sizeof(_lastLightData));
}

SensorManager::~SensorManager() {
#ifdef SENSOR_GPS_ENABLED
    if (gpsSerial != nullptr) {
        gpsSerial->end();
        delete gpsSerial;
        gpsSerial = nullptr;
    }
#endif
}

//==============================================================================
// INITIALIZATION
//==============================================================================

bool SensorManager::init() {
    Serial.println("[Sensors] Initializing Sensor Manager...");
    
    bool anyInitialized = false;
    
    // Initialize I2C with default pins if not already done
    if (!_i2cInitialized) {
#ifdef SENSOR_I2C_SDA_PIN
        initI2C(SENSOR_I2C_SDA_PIN, SENSOR_I2C_SCL_PIN);
#else
        initI2C(21, 22);  // Default ESP32 I2C pins
#endif
    }
    
    // Initialize BME280 if enabled
#ifdef SENSOR_BME280_ENABLED
    if (initBME280(SENSOR_BME280_ADDRESS)) {
        anyInitialized = true;
    }
#endif
    
    // Initialize GPS if enabled
#ifdef SENSOR_GPS_ENABLED
    if (initGPS(SENSOR_GPS_RX_PIN, SENSOR_GPS_TX_PIN, SENSOR_GPS_BAUD)) {
        anyInitialized = true;
    }
#endif
    
    // Initialize light sensor if enabled
#ifdef SENSOR_LIGHT_ENABLED
    #ifdef SENSOR_LIGHT_ANALOG_PIN
        if (initLightSensor(SENSOR_LIGHT_ANALOG_PIN)) {
            anyInitialized = true;
        }
    #else
        if (initLightSensor(-1, SENSOR_BH1750_ADDRESS)) {
            anyInitialized = true;
        }
    #endif
#endif
    
    if (anyInitialized) {
        Serial.println("[Sensors] Sensor Manager initialized");
    } else {
        Serial.println("[Sensors] No sensors initialized (check configuration)");
    }
    
    return anyInitialized;
}

bool SensorManager::initI2C(int sda, int scl) {
    if (_i2cInitialized) {
        return true;
    }
    
    Serial.printf("[Sensors] Initializing I2C (SDA: %d, SCL: %d)...\n", sda, scl);
    Wire.begin(sda, scl);
    Wire.setClock(100000);  // 100kHz I2C clock
    
    _i2cInitialized = true;
    Serial.println("[Sensors] I2C initialized");
    return true;
}

bool SensorManager::initBME280(uint8_t address) {
    if (_bme280Initialized) {
        return true;
    }
    
    Serial.printf("[Sensors] Initializing BME280 at 0x%02X...\n", address);
    _bme280Address = address;
    
    // Check if device is present
    if (!scanI2CDevice(address)) {
        Serial.println("[Sensors] BME280 not found");
        return false;
    }
    
    // Read and verify chip ID
    Wire.beginTransmission(address);
    Wire.write(BME280_REG_CHIP_ID);
    Wire.endTransmission();
    Wire.requestFrom(address, (uint8_t)1);
    
    if (Wire.available()) {
        uint8_t chipId = Wire.read();
        if (chipId != BME280_CHIP_ID && chipId != 0x58) {  // 0x58 is BMP280
            Serial.printf("[Sensors] Invalid chip ID: 0x%02X\n", chipId);
            return false;
        }
    } else {
        Serial.println("[Sensors] Failed to read BME280 chip ID");
        return false;
    }
    
    // Read calibration data
    Wire.beginTransmission(address);
    Wire.write(BME280_REG_CALIB_START);
    Wire.endTransmission();
    Wire.requestFrom(address, (uint8_t)26);
    
    if (Wire.available() >= 26) {
        bme280_calib.dig_T1 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_T2 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_T3 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P1 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P2 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P3 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P4 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P5 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P6 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P7 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P8 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_P9 = Wire.read() | (Wire.read() << 8);
        Wire.read();  // Skip one byte
        bme280_calib.dig_H1 = Wire.read();
    }
    
    // Read humidity calibration data
    Wire.beginTransmission(address);
    Wire.write(BME280_REG_CALIB_HUM);
    Wire.endTransmission();
    Wire.requestFrom(address, (uint8_t)7);
    
    if (Wire.available() >= 7) {
        bme280_calib.dig_H2 = Wire.read() | (Wire.read() << 8);
        bme280_calib.dig_H3 = Wire.read();
        uint8_t e4 = Wire.read();
        uint8_t e5 = Wire.read();
        uint8_t e6 = Wire.read();
        bme280_calib.dig_H4 = (e4 << 4) | (e5 & 0x0F);
        bme280_calib.dig_H5 = (e6 << 4) | (e5 >> 4);
        bme280_calib.dig_H6 = Wire.read();
    }
    
    // Configure BME280: humidity oversampling x1
    Wire.beginTransmission(address);
    Wire.write(BME280_REG_CTRL_HUM);
    Wire.write(0x01);
    Wire.endTransmission();
    
    // Configure BME280: normal mode, temp/pressure oversampling x1
    Wire.beginTransmission(address);
    Wire.write(BME280_REG_CTRL_MEAS);
    Wire.write(0x27);  // 001 001 11 = temp x1, pressure x1, normal mode
    Wire.endTransmission();
    
    // Configure BME280: standby time 1000ms, filter off
    Wire.beginTransmission(address);
    Wire.write(BME280_REG_CONFIG);
    Wire.write(0xA0);  // 101 000 00 = standby 1000ms, filter off
    Wire.endTransmission();
    
    _bme280Initialized = true;
    Serial.println("[Sensors] BME280 initialized successfully");
    return true;
}

bool SensorManager::initGPS(int rxPin, int txPin, int baudRate) {
#ifdef SENSOR_GPS_ENABLED
    if (_gpsInitialized) {
        return true;
    }
    
    Serial.printf("[Sensors] Initializing GPS (RX: %d, TX: %d, Baud: %d)...\n", 
                  rxPin, txPin, baudRate);
    
    _gpsRxPin = rxPin;
    _gpsTxPin = txPin;
    
    // Create hardware serial for GPS (UART2)
    gpsSerial = new HardwareSerial(2);
    gpsSerial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    
    delay(100);
    
    _gpsInitialized = true;
    Serial.println("[Sensors] GPS initialized");
    return true;
#else
    Serial.println("[Sensors] GPS support not enabled in config");
    return false;
#endif
}

bool SensorManager::initLightSensor(int pin, uint8_t address) {
    if (_lightSensorInitialized) {
        return true;
    }
    
    _lightSensorPin = pin;
    _lightSensorAddress = address;
    
    if (pin >= 0) {
        // Analog light sensor (LDR or similar)
        Serial.printf("[Sensors] Initializing analog light sensor on pin %d...\n", pin);
        pinMode(pin, INPUT);
        _lightSensorInitialized = true;
        Serial.println("[Sensors] Analog light sensor initialized");
        return true;
    } else {
        // I2C BH1750 sensor
        Serial.printf("[Sensors] Initializing BH1750 at 0x%02X...\n", address);
        
        if (!scanI2CDevice(address)) {
            Serial.println("[Sensors] BH1750 not found");
            return false;
        }
        
        // Power on BH1750
        Wire.beginTransmission(address);
        Wire.write(BH1750_POWER_ON);
        Wire.endTransmission();
        
        delay(10);
        
        // Set continuous high resolution mode
        Wire.beginTransmission(address);
        Wire.write(BH1750_CONTINUOUS_HIGH);
        Wire.endTransmission();
        
        _lightSensorInitialized = true;
        Serial.println("[Sensors] BH1750 initialized successfully");
        return true;
    }
}

//==============================================================================
// DATA READING
//==============================================================================

EnvironmentalData SensorManager::readEnvironmental() {
    EnvironmentalData data;
    memset(&data, 0, sizeof(data));
    data.valid = false;
    
    if (!_bme280Initialized) {
        return data;
    }
    
    float temp, hum, pres;
    if (readBME280Raw(&temp, &hum, &pres)) {
        data.temperature = temp;
        data.humidity = hum;
        data.pressure = pres;
        
        // Calculate altitude using barometric formula
        data.altitude = 44330.0 * (1.0 - pow(pres / _seaLevelPressure, 0.1903));
        
        data.valid = true;
        data.timestamp = millis();
        
        _lastEnvData = data;
    }
    
    return data;
}

GPSData SensorManager::readGPS() {
    GPSData data;
    memset(&data, 0, sizeof(data));
    data.hasFix = false;
    
#ifdef SENSOR_GPS_ENABLED
    if (!_gpsInitialized || gpsSerial == nullptr) {
        return data;
    }
    
    // Update GPS data
    updateGPS();
    
    if (gps.location.isValid()) {
        data.latitude = gps.location.lat();
        data.longitude = gps.location.lng();
        data.hasFix = true;
    }
    
    if (gps.altitude.isValid()) {
        data.altitude = gps.altitude.meters();
    }
    
    if (gps.satellites.isValid()) {
        data.satellites = gps.satellites.value();
    }
    
    if (gps.hdop.isValid()) {
        data.hdop = gps.hdop.hdop();
    }
    
    data.timestamp = millis();
    _lastGPSData = data;
#endif
    
    return data;
}

LightData SensorManager::readLight() {
    LightData data;
    memset(&data, 0, sizeof(data));
    data.valid = false;
    
    if (!_lightSensorInitialized) {
        return data;
    }
    
    float lux;
    
    if (_lightSensorPin >= 0) {
        // Read analog sensor
        lux = readLightAnalog();
    } else {
        // Read BH1750
        lux = readLightBH1750();
    }
    
    if (lux >= 0) {
        data.lux = lux;
        data.isDaytime = (lux >= _daytimeThreshold);
        data.valid = true;
        data.timestamp = millis();
        
        _lastLightData = data;
    }
    
    return data;
}

bool SensorManager::updateGPS() {
#ifdef SENSOR_GPS_ENABLED
    if (!_gpsInitialized || gpsSerial == nullptr) {
        return false;
    }
    
    bool newData = false;
    while (gpsSerial->available() > 0) {
        if (gps.encode(gpsSerial->read())) {
            newData = true;
        }
    }
    
    return newData;
#else
    return false;
#endif
}

//==============================================================================
// UTILITY FUNCTIONS
//==============================================================================

LightCondition SensorManager::getLightCondition() {
    LightData light = readLight();
    
    if (!light.valid) {
        return LightCondition::DAYLIGHT;  // Default assumption
    }
    
    if (light.lux < 10) {
        return LightCondition::NIGHT;
    } else if (light.lux < 100) {
        return LightCondition::TWILIGHT;
    } else if (light.lux < 1000) {
        return LightCondition::CLOUDY;
    } else if (light.lux < 10000) {
        return LightCondition::DAYLIGHT;
    } else {
        return LightCondition::BRIGHT_SUN;
    }
}

String SensorManager::getLocationString() {
#ifdef SENSOR_GPS_ENABLED
    GPSData data = readGPS();
    
    if (data.hasFix) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%.6f,%.6f,%.1f",
                 data.latitude, data.longitude, data.altitude);
        return String(buffer);
    }
#endif
    return "No GPS";
}

void SensorManager::printStatus() {
    Serial.println("\n========== Sensor Status ==========");
    
    // BME280 Status
    Serial.printf("BME280: %s\n", _bme280Initialized ? "Enabled" : "Disabled");
    if (_bme280Initialized) {
        EnvironmentalData env = readEnvironmental();
        if (env.valid) {
            Serial.printf("  Temperature: %.1f°C\n", env.temperature);
            Serial.printf("  Humidity: %.1f%%\n", env.humidity);
            Serial.printf("  Pressure: %.1f hPa\n", env.pressure);
            Serial.printf("  Altitude: %.1f m\n", env.altitude);
        } else {
            Serial.println("  Read error");
        }
    }
    
    // GPS Status
    Serial.printf("GPS: %s\n", _gpsInitialized ? "Enabled" : "Disabled");
#ifdef SENSOR_GPS_ENABLED
    if (_gpsInitialized) {
        GPSData gpsData = readGPS();
        Serial.printf("  Fix: %s\n", gpsData.hasFix ? "Yes" : "No");
        if (gpsData.hasFix) {
            Serial.printf("  Location: %.6f, %.6f\n", 
                         gpsData.latitude, gpsData.longitude);
            Serial.printf("  Altitude: %.1f m\n", gpsData.altitude);
            Serial.printf("  Satellites: %d\n", gpsData.satellites);
        }
    }
#endif
    
    // Light Sensor Status
    Serial.printf("Light Sensor: %s\n", _lightSensorInitialized ? "Enabled" : "Disabled");
    if (_lightSensorInitialized) {
        LightData light = readLight();
        if (light.valid) {
            Serial.printf("  Light Level: %.1f lux\n", light.lux);
            Serial.printf("  Daytime: %s\n", light.isDaytime ? "Yes" : "No");
            
            const char* condition;
            switch (getLightCondition()) {
                case LightCondition::NIGHT:      condition = "Night"; break;
                case LightCondition::TWILIGHT:   condition = "Twilight"; break;
                case LightCondition::CLOUDY:     condition = "Cloudy"; break;
                case LightCondition::DAYLIGHT:   condition = "Daylight"; break;
                case LightCondition::BRIGHT_SUN: condition = "Bright Sun"; break;
                default:                         condition = "Unknown"; break;
            }
            Serial.printf("  Condition: %s\n", condition);
        } else {
            Serial.println("  Read error");
        }
    }
    
    Serial.println("====================================\n");
}

bool SensorManager::isBME280Available() {
    return _bme280Initialized;
}

bool SensorManager::isGPSAvailable() {
    return _gpsInitialized;
}

bool SensorManager::hasGPSFix() {
#ifdef SENSOR_GPS_ENABLED
    if (!_gpsInitialized) return false;
    updateGPS();
    return gps.location.isValid();
#else
    return false;
#endif
}

bool SensorManager::isLightSensorAvailable() {
    return _lightSensorInitialized;
}

void SensorManager::setDaytimeThreshold(float luxThreshold) {
    _daytimeThreshold = luxThreshold;
}

void SensorManager::setSeaLevelPressure(float pressure) {
    _seaLevelPressure = pressure;
}

//==============================================================================
// PRIVATE HELPER METHODS
//==============================================================================

bool SensorManager::readBME280Raw(float* temp, float* hum, float* pres) {
    if (!_bme280Initialized) {
        return false;
    }
    
    // Read raw data (8 bytes: pressure, temperature, humidity)
    Wire.beginTransmission(_bme280Address);
    Wire.write(BME280_REG_PRESS_MSB);
    Wire.endTransmission();
    Wire.requestFrom(_bme280Address, (uint8_t)8);
    
    if (Wire.available() < 8) {
        return false;
    }
    
    uint32_t press_raw = ((uint32_t)Wire.read() << 12) | 
                         ((uint32_t)Wire.read() << 4) | 
                         ((uint32_t)Wire.read() >> 4);
    uint32_t temp_raw = ((uint32_t)Wire.read() << 12) | 
                        ((uint32_t)Wire.read() << 4) | 
                        ((uint32_t)Wire.read() >> 4);
    uint32_t hum_raw = ((uint32_t)Wire.read() << 8) | 
                       (uint32_t)Wire.read();
    
    // Compensate temperature
    int32_t var1 = ((((temp_raw >> 3) - ((int32_t)bme280_calib.dig_T1 << 1))) *
                   ((int32_t)bme280_calib.dig_T2)) >> 11;
    int32_t var2 = (((((temp_raw >> 4) - ((int32_t)bme280_calib.dig_T1)) *
                   ((temp_raw >> 4) - ((int32_t)bme280_calib.dig_T1))) >> 12) *
                   ((int32_t)bme280_calib.dig_T3)) >> 14;
    bme280_calib.t_fine = var1 + var2;
    *temp = (float)((bme280_calib.t_fine * 5 + 128) >> 8) / 100.0;
    
    // Compensate pressure
    int64_t var1_p = (int64_t)bme280_calib.t_fine - 128000;
    int64_t var2_p = var1_p * var1_p * (int64_t)bme280_calib.dig_P6;
    var2_p = var2_p + ((var1_p * (int64_t)bme280_calib.dig_P5) << 17);
    var2_p = var2_p + (((int64_t)bme280_calib.dig_P4) << 35);
    var1_p = ((var1_p * var1_p * (int64_t)bme280_calib.dig_P3) >> 8) +
             ((var1_p * (int64_t)bme280_calib.dig_P2) << 12);
    var1_p = (((((int64_t)1) << 47) + var1_p)) * ((int64_t)bme280_calib.dig_P1) >> 33;
    
    if (var1_p != 0) {
        int64_t p = 1048576 - press_raw;
        p = (((p << 31) - var2_p) * 3125) / var1_p;
        var1_p = (((int64_t)bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
        var2_p = (((int64_t)bme280_calib.dig_P8) * p) >> 19;
        p = ((p + var1_p + var2_p) >> 8) + (((int64_t)bme280_calib.dig_P7) << 4);
        *pres = (float)p / 256.0 / 100.0;  // Convert to hPa
    } else {
        *pres = 0;
    }
    
    // Compensate humidity
    int32_t v_x1_u32r = bme280_calib.t_fine - ((int32_t)76800);
    v_x1_u32r = (((((hum_raw << 14) - (((int32_t)bme280_calib.dig_H4) << 20) -
                 (((int32_t)bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) *
                (((((((v_x1_u32r * ((int32_t)bme280_calib.dig_H6)) >> 10) *
                (((v_x1_u32r * ((int32_t)bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
                ((int32_t)2097152)) * ((int32_t)bme280_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                ((int32_t)bme280_calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    *hum = (float)(v_x1_u32r >> 12) / 1024.0;
    
    return true;
}

float SensorManager::readLightAnalog() {
    if (_lightSensorPin < 0) {
        return -1;
    }
    
    // Read analog value (0-4095 for ESP32)
    int rawValue = analogRead(_lightSensorPin);
    
    // Convert to approximate lux (assuming LDR with voltage divider)
    // This is a rough approximation - calibrate for specific LDR
    float voltage = (rawValue / 4095.0) * 3.3;
    float resistance = (3.3 - voltage) * 10000.0 / voltage;  // Assuming 10k resistor
    float lux = 500000.0 / resistance;  // Rough approximation
    
    return lux;
}

float SensorManager::readLightBH1750() {
    if (_lightSensorPin >= 0) {
        return -1;  // Using analog sensor
    }
    
    Wire.beginTransmission(_lightSensorAddress);
    Wire.write(BH1750_CONTINUOUS_HIGH);
    Wire.endTransmission();
    
    delay(180);  // Max measurement time for high resolution mode
    
    Wire.requestFrom(_lightSensorAddress, (uint8_t)2);
    
    if (Wire.available() >= 2) {
        uint16_t raw = (Wire.read() << 8) | Wire.read();
        // Convert raw value to lux (default mode factor: 1.2)
        return (float)raw / 1.2;
    }
    
    return -1;
}

bool SensorManager::scanI2CDevice(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}
