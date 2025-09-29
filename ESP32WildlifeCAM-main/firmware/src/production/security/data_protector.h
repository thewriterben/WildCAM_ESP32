/**
 * Data Protector - Security and Privacy Protection System
 * 
 * Provides comprehensive data protection, encryption, and privacy features
 * for wildlife monitoring systems. Ensures compliance with data protection
 * regulations and protects sensitive wildlife location data.
 * 
 * Features:
 * - End-to-end encryption for all data
 * - Role-based access control
 * - Privacy-preserving analytics
 * - Audit logging and compliance
 * - Secure key management
 */

#ifndef DATA_PROTECTOR_H
#define DATA_PROTECTOR_H

#include <Arduino.h>
#include <mbedtls/aes.h>
#include <mbedtls/rsa.h>
#include <mbedtls/sha256.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <vector>
#include <map>

// Security Levels
enum SecurityLevel {
    SECURITY_NONE,          // No security (for testing only)
    SECURITY_BASIC,         // Basic encryption
    SECURITY_STANDARD,      // Standard enterprise security
    SECURITY_HIGH,          // High security for sensitive data
    SECURITY_MAXIMUM        // Maximum security for critical systems
};

// Data Classification Levels
enum DataClassification {
    DATA_PUBLIC,            // Public information
    DATA_INTERNAL,          // Internal use only
    DATA_CONFIDENTIAL,      // Confidential data
    DATA_RESTRICTED,        // Restricted access data
    DATA_TOP_SECRET         // Highest sensitivity data
};

// Access Control Roles
enum AccessRole {
    ROLE_VIEWER,            // Read-only access
    ROLE_OPERATOR,          // Basic operations
    ROLE_RESEARCHER,        // Research data access
    ROLE_MANAGER,           // Site management
    ROLE_ADMIN,             // System administration
    ROLE_SUPER_ADMIN        // Full system access
};

// Encryption Types
enum EncryptionType {
    ENCRYPT_AES_128,        // AES 128-bit encryption
    ENCRYPT_AES_256,        // AES 256-bit encryption
    ENCRYPT_RSA_2048,       // RSA 2048-bit encryption
    ENCRYPT_RSA_4096,       // RSA 4096-bit encryption
    ENCRYPT_HYBRID          // Hybrid AES + RSA encryption
};

// Privacy Protection Modes
enum PrivacyMode {
    PRIVACY_NONE,           // No privacy protection
    PRIVACY_BASIC,          // Basic location fuzzing
    PRIVACY_ADVANCED,       // Advanced privacy protection
    PRIVACY_ANONYMOUS,      // Full anonymization
    PRIVACY_RESEARCH_ONLY   // Research-grade protection
};

// Security Configuration
struct SecurityConfig {
    SecurityLevel securityLevel;
    EncryptionType encryptionType;
    PrivacyMode privacyMode;
    bool enableAuditLogging;
    bool requireAuthentication;
    bool enableAccessControl;
    bool encryptAtRest;
    bool encryptInTransit;
    uint32_t keyRotationInterval;   // Key rotation interval in hours
    uint32_t sessionTimeout;        // Session timeout in minutes
    bool enableTwoFactor;          // Two-factor authentication
    String certificatePath;        // SSL certificate path
    String privateKeyPath;         // Private key path
    
    SecurityConfig() : 
        securityLevel(SECURITY_STANDARD), encryptionType(ENCRYPT_AES_256),
        privacyMode(PRIVACY_ADVANCED), enableAuditLogging(true),
        requireAuthentication(true), enableAccessControl(true),
        encryptAtRest(true), encryptInTransit(true), keyRotationInterval(168),
        sessionTimeout(60), enableTwoFactor(false), certificatePath(""),
        privateKeyPath("") {}
};

// User Credentials
struct UserCredentials {
    String userId;
    String username;
    String passwordHash;
    AccessRole role;
    std::vector<String> permissions;
    uint32_t lastLogin;
    uint32_t createdTime;
    bool isActive;
    bool requiresPasswordChange;
    String twoFactorSecret;
    uint32_t failedLoginAttempts;
    
