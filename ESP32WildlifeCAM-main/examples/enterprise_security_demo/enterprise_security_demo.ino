/**
 * Enterprise Security Features Demo
 * 
 * Demonstrates:
 * - AES-256 data encryption
 * - Secure key management
 * - Role-based access control
 * - User authentication
 * - Session management
 * - Audit logging
 * - Secure OTA updates
 * 
 * Hardware: ESP32 or ESP32-CAM
 * Security Level: ENHANCED or MAXIMUM
 */

#include <Arduino.h>
#include "../../firmware/security/security_manager.h"
#include "../../firmware/src/production/security/data_protector.h"

// Security managers
SecurityManager* securityMgr = nullptr;
DataProtector* dataProtector = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n===========================================");
    Serial.println("   WildCAM ESP32 Enterprise Security Demo");
    Serial.println("===========================================\n");
    
    // Initialize Security Manager
    Serial.println("[1] Initializing Security Manager...");
    securityMgr = new SecurityManager(SecurityLevel::ENHANCED, true);
    
    if (!securityMgr->begin()) {
        Serial.println("ERROR: Security Manager initialization failed!");
        return;
    }
    
    Serial.println("✓ Security Manager initialized successfully\n");
    
    // Initialize Data Protector with enterprise features
    Serial.println("[2] Initializing Data Protector...");
    
    SecurityConfig config;
    config.securityLevel = SECURITY_HIGH;
    config.encryptionType = ENCRYPT_AES_256;
    config.privacyMode = PRIVACY_ADVANCED;
    config.enableAuditLogging = true;
    config.requireAuthentication = true;
    config.enableAccessControl = true;
    config.encryptAtRest = true;
    config.encryptInTransit = true;
    config.sessionTimeout = 30; // 30 minutes
    
    dataProtector = new DataProtector();
    if (!dataProtector->init(config)) {
        Serial.println("ERROR: Data Protector initialization failed!");
        return;
    }
    
    Serial.println("✓ Data Protector initialized successfully\n");
    
    // Demonstrate features
    delay(1000);
    demonstrateEncryption();
    delay(1000);
    demonstrateRoleBasedAccess();
    delay(1000);
    demonstrateUserAuthentication();
    delay(1000);
    demonstrateAuditLogging();
    delay(1000);
    demonstrateKeyManagement();
    delay(1000);
    printSecurityStatus();
}

void loop() {
    // Monitor security system
    if (securityMgr && securityMgr->isSecurityOperational()) {
        // Check for tampering
        if (securityMgr->detectTampering()) {
            Serial.println("\n⚠️  WARNING: Tampering detected!");
        }
    }
    
    delay(10000); // Check every 10 seconds
}

void demonstrateEncryption() {
    Serial.println("\n=== [DEMO 1] Data Encryption ===");
    
    // Test data
    String sensitiveData = "Wildlife Location: 37.7749° N, 122.4194° W - Species: Panthera leo";
    Serial.println("Original Data: " + sensitiveData);
    
    // Encrypt using Security Manager
    Serial.println("\n[A] Testing AES-256 Encryption with Security Manager...");
    
    const uint8_t* plaintext = (const uint8_t*)sensitiveData.c_str();
    size_t plaintext_len = sensitiveData.length();
    
    uint8_t ciphertext[256];
    size_t ciphertext_len = 0;
    
    if (securityMgr->encryptData(plaintext, plaintext_len, ciphertext, &ciphertext_len)) {
        Serial.println("✓ Encryption successful");
        Serial.printf("  Ciphertext length: %d bytes\n", ciphertext_len);
        
        // Decrypt to verify
        uint8_t decrypted[256];
        size_t decrypted_len = 0;
        
        if (securityMgr->decryptData(ciphertext, ciphertext_len, decrypted, &decrypted_len)) {
            String decryptedStr = String((char*)decrypted, decrypted_len);
            Serial.println("✓ Decryption successful");
            Serial.println("  Decrypted Data: " + decryptedStr);
            
            if (decryptedStr == sensitiveData) {
                Serial.println("✓ Data integrity verified!");
            }
        }
    }
    
    // Encrypt using Data Protector
    Serial.println("\n[B] Testing Data Protector Encryption...");
    
    String encrypted;
    EncryptionContext context;
    
    if (dataProtector->encryptData(sensitiveData, encrypted, context)) {
        Serial.println("✓ Encryption successful");
        Serial.println("  Key ID: " + context.keyId);
        Serial.println("  Algorithm: " + context.algorithm);
        Serial.printf("  Encrypted length: %d bytes\n", encrypted.length());
        
        // Decrypt
        String decrypted;
        if (dataProtector->decryptData(encrypted, context, decrypted)) {
            Serial.println("✓ Decryption successful");
            Serial.println("  Decrypted Data: " + decrypted);
        }
    }
}

