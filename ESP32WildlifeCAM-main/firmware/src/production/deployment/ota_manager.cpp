/**
 * OTA Manager Implementation
 * 
 * Secure over-the-air update system for wildlife camera networks
 * Enhanced with AsyncElegantOTA for web-based updates
 */

#include "ota_manager.h"
#include "../../config.h"
#include "../../debug_utils.h"
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

// AsyncElegantOTA integration for Phase 1 enhancement
#ifdef OTA_ENABLED
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#endif

// Global instance
OTAManager* g_otaManager = nullptr;

// Constructor
OTAManager::OTAManager() 
    : status_(OTA_IDLE), initialized_(false), hasAvailableUpdate_(false),
      progress_(0.0), updateStartTime_(0), rollbackTimeout_(0),
      progressCallback_(nullptr), statusCallback_(nullptr), errorCallback_(nullptr)
#ifdef OTA_ENABLED
    , webOTAServer_(nullptr), webOTARunning_(false), webOTAAuthEnabled_(true),
      webOTAUsername_("admin"), webOTAPassword_("wildlife"), webOTAPath_("/update"), webOTAPort_(80)
#endif
{
}

// Destructor
OTAManager::~OTAManager() {
    cleanup();
}

// Initialize OTA manager
bool OTAManager::init(const OTAConfig& config) {
    if (initialized_) return true;
    
    DEBUG_PRINTLN("Initializing OTA Manager...");
    
    config_ = config;
    status_ = OTA_IDLE;
    progress_ = 0.0;
    hasAvailableUpdate_ = false;
    statusMessage_ = "OTA Manager initialized";
    
    // Configure HTTP update client
    httpUpdate_.setLedPin(-1);  // Disable LED feedback
    httpUpdate_.rebootOnUpdate(false);  // Manual reboot control
    
    // Set up security if enabled
    if (config_.signatureVerification) {
        if (!initializeCrypto()) {
            DEBUG_PRINTLN("ERROR: Failed to initialize crypto for OTA");
            return false;
        }
    }
    
    // Load health metrics from storage
    // TODO: Implement persistent storage for metrics
    
    initialized_ = true;
    DEBUG_PRINTLN("OTA Manager initialized successfully");
    return true;
}

// Cleanup resources
void OTAManager::cleanup() {
    if (!initialized_) return;
    
    if (isUpdateInProgress()) {
        cancelUpdate();
    }
    
    initialized_ = false;
    status_ = OTA_IDLE;
    DEBUG_PRINTLN("OTA Manager cleaned up");
}

// Check for available updates
bool OTAManager::checkForUpdates() {
    if (!initialized_) return false;
    
    updateStatus(OTA_CHECKING, "Checking for updates...");
    
    // Construct update check URL
    String checkURL = config_.updateServerURL + "/check";
    checkURL += "?device=" + config_.deviceID;
    checkURL += "&network=" + config_.networkID;
    checkURL += "&version=" + config_.currentVersion;
    checkURL += "&stage=" + String((int)config_.currentStage);
    
    DEBUG_PRINTLN("Checking for updates: " + checkURL);
    
    // Make HTTP request to check for updates
    WiFiClient client;
    if (client.connect(config_.updateServerURL.c_str(), 80)) {
        client.print("GET " + checkURL + " HTTP/1.1\r\n");
        client.print("Host: " + config_.updateServerURL + "\r\n");
        client.print("Connection: close\r\n\r\n");
        
        // Read response
        String response = "";
        while (client.connected() || client.available()) {
            if (client.available()) {
                response += client.readString();
            }
        }
        client.stop();
        
        // Parse response for update information
        if (response.indexOf("200 OK") != -1) {
            // Extract JSON payload
            int jsonStart = response.indexOf("\r\n\r\n");
            if (jsonStart != -1) {
                String jsonPayload = response.substring(jsonStart + 4);
                
                // Parse update package information
                // TODO: Implement proper JSON parsing
                if (jsonPayload.indexOf("\"available\":true") != -1) {
                    hasAvailableUpdate_ = true;
                    updateStatus(OTA_IDLE, "Update available");
                    return true;
                }
            }
        }
    }
    
    hasAvailableUpdate_ = false;
    updateStatus(OTA_IDLE, "No updates available");
    return false;
}

