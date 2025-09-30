# Enterprise Security Examples

This directory contains comprehensive examples demonstrating the enterprise security features of WildCAM ESP32 v3.0.

## Available Examples

### 1. Enterprise Security Demo (`enterprise_security_demo/`)

**Purpose**: Demonstrates all core security features in isolation.

**Features Demonstrated**:
- AES-256 encryption and decryption
- Secure key generation and management
- Role-based access control (RBAC)
- User authentication and session management
- Audit logging
- Security status monitoring

**Use Case**: Learning and testing individual security components.

**Hardware Requirements**: ESP32 or ESP32-CAM

**How to Run**:
```bash
# Using PlatformIO
cd enterprise_security_demo
pio run -t upload -t monitor

# Using Arduino IDE
# Open enterprise_security_demo.ino
# Select Board: ESP32 Dev Module
# Upload and open Serial Monitor (115200 baud)
```

**Expected Output**:
- Successful initialization of security components
- Encryption/decryption demonstrations
- User creation and authentication tests
- Access control verification
- Audit log entries
- Security status report

---

### 2. Secure Deployment Demo (`secure_deployment_demo/`)

**Purpose**: Complete production deployment example integrating all security features.

**Features Demonstrated**:
- WiFi connectivity setup
- Full security system initialization
- Multi-user role-based access control
- Wildlife detection with privacy protection
- Secure data encryption before transmission
- Automatic security monitoring
- Secure OTA update checking
- Comprehensive system status reporting

**Use Case**: Production deployment reference implementation.

**Hardware Requirements**: ESP32 or ESP32-CAM with WiFi

**Configuration**:
```cpp
// Edit these in the sketch:
const char* WIFI_SSID = "Your_Network_SSID";
const char* WIFI_PASSWORD = "Your_Network_Password";
```

**How to Run**:
```bash
# Using PlatformIO
cd secure_deployment_demo
pio run -t upload -t monitor

# Using Arduino IDE
# Open secure_deployment_demo.ino
# Configure WiFi credentials
# Select Board: ESP32 Dev Module
# Upload and open Serial Monitor (115200 baud)
```

**Expected Output**:
- WiFi connection status
- Security system initialization
- User account creation (3 users with different roles)
- Simulated wildlife detection event
- Privacy protection application
- Data encryption
- Security health checks (every 30 seconds)
- OTA update checks (every hour)
- Detailed system status reports

---

## Security Components Overview

### Security Manager
- **Location**: `firmware/security/security_manager.h/cpp`
- **Purpose**: Low-level cryptographic operations
- **Features**: AES-256, SHA-256, secure random generation, tamper detection

### Data Protector
- **Location**: `ESP32WildlifeCAM-main/firmware/src/production/security/data_protector.h/cpp`
- **Purpose**: Enterprise security management
- **Features**: RBAC, authentication, audit logging, privacy protection

### OTA Manager
- **Location**: `ESP32WildlifeCAM-main/firmware/src/production/deployment/ota_manager.h/cpp`
- **Purpose**: Secure firmware updates
- **Features**: Signature verification, staged deployment, automatic rollback

---

## User Roles and Permissions

### Role Hierarchy

1. **VIEWER** (Level 1)
   - Permissions: `read`
   - Use Case: Public viewing of non-sensitive data

2. **OPERATOR** (Level 2)
   - Permissions: `read`, `write`
   - Use Case: Field technicians operating cameras

3. **RESEARCHER** (Level 3)
   - Permissions: `read`, `write`, `analyze`
   - Use Case: Scientists analyzing wildlife data

4. **MANAGER** (Level 4)
   - Permissions: `read`, `write`, `analyze`, `manage`
   - Use Case: Site managers configuring deployments

5. **ADMIN** (Level 5)
   - Permissions: `read`, `write`, `analyze`, `manage`, `admin`
   - Use Case: System administrators

6. **SUPER_ADMIN** (Level 6)
   - Permissions: `all`
   - Use Case: Full system access for critical operations

---

## Security Configuration

### Basic Configuration
```cpp
SecurityConfig config;
config.securityLevel = SECURITY_STANDARD;
config.encryptionType = ENCRYPT_AES_256;
config.privacyMode = PRIVACY_BASIC;
config.enableAuditLogging = true;
config.requireAuthentication = true;
```

### High Security Configuration
```cpp
SecurityConfig config;
config.securityLevel = SECURITY_HIGH;
config.encryptionType = ENCRYPT_AES_256;
config.privacyMode = PRIVACY_ADVANCED;
config.enableAuditLogging = true;
config.requireAuthentication = true;
config.enableAccessControl = true;
config.encryptAtRest = true;
config.encryptInTransit = true;
config.keyRotationInterval = 168; // 7 days
config.sessionTimeout = 30; // 30 minutes
config.enableTwoFactor = false;
```

