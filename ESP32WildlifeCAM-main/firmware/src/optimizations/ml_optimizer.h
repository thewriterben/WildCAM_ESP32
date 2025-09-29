#ifndef ML_OPTIMIZER_H
#define ML_OPTIMIZER_H

#include <Arduino.h>
#include <vector>
#include <memory>

/**
 * ML Optimizer - Enhanced AI/ML performance for ESP32WildlifeCAM
 * 
 * Implements quantized neural networks, optimized image processing,
 * and adaptive AI pipeline for 3x speed improvement and better accuracy.
 */

// Forward declarations
struct CameraFrame;

// Enhanced quantized neural network for 3x speed improvement with edge AI optimizations
class QuantizedNeuralNetwork {
    int8_t* weights_q8;          // 8-bit quantized weights
    int8_t* calibration_data;    // Calibration dataset for quantization
    float scale_factor;
    int32_t zero_point;
    size_t input_size;
    size_t output_size;
    size_t layer_count;
    bool initialized;
    
    // Enhanced optimization features
    bool simd_enabled;
    bool multi_core_enabled;
    uint8_t* tensor_arena;       // Shared memory pool for tensors
    size_t arena_size;
    
public:
    QuantizedNeuralNetwork();
    ~QuantizedNeuralNetwork();
    
    bool init(size_t input_size, size_t output_size, size_t layers);
    void cleanup();
    
    // Enhanced SIMD-optimized inference for ESP32-S3 vector instructions
    std::vector<float> inferenceOptimized(const std::vector<float>& input);
    std::vector<float> inferenceMultiCore(const std::vector<float>& input);
    std::vector<float> inferenceSIMD(const std::vector<float>& input);
    
    // Fixed-point arithmetic for ESP32 optimization
    int32_t dotProductQ8(const int8_t* a, const int8_t* b, size_t len);
    int32_t dotProductSIMD(const int8_t* a, const int8_t* b, size_t len);
    int32_t matrixMultiplyQ8(const int8_t* matrix, const int8_t* vector, 
                           size_t rows, size_t cols);
    
    // Enhanced model compression and optimization
    bool quantizeWithCalibration(const std::vector<std::vector<float>>& weights,
                               const std::vector<std::vector<float>>& calibration_dataset);
    void pruneWeights(float threshold = 0.01f);
    bool knowledgeDistillation(const QuantizedNeuralNetwork& teacher_model,
                             const std::vector<std::vector<float>>& training_data);
    bool quantizeFromFloat(const std::vector<std::vector<float>>& weights);
    void optimizeForESP32();
    void optimizeForESP32S3();
    
    // Dynamic model management
    bool enableDynamicModelSelection(bool enable = true);
    bool loadModelVariant(const uint8_t* model_data, size_t model_size, const String& variant_name);
    String selectOptimalModel(float battery_level, float performance_requirement);
    
    // Performance monitoring
    float getInferenceTime() const { return lastInferenceTime; }
    size_t getModelSize() const;
    float getCompressionRatio() const;
    
private:
    float lastInferenceTime;
    size_t originalModelSize;
    
    // Enhanced memory management
    std::vector<String> loaded_models;
    String current_model_variant;
    bool dynamic_selection_enabled;
    
    void allocateQuantizedMemory();
    void deallocateQuantizedMemory();
    void allocateTensorArena();
    void deallocateTensorArena();
    float dequantize(int8_t value);
    int8_t quantize(float value);
    
    // SIMD optimization helpers
    void initializeSIMD();
    bool isSIMDSupported();
    
    // Multi-core optimization helpers
    static void inferenceTask(void* parameters);
    void distributeInferenceWork(const std::vector<float>& input);
};

// Enhanced optimized image processing pipeline with edge AI features
class FastImageProcessor {
    uint8_t* processingBuffer;
    uint8_t* preprocessingBuffer;  // Separate buffer for preprocessing pipeline
    uint8_t* parallelBuffer;       // Buffer for parallel processing
    size_t bufferSize;
    bool initialized;
    
    // Performance metrics
    float lastProcessingTime;
    uint32_t totalProcessedFrames;
    
    // Pipeline parallelism
    bool pipeline_enabled;
    bool preprocessing_active;
    bool inference_active;
    bool postprocessing_active;
    
public:
    FastImageProcessor();
    ~FastImageProcessor();
    
    bool init(size_t maxImageSize);
    void cleanup();
    
