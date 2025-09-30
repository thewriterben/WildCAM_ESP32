# Enterprise Security Features

## Overview

WildCAM ESP32 v3.0 includes enterprise-grade security features designed for production deployments in wildlife monitoring systems. These features ensure data confidentiality, integrity, and availability while meeting compliance requirements.

## Core Security Components

### 1. Security Manager (`firmware/security/security_manager.cpp`)

Provides low-level cryptographic operations and hardware security integration.

**Key Features:**
- **AES-256 Encryption**: Military-grade symmetric encryption for data protection
- **Hardware Security Element**: Optional ATECC608A integration for secure key storage
- **Secure Boot**: Firmware integrity verification at startup
- **Tamper Detection**: Runtime integrity monitoring
- **Secure Random Generation**: Cryptographically secure random number generation

**Usage Example:**
```cpp
#include "security/security_manager.h"

// Initialize security manager
SecurityManager* secMgr = new SecurityManager(SecurityLevel::ENHANCED, true);
if (!secMgr->begin()) {
    Serial.println("Security initialization failed!");
    return;
}

// Encrypt data
const uint8_t* plaintext = (const uint8_t*)"Sensitive data";
uint8_t ciphertext[256];
size_t ciphertext_len;

if (secMgr->encryptData(plaintext, 14, ciphertext, &ciphertext_len)) {
    Serial.println("Data encrypted successfully");
}

// Decrypt data
uint8_t decrypted[256];
size_t decrypted_len;

if (secMgr->decryptData(ciphertext, ciphertext_len, decrypted, &decrypted_len)) {
    Serial.println("Data decrypted successfully");
}
```

### 2. Data Protector (`ESP32WildlifeCAM-main/firmware/src/production/security/data_protector.cpp`)

Enterprise-level security management with role-based access control and audit logging.

**Key Features:**
- **Role-Based Access Control (RBAC)**: Six-tier role hierarchy
- **User Authentication**: Secure password hashing and session management
- **Audit Logging**: Comprehensive security event logging
- **Privacy Protection**: Location fuzzing and data anonymization
- **Key Management**: Secure encryption key generation and rotation
- **Session Management**: Secure token-based session handling

**Supported Roles:**
1. **VIEWER**: Read-only access
2. **OPERATOR**: Basic operations (read/write)
3. **RESEARCHER**: Research data access and analysis
4. **MANAGER**: Site management capabilities
5. **ADMIN**: System administration
6. **SUPER_ADMIN**: Full system access

**Usage Example:**
```cpp
#include "production/security/data_protector.h"

// Initialize data protector
SecurityConfig config;
config.securityLevel = SECURITY_HIGH;
config.encryptionType = ENCRYPT_AES_256;
config.enableAuditLogging = true;
config.requireAuthentication = true;
config.enableAccessControl = true;

DataProtector* protector = new DataProtector();
if (!protector->init(config)) {
    Serial.println("Data Protector initialization failed!");
    return;
}

// Create user
UserCredentials user;
user.userId = "user-001";
user.username = "john_doe";
user.passwordHash = protector->hashPassword("SecurePass123!");
user.role = ROLE_RESEARCHER;
user.isActive = true;

protector->createUser(user);

// Authenticate user
String sessionToken;
if (protector->authenticateUser("john_doe", "SecurePass123!", sessionToken)) {
    Serial.println("User authenticated successfully");
    
    // Grant access to resource
    protector->grantAccess(user.userId, "wildlife-data", ROLE_RESEARCHER);
    
    // Check access
    if (protector->checkAccess(user.userId, "wildlife-data", "read")) {
        Serial.println("User has read access");
    }
}
```

## Security Features

### Data Encryption

**At Rest:**
- All sensitive data encrypted with AES-256
- Automatic key generation and management
- Support for multiple encryption keys
- Secure key storage

**In Transit:**
- TLS/SSL support for network communications
- Certificate-based authentication
- Encrypted sensor data transmission

### Key Management

**Key Generation:**
```cpp
// Generate new encryption key
String keyId;
if (dataProtector->generateEncryptionKey(ENCRYPT_AES_256, keyId)) {
    Serial.println("Key generated: " + keyId);
}
```

**Key Rotation:**
```cpp
// Rotate encryption keys (recommended every 90 days)
if (dataProtector->rotateEncryptionKeys()) {
    Serial.println("Keys rotated successfully");
}
```

### Role-Based Access Control

**Create Users with Different Roles:**
```cpp
// Create researcher
UserCredentials researcher;
researcher.userId = "researcher-001";
researcher.username = "jane_researcher";
researcher.passwordHash = protector->hashPassword("Research2025!");
researcher.role = ROLE_RESEARCHER;
protector->createUser(researcher);

// Create operator
UserCredentials operator_user;
operator_user.userId = "operator-001";
operator_user.username = "john_operator";
operator_user.passwordHash = protector->hashPassword("Operator2025!");
operator_user.role = ROLE_OPERATOR;
protector->createUser(operator_user);
```

