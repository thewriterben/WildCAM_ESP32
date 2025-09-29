/**
 * @file test_edge_ai_optimizations.cpp
 * @brief Comprehensive test suite for enhanced edge AI optimizations
 * 
 * Tests all the new edge AI features including model optimization, inference
 * acceleration, power-aware processing, and advanced AI capabilities.
 */

#include <Arduino.h>
#include <unity.h>
#include "../src/ai/v2/edge_processor.h"
#include "../firmware/src/optimizations/ml_optimizer.h"

// Test configuration
const uint16_t TEST_IMAGE_WIDTH = 224;
const uint16_t TEST_IMAGE_HEIGHT = 224;
const size_t TEST_IMAGE_SIZE = TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT * 3;

// Test data
uint8_t test_image_data[TEST_IMAGE_SIZE];
EdgeProcessor* edge_processor = nullptr;
QuantizedNeuralNetwork* quantized_network = nullptr;
FastImageProcessor* fast_processor = nullptr;
AdaptiveAIPipeline* adaptive_pipeline = nullptr;
EdgeAIPerformanceMonitor* performance_monitor = nullptr;

void setUp(void) {
    // Initialize test data
    for (size_t i = 0; i < TEST_IMAGE_SIZE; i++) {
        test_image_data[i] = (i % 256); // Create test pattern
    }
    
    // Initialize edge processor
    if (!edge_processor) {
        edge_processor = new EdgeProcessor();
        
        EdgeProcessorConfig config;
        config.inputWidth = TEST_IMAGE_WIDTH;
        config.inputHeight = TEST_IMAGE_HEIGHT;
        config.confidenceThreshold = 0.7f;
        config.inferenceInterval = 1000;
        config.powerOptimizationEnabled = true;
        config.temporalFilteringEnabled = true;
        config.environmentalContextEnabled = true;
        config.speciesClassificationEnabled = true;
        config.behaviorAnalysisEnabled = true;
        
        TEST_ASSERT_TRUE(edge_processor->initialize(config));
    }
    
    // Initialize optimized components
    if (!quantized_network) {
        quantized_network = new QuantizedNeuralNetwork();
        TEST_ASSERT_TRUE(quantized_network->init(784, 10, 3));
    }
    
    if (!fast_processor) {
        fast_processor = new FastImageProcessor();
        TEST_ASSERT_TRUE(fast_processor->init(TEST_IMAGE_SIZE));
    }
    
    if (!adaptive_pipeline) {
        adaptive_pipeline = new AdaptiveAIPipeline();
        TEST_ASSERT_TRUE(adaptive_pipeline->init());
    }
    
    if (!performance_monitor) {
        performance_monitor = new EdgeAIPerformanceMonitor();
        TEST_ASSERT_TRUE(performance_monitor->init());
    }
}

void tearDown(void) {
    // Cleanup is handled by destructors
}

// ===========================
// MODEL OPTIMIZATION TESTS
// ===========================

void test_quantized_neural_network_init() {
    TEST_ASSERT_NOT_NULL(quantized_network);
    TEST_ASSERT_TRUE(quantized_network->init(224 * 224, 50, 5));
    TEST_ASSERT_GREATER_THAN(0, quantized_network->getModelSize());
}

void test_simd_optimizations() {
    if (quantized_network) {
        std::vector<float> test_input(784, 0.5f);
        
        // Test SIMD inference
        std::vector<float> result = quantized_network->inferenceSIMD(test_input);
        TEST_ASSERT_EQUAL(10, result.size());
        
        // Verify performance improvement (should be faster than standard)
        float inference_time = quantized_network->getInferenceTime();
        TEST_ASSERT_GREATER_THAN(0, inference_time);
        TEST_ASSERT_LESS_THAN(100, inference_time); // Should be under 100ms
    }
}

void test_multi_core_inference() {
    if (quantized_network) {
        std::vector<float> test_input(784, 0.3f);
        
        // Test multi-core inference
        std::vector<float> result = quantized_network->inferenceMultiCore(test_input);
        TEST_ASSERT_EQUAL(10, result.size());
        
        // Verify results are valid
        for (float val : result) {
            TEST_ASSERT_TRUE(val >= -10.0f && val <= 10.0f); // Reasonable range
        }
    }
}

void test_knowledge_distillation() {
    if (quantized_network) {
        // Create teacher network
        QuantizedNeuralNetwork teacher_network;
        TEST_ASSERT_TRUE(teacher_network.init(784, 10, 4)); // Larger teacher
        
        // Create training data
        std::vector<std::vector<float>> training_data;
        for (int i = 0; i < 10; i++) {
            std::vector<float> sample(784, (float)i / 10.0f);
            training_data.push_back(sample);
        }
        
        // Test knowledge distillation
        bool success = quantized_network->knowledgeDistillation(teacher_network, training_data);
        TEST_ASSERT_TRUE(success);
    }
}

