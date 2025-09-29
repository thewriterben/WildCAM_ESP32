/**
 * Cloud Integrator - Enterprise Cloud Platform Integration
 * 
 * Provides seamless integration with cloud platforms for enterprise-scale
 * wildlife monitoring deployments. Supports real-time synchronization,
 * analytics, and multi-site management.
 * 
 * Features:
 * - Multi-cloud platform support (AWS, Azure, GCP)
 * - Real-time data streaming and synchronization
 * - Edge computing with cloud backup
 * - Scalable API endpoints
 * - Enterprise authentication and security
 */

#ifndef CLOUD_INTEGRATOR_H
#define CLOUD_INTEGRATOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <map>
#include <queue>

// Cloud Platform Types
enum CloudPlatform {
    CLOUD_AWS,              // Amazon Web Services
    CLOUD_AZURE,            // Microsoft Azure
    CLOUD_GCP,              // Google Cloud Platform
    CLOUD_CUSTOM,           // Custom cloud implementation
    CLOUD_HYBRID            // Multi-cloud deployment
};

// Data Synchronization Modes
enum SyncMode {
    SYNC_REAL_TIME,         // Immediate synchronization
    SYNC_BATCH,             // Batch synchronization
    SYNC_OFFLINE_FIRST,     // Store locally, sync when connected
    SYNC_SELECTIVE,         // Selective synchronization based on criteria
    SYNC_BACKUP_ONLY        // Backup synchronization only
};

// Cloud Service Types
enum ServiceType {
    SERVICE_STORAGE,        // File and data storage
    SERVICE_DATABASE,       // Database services
    SERVICE_ANALYTICS,      // Analytics and ML services
    SERVICE_MESSAGING,      // Real-time messaging
    SERVICE_COMPUTE,        // Computing and processing
    SERVICE_IOT,            // IoT device management
    SERVICE_AI,             // AI/ML model services
    SERVICE_MONITORING      // System monitoring and logging
};

// Data Types for Cloud Storage
enum DataType {
    DATA_IMAGE,             // Image files
    DATA_VIDEO,             // Video files
    DATA_TELEMETRY,         // Sensor and system telemetry
    DATA_DETECTION,         // Wildlife detection events
    DATA_CONFIGURATION,     // Configuration data
    DATA_LOGS,              // System logs
    DATA_ANALYTICS,         // Processed analytics data
    DATA_REPORTS            // Generated reports
};

// Cloud Configuration Structure
struct CloudConfig {
    CloudPlatform platform;
    String endpoint;                // Cloud service endpoint
    String region;                  // Cloud region
    String accessKey;               // Access credentials
    String secretKey;               // Secret credentials
    String projectId;               // Project/subscription ID
    String bucketName;              // Storage bucket name
    String databaseName;            // Database name
    
    // Synchronization settings
    SyncMode syncMode;
    uint32_t syncInterval;          // Sync interval in seconds
    uint32_t batchSize;             // Batch upload size
    bool autoRetry;                 // Auto-retry failed uploads
    uint32_t maxRetries;            // Maximum retry attempts
    
    // Security settings
    bool useSSL;                    // Use SSL/TLS encryption
    bool authenticateRequests;      // Authenticate all requests
    String certificatePath;         // Certificate file path
    
    // Data management
    bool compressData;              // Compress data before upload
    bool encryptData;               // Encrypt sensitive data
    uint32_t dataRetentionDays;     // Cloud data retention period
    
    CloudConfig() : 
        platform(CLOUD_CUSTOM), endpoint(""), region(""), accessKey(""),
        secretKey(""), projectId(""), bucketName(""), databaseName(""),
        syncMode(SYNC_OFFLINE_FIRST), syncInterval(300), batchSize(10),
        autoRetry(true), maxRetries(3), useSSL(true), authenticateRequests(true),
        certificatePath(""), compressData(true), encryptData(true),
        dataRetentionDays(365) {}
};

// Upload Request Structure
struct UploadRequest {
    String requestId;               // Unique request identifier
    DataType dataType;              // Type of data being uploaded
    String localFilePath;           // Local file path
    String cloudPath;               // Cloud storage path
    String metadata;                // JSON metadata
    uint32_t priority;              // Upload priority (0 = highest)
    uint32_t timestamp;             // Request timestamp
    uint32_t retryCount;            // Number of retry attempts
    bool urgent;                    // Urgent upload flag
    
    UploadRequest() : 
        requestId(""), dataType(DATA_TELEMETRY), localFilePath(""),
        cloudPath(""), metadata(""), priority(5), timestamp(0),
        retryCount(0), urgent(false) {}
};

