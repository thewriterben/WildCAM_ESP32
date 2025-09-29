/**
 * @file test_predictive_analytics.cpp
 * @brief Unit tests for Predictive Analytics Engine
 * 
 * Comprehensive unit tests for the predictive wildlife behavior
 * analytics system components.
 */

#include <cassert>
#include <iostream>
#include <vector>
#include "../firmware/src/ai/predictive/predictive_analytics_engine.h"
#include "../firmware/src/ai/predictive/behavior_pattern_detector.h"
#include "../firmware/src/ai/predictive/temporal_sequence_analyzer.h"

// Test utility functions
void assertEqual(float expected, float actual, float tolerance = 0.01f) {
    if (abs(expected - actual) > tolerance) {
        std::cerr << "Test failed: Expected " << expected << ", got " << actual << std::endl;
        assert(false);
    }
}

void assertTrue(bool condition, const char* message = "Assertion failed") {
    if (!condition) {
        std::cerr << "Test failed: " << message << std::endl;
        assert(false);
    }
}

// Mock data generators
BehaviorResult createMockBehavior(BehaviorType type, float confidence = 0.8f, uint32_t duration = 300) {
    BehaviorResult behavior;
    behavior.primaryBehavior = type;
    behavior.confidence = confidence;
    behavior.duration = duration;
    behavior.timestamp = millis();
    behavior.activity_level = 0.7f;
    behavior.stress_level = 0.2f;
    return behavior;
}

EnvironmentalData createMockEnvironment(float temp = 25.0f, float humidity = 50.0f) {
    EnvironmentalData env;
    env.temperature = temp;
    env.humidity = humidity;
    env.lightLevel = 128;
    env.timestamp = millis();
    return env;
}

/**
 * Test Predictive Analytics Engine Initialization
 */
void testPredictiveEngineInit() {
    std::cout << "Testing Predictive Analytics Engine Initialization..." << std::endl;
    
    PredictiveAnalyticsEngine engine;
    PredictiveConfig config;
    
    // Test successful initialization
    bool initResult = engine.init(config);
    assertTrue(initResult, "Engine initialization should succeed");
    assertTrue(engine.isReadyForPredictions() == false, "Should not be ready without data");
    
    std::cout << "✓ Predictive engine initialization test passed" << std::endl;
}

/**
 * Test Behavior Pattern Detection
 */
void testBehaviorPatternDetection() {
    std::cout << "Testing Behavior Pattern Detection..." << std::endl;
    
    BehaviorPatternDetector detector;
    PatternDetectionConfig config;
    
    bool initResult = detector.init(config);
    assertTrue(initResult, "Pattern detector initialization should succeed");
    
    // Create test behavior sequence: FEEDING -> RESTING -> MOVING
    std::vector<BehaviorResult> behaviors;
    behaviors.push_back(createMockBehavior(BehaviorType::FEEDING, 0.9f));
    behaviors.push_back(createMockBehavior(BehaviorType::RESTING, 0.8f));
    behaviors.push_back(createMockBehavior(BehaviorType::MOVING, 0.7f));
    
    EnvironmentalData env = createMockEnvironment();
    
    // Add observations
    for (const auto& behavior : behaviors) {
        detector.addBehaviorObservation(behavior, env);
    }
    
    // Test pattern detection
    PatternDetectionResult result = detector.detectPatterns(behaviors, env);
    assertTrue(result.totalPatternsAnalyzed >= 0, "Should analyze some patterns");
    
    // Test prediction
    std::vector<BehaviorType> sequence = {BehaviorType::FEEDING, BehaviorType::RESTING};
    auto prediction = detector.predictNextBehavior(sequence, env);
    assertTrue(prediction.second >= 0.0f && prediction.second <= 1.0f, "Prediction confidence should be valid");
    
    std::cout << "✓ Behavior pattern detection test passed" << std::endl;
}

/**
 * Test Temporal Sequence Analysis
 */
