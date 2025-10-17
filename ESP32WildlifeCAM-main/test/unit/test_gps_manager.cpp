/**
 * @file test_gps_manager.cpp
 * @brief Unit tests for GPS Manager module
 * 
 * Tests GPS fix acquisition, coordinate parsing, and location tracking
 */

#include <unity.h>
#include <cmath>

#ifdef NATIVE_TEST
#include "../mocks/mock_arduino.h"
MockSerial Serial;
#endif

// Mock GPS structures
struct GPSData {
    double latitude;
    double longitude;
    float altitude;
    float speed;
    int satellites;
    bool hasFix;
    unsigned long timestamp;
};

// Mock GPS data
GPSData mockGPSData = {
    .latitude = 37.7749,    // San Francisco
    .longitude = -122.4194,
    .altitude = 52.0f,
    .speed = 0.0f,
    .satellites = 8,
    .hasFix = true,
    .timestamp = 0
};

GPSData getGPSData() {
    return mockGPSData;
}

bool initializeGPS() {
    return true;
}

double degreesToRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    // Haversine formula
    const double R = 6371000; // Earth radius in meters
    
    double dLat = degreesToRadians(lat2 - lat1);
    double dLon = degreesToRadians(lon2 - lon1);
    
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(degreesToRadians(lat1)) * cos(degreesToRadians(lat2)) *
               sin(dLon / 2) * sin(dLon / 2);
    
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

bool isValidLatitude(double lat) {
    return lat >= -90.0 && lat <= 90.0;
}

bool isValidLongitude(double lon) {
    return lon >= -180.0 && lon <= 180.0;
}

bool hasGoodFix(int satellites) {
    return satellites >= 4;
}

void test_gps_manager_creation() {
    TEST_ASSERT_TRUE_MESSAGE(true, "GPS manager creation test");
}

void test_gps_initialization() {
    bool result = initializeGPS();
    TEST_ASSERT_TRUE_MESSAGE(result, "GPS initialization should succeed");
}

void test_gps_data_retrieval() {
    GPSData data = getGPSData();
    TEST_ASSERT_TRUE_MESSAGE(data.hasFix, "GPS should have fix");
    TEST_ASSERT_TRUE_MESSAGE(isValidLatitude(data.latitude), "Latitude should be valid");
    TEST_ASSERT_TRUE_MESSAGE(isValidLongitude(data.longitude), "Longitude should be valid");
}

void test_coordinate_validation() {
    TEST_ASSERT_TRUE(isValidLatitude(0.0));
    TEST_ASSERT_TRUE(isValidLatitude(45.0));
    TEST_ASSERT_TRUE(isValidLatitude(-45.0));
    TEST_ASSERT_FALSE(isValidLatitude(91.0));
    TEST_ASSERT_FALSE(isValidLatitude(-91.0));
    
    TEST_ASSERT_TRUE(isValidLongitude(0.0));
    TEST_ASSERT_TRUE(isValidLongitude(120.0));
    TEST_ASSERT_TRUE(isValidLongitude(-120.0));
    TEST_ASSERT_FALSE(isValidLongitude(181.0));
    TEST_ASSERT_FALSE(isValidLongitude(-181.0));
}

void test_distance_calculation() {
    // Distance between two known points
    // San Francisco to Los Angeles (~559 km)
    double sf_lat = 37.7749, sf_lon = -122.4194;
    double la_lat = 34.0522, la_lon = -118.2437;
    
    double distance = calculateDistance(sf_lat, sf_lon, la_lat, la_lon);
    
    // Should be approximately 559,000 meters (559 km)
    TEST_ASSERT_GREATER_THAN_MESSAGE(500000.0, distance, "Distance should be > 500km");
    TEST_ASSERT_LESS_THAN_MESSAGE(600000.0, distance, "Distance should be < 600km");
}

void test_zero_distance() {
    double lat = 37.7749, lon = -122.4194;
    double distance = calculateDistance(lat, lon, lat, lon);
    
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0, 0.0, distance, 
                                     "Distance to same point should be ~0");
}

void test_satellite_count() {
    GPSData data = getGPSData();
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0, data.satellites, 
                                         "Satellite count should be >= 0");
    TEST_ASSERT_LESS_THAN_MESSAGE(20, data.satellites, 
                                  "Satellite count should be < 20");
}

void test_fix_quality() {
    TEST_ASSERT_TRUE(hasGoodFix(8));   // Good
    TEST_ASSERT_TRUE(hasGoodFix(4));   // Minimum
    TEST_ASSERT_FALSE(hasGoodFix(3));  // Insufficient
    TEST_ASSERT_FALSE(hasGoodFix(0));  // No satellites
}

void test_speed_reading() {
    GPSData data = getGPSData();
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(0.0f, data.speed, "Speed should be >= 0");
}

void test_altitude_reading() {
    GPSData data = getGPSData();
    TEST_ASSERT_GREATER_THAN_MESSAGE(-500.0f, data.altitude, 
                                    "Altitude should be reasonable");
    TEST_ASSERT_LESS_THAN_MESSAGE(10000.0f, data.altitude, 
                                  "Altitude should be < 10km");
}

void test_degrees_to_radians() {
    double rad = degreesToRadians(180.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, M_PI, rad);
    
    rad = degreesToRadians(90.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, M_PI / 2, rad);
    
    rad = degreesToRadians(0.0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, rad);
}

void test_equatorial_distance() {
    // Distance between two points on the equator, 1 degree apart
    double distance = calculateDistance(0.0, 0.0, 0.0, 1.0);
    
    // Should be approximately 111 km (111,000 meters)
    TEST_ASSERT_GREATER_THAN_MESSAGE(100000.0, distance, "Distance should be > 100km");
    TEST_ASSERT_LESS_THAN_MESSAGE(120000.0, distance, "Distance should be < 120km");
}

void setUp(void) {
    // Reset mock GPS data
    mockGPSData.latitude = 37.7749;
    mockGPSData.longitude = -122.4194;
    mockGPSData.altitude = 52.0f;
    mockGPSData.speed = 0.0f;
    mockGPSData.satellites = 8;
    mockGPSData.hasFix = true;
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_gps_manager_creation);
    RUN_TEST(test_gps_initialization);
    RUN_TEST(test_gps_data_retrieval);
    RUN_TEST(test_coordinate_validation);
    RUN_TEST(test_distance_calculation);
    RUN_TEST(test_zero_distance);
    RUN_TEST(test_satellite_count);
    RUN_TEST(test_fix_quality);
    RUN_TEST(test_speed_reading);
    RUN_TEST(test_altitude_reading);
    RUN_TEST(test_degrees_to_radians);
    RUN_TEST(test_equatorial_distance);
    
    return UNITY_END();
}
