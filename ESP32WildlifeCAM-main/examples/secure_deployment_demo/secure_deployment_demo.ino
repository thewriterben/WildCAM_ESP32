/**
 * Secure Enterprise Deployment Demo
 * 
 * Complete example demonstrating all enterprise security features:
 * - AES-256 data encryption
 * - Role-based access control
 * - User authentication & session management
 * - Audit logging
 * - Secure OTA updates
 * - Privacy protection
 * - Key management
 * 
 * This example shows how to integrate all security components
 * in a production wildlife monitoring deployment.
 * 
 * Hardware: ESP32 or ESP32-CAM
 */

#include <Arduino.h>
#include <WiFi.h>
#include "../../firmware/security/security_manager.h"
#include "../../firmware/src/production/security/data_protector.h"
#include "../../firmware/src/production/deployment/ota_manager.h"

// WiFi credentials
const char* WIFI_SSID = "WildCAM_Network";
const char* WIFI_PASSWORD = "SecureWildlife2025";

// Security components
SecurityManager* securityMgr = nullptr;
DataProtector* dataProtector = nullptr;
OTAManager* otaManager = nullptr;

// Simulated wildlife detection data
struct WildlifeDetection {
    String species;
    float latitude;
    float longitude;
    uint32_t timestamp;
    String imageHash;
    float confidence;
};

// Function prototypes
void setupWiFi();
void initializeSecurity();
void initializeOTA();
void simulateWildlifeDetection();
void handleSecureDataTransmission(const WildlifeDetection& detection);
void demonstrateAccessControl();
void checkForSecureUpdates();
void printSystemStatus();

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n=============================================");
    Serial.println("  WildCAM Secure Enterprise Deployment Demo");
    Serial.println("=============================================\n");
    
    // Step 1: Initialize WiFi
    Serial.println("[Step 1] Initializing WiFi...");
    setupWiFi();
    
    // Step 2: Initialize Security System
    Serial.println("\n[Step 2] Initializing Security System...");
    initializeSecurity();
    
    // Step 3: Initialize OTA Manager
    Serial.println("\n[Step 3] Initializing Secure OTA...");
    initializeOTA();
    
    // Step 4: Demonstrate Access Control
    Serial.println("\n[Step 4] Setting up Access Control...");
    demonstrateAccessControl();
    
    // Step 5: Simulate Wildlife Detection with Security
    Serial.println("\n[Step 5] Simulating Secure Wildlife Detection...");
    simulateWildlifeDetection();
    
    // Step 6: Print System Status
    Serial.println("\n[Step 6] System Status Report");
    printSystemStatus();
    
    Serial.println("\n=============================================");
    Serial.println("  Secure Deployment Initialized Successfully");
    Serial.println("=============================================\n");
}

void loop() {
    // Continuous security monitoring
    static unsigned long lastCheck = 0;
    static unsigned long lastOTACheck = 0;
    unsigned long currentTime = millis();
    
    // Security health check every 30 seconds
    if (currentTime - lastCheck > 30000) {
        Serial.println("\n--- Security Health Check ---");
        
        // Check for tampering
        if (securityMgr && securityMgr->detectTampering()) {
            Serial.println("⚠️  ALERT: Tampering detected!");
            // Trigger security response
        } else {
            Serial.println("✓ System integrity verified");
        }
        
        // Check for security threats
        if (dataProtector && dataProtector->detectSecurityThreats()) {
            auto alerts = dataProtector->getSecurityAlerts();
            Serial.printf("⚠️  %d security alerts detected\n", alerts.size());
            for (const auto& alert : alerts) {
                Serial.println("  - " + alert);
            }
        }
        
        // Cleanup expired sessions
        if (dataProtector) {
            dataProtector->cleanupExpiredSessions();
        }
        
        lastCheck = currentTime;
    }
    
    // Check for OTA updates every 1 hour (3600000 ms)
    if (currentTime - lastOTACheck > 3600000) {
        checkForSecureUpdates();
        lastOTACheck = currentTime;
    }
    
    delay(1000);
}

void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi connected");
        Serial.print("  IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n✗ WiFi connection failed");
        Serial.println("  Continuing in offline mode...");
    }
}