    // Enhanced pipeline processing
    bool enablePipelineParallelism(bool enable = true);
    bool processImagePipelined(const uint8_t* image, uint8_t* output, 
                              uint16_t width, uint16_t height);
    
    // Hardware-accelerated edge detection using ESP32 optimizations
    bool detectEdgesHardware(const uint8_t* image, uint8_t* edges, 
                           uint16_t width, uint16_t height);
    bool detectEdgesAccelerated(const uint8_t* image, uint8_t* edges,
                              uint16_t width, uint16_t height);
    
    // Enhanced optimized color space conversion
    void rgb565ToGrayscale(const uint16_t* rgb, uint8_t* gray, size_t pixels);
    void rgb565ToGrayscaleSIMD(const uint16_t* rgb, uint8_t* gray, size_t pixels);
    void yuvToRgb(const uint8_t* yuv, uint8_t* rgb, size_t pixels);
    void yuvToRgbParallel(const uint8_t* yuv, uint8_t* rgb, size_t pixels);
    void applyGammaCorrection(uint8_t* image, size_t pixels, float gamma = 2.2f);
    
    // Enhanced motion detection using background subtraction
    bool detectMotionOptimized(const uint8_t* current, const uint8_t* background,
                              uint16_t width, uint16_t height, float threshold);
    bool detectMotionAdaptive(const uint8_t* current, const uint8_t* background,
                            uint16_t width, uint16_t height, float base_threshold);
    
    // Batch processing for multiple detections
    bool processBatch(const std::vector<uint8_t*>& images, 
                     std::vector<uint8_t*>& outputs,
                     uint16_t width, uint16_t height);
    
    // Feature extraction optimization
    std::vector<float> extractHOGFeatures(const uint8_t* image, 
                                         uint16_t width, uint16_t height);
    std::vector<float> extractLBPFeatures(const uint8_t* image,
                                         uint16_t width, uint16_t height);
    std::vector<float> extractFeaturesParallel(const uint8_t* image,
                                             uint16_t width, uint16_t height);
    
    // Enhanced image enhancement
    void enhanceContrast(uint8_t* image, uint16_t width, uint16_t height);
    void enhanceContrastAdaptive(uint8_t* image, uint16_t width, uint16_t height);
    void reduceNoise(uint8_t* image, uint16_t width, uint16_t height);
    void reduceNoiseAdaptive(uint8_t* image, uint16_t width, uint16_t height);
    void sharpenImage(uint8_t* image, uint16_t width, uint16_t height);
    void sharpenImageAdaptive(uint8_t* image, uint16_t width, uint16_t height);
    
    // Wildlife-specific preprocessing
    void enhanceWildlifeFeatures(uint8_t* image, uint16_t width, uint16_t height,
                                const String& target_species);
    void optimizeForNightVision(uint8_t* image, uint16_t width, uint16_t height);
    void compensateForWeather(uint8_t* image, uint16_t width, uint16_t height,
                            float weather_factor);
    
    // Performance monitoring
    float getProcessingTime() const { return lastProcessingTime; }
    uint32_t getProcessedFrameCount() const { return totalProcessedFrames; }
    float getAverageProcessingTime() const;
    float getPipelineEfficiency() const;

private:
    // Enhanced assembly-optimized kernels for ESP32/ESP32-S3
    void convolveASM(const uint8_t* input, uint8_t* output, 
                    const int8_t* kernel, uint16_t width, uint16_t height);
    void convolveSIMD(const uint8_t* input, uint8_t* output,
                     const int8_t* kernel, uint16_t width, uint16_t height);
    void sobel3x3(const uint8_t* input, uint8_t* output, 
                  uint16_t width, uint16_t height);
    void sobel3x3SIMD(const uint8_t* input, uint8_t* output,
                     uint16_t width, uint16_t height);
    void gaussian3x3(const uint8_t* input, uint8_t* output,
                     uint16_t width, uint16_t height);
    void gaussian3x3Parallel(const uint8_t* input, uint8_t* output,
                           uint16_t width, uint16_t height);
    
    // Enhanced optimized histogram operations
    void calculateHistogram(const uint8_t* image, uint32_t* histogram, size_t pixels);
    void calculateHistogramParallel(const uint8_t* image, uint32_t* histogram, size_t pixels);
    void equalizeHistogram(uint8_t* image, size_t pixels);
    void equalizeHistogramAdaptive(uint8_t* image, size_t pixels);
    
    // Pipeline management
    void initializePipelineBuffers();
    void cleanupPipelineBuffers();
    static void preprocessingTask(void* parameters);
    static void postprocessingTask(void* parameters);
    