void demonstrateRoleBasedAccess() {
    Serial.println("\n=== [DEMO 2] Role-Based Access Control ===");
    
    // Create users with different roles
    Serial.println("\n[A] Creating Users...");
    
    // Create researcher user
    UserCredentials researcher;
    researcher.userId = "user-researcher-001";
    researcher.username = "jane_researcher";
    researcher.passwordHash = dataProtector->hashPassword("ResearchPass2025!");
    researcher.role = ROLE_RESEARCHER;
    researcher.isActive = true;
    researcher.createdTime = millis() / 1000;
    
    if (dataProtector->createUser(researcher)) {
        Serial.println("✓ Researcher user created: " + researcher.username);
    }
    
    // Create operator user
    UserCredentials operator_user;
    operator_user.userId = "user-operator-001";
    operator_user.username = "john_operator";
    operator_user.passwordHash = dataProtector->hashPassword("OperatorPass2025!");
    operator_user.role = ROLE_OPERATOR;
    operator_user.isActive = true;
    operator_user.createdTime = millis() / 1000;
    
    if (dataProtector->createUser(operator_user)) {
        Serial.println("✓ Operator user created: " + operator_user.username);
    }
    
    // Grant access to resources
    Serial.println("\n[B] Granting Access...");
    
    if (dataProtector->grantAccess(researcher.userId, "wildlife-data", ROLE_RESEARCHER)) {
        Serial.println("✓ Researcher granted access to wildlife-data");
    }
    
    if (dataProtector->grantAccess(operator_user.userId, "camera-controls", ROLE_OPERATOR)) {
        Serial.println("✓ Operator granted access to camera-controls");
    }
    
    // Test access control
    Serial.println("\n[C] Testing Access Control...");
    
    if (dataProtector->checkAccess(researcher.userId, "wildlife-data", "read")) {
        Serial.println("✓ Researcher can READ wildlife-data");
    }
    
    if (dataProtector->checkAccess(researcher.userId, "wildlife-data", "analyze")) {
        Serial.println("✓ Researcher can ANALYZE wildlife-data");
    }
    
    if (!dataProtector->checkAccess(operator_user.userId, "wildlife-data", "analyze")) {
        Serial.println("✗ Operator CANNOT ANALYZE wildlife-data (as expected)");
    }
    
    if (dataProtector->checkAccess(operator_user.userId, "camera-controls", "write")) {
        Serial.println("✓ Operator can WRITE to camera-controls");
    }
}

void demonstrateUserAuthentication() {
    Serial.println("\n=== [DEMO 3] User Authentication ===");
    
    // Test valid authentication
    Serial.println("\n[A] Testing Valid Authentication...");
    String sessionToken;
    
    if (dataProtector->authenticateUser("admin", "WildCAM2025!", sessionToken)) {
        Serial.println("✓ Admin user authenticated successfully");
        Serial.println("  Session Token: " + sessionToken.substring(0, 16) + "...");
        
        // Validate session
        String userId;
        if (dataProtector->validateSession(sessionToken, userId)) {
            Serial.println("✓ Session validated successfully");
            Serial.println("  User ID: " + userId);
        }
        
        // Terminate session
        if (dataProtector->terminateSession(sessionToken)) {
            Serial.println("✓ Session terminated successfully");
        }
    }
    
    // Test invalid authentication
    Serial.println("\n[B] Testing Invalid Authentication...");
    
    if (!dataProtector->authenticateUser("admin", "WrongPassword", sessionToken)) {
        Serial.println("✗ Authentication failed (as expected)");
    }
}

