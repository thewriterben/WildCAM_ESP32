/**
 * Data Protector Implementation
 * 
 * Implements comprehensive data protection, encryption, and role-based access control
 * for enterprise-grade wildlife monitoring systems.
 */

#include "data_protector.h"
#include <esp_random.h>

// Global instance
DataProtector* g_dataProtector = nullptr;

// Constructor
DataProtector::DataProtector() 
    : initialized_(false),
      currentKeyVersion_(1),
      auditRetentionDays_(90),
      securityCallback_(nullptr),
      authCallback_(nullptr),
      violationCallback_(nullptr),
      threatCallback_(nullptr) {
}

// Destructor
DataProtector::~DataProtector() {
    cleanup();
}

// Initialize data protector
bool DataProtector::init(const SecurityConfig& config) {
    Serial.println("[DataProtector] Initializing enterprise security...");
    
    config_ = config;
    
    // Initialize mbedTLS contexts
    if (!initializeCrypto()) {
        Serial.println("[DataProtector] ERROR: Failed to initialize crypto");
        return false;
    }
    
    // Generate initial encryption key
    String keyId;
    if (!generateEncryptionKey(config_.encryptionType, keyId)) {
        Serial.println("[DataProtector] ERROR: Failed to generate encryption key");
        return false;
    }
    
    Serial.printf("[DataProtector] Generated initial key: %s\n", keyId.c_str());
    
    // Create default admin user if authentication is enabled
    if (config_.requireAuthentication) {
        UserCredentials admin;
        admin.userId = "admin-001";
        admin.username = "admin";
        admin.passwordHash = hashPassword("WildCAM2025!");
        admin.role = ROLE_SUPER_ADMIN;
        admin.isActive = true;
        admin.createdTime = millis() / 1000;
        
        if (createUser(admin)) {
            Serial.println("[DataProtector] Default admin user created");
        }
    }
    
    initialized_ = true;
    Serial.println("[DataProtector] Enterprise security initialized successfully");
    
    return true;
}

// Cleanup resources
void DataProtector::cleanup() {
    // Free encryption keys
    for (auto& pair : encryptionKeys_) {
        if (pair.second != nullptr) {
            secureWipe(pair.second, 32);
            free(pair.second);
        }
    }
    encryptionKeys_.clear();
    
    // Free mbedTLS contexts
    mbedtls_aes_free(&aesContext_);
    mbedtls_rsa_free(&rsaContext_);
    mbedtls_entropy_free(&entropyContext_);
    mbedtls_ctr_drbg_free(&ctrDrbgContext_);
    
    initialized_ = false;
}

// Initialize cryptographic contexts
bool DataProtector::initializeCrypto() {
    mbedtls_aes_init(&aesContext_);
    mbedtls_rsa_init(&rsaContext_);
    mbedtls_entropy_init(&entropyContext_);
    mbedtls_ctr_drbg_init(&ctrDrbgContext_);
    
    const char* personalization = "WildCAM_DataProtector";
    int ret = mbedtls_ctr_drbg_seed(&ctrDrbgContext_, mbedtls_entropy_func, &entropyContext_,
                                    (const unsigned char*)personalization,
                                    strlen(personalization));
    
    if (ret != 0) {
        Serial.printf("[DataProtector] RNG initialization failed: -0x%04x\n", -ret);
        return false;
    }
    
    return true;
}

// Generate random data
bool DataProtector::generateRandomData(uint8_t* buffer, size_t length) {
    if (buffer == nullptr || length == 0) {
        return false;
    }
    
    int ret = mbedtls_ctr_drbg_random(&ctrDrbgContext_, buffer, length);
    return (ret == 0);
}

