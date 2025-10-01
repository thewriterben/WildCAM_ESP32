/**
 * Security Features Validation Test
 * 
 * Basic compilation and API validation test for enterprise security features.
 * This test verifies that all security components can be instantiated and
 * their APIs are correctly defined.
 */

#include <unity.h>
#include "../firmware/security/security_manager.h"
#include "../ESP32WildlifeCAM-main/firmware/src/production/security/data_protector.h"

// Test fixtures
SecurityManager* testSecMgr = nullptr;
DataProtector* testDataProtector = nullptr;

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
    if (testSecMgr) {
        delete testSecMgr;
        testSecMgr = nullptr;
    }
    if (testDataProtector) {
        delete testDataProtector;
        testDataProtector = nullptr;
    }
}

// Test: Security Manager Instantiation
void test_security_manager_creation(void) {
    testSecMgr = new SecurityManager(SecurityLevel::BASIC, false);
    TEST_ASSERT_NOT_NULL(testSecMgr);
}

// Test: Security Manager Initialization
void test_security_manager_init(void) {
    testSecMgr = new SecurityManager(SecurityLevel::BASIC, false);
    TEST_ASSERT_NOT_NULL(testSecMgr);
    
    bool result = testSecMgr->begin();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testSecMgr->isSecurityOperational());
}

// Test: Data Encryption/Decryption
void test_encryption_decryption(void) {
    testSecMgr = new SecurityManager(SecurityLevel::BASIC, false);
    TEST_ASSERT_TRUE(testSecMgr->begin());
    
    // Test data
    const char* plaintext = "Test Wildlife Data";
    size_t plaintext_len = strlen(plaintext);
    
    // Encrypt
    uint8_t ciphertext[256];
    size_t ciphertext_len = 0;
    
    bool encrypted = testSecMgr->encryptData(
        (const uint8_t*)plaintext, 
        plaintext_len,
        ciphertext, 
        &ciphertext_len
    );
    
    TEST_ASSERT_TRUE(encrypted);
    TEST_ASSERT_GREATER_THAN(0, ciphertext_len);
    
    // Decrypt
    uint8_t decrypted[256];
    size_t decrypted_len = 0;
    
    bool decryptedSuccess = testSecMgr->decryptData(
        ciphertext,
        ciphertext_len,
        decrypted,
        &decrypted_len
    );
    
    TEST_ASSERT_TRUE(decryptedSuccess);
    TEST_ASSERT_EQUAL(plaintext_len, decrypted_len);
    TEST_ASSERT_EQUAL_STRING(plaintext, (const char*)decrypted);
}

// Test: Hash Generation
void test_hash_generation(void) {
    testSecMgr = new SecurityManager(SecurityLevel::BASIC, false);
    TEST_ASSERT_TRUE(testSecMgr->begin());
    
    const char* data = "Test data for hashing";
    uint8_t hash1[32];
    uint8_t hash2[32];
    
    // Generate hash twice
    bool result1 = testSecMgr->generateHash((const uint8_t*)data, strlen(data), hash1);
    bool result2 = testSecMgr->generateHash((const uint8_t*)data, strlen(data), hash2);
    
    TEST_ASSERT_TRUE(result1);
    TEST_ASSERT_TRUE(result2);
    
    // Hashes should be identical for same input
    TEST_ASSERT_EQUAL_MEMORY(hash1, hash2, 32);
}