void demonstrateAuditLogging() {
    Serial.println("\n=== [DEMO 4] Audit Logging ===");
    
    // Log various events
    Serial.println("\n[A] Logging Security Events...");
    
    dataProtector->logAuditEvent("admin", "DATA_ACCESS", "wildlife-camera-01", 
                                 "Accessed camera feed", true);
    Serial.println("✓ Logged: DATA_ACCESS event");
    
    dataProtector->logAuditEvent("researcher-001", "DATA_EXPORT", "detection-data", 
                                 "Exported detection data", true);
    Serial.println("✓ Logged: DATA_EXPORT event");
    
    dataProtector->logAuditEvent("operator-001", "CONFIG_CHANGE", "camera-settings", 
                                 "Updated camera resolution", true);
    Serial.println("✓ Logged: CONFIG_CHANGE event");
    
    // Retrieve audit log
    Serial.println("\n[B] Retrieving Audit Log...");
    
    auto auditLog = dataProtector->getAuditLog();
    Serial.printf("Total Audit Entries: %d\n", auditLog.size());
    
    if (!auditLog.empty()) {
        Serial.println("\nRecent Audit Entries:");
        int count = 0;
        for (auto it = auditLog.rbegin(); it != auditLog.rend() && count < 5; ++it, ++count) {
            Serial.printf("  [%s] %s -> %s: %s\n", 
                         it->userId.c_str(),
                         it->action.c_str(),
                         it->resourceId.c_str(),
                         it->details.c_str());
        }
    }
}

void demonstrateKeyManagement() {
    Serial.println("\n=== [DEMO 5] Secure Key Management ===");
    
    // Generate new encryption keys
    Serial.println("\n[A] Generating Encryption Keys...");
    
    String keyId1, keyId2;
    
    if (dataProtector->generateEncryptionKey(ENCRYPT_AES_256, keyId1)) {
        Serial.println("✓ Generated AES-256 key: " + keyId1);
    }
    
    if (dataProtector->generateEncryptionKey(ENCRYPT_AES_256, keyId2)) {
        Serial.println("✓ Generated AES-256 key: " + keyId2);
    }
    
    // List all keys
    Serial.println("\n[B] Listing Encryption Keys...");
    auto keys = dataProtector->listKeys();
    Serial.printf("Total Keys: %d\n", keys.size());
    for (const auto& key : keys) {
        Serial.println("  - " + key);
    }
    
    // Generate session key
    Serial.println("\n[C] Generating Session Key...");
    if (securityMgr->generateSessionKey()) {
        Serial.println("✓ Session key generated successfully");
    }
    
    // Test secure random number generation
    Serial.println("\n[D] Generating Secure Random Data...");
    uint8_t randomData[16];
    if (securityMgr->generateSecureRandom(randomData, 16)) {
        Serial.print("✓ Random data: ");
        for (int i = 0; i < 16; i++) {
            Serial.printf("%02x", randomData[i]);
        }
        Serial.println();
    }
}

void printSecurityStatus() {
    Serial.println("\n=== [SECURITY STATUS] ===");
    
    // Security Manager Status
    Serial.println("\n[Security Manager]");
    unsigned long total_ops, failed_ops, boot_verifications;
    securityMgr->getSecurityStats(&total_ops, &failed_ops, &boot_verifications);
    
    Serial.printf("  Encryption Operations: %lu\n", total_ops);
    Serial.printf("  Failed Operations: %lu\n", failed_ops);
    Serial.printf("  Boot Verifications: %lu\n", boot_verifications);
    Serial.printf("  Security Level: %s\n", 
                  securityMgr->getSecurityLevel() == SecurityLevel::ENHANCED ? "ENHANCED" : "BASIC");
    Serial.printf("  Operational: %s\n", 
                  securityMgr->isSecurityOperational() ? "YES" : "NO");
    Serial.printf("  Hardware Security: %s\n", 
                  securityMgr->isHardwareSecurityAvailable() ? "YES" : "NO");
    
    // Data Protector Status
    Serial.println("\n[Data Protector]");
    Serial.print(dataProtector->getSecurityStatus());
    
    Serial.println("\n===========================================");
    Serial.println("   Enterprise Security Demo Complete!");
    Serial.println("===========================================\n");
}
