/**
 * @file model_manager.h
 * @brief Model Manager for OTA model updates and versioning
 * 
 * Handles TensorFlow Lite model lifecycle including loading, validation,
 * over-the-air updates, and version management for wildlife classification.
 */

#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

// Maximum number of models that can be managed
#define MAX_MODELS 4
#define MODEL_NAME_MAX_LEN 64
#define MODEL_VERSION_MAX_LEN 16
#define MODEL_CHECKSUM_LEN 32

/**
 * Model types supported by the system
 */
enum ModelType {
    MODEL_SPECIES_CLASSIFIER = 0,
    MODEL_BEHAVIOR_ANALYZER = 1,
    MODEL_MOTION_DETECTOR = 2,
    MODEL_OBJECT_DETECTOR = 3
};

/**
 * Model information structure
 */
typedef struct {
    ModelType type;                          // Model type
    char name[MODEL_NAME_MAX_LEN];          // Model name
    char version[MODEL_VERSION_MAX_LEN];    // Model version
    char filename[MODEL_NAME_MAX_LEN];      // Model filename
    uint8_t checksum[MODEL_CHECKSUM_LEN];   // SHA-256 checksum
    size_t modelSize;                       // Model size in bytes
    size_t arenaSize;                       // Required arena size
    float accuracy;                         // Model accuracy
    uint32_t inferenceTimeMs;               // Expected inference time
    bool loaded;                            // Whether model is loaded
    bool validated;                         // Whether model is validated
} ModelInfo;

/**
 * OTA update status
 */
enum OTAStatus {
    OTA_IDLE = 0,
    OTA_CHECKING = 1,
    OTA_DOWNLOADING = 2,
    OTA_VALIDATING = 3,
    OTA_INSTALLING = 4,
    OTA_SUCCESS = 5,
    OTA_ERROR = 6
};

/**
 * Model Manager Class
 * 
 * Manages the lifecycle of TensorFlow Lite models including:
 * - Loading and validation
 * - Version management
 * - Over-the-air updates
 * - Storage management
 */
class ModelManager {
public:
    ModelManager();
    ~ModelManager();

    /**
     * Initialize the model manager
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * Load model metadata from manifest
     * @param manifestPath Path to model manifest JSON file
     * @return true if manifest loaded successfully
     */
    bool loadManifest(const char* manifestPath);

    /**
     * Load a specific model into memory
     * @param type Model type to load
     * @return true if model loaded successfully
     */
    bool loadModel(ModelType type);

    /**
     * Unload a specific model from memory
     * @param type Model type to unload
     */
    void unloadModel(ModelType type);

    /**
     * Get model information
     * @param type Model type
     * @return Pointer to model info structure (nullptr if not found)
     */
    const ModelInfo* getModelInfo(ModelType type) const;

    /**
     * Validate model integrity
     * @param type Model type to validate
     * @return true if model is valid
     */
    bool validateModel(ModelType type);

    /**
     * Check for model updates
     * @param serverUrl URL of update server
     * @return true if updates are available
     */
    bool checkForUpdates(const char* serverUrl);

    /**
     * Download and install model update
     * @param type Model type to update
     * @param updateUrl URL to download update from
     * @return true if update successful
     */
    bool updateModel(ModelType type, const char* updateUrl);

    /**
     * Get OTA update status
     * @return Current OTA status
     */
    OTAStatus getOTAStatus() const { return otaStatus_; }

    /**
     * Get update progress (0-100)
     * @return Update progress percentage
     */
    uint8_t getUpdateProgress() const { return updateProgress_; }

    /**
     * Get total memory usage of loaded models
     * @return Memory usage in bytes
     */
    size_t getTotalMemoryUsage() const;

    /**
     * Get number of loaded models
     * @return Number of loaded models
     */
    uint8_t getLoadedModelCount() const;

    /**
     * List all available models
     * @param models Array to store model info (size MAX_MODELS)
     * @return Number of models found
     */
    uint8_t listModels(ModelInfo* models) const;

private:
    ModelInfo models_[MAX_MODELS];    // Model information array
    uint8_t modelCount_;              // Number of models configured
    OTAStatus otaStatus_;             // Current OTA status
    uint8_t updateProgress_;          // Update progress percentage
    bool initialized_;                // Initialization status

    /**
     * Calculate SHA-256 checksum of file
     * @param filepath Path to file
     * @param checksum Output buffer for checksum (32 bytes)
     * @return true if checksum calculated successfully
     */
    bool calculateChecksum(const char* filepath, uint8_t* checksum);

    /**
     * Compare two checksums
     * @param checksum1 First checksum
     * @param checksum2 Second checksum
     * @return true if checksums match
     */
    bool compareChecksums(const uint8_t* checksum1, const uint8_t* checksum2);

    /**
     * Load model data from storage
     * @param type Model type
     * @param data Output buffer for model data
     * @param maxSize Maximum size of output buffer
     * @return Actual size of model data (0 if error)
     */
    size_t loadModelData(ModelType type, uint8_t* data, size_t maxSize);

    /**
     * Save model data to storage
     * @param type Model type
     * @param data Model data buffer
     * @param size Size of model data
     * @return true if save successful
     */
    bool saveModelData(ModelType type, const uint8_t* data, size_t size);

    /**
     * Download file from URL
     * @param url URL to download from
     * @param filepath Local filepath to save to
     * @param progressCallback Optional progress callback function
     * @return true if download successful
     */
    bool downloadFile(const char* url, const char* filepath, 
                     void (*progressCallback)(uint8_t progress) = nullptr);

    /**
     * Parse JSON manifest file
     * @param jsonData JSON data string
     * @return true if parsing successful
     */
    bool parseManifest(const char* jsonData);

    /**
     * Get model type string
     * @param type Model type
     * @return String representation of model type
     */
    const char* getModelTypeString(ModelType type) const;

    /**
     * Initialize default model configurations
     */
    void initializeDefaultModels();
};

#endif // MODEL_MANAGER_H