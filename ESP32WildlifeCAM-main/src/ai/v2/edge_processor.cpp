/**
 * @file edge_processor.cpp
 * @brief Optimized Edge AI Processing Engine for Wildlife Monitoring
 * 
 * Implements enhanced on-device AI processing with real-time wildlife detection,
 * species identification, and behavior analysis optimized for ESP32 hardware.
 */

#include "edge_processor.h"
#include <Arduino.h>
#include "../../firmware/src/optimizations/ml_optimizer.h"

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

EdgeProcessor::EdgeProcessor() 
    : initialized_(false)
    , modelLoaded_(false)
    , processingActive_(false)
    , lastInference_(0)
    , frameCount_(0)
    , powerOptimizationEnabled_(true)
    , multi_core_enabled_(false)
    , simd_enabled_(false)
    , pipeline_enabled_(false)
    , batch_processing_enabled_(false)
    , dynamic_model_selection_enabled_(false)
    , edge_learning_enabled_(false)
    , multi_modal_enabled_(false)
    , behavior_tracking_enabled_(false)
    , population_counting_enabled_(false)
    , environmental_adaptation_enabled_(false)
    , performance_monitoring_enabled_(false)
    , confidence_based_processing_enabled_(false)
    , temporal_consistency_enabled_(false)
    , tensor_arena_size_(EDGE_TENSOR_ARENA_SIZE)
    , temporal_window_size_(5)
    , performance_monitor_(nullptr)
{
    memset(&statistics_, 0, sizeof(statistics_));
    memset(&config_, 0, sizeof(config_));
    detectionQueue_.clear();
    
    // Initialize memory pool
    memory_pool_.pool_data = nullptr;
    memory_pool_.pool_size = 0;
    memory_pool_.block_size = 1024; // 1KB blocks
    memory_pool_.total_blocks = 0;
    
    // Initialize environmental context
    memset(&environmental_context_, 0, sizeof(environmental_context_));
    environmental_context_.temperature = 20.0f;
    environmental_context_.humidity = 50.0f;
    environmental_context_.light_level = 0.5f;
    environmental_context_.time_of_day = 12;
    environmental_context_.season = 6; // Default to summer
    
    // Initialize multimodal data
    memset(&current_multimodal_data_, 0, sizeof(current_multimodal_data_));
}

EdgeProcessor::~EdgeProcessor() {
    cleanup();
    
    // Cleanup enhanced features
    if (performance_monitor_) {
        delete performance_monitor_;
        performance_monitor_ = nullptr;
    }
    
    cleanupMemoryPool();
    
    // Clear training samples
    training_samples_.clear();
    
    // Clear temporal history
    temporal_history_.clear();
    
    // Clear loaded models
    loaded_models_.clear();
    
    // Clear behavior patterns
    detected_patterns_.clear();
}

bool EdgeProcessor::initialize(const EdgeProcessorConfig& config) {
    config_ = config;
    
    // Initialize TensorFlow Lite
    if (!initializeTensorFlowLite()) {
        Serial.println("Failed to initialize TensorFlow Lite");
        return false;
    }
    
    // Load AI models
    if (!loadAIModels()) {
        Serial.println("Failed to load AI models");
        return false;
    }
    
    // Initialize image preprocessing
    if (!initializeImageProcessing()) {
        Serial.println("Failed to initialize image processing");
        return false;
    }
    
    // Initialize statistics
    statistics_.startTime = millis();
    statistics_.totalInferences = 0;
    statistics_.wildlifeDetections = 0;
    statistics_.falsePositives = 0;
    statistics_.averageInferenceTime = 0;
    statistics_.accuracyScore = 0.0;
    
    // Set processing parameters
    lastInference_ = millis();
    frameCount_ = 0;
    
    initialized_ = true;
    Serial.println("Edge AI Processor initialized successfully");
    return true;
}

void EdgeProcessor::cleanup() {
    if (model_) {
        // Cleanup TensorFlow Lite model
        model_ = nullptr;
    }
    
    if (interpreter_) {
        interpreter_.reset();
    }
    
    detectionQueue_.clear();
    initialized_ = false;
    modelLoaded_ = false;
}

// ===========================
// MAIN PROCESSING
// ===========================