    // Multi-core processing helpers
    struct ProcessingTask {
        const uint8_t* input;
        uint8_t* output;
        uint16_t start_row;
        uint16_t end_row;
        uint16_t width;
        uint16_t height;
        void* processor;
    };
    
    void distributeProcessingWork(const uint8_t* input, uint8_t* output,
                                uint16_t width, uint16_t height);
};

// Enhanced adaptive AI pipeline for dynamic model selection and edge learning
class AdaptiveAIPipeline {
    QuantizedNeuralNetwork primaryModel;
    QuantizedNeuralNetwork lightweightModel;
    QuantizedNeuralNetwork behaviorModel;
    FastImageProcessor imageProcessor;
    
    float batteryThreshold;
    float performanceThreshold;
    bool useLightweightModel;
    bool initialized;
    
    // Enhanced features
    bool edge_learning_enabled;
    bool temporal_consistency_enabled;
    bool confidence_based_processing_enabled;
    float confidence_threshold;
    
    // On-device learning
    std::vector<std::vector<float>> local_training_data;
    std::vector<int> local_training_labels;
    uint32_t adaptation_cycle_count;
    
    // Temporal consistency
    struct FrameHistory {
        std::vector<float> features;
        float confidence;
        String prediction;
        unsigned long timestamp;
    };
    std::vector<FrameHistory> frame_history;
    static const size_t MAX_FRAME_HISTORY = 10;
    
    // Model performance tracking
    struct ModelPerformance {
        float accuracy;
        float inferenceTime;
        float powerConsumption;
        uint32_t usageCount;
        float adaptationScore;  // How well model adapts to local conditions
        
        ModelPerformance() : accuracy(0), inferenceTime(0), 
                           powerConsumption(0), usageCount(0), adaptationScore(0) {}
    };
    
    ModelPerformance primaryPerformance;
    ModelPerformance lightweightPerformance;
    ModelPerformance behaviorPerformance;
    
public:
    AdaptiveAIPipeline();
    ~AdaptiveAIPipeline();
    
    bool init();
    void cleanup();
    
    // Enhanced dynamic model selection based on power/performance requirements
    bool runInference(const uint8_t* image, uint16_t width, uint16_t height,
                     float& confidence, String& species, String& behavior);
    bool runInferenceWithTemporal(const uint8_t* image, uint16_t width, uint16_t height,
                                float& confidence, String& species, String& behavior);
    
    void selectModelBasedOnPower(float batteryLevel);
    void selectModelBasedOnPerformance(float requiredAccuracy, float maxInferenceTime);
    void selectModelBasedOnConditions(float batteryLevel, float lightLevel, 
                                    float temperature, float urgency);
    
    // Enhanced progressive inference - stop early if confident
    bool earlyExitInference(const std::vector<float>& features, 
                           float confidenceThreshold = 0.9f);
    bool confidenceBasedProcessing(const uint8_t* image, uint16_t width, uint16_t height,
                                 float& confidence, String& species);
    
    // Enhanced model management
    bool loadPrimaryModel(const uint8_t* modelData, size_t modelSize);
    bool loadLightweightModel(const uint8_t* modelData, size_t modelSize);
    bool loadBehaviorModel(const uint8_t* modelData, size_t modelSize);
    void unloadModels();
    bool updateModelDynamically(const uint8_t* modelData, size_t modelSize, 
                              const String& modelType);
    
    // Enhanced performance optimization
    void optimizeForBatteryLife();
    void optimizeForAccuracy();
    void optimizeForSpeed();
    void optimizeForEdgeConditions();
    
    // On-device learning and adaptation
    bool enableEdgeLearning(bool enable = true);
    void addTrainingSample(const std::vector<float>& features, int label, float confidence);
    bool adaptModelToLocalPatterns();
    bool performFederatedLearning();
    void shareModelUpdates();
    
    // Temporal consistency
    bool enableTemporalConsistency(bool enable = true);
    void updateFrameHistory(const std::vector<float>& features, float confidence, 
                          const String& prediction);
    String applyTemporalSmoothing(const String& current_prediction, float current_confidence);
    float calculateTemporalConsistency();
    
    // Multi-modal processing
    bool processMultiModal(const uint8_t* image, const float* audio_features,
                         const float* sensor_data, float& confidence, 
                         String& species, String& behavior);
    