// Cloud Response Structure
struct CloudResponse {
    bool success;                   // Operation success status
    int httpCode;                   // HTTP response code
    String responseData;            // Response data/body
    String errorMessage;            // Error message if failed
    uint32_t responseTime;          // Response time in milliseconds
    String cloudUrl;                // Final cloud URL (for uploads)
    
    CloudResponse() : 
        success(false), httpCode(0), responseData(""), errorMessage(""),
        responseTime(0), cloudUrl("") {}
};

// Synchronization Statistics
struct SyncStats {
    uint32_t totalUploads;          // Total upload attempts
    uint32_t successfulUploads;     // Successful uploads
    uint32_t failedUploads;         // Failed uploads
    uint32_t retryUploads;          // Uploads requiring retry
    uint64_t totalDataUploaded;     // Total bytes uploaded
    uint32_t averageResponseTime;   // Average response time
    float successRate;              // Success rate percentage
    uint32_t lastSyncTime;          // Last successful sync
    String lastError;               // Last error message
    
    SyncStats() : 
        totalUploads(0), successfulUploads(0), failedUploads(0),
        retryUploads(0), totalDataUploaded(0), averageResponseTime(0),
        successRate(100.0), lastSyncTime(0), lastError("") {}
};

// Real-time Event Structure
struct RealTimeEvent {
    String eventType;               // Event type identifier
    String deviceId;                // Source device ID
    String siteId;                  // Deployment site ID
    uint32_t timestamp;             // Event timestamp
    String payload;                 // JSON event payload
    uint32_t priority;              // Event priority
    bool critical;                  // Critical event flag
    
    RealTimeEvent() : 
        eventType(""), deviceId(""), siteId(""), timestamp(0),
        payload(""), priority(5), critical(false) {}
};

/**
 * Cloud Integrator Class
 * 
 * Provides comprehensive cloud integration for wildlife monitoring systems
 */
class CloudIntegrator {
public:
    CloudIntegrator();
    ~CloudIntegrator();
    
    // Initialization and configuration
    bool init(const CloudConfig& config);
    void cleanup();
    bool configure(const CloudConfig& config);
    CloudConfig getConfiguration() const { return config_; }
    
    // Connection management
    bool connect();
    bool disconnect();
    bool isConnected() const { return connected_; }
    bool testConnection();
    bool authenticateConnection();
    
    // File upload and storage
    CloudResponse uploadFile(const String& localPath, const String& cloudPath, DataType dataType);
    bool uploadFileAsync(const UploadRequest& request);
    CloudResponse downloadFile(const String& cloudPath, const String& localPath);
    bool deleteCloudFile(const String& cloudPath);
    std::vector<String> listCloudFiles(const String& directory = "");
    
    // Data synchronization
    bool synchronizeData();
    bool syncDetectionEvents();
    bool syncTelemetryData();
    bool syncConfigurationData();
    bool forceSyncAll();
    
    // Real-time streaming
    bool startRealTimeStream();
    bool stopRealTimeStream();
    bool sendRealTimeEvent(const RealTimeEvent& event);
    bool subscribeToEvents(const String& eventType);
    
    // Database operations
    bool insertRecord(const String& table, const String& jsonData);
    bool updateRecord(const String& table, const String& recordId, const String& jsonData);
    bool deleteRecord(const String& table, const String& recordId);
    String queryRecords(const String& query);
    
    // Analytics and reporting
    bool uploadAnalyticsData(const String& analyticsJson);
    bool requestReport(const String& reportType, const String& parameters);
    String getProcessedAnalytics(const String& dateRange);
    bool triggerCloudAnalysis();
    
    // Device management
    bool registerDevice();
    bool updateDeviceStatus(const String& status);
    bool requestConfiguration();
    bool reportDeviceHealth(const String& healthData);
    
    // Multi-site coordination
    bool joinMultiSiteNetwork(const String& networkId);
    bool shareDataWithSites(const std::vector<String>& siteIds);
    bool requestSiteData(const String& siteId, const String& dataType);
    std::vector<String> getConnectedSites();
    
    // Backup and recovery
    bool createBackup(const String& backupName);
    bool restoreFromBackup(const String& backupName);
    std::vector<String> listBackups();
    bool scheduleAutomaticBackup(uint32_t intervalHours);
    
    // Cloud AI integration
    bool uploadModelForTraining(const String& modelData);
    bool downloadUpdatedModel(const String& modelId);
    bool requestInference(const String& imageData, String& result);
    bool enableCloudAI(bool enabled = true);
    