// Test: Secure Random Generation
void test_secure_random(void) {
    testSecMgr = new SecurityManager(SecurityLevel::BASIC, false);
    TEST_ASSERT_TRUE(testSecMgr->begin());
    
    uint8_t random1[32];
    uint8_t random2[32];
    
    bool result1 = testSecMgr->generateSecureRandom(random1, 32);
    bool result2 = testSecMgr->generateSecureRandom(random2, 32);
    
    TEST_ASSERT_TRUE(result1);
    TEST_ASSERT_TRUE(result2);
    
    // Random values should be different
    bool different = false;
    for (int i = 0; i < 32; i++) {
        if (random1[i] != random2[i]) {
            different = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(different);
}

// Test: Data Protector Instantiation
void test_data_protector_creation(void) {
    testDataProtector = new DataProtector();
    TEST_ASSERT_NOT_NULL(testDataProtector);
}

// Test: Data Protector Initialization
void test_data_protector_init(void) {
    testDataProtector = new DataProtector();
    TEST_ASSERT_NOT_NULL(testDataProtector);
    
    SecurityConfig config;
    config.securityLevel = SECURITY_BASIC;
    config.encryptionType = ENCRYPT_AES_256;
    config.enableAuditLogging = false;
    config.requireAuthentication = false;
    
    bool result = testDataProtector->init(config);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(testDataProtector->isSecurityEnabled());
}

// Test: User Creation
void test_user_creation(void) {
    testDataProtector = new DataProtector();
    
    SecurityConfig config;
    config.securityLevel = SECURITY_BASIC;
    config.requireAuthentication = true;
    
    TEST_ASSERT_TRUE(testDataProtector->init(config));
    
    // Create test user
    UserCredentials user;
    user.userId = "test-user-001";
    user.username = "testuser";
    user.passwordHash = testDataProtector->hashPassword("TestPass123!");
    user.role = ROLE_OPERATOR;
    user.isActive = true;
    
    bool created = testDataProtector->createUser(user);
    TEST_ASSERT_TRUE(created);
    
    // Verify user exists
    UserCredentials retrieved = testDataProtector->getUser("test-user-001");
    TEST_ASSERT_EQUAL_STRING(user.userId.c_str(), retrieved.userId.c_str());
}

// Test: Access Control
void test_access_control(void) {
    testDataProtector = new DataProtector();
    
    SecurityConfig config;
    config.securityLevel = SECURITY_BASIC;
    config.enableAccessControl = true;
    
    TEST_ASSERT_TRUE(testDataProtector->init(config));
    
    // Grant access
    bool granted = testDataProtector->grantAccess("user-001", "resource-001", ROLE_RESEARCHER);
    TEST_ASSERT_TRUE(granted);
    
    // Check access
    bool hasAccess = testDataProtector->checkAccess("user-001", "resource-001", "read");
    TEST_ASSERT_TRUE(hasAccess);
}

// Test: Audit Logging
void test_audit_logging(void) {
    testDataProtector = new DataProtector();
    
    SecurityConfig config;
    config.securityLevel = SECURITY_BASIC;
    config.enableAuditLogging = true;
    
    TEST_ASSERT_TRUE(testDataProtector->init(config));
    
    // Log event
    bool logged = testDataProtector->logAuditEvent(
        "test-user", 
        "TEST_ACTION", 
        "test-resource",
        "Test audit entry", 
        true
    );
    
    TEST_ASSERT_TRUE(logged);
    
    // Retrieve logs
    auto logs = testDataProtector->getAuditLog();
    TEST_ASSERT_GREATER_THAN(0, logs.size());
}

// Test: Password Hashing
void test_password_hashing(void) {
    testDataProtector = new DataProtector();
    
    SecurityConfig config;
    config.securityLevel = SECURITY_BASIC;
    
    TEST_ASSERT_TRUE(testDataProtector->init(config));
    
    String password = "SecurePassword123!";
    String hash1 = testDataProtector->hashPassword(password);
    String hash2 = testDataProtector->hashPassword(password);
    
    // Hashes should be identical for same password
    TEST_ASSERT_EQUAL_STRING(hash1.c_str(), hash2.c_str());
    TEST_ASSERT_GREATER_THAN(32, hash1.length());
}

// Test: Security Stats
void test_security_stats(void) {
    testSecMgr = new SecurityManager(SecurityLevel::BASIC, false);
    TEST_ASSERT_TRUE(testSecMgr->begin());
    
    // Perform some operations
    const char* data = "test";
    uint8_t ciphertext[256];
    size_t ciphertext_len;
    
    testSecMgr->encryptData((const uint8_t*)data, 4, ciphertext, &ciphertext_len);
    
    // Check stats
    unsigned long total_ops, failed_ops, boot_verifications;
    testSecMgr->getSecurityStats(&total_ops, &failed_ops, &boot_verifications);
    
    TEST_ASSERT_GREATER_THAN(0, total_ops);
}

void process(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_security_manager_creation);
    RUN_TEST(test_security_manager_init);
    RUN_TEST(test_encryption_decryption);
    RUN_TEST(test_hash_generation);
    RUN_TEST(test_secure_random);
    RUN_TEST(test_data_protector_creation);
    RUN_TEST(test_data_protector_init);
    RUN_TEST(test_user_creation);
    RUN_TEST(test_access_control);
    RUN_TEST(test_audit_logging);
    RUN_TEST(test_password_hashing);
    RUN_TEST(test_security_stats);
    
    UNITY_END();
}

#ifdef ARDUINO

void setup() {
    delay(2000);
    process();
}

void loop() {
    // Empty loop
}

#else

int main(int argc, char **argv) {
    process();
    return 0;
}

#endif