    // Behavioral analysis
    String analyzeBehavior(const std::vector<float>& image_features, 
                         const std::vector<FrameHistory>& history);
    bool detectBehaviorPattern(const String& species, const std::vector<String>& recent_behaviors);
    
    // Environmental context integration
    void updateEnvironmentalContext(float temperature, float humidity, float light_level,
                                  uint8_t time_of_day, uint8_t season);
    float calculateEnvironmentalWeight(const String& species);
    
    // Analytics and monitoring
    struct PipelineMetrics {
        float averageInferenceTime;
        float averageAccuracy;
        float powerEfficiency;
        uint32_t totalInferences;
        uint32_t successfulInferences;
        uint32_t adaptationCycles;
        float localAdaptationScore;
        float temporalConsistencyScore;
        
        PipelineMetrics() : averageInferenceTime(0), averageAccuracy(0),
                          powerEfficiency(0), totalInferences(0), successfulInferences(0),
                          adaptationCycles(0), localAdaptationScore(0), temporalConsistencyScore(0) {}
    };
    
    PipelineMetrics getMetrics() const;
    void resetMetrics();
    void generatePerformanceReport();
    
private:
    // Environmental context
    struct EnvironmentalContext {
        float temperature;
        float humidity;
        float light_level;
        uint8_t time_of_day;
        uint8_t season;
        float weather_factor;
    } environmental_context;
    
    void updatePerformanceMetrics(bool primaryUsed, float inferenceTime, 
                                float accuracy, float powerUsed);
    bool shouldUseLightweightModel(float batteryLevel, float urgency);
    std::vector<float> preprocessImage(const uint8_t* image, uint16_t width, uint16_t height);
    std::vector<float> preprocessImageEnhanced(const uint8_t* image, uint16_t width, uint16_t height,
                                             const String& target_species);
    bool postprocessResults(const std::vector<float>& output, float& confidence, 
                          String& species, String& behavior);
    bool postprocessResultsEnhanced(const std::vector<float>& output, 
                                   const EnvironmentalContext& context,
                                   float& confidence, String& species, String& behavior);
    
    // Enhanced processing methods
    void performLocalAdaptation();
    float calculateAdaptationScore();
    bool shouldTriggerAdaptation();
    void cleanupOldTrainingData();
    
    // Temporal consistency helpers
    float calculatePredictionSimilarity(const String& pred1, const String& pred2);
    void smoothPredictionHistory();
    
    // Multi-modal fusion
    std::vector<float> fuseModalFeatures(const std::vector<float>& visual_features,
                                       const std::vector<float>& audio_features,
                                       const std::vector<float>& sensor_features);
};

// Enhanced specialized wildlife detection optimizations with behavioral analysis
class WildlifeDetectionOptimizer {
public:
    // Enhanced species-specific optimization
    struct SpeciesProfile {
        String speciesName;
        float sizeRange[2];        // Min/max size in pixels
        float colorProfile[3];     // Average RGB values
        float movementPattern;     // Movement speed indicator
        float detectionDifficulty; // 0-1 scale
        float groupingTendency;    // Likelihood of being in groups (0-1)
        float activityPatterns[24]; // Activity level by hour of day
        uint8_t typicalGroupSize[2]; // Min/max typical group size
        float seasonalActivityFactor[12]; // Activity factor by month
        
        SpeciesProfile() : speciesName("unknown"), movementPattern(0.5f), 
                          detectionDifficulty(0.5f), groupingTendency(0.3f) {
            sizeRange[0] = 10; sizeRange[1] = 500;
            colorProfile[0] = colorProfile[1] = colorProfile[2] = 128;
            typicalGroupSize[0] = 1; typicalGroupSize[1] = 3;
            for(int i = 0; i < 24; i++) activityPatterns[i] = 0.5f;
            for(int i = 0; i < 12; i++) seasonalActivityFactor[i] = 1.0f;
        }
    };
    
    // Behavioral analysis structures
    enum class BehaviorType {
        FEEDING,
        MATING,
        TERRITORIAL,
        MIGRATION,
        RESTING,
        PLAYING,
        HUNTING,
        FLEEING,
        SOCIALIZING,
        UNKNOWN
    };
    
    struct BehaviorPattern {
        BehaviorType behavior;
        float confidence;
        std::vector<float> movement_signature;
        float duration_seconds;
        String associated_species;
        uint8_t group_size;
    };
    
    // Population counting structures
    struct PopulationCount {
        String species;
        uint8_t individual_count;
        float counting_confidence;
        float density_per_frame;
        bool is_group_behavior;
        std::vector<BoundingBox> individual_boxes;
    };
    