    // Status and monitoring
    SyncStats getSyncStatistics() const { return syncStats_; }
    bool isUploadPending() const { return !uploadQueue_.empty(); }
    uint32_t getPendingUploads() const { return uploadQueue_.size(); }
    String getLastError() const { return lastError_; }
    
    // Queue management
    bool addToUploadQueue(const UploadRequest& request);
    void clearUploadQueue();
    void prioritizeUpload(const String& requestId);
    std::vector<UploadRequest> getPendingRequests() const;
    
    // Platform-specific implementations
    bool configureAWS();
    bool configureAzure();
    bool configureGCP();
    bool configureCustomCloud();
    
    // Security features
    bool encryptData(const String& data, String& encrypted);
    bool decryptData(const String& encrypted, String& decrypted);
    bool validateCertificate();
    bool refreshAuthToken();
    
    // Utility functions
    String generateRequestId();
    String formatCloudPath(const String& path, DataType dataType);
    String createMetadata(DataType dataType, const String& additionalInfo = "");
    uint64_t getCloudStorageUsed();
    bool optimizeCloudCosts();
    
    // Event callbacks
    typedef void (*UploadCompleteCallback)(const UploadRequest& request, bool success);
    typedef void (*RealTimeEventCallback)(const RealTimeEvent& event);
    typedef void (*CloudErrorCallback)(const String& error, int errorCode);
    typedef void (*SyncCompleteCallback)(bool success, const SyncStats& stats);
    
    void setUploadCompleteCallback(UploadCompleteCallback callback) { uploadCallback_ = callback; }
    void setRealTimeEventCallback(RealTimeEventCallback callback) { eventCallback_ = callback; }
    void setCloudErrorCallback(CloudErrorCallback callback) { errorCallback_ = callback; }
    void setSyncCompleteCallback(SyncCompleteCallback callback) { syncCallback_ = callback; }

private:
    // Core configuration and state
    CloudConfig config_;
    bool initialized_;
    bool connected_;
    String authToken_;
    uint32_t tokenExpiry_;
    String lastError_;
    
    // Upload queue and management
    std::queue<UploadRequest> uploadQueue_;
    std::map<String, UploadRequest> activeUploads_;
    SyncStats syncStats_;
    
    // HTTP client
    HTTPClient httpClient_;
    WiFiClientSecure secureClient_;
    
    // Real-time streaming
    bool streamingEnabled_;
    String streamEndpoint_;
    uint32_t lastHeartbeat_;
    
    // Callbacks
    UploadCompleteCallback uploadCallback_;
    RealTimeEventCallback eventCallback_;
    CloudErrorCallback errorCallback_;
    SyncCompleteCallback syncCallback_;
    
    // Internal methods
    bool initializeHTTPClient();
    CloudResponse makeHTTPRequest(const String& method, const String& url, const String& payload = "");
    bool processUploadQueue();
    bool uploadSingleFile(const UploadRequest& request);
    
    // Platform-specific helpers
    String getAWSSignature(const String& request);
    String getAzureSASToken();
    String getGCPAuthHeader();
    
    // Authentication helpers
    bool performOAuthFlow();
    bool renewCredentials();
    bool validateCredentials();
    
    // Data processing
    String compressData(const String& data);
    String decompressData(const String& compressed);
    String encryptPayload(const String& payload);
    String decryptPayload(const String& encrypted);
    
    // Error handling
    void handleHTTPError(int httpCode, const String& response);
    void updateSyncStats(bool success, uint32_t responseTime);
    void logCloudOperation(const String& operation, bool success);
    
    // Queue processing
    void processHighPriorityUploads();
    void retryFailedUploads();
    bool shouldRetryUpload(const UploadRequest& request);
    
    // Notification helpers
    void notifyUploadComplete(const UploadRequest& request, bool success);
    void notifyRealTimeEvent(const RealTimeEvent& event);
    void notifyCloudError(const String& error, int errorCode);
    void notifySyncComplete(bool success);
};

// Global cloud integrator instance
extern CloudIntegrator* g_cloudIntegrator;

// Utility functions for easy integration
bool initializeCloudIntegration(const CloudConfig& config);
bool uploadToCloud(const String& localPath, DataType dataType);
bool syncAllData();
bool sendCloudEvent(const String& eventType, const String& data);
void cleanupCloudIntegration();

// Quick access functions
bool isCloudConnected();
String getCloudStatus();
uint32_t getPendingCloudUploads();
SyncStats getCloudStats();
bool hasCloudErrors();

#endif // CLOUD_INTEGRATOR_H