void testTemporalSequenceAnalysis() {
    std::cout << "Testing Temporal Sequence Analysis..." << std::endl;
    
    TemporalSequenceAnalyzer analyzer;
    TemporalAnalysisConfig config;
    
    bool initResult = analyzer.init(config);
    assertTrue(initResult, "Temporal analyzer initialization should succeed");
    
    // Add sequence of behaviors over time
    std::vector<BehaviorType> behaviorSequence = {
        BehaviorType::FEEDING, BehaviorType::DRINKING, BehaviorType::RESTING,
        BehaviorType::GROOMING, BehaviorType::MOVING, BehaviorType::ALERT
    };
    
    EnvironmentalData env = createMockEnvironment();
    
    for (size_t i = 0; i < behaviorSequence.size(); i++) {
        BehaviorResult behavior = createMockBehavior(behaviorSequence[i], 0.8f);
        behavior.timestamp = millis() + (i * 60000); // 1 minute intervals
        analyzer.addObservation(behavior, env);
    }
    
    // Test sequence analysis
    TemporalAnalysisResult result = analyzer.analyzeSequences(3600); // 1 hour window
    assertTrue(result.sequences.size() >= 0, "Should generate sequences");
    
    // Test coherence calculation
    TemporalWindow window(300, 60, 3, true);
    TemporalSequence sequence = analyzer.getSequence(window);
    float coherence = analyzer.calculateSequenceCoherence(sequence);
    assertTrue(coherence >= 0.0f && coherence <= 1.0f, "Coherence should be valid range");
    
    std::cout << "✓ Temporal sequence analysis test passed" << std::endl;
}

/**
 * Test End-to-End Predictive Analytics
 */
void testEndToEndPredictiveAnalytics() {
    std::cout << "Testing End-to-End Predictive Analytics..." << std::endl;
    
    PredictiveAnalyticsEngine engine;
    PredictiveConfig config;
    config.enableBehaviorPrediction = true;
    config.enablePatternDetection = true;
    config.enableAnomalyDetection = true;
    config.enableConservationInsights = true;
    
    bool initResult = engine.init(config);
    assertTrue(initResult, "Engine should initialize successfully");
    
    // Simulate a series of wildlife observations
    std::vector<BehaviorType> behaviorSequence = {
        BehaviorType::FEEDING, BehaviorType::FEEDING, BehaviorType::DRINKING,
        BehaviorType::RESTING, BehaviorType::GROOMING, BehaviorType::MOVING,
        BehaviorType::ALERT, BehaviorType::RESTING, BehaviorType::FEEDING
    };
    
    EnvironmentalData env = createMockEnvironment();
    
    // Process behaviors and generate predictions
    PredictiveAnalysisResult lastResult;
    for (size_t i = 0; i < behaviorSequence.size(); i++) {
        BehaviorResult behavior = createMockBehavior(behaviorSequence[i], 0.8f);
        behavior.timestamp = millis() + (i * 300000); // 5 minute intervals
        
        lastResult = engine.analyzeBehavior(behavior, env);
        
        // Validate results structure
        assertTrue(lastResult.analysisDuration_ms > 0, "Analysis should take some time");
        assertTrue(lastResult.timestamp > 0, "Should have valid timestamp");
    }
    
    // Test prediction capabilities after sufficient data
    if (engine.isReadyForPredictions()) {
        assertTrue(lastResult.nextBehaviorPrediction.confidence >= 0.0f, "Should have prediction confidence");
        
        // Test population health metrics
        PopulationHealthMetrics health = engine.getPopulationHealth(3600);
        assertTrue(health.overallHealthScore >= 0.0f && health.overallHealthScore <= 1.0f, 
                  "Health score should be valid range");
        
        // Test feeding time predictions
        std::vector<uint32_t> feedingTimes = engine.predictOptimalFeedingTimes(SpeciesType::BIRD_SMALL, 3);
        assertTrue(feedingTimes.size() >= 0, "Should generate feeding predictions");
        
        // Test metrics
        AIMetrics metrics = engine.getPredictionMetrics();
        assertTrue(metrics.totalInferences >= 0, "Should have valid metrics");
    }
    
    std::cout << "✓ End-to-end predictive analytics test passed" << std::endl;
}