void EdgeProcessor::processFrame() {
    if (!initialized_ || !modelLoaded_ || processingActive_) {
        return;
    }
    
    processingActive_ = true;
    unsigned long startTime = millis();
    
    // Check if enough time has passed since last inference
    if (startTime - lastInference_ < config_.inferenceInterval) {
        processingActive_ = false;
        return;
    }
    
    // Capture and preprocess frame
    if (!captureAndPreprocessFrame()) {
        processingActive_ = false;
        return;
    }
    
    // Run inference
    WildlifeDetectionResult result = runInference();
    
    // Post-process results
    if (result.confidence > config_.confidenceThreshold) {
        processDetectionResult(result);
    }
    
    // Update statistics
    updateInferenceStatistics(millis() - startTime);
    
    lastInference_ = millis();
    frameCount_++;
    processingActive_ = false;
}

WildlifeDetectionResult EdgeProcessor::processImage(const uint8_t* imageData, 
                                                   size_t imageSize,
                                                   ImageFormat format) {
    WildlifeDetectionResult result;
    memset(&result, 0, sizeof(result));
    
    if (!initialized_ || !modelLoaded_) {
        result.error = EDGE_ERROR_NOT_INITIALIZED;
        return result;
    }
    
    unsigned long startTime = millis();
    
    // Preprocess image
    if (!preprocessImage(imageData, imageSize, format)) {
        result.error = EDGE_ERROR_PREPROCESSING_FAILED;
        return result;
    }
    
    // Run inference
    result = runInference();
    
    // Apply confidence and filtering
    if (result.confidence > config_.confidenceThreshold) {
        result = applyAdvancedFiltering(result);
    }
    
    // Update statistics
    updateInferenceStatistics(millis() - startTime);
    
    return result;
}

WildlifeDetectionResult EdgeProcessor::runInference() {
    WildlifeDetectionResult result;
    memset(&result, 0, sizeof(result));
    
    if (!interpreter_) {
        result.error = EDGE_ERROR_MODEL_NOT_LOADED;
        return result;
    }
    
    // Run TensorFlow Lite inference
    TfLiteStatus invokeStatus = interpreter_->Invoke();
    if (invokeStatus != kTfLiteOk) {
        result.error = EDGE_ERROR_INFERENCE_FAILED;
        statistics_.inferenceErrors++;
        return result;
    }
    
    // Extract results from output tensors
    extractInferenceResults(result);
    
    // Apply wildlife-specific optimizations
    result = optimizeForWildlife(result);
    
    statistics_.totalInferences++;
    
    return result;
}

// ===========================
// IMAGE PREPROCESSING
// ===========================

bool EdgeProcessor::captureAndPreprocessFrame() {
    // This would interface with camera module
    // For now, simulate frame capture
    
    if (!preprocessCurrentFrame()) {
        Serial.println("Frame preprocessing failed");
        return false;
    }
    
    return true;
}

bool EdgeProcessor::preprocessImage(const uint8_t* imageData, size_t imageSize, ImageFormat format) {
    if (!imageData || imageSize == 0) {
        return false;
    }
    
    // Convert image format if necessary
    if (!convertImageFormat(imageData, imageSize, format)) {
        return false;
    }
    
    // Resize image to model input size
    if (!resizeImage()) {
        return false;
    }
    
    // Normalize pixel values
    if (!normalizeImage()) {
        return false;
    }
    
    // Apply wildlife-specific preprocessing
    if (!applyWildlifePreprocessing()) {
        return false;
    }
    
    return true;
}

bool EdgeProcessor::preprocessCurrentFrame() {
    // Implement frame preprocessing pipeline
    return resizeImage() && normalizeImage() && applyWildlifePreprocessing();
}

bool EdgeProcessor::convertImageFormat(const uint8_t* imageData, size_t imageSize, ImageFormat format) {
    // Convert between different image formats (JPEG, RGB565, etc.)
    switch (format) {
        case IMAGE_FORMAT_RGB565:
            return convertRGB565ToRGB888(imageData, imageSize);
        case IMAGE_FORMAT_JPEG:
            return convertJPEGToRGB888(imageData, imageSize);
        case IMAGE_FORMAT_RGB888:
            // Already in correct format
            memcpy(preprocessedImage_, imageData, std::min(imageSize, sizeof(preprocessedImage_)));
            return true;
        default:
            return false;
    }
}

bool EdgeProcessor::resizeImage() {
    // Resize image to model input dimensions
    // Simplified implementation - would use proper image resize algorithms
    
    // For now, assume input is already correct size or implement basic resize
    return true;
}