// Encrypt data
bool DataProtector::encryptData(const String& plaintext, String& encrypted, EncryptionContext& context) {
    if (!initialized_ || plaintext.isEmpty()) {
        return false;
    }
    
    // Get current encryption key
    if (encryptionKeys_.empty()) {
        Serial.println("[DataProtector] No encryption keys available");
        return false;
    }
    
    String keyId = encryptionKeys_.begin()->first;
    uint8_t* key = encryptionKeys_.begin()->second;
    
    // Generate IV
    uint8_t iv[16];
    if (!generateRandomData(iv, 16)) {
        return false;
    }
    
    // Prepare plaintext
    const uint8_t* plain_data = (const uint8_t*)plaintext.c_str();
    size_t plain_len = plaintext.length();
    
    // Calculate padded length
    size_t padded_len = ((plain_len + 15) / 16) * 16;
    uint8_t* padded_data = (uint8_t*)malloc(padded_len);
    if (padded_data == nullptr) {
        return false;
    }
    
    memcpy(padded_data, plain_data, plain_len);
    uint8_t padding = padded_len - plain_len;
    for (size_t i = plain_len; i < padded_len; i++) {
        padded_data[i] = padding;
    }
    
    // Encrypt
    uint8_t* cipher_data = (uint8_t*)malloc(padded_len);
    if (cipher_data == nullptr) {
        free(padded_data);
        return false;
    }
    
    if (!performAESEncryption(padded_data, padded_len, key, iv, cipher_data)) {
        free(padded_data);
        free(cipher_data);
        return false;
    }
    
    // Encode result as hex string with IV prepended
    encrypted = "";
    for (int i = 0; i < 16; i++) {
        char hex[3];
        sprintf(hex, "%02x", iv[i]);
        encrypted += hex;
    }
    for (size_t i = 0; i < padded_len; i++) {
        char hex[3];
        sprintf(hex, "%02x", cipher_data[i]);
        encrypted += hex;
    }
    
    // Set context
    context.keyId = keyId;
    context.type = config_.encryptionType;
    context.algorithm = "AES-256-CBC";
    context.keyVersion = currentKeyVersion_;
    context.encryptionTime = millis() / 1000;
    
    free(padded_data);
    free(cipher_data);
    
    return true;
}

// Decrypt data
bool DataProtector::decryptData(const String& encrypted, const EncryptionContext& context, String& plaintext) {
    if (!initialized_ || encrypted.isEmpty()) {
        return false;
    }
    
    // Find key
    auto keyIt = encryptionKeys_.find(context.keyId);
    if (keyIt == encryptionKeys_.end()) {
        Serial.println("[DataProtector] Encryption key not found");
        return false;
    }
    
    uint8_t* key = keyIt->second;
    
    // Decode hex string
    if (encrypted.length() < 32 || encrypted.length() % 2 != 0) {
        return false;
    }
    
    // Extract IV
    uint8_t iv[16];
    for (int i = 0; i < 16; i++) {
        String byteStr = encrypted.substring(i * 2, i * 2 + 2);
        iv[i] = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    }
    
    // Extract ciphertext
    size_t cipher_len = (encrypted.length() - 32) / 2;
    uint8_t* cipher_data = (uint8_t*)malloc(cipher_len);
    if (cipher_data == nullptr) {
        return false;
    }
    
    for (size_t i = 0; i < cipher_len; i++) {
        String byteStr = encrypted.substring(32 + i * 2, 32 + i * 2 + 2);
        cipher_data[i] = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    }
    
    // Decrypt
    uint8_t* plain_data = (uint8_t*)malloc(cipher_len);
    if (plain_data == nullptr) {
        free(cipher_data);
        return false;
    }
    
    if (!performAESDecryption(cipher_data, cipher_len, key, iv, plain_data)) {
        free(cipher_data);
        free(plain_data);
        return false;
    }
    
    // Remove padding
    uint8_t padding = plain_data[cipher_len - 1];
    if (padding > 0 && padding <= 16) {
        plaintext = String((char*)plain_data, cipher_len - padding);
    } else {
        plaintext = String((char*)plain_data, cipher_len);
    }
    
    free(cipher_data);
    free(plain_data);
    
    return true;
}

// Perform AES encryption
bool DataProtector::performAESEncryption(const uint8_t* plaintext, size_t length,
                                        const uint8_t* key, const uint8_t* iv,
                                        uint8_t* ciphertext) {
    int ret = mbedtls_aes_setkey_enc(&aesContext_, key, 256);
    if (ret != 0) {
        return false;
    }
    
    uint8_t iv_copy[16];
    memcpy(iv_copy, iv, 16);
    
    ret = mbedtls_aes_crypt_cbc(&aesContext_, MBEDTLS_AES_ENCRYPT, length,
                                iv_copy, plaintext, ciphertext);
    
    return (ret == 0);
}

// Perform AES decryption
bool DataProtector::performAESDecryption(const uint8_t* ciphertext, size_t length,
                                        const uint8_t* key, const uint8_t* iv,
                                        uint8_t* plaintext) {
    int ret = mbedtls_aes_setkey_dec(&aesContext_, key, 256);
    if (ret != 0) {
        return false;
    }
    
    uint8_t iv_copy[16];
    memcpy(iv_copy, iv, 16);
    
    ret = mbedtls_aes_crypt_cbc(&aesContext_, MBEDTLS_AES_DECRYPT, length,
                                iv_copy, ciphertext, plaintext);
    
    return (ret == 0);
}