// Determine if update should be applied
bool OTAManager::shouldUpdate() const {
    if (!hasAvailableUpdate_) return false;
    if (!config_.autoUpdate) return false;
    
    // Check priority threshold
    if (availableUpdate_.priority > config_.minPriority) return false;
    
    // Check deployment stage eligibility
    if (!isEligibleForStage(availableUpdate_.targetStage)) return false;
    
    return true;
}

// Start update process
bool OTAManager::startUpdate(bool forceUpdate) {
    if (!initialized_ || !hasAvailableUpdate_) return false;
    
    if (!forceUpdate && !shouldUpdate()) {
        DEBUG_PRINTLN("Update conditions not met");
        return false;
    }
    
    updateStatus(OTA_DOWNLOADING, "Starting update download...");
    updateStartTime_ = millis();
    
    // Download and install update
    if (!downloadUpdate(availableUpdate_)) {
        updateStatus(OTA_FAILED, "Update download failed");
        updateHealthMetrics(false);
        return false;
    }
    
    updateStatus(OTA_COMPLETE, "Update completed successfully");
    updateHealthMetrics(true);
    
    // Set rollback timeout
    rollbackTimeout_ = millis() + config_.rollbackTimeout;
    
    return true;
}

// Download and install update
bool OTAManager::downloadUpdate(const UpdatePackage& package) {
    DEBUG_PRINTLN("Downloading update: " + package.version);
    
    updateProgress(0.0, "Connecting to update server...");
    
    // Configure HTTP update
    t_httpUpdate_return result = httpUpdate_.update(package.downloadURL);
    
    switch (result) {
        case HTTP_UPDATE_FAILED:
            reportError(httpUpdate_.getLastError(), httpUpdate_.getLastErrorString());
            return false;
            
        case HTTP_UPDATE_NO_UPDATES:
            updateStatus(OTA_IDLE, "No update needed");
            return false;
            
        case HTTP_UPDATE_OK:
            updateProgress(100.0, "Update installed successfully");
            return true;
            
        default:
            reportError(-1, "Unknown update error");
            return false;
    }
}

// Verify signature
bool OTAManager::verifySignature(const uint8_t* data, size_t length, const String& signature) {
    if (!config_.signatureVerification) return true;
    
    DEBUG_PRINTLN("Verifying firmware signature...");
    
    // Calculate hash of data
    uint8_t hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, data, length);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    
    // Convert signature from hex string to bytes
    if (signature.length() != 64) {
        DEBUG_PRINTLN("Invalid signature length");
        return false;
    }
    
    uint8_t signatureBytes[32];
    for (int i = 0; i < 32; i++) {
        String byteStr = signature.substring(i * 2, i * 2 + 2);
        signatureBytes[i] = (uint8_t)strtol(byteStr.c_str(), nullptr, 16);
    }
    
    // Compare hash with signature (simple verification)
    // In production, use RSA/ECDSA signature verification
    bool valid = (memcmp(hash, signatureBytes, 32) == 0);
    
    if (valid) {
        DEBUG_PRINTLN("✓ Firmware signature verified successfully");
    } else {
        DEBUG_PRINTLN("✗ Firmware signature verification FAILED");
        
        // Log the mismatch for debugging
        DEBUG_PRINT("Expected: ");
        for (int i = 0; i < 32; i++) {
            Serial.printf("%02x", signatureBytes[i]);
        }
        DEBUG_PRINTLN("");
        
        DEBUG_PRINT("Calculated: ");
        for (int i = 0; i < 32; i++) {
            Serial.printf("%02x", hash[i]);
        }
        DEBUG_PRINTLN("");
    }
    
    return valid;
}

// Verify checksum
bool OTAManager::verifyChecksum(const uint8_t* data, size_t length, const String& checksum) {
    String calculated = calculateChecksum(data, length);
    bool valid = calculated.equals(checksum);
    
    DEBUG_PRINTLN("Checksum verification: " + String(valid ? "PASS" : "FAIL"));
    DEBUG_PRINTLN("Expected: " + checksum);
    DEBUG_PRINTLN("Calculated: " + calculated);
    
    return valid;
}

// Calculate SHA256 checksum
String OTAManager::calculateChecksum(const uint8_t* data, size_t length) {
    mbedtls_sha256_context ctx;
    uint8_t hash[32];
    
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);  // SHA256, not SHA224
    mbedtls_sha256_update(&ctx, data, length);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    
    // Convert to hex string
    String result = "";
    for (int i = 0; i < 32; i++) {
        if (hash[i] < 16) result += "0";
        result += String(hash[i], HEX);
    }
    
    return result;
}