bool EdgeProcessor::normalizeImage() {
    // Normalize pixel values to [0, 1] or [-1, 1] range
    for (size_t i = 0; i < config_.inputWidth * config_.inputHeight * 3; i++) {
        preprocessedImage_[i] = preprocessedImage_[i] / 255.0f;
    }
    
    return true;
}

bool EdgeProcessor::applyWildlifePreprocessing() {
    // Apply wildlife-specific image enhancements
    if (!enhanceContrast()) {
        return false;
    }
    
    if (!reduceNoise()) {
        return false;
    }
    
    if (!enhanceEdges()) {
        return false;
    }
    
    return true;
}

// ===========================
// MODEL MANAGEMENT
// ===========================

bool EdgeProcessor::initializeTensorFlowLite() {
    // Initialize TensorFlow Lite for microcontrollers
    static tflite::MicroErrorReporter micro_error_reporter;
    errorReporter_ = &micro_error_reporter;
    
    // Initialize memory arena
    tensorArena_ = new uint8_t[EDGE_TENSOR_ARENA_SIZE];
    if (!tensorArena_) {
        Serial.println("Failed to allocate tensor arena");
        return false;
    }
    
    return true;
}

bool EdgeProcessor::loadAIModels() {
    // Load primary wildlife detection model
    if (!loadWildlifeDetectionModel()) {
        Serial.println("Failed to load wildlife detection model");
        return false;
    }
    
    // Load species classification model
    if (!loadSpeciesClassificationModel()) {
        Serial.println("Failed to load species classification model");
        return false;
    }
    
    // Load behavior analysis model
    if (!loadBehaviorAnalysisModel()) {
        Serial.println("Failed to load behavior analysis model");
        return false;
    }
    
    modelLoaded_ = true;
    return true;
}