// Generate encryption key
bool DataProtector::generateEncryptionKey(EncryptionType type, String& keyId) {
    uint8_t* key = (uint8_t*)malloc(32);
    if (key == nullptr) {
        return false;
    }
    
    if (!generateRandomData(key, 32)) {
        free(key);
        return false;
    }
    
    // Generate key ID
    keyId = "key-";
    keyId += String(currentKeyVersion_);
    keyId += "-";
    keyId += String(millis());
    
    encryptionKeys_[keyId] = key;
    keyTypes_[keyId] = type;
    
    Serial.printf("[DataProtector] Generated key: %s\n", keyId.c_str());
    
    return true;
}

// Authenticate user
bool DataProtector::authenticateUser(const String& username, const String& password, String& sessionToken) {
    if (!config_.requireAuthentication) {
        return true;
    }
    
    // Find user by username
    UserCredentials* user = nullptr;
    for (auto& pair : users_) {
        if (pair.second.username == username) {
            user = &pair.second;
            break;
        }
    }
    
    if (user == nullptr) {
        Serial.printf("[DataProtector] User not found: %s\n", username.c_str());
        if (authCallback_) authCallback_(username, false);
        return false;
    }
    
    // Check if user is active
    if (!user->isActive) {
        Serial.printf("[DataProtector] User inactive: %s\n", username.c_str());
        if (authCallback_) authCallback_(username, false);
        return false;
    }
    
    // Verify password
    String passwordHash = hashPassword(password);
    if (passwordHash != user->passwordHash) {
        user->failedLoginAttempts++;
        Serial.printf("[DataProtector] Invalid password for user: %s\n", username.c_str());
        if (authCallback_) authCallback_(username, false);
        
        // Lock account after 5 failed attempts
        if (user->failedLoginAttempts >= 5) {
            user->isActive = false;
            Serial.printf("[DataProtector] Account locked: %s\n", username.c_str());
        }
        return false;
    }
    
    // Reset failed login attempts
    user->failedLoginAttempts = 0;
    user->lastLogin = millis() / 1000;
    
    // Create session
    if (!createSession(user->userId, sessionToken)) {
        return false;
    }
    
    // Log authentication
    logAuditEvent(user->userId, "LOGIN", "system", "User logged in", true);
    
    if (authCallback_) authCallback_(username, true);
    
    Serial.printf("[DataProtector] User authenticated: %s\n", username.c_str());
    
    return true;
}

// Create user
bool DataProtector::createUser(const UserCredentials& user) {
    if (users_.find(user.userId) != users_.end()) {
        Serial.printf("[DataProtector] User already exists: %s\n", user.userId.c_str());
        return false;
    }
    
    users_[user.userId] = user;
    
    logAuditEvent("system", "CREATE_USER", user.userId, 
                  "User created: " + user.username, true);
    
    return true;
}

// Grant access
bool DataProtector::grantAccess(const String& userId, const String& resourceId, AccessRole role) {
    if (!config_.enableAccessControl) {
        return true;
    }
    
    // Check if user exists
    if (users_.find(userId) == users_.end()) {
        Serial.printf("[DataProtector] User not found: %s\n", userId.c_str());
        return false;
    }
    
    // Create access control entry
    AccessControlEntry ace;
    ace.userId = userId;
    ace.resourceId = resourceId;
    ace.role = role;
    ace.grantedTime = millis() / 1000;
    ace.expiryTime = 0; // Never expires
    ace.isActive = true;
    
    // Add default permissions for role
    addDefaultPermissions(role, ace.permissions);
    
    accessControlList_.push_back(ace);
    
    logAuditEvent("system", "GRANT_ACCESS", resourceId,
                  "Access granted to user: " + userId, true);
    
    Serial.printf("[DataProtector] Access granted: %s -> %s\n", 
                  userId.c_str(), resourceId.c_str());
    
    return true;
}

// Check access
bool DataProtector::checkAccess(const String& userId, const String& resourceId, 
                               const String& permission) {
    if (!config_.enableAccessControl) {
        return true;
    }
    
    // Check if user has SUPER_ADMIN role (bypass all checks)
    auto userIt = users_.find(userId);
    if (userIt != users_.end() && userIt->second.role == ROLE_SUPER_ADMIN) {
        return true;
    }
    
    // Find matching access control entry
    for (const auto& ace : accessControlList_) {
        if (ace.userId == userId && ace.resourceId == resourceId && ace.isActive) {
            // Check if access has expired
            if (ace.expiryTime > 0 && (millis() / 1000) > ace.expiryTime) {
                continue;
            }
            
            // Check if user has the required permission
            for (const auto& perm : ace.permissions) {
                if (perm == permission || perm == "all") {
                    return true;
                }
            }
        }
    }
    
    // Access denied - log violation
    logAuditEvent(userId, "ACCESS_DENIED", resourceId,
                  "Permission denied: " + permission, false);
    
    if (violationCallback_) {
        violationCallback_(userId, resourceId);
    }
    
    return false;
}