    UserCredentials() : 
        userId(""), username(""), passwordHash(""), role(ROLE_VIEWER),
        lastLogin(0), createdTime(0), isActive(true), requiresPasswordChange(false),
        twoFactorSecret(""), failedLoginAttempts(0) {}
};

// Access Control Entry
struct AccessControlEntry {
    String resourceId;         // Resource identifier
    String userId;             // User identifier
    AccessRole role;           // User role
    std::vector<String> permissions; // Specific permissions
    uint32_t grantedTime;      // When access was granted
    uint32_t expiryTime;       // When access expires (0 = never)
    bool isActive;             // Whether access is currently active
    
    AccessControlEntry() : 
        resourceId(""), userId(""), role(ROLE_VIEWER), grantedTime(0),
        expiryTime(0), isActive(true) {}
};

// Audit Log Entry
struct AuditLogEntry {
    String eventId;            // Unique event identifier
    uint32_t timestamp;        // Event timestamp
    String userId;             // User who performed action
    String action;             // Action performed
    String resourceId;         // Resource affected
    String details;            // Additional details
    String sourceIP;           // Source IP address
    bool success;              // Whether action was successful
    String errorMessage;       // Error message if failed
    
    AuditLogEntry() : 
        eventId(""), timestamp(0), userId(""), action(""), resourceId(""),
        details(""), sourceIP(""), success(true), errorMessage("") {}
};

// Encryption Context
struct EncryptionContext {
    String keyId;              // Encryption key identifier
    EncryptionType type;       // Encryption type used
    String algorithm;          // Specific algorithm
    String initVector;         // Initialization vector
    uint32_t keyVersion;       // Key version number
    uint32_t encryptionTime;   // When data was encrypted
    
    EncryptionContext() : 
        keyId(""), type(ENCRYPT_AES_256), algorithm(""), initVector(""),
        keyVersion(0), encryptionTime(0) {}
};

// Privacy Protection Result
struct PrivacyResult {
    bool locationFuzzed;       // Location was fuzzed
    bool identifierRemoved;    // Personal identifiers removed
    bool timestampRounded;     // Timestamp rounded for privacy
    bool dataAggregated;       // Data was aggregated
    float privacyLevel;        // Privacy protection level (0-1)
    String protectionMethod;   // Method used for protection
    
    PrivacyResult() : 
        locationFuzzed(false), identifierRemoved(false), timestampRounded(false),
        dataAggregated(false), privacyLevel(0.0), protectionMethod("") {}
};

/**
 * Data Protector Class
 * 
 * Comprehensive data protection and security management system
 */
class DataProtector {
public:
    DataProtector();
    ~DataProtector();
    
    // Initialization and configuration
    bool init(const SecurityConfig& config);
    void cleanup();
    bool configure(const SecurityConfig& config);
    SecurityConfig getConfiguration() const { return config_; }
    
    // Encryption and decryption
    bool encryptData(const String& plaintext, String& encrypted, EncryptionContext& context);
    bool decryptData(const String& encrypted, const EncryptionContext& context, String& plaintext);
    bool encryptFile(const String& inputFile, const String& outputFile, EncryptionContext& context);
    bool decryptFile(const String& inputFile, const EncryptionContext& context, const String& outputFile);
    
    // Key management
    bool generateEncryptionKey(EncryptionType type, String& keyId);
    bool rotateEncryptionKeys();
    bool importKey(const String& keyData, EncryptionType type, String& keyId);
    bool exportKey(const String& keyId, String& keyData);
    bool deleteKey(const String& keyId);
    std::vector<String> listKeys() const;
    
    // User authentication and management
    bool authenticateUser(const String& username, const String& password, String& sessionToken);
    bool createUser(const UserCredentials& user);
    bool updateUser(const UserCredentials& user);
    bool deleteUser(const String& userId);
    bool changePassword(const String& userId, const String& newPassword);
    UserCredentials getUser(const String& userId) const;
    std::vector<UserCredentials> getAllUsers() const;
    