**Grant Resource Access:**
```cpp
// Grant researcher access to wildlife data
protector->grantAccess("researcher-001", "wildlife-data", ROLE_RESEARCHER);

// Grant operator access to camera controls
protector->grantAccess("operator-001", "camera-controls", ROLE_OPERATOR);
```

**Check Permissions:**
```cpp
// Check if user can perform action
if (protector->checkAccess("researcher-001", "wildlife-data", "analyze")) {
    // User has permission to analyze data
    performDataAnalysis();
}
```

### Audit Logging

**Log Security Events:**
```cpp
// Log data access
protector->logAuditEvent("user-001", "DATA_ACCESS", "camera-01", 
                         "Accessed camera feed", true);

// Log configuration change
protector->logAuditEvent("admin", "CONFIG_CHANGE", "system-settings",
                         "Updated recording interval", true);

// Log failed access attempt
protector->logAuditEvent("user-002", "ACCESS_DENIED", "admin-panel",
                         "Insufficient permissions", false);
```

**Retrieve Audit Log:**
```cpp
// Get all audit entries
auto auditLog = protector->getAuditLog();

// Get audit entries for specific time period
uint32_t startTime = 1609459200; // 2021-01-01 00:00:00
uint32_t endTime = 1640995200;   // 2022-01-01 00:00:00
auto filteredLog = protector->getAuditLog(startTime, endTime);

// Export audit log
protector->exportAuditLog("/audit_log.txt", startTime, endTime);
```

### Privacy Protection

**Location Fuzzing:**
```cpp
float latitude = 37.7749;
float longitude = -122.4194;
float radiusKm = 1.0; // Fuzz location within 1km

if (protector->fuzzLocation(latitude, longitude, radiusKm)) {
    Serial.println("Location fuzzed for privacy");
}
```

**Data Anonymization:**
```cpp
String data = "User: John Doe, Email: john@example.com";

if (protector->removePersonalIdentifiers(data)) {
    Serial.println("Personal identifiers removed");
}
```

**Privacy-Preserving Analytics:**
```cpp
PrivacyResult result = protector->applyPrivacyProtection(data, DATA_CONFIDENTIAL);
Serial.printf("Privacy Level: %.2f\n", result.privacyLevel);
```

## Secure OTA Updates

The OTA Manager (`ESP32WildlifeCAM-main/firmware/src/production/deployment/ota_manager.h`) provides secure firmware updates with:

- **Signature Verification**: Cryptographic verification of firmware authenticity
- **Staged Deployment**: Gradual rollout across device fleet
- **Automatic Rollback**: Revert to previous firmware on failure
- **Health Monitoring**: Post-update system health checks

**Configuration:**
```cpp
OTAConfig config;
config.updateServerURL = "https://updates.example.com";
config.deviceID = "wildCAM-001";
config.signatureVerification = true;
config.autoUpdate = false; // Manual approval required
config.stagedDeployment = true;

if (initializeOTA(config)) {
    Serial.println("OTA initialized");
}
```

## Security Best Practices

### 1. Password Policy
- Minimum 12 characters
- Mix of uppercase, lowercase, numbers, and symbols
- Regular password rotation (90 days)
- Account lockout after 5 failed attempts

### 2. Session Management
- Token-based authentication
- Session timeout (default: 30 minutes)
- Automatic session cleanup
- Secure token generation

### 3. Key Management
- Regular key rotation (168 hours)
- Hardware-backed key storage when available
- Secure key deletion
- Key version tracking

### 4. Audit Requirements
- Log all authentication attempts
- Log all access control decisions
- Log all configuration changes
- Retain logs for 90 days minimum

### 5. Network Security
- Use TLS 1.2 or higher
- Validate SSL certificates
- Implement certificate pinning
- Use VPN for remote access

## Compliance Features

### GDPR Compliance
- **Right to Access**: Export user data
- **Right to Erasure**: Secure data deletion
- **Data Minimization**: Privacy protection features
- **Audit Trail**: Complete activity logging

```cpp
// Handle data deletion request
if (protector->handleDataDeletionRequest("subject-001")) {
    Serial.println("Data deleted per GDPR request");
}

// Generate compliance report
String report;
if (protector->generateComplianceReport("GDPR", report)) {
    Serial.println(report);
}
```

### Security Monitoring

**Threat Detection:**
```cpp
// Detect security threats
if (protector->detectSecurityThreats()) {
    auto alerts = protector->getSecurityAlerts();
    for (const auto& alert : alerts) {
        Serial.println("Security Alert: " + alert);
    }
}

// Analyze access patterns
if (protector->analyzeAccessPatterns()) {
    Serial.println("Anomalous access patterns detected");
}
```

