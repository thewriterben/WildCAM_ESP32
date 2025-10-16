/**
 * @file test_wifi_credentials.cpp
 * @brief Unit tests for WiFi credential storage functionality
 * 
 * Tests WiFi credential save, load, and clear operations using ESP32 Preferences (NVS).
 * Validates password encryption/decryption and error handling.
 */

#include <unity.h>
#include "../firmware/src/wifi_manager.h"

// Test WiFiManager instance
WiFiManager* testWifiManager = nullptr;

// Test setup
void setUp(void) {
    // Create new WiFiManager for each test
    testWifiManager = new WiFiManager();
}

// Test teardown
void tearDown(void) {
    // Clean up WiFiManager
    if (testWifiManager) {
        testWifiManager->clearWiFiCredentials();
        delete testWifiManager;
        testWifiManager = nullptr;
    }
}

/**
 * Test saving WiFi credentials to NVS
 */
void test_save_wifi_credentials(void) {
    const char* testSSID = "TestNetwork";
    const char* testPassword = "TestPassword123";
    
    bool result = testWifiManager->saveWiFiCredentials(testSSID, testPassword);
    
    TEST_ASSERT_TRUE(result);
}

/**
 * Test saving credentials with empty SSID (should fail)
 */
void test_save_empty_ssid(void) {
    const char* emptySSID = "";
    const char* testPassword = "TestPassword123";
    
    bool result = testWifiManager->saveWiFiCredentials(emptySSID, testPassword);
    
    TEST_ASSERT_FALSE(result);
}

/**
 * Test saving credentials with empty password (should succeed)
 */
void test_save_empty_password(void) {
    const char* testSSID = "OpenNetwork";
    const char* emptyPassword = "";
    
    bool result = testWifiManager->saveWiFiCredentials(testSSID, emptyPassword);
    
    TEST_ASSERT_TRUE(result);
}

/**
 * Test loading WiFi credentials from NVS
 */
void test_load_wifi_credentials(void) {
    // First save some credentials
    const char* testSSID = "TestNetwork";
    const char* testPassword = "TestPassword123";
    
    testWifiManager->saveWiFiCredentials(testSSID, testPassword);
    
    // Now load them back
    String loadedSSID, loadedPassword;
    bool result = testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_STRING(testSSID, loadedSSID.c_str());
    TEST_ASSERT_EQUAL_STRING(testPassword, loadedPassword.c_str());
}

/**
 * Test loading credentials when none are stored
 */
void test_load_no_credentials(void) {
    // Clear any existing credentials
    testWifiManager->clearWiFiCredentials();
    
    String loadedSSID, loadedPassword;
    bool result = testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(loadedSSID.isEmpty());
    TEST_ASSERT_TRUE(loadedPassword.isEmpty());
}

/**
 * Test clearing WiFi credentials
 */
void test_clear_wifi_credentials(void) {
    // First save some credentials
    const char* testSSID = "TestNetwork";
    const char* testPassword = "TestPassword123";
    
    testWifiManager->saveWiFiCredentials(testSSID, testPassword);
    
    // Clear them
    testWifiManager->clearWiFiCredentials();
    
    // Try to load - should fail
    String loadedSSID, loadedPassword;
    bool result = testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_TRUE(loadedSSID.isEmpty());
    TEST_ASSERT_TRUE(loadedPassword.isEmpty());
}

/**
 * Test saving and loading special characters in password
 */
void test_special_characters_in_password(void) {
    const char* testSSID = "TestNetwork";
    const char* testPassword = "P@$$w0rd!#%&*()_+-=[]{}|;:,.<>?";
    
    testWifiManager->saveWiFiCredentials(testSSID, testPassword);
    
    String loadedSSID, loadedPassword;
    testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    TEST_ASSERT_EQUAL_STRING(testSSID, loadedSSID.c_str());
    TEST_ASSERT_EQUAL_STRING(testPassword, loadedPassword.c_str());
}

/**
 * Test saving and loading long credentials
 */
void test_long_credentials(void) {
    // Maximum SSID length is typically 32 characters
    const char* testSSID = "VeryLongNetworkNameWith32Chars!";
    // Password can be up to 63 characters
    const char* testPassword = "VeryLongPasswordWith63CharactersIncludingSpecialChars!@#$%^";
    
    testWifiManager->saveWiFiCredentials(testSSID, testPassword);
    
    String loadedSSID, loadedPassword;
    testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    TEST_ASSERT_EQUAL_STRING(testSSID, loadedSSID.c_str());
    TEST_ASSERT_EQUAL_STRING(testPassword, loadedPassword.c_str());
}

/**
 * Test overwriting existing credentials
 */
void test_overwrite_credentials(void) {
    // Save initial credentials
    testWifiManager->saveWiFiCredentials("Network1", "Password1");
    
    // Overwrite with new credentials
    const char* newSSID = "Network2";
    const char* newPassword = "Password2";
    testWifiManager->saveWiFiCredentials(newSSID, newPassword);
    
    // Load and verify new credentials
    String loadedSSID, loadedPassword;
    testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    TEST_ASSERT_EQUAL_STRING(newSSID, loadedSSID.c_str());
    TEST_ASSERT_EQUAL_STRING(newPassword, loadedPassword.c_str());
}

/**
 * Test password encryption/decryption
 */
void test_password_encryption(void) {
    const char* testSSID = "TestNetwork";
    const char* testPassword = "SecretPassword";
    
    // Save credentials (password will be encrypted)
    testWifiManager->saveWiFiCredentials(testSSID, testPassword);
    
    // Load credentials (password will be decrypted)
    String loadedSSID, loadedPassword;
    testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
    
    // Verify decrypted password matches original
    TEST_ASSERT_EQUAL_STRING(testPassword, loadedPassword.c_str());
}

/**
 * Test multiple save/load cycles
 */
void test_multiple_save_load_cycles(void) {
    const int cycles = 5;
    
    for (int i = 0; i < cycles; i++) {
        String testSSID = "Network" + String(i);
        String testPassword = "Password" + String(i);
        
        // Save
        bool saveResult = testWifiManager->saveWiFiCredentials(
            testSSID.c_str(), 
            testPassword.c_str()
        );
        TEST_ASSERT_TRUE(saveResult);
        
        // Load
        String loadedSSID, loadedPassword;
        bool loadResult = testWifiManager->loadWiFiCredentials(loadedSSID, loadedPassword);
        TEST_ASSERT_TRUE(loadResult);
        
        // Verify
        TEST_ASSERT_EQUAL_STRING(testSSID.c_str(), loadedSSID.c_str());
        TEST_ASSERT_EQUAL_STRING(testPassword.c_str(), loadedPassword.c_str());
    }
}

// Main test runner
void runTests() {
    UNITY_BEGIN();
    
    RUN_TEST(test_save_wifi_credentials);
    RUN_TEST(test_save_empty_ssid);
    RUN_TEST(test_save_empty_password);
    RUN_TEST(test_load_wifi_credentials);
    RUN_TEST(test_load_no_credentials);
    RUN_TEST(test_clear_wifi_credentials);
    RUN_TEST(test_special_characters_in_password);
    RUN_TEST(test_long_credentials);
    RUN_TEST(test_overwrite_credentials);
    RUN_TEST(test_password_encryption);
    RUN_TEST(test_multiple_save_load_cycles);
    
    UNITY_END();
}

#ifdef ARDUINO
void setup() {
    delay(2000); // Wait for serial monitor
    runTests();
}

void loop() {
    // Nothing to do here
}
#else
int main(int argc, char **argv) {
    runTests();
    return 0;
}
#endif
