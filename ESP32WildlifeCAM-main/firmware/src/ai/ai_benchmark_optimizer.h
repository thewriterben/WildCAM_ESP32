/**
 * @file ai_benchmark_optimizer.h
 * @brief AI Performance Benchmarking and Auto-Optimization System
 * 
 * Provides comprehensive benchmarking capabilities for AI models and
 * automatic optimization of inference parameters to maximize wildlife
 * detection performance while maintaining power efficiency.
 */

#ifndef AI_BENCHMARK_OPTIMIZER_H
#define AI_BENCHMARK_OPTIMIZER_H

#include "ai_common.h"
#include "tinyml/inference_engine.h"
#include "power_ai_optimizer.h"
#include <vector>
#include <memory>
#include <map>

/**
 * Benchmark Test Types
 */
enum class BenchmarkType {
    INFERENCE_SPEED,        // Model inference time
    POWER_CONSUMPTION,      // Power usage during inference
    MEMORY_USAGE,          // RAM and flash usage
    ACCURACY_TEST,         // Detection accuracy with test dataset
    THERMAL_STRESS,        // Performance under temperature stress
    BATTERY_ENDURANCE,     // Long-term power efficiency
    REAL_WORLD_PERFORMANCE // Field testing with actual wildlife
};

/**
 * Optimization Target
 */
enum class OptimizationTarget {
    MAXIMIZE_ACCURACY,      // Prioritize detection accuracy
    MINIMIZE_POWER,         // Prioritize power efficiency
    MAXIMIZE_SPEED,         // Prioritize inference speed
    BALANCED_PERFORMANCE,   // Balance all metrics
    CUSTOM_WEIGHTED        // Custom weight-based optimization
};

/**
 * Benchmark Result Structure
 */
struct BenchmarkResult {
    BenchmarkType testType;
    String modelName;
    float averageInferenceTime_ms;
    float averagePowerConsumption_mW;
    float memoryUsage_KB;
    float detectionAccuracy_percent;
    float falsePositiveRate_percent;
    float falseNegativeRate_percent;
    float temperatureStability;
    float batteryLifeImpact_hours;
    uint32_t totalTestSamples;
    uint32_t successfulDetections;
    unsigned long testDuration_ms;
    String environmentalConditions;
    bool isValid;
    
    BenchmarkResult() : testType(BenchmarkType::INFERENCE_SPEED),
                       averageInferenceTime_ms(0.0f),
                       averagePowerConsumption_mW(0.0f),
                       memoryUsage_KB(0.0f),
                       detectionAccuracy_percent(0.0f),
                       falsePositiveRate_percent(0.0f),
                       falseNegativeRate_percent(0.0f),
                       temperatureStability(0.0f),
                       batteryLifeImpact_hours(0.0f),
                       totalTestSamples(0),
                       successfulDetections(0),
                       testDuration_ms(0),
                       isValid(false) {}
};

/**
 * Optimization Configuration
 */
struct OptimizationConfig {
    OptimizationTarget target;
    std::map<String, float> weightings; // Custom weights for different metrics
    float minAccuracyThreshold;         // Minimum acceptable accuracy
    float maxPowerBudget_mW;           // Maximum power consumption
    float maxInferenceTime_ms;         // Maximum inference time
    bool enableModelQuantization;       // Allow model quantization
    bool enableDynamicOptimization;     // Enable runtime optimization
    uint32_t optimizationInterval_hours; // How often to re-optimize
    
    OptimizationConfig() : target(OptimizationTarget::BALANCED_PERFORMANCE),
                          minAccuracyThreshold(0.8f),
                          maxPowerBudget_mW(500.0f),
                          maxInferenceTime_ms(2000.0f),
                          enableModelQuantization(true),
                          enableDynamicOptimization(true),
                          optimizationInterval_hours(24) {}
};

/**
 * Performance Baseline
 */
struct PerformanceBaseline {
    float baselineAccuracy;
    float baselinePowerConsumption;
    float baselineInferenceTime;
    float baselineMemoryUsage;
    String baselineModelVersion;
    unsigned long baselineTimestamp;
    