    static std::vector<SpeciesProfile> getCommonSpeciesProfiles();
    static SpeciesProfile getProfileForSpecies(const String& species);
    static std::vector<SpeciesProfile> getLocalSpeciesProfiles(float latitude, float longitude);
    
    // Enhanced adaptive detection parameters
    static float calculateOptimalThreshold(const String& species, float lightLevel);
    static float calculateOptimalThresholdEnhanced(const String& species, float lightLevel,
                                                  float temperature, float season_factor);
    static void adjustCameraSettings(const String& targetSpecies, float lightLevel);
    static void adjustCameraSettingsAdvanced(const String& targetSpecies, 
                                            const EnvironmentalContext& context);
    static bool isOptimalTimeForSpecies(const String& species, uint32_t timeOfDay);
    static bool isOptimalTimeForSpeciesAdvanced(const String& species, uint32_t timeOfDay,
                                              uint8_t month, float weather_factor);
    
    // Enhanced environmental optimization
    static float compensateForWeather(float baseThreshold, float temperature, 
                                    float humidity, float windSpeed);
    static float compensateForWeatherAdvanced(float baseThreshold, float temperature,
                                            float humidity, float windSpeed, float precipitation);
    static void optimizeForLightConditions(float lightLevel, bool& useIRIllumination,
                                         float& exposureAdjustment);
    static void optimizeForLightConditionsAdvanced(float lightLevel, uint8_t timeOfDay,
                                                  bool& useIRIllumination, float& exposureAdjustment,
                                                  float& gainAdjustment);
    
    // Enhanced behavior-specific optimizations
    static float getPredictedActivityLevel(const String& species, uint32_t timeOfDay, 
                                         uint32_t season);
    static BehaviorPattern analyzeBehaviorFromMovement(const std::vector<float>& movement_data,
                                                     const String& species);
    static std::vector<String> getActiveSpeciesForTime(uint32_t timeOfDay, uint32_t season);
    static std::vector<String> getActiveSpeciesForTimeAdvanced(uint32_t timeOfDay, uint32_t season,
                                                             float temperature, float weather_factor);
    
    // Population counting and group analysis
    static PopulationCount countIndividuals(const std::vector<BoundingBox>& detections,
                                          const String& species);
    static PopulationCount countIndividualsAdvanced(const std::vector<BoundingBox>& detections,
                                                   const String& species,
                                                   const uint8_t* image_data,
                                                   uint16_t image_width, uint16_t image_height);
    static bool detectGroupBehavior(const std::vector<BoundingBox>& detections,
                                  const String& species);
    static float estimatePopulationDensity(const PopulationCount& count, float camera_fov,
                                         float estimated_distance);
    
    // Activity classification
    static BehaviorType classifyActivity(const std::vector<float>& movement_features,
                                       const String& species, uint8_t timeOfDay);
    static BehaviorType classifyActivityAdvanced(const std::vector<float>& movement_features,
                                               const std::vector<float>& audio_features,
                                               const String& species, 
                                               const EnvironmentalContext& context);
    static bool detectFeedingBehavior(const std::vector<float>& movement_features,
                                     const String& species);
    static bool detectMatingBehavior(const std::vector<float>& movement_features,
                                   const String& species, uint8_t month);
    static bool detectTerritorialBehavior(const std::vector<float>& movement_features,
                                        const String& species);
    
    // Species-specific optimizations for local populations
    static void optimizeForLocalPopulation(const String& species, float local_adaptation_factor);
    static SpeciesProfile createCustomProfile(const String& species,
                                             const std::vector<float>& local_observations);
    static void updateProfileFromObservations(SpeciesProfile& profile,
                                             const std::vector<BehaviorPattern>& observations);
    
    // Advanced environmental context
    struct EnvironmentalContext {
        float temperature;
        float humidity;
        float light_level;
        float wind_speed;
        float precipitation;
        uint8_t time_of_day;
        uint8_t month;
        float moon_phase;
        float pressure;
        String weather_condition;
    };
    
    static float calculateEnvironmentalOptimization(const EnvironmentalContext& context,
                                                   const String& species);
    static void adaptToSeasonalChanges(const String& species, uint8_t month, 
                                     float& detection_threshold, uint32_t& check_interval);
};