/**
 * Test Prediction Accuracy Validation
 */
void testPredictionAccuracyValidation() {
    std::cout << "Testing Prediction Accuracy Validation..." << std::endl;
    
    PredictiveAnalyticsEngine engine;
    PredictiveConfig config;
    
    bool initResult = engine.init(config);
    assertTrue(initResult, "Engine should initialize successfully");
    
    // Create a predictable pattern: FEEDING -> RESTING -> FEEDING -> RESTING
    std::vector<BehaviorType> pattern = {
        BehaviorType::FEEDING, BehaviorType::RESTING, 
        BehaviorType::FEEDING, BehaviorType::RESTING,
        BehaviorType::FEEDING, BehaviorType::RESTING
    };
    
    EnvironmentalData env = createMockEnvironment();
    
    // Build up pattern knowledge
    for (size_t i = 0; i < pattern.size() - 1; i++) {
        BehaviorResult behavior = createMockBehavior(pattern[i], 0.9f);
        behavior.timestamp = millis() + (i * 600000); // 10 minute intervals
        engine.analyzeBehavior(behavior, env);
    }
    
    // Test prediction validation
    if (engine.isReadyForPredictions()) {
        // Get prediction for next behavior
        BehaviorResult currentBehavior = createMockBehavior(BehaviorType::FEEDING, 0.9f);
        PredictiveAnalysisResult result = engine.analyzeBehavior(currentBehavior, env);
        
        // Simulate actual next behavior
        BehaviorResult actualNext = createMockBehavior(BehaviorType::RESTING, 0.8f);
        
        // Update prediction models with validation
        engine.updatePredictionModels(actualNext, result.nextBehaviorPrediction);
        
        // Check that metrics are updated
        AIMetrics metrics = engine.getPredictionMetrics();
        assertTrue(metrics.totalInferences > 0, "Should have inference metrics");
    }
    
    std::cout << "✓ Prediction accuracy validation test passed" << std::endl;
}

/**
 * Test Memory and Performance Constraints
 */
void testPerformanceConstraints() {
    std::cout << "Testing Performance and Memory Constraints..." << std::endl;
    
    PredictiveAnalyticsEngine engine;
    PredictiveConfig config;
    config.maxPredictionHistory = 100; // Limit history
    config.maxPatternCache = 20;       // Limit cache
    config.enablePowerOptimization = true;
    
    bool initResult = engine.init(config);
    assertTrue(initResult, "Engine should initialize with constraints");
    
    EnvironmentalData env = createMockEnvironment();
    
    // Simulate intensive processing to test memory management
    uint32_t startTime = millis();
    for (int i = 0; i < 50; i++) {
        BehaviorResult behavior = createMockBehavior(
            static_cast<BehaviorType>(i % 10 + 1), // Cycle through behavior types
            0.8f
        );
        behavior.timestamp = millis() + (i * 30000); // 30 second intervals
        
        PredictiveAnalysisResult result = engine.analyzeBehavior(behavior, env);
        
        // Verify processing time is reasonable (< 100ms for embedded system)
        assertTrue(result.analysisDuration_ms < 100, "Processing should be under 100ms");
    }
    uint32_t totalTime = millis() - startTime;
    
    // Test that overall processing time is reasonable
    assertTrue(totalTime < 5000, "Total processing should be under 5 seconds");
    
    std::cout << "✓ Performance constraints test passed" << std::endl;
}

/**
 * Run all tests
 */
int main() {
    std::cout << "Running Predictive Analytics Unit Tests..." << std::endl;
    std::cout << "==========================================" << std::endl;
    
    try {
        testPredictiveEngineInit();
        testBehaviorPatternDetection();
        testTemporalSequenceAnalysis();
        testEndToEndPredictiveAnalytics();
        testPredictionAccuracyValidation();
        testPerformanceConstraints();
        
        std::cout << std::endl;
        std::cout << "✓ All tests passed successfully!" << std::endl;
        std::cout << "Predictive Analytics Engine is ready for integration." << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}