void test_dynamic_model_selection() {
    if (quantized_network) {
        TEST_ASSERT_TRUE(quantized_network->enableDynamicModelSelection(true));
        
        // Test model selection based on battery level
        String selected_model = quantized_network->selectOptimalModel(0.2f, 0.8f);
        TEST_ASSERT_EQUAL_STRING("lightweight", selected_model.c_str());
        
        selected_model = quantized_network->selectOptimalModel(0.8f, 0.9f);
        TEST_ASSERT_EQUAL_STRING("high_accuracy", selected_model.c_str());
        
        selected_model = quantized_network->selectOptimalModel(0.6f, 0.6f);
        TEST_ASSERT_EQUAL_STRING("balanced", selected_model.c_str());
    }
}

// ===========================
// IMAGE PROCESSING TESTS
// ===========================

void test_pipeline_parallelism() {
    if (fast_processor) {
        TEST_ASSERT_TRUE(fast_processor->enablePipelineParallelism(true));
        
        uint8_t output_image[TEST_IMAGE_SIZE];
        bool success = fast_processor->processImagePipelined(
            test_image_data, output_image, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        
        TEST_ASSERT_TRUE(success);
        TEST_ASSERT_GREATER_THAN(0, fast_processor->getProcessingTime());
        TEST_ASSERT_GREATER_THAN(0, fast_processor->getPipelineEfficiency());
    }
}

void test_batch_processing() {
    if (fast_processor) {
        // Create batch of images
        std::vector<uint8_t*> image_batch;
        std::vector<uint8_t*> output_batch;
        
        const int batch_size = 3;
        uint8_t batch_outputs[batch_size][TEST_IMAGE_SIZE];
        
        for (int i = 0; i < batch_size; i++) {
            image_batch.push_back(test_image_data);
            output_batch.push_back(batch_outputs[i]);
        }
        
        bool success = fast_processor->processBatch(
            image_batch, output_batch, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        
        TEST_ASSERT_TRUE(success);
        TEST_ASSERT_EQUAL(batch_size, fast_processor->getProcessedFrameCount());
    }
}

void test_wildlife_specific_processing() {
    if (fast_processor) {
        uint8_t enhanced_image[TEST_IMAGE_SIZE];
        memcpy(enhanced_image, test_image_data, TEST_IMAGE_SIZE);
        
        // Test species-specific enhancement
        fast_processor->enhanceWildlifeFeatures(
            enhanced_image, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, "deer");
        
        // Test night vision optimization
        fast_processor->optimizeForNightVision(
            enhanced_image, TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
        
        // Verify image was modified
        bool modified = false;
        for (size_t i = 0; i < TEST_IMAGE_SIZE && !modified; i++) {
            if (enhanced_image[i] != test_image_data[i]) {
                modified = true;
            }
        }
        TEST_ASSERT_TRUE(modified);
    }
}

// ===========================
// ADAPTIVE AI PIPELINE TESTS
// ===========================

void test_edge_learning() {
    if (adaptive_pipeline) {
        TEST_ASSERT_TRUE(adaptive_pipeline->enableEdgeLearning(true));
        
        // Add training samples
        std::vector<float> features = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
        adaptive_pipeline->addTrainingSample(features, 1, 0.9f);
        adaptive_pipeline->addTrainingSample(features, 2, 0.8f);
        
        // Test local adaptation
        bool adapted = adaptive_pipeline->adaptModelToLocalPatterns();
        TEST_ASSERT_TRUE(adapted);
    }
}

void test_temporal_consistency() {
    if (adaptive_pipeline) {
        TEST_ASSERT_TRUE(adaptive_pipeline->enableTemporalConsistency(true));
        
        // Update frame history with consistent predictions
        std::vector<float> features = {0.1f, 0.2f, 0.3f};
        adaptive_pipeline->updateFrameHistory(features, 0.9f, "deer");
        adaptive_pipeline->updateFrameHistory(features, 0.8f, "deer");
        adaptive_pipeline->updateFrameHistory(features, 0.7f, "rabbit");
        
        // Test temporal smoothing
        String smoothed = adaptive_pipeline->applyTemporalSmoothing("deer", 0.6f);
        TEST_ASSERT_EQUAL_STRING("deer", smoothed.c_str());
        
        float consistency = adaptive_pipeline->calculateTemporalConsistency();
        TEST_ASSERT_GREATER_THAN(0.0f, consistency);
    }
}

void test_multimodal_processing() {
    if (adaptive_pipeline) {
        std::vector<float> visual_features = {0.1f, 0.2f, 0.3f};
        std::vector<float> audio_features = {0.4f, 0.5f};
        std::vector<float> sensor_features = {0.6f};
        
        float confidence;
        String species, behavior;
        
        bool success = adaptive_pipeline->processMultiModal(
            test_image_data, audio_features.data(), sensor_features.data(),
            confidence, species, behavior);
        
        TEST_ASSERT_TRUE(success);
        TEST_ASSERT_GREATER_THAN(0.0f, confidence);
        TEST_ASSERT_NOT_EQUAL(0, species.length());
    }
}

void test_environmental_adaptation() {
    if (adaptive_pipeline) {
        // Update environmental context
        adaptive_pipeline->updateEnvironmentalContext(
            25.0f, 60.0f, 0.7f, 14, 6); // 25°C, 60% humidity, good light, 2PM, June
        
        // Test environmental weight calculation
        float weight = adaptive_pipeline->calculateEnvironmentalWeight("deer");
        TEST_ASSERT_GREATER_THAN(0.0f, weight);
        TEST_ASSERT_LESS_OR_EQUAL(1.0f, weight);
    }
}

// ===========================
// PERFORMANCE MONITORING TESTS
// ===========================

void test_performance_monitoring() {
    if (performance_monitor) {
        // Start monitoring
        performance_monitor->startInferenceTimer();
        delay(10); // Simulate inference
        performance_monitor->endInferenceTimer(true);
        
        // Record metrics
        performance_monitor->recordMemoryUsage();
        performance_monitor->recordPowerConsumption(500.0f);
        performance_monitor->recordModelPerformance("test_model", 85.0f, 1024, false);
        performance_monitor->recordDetectionQuality(true, true, true, 0.9f);
        
        // Get metrics
        auto inference_metrics = performance_monitor->getInferenceMetrics();
        TEST_ASSERT_GREATER_THAN(0, inference_metrics.total_inferences);
        
        auto power_metrics = performance_monitor->getPowerMetrics();
        TEST_ASSERT_GREATER_THAN(0, power_metrics.average_power_consumption_mW);
        
        // Test system health
        float health = performance_monitor->calculateOverallSystemHealth();
        TEST_ASSERT_GREATER_THAN(0.0f, health);
        TEST_ASSERT_LESS_OR_EQUAL(1.0f, health);
    }
}

void test_performance_alerts() {
    if (performance_monitor) {
        performance_monitor->enablePerformanceAlerts(true);
        performance_monitor->setPerformanceThresholds(1.0f, 1000.0f, 80.0f, 1000.0f);
        
        // Trigger some performance issues
        performance_monitor->recordPowerConsumption(1500.0f); // Over threshold
        
        std::vector<String> alerts = performance_monitor->getActiveAlerts();
        TEST_ASSERT_GREATER_THAN(0, alerts.size());
    }
}

void test_optimization_recommendations() {
    if (performance_monitor) {
        std::vector<String> recommendations = performance_monitor->getOptimizationRecommendations();
        TEST_ASSERT_GREATER_OR_EQUAL(0, recommendations.size());
    }
}

// ===========================
// EDGE PROCESSOR INTEGRATION TESTS
// ===========================

void test_edge_processor_multicore() {
    if (edge_processor) {
        edge_processor->enableMultiCoreProcessing(true);
        
        // Process a frame
        WildlifeDetectionResult result = edge_processor->processImage(
            test_image_data, TEST_IMAGE_SIZE, IMAGE_FORMAT_RGB888);
        
        TEST_ASSERT_EQUAL(EDGE_ERROR_NONE, result.error);
    }
}

void test_edge_processor_dynamic_models() {
    if (edge_processor) {
        edge_processor->enableDynamicModelSelection(true);
        
        // Test model selection based on conditions
        edge_processor->selectOptimalModel(0.3f, 0.8f, "low_light");
        
        // Process with selected model
        WildlifeDetectionResult result = edge_processor->processImage(
            test_image_data, TEST_IMAGE_SIZE, IMAGE_FORMAT_RGB888);
        
        TEST_ASSERT_EQUAL(EDGE_ERROR_NONE, result.error);
    }
}

void test_edge_processor_edge_learning() {
    if (edge_processor) {
        edge_processor->enableEdgeLearning(true);
        
        // Add training sample
        edge_processor->addTrainingSample(test_image_data, "deer");
        
        // Trigger adaptation
        bool adapted = edge_processor->performLocalModelAdaptation();
        TEST_ASSERT_TRUE(adapted);
    }
}

void test_edge_processor_temporal_consistency() {
    if (edge_processor) {
        edge_processor->enableTemporalConsistency(true);
        edge_processor->setTemporalWindow(5);
        
        // Apply temporal smoothing
        String smoothed = edge_processor->applyTemporalSmoothing("deer", 0.8f);
        TEST_ASSERT_NOT_EQUAL(0, smoothed.length());
    }
}

void test_edge_processor_environmental_context() {
    if (edge_processor) {
        edge_processor->enableEnvironmentalAdaptation(true);
        edge_processor->updateEnvironmentalContext(20.0f, 65.0f, 0.6f, 10, 3);
        
        // Process image with environmental context
        WildlifeDetectionResult result = edge_processor->processImage(
            test_image_data, TEST_IMAGE_SIZE, IMAGE_FORMAT_RGB888);
        
        TEST_ASSERT_EQUAL(EDGE_ERROR_NONE, result.error);
    }
}

void test_system_health_and_alerts() {
    if (edge_processor) {
        edge_processor->enablePerformanceMonitoring(true);
        
        float health = edge_processor->getSystemHealthScore();
        TEST_ASSERT_GREATER_THAN(0.0f, health);
        
        std::vector<String> alerts = edge_processor->getPerformanceAlerts();
        TEST_ASSERT_GREATER_OR_EQUAL(0, alerts.size());
    }
}

// ===========================
// WILDLIFE DETECTION OPTIMIZER TESTS
// ===========================

void test_population_counting() {
    std::vector<BoundingBox> detections;
    
    // Create test detections
    BoundingBox box1 = {10, 10, 50, 50};
    BoundingBox box2 = {100, 100, 50, 50};
    BoundingBox box3 = {200, 200, 50, 50};
    
    detections.push_back(box1);
    detections.push_back(box2);
    detections.push_back(box3);
    
    WildlifeDetectionOptimizer::PopulationCount count = 
        WildlifeDetectionOptimizer::countIndividuals(detections, "deer");
    
    TEST_ASSERT_EQUAL(3, count.individual_count);
    TEST_ASSERT_GREATER_THAN(0.0f, count.counting_confidence);
    TEST_ASSERT_EQUAL(3, count.individual_boxes.size());
}

void test_behavior_analysis() {
    std::vector<float> movement_data = {0.1f, 0.3f, 0.2f, 0.5f, 0.4f};
    
    WildlifeDetectionOptimizer::BehaviorType behavior = 
        WildlifeDetectionOptimizer::classifyActivity(movement_data, "deer", 14);
    
    TEST_ASSERT_NOT_EQUAL(WildlifeDetectionOptimizer::BehaviorType::UNKNOWN, behavior);
}

void test_environmental_optimization() {
    WildlifeDetectionOptimizer::EnvironmentalContext context;
    context.temperature = 22.0f;
    context.humidity = 55.0f;
    context.light_level = 0.8f;
    context.wind_speed = 5.0f;
    context.precipitation = 0.0f;
    context.time_of_day = 16;
    context.month = 7;
    
    float optimization = WildlifeDetectionOptimizer::calculateEnvironmentalOptimization(
        context, "deer");
    
    TEST_ASSERT_GREATER_THAN(0.0f, optimization);
    TEST_ASSERT_LESS_OR_EQUAL(2.0f, optimization);
}

// ===========================
// MAIN TEST RUNNER
// ===========================

void setup() {
    delay(2000); // Wait for serial monitor
    
    UNITY_BEGIN();
    
    // Model optimization tests
    RUN_TEST(test_quantized_neural_network_init);
    RUN_TEST(test_simd_optimizations);
    RUN_TEST(test_multi_core_inference);
    RUN_TEST(test_knowledge_distillation);
    RUN_TEST(test_dynamic_model_selection);
    
    // Image processing tests
    RUN_TEST(test_pipeline_parallelism);
    RUN_TEST(test_batch_processing);
    RUN_TEST(test_wildlife_specific_processing);
    
    // Adaptive AI pipeline tests
    RUN_TEST(test_edge_learning);
    RUN_TEST(test_temporal_consistency);
    RUN_TEST(test_multimodal_processing);
    RUN_TEST(test_environmental_adaptation);
    
    // Performance monitoring tests
    RUN_TEST(test_performance_monitoring);
    RUN_TEST(test_performance_alerts);
    RUN_TEST(test_optimization_recommendations);
    
    // Edge processor integration tests
    RUN_TEST(test_edge_processor_multicore);
    RUN_TEST(test_edge_processor_dynamic_models);
    RUN_TEST(test_edge_processor_edge_learning);
    RUN_TEST(test_edge_processor_temporal_consistency);
    RUN_TEST(test_edge_processor_environmental_context);
    RUN_TEST(test_system_health_and_alerts);
    
    // Wildlife detection optimizer tests
    RUN_TEST(test_population_counting);
    RUN_TEST(test_behavior_analysis);
    RUN_TEST(test_environmental_optimization);
    
    UNITY_END();
}

void loop() {
    // Test complete
}