### Maximum Security Configuration
```cpp
SecurityConfig config;
config.securityLevel = SECURITY_MAXIMUM;
config.encryptionType = ENCRYPT_AES_256;
config.privacyMode = PRIVACY_ANONYMOUS;
config.enableAuditLogging = true;
config.requireAuthentication = true;
config.enableAccessControl = true;
config.encryptAtRest = true;
config.encryptInTransit = true;
config.keyRotationInterval = 24; // Daily
config.sessionTimeout = 15; // 15 minutes
config.enableTwoFactor = true;
```

---

## Common Operations

### User Authentication
```cpp
String sessionToken;
if (dataProtector->authenticateUser("username", "password", sessionToken)) {
    // Authentication successful
    String userId;
    if (dataProtector->validateSession(sessionToken, userId)) {
        // Session is valid
    }
}
```

### Access Control
```cpp
// Grant access
dataProtector->grantAccess("user-001", "resource-name", ROLE_RESEARCHER);

// Check access
if (dataProtector->checkAccess("user-001", "resource-name", "read")) {
    // User has permission
}
```

### Data Encryption
```cpp
String sensitive = "Wildlife location data";
String encrypted;
EncryptionContext context;

if (dataProtector->encryptData(sensitive, encrypted, context)) {
    // Data encrypted - safe to transmit
    
    // Later, decrypt:
    String decrypted;
    dataProtector->decryptData(encrypted, context, decrypted);
}
```

### Privacy Protection
```cpp
float lat = 37.7749;
float lon = -122.4194;

// Fuzz location within 1km radius
dataProtector->fuzzLocation(lat, lon, 1.0);

// Remove personal identifiers
String data = "User: John, Location: Park";
dataProtector->removePersonalIdentifiers(data);
```

### Audit Logging
```cpp
// Log event
dataProtector->logAuditEvent("user-001", "DATA_ACCESS", 
                             "camera-01", "Viewed feed", true);

// Retrieve logs
auto logs = dataProtector->getAuditLog();
for (const auto& entry : logs) {
    Serial.println(entry.action);
}
```

---

## Troubleshooting

### Issue: "Security Manager initialization failed"
**Solution**: Check that mbedTLS libraries are properly linked. Ensure sufficient heap memory (>50KB free).

### Issue: "Encryption operations failing"
**Solution**: 
1. Check security manager operational status
2. Verify keys are initialized
3. Monitor failed operation counter

### Issue: "Authentication always failing"
**Solution**:
1. Verify password hash matches stored hash
2. Check if user account is active
3. Verify session timeout hasn't expired

### Issue: "Out of memory errors"
**Solution**:
1. Reduce audit log retention
2. Cleanup expired sessions regularly
3. Limit number of concurrent sessions

### Issue: "OTA updates not working"
**Solution**:
1. Verify WiFi connectivity
2. Check update server URL
3. Ensure signature verification is properly configured
4. Verify sufficient flash space for new firmware

---

## Performance Notes

### Encryption Performance
- AES-256 encryption: ~50-100ms per 1KB
- Typical overhead: 5-10% for encrypted data transmission
- Minimal impact on wildlife detection performance

### Memory Usage
- Security Manager: ~10KB RAM
- Data Protector: ~20-30KB RAM (scales with users)
- Total overhead: ~30-40KB RAM

### Storage Usage
- Audit logs: ~100 bytes per entry
- User database: ~500 bytes per user
- Session tokens: ~64 bytes per session

---

## Security Best Practices

1. **Change Default Passwords**: Update the default admin password immediately
2. **Enable All Features**: Use maximum security in production
3. **Regular Key Rotation**: Rotate encryption keys every 90 days
4. **Monitor Audit Logs**: Review logs daily for suspicious activity
5. **Secure WiFi**: Use WPA3 if available, WPA2 minimum
6. **Firmware Updates**: Keep firmware up to date with security patches
7. **Session Management**: Use short session timeouts (15-30 minutes)
8. **Network Isolation**: Deploy on isolated network segments when possible
9. **Backup Keys**: Securely backup encryption keys
10. **Physical Security**: Protect hardware from physical tampering

---

## Additional Resources

- **Full Documentation**: See `/ENTERPRISE_SECURITY.md`
- **Production Deployment**: See `/PRODUCTION_DEPLOYMENT.md`
- **API Reference**: See security headers in `/firmware/security/`
- **Architecture**: See `/docs/architecture/security.md`

---

## Support

For security-related questions or to report vulnerabilities:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Email: security@wildCAM.example.com

**Note**: Please report security vulnerabilities privately via email, not through public GitHub issues.