void initializeSecurity() {
    // Initialize Security Manager for low-level crypto
    securityMgr = new SecurityManager(SecurityLevel::ENHANCED, true);
    
    if (!securityMgr->begin()) {
        Serial.println("✗ ERROR: Security Manager initialization failed!");
        return;
    }
    
    Serial.println("✓ Security Manager initialized");
    
    // Initialize Data Protector for enterprise features
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
    
    dataProtector = new DataProtector();
    
    if (!dataProtector->init(config)) {
        Serial.println("✗ ERROR: Data Protector initialization failed!");
        return;
    }
    
    Serial.println("✓ Data Protector initialized");
    Serial.println("✓ Enterprise security features active");
}

void initializeOTA() {
    OTAConfig otaConfig;
    otaConfig.updateServerURL = "https://updates.wildCAM.example.com";
    otaConfig.deviceID = WiFi.macAddress();
    otaConfig.networkID = "wildCAM-network-01";
    otaConfig.currentVersion = "3.0.0";
    otaConfig.checkInterval = 3600000; // 1 hour
    otaConfig.autoUpdate = false; // Require manual approval
    otaConfig.stagedDeployment = true;
    otaConfig.currentStage = STAGE_CANARY;
    otaConfig.minPriority = PRIORITY_NORMAL;
    otaConfig.rollbackTimeout = 300000; // 5 minutes
    otaConfig.signatureVerification = true; // CRITICAL: Verify firmware signatures
    
    otaManager = new OTAManager();
    
    if (!otaManager->init(otaConfig)) {
        Serial.println("✗ ERROR: OTA Manager initialization failed!");
        return;
    }
    
    Serial.println("✓ OTA Manager initialized");
    Serial.println("✓ Secure firmware updates enabled");
    Serial.println("  - Signature verification: ON");
    Serial.println("  - Staged deployment: ON");
    Serial.println("  - Auto-update: OFF (manual approval)");
}

void demonstrateAccessControl() {
    if (!dataProtector) return;
    
    // Create different user roles
    UserCredentials users[] = {
        {"field-tech-001", "field_tech", "FieldTech2025!", ROLE_OPERATOR},
        {"researcher-001", "dr_smith", "Research2025!", ROLE_RESEARCHER},
        {"manager-001", "site_manager", "Manager2025!", ROLE_MANAGER}
    };
    
    for (int i = 0; i < 3; i++) {
        UserCredentials user;
        user.userId = users[i].userId;
        user.username = users[i].username;
        user.passwordHash = dataProtector->hashPassword(users[i].passwordHash);
        user.role = users[i].role;
        user.isActive = true;
        user.createdTime = millis() / 1000;
        
        if (dataProtector->createUser(user)) {
            Serial.printf("✓ Created user: %s (Role: %d)\n", 
                         user.username.c_str(), user.role);
            
            // Grant appropriate access
            String resource;
            switch (user.role) {
                case ROLE_OPERATOR:
                    resource = "camera-controls";
                    break;
                case ROLE_RESEARCHER:
                    resource = "wildlife-data";
                    break;
                case ROLE_MANAGER:
                    resource = "system-config";
                    break;
                default:
                    resource = "public-data";
            }
            
            dataProtector->grantAccess(user.userId, resource, user.role);
            Serial.printf("  → Access granted to: %s\n", resource.c_str());
        }
    }
}

void simulateWildlifeDetection() {
    if (!dataProtector || !securityMgr) return;
    
    // Simulate wildlife detection
    WildlifeDetection detection;
    detection.species = "Panthera leo";
    detection.latitude = 37.7749;
    detection.longitude = -122.4194;
    detection.timestamp = millis() / 1000;
    detection.confidence = 0.92;
    
    // Generate image hash
    const char* imageData = "SIMULATED_IMAGE_DATA";
    uint8_t imageHash[32];
    securityMgr->generateHash((const uint8_t*)imageData, strlen(imageData), imageHash);
    
    detection.imageHash = "";
    for (int i = 0; i < 8; i++) { // First 8 bytes only
        char hex[3];
        sprintf(hex, "%02x", imageHash[i]);
        detection.imageHash += hex;
    }
    
    Serial.println("\n--- Wildlife Detection Event ---");
    Serial.println("Species: " + detection.species);
    Serial.printf("Location: %.4f, %.4f\n", detection.latitude, detection.longitude);
    Serial.printf("Confidence: %.2f%%\n", detection.confidence * 100);
    Serial.println("Image Hash: " + detection.imageHash);
    
    // Apply privacy protection to location
    float privLat = detection.latitude;
    float privLon = detection.longitude;
    
    if (dataProtector->fuzzLocation(privLat, privLon, 1.0)) { // 1km radius
        Serial.println("\n✓ Privacy protection applied");
        Serial.printf("  Original: %.6f, %.6f\n", detection.latitude, detection.longitude);
        Serial.printf("  Fuzzed:   %.6f, %.6f\n", privLat, privLon);
    }
    
    // Prepare data for transmission
    String detectionData = "species=" + detection.species;
    detectionData += ",lat=" + String(privLat, 6);
    detectionData += ",lon=" + String(privLon, 6);
    detectionData += ",time=" + String(detection.timestamp);
    detectionData += ",conf=" + String(detection.confidence, 2);
    detectionData += ",hash=" + detection.imageHash;
    
    // Encrypt data
    Serial.println("\n--- Securing Detection Data ---");
    String encrypted;
    EncryptionContext context;
    
    if (dataProtector->encryptData(detectionData, encrypted, context)) {
        Serial.println("✓ Data encrypted successfully");
        Serial.println("  Algorithm: " + context.algorithm);
        Serial.println("  Key ID: " + context.keyId);
        Serial.printf("  Size: %d → %d bytes\n", 
                     detectionData.length(), encrypted.length());
        
        // Log the event
        dataProtector->logAuditEvent("system", "WILDLIFE_DETECTION", 
                                    detection.species,
                                    "Detection recorded and encrypted", true);
    }
    
    Serial.println("\n✓ Wildlife detection securely processed");
}

