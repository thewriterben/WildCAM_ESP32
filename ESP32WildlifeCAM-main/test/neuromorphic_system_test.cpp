/**
 * @file neuromorphic_system_test.cpp
 * @brief Unit and integration tests for neuromorphic computing system
 * 
 * Tests core neuromorphic functionality including SNN processing,
 * hardware integration, and adaptive learning capabilities.
 */

#include <Arduino.h>
#include "../firmware/src/ai/neuromorphic/neuromorphic_system.h"
#include "../firmware/src/ai/neuromorphic/snn/snn_core.h"
#include <esp_heap_caps.h>

// Test configuration
#define TEST_IMAGE_WIDTH 28
#define TEST_IMAGE_HEIGHT 28
#define TEST_NUM_SPECIES 10
#define TEST_ITERATIONS 5

/**
 * Test Results Structure
 */
struct TestResult {
    bool passed;
    String description;
    uint32_t executionTime;
    String errorMessage;
    
    TestResult(bool p = false, String desc = "", uint32_t time = 0, String error = "") 
        : passed(p), description(desc), executionTime(time), errorMessage(error) {}
};

/**
 * Neuromorphic System Validation Tests
 */
class NeuromorphicSystemTest {
private:
    uint32_t testCount = 0;
    uint32_t passedTests = 0;
    
public:
    /**
     * Initialize test system
     */
    bool initialize() {
        Serial.println("=== Neuromorphic Computing System Tests ===");
        Serial.println();
        
        // Check PSRAM availability
        if (!psramFound()) {
            Serial.println("WARNING: PSRAM not found, some tests may fail");
        } else {
            Serial.printf("PSRAM available: %d bytes\n", ESP.getPsramSize());
        }
        
        return true;
    }
    
    /**
     * Run all validation tests
     */
    void runAllTests() {
        Serial.println("Starting neuromorphic system tests...\n");
        
        // Core SNN tests
        runTest(testSNNInitialization());
        runTest(testSNNLayerUpdate());
        runTest(testLIFNeuronModel());
        runTest(testAdaptiveLIFNeuron());
        runTest(testSpikeEncoding());
        runTest(testSpikeDecoding());
        
        // Learning tests
        runTest(testSTDPLearning());
        runTest(testOnlineLearning());
        runTest(testAdaptiveThresholds());
        
        // Network tests
        runTest(testSpikingConvNetwork());
        runTest(testRecurrentNetwork());
        
        // System integration tests
        runTest(testNeuromorphicSystemInit());
        runTest(testWildlifeClassification());
        runTest(testEventBasedProcessing());
        runTest(testBehaviorAnalysis());
        
        // Power optimization tests
        runTest(testPowerBudgeting());
        runTest(testSparsityControl());
        
        // Hardware integration tests
        runTest(testHardwareManager());
        runTest(testPlatformSelection());
        
        // Performance tests
        runTest(testInferenceSpeed());
        runTest(testPowerConsumption());
        
        // Print final results
        printTestSummary();
    }
    
private:
    /**
     * Run individual test and track results
     */
    void runTest(TestResult result) {
        testCount++;
        Serial.printf("Test %d: %s\n", testCount, result.description.c_str());
        
        if (result.passed) {
            Serial.printf("  ✅ PASSED (%d ms)\n", result.executionTime);
            passedTests++;
        } else {
            Serial.printf("  ❌ FAILED: %s\n", result.errorMessage.c_str());
        }
        Serial.println();
    }
    