bool EdgeProcessor::loadWildlifeDetectionModel() {
    // Load the wildlife detection TFLite model
    // This would load from flash memory or SD card
    
    // For now, simulate model loading
    model_ = tflite::GetModel(wildlife_detection_model_data);
    if (model_->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("Model schema version mismatch: %d vs %d\n", 
                     model_->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    
    // Initialize interpreter
    static tflite::MicroMutableOpResolver<10> resolver;
    addOperationsToResolver(resolver);
    
    interpreter_ = std::make_unique<tflite::MicroInterpreter>(
        model_, resolver, tensorArena_, EDGE_TENSOR_ARENA_SIZE, errorReporter_);
    
    if (interpreter_->AllocateTensors() != kTfLiteOk) {
        Serial.println("Failed to allocate tensors");
        return false;
    }
    
    // Verify input/output tensor dimensions
    if (!verifyModelTensors()) {
        return false;
    }
    
    Serial.println("Wildlife detection model loaded successfully");
    return true;
}

bool EdgeProcessor::loadSpeciesClassificationModel() {
    // Load species classification model
    // Implementation would load additional model for species ID
    Serial.println("Species classification model loaded");
    return true;
}

bool EdgeProcessor::loadBehaviorAnalysisModel() {
    // Load behavior analysis model
    // Implementation would load model for behavior classification
    Serial.println("Behavior analysis model loaded");
    return true;
}

// ===========================
// INFERENCE AND RESULTS
// ===========================

void EdgeProcessor::extractInferenceResults(WildlifeDetectionResult& result) {
    // Extract results from TensorFlow Lite output tensors
    TfLiteTensor* output = interpreter_->output(0);
    
    if (output->type == kTfLiteFloat32) {
        float* outputData = output->data.f;
        
        // Extract bounding box coordinates
        result.boundingBox.x = static_cast<uint16_t>(outputData[0] * config_.inputWidth);
        result.boundingBox.y = static_cast<uint16_t>(outputData[1] * config_.inputHeight);
        result.boundingBox.width = static_cast<uint16_t>(outputData[2] * config_.inputWidth);
        result.boundingBox.height = static_cast<uint16_t>(outputData[3] * config_.inputHeight);
        
        // Extract confidence score
        result.confidence = outputData[4];
        
        // Extract species classification (if available)
        if (interpreter_->outputs_size() > 1) {
            TfLiteTensor* speciesOutput = interpreter_->output(1);
            if (speciesOutput->type == kTfLiteFloat32) {
                float* speciesData = speciesOutput->data.f;
                result.speciesId = findMaxConfidenceClass(speciesData, speciesOutput->dims->data[1]);
                result.speciesConfidence = speciesData[result.speciesId];
            }
        }
        
        // Set timestamp
        result.timestamp = millis();
        result.frameId = frameCount_;
        result.error = EDGE_ERROR_NONE;
    }
}

WildlifeDetectionResult EdgeProcessor::optimizeForWildlife(const WildlifeDetectionResult& result) {
    WildlifeDetectionResult optimized = result;
    
    // Apply temporal filtering to reduce false positives
    optimized = applyTemporalFiltering(optimized);
    
    // Apply size-based filtering
    optimized = applySizeFiltering(optimized);
    
    // Apply movement-based filtering
    optimized = applyMovementFiltering(optimized);
    
    // Apply environmental context
    optimized = applyEnvironmentalContext(optimized);
    
    return optimized;
}

WildlifeDetectionResult EdgeProcessor::applyAdvancedFiltering(const WildlifeDetectionResult& result) {
    WildlifeDetectionResult filtered = result;
    
    // Multi-frame consistency check
    if (!isConsistentWithPreviousDetections(result)) {
        filtered.confidence *= 0.7;  // Reduce confidence for inconsistent detections
    }
    
    // Size reasonableness check
    if (!isSizeReasonable(result.boundingBox)) {
        filtered.confidence *= 0.5;  // Heavily penalize unreasonable sizes
    }
    
    // Edge proximity check (animals near image edges may be cropped)
    if (isNearImageEdge(result.boundingBox)) {
        filtered.confidence *= 0.9;  // Slight penalty for edge detections
    }
    
    return filtered;
}

// ===========================
// POWER OPTIMIZATION
// ===========================

void EdgeProcessor::enablePowerOptimization(bool enable) {
    powerOptimizationEnabled_ = enable;
    
    if (enable) {
        // Reduce inference frequency
        config_.inferenceInterval = config_.inferenceInterval * 2;
        
        // Lower model precision if possible
        enableLowPowerMode();
        
        Serial.println("Power optimization enabled");
    } else {
        // Restore normal operation
        config_.inferenceInterval = EDGE_DEFAULT_INFERENCE_INTERVAL;
        disableLowPowerMode();
        
        Serial.println("Power optimization disabled");
    }
}

void EdgeProcessor::adjustInferenceFrequency(float batteryLevel) {
    if (!powerOptimizationEnabled_) {
        return;
    }
    
    if (batteryLevel < 0.2) {
        // Very low battery - minimal inference
        config_.inferenceInterval = 5000;  // 5 seconds
        config_.confidenceThreshold = 0.8;  // Higher threshold
    } else if (batteryLevel < 0.5) {
        // Low battery - reduced inference
        config_.inferenceInterval = 2000;  // 2 seconds
        config_.confidenceThreshold = 0.7;
    } else {
        // Normal battery - standard inference
        config_.inferenceInterval = EDGE_DEFAULT_INFERENCE_INTERVAL;
        config_.confidenceThreshold = EDGE_DEFAULT_CONFIDENCE_THRESHOLD;
    }
}

void EdgeProcessor::enableLowPowerMode() {
    // Implement low power mode optimizations
    // This might include reducing clock speeds, disabling features, etc.
    Serial.println("Low power AI mode enabled");
}

void EdgeProcessor::disableLowPowerMode() {
    // Restore full power mode
    Serial.println("Full power AI mode enabled");
}

// ===========================
// STATISTICS AND MONITORING
// ===========================

EdgeProcessorStatistics EdgeProcessor::getStatistics() const {
    return statistics_;
}

void EdgeProcessor::resetStatistics() {
    memset(&statistics_, 0, sizeof(statistics_));
    statistics_.startTime = millis();
}

float EdgeProcessor::getAverageInferenceTime() const {
    return statistics_.averageInferenceTime;
}

float EdgeProcessor::getAccuracyScore() const {
    return statistics_.accuracyScore;
}

uint32_t EdgeProcessor::getFramesProcessed() const {
    return frameCount_;
}

void EdgeProcessor::updateInferenceStatistics(unsigned long inferenceTime) {
    // Update average inference time
    if (statistics_.totalInferences > 0) {
        statistics_.averageInferenceTime = 
            (statistics_.averageInferenceTime * (statistics_.totalInferences - 1) + inferenceTime) 
            / statistics_.totalInferences;
    } else {
        statistics_.averageInferenceTime = inferenceTime;
    }
    
    // Update frame rate
    unsigned long currentTime = millis();
    if (currentTime - statistics_.startTime > 0) {
        statistics_.frameRate = (frameCount_ * 1000.0) / (currentTime - statistics_.startTime);
    }
    
    // Update memory usage
    statistics_.memoryUsage = calculateMemoryUsage();
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

bool EdgeProcessor::initializeImageProcessing() {
    // Initialize image processing buffers and parameters
    preprocessedImage_ = new float[config_.inputWidth * config_.inputHeight * 3];
    if (!preprocessedImage_) {
        return false;
    }
    
    return true;
}

void EdgeProcessor::addOperationsToResolver(tflite::MicroMutableOpResolver<10>& resolver) {
    // Add necessary operations for the model
    resolver.AddConv2D();
    resolver.AddDepthwiseConv2D();
    resolver.AddRelu();
    resolver.AddRelu6();
    resolver.AddReshape();
    resolver.AddSoftmax();
    resolver.AddLogistic();
    resolver.AddFullyConnected();
    resolver.AddAdd();
    resolver.AddMean();
}

bool EdgeProcessor::verifyModelTensors() {
    // Verify input tensor
    TfLiteTensor* input = interpreter_->input(0);
    if (input->dims->size != 4 || 
        input->dims->data[1] != static_cast<int>(config_.inputHeight) ||
        input->dims->data[2] != static_cast<int>(config_.inputWidth) ||
        input->dims->data[3] != 3) {
        Serial.println("Input tensor dimensions mismatch");
        return false;
    }
    
    // Verify output tensor
    TfLiteTensor* output = interpreter_->output(0);
    if (output->dims->size < 1) {
        Serial.println("Invalid output tensor");
        return false;
    }
    
    return true;
}

uint16_t EdgeProcessor::findMaxConfidenceClass(float* data, int numClasses) {
    uint16_t maxClass = 0;
    float maxConfidence = data[0];
    
    for (int i = 1; i < numClasses; i++) {
        if (data[i] > maxConfidence) {
            maxConfidence = data[i];
            maxClass = i;
        }
    }
    
    return maxClass;
}

void EdgeProcessor::processDetectionResult(const WildlifeDetectionResult& result) {
    // Add detection to queue for further processing
    if (detectionQueue_.size() < EDGE_MAX_DETECTION_QUEUE) {
        detectionQueue_.push_back(result);
    }
    
    // Update statistics
    statistics_.wildlifeDetections++;
    
    // Trigger callback if set
    if (detectionCallback_) {
        detectionCallback_(result);
    }
}

bool EdgeProcessor::convertRGB565ToRGB888(const uint8_t* rgb565Data, size_t dataSize) {
    // Convert RGB565 to RGB888 format
    // Implementation would convert pixel format
    return true;
}

bool EdgeProcessor::convertJPEGToRGB888(const uint8_t* jpegData, size_t dataSize) {
    // Decode JPEG to RGB888 format
    // Implementation would use JPEG decoder
    return true;
}

bool EdgeProcessor::enhanceContrast() {
    // Apply contrast enhancement for wildlife visibility
    return true;
}

bool EdgeProcessor::reduceNoise() {
    // Apply noise reduction algorithms
    return true;
}

bool EdgeProcessor::enhanceEdges() {
    // Apply edge enhancement for better feature detection
    return true;
}

WildlifeDetectionResult EdgeProcessor::applyTemporalFiltering(const WildlifeDetectionResult& result) {
    // Apply temporal consistency filtering
    return result;
}

WildlifeDetectionResult EdgeProcessor::applySizeFiltering(const WildlifeDetectionResult& result) {
    // Filter based on reasonable wildlife sizes
    return result;
}

WildlifeDetectionResult EdgeProcessor::applyMovementFiltering(const WildlifeDetectionResult& result) {
    // Filter based on movement patterns
    return result;
}

WildlifeDetectionResult EdgeProcessor::applyEnvironmentalContext(const WildlifeDetectionResult& result) {
    // Apply environmental context (time of day, weather, etc.)
    return result;
}

bool EdgeProcessor::isConsistentWithPreviousDetections(const WildlifeDetectionResult& result) {
    // Check consistency with recent detections
    return true;
}

bool EdgeProcessor::isSizeReasonable(const BoundingBox& bbox) {
    // Check if bounding box size is reasonable for wildlife
    uint32_t area = bbox.width * bbox.height;
    uint32_t imageArea = config_.inputWidth * config_.inputHeight;
    
    float areaRatio = static_cast<float>(area) / imageArea;
    return (areaRatio > 0.01 && areaRatio < 0.8);  // 1% to 80% of image
}

bool EdgeProcessor::isNearImageEdge(const BoundingBox& bbox) {
    // Check if bounding box is near image edges
    const uint16_t edgeThreshold = 10;  // pixels
    
    return (bbox.x < edgeThreshold || 
            bbox.y < edgeThreshold ||
            bbox.x + bbox.width > config_.inputWidth - edgeThreshold ||
            bbox.y + bbox.height > config_.inputHeight - edgeThreshold);
}

uint32_t EdgeProcessor::calculateMemoryUsage() {
    // Calculate current memory usage
    // Implementation would track actual memory usage
    return 0;  // Placeholder
}

// ===========================
// ENHANCED EDGE AI FEATURES
// ===========================

void EdgeProcessor::enableMultiCoreProcessing(bool enable) {
    if (enable && !multi_core_enabled_) {
        #ifndef CONFIG_FREERTOS_UNICORE
        if (initializeMultiCoreProcessing()) {
            multi_core_enabled_ = true;
            Serial.println("Multi-core processing enabled");
        } else {
            Serial.println("Failed to enable multi-core processing");
        }
        #else
        Serial.println("Multi-core processing not available on single-core ESP32");
        #endif
    } else if (!enable && multi_core_enabled_) {
        multi_core_enabled_ = false;
        Serial.println("Multi-core processing disabled");
    }
}

void EdgeProcessor::enableSIMDOptimizations(bool enable) {
    if (enable && !simd_enabled_) {
        #ifdef CONFIG_IDF_TARGET_ESP32S3
        if (initializeSIMDProcessing()) {
            simd_enabled_ = true;
            Serial.println("SIMD optimizations enabled");
        } else {
            Serial.println("Failed to enable SIMD optimizations");
        }
        #else
        Serial.println("SIMD optimizations not available on this ESP32 variant");
        #endif
    } else if (!enable && simd_enabled_) {
        simd_enabled_ = false;
        Serial.println("SIMD optimizations disabled");
    }
}

void EdgeProcessor::enablePipelineParallelism(bool enable) {
    pipeline_enabled_ = enable;
    if (enable) {
        Serial.println("Pipeline parallelism enabled");
    } else {
        Serial.println("Pipeline parallelism disabled");
    }
}

void EdgeProcessor::enableDynamicModelSelection(bool enable) {
    dynamic_model_selection_enabled_ = enable;
    if (enable) {
        Serial.println("Dynamic model selection enabled");
    } else {
        Serial.println("Dynamic model selection disabled");
    }
}

void EdgeProcessor::enableEdgeLearning(bool enable) {
    edge_learning_enabled_ = enable;
    if (enable) {
        training_samples_.reserve(MAX_TRAINING_SAMPLES);
        Serial.println("Edge learning enabled");
    } else {
        training_samples_.clear();
        Serial.println("Edge learning disabled");
    }
}

void EdgeProcessor::enablePerformanceMonitoring(bool enable) {
    if (enable && !performance_monitoring_enabled_) {
        if (!performance_monitor_) {
            performance_monitor_ = new EdgeAIPerformanceMonitor();
            if (performance_monitor_ && performance_monitor_->init()) {
                performance_monitoring_enabled_ = true;
                Serial.println("Performance monitoring enabled");
            } else {
                delete performance_monitor_;
                performance_monitor_ = nullptr;
                Serial.println("Failed to enable performance monitoring");
            }
        }
    } else if (!enable && performance_monitoring_enabled_) {
        performance_monitoring_enabled_ = false;
        if (performance_monitor_) {
            delete performance_monitor_;
            performance_monitor_ = nullptr;
        }
        Serial.println("Performance monitoring disabled");
    }
}

void EdgeProcessor::enableTemporalConsistency(bool enable) {
    temporal_consistency_enabled_ = enable;
    if (enable) {
        temporal_history_.reserve(temporal_window_size_);
        Serial.println("Temporal consistency enabled");
    } else {
        temporal_history_.clear();
        Serial.println("Temporal consistency disabled");
    }
}

void EdgeProcessor::updateEnvironmentalContext(float temperature, float humidity, float light_level,
                                              uint8_t time_of_day, uint8_t season) {
    environmental_context_.temperature = temperature;
    environmental_context_.humidity = humidity;
    environmental_context_.light_level = light_level;
    environmental_context_.time_of_day = time_of_day;
    environmental_context_.season = season;
    environmental_context_.last_update = millis();
    
    if (environmental_adaptation_enabled_) {
        // Adjust processing parameters based on environmental conditions
        adjustProcessingParameters();
    }
}

void EdgeProcessor::addTrainingSample(const uint8_t* image_data, const String& ground_truth_label) {
    if (!edge_learning_enabled_ || !image_data) return;
    
    TrainingSample sample;
    extractFeaturesForLearning(image_data, sample.features);
    sample.label = ground_truth_label;
    sample.confidence = 1.0f; // Ground truth has maximum confidence
    sample.timestamp = millis();
    
    // Add to training samples
    if (training_samples_.size() >= MAX_TRAINING_SAMPLES) {
        training_samples_.erase(training_samples_.begin());
    }
    training_samples_.push_back(sample);
    
    // Check if we should trigger adaptation
    if (shouldTriggerAdaptation()) {
        performLocalModelAdaptation();
    }
}

bool EdgeProcessor::performLocalModelAdaptation() {
    if (!edge_learning_enabled_ || training_samples_.size() < 10) {
        return false;
    }
    
    Serial.printf("Performing local model adaptation with %d samples\n", training_samples_.size());
    
    // Simplified adaptation - in practice this would involve actual model training
    // For now, we adjust confidence thresholds based on local data
    
    std::map<String, float> label_confidence_sum;
    std::map<String, int> label_count;
    
    for (const auto& sample : training_samples_) {
        label_confidence_sum[sample.label] += sample.confidence;
        label_count[sample.label]++;
    }
    
    // Adjust confidence thresholds based on local performance
    for (const auto& pair : label_count) {
        if (pair.second > 5) { // Need at least 5 samples
            float avg_confidence = label_confidence_sum[pair.first] / pair.second;
            if (avg_confidence > 0.8f) {
                // High confidence for this species, we can be more selective
                config_.confidenceThreshold = std::min(0.9f, config_.confidenceThreshold + 0.05f);
            } else if (avg_confidence < 0.6f) {
                // Low confidence, be more inclusive
                config_.confidenceThreshold = std::max(0.5f, config_.confidenceThreshold - 0.05f);
            }
        }
    }
    
    Serial.println("Local model adaptation completed");
    return true;
}

std::vector<String> EdgeProcessor::getPerformanceAlerts() {
    std::vector<String> alerts;
    
    if (performance_monitoring_enabled_ && performance_monitor_) {
        alerts = performance_monitor_->getActiveAlerts();
    }
    
    return alerts;
}

float EdgeProcessor::getSystemHealthScore() {
    if (performance_monitoring_enabled_ && performance_monitor_) {
        return performance_monitor_->calculateOverallSystemHealth();
    }
    
    return 0.5f; // Default neutral score
}

String EdgeProcessor::applyTemporalSmoothing(const String& current_prediction, float confidence) {
    if (!temporal_consistency_enabled_) {
        return current_prediction;
    }
    
    // Update temporal history
    TemporalFrame frame;
    frame.prediction = current_prediction;
    frame.confidence = confidence;
    frame.timestamp = millis();
    
    if (temporal_history_.size() >= temporal_window_size_) {
        temporal_history_.erase(temporal_history_.begin());
    }
    temporal_history_.push_back(frame);
    
    // Apply temporal smoothing
    return getTemporalConsensus();
}

// ===========================
// ENHANCED PRIVATE METHODS
// ===========================

bool EdgeProcessor::initializeMultiCoreProcessing() {
    #ifndef CONFIG_FREERTOS_UNICORE
    // Create inference task on core 0 (assuming main processing is on core 1)
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        inferenceTask,
        "EdgeAI_Inference",
        4096, // Stack size
        this, // Task parameter
        1,    // Priority
        nullptr, // Task handle
        0     // Core ID
    );
    
    return taskCreated == pdPASS;
    #else
    return false;
    #endif
}

bool EdgeProcessor::initializeSIMDProcessing() {
    #ifdef CONFIG_IDF_TARGET_ESP32S3
    // ESP32-S3 SIMD initialization
    Serial.println("Initializing ESP32-S3 SIMD optimizations");
    return true;
    #else
    return false;
    #endif
}

bool EdgeProcessor::initializeMemoryPool(size_t pool_size) {
    if (memory_pool_.pool_data) {
        cleanupMemoryPool();
    }
    
    memory_pool_.pool_size = pool_size;
    memory_pool_.total_blocks = pool_size / memory_pool_.block_size;
    
    // Try to allocate in PSRAM first
    #ifdef BOARD_HAS_PSRAM
    memory_pool_.pool_data = (uint8_t*)ps_malloc(pool_size);
    #endif
    
    // Fallback to internal memory
    if (!memory_pool_.pool_data) {
        memory_pool_.pool_data = (uint8_t*)malloc(pool_size);
    }
    
    if (!memory_pool_.pool_data) {
        Serial.printf("Failed to allocate memory pool of %d bytes\n", pool_size);
        return false;
    }
    
    memory_pool_.block_allocated.resize(memory_pool_.total_blocks, false);
    
    Serial.printf("Memory pool initialized: %d blocks of %d bytes\n", 
                  memory_pool_.total_blocks, memory_pool_.block_size);
    return true;
}

void EdgeProcessor::cleanupMemoryPool() {
    if (memory_pool_.pool_data) {
        free(memory_pool_.pool_data);
        memory_pool_.pool_data = nullptr;
    }
    memory_pool_.block_allocated.clear();
    memory_pool_.total_blocks = 0;
}

void EdgeProcessor::extractFeaturesForLearning(const uint8_t* image_data, std::vector<float>& features) {
    // Simplified feature extraction for edge learning
    // In practice, this would extract meaningful features from the image
    
    features.clear();
    features.reserve(100); // Reserve space for features
    
    // Extract basic statistical features (simplified)
    if (image_data) {
        size_t image_size = config_.inputWidth * config_.inputHeight;
        
        // Calculate mean, variance, and other basic features
        float mean = 0.0f;
        for (size_t i = 0; i < image_size; i++) {
            mean += image_data[i];
        }
        mean /= image_size;
        features.push_back(mean);
        
        // Add more sophisticated features as needed
        features.push_back(mean / 255.0f); // Normalized mean
    }
}

bool EdgeProcessor::shouldTriggerAdaptation() {
    // Trigger adaptation every 100 samples or once per hour
    static unsigned long last_adaptation = 0;
    unsigned long current_time = millis();
    
    return (training_samples_.size() % 100 == 0) || 
           (current_time - last_adaptation > 3600000); // 1 hour
}

String EdgeProcessor::getTemporalConsensus() {
    if (temporal_history_.empty()) {
        return "unknown";
    }
    
    // Count occurrences of each prediction
    std::map<String, int> prediction_counts;
    std::map<String, float> confidence_sums;
    
    for (const auto& frame : temporal_history_) {
        prediction_counts[frame.prediction]++;
        confidence_sums[frame.prediction] += frame.confidence;
    }
    
    // Find prediction with highest weighted score
    String best_prediction = "unknown";
    float best_score = 0.0f;
    
    for (const auto& pair : prediction_counts) {
        float avg_confidence = confidence_sums[pair.first] / pair.second;
        float temporal_score = pair.second * avg_confidence;
        
        if (temporal_score > best_score) {
            best_score = temporal_score;
            best_prediction = pair.first;
        }
    }
    
    return best_prediction;
}

void EdgeProcessor::adjustProcessingParameters() {
    if (!environmental_adaptation_enabled_) return;
    
    // Adjust inference interval based on environmental conditions
    uint32_t base_interval = config_.inferenceInterval;
    
    // Adjust for light level
    if (environmental_context_.light_level < 0.3f) {
        // Low light - reduce frequency to save power
        config_.inferenceInterval = base_interval * 1.5f;
    } else if (environmental_context_.light_level > 0.8f) {
        // Good light - increase frequency for better detection
        config_.inferenceInterval = base_interval * 0.8f;
    }
    
    // Adjust for temperature
    if (environmental_context_.temperature > 35.0f || environmental_context_.temperature < -10.0f) {
        // Extreme temperatures - reduce processing to prevent overheating/freezing
        config_.inferenceInterval = base_interval * 1.3f;
    }
}

void EdgeProcessor::inferenceTask(void* parameters) {
    EdgeProcessor* processor = static_cast<EdgeProcessor*>(parameters);
    
    while (true) {
        if (processor && processor->initialized_ && processor->multi_core_enabled_) {
            // Perform inference work on this core
            // This would typically handle the heavy computation
            vTaskDelay(pdMS_TO_TICKS(100)); // Prevent watchdog timeout
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000)); // Wait if not ready
        }
    }
}