void checkForSecureUpdates() {
    if (!otaManager) return;
    
    Serial.println("\n--- Checking for Secure Updates ---");
    
    if (otaManager->checkForUpdates()) {
        if (otaManager->hasAvailableUpdate()) {
            UpdatePackage update = otaManager->getAvailableUpdate();
            
            Serial.println("Update available:");
            Serial.println("  Version: " + update.version);
            Serial.println("  Description: " + update.description);
            Serial.printf("  Size: %d bytes\n", update.size);
            Serial.println("  Priority: " + String(update.priority));
            Serial.println("  Signature: " + update.signature.substring(0, 16) + "...");
            
            // Check if update should be applied
            if (otaManager->shouldUpdate()) {
                Serial.println("\n⚠️  Update recommended - Manual approval required");
                Serial.println("  Call otaManager->startUpdate() to begin installation");
                
                // Log the update check
                dataProtector->logAuditEvent("system", "UPDATE_AVAILABLE", 
                                           update.version,
                                           "Secure update pending approval", true);
            }
        } else {
            Serial.println("✓ System is up to date");
        }
    } else {
        Serial.println("✗ Update check failed");
    }
}

void printSystemStatus() {
    Serial.println("\n=============================================");
    Serial.println("  System Security Status");
    Serial.println("=============================================\n");
    
    // Security Manager Status
    if (securityMgr) {
        unsigned long total_ops, failed_ops, boot_verifications;
        securityMgr->getSecurityStats(&total_ops, &failed_ops, &boot_verifications);
        
        Serial.println("[Security Manager]");
        Serial.printf("  Operations: %lu (Failed: %lu)\n", total_ops, failed_ops);
        Serial.printf("  Boot Verifications: %lu\n", boot_verifications);
        Serial.printf("  Security Level: %s\n", 
                     securityMgr->getSecurityLevel() == SecurityLevel::ENHANCED ? "ENHANCED" : "BASIC");
        Serial.printf("  Operational: %s\n", 
                     securityMgr->isSecurityOperational() ? "YES" : "NO");
        Serial.printf("  Hardware Security: %s\n", 
                     securityMgr->isHardwareSecurityAvailable() ? "YES" : "NO");
    }
    
    // Data Protector Status
    if (dataProtector) {
        Serial.println("\n[Data Protector]");
        Serial.print(dataProtector->getSecurityStatus());
    }
    
    // OTA Manager Status
    if (otaManager) {
        Serial.println("\n[OTA Manager]");
        Serial.println("  Current Version: " + otaManager->getCurrentVersion());
        Serial.println("  Status: " + otaManager->getStatusMessage());
        Serial.printf("  Initialized: %s\n", otaManager->isInitialized() ? "YES" : "NO");
    }
    
    // Network Status
    Serial.println("\n[Network]");
    Serial.printf("  WiFi: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("  IP: " + WiFi.localIP().toString());
        Serial.printf("  RSSI: %d dBm\n", WiFi.RSSI());
    }
    
    // Memory Status
    Serial.println("\n[Memory]");
    Serial.printf("  Free Heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("  Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
    Serial.printf("  Heap Size: %d bytes\n", ESP.getHeapSize());
    
    Serial.println("\n=============================================\n");
}