    // Access control
    bool grantAccess(const String& userId, const String& resourceId, AccessRole role);
    bool revokeAccess(const String& userId, const String& resourceId);
    bool checkAccess(const String& userId, const String& resourceId, const String& permission);
    std::vector<AccessControlEntry> getUserAccess(const String& userId) const;
    std::vector<AccessControlEntry> getResourceAccess(const String& resourceId) const;
    
    // Session management
    bool createSession(const String& userId, String& sessionToken);
    bool validateSession(const String& sessionToken, String& userId);
    bool refreshSession(const String& sessionToken);
    bool terminateSession(const String& sessionToken);
    void cleanupExpiredSessions();
    
    // Privacy protection
    PrivacyResult applyPrivacyProtection(String& data, DataClassification classification);
    bool fuzzLocation(float& latitude, float& longitude, float radiusKm);
    bool removePersonalIdentifiers(String& data);
    bool anonymizeTimestamp(uint32_t& timestamp, uint32_t roundingMinutes);
    String generateAnonymousId(const String& originalId);
    
    // Audit logging
    bool logAuditEvent(const String& userId, const String& action, const String& resourceId, 
                       const String& details = "", bool success = true);
    std::vector<AuditLogEntry> getAuditLog(uint32_t startTime = 0, uint32_t endTime = 0) const;
    bool exportAuditLog(const String& filename, uint32_t startTime = 0, uint32_t endTime = 0) const;
    void clearOldAuditEntries(uint32_t retentionDays);
    
    // Data classification
    bool classifyData(const String& data, DataClassification& classification);
    bool setDataClassification(const String& resourceId, DataClassification classification);
    DataClassification getDataClassification(const String& resourceId) const;
    
    // Compliance features
    bool generateComplianceReport(const String& reportType, String& report);
    bool validateGDPRCompliance();
    bool performDataInventory(std::vector<String>& dataInventory);
    bool handleDataDeletionRequest(const String& subjectId);
    
    // Secure communication
    bool establishSecureChannel(const String& remoteEndpoint, String& channelId);
    bool sendSecureMessage(const String& channelId, const String& message);
    bool receiveSecureMessage(const String& channelId, String& message);
    bool closeSecureChannel(const String& channelId);
    
    // Certificate management
    bool loadCertificate(const String& certificatePath);
    bool validateCertificate(const String& certificate);
    bool generateSelfSignedCertificate();
    String getCertificateFingerprint() const;
    
    // Threat detection
    bool detectSecurityThreats();
    bool analyzeAccessPatterns();
    bool detectAnomalousActivity(const String& userId);
    std::vector<String> getSecurityAlerts() const;
    
    // Backup and recovery
    bool createSecurityBackup(const String& backupPath);
    bool restoreFromSecurityBackup(const String& backupPath);
    bool verifyBackupIntegrity(const String& backupPath);
    
    // Utility functions
    String hashPassword(const String& password, const String& salt = "");
    String generateSalt();
    String generateSecureToken(uint32_t length = 32);
    bool secureDelete(const String& filePath);
    String calculateChecksum(const String& data);
    bool validateDataIntegrity(const String& data, const String& checksum);
    
    // Status and monitoring
    bool isSecurityEnabled() const { return initialized_ && config_.securityLevel > SECURITY_NONE; }
    SecurityLevel getSecurityLevel() const { return config_.securityLevel; }
    uint32_t getActiveSessionCount() const;
    uint32_t getFailedLoginAttempts() const;
    String getSecurityStatus() const;
    
    // Event callbacks
    typedef void (*SecurityEventCallback)(const String& event, const String& details);
    typedef void (*AuthenticationCallback)(const String& userId, bool success);
    typedef void (*AccessViolationCallback)(const String& userId, const String& resource);
    typedef void (*ThreatDetectedCallback)(const String& threat, const String& source);
    