    PerformanceBaseline() : baselineAccuracy(0.0f),
                           baselinePowerConsumption(0.0f),
                           baselineInferenceTime(0.0f),
                           baselineMemoryUsage(0.0f),
                           baselineTimestamp(0) {}
};

/**
 * Wildlife Test Dataset Entry
 */
struct WildlifeTestSample {
    CameraFrame testFrame;
    SpeciesType expectedSpecies;
    BehaviorType expectedBehavior;
    float expectedConfidence;
    String sampleDescription;
    String environmentalContext;
    bool isGroundTruth;
    
    WildlifeTestSample() : expectedSpecies(SpeciesType::UNKNOWN),
                          expectedBehavior(BehaviorType::UNKNOWN),
                          expectedConfidence(0.0f),
                          isGroundTruth(false) {}
};

/**
 * AI Benchmark and Auto-Optimization System
 * 
 * Provides comprehensive testing and optimization capabilities
 * for AI models in wildlife detection applications.
 */
class AIBenchmarkOptimizer {
public:
    AIBenchmarkOptimizer();
    ~AIBenchmarkOptimizer();

    // Core initialization
    bool init(InferenceEngine* inferenceEngine, PowerAIOptimizer* powerOptimizer);
    void cleanup();
    bool isInitialized() const;
    
    // Benchmark execution
    BenchmarkResult runBenchmark(BenchmarkType type, ModelType model);
    std::vector<BenchmarkResult> runComprehensiveBenchmark();
    BenchmarkResult runCustomBenchmark(const OptimizationConfig& config);
    
    // Model comparison and analysis
    std::vector<BenchmarkResult> compareModels(const std::vector<ModelType>& models);
    BenchmarkResult findBestModel(OptimizationTarget target);
    void analyzeModelPerformance(ModelType model);
    
    // Auto-optimization
    bool enableAutoOptimization(const OptimizationConfig& config);
    void disableAutoOptimization();
    bool runOptimizationCycle();
    OptimizationConfig optimizeConfiguration(OptimizationTarget target);
    
    // Test dataset management
    bool loadTestDataset(const char* datasetPath);
    bool addTestSample(const WildlifeTestSample& sample);
    void clearTestDataset();
    size_t getTestDatasetSize() const;
    bool validateTestDataset();
    
    // Baseline management
    void establishBaseline(ModelType model);
    PerformanceBaseline getCurrentBaseline() const;
    float calculatePerformanceImprovement(const BenchmarkResult& result);
    
    // Environmental testing
    BenchmarkResult runTemperatureStressTest(ModelType model, float minTemp, float maxTemp);
    BenchmarkResult runBatteryEnduranceTest(ModelType model, uint32_t testDuration_hours);
    BenchmarkResult runFieldPerformanceTest(ModelType model, uint32_t fieldTestDuration_hours);
    
    // Real-time optimization
    void enableRealTimeOptimization(bool enable = true);
    void updateOptimizationParameters();
    bool adaptToCurrentConditions();
    void optimizeForCurrentWildlife();
    
    // Performance monitoring
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    std::vector<BenchmarkResult> getPerformanceHistory();
    void generatePerformanceReport(const char* filename);
    
    // Advanced optimization features
    bool optimizeModelQuantization(ModelType model);
    bool optimizeInferenceScheduling();
    bool optimizePowerAllocation();
    bool optimizeConfidenceThresholds();
    
    // Machine learning optimization
    bool enableMLOptimization(bool enable = true);
    void trainOptimizationModel();
    OptimizationConfig predictOptimalConfiguration();
    
    // Conservation-specific optimization
    void setConservationTargets(const std::vector<SpeciesType>& targetSpecies);
    bool optimizeForConservation();
    BenchmarkResult evaluateConservationImpact();
    
    // Multi-objective optimization
    OptimizationConfig optimizeMultiObjective(const std::vector<OptimizationTarget>& targets,
                                             const std::vector<float>& weights);
    bool runParetoOptimization();
    std::vector<OptimizationConfig> getParetoFrontier();
    