// Check if eligible for deployment stage
bool OTAManager::isEligibleForStage(DeploymentStage stage) const {
    // Simple eligibility based on device ID hash
    uint32_t deviceHash = 0;
    for (char c : config_.deviceID) {
        deviceHash = deviceHash * 31 + c;
    }
    
    uint8_t percentage = deviceHash % 100;
    
    switch (stage) {
        case STAGE_CANARY:
            return percentage < 5;   // First 5% of devices
        case STAGE_PILOT:
            return percentage < 25;  // First 25% of devices
        case STAGE_PRODUCTION:
            return true;             // All devices
        default:
            return false;
    }
}

// Get current firmware version
String OTAManager::getCurrentVersion() const {
    const esp_app_desc_t* app_desc = esp_ota_get_app_description();
    return String(app_desc->version);
}

// Initialize cryptographic components
bool OTAManager::initializeCrypto() {
    DEBUG_PRINTLN("Initializing OTA cryptographic components...");
    
    // Initialize mbedTLS contexts for signature verification
    mbedtls_sha256_context shaCtx;
    mbedtls_sha256_init(&shaCtx);
    
    // Test crypto functionality
    const char* testData = "OTA_CRYPTO_TEST";
    uint8_t testHash[32];
    
    int ret = mbedtls_sha256((const uint8_t*)testData, strlen(testData), testHash, 0);
    
    mbedtls_sha256_free(&shaCtx);
    
    if (ret != 0) {
        DEBUG_PRINTLN("ERROR: Crypto initialization failed");
        return false;
    }
    
    DEBUG_PRINTLN("✓ OTA cryptographic components initialized");
    return true;
}

// Update progress with callback
void OTAManager::updateProgress(float progress, const String& message) {
    progress_ = progress;
    statusMessage_ = message;
    
    if (progressCallback_) {
        progressCallback_(progress, message);
    }
    
    DEBUG_PRINTLN("OTA Progress: " + String(progress) + "% - " + message);
}

// Update status with callback
void OTAManager::updateStatus(OTAStatus status, const String& message) {
    status_ = status;
    statusMessage_ = message;
    
    if (statusCallback_) {
        statusCallback_(status, message);
    }
    
    DEBUG_PRINTLN("OTA Status: " + String(status) + " - " + message);
}

// Report error with callback
void OTAManager::reportError(int errorCode, const String& errorMessage) {
    updateStatus(OTA_FAILED, errorMessage);
    
    if (errorCallback_) {
        errorCallback_(errorCode, errorMessage);
    }
    
    DEBUG_PRINTLN("OTA Error " + String(errorCode) + ": " + errorMessage);
}

// Update health metrics
void OTAManager::updateHealthMetrics(bool successful) {
    if (successful) {
        healthMetrics_.successfulUpdates++;
    } else {
        healthMetrics_.failedUpdates++;
    }
    
    uint32_t totalUpdates = healthMetrics_.successfulUpdates + healthMetrics_.failedUpdates;
    if (totalUpdates > 0) {
        healthMetrics_.successRate = (float)healthMetrics_.successfulUpdates / totalUpdates * 100.0;
    }
    
    healthMetrics_.lastUpdateTimestamp = millis();
    healthMetrics_.systemHealthy = healthMetrics_.successRate >= 80.0;  // 80% success threshold
}

// Global utility functions
bool initializeOTA(const OTAConfig& config) {
    if (!g_otaManager) {
        g_otaManager = new OTAManager();
    }
    return g_otaManager->init(config);
}

bool checkAndUpdateFirmware() {
    if (!g_otaManager) return false;
    
    if (g_otaManager->checkForUpdates() && g_otaManager->shouldUpdate()) {
        return g_otaManager->startUpdate();
    }
    return false;
}

#ifdef OTA_ENABLED
// AsyncElegantOTA Web Interface Implementation - Phase 1 Enhancement