    void setSecurityEventCallback(SecurityEventCallback callback) { securityCallback_ = callback; }
    void setAuthenticationCallback(AuthenticationCallback callback) { authCallback_ = callback; }
    void setAccessViolationCallback(AccessViolationCallback callback) { violationCallback_ = callback; }
    void setThreatDetectedCallback(ThreatDetectedCallback callback) { threatCallback_ = callback; }

private:
    // Core configuration and state
    SecurityConfig config_;
    bool initialized_;
    
    // User and session management
    std::map<String, UserCredentials> users_;
    std::map<String, String> activeSessions_;  // sessionToken -> userId
    std::map<String, uint32_t> sessionExpiry_;
    
    // Access control
    std::vector<AccessControlEntry> accessControlList_;
    
    // Encryption keys
    std::map<String, uint8_t*> encryptionKeys_;
    std::map<String, EncryptionType> keyTypes_;
    uint32_t currentKeyVersion_;
    
    // Audit logging
    std::vector<AuditLogEntry> auditLog_;
    uint32_t auditRetentionDays_;
    
    // mbedTLS contexts
    mbedtls_aes_context aesContext_;
    mbedtls_rsa_context rsaContext_;
    mbedtls_entropy_context entropyContext_;
    mbedtls_ctr_drbg_context ctrDrbgContext_;
    
    // Callbacks
    SecurityEventCallback securityCallback_;
    AuthenticationCallback authCallback_;
    AccessViolationCallback violationCallback_;
    ThreatDetectedCallback threatCallback_;
    
    // Internal methods
    bool initializeCrypto();
    bool generateRandomData(uint8_t* buffer, size_t length);
    bool performAESEncryption(const uint8_t* plaintext, size_t length, 
                             const uint8_t* key, const uint8_t* iv, uint8_t* ciphertext);
    bool performAESDecryption(const uint8_t* ciphertext, size_t length,
                             const uint8_t* key, const uint8_t* iv, uint8_t* plaintext);
    
    // User management helpers
    bool validateUsername(const String& username) const;
    bool validatePassword(const String& password) const;
    String hashWithSalt(const String& data, const String& salt);
    bool verifyPasswordHash(const String& password, const String& hash);
    
    // Access control helpers
    bool hasPermission(const String& userId, const String& permission) const;
    AccessRole getUserRole(const String& userId) const;
    void addDefaultPermissions(AccessRole role, std::vector<String>& permissions);
    
    // Privacy protection helpers
    float calculateFuzzingRadius(DataClassification classification) const;
    bool shouldApplyPrivacyProtection(DataClassification classification) const;
    String applyKAnonymity(const String& data, int k) const;
    
    // Audit helpers
    String generateEventId();
    void addAuditEntry(const AuditLogEntry& entry);
    bool shouldLogEvent(const String& action) const;
    
    // Security monitoring
    void checkSecurityThresholds();
    void updateSecurityMetrics();
    bool detectBruteForceAttack(const String& userId) const;
    
    // Notification helpers
    void notifySecurityEvent(const String& event, const String& details);
    void notifyAuthentication(const String& userId, bool success);
    void notifyAccessViolation(const String& userId, const String& resource);
    void notifyThreatDetected(const String& threat, const String& source);
};

// Global data protector instance
extern DataProtector* g_dataProtector;

// Utility functions for easy integration
bool initializeDataProtection(const SecurityConfig& config);
bool encryptSensitiveData(const String& data, String& encrypted);
bool authenticateRequest(const String& sessionToken, String& userId);
bool checkUserPermission(const String& userId, const String& resource, const String& action);
void cleanupDataProtection();

// Quick access functions
bool isSecurityEnabled();
SecurityLevel getCurrentSecurityLevel();
bool hasActiveSession(const String& sessionToken);
String getSecuritySummary();
bool hasSecurityAlerts();

#endif // DATA_PROTECTOR_H