    // Debug and analysis
    void enableDebugMode(bool enable = true);
    void printBenchmarkSummary(const BenchmarkResult& result);
    void printOptimizationReport();
    void exportBenchmarkData(const char* filename);
    bool validateOptimizationResults();

private:
    // Core components
    InferenceEngine* inferenceEngine_;
    PowerAIOptimizer* powerOptimizer_;
    bool initialized_;
    
    // Configuration
    OptimizationConfig currentConfig_;
    bool autoOptimizationEnabled_;
    bool realTimeOptimizationEnabled_;
    bool mlOptimizationEnabled_;
    bool debugModeEnabled_;
    
    // Test datasets
    std::vector<WildlifeTestSample> testDataset_;
    std::vector<CameraFrame> syntheticDataset_;
    String currentDatasetPath_;
    
    // Performance tracking
    PerformanceBaseline baseline_;
    std::vector<BenchmarkResult> benchmarkHistory_;
    std::vector<SpeciesType> conservationTargets_;
    unsigned long lastOptimizationTime_;
    
    // Optimization state
    std::map<ModelType, BenchmarkResult> modelPerformanceCache_;
    std::vector<OptimizationConfig> paretoFrontier_;
    bool optimizationInProgress_;
    
    // Internal benchmark methods
    BenchmarkResult runInferenceSpeedTest(ModelType model);
    BenchmarkResult runPowerConsumptionTest(ModelType model);
    BenchmarkResult runMemoryUsageTest(ModelType model);
    BenchmarkResult runAccuracyTest(ModelType model);
    
    // Test execution helpers
    bool prepareTestEnvironment();
    void cleanupTestEnvironment();
    float measureInferenceTime(ModelType model, const CameraFrame& frame);
    float measurePowerConsumption(ModelType model, const CameraFrame& frame);
    float calculateAccuracy(ModelType model, const std::vector<WildlifeTestSample>& samples);
    
    // Optimization algorithms
    OptimizationConfig gradientDescentOptimization(OptimizationTarget target);
    OptimizationConfig geneticAlgorithmOptimization(OptimizationTarget target);
    OptimizationConfig bayesianOptimization(OptimizationTarget target);
    OptimizationConfig swarmOptimization(OptimizationTarget target);
    
    // Performance analysis
    float calculateObjectiveFunction(const BenchmarkResult& result, OptimizationTarget target);
    float calculateWeightedScore(const BenchmarkResult& result, const std::map<String, float>& weights);
    bool isPerformanceImprovement(const BenchmarkResult& current, const BenchmarkResult& baseline);
    
    // Model optimization helpers
    bool quantizeModel(ModelType model, float targetAccuracy);
    bool pruneModel(ModelType model, float compressionRatio);
    bool optimizeModelArchitecture(ModelType model);
    
    // Environmental adaptation
    void adaptToTemperature(float temperature);
    void adaptToBatteryLevel(float batteryLevel);
    void adaptToLightConditions(float lightLevel);
    void adaptToWildlifeActivity(const std::vector<SpeciesType>& activeSpecies);
    
    // Utility functions
    void updatePerformanceCache(ModelType model, const BenchmarkResult& result);
    BenchmarkResult getCachedPerformance(ModelType model);
    bool shouldRunOptimization();
    void logOptimizationEvent(const String& event);
    
    // Constants
    static const size_t MAX_TEST_SAMPLES = 1000;
    static const size_t MAX_BENCHMARK_HISTORY = 1000;
    static const float DEFAULT_ACCURACY_THRESHOLD;
    static const uint32_t DEFAULT_OPTIMIZATION_INTERVAL_HOURS;
    static const float PERFORMANCE_IMPROVEMENT_THRESHOLD;
};

// Utility functions
const char* benchmarkTypeToString(BenchmarkType type);
const char* optimizationTargetToString(OptimizationTarget target);
float calculatePerformanceScore(const BenchmarkResult& result);
bool compareBenchmarkResults(const BenchmarkResult& a, const BenchmarkResult& b, OptimizationTarget target);

#endif // AI_BENCHMARK_OPTIMIZER_H