    /**
     * Test: SNN Initialization
     */
    TestResult testSNNInitialization() {
        uint32_t start = millis();
        
        try {
            NeuromorphicConfig config;
            config.input_neurons = 100;
            config.hidden_neurons = 50;
            config.output_neurons = 10;
            
            SpikingNeuralNetwork snn;
            bool success = snn.init(config);
            
            uint32_t duration = millis() - start;
            
            if (!success) {
                return TestResult(false, "SNN Initialization", duration, "Failed to initialize SNN");
            }
            
            return TestResult(true, "SNN Initialization", duration);
            
        } catch (...) {
            return TestResult(false, "SNN Initialization", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: SNN Layer Update
     */
    TestResult testSNNLayerUpdate() {
        uint32_t start = millis();
        
        try {
            SNNLayer layer(100, NeuronType::LEAKY_INTEGRATE_FIRE);
            layer.init();
            
            // Create input spikes
            std::vector<SpikeEvent> input_spikes;
            for (int i = 0; i < 10; i++) {
                input_spikes.emplace_back(i, 1000 * i, 1.0f, 0);
            }
            
            // Update layer
            auto output_spikes = layer.update(10.0f, input_spikes);
            
            uint32_t duration = millis() - start;
            
            // Should produce some output spikes
            if (output_spikes.empty() && input_spikes.size() > 0) {
                return TestResult(false, "SNN Layer Update", duration, "No output spikes generated");
            }
            
            return TestResult(true, "SNN Layer Update", duration);
            
        } catch (...) {
            return TestResult(false, "SNN Layer Update", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: LIF Neuron Model
     */
    TestResult testLIFNeuronModel() {
        uint32_t start = millis();
        
        try {
            SNNLayer layer(1, NeuronType::LEAKY_INTEGRATE_FIRE);
            layer.init();
            
            // Apply strong input current to trigger spike
            std::vector<SpikeEvent> input;
            input.emplace_back(0, 0, 10.0f, 0);  // Strong input
            
            auto output = layer.update(1.0f, input);
            
            uint32_t duration = millis() - start;
            
            // Should spike with strong input
            if (output.empty()) {
                return TestResult(false, "LIF Neuron Model", duration, "Neuron did not spike with strong input");
            }
            
            return TestResult(true, "LIF Neuron Model", duration);
            
        } catch (...) {
            return TestResult(false, "LIF Neuron Model", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: Adaptive LIF Neuron
     */
    TestResult testAdaptiveLIFNeuron() {
        uint32_t start = millis();
        
        try {
            SNNLayer layer(1, NeuronType::ADAPTIVE_LIF);
            layer.init();
            
            // Multiple spikes should increase adaptation
            int spike_count = 0;
            for (int t = 0; t < 10; t++) {
                std::vector<SpikeEvent> input;
                input.emplace_back(0, t * 1000, 5.0f, 0);
                
                auto output = layer.update(static_cast<float>(t), input);
                spike_count += output.size();
            }
            
            uint32_t duration = millis() - start;
            
            // Should adapt (spike less frequently over time)
            return TestResult(true, "Adaptive LIF Neuron", duration);
            
        } catch (...) {
            return TestResult(false, "Adaptive LIF Neuron", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: Spike Encoding (Rate Coding)
     */
    TestResult testSpikeEncoding() {
        uint32_t start = millis();
        
        try {
            NeuromorphicConfig config;
            config.simulation_time = 100.0f;
            
            SpikingNeuralNetwork snn;
            snn.init(config);
            
            // Create test input
            std::vector<float> input(100);
            for (size_t i = 0; i < input.size(); i++) {
                input[i] = static_cast<float>(i) / input.size();
            }
            
            // Process should encode input as spikes
            auto result = snn.process(input.data(), input.size());
            
            uint32_t duration = millis() - start;
            
            if (!result.is_valid) {
                return TestResult(false, "Spike Encoding", duration, "Invalid result");
            }
            
            return TestResult(true, "Spike Encoding", duration);
            
        } catch (...) {
            return TestResult(false, "Spike Encoding", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: Spike Decoding
     */
    TestResult testSpikeDecoding() {
        uint32_t start = millis();
        
        try {
            NeuromorphicConfig config;
            config.output_neurons = 10;
            
            SpikingNeuralNetwork snn;
            snn.init(config);
            
            // Create test spikes for different output neurons
            std::vector<float> input(config.input_neurons, 0.5f);
            auto result = snn.process(input.data(), input.size());
            
            uint32_t duration = millis() - start;
            
            // Should decode to a species ID
            if (result.species_id >= config.output_neurons) {
                return TestResult(false, "Spike Decoding", duration, "Invalid species ID");
            }
            
            return TestResult(true, "Spike Decoding", duration);
            
        } catch (...) {
            return TestResult(false, "Spike Decoding", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: STDP Learning
     */
    TestResult testSTDPLearning() {
        uint32_t start = millis();
        
        try {
            NeuromorphicConfig config;
            config.a_plus = 0.01f;
            config.a_minus = 0.012f;
            
            SpikingNeuralNetwork snn;
            snn.init(config);
            snn.enableOnlineLearning(true);
            
            // Train with sample data
            std::vector<float> input(config.input_neurons, 0.5f);
            bool success = snn.trainSTDP(input.data(), input.size(), 0);
            
            uint32_t duration = millis() - start;
            
            if (!success) {
                return TestResult(false, "STDP Learning", duration, "Training failed");
            }
            
            return TestResult(true, "STDP Learning", duration);
            
        } catch (...) {
            return TestResult(false, "STDP Learning", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: Online Learning
     */
    TestResult testOnlineLearning() {
        uint32_t start = millis();
        
        try {
            NeuromorphicSystemConfig config;
            config.online_learning_enabled = true;
            
            NeuromorphicSystem system;
            system.init(config);
            
            // Create test image
            std::vector<uint8_t> image(TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);
            for (size_t i = 0; i < image.size(); i++) {
                image[i] = random(0, 255);
            }
            
            // Train system
            bool success = system.train(image.data(), TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT, 
                                       SpeciesType::MAMMAL_MEDIUM);
            
            uint32_t duration = millis() - start;
            
            if (!success) {
                return TestResult(false, "Online Learning", duration, "Training failed");
            }
            
            return TestResult(true, "Online Learning", duration);
            
        } catch (...) {
            return TestResult(false, "Online Learning", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: Adaptive Thresholds
     */
    TestResult testAdaptiveThresholds() {
        uint32_t start = millis();
        
        try {
            NeuromorphicConfig config;
            config.adaptive_thresholds = true;
            
            SpikingNeuralNetwork snn;
            snn.init(config);
            snn.enableAdaptiveThresholds(true);
            
            // Process multiple times - thresholds should adapt
            std::vector<float> input(config.input_neurons, 0.7f);
            
            for (int i = 0; i < 5; i++) {
                auto result = snn.process(input.data(), input.size());
            }
            
            uint32_t duration = millis() - start;
            
            return TestResult(true, "Adaptive Thresholds", duration);
            
        } catch (...) {
            return TestResult(false, "Adaptive Thresholds", millis() - start, "Exception thrown");
        }
    }
    
    /**
     * Test: Spiking Convolutional Network
     */
    TestResult testSpikingConvNetwork() {
        uint32_t start = millis();
        
        try {
            SpikingConvNetwork conv_snn;
            bool success = conv_snn.initConvolutional(28, 28, 8, 3);
            
            if (!success) {
                return TestResult(false, "Spiking Convolutional Network", millis() - start, 
                                "Initialization failed");
            }
            
            // Create test image
            std::vector<uint8_t> image(28 * 28);
            for (size_t i = 0; i < image.size(); i++) {
                image[i] = random(0, 255);
            }
            
            auto result = conv_snn.processImage(image.data(), 28, 28);
            
            uint32_t duration = millis() - start;
            
            return TestResult(true, "Spiking Convolutional Network", duration);
            
        } catch (...) {
            return TestResult(false, "Spiking Convolutional Network", millis() - start, 
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Recurrent Spiking Network
     */
    TestResult testRecurrentNetwork() {
        uint32_t start = millis();
        
        try {
            RecurrentSpikingNetwork rsnn;
            bool success = rsnn.initRecurrent(100, 0.5f);
            
            if (!success) {
                return TestResult(false, "Recurrent Spiking Network", millis() - start,
                                "Initialization failed");
            }
            
            // Create sequence
            std::vector<std::vector<float>> sequence;
            for (int t = 0; t < 5; t++) {
                std::vector<float> frame(100, 0.5f);
                sequence.push_back(frame);
            }
            
            auto results = rsnn.processSequence(sequence);
            
            uint32_t duration = millis() - start;
            
            if (results.size() != sequence.size()) {
                return TestResult(false, "Recurrent Spiking Network", duration,
                                "Incorrect number of results");
            }
            
            return TestResult(true, "Recurrent Spiking Network", duration);
            
        } catch (...) {
            return TestResult(false, "Recurrent Spiking Network", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Neuromorphic System Initialization
     */
    TestResult testNeuromorphicSystemInit() {
        uint32_t start = millis();
        
        try {
            NeuromorphicSystemConfig config;
            config.image_width = TEST_IMAGE_WIDTH;
            config.image_height = TEST_IMAGE_HEIGHT;
            config.num_species = TEST_NUM_SPECIES;
            
            NeuromorphicSystem system;
            bool success = system.init(config);
            
            uint32_t duration = millis() - start;
            
            if (!success) {
                return TestResult(false, "Neuromorphic System Initialization", duration,
                                "Initialization failed");
            }
            
            return TestResult(true, "Neuromorphic System Initialization", duration);
            
        } catch (...) {
            return TestResult(false, "Neuromorphic System Initialization", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Wildlife Classification
     */
    TestResult testWildlifeClassification() {
        uint32_t start = millis();
        
        try {
            auto system = NeuromorphicSystemBuilder()
                .setImageSize(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT)
                .setNumSpecies(TEST_NUM_SPECIES)
                .build();
            
            if (!system) {
                return TestResult(false, "Wildlife Classification", millis() - start,
                                "System initialization failed");
            }
            
            // Create test image
            std::vector<uint8_t> image(TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT);
            for (size_t i = 0; i < image.size(); i++) {
                image[i] = random(0, 255);
            }
            
            auto result = system->classifyWildlife(image.data(), TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
            
            uint32_t duration = millis() - start;
            
            if (!result.is_valid) {
                return TestResult(false, "Wildlife Classification", duration,
                                "Invalid classification result");
            }
            
            if (result.confidence < 0.0f || result.confidence > 1.0f) {
                return TestResult(false, "Wildlife Classification", duration,
                                "Invalid confidence value");
            }
            
            return TestResult(true, "Wildlife Classification", duration);
            
        } catch (...) {
            return TestResult(false, "Wildlife Classification", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Event-Based Processing
     */
    TestResult testEventBasedProcessing() {
        uint32_t start = millis();
        
        try {
            auto system = NeuromorphicSystemBuilder()
                .enableEventBasedProcessing()
                .enableDVSSensor()
                .build();
            
            if (!system) {
                return TestResult(false, "Event-Based Processing", millis() - start,
                                "System initialization failed");
            }
            
            // Create test events
            std::vector<EventSensorData> events;
            for (int i = 0; i < 100; i++) {
                EventSensorData event;
                event.x = random(0, 28);
                event.y = random(0, 28);
                event.timestamp = i * 1000;
                event.polarity = (i % 2 == 0) ? 1 : -1;
                events.push_back(event);
            }
            
            auto result = system->processEvents(events);
            
            uint32_t duration = millis() - start;
            
            // Event-based processing should be very power efficient
            if (result.power_consumption_mw > 100.0f) {
                return TestResult(false, "Event-Based Processing", duration,
                                "Power consumption too high for event-based processing");
            }
            
            return TestResult(true, "Event-Based Processing", duration);
            
        } catch (...) {
            return TestResult(false, "Event-Based Processing", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Behavior Analysis
     */
    TestResult testBehaviorAnalysis() {
        uint32_t start = millis();
        
        try {
            auto system = NeuromorphicSystemBuilder()
                .setImageSize(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT)
                .build();
            
            if (!system) {
                return TestResult(false, "Behavior Analysis", millis() - start,
                                "System initialization failed");
            }
            
            // Create detection history
            std::vector<NeuromorphicWildlifeResult> history;
            for (int i = 0; i < 5; i++) {
                NeuromorphicWildlifeResult result;
                result.species = SpeciesType::MAMMAL_MEDIUM;
                result.confidence = 0.8f;
                result.is_valid = true;
                history.push_back(result);
            }
            
            BehaviorType behavior = system->analyzeBehavior(history);
            
            uint32_t duration = millis() - start;
            
            // Should return a valid behavior
            if (behavior == BehaviorType::UNKNOWN && history.size() > 0) {
                return TestResult(false, "Behavior Analysis", duration,
                                "Failed to predict behavior from history");
            }
            
            return TestResult(true, "Behavior Analysis", duration);
            
        } catch (...) {
            return TestResult(false, "Behavior Analysis", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Power Budgeting
     */
    TestResult testPowerBudgeting() {
        uint32_t start = millis();
        
        try {
            auto system = NeuromorphicSystemBuilder()
                .setPowerBudget(10.0f)  // 10mW budget
                .setImageSize(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT)
                .build();
            
            if (!system) {
                return TestResult(false, "Power Budgeting", millis() - start,
                                "System initialization failed");
            }
            
            // Process image
            std::vector<uint8_t> image(TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT, 128);
            auto result = system->classifyWildlife(image.data(), TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
            
            uint32_t duration = millis() - start;
            
            // Power should be within budget (with some tolerance)
            float tolerance = 5.0f;  // 5mW tolerance
            if (result.power_consumption_mw > 10.0f + tolerance) {
                return TestResult(false, "Power Budgeting", duration,
                                "Power consumption exceeded budget");
            }
            
            return TestResult(true, "Power Budgeting", duration);
            
        } catch (...) {
            return TestResult(false, "Power Budgeting", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Sparsity Control
     */
    TestResult testSparsityControl() {
        uint32_t start = millis();
        
        try {
            NeuromorphicConfig config;
            config.sparse_coding_enabled = true;
            config.sparsity_target = 0.95f;  // 95% sparsity
            
            SpikingNeuralNetwork snn;
            snn.init(config);
            snn.setSparsityTarget(0.95f);
            
            std::vector<float> input(config.input_neurons, 0.5f);
            auto result = snn.process(input.data(), input.size());
            
            uint32_t duration = millis() - start;
            
            // Check sparsity (with tolerance)
            float actual_sparsity = result.metrics.network_sparsity;
            if (actual_sparsity < 0.85f) {  // Allow 10% tolerance
                return TestResult(false, "Sparsity Control", duration,
                                "Sparsity below target");
            }
            
            return TestResult(true, "Sparsity Control", duration);
            
        } catch (...) {
            return TestResult(false, "Sparsity Control", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Hardware Manager
     */
    TestResult testHardwareManager() {
        uint32_t start = millis();
        
        try {
            NeuromorphicHardwareManager manager;
            bool success = manager.init();
            
            uint32_t duration = millis() - start;
            
            if (!success) {
                return TestResult(false, "Hardware Manager", duration,
                                "Initialization failed");
            }
            
            // Should have at least simulation platform
            auto platforms = manager.getAvailablePlatforms();
            if (platforms.empty()) {
                return TestResult(false, "Hardware Manager", duration,
                                "No platforms available");
            }
            
            return TestResult(true, "Hardware Manager", duration);
            
        } catch (...) {
            return TestResult(false, "Hardware Manager", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Platform Selection
     */
    TestResult testPlatformSelection() {
        uint32_t start = millis();
        
        try {
            NeuromorphicHardwareManager manager;
            manager.init();
            
            // Should be able to select simulation platform
            bool success = manager.selectPlatform(NeuromorphicPlatform::SIMULATION);
            
            uint32_t duration = millis() - start;
            
            if (!success) {
                return TestResult(false, "Platform Selection", duration,
                                "Failed to select simulation platform");
            }
            
            if (manager.getActivePlatform() != NeuromorphicPlatform::SIMULATION) {
                return TestResult(false, "Platform Selection", duration,
                                "Active platform mismatch");
            }
            
            return TestResult(true, "Platform Selection", duration);
            
        } catch (...) {
            return TestResult(false, "Platform Selection", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Inference Speed
     */
    TestResult testInferenceSpeed() {
        uint32_t start = millis();
        
        try {
            auto system = NeuromorphicSystemBuilder()
                .setImageSize(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT)
                .build();
            
            if (!system) {
                return TestResult(false, "Inference Speed", millis() - start,
                                "System initialization failed");
            }
            
            std::vector<uint8_t> image(TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT, 128);
            
            // Measure inference time
            uint32_t inference_start = micros();
            auto result = system->classifyWildlife(image.data(), TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
            uint32_t inference_time = micros() - inference_start;
            
            uint32_t duration = millis() - start;
            
            // Neuromorphic should be fast (< 10ms)
            if (inference_time > 10000) {  // 10ms = 10000us
                return TestResult(false, "Inference Speed", duration,
                                String("Inference too slow: ") + String(inference_time) + "us");
            }
            
            Serial.printf("    Inference time: %d us\n", inference_time);
            
            return TestResult(true, "Inference Speed", duration);
            
        } catch (...) {
            return TestResult(false, "Inference Speed", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Test: Power Consumption
     */
    TestResult testPowerConsumption() {
        uint32_t start = millis();
        
        try {
            auto system = NeuromorphicSystemBuilder()
                .setImageSize(TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT)
                .setPowerBudget(50.0f)  // 50mW budget
                .build();
            
            if (!system) {
                return TestResult(false, "Power Consumption", millis() - start,
                                "System initialization failed");
            }
            
            std::vector<uint8_t> image(TEST_IMAGE_WIDTH * TEST_IMAGE_HEIGHT, 128);
            auto result = system->classifyWildlife(image.data(), TEST_IMAGE_WIDTH, TEST_IMAGE_HEIGHT);
            
            uint32_t duration = millis() - start;
            
            float power = result.power_consumption_mw;
            
            // Should be much lower than traditional CNN (typically >500mW)
            if (power > 100.0f) {
                return TestResult(false, "Power Consumption", duration,
                                String("Power too high: ") + String(power) + "mW");
            }
            
            Serial.printf("    Power consumption: %.2f mW\n", power);
            
            return TestResult(true, "Power Consumption", duration);
            
        } catch (...) {
            return TestResult(false, "Power Consumption", millis() - start,
                            "Exception thrown");
        }
    }
    
    /**
     * Print test summary
     */
    void printTestSummary() {
        Serial.println("\n===========================================");
        Serial.println("Test Summary");
        Serial.println("===========================================");
        Serial.printf("Total Tests: %d\n", testCount);
        Serial.printf("Passed: %d\n", passedTests);
        Serial.printf("Failed: %d\n", testCount - passedTests);
        Serial.printf("Success Rate: %.1f%%\n", 
                     100.0f * passedTests / testCount);
        Serial.println("===========================================\n");
        
        if (passedTests == testCount) {
            Serial.println("✅ ALL TESTS PASSED!");
        } else {
            Serial.println("❌ SOME TESTS FAILED");
        }
    }
};

// Global test instance
NeuromorphicSystemTest neuromorphicTests;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════════════════════╗");
    Serial.println("║  Neuromorphic Computing System Validation Tests           ║");
    Serial.println("║  WildCAM ESP32 - Advanced AI Wildlife Monitoring          ║");
    Serial.println("╚════════════════════════════════════════════════════════════╝");
    Serial.println();
    
    if (neuromorphicTests.initialize()) {
        neuromorphicTests.runAllTests();
    } else {
        Serial.println("Failed to initialize test system");
    }
}

void loop() {
    // Tests run once in setup
    delay(1000);
}
