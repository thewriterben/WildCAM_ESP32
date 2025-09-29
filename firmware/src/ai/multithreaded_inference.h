/**
 * @file multithreaded_inference.h
 * @brief Multi-threaded inference engine for ESP32 wildlife classification
 * 
 * Provides parallel processing capabilities using FreeRTOS tasks for
 * improved throughput and real-time performance on ESP32-S3 dual-core.
 */

#ifndef MULTITHREADED_INFERENCE_H
#define MULTITHREADED_INFERENCE_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <stdint.h>
#include <stddef.h>

#include "inference_engine.h"
#include "postprocessing.h"

// Task configuration
#define INFERENCE_TASK_STACK_SIZE   8192
#define PREPROCESSING_TASK_STACK_SIZE 4096
#define POSTPROCESSING_TASK_STACK_SIZE 4096
#define MAX_INFERENCE_QUEUE_SIZE    5
#define MAX_RESULT_QUEUE_SIZE      10

/**
 * Task priorities (higher number = higher priority)
 */
enum TaskPriority {
    PRIORITY_PREPROCESSING = 2,
    PRIORITY_INFERENCE = 3,
    PRIORITY_POSTPROCESSING = 2,
    PRIORITY_MAIN_COORDINATOR = 4
};

/**
 * Task core assignments for ESP32-S3
 */
enum TaskCore {
    CORE_0 = 0,  // Protocol CPU core
    CORE_1 = 1   // Application CPU core
};

/**
 * Processing stage enumeration
 */
enum ProcessingStage {
    STAGE_IDLE = 0,
    STAGE_PREPROCESSING = 1,
    STAGE_INFERENCE = 2,
    STAGE_POSTPROCESSING = 3,
    STAGE_COMPLETE = 4,
    STAGE_ERROR = 5
};

/**
 * Inference request structure
 */
typedef struct {
    uint32_t requestId;                // Unique request ID
    uint8_t* imageData;               // Raw image data
    uint16_t width;                   // Image width
    uint16_t height;                  // Image height
    uint8_t channels;                 // Number of channels
    uint32_t timestamp;               // Request timestamp
    ProcessingStage stage;            // Current processing stage
    float* preprocessedData;          // Preprocessed image data
    SpeciesResult* result;            // Final inference result
    void* userContext;                // User context pointer
} InferenceRequest;

/**
 * Processing statistics
 */
typedef struct {
    uint32_t totalRequests;           // Total requests processed
    uint32_t completedRequests;       // Successfully completed requests
    uint32_t failedRequests;          // Failed requests
    uint32_t avgPreprocessingTime;    // Average preprocessing time (ms)
    uint32_t avgInferenceTime;        // Average inference time (ms)
    uint32_t avgPostprocessingTime;   // Average postprocessing time (ms)
    uint32_t avgTotalTime;            // Average total processing time (ms)
    uint32_t currentQueueDepth;       // Current queue depth
    uint32_t maxQueueDepth;           // Maximum queue depth reached
    float throughput;                 // Requests per second
} MultithreadedStats;

/**
 * Task status information
 */
typedef struct {
    bool running;                     // Whether task is running
    uint32_t processedCount;          // Number of items processed
    uint32_t errorCount;              // Number of errors
    uint32_t avgProcessingTime;       // Average processing time
    size_t freeStackSpace;            // Free stack space (bytes)
} TaskStatus;

/**
 * Multi-threaded inference engine class
 * 
 * Orchestrates parallel processing pipeline using FreeRTOS tasks
 * for optimal performance on ESP32-S3 dual-core architecture.
 */
class MultithreadedInferenceEngine {
public:
    MultithreadedInferenceEngine();
    ~MultithreadedInferenceEngine();

    /**
     * Initialize multi-threaded inference engine
     * @param inferenceEngine Pointer to inference engine
     * @return true if initialization successful
     */
    bool initialize(InferenceEngine* inferenceEngine);

    /**
     * Start all processing tasks
     * @return true if all tasks started successfully
     */
    bool startTasks();

    /**
     * Stop all processing tasks
     */
    void stopTasks();

    /**
     * Submit inference request (non-blocking)
     * @param imageData Raw image data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     * @param result Pointer to result structure
     * @param userContext Optional user context
     * @return Request ID (0 if failed)
     */
    uint32_t submitInferenceRequest(const uint8_t* imageData, uint16_t width,
                                   uint16_t height, uint8_t channels,
                                   SpeciesResult* result, void* userContext = nullptr);

    /**
     * Submit inference request and wait for completion (blocking)
     * @param imageData Raw image data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     * @param result Pointer to result structure
     * @param timeoutMs Timeout in milliseconds
     * @return true if inference completed successfully
     */
    bool processImageBlocking(const uint8_t* imageData, uint16_t width,
                             uint16_t height, uint8_t channels,
                             SpeciesResult* result, uint32_t timeoutMs = 5000);