bool OTAManager::initWebOTA(AsyncWebServer* server) {
    if (!server) {
        DEBUG_PRINTLN("ERROR: Web server is null");
        return false;
    }
    
    webOTAServer_ = server;
    
    // Initialize AsyncElegantOTA with the provided server
    AsyncElegantOTA.begin(webOTAServer_, webOTAUsername_.c_str(), webOTAPassword_.c_str(), webOTAPath_.c_str());
    
    // Set up callbacks
    AsyncElegantOTA.onStart([this]() {
        DEBUG_PRINTLN("Web OTA update started");
        updateStatus(OTA_DOWNLOADING, "Web OTA update started");
        if (webOTAStartCallback_) webOTAStartCallback_();
    });
    
    AsyncElegantOTA.onProgress([this](size_t current, size_t total) {
        float progressPercent = (float)current / total * 100.0;
        progress_ = progressPercent / 100.0;
        String message = "Web OTA Progress: " + String((int)progressPercent) + "%";
        updateProgress(progress_, message);
        if (webOTAProgressCallback_) webOTAProgressCallback_(current, total);
    });
    
    AsyncElegantOTA.onEnd([this](bool success) {
        if (success) {
            DEBUG_PRINTLN("Web OTA update completed successfully");
            updateStatus(OTA_COMPLETE, "Web OTA update completed successfully");
        } else {
            DEBUG_PRINTLN("Web OTA update failed");
            updateStatus(OTA_FAILED, "Web OTA update failed");
        }
        if (webOTAEndCallback_) webOTAEndCallback_(success);
    });
    
    webOTARunning_ = true;
    DEBUG_PRINTF("AsyncElegantOTA initialized on path: %s\n", webOTAPath_.c_str());
    return true;
}

bool OTAManager::startWebOTA(uint16_t port, const String& username, const String& password) {
    if (webOTARunning_) {
        DEBUG_PRINTLN("Web OTA already running");
        return true;
    }
    
    webOTAPort_ = port;
    webOTAUsername_ = username;
    webOTAPassword_ = password;
    
    // Create a new web server if one wasn't provided
    if (!webOTAServer_) {
        webOTAServer_ = new AsyncWebServer(port);
    }
    
    // Initialize web OTA
    if (!initWebOTA(webOTAServer_)) {
        DEBUG_PRINTLN("ERROR: Failed to initialize web OTA");
        return false;
    }
    
    // Add a root page with OTA info
    webOTAServer_->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html><html><head><title>ESP32 Wildlife Camera - OTA Update</title></head>";
        html += "<body><h1>ESP32 Wildlife Camera</h1>";
        html += "<h2>Over-The-Air Update System</h2>";
        html += "<p>Current Version: " + getCurrentVersion() + "</p>";
        html += "<p>Status: " + getStatusMessage() + "</p>";
        html += "<p>Free Space: " + String(getFreeSpace()) + " bytes</p>";
        html += "<br><a href='" + webOTAPath_ + "'>Go to Update Page</a>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });
    
    // Start the server
    webOTAServer_->begin();
    webOTARunning_ = true;
    
    DEBUG_PRINTF("Web OTA started on port %d\n", port);
    DEBUG_PRINTF("OTA URL: http://%s:%d%s\n", WiFi.localIP().toString().c_str(), port, webOTAPath_.c_str());
    
    return true;
}

void OTAManager::stopWebOTA() {
    if (!webOTARunning_) return;
    
    AsyncElegantOTA.end();
    
    if (webOTAServer_) {
        webOTAServer_->end();
        delete webOTAServer_;
        webOTAServer_ = nullptr;
    }
    
    webOTARunning_ = false;
    DEBUG_PRINTLN("Web OTA stopped");
}

String OTAManager::getWebOTAURL() const {
    if (!webOTARunning_) return "";
    return "http://" + WiFi.localIP().toString() + ":" + String(webOTAPort_) + webOTAPath_;
}

void OTAManager::setWebOTACredentials(const String& username, const String& password) {
    webOTAUsername_ = username;
    webOTAPassword_ = password;
    DEBUG_PRINTF("Web OTA credentials updated: %s\n", username.c_str());
}

void OTAManager::setWebOTAPath(const String& path) {
    webOTAPath_ = path;
    DEBUG_PRINTF("Web OTA path set to: %s\n", path.c_str());
}

#endif // OTA_ENABLED

OTAStatus getOTAStatus() {
    return g_otaManager ? g_otaManager->getStatus() : OTA_IDLE;
}

float getOTAProgress() {
    return g_otaManager ? g_otaManager->getProgress() : 0.0;
}

String getOTAMessage() {
    return g_otaManager ? g_otaManager->getStatusMessage() : "OTA not initialized";
}

void cleanupOTA() {
    if (g_otaManager) {
        delete g_otaManager;
        g_otaManager = nullptr;
    }
}