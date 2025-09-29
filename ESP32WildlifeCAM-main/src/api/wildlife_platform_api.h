/**
 * @file wildlife_platform_api.h
 * @brief Universal wildlife platform API wrapper
 * @author ESP32WildlifeCAM Team
 * @date 2024-09-01
 */

#ifndef WILDLIFE_PLATFORM_API_H
#define WILDLIFE_PLATFORM_API_H

#include <Arduino.h>
#include <ArduinoJson.h>

namespace Wildlife {
namespace API {

/**
 * Supported wildlife monitoring platforms
 */
enum class Platform {
    NONE = 0,
    RECONYX,        // Reconyx camera platform compatibility
    BUSHNELL,       // Bushnell trail camera integration
    CAMTRAPTIONS,   // Camtraptions research platform support
    MOVEBANK,       // Movebank research database
    EBIRD,          // eBird citizen science platform
    GBIF            // Global Biodiversity Information Facility
};

/**
 * Image data structure for platform uploads
 */
struct ImageData {
    uint8_t* image_data = nullptr;
    size_t image_size = 0;
    String format = "JPEG";
    uint32_t timestamp = 0;
    String device_id = "";
    float latitude = 0.0;
    float longitude = 0.0;
    String species_detected = "";
    float confidence = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
};

/**
 * Telemetry data structure
 */
struct TelemetryData {
    uint32_t timestamp = 0;
    String device_id = "";
    float latitude = 0.0;
    float longitude = 0.0;
    float battery_level = 0.0;
    float temperature = 0.0;
    float humidity = 0.0;
    bool motion_detected = false;
    uint32_t image_count = 0;
};

/**
 * Platform-specific data format
 */
struct PlatformData {
    String metadata = "";
    uint8_t* image_data = nullptr;
    size_t image_size = 0;
    String format = "";
};

/**
 * Universal Wildlife Platform API
 * 
 * Provides unified interface for integrating with commercial wildlife
 * monitoring platforms and research databases.
 */
class WildlifePlatformAPI {
public:
    WildlifePlatformAPI();
    ~WildlifePlatformAPI();
    
    // Platform management
    bool initialize(Platform platform, const char* config_json);
    bool authenticate(const char* credentials);
    bool disconnect();
    
    // Data upload
    bool uploadImage(const ImageData& image);
    bool uploadTelemetry(const TelemetryData& telemetry);
    
    // Data download
    bool downloadSpeciesDatabase(const char* region);
    
    // Status queries
    bool isInitialized() const { return initialized_; }
    Platform getCurrentPlatform() const { return current_platform_; }
    bool isAuthenticated() const { return !authentication_token_.isEmpty(); }
    
    // Utility functions
    static const char* getPlatformName(Platform platform);
    
private:
    bool initialized_;
    Platform current_platform_;
    String authentication_token_;
    uint32_t last_sync_time_;
    
    // Platform configuration
    String base_url_;
    String api_key_;
    String username_;
    String password_;
    String research_project_id_;
    
    // Platform-specific initialization
    bool initializeReconyx(const JsonDocument& config);
    bool initializeBushnell(const JsonDocument& config);
    bool initializeCamtraptions(const JsonDocument& config);
    bool initializeMovebank(const JsonDocument& config);
    bool initializeEBird(const JsonDocument& config);
    bool initializeGBIF(const JsonDocument& config);
    
    // Authentication methods
    bool authenticateReconyx(const JsonDocument& creds);
    bool authenticateBushnell(const JsonDocument& creds);
    bool authenticateCamtraptions(const JsonDocument& creds);
    bool authenticateMovebank(const JsonDocument& creds);
    bool authenticateEBird(const JsonDocument& creds);
    bool authenticateGBIF(const JsonDocument& creds);
    
    // Data conversion
    PlatformData convertImageToPlatformFormat(const ImageData& image);
    
    // Upload methods
    bool uploadToReconyx(const PlatformData& data);
    bool uploadToBushnell(const PlatformData& data);
    bool uploadToCamtraptions(const PlatformData& data);
    bool uploadToMovebank(const PlatformData& data);
    bool uploadToEBird(const PlatformData& data);
    bool uploadToGBIF(const PlatformData& data);
    
    // Telemetry
    bool sendTelemetryData(const String& json_data);
    
    // Species database downloads
    bool downloadEBirdSpeciesData(const char* region);
    bool downloadGBIFSpeciesData(const char* region);
    bool downloadCamtraptionsSpeciesData(const char* region);
};

} // namespace API
} // namespace Wildlife

#endif // WILDLIFE_PLATFORM_API_H