// Create session
bool DataProtector::createSession(const String& userId, String& sessionToken) {
    // Generate secure session token
    sessionToken = generateSecureToken(32);
    
    activeSessions_[sessionToken] = userId;
    sessionExpiry_[sessionToken] = millis() / 1000 + (config_.sessionTimeout * 60);
    
    return true;
}

// Validate session
bool DataProtector::validateSession(const String& sessionToken, String& userId) {
    auto it = activeSessions_.find(sessionToken);
    if (it == activeSessions_.end()) {
        return false;
    }
    
    // Check if session has expired
    uint32_t currentTime = millis() / 1000;
    if (sessionExpiry_[sessionToken] < currentTime) {
        terminateSession(sessionToken);
        return false;
    }
    
    userId = it->second;
    return true;
}

// Terminate session
bool DataProtector::terminateSession(const String& sessionToken) {
    activeSessions_.erase(sessionToken);
    sessionExpiry_.erase(sessionToken);
    return true;
}

// Apply privacy protection
PrivacyResult DataProtector::applyPrivacyProtection(String& data, DataClassification classification) {
    PrivacyResult result;
    
    if (config_.privacyMode == PRIVACY_NONE) {
        return result;
    }
    
    // Apply privacy based on classification
    if (classification >= DATA_CONFIDENTIAL) {
        // Remove personal identifiers
        if (removePersonalIdentifiers(data)) {
            result.identifierRemoved = true;
        }
    }
    
    // Calculate privacy level
    result.privacyLevel = 0.0;
    if (result.identifierRemoved) result.privacyLevel += 0.3;
    if (result.locationFuzzed) result.privacyLevel += 0.4;
    if (result.timestampRounded) result.privacyLevel += 0.3;
    
    result.protectionMethod = "Standard Privacy Protection";
    
    return result;
}

// Fuzz location for privacy
bool DataProtector::fuzzLocation(float& latitude, float& longitude, float radiusKm) {
    // Generate random offset within radius
    float angle = ((float)esp_random() / UINT32_MAX) * 2.0 * PI;
    float distance = ((float)esp_random() / UINT32_MAX) * radiusKm;
    
    // Convert to lat/lon offset (approximate)
    float latOffset = (distance / 111.0) * cos(angle);
    float lonOffset = (distance / (111.0 * cos(latitude * PI / 180.0))) * sin(angle);
    
    latitude += latOffset;
    longitude += lonOffset;
    
    return true;
}

// Remove personal identifiers
bool DataProtector::removePersonalIdentifiers(String& data) {
    // Simple implementation - in production, use regex or NLP
    data.replace("name", "***");
    data.replace("email", "***");
    data.replace("phone", "***");
    data.replace("address", "***");
    
    return true;
}

// Log audit event
bool DataProtector::logAuditEvent(const String& userId, const String& action,
                                 const String& resourceId, const String& details,
                                 bool success) {
    if (!config_.enableAuditLogging) {
        return true;
    }
    
    AuditLogEntry entry;
    entry.eventId = generateEventId();
    entry.timestamp = millis() / 1000;
    entry.userId = userId;
    entry.action = action;
    entry.resourceId = resourceId;
    entry.details = details;
    entry.success = success;
    
    auditLog_.push_back(entry);
    
    // Limit audit log size
    if (auditLog_.size() > 10000) {
        auditLog_.erase(auditLog_.begin());
    }
    
    return true;
}

// Get audit log
std::vector<AuditLogEntry> DataProtector::getAuditLog(uint32_t startTime, uint32_t endTime) const {
    std::vector<AuditLogEntry> result;
    
    for (const auto& entry : auditLog_) {
        if ((startTime == 0 || entry.timestamp >= startTime) &&
            (endTime == 0 || entry.timestamp <= endTime)) {
            result.push_back(entry);
        }
    }
    
    return result;
}

// Hash password
String DataProtector::hashPassword(const String& password, const String& salt) {
    String saltToUse = salt.isEmpty() ? generateSalt() : salt;
    String combined = password + saltToUse;
    
    uint8_t hash[32];
    mbedtls_sha256((const uint8_t*)combined.c_str(), combined.length(), hash, 0);
    
    String hashStr = "";
    for (int i = 0; i < 32; i++) {
        char hex[3];
        sprintf(hex, "%02x", hash[i]);
        hashStr += hex;
    }
    
    return hashStr;
}