    /**
     * Check if specific request is complete
     * @param requestId Request ID to check
     * @return true if request is complete
     */
    bool isRequestComplete(uint32_t requestId);

    /**
     * Get processing statistics
     * @return Current processing statistics
     */
    MultithreadedStats getStatistics() const;

    /**
     * Get task status information
     * @param preprocessingStatus Preprocessing task status
     * @param inferenceStatus Inference task status
     * @param postprocessingStatus Post-processing task status
     */
    void getTaskStatus(TaskStatus* preprocessingStatus,
                      TaskStatus* inferenceStatus,
                      TaskStatus* postprocessingStatus);

    /**
     * Reset statistics
     */
    void resetStatistics();

    /**
     * Check if system is ready for processing
     * @return true if ready
     */
    bool isReady() const { return initialized_ && tasksRunning_; }

    /**
     * Set processing thread affinity
     * @param enableCoreAffinity Whether to pin tasks to specific cores
     */
    void setCoreAffinity(bool enableCoreAffinity);

private:
    bool initialized_;                    // Initialization status
    bool tasksRunning_;                   // Whether tasks are running
    InferenceEngine* inferenceEngine_;    // Inference engine instance
    PostProcessor* postProcessor_;        // Post-processor instance
    uint32_t nextRequestId_;              // Next request ID

    // FreeRTOS task handles
    TaskHandle_t preprocessingTaskHandle_;
    TaskHandle_t inferenceTaskHandle_;
    TaskHandle_t postprocessingTaskHandle_;

    // Inter-task communication queues
    QueueHandle_t preprocessingQueue_;    // Raw image -> preprocessing
    QueueHandle_t inferenceQueue_;       // Preprocessed -> inference
    QueueHandle_t postprocessingQueue_;  // Raw results -> postprocessing
    QueueHandle_t resultQueue_;          // Final results

    // Synchronization primitives
    SemaphoreHandle_t statsMutex_;       // Statistics mutex
    SemaphoreHandle_t requestMapMutex_;  // Request map mutex

    // Request tracking
    InferenceRequest* activeRequests_[MAX_INFERENCE_QUEUE_SIZE];
    uint8_t activeRequestCount_;

    // Statistics
    MultithreadedStats stats_;

    // Task status
    TaskStatus preprocessingStatus_;
    TaskStatus inferenceStatus_;
    TaskStatus postprocessingStatus_;

    // Configuration
    bool coreAffinityEnabled_;

    /**
     * Preprocessing task function
     * @param parameters Task parameters (MultithreadedInferenceEngine instance)
     */
    static void preprocessingTask(void* parameters);

    /**
     * Inference task function
     * @param parameters Task parameters (MultithreadedInferenceEngine instance)
     */
    static void inferenceTask(void* parameters);

    /**
     * Post-processing task function
     * @param parameters Task parameters (MultithreadedInferenceEngine instance)
     */
    static void postprocessingTask(void* parameters);

    /**
     * Create inference request
     * @param imageData Raw image data
     * @param width Image width
     * @param height Image height
     * @param channels Number of channels
     * @param result Result structure
     * @param userContext User context
     * @return Pointer to created request (nullptr if failed)
     */
    InferenceRequest* createRequest(const uint8_t* imageData, uint16_t width,
                                   uint16_t height, uint8_t channels,
                                   SpeciesResult* result, void* userContext);

    /**
     * Free inference request
     * @param request Request to free
     */
    void freeRequest(InferenceRequest* request);

    /**
     * Find request by ID
     * @param requestId Request ID
     * @return Pointer to request (nullptr if not found)
     */
    InferenceRequest* findRequest(uint32_t requestId);

    /**
     * Update statistics
     * @param stage Processing stage that completed
     * @param processingTime Time taken for processing
     * @param success Whether processing was successful
     */
    void updateStatistics(ProcessingStage stage, uint32_t processingTime, bool success);

    /**
     * Update task status
     * @param status Task status structure to update
     * @param processingTime Time taken for last operation
     * @param success Whether operation was successful
     */
    void updateTaskStatus(TaskStatus* status, uint32_t processingTime, bool success);

    /**
     * Monitor task health and restart if necessary
     */
    void monitorTaskHealth();

    /**
     * Calculate current throughput
     * @return Throughput in requests per second
     */
    float calculateThroughput();
};

/**
 * Inference result callback function type
 * @param requestId Request ID
 * @param result Inference result
 * @param userContext User context
 */
typedef void (*InferenceCallback)(uint32_t requestId, const SpeciesResult* result, void* userContext);

/**
 * Global multithreaded inference engine instance
 */
extern MultithreadedInferenceEngine* g_multithreaded_engine;

/**
 * Initialize global multithreaded inference engine
 * @param inferenceEngine Inference engine instance
 * @return true if initialization successful
 */
bool initializeMultithreadedInference(InferenceEngine* inferenceEngine);

/**
 * Cleanup global multithreaded inference engine
 */
void cleanupMultithreadedInference();

#endif // MULTITHREADED_INFERENCE_H