// Advanced Edge AI Performance Monitor
class EdgeAIPerformanceMonitor {
public:
    struct InferenceMetrics {
        float average_inference_time_ms;
        float min_inference_time_ms;
        float max_inference_time_ms;
        uint32_t total_inferences;
        uint32_t successful_inferences;
        uint32_t failed_inferences;
        float frames_per_second;
        float inference_efficiency; // successful inferences / total time
    };
    
    struct MemoryMetrics {
        uint32_t heap_usage_bytes;
        uint32_t psram_usage_bytes;
        uint32_t tensor_arena_usage_bytes;
        float memory_fragmentation_percent;
        uint32_t allocation_failures;
        float memory_efficiency; // useful memory / total allocated
    };
    
    struct PowerMetrics {
        float average_power_consumption_mW;
        float inference_power_consumption_mW;
        float standby_power_consumption_mW;
        float power_efficiency; // inferences per mW
        uint32_t power_optimization_events;
        float battery_life_extension_percent;
    };
    
    struct ModelMetrics {
        String current_model_name;
        float model_accuracy_percent;
        uint32_t model_size_bytes;
        float compression_ratio;
        uint32_t quantization_level;
        float false_positive_rate;
        float false_negative_rate;
        uint32_t model_switches;
        float adaptation_score;
    };
    
    struct QualityMetrics {
        float detection_accuracy_percent;
        float species_classification_accuracy_percent;
        float behavior_classification_accuracy_percent;
        uint32_t total_detections;
        uint32_t verified_detections;
        float temporal_consistency_score;
        float environmental_adaptation_score;
    };
    
    EdgeAIPerformanceMonitor();
    ~EdgeAIPerformanceMonitor();
    
    bool init();
    void cleanup();
    
    // Real-time profiling
    void startInferenceTimer();
    void endInferenceTimer(bool successful);
    void recordMemoryUsage();
    void recordPowerConsumption(float power_mW);
    void recordModelPerformance(const String& model_name, float accuracy, 
                               uint32_t model_size, bool model_switched);
    void recordDetectionQuality(bool detection_successful, bool species_correct,
                              bool behavior_correct, float confidence);
    
    // Analytics and reporting
    InferenceMetrics getInferenceMetrics() const;
    MemoryMetrics getMemoryMetrics() const;
    PowerMetrics getPowerMetrics() const;
    ModelMetrics getModelMetrics() const;
    QualityMetrics getQualityMetrics() const;
    
    // Adaptive optimization triggers
    bool shouldOptimizeForSpeed();
    bool shouldOptimizeForAccuracy();
    bool shouldOptimizeForPower();
    bool shouldSwitchModel();
    bool shouldTriggerMaintenance();
    
    // Performance alerts
    void enablePerformanceAlerts(bool enable = true);
    void setPerformanceThresholds(float min_fps, float max_inference_time_ms,
                                float min_accuracy_percent, float max_power_mW);
    std::vector<String> getActiveAlerts();
    
    // Reporting and visualization
    void generatePerformanceReport(const char* filename);
    void exportMetricsToCSV(const char* filename);
    void printRealTimeStatus();
    void resetAllMetrics();
    
    // Advanced analytics
    float calculateOverallSystemHealth();
    float predictBatteryLife();
    std::vector<String> getOptimizationRecommendations();
    void generateTrendAnalysis();

private:
    bool initialized_;
    unsigned long monitoring_start_time_;
    unsigned long last_inference_start_;
    
    // Metric storage
    InferenceMetrics inference_metrics_;
    MemoryMetrics memory_metrics_;
    PowerMetrics power_metrics_;
    ModelMetrics model_metrics_;
    QualityMetrics quality_metrics_;
    
    // Performance tracking
    std::vector<float> inference_time_history_;
    std::vector<float> power_consumption_history_;
    std::vector<float> accuracy_history_;
    static const size_t MAX_HISTORY_SIZE = 1000;
    
    // Alert system
    bool alerts_enabled_;
    struct PerformanceThresholds {
        float min_fps;
        float max_inference_time_ms;
        float min_accuracy_percent;
        float max_power_mW;
        float min_free_memory_kb;
    } thresholds_;
    
    std::vector<String> active_alerts_;
    
    // Internal methods
    void updateInferenceMetrics(float inference_time_ms, bool successful);
    void updateMemoryMetrics();
    void updatePowerMetrics(float power_mW);
    void checkPerformanceAlerts();
    void cleanupHistoryData();
    float calculateMovingAverage(const std::vector<float>& data, size_t window_size);
    float calculateTrend(const std::vector<float>& data);
};

#endif // ML_OPTIMIZER_H