**Tamper Detection:**
```cpp
// Monitor for tampering
if (securityMgr->detectTampering()) {
    Serial.println("Tampering detected!");
    // Trigger security response
}
```

## Hardware Security

### ATECC608A Integration

The system supports optional hardware security element (ATECC608A) for:
- Secure key storage
- Hardware-backed encryption
- Device authentication
- Secure boot validation

**Initialization:**
```cpp
SecurityManager* secMgr = new SecurityManager(SecurityLevel::MAXIMUM, true);

if (secMgr->isHardwareSecurityAvailable()) {
    Serial.println("Hardware security element available");
    
    // Get hardware device ID
    uint8_t deviceId[9];
    if (secMgr->getHardwareDeviceID(deviceId)) {
        Serial.print("Device ID: ");
        for (int i = 0; i < 9; i++) {
            Serial.printf("%02x", deviceId[i]);
        }
        Serial.println();
    }
}
```

## Performance Considerations

### Encryption Overhead
- AES-256 encryption: ~50-100ms per 1KB
- Hash generation: ~10-20ms per 1KB
- Minimal impact on normal operations

### Memory Requirements
- Security Manager: ~10KB RAM
- Data Protector: ~20KB RAM (varies with user count)
- Crypto contexts: ~5KB RAM

### Storage Requirements
- Audit logs: ~100 bytes per entry
- Session tokens: ~64 bytes per session
- Encryption keys: 32 bytes per key

## Troubleshooting

### Common Issues

**1. Encryption Failures**
```cpp
unsigned long total_ops, failed_ops, boot_verifications;
secMgr->getSecurityStats(&total_ops, &failed_ops, &boot_verifications);

if (failed_ops > 0) {
    Serial.printf("Failed operations: %lu\n", failed_ops);
    // Check key initialization, memory, etc.
}
```

**2. Authentication Issues**
```cpp
// Check if user is active
UserCredentials user = protector->getUser("user-001");
if (!user.isActive) {
    Serial.println("User account locked");
    // Reset failed login attempts if needed
}
```

**3. Memory Issues**
```cpp
// Monitor free heap
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

// Cleanup expired sessions
protector->cleanupExpiredSessions();
```

## Examples

Complete working examples available in:
- `ESP32WildlifeCAM-main/examples/enterprise_security_demo/` - Full security feature demonstration
- `ESP32WildlifeCAM-main/examples/ota_update_demo/` - Secure OTA update example
- `firmware/main.cpp` - Security integration in main application

## API Reference

### Security Manager

```cpp
class SecurityManager {
    bool begin();
    bool encryptData(const uint8_t* plaintext, size_t plaintext_len,
                     uint8_t* ciphertext, size_t* ciphertext_len,
                     KeyType key_type = KeyType::DEVICE_KEY);
    bool decryptData(const uint8_t* ciphertext, size_t ciphertext_len,
                     uint8_t* plaintext, size_t* plaintext_len,
                     KeyType key_type = KeyType::DEVICE_KEY);
    bool generateHash(const uint8_t* data, size_t data_len, uint8_t hash[32]);
    bool verifyIntegrity(const uint8_t* data, size_t data_len, 
                        const uint8_t expected_hash[32]);
    bool generateSessionKey();
    bool verifyFirmwareIntegrity();
    bool detectTampering();
    bool generateSecureRandom(uint8_t* output, size_t len);
    void getSecurityStats(unsigned long* total_operations,
                         unsigned long* failed_operations,
                         unsigned long* boot_verifications);
    bool isSecurityOperational() const;
};
```

### Data Protector

```cpp
class DataProtector {
    bool init(const SecurityConfig& config);
    bool encryptData(const String& plaintext, String& encrypted, 
                    EncryptionContext& context);
    bool decryptData(const String& encrypted, const EncryptionContext& context,
                    String& plaintext);
    bool authenticateUser(const String& username, const String& password,
                         String& sessionToken);
    bool createUser(const UserCredentials& user);
    bool grantAccess(const String& userId, const String& resourceId,
                    AccessRole role);
    bool checkAccess(const String& userId, const String& resourceId,
                    const String& permission);
    bool logAuditEvent(const String& userId, const String& action,
                      const String& resourceId, const String& details = "",
                      bool success = true);
    std::vector<AuditLogEntry> getAuditLog(uint32_t startTime = 0,
                                          uint32_t endTime = 0) const;
    String hashPassword(const String& password, const String& salt = "");
    String generateSecureToken(uint32_t length = 32);
    String getSecurityStatus() const;
};
```

## Support

For security issues or questions:
- Create an issue on GitHub (for non-sensitive matters)
- Email: security@wildCAM.example.com (for sensitive security issues)
- Documentation: See `/docs/security/` for detailed guides

## License

Copyright (c) 2025 WildCAM ESP32 Team
Licensed under MIT License - See LICENSE file for details