// Generate salt
String DataProtector::generateSalt() {
    return String(esp_random(), HEX) + String(esp_random(), HEX);
}

// Generate secure token
String DataProtector::generateSecureToken(uint32_t length) {
    String token = "";
    for (uint32_t i = 0; i < length; i++) {
        uint8_t randomByte = esp_random() % 62;
        if (randomByte < 10) {
            token += char('0' + randomByte);
        } else if (randomByte < 36) {
            token += char('A' + randomByte - 10);
        } else {
            token += char('a' + randomByte - 36);
        }
    }
    return token;
}

// Generate event ID
String DataProtector::generateEventId() {
    return "evt-" + String(millis()) + "-" + String(esp_random(), HEX);
}

// Add default permissions for role
void DataProtector::addDefaultPermissions(AccessRole role, std::vector<String>& permissions) {
    permissions.clear();
    
    switch (role) {
        case ROLE_VIEWER:
            permissions.push_back("read");
            break;
        case ROLE_OPERATOR:
            permissions.push_back("read");
            permissions.push_back("write");
            break;
        case ROLE_RESEARCHER:
            permissions.push_back("read");
            permissions.push_back("write");
            permissions.push_back("analyze");
            break;
        case ROLE_MANAGER:
            permissions.push_back("read");
            permissions.push_back("write");
            permissions.push_back("analyze");
            permissions.push_back("manage");
            break;
        case ROLE_ADMIN:
            permissions.push_back("read");
            permissions.push_back("write");
            permissions.push_back("analyze");
            permissions.push_back("manage");
            permissions.push_back("admin");
            break;
        case ROLE_SUPER_ADMIN:
            permissions.push_back("all");
            break;
    }
}

// Secure wipe
void DataProtector::secureWipe(uint8_t* buffer, size_t len) {
    if (buffer == nullptr || len == 0) {
        return;
    }
    
    for (int pass = 0; pass < 3; pass++) {
        memset(buffer, 0x00 + pass, len);
    }
    memset(buffer, 0x00, len);
}

// Get active session count
uint32_t DataProtector::getActiveSessionCount() const {
    uint32_t count = 0;
    uint32_t currentTime = millis() / 1000;
    
    for (const auto& pair : sessionExpiry_) {
        if (pair.second > currentTime) {
            count++;
        }
    }
    
    return count;
}

// Get security status
String DataProtector::getSecurityStatus() const {
    String status = "Security Status:\n";
    status += "- Initialized: " + String(initialized_ ? "Yes" : "No") + "\n";
    status += "- Security Level: " + String(config_.securityLevel) + "\n";
    status += "- Active Users: " + String(users_.size()) + "\n";
    status += "- Active Sessions: " + String(getActiveSessionCount()) + "\n";
    status += "- Encryption Keys: " + String(encryptionKeys_.size()) + "\n";
    status += "- Audit Log Entries: " + String(auditLog_.size()) + "\n";
    return status;
}

// Utility functions
bool initializeDataProtection(const SecurityConfig& config) {
    if (!g_dataProtector) {
        g_dataProtector = new DataProtector();
    }
    return g_dataProtector->init(config);
}

bool encryptSensitiveData(const String& data, String& encrypted) {
    if (!g_dataProtector) return false;
    EncryptionContext context;
    return g_dataProtector->encryptData(data, encrypted, context);
}

bool authenticateRequest(const String& sessionToken, String& userId) {
    if (!g_dataProtector) return false;
    return g_dataProtector->validateSession(sessionToken, userId);
}

bool checkUserPermission(const String& userId, const String& resource, const String& action) {
    if (!g_dataProtector) return true;
    return g_dataProtector->checkAccess(userId, resource, action);
}

void cleanupDataProtection() {
    if (g_dataProtector) {
        g_dataProtector->cleanup();
        delete g_dataProtector;
        g_dataProtector = nullptr;
    }
}

bool isSecurityEnabled() {
    return g_dataProtector && g_dataProtector->isSecurityEnabled();
}

SecurityLevel getCurrentSecurityLevel() {
    if (!g_dataProtector) return SECURITY_NONE;
    return g_dataProtector->getSecurityLevel();
}

bool hasActiveSession(const String& sessionToken) {
    if (!g_dataProtector) return false;
    String userId;
    return g_dataProtector->validateSession(sessionToken, userId);
}

String getSecuritySummary() {
    if (!g_dataProtector) return "Security not initialized";
    return g_dataProtector->getSecurityStatus();
}
