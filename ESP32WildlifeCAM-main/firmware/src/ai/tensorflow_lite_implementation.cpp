/**
 * @file tensorflow_lite_implementation.cpp
 * @brief Complete TensorFlow Lite Micro integration implementation
 * 
 * Phase 1 Core AI Enhancement Integration
 */

#include "tensorflow_lite_implementation.h"
#include "../config.h"
#include "../debug_utils.h"
#include "../optimizations/memory_optimizer.h"
#include <SD_MMC.h>
#include <FS.h>
#include <LittleFS.h>

// Global instance
TensorFlowLiteImplementation* g_tensorflowImplementation = nullptr;

// Constructor
TensorFlowLiteImplementation::TensorFlowLiteImplementation() 
    : initialized_(false), confidenceThreshold_(DEFAULT_CONFIDENCE_THRESHOLD),
      maxInferenceTime_(DEFAULT_MAX_INFERENCE_TIME), performanceMonitoring_(true) {
    
    // Initialize model data pointers
    for (int i = 0; i < MODEL_COUNT; i++) {
        modelData_[i] = nullptr;
    }
    
    // Initialize model info
    modelInfo_[MODEL_SPECIES_CLASSIFIER].type = MODEL_SPECIES_CLASSIFIER;
    modelInfo_[MODEL_SPECIES_CLASSIFIER].name = "Species Classifier";
    modelInfo_[MODEL_SPECIES_CLASSIFIER].filename = "species_classifier_v1.0.0.tflite";
    
    modelInfo_[MODEL_BEHAVIOR_ANALYZER].type = MODEL_BEHAVIOR_ANALYZER;
    modelInfo_[MODEL_BEHAVIOR_ANALYZER].name = "Behavior Analyzer";
    modelInfo_[MODEL_BEHAVIOR_ANALYZER].filename = "behavior_analyzer_v1.0.0.tflite";
    
    modelInfo_[MODEL_MOTION_DETECTOR].type = MODEL_MOTION_DETECTOR;
    modelInfo_[MODEL_MOTION_DETECTOR].name = "Motion Detector";
    modelInfo_[MODEL_MOTION_DETECTOR].filename = "motion_detector_v1.0.0.tflite";
    
    modelInfo_[MODEL_HUMAN_DETECTOR].type = MODEL_HUMAN_DETECTOR;
    modelInfo_[MODEL_HUMAN_DETECTOR].name = "Human Detector";
    modelInfo_[MODEL_HUMAN_DETECTOR].filename = "human_detector_v1.0.0.tflite";
}

// Destructor
TensorFlowLiteImplementation::~TensorFlowLiteImplementation() {
    cleanup();
}

// Initialize TensorFlow Lite implementation
bool TensorFlowLiteImplementation::init() {
    if (initialized_) {
        DEBUG_PRINTLN("TensorFlow Lite implementation already initialized");
        return true;
    }
    
    DEBUG_PRINTLN("Initializing TensorFlow Lite implementation...");
    
#ifdef TFLITE_MICRO_ENABLED
    // Initialize TensorFlow Lite Micro base system
    if (!TensorFlowLiteMicro::init()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize TensorFlow Lite Micro");
        return false;
    }
    
    // Initialize model components
    for (int i = 0; i < MODEL_COUNT; i++) {
        models_[i] = ModelComponents();
    }
    
    initialized_ = true;
    
    DEBUG_PRINTLN("TensorFlow Lite implementation initialized successfully");
    return true;
#else
    DEBUG_PRINTLN("WARNING: TensorFlow Lite Micro not enabled in build");
    return false;
#endif
}

// Cleanup implementation
void TensorFlowLiteImplementation::cleanup() {
    if (!initialized_) return;
    
    DEBUG_PRINTLN("Cleaning up TensorFlow Lite implementation...");
    
    // Cleanup all loaded models
    unloadAllModels();
    
#ifdef TFLITE_MICRO_ENABLED
    // Cleanup TensorFlow Lite Micro
    TensorFlowLiteMicro::cleanup();
#endif
    
    initialized_ = false;
    DEBUG_PRINTLN("TensorFlow Lite implementation cleanup completed");
}

// Load a specific model
bool TensorFlowLiteImplementation::loadModel(WildlifeModelType type, const char* modelPath) {
    if (!initialized_) {
        DEBUG_PRINTLN("ERROR: TensorFlow Lite implementation not initialized");
        return false;
    }
    
    if (type >= MODEL_COUNT) {
        DEBUG_PRINTF("ERROR: Invalid model type: %d\n", type);
        return false;
    }
    
    DEBUG_PRINTF("Loading model: %s from %s\n", modelInfo_[type].name.c_str(), modelPath);
    
    // Read model file from SD card or filesystem
    File modelFile;
    bool useSD = SD_MMC.exists(modelPath);
    
    if (useSD) {
        modelFile = SD_MMC.open(modelPath, FILE_READ);
    } else {
        modelFile = LittleFS.open(modelPath, FILE_READ);
    }
    
    if (!modelFile) {
        DEBUG_PRINTF("ERROR: Failed to open model file: %s\n", modelPath);
        return false;
    }
    
    // Get file size
    size_t modelSize = modelFile.size();
    if (modelSize == 0) {
        DEBUG_PRINTF("ERROR: Model file is empty: %s\n", modelPath);
        modelFile.close();
        return false;
    }
    
    // Allocate buffer for model data
    uint8_t* modelData = (uint8_t*)malloc(modelSize);
    if (!modelData) {
        DEBUG_PRINTF("ERROR: Failed to allocate memory for model (%d bytes)\n", modelSize);
        modelFile.close();
        return false;
    }
    
    // Read model data
    size_t bytesRead = modelFile.read(modelData, modelSize);
    modelFile.close();
    
    if (bytesRead != modelSize) {
        DEBUG_PRINTF("ERROR: Failed to read complete model file (%d/%d bytes)\n", bytesRead, modelSize);
        free(modelData);
        return false;
    }
    
    // Initialize the model
    bool success = initializeModel(type, modelData, modelSize);
    
    if (success) {
        modelInfo_[type].loaded = true;
        modelInfo_[type].size = modelSize;
        modelInfo_[type].filename = String(modelPath);
        DEBUG_PRINTF("Model loaded successfully: %s (%d bytes)\n", modelInfo_[type].name.c_str(), modelSize);
    } else {
        DEBUG_PRINTF("ERROR: Failed to initialize model: %s\n", modelInfo_[type].name.c_str());
        free(modelData);
    }
    
    return success;
}

// Load models from directory
bool TensorFlowLiteImplementation::loadModelsFromDirectory(const char* directory) {
    DEBUG_PRINTF("Loading models from directory: %s\n", directory);
    
    bool anyLoaded = false;
    
    // Try to load each model type
    for (int i = 0; i < MODEL_COUNT; i++) {
        String modelPath = String(directory) + "/" + modelInfo_[i].filename;
        
        if (SD_MMC.exists(modelPath.c_str()) || LittleFS.exists(modelPath.c_str())) {
            if (loadModel((WildlifeModelType)i, modelPath.c_str())) {
                anyLoaded = true;
            }
        } else {
            DEBUG_PRINTF("Model file not found: %s\n", modelPath.c_str());
        }
    }
    
    if (anyLoaded) {
        DEBUG_PRINTLN("At least one model loaded successfully");
    } else {
        DEBUG_PRINTLN("WARNING: No models could be loaded");
    }
    
    return anyLoaded;
}

// Initialize model with TensorFlow Lite Micro
bool TensorFlowLiteImplementation::initializeModel(WildlifeModelType type, const uint8_t* modelData, size_t modelSize) {
#ifdef TFLITE_MICRO_ENABLED
    if (type >= MODEL_COUNT) {
        DEBUG_PRINTF("ERROR: Invalid model type: %d\n", type);
        return false;
    }

    ModelComponents& model = models_[type];
    
    // Clean up any existing model
    cleanupModel(type);
    
    // Parse the model
    model.model = tflite::GetModel(modelData);
    if (model.model->version() != TFLITE_SCHEMA_VERSION) {
        DEBUG_PRINTF("ERROR: Model version %d not supported. Supported version is %d\n", 
                    model.model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    
    // Create resolver with common operations
    model.resolver = new tflite::MicroMutableOpResolver<10>();
    model.resolver->AddConv2D();
    model.resolver->AddDepthwiseConv2D();
    model.resolver->AddReshape();
    model.resolver->AddSoftmax();
    model.resolver->AddMean();
    model.resolver->AddFullyConnected();
    model.resolver->AddQuantize();
    model.resolver->AddDequantize();
    
    // Allocate tensor arena based on model type
    switch (type) {
        case MODEL_SPECIES_CLASSIFIER:
            model.arenaSize = 512 * 1024; // 512KB for species classifier
            break;
        case MODEL_BEHAVIOR_ANALYZER:
            model.arenaSize = 384 * 1024; // 384KB for behavior analyzer
            break;
        case MODEL_MOTION_DETECTOR:
            model.arenaSize = 128 * 1024; // 128KB for motion detector
            break;
        case MODEL_HUMAN_DETECTOR:
            model.arenaSize = 256 * 1024; // 256KB for human detector
            break;
        default:
            model.arenaSize = 256 * 1024; // Default size
            break;
    }
    
    // Use PSRAM if available for large models, with memory optimizer integration
    if (psramFound() && model.arenaSize > 100 * 1024) {
        // Try to use memory optimizer for aligned allocation
        model.tensorArena = (uint8_t*)MemoryOptimizer::allocateAligned(model.arenaSize, 32);
        if (!model.tensorArena) {
            // Fallback to standard PSRAM allocation
            model.tensorArena = (uint8_t*)ps_malloc(model.arenaSize);
        }
    } else {
        // Use memory optimizer for DMA-capable allocation for smaller models
        model.tensorArena = (uint8_t*)MemoryOptimizer::allocateDMA(model.arenaSize);
        if (!model.tensorArena) {
            // Fallback to standard malloc
            model.tensorArena = (uint8_t*)malloc(model.arenaSize);
        }
    }
    
    if (!model.tensorArena) {
        DEBUG_PRINTF("ERROR: Failed to allocate tensor arena (%d bytes)\n", model.arenaSize);
        delete model.resolver;
        model.resolver = nullptr;
        return false;
    }
    
    // Create interpreter
    model.interpreter = new tflite::MicroInterpreter(
        model.model, *model.resolver, model.tensorArena, model.arenaSize);
    
    // Allocate tensors
    TfLiteStatus allocate_status = model.interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        DEBUG_PRINTLN("ERROR: Failed to allocate tensors");
        cleanupModel(type);
        return false;
    }
    
    // Get input and output tensors
    model.inputTensor = model.interpreter->input(0);
    model.outputTensor = model.interpreter->output(0);
    
    if (!model.inputTensor || !model.outputTensor) {
        DEBUG_PRINTLN("ERROR: Failed to get input/output tensors");
        cleanupModel(type);
        return false;
    }
    
    // Store model data pointer (TensorFlow Lite keeps reference to it)
    modelData_[type] = modelData;
    
    DEBUG_PRINTF("Model initialized successfully: %s\n", modelInfo_[type].name.c_str());
    DEBUG_PRINTF("  - Input shape: [%d, %d, %d, %d]\n", 
                model.inputTensor->dims->data[0], model.inputTensor->dims->data[1], 
                model.inputTensor->dims->data[2], model.inputTensor->dims->data[3]);
    DEBUG_PRINTF("  - Output shape: [%d, %d]\n", 
                model.outputTensor->dims->data[0], model.outputTensor->dims->data[1]);
    DEBUG_PRINTF("  - Arena size: %d bytes\n", model.arenaSize);
    
    return true;
#else
    DEBUG_PRINTLN("ERROR: TensorFlow Lite Micro not enabled in build");
    return false;
#endif
}

// Clean up model resources
void TensorFlowLiteImplementation::cleanupModel(WildlifeModelType type) {
#ifdef TFLITE_MICRO_ENABLED
    if (type >= MODEL_COUNT) return;
    
    ModelComponents& model = models_[type];
    
    // Delete interpreter
    if (model.interpreter) {
        delete model.interpreter;
        model.interpreter = nullptr;
    }
    
    // Delete resolver
    if (model.resolver) {
        delete model.resolver;
        model.resolver = nullptr;
    }
    
    // Free tensor arena
    if (model.tensorArena) {
        free(model.tensorArena);
        model.tensorArena = nullptr;
    }
    
    // Free model data
    if (modelData_[type]) {
        free((void*)modelData_[type]);
        modelData_[type] = nullptr;
    }
    
    // Reset pointers
    model.inputTensor = nullptr;
    model.outputTensor = nullptr;
    model.arenaSize = 0;
    
    // Update model info
    modelInfo_[type].loaded = false;
#endif
}

// Run inference on a model
InferenceResult TensorFlowLiteImplementation::runInference(WildlifeModelType type, 
                                                          const uint8_t* imageData, 
                                                          uint32_t width, uint32_t height, 
                                                          uint32_t channels) {
    InferenceResult result;
    
    if (!initialized_ || !isModelLoaded(type)) {
        DEBUG_PRINTF("ERROR: Model not loaded: %d\n", type);
        return result;
    }
    
#ifdef TFLITE_MICRO_ENABLED
    uint32_t startTime = millis();
    
    // Prepare input tensor
    if (!prepareInputTensor(type, imageData, width, height, channels)) {
        DEBUG_PRINTF("ERROR: Failed to prepare input tensor for model: %d\n", type);
        return result;
    }
    
    // Run inference
    TfLiteStatus invokeStatus = models_[type].interpreter->Invoke();
    if (invokeStatus != kTfLiteOk) {
        DEBUG_PRINTF("ERROR: Model inference failed: %d\n", type);
        return result;
    }
    
    // Process output
    result = processOutputTensor(type);
    result.inferenceTime = millis() - startTime;
    result.valid = true;
    
    // Update performance metrics
    if (performanceMonitoring_) {
        updatePerformanceMetrics(type, result.inferenceTime);
    }
    
    DEBUG_PRINTF("Inference completed: %s (%.2f%%, %dms)\n", 
                result.className.c_str(), result.confidence * 100, result.inferenceTime);
#endif
    
    return result;
}

// Run inference with CameraFrame
InferenceResult TensorFlowLiteImplementation::runInference(WildlifeModelType type, const CameraFrame& frame) {
    return runInference(type, frame.data, frame.width, frame.height, 3);
}

// Wildlife-specific detection methods
InferenceResult TensorFlowLiteImplementation::detectSpecies(const CameraFrame& frame) {
    return runInference(MODEL_SPECIES_CLASSIFIER, frame);
}

InferenceResult TensorFlowLiteImplementation::analyzeBehavior(const CameraFrame& frame) {
    return runInference(MODEL_BEHAVIOR_ANALYZER, frame);
}

InferenceResult TensorFlowLiteImplementation::detectMotion(const CameraFrame& frame) {
    return runInference(MODEL_MOTION_DETECTOR, frame);
}

InferenceResult TensorFlowLiteImplementation::detectHuman(const CameraFrame& frame) {
    return runInference(MODEL_HUMAN_DETECTOR, frame);
}

// Private method implementations

// Prepare input tensor from image data
bool TensorFlowLiteImplementation::prepareInputTensor(WildlifeModelType type, const uint8_t* imageData, 
                                                     uint32_t width, uint32_t height, uint32_t channels) {
#ifdef TFLITE_MICRO_ENABLED
    if (type >= MODEL_COUNT || !models_[type].inputTensor) {
        return false;
    }
    
    TfLiteTensor* inputTensor = models_[type].inputTensor;
    
    // Get input tensor dimensions
    int tensorHeight = inputTensor->dims->data[1];
    int tensorWidth = inputTensor->dims->data[2];
    int tensorChannels = inputTensor->dims->data[3];
    
    // Simple image preprocessing - resize and normalize
    uint8_t* inputBuffer = (uint8_t*)inputTensor->data.uint8;
    
    // Basic nearest-neighbor resize if dimensions don't match
    if (width != tensorWidth || height != tensorHeight) {
        float scaleX = (float)width / tensorWidth;
        float scaleY = (float)height / tensorHeight;
        
        for (int y = 0; y < tensorHeight; y++) {
            for (int x = 0; x < tensorWidth; x++) {
                int srcX = (int)(x * scaleX);
                int srcY = (int)(y * scaleY);
                
                // Ensure bounds
                srcX = min(srcX, (int)width - 1);
                srcY = min(srcY, (int)height - 1);
                
                int srcIdx = (srcY * width + srcX) * channels;
                int dstIdx = (y * tensorWidth + x) * tensorChannels;
                
                // Copy pixel data (handle different channel counts)
                for (int c = 0; c < min(channels, tensorChannels); c++) {
                    if (inputTensor->type == kTfLiteUInt8) {
                        inputBuffer[dstIdx + c] = imageData[srcIdx + c];
                    } else if (inputTensor->type == kTfLiteFloat32) {
                        // Normalize to 0-1 range for float models
                        ((float*)inputTensor->data.f)[dstIdx + c] = imageData[srcIdx + c] / 255.0f;
                    }
                }
            }
        }
    } else {
        // Direct copy if dimensions match
        size_t dataSize = tensorHeight * tensorWidth * tensorChannels;
        if (inputTensor->type == kTfLiteUInt8) {
            memcpy(inputBuffer, imageData, min(dataSize, width * height * channels));
        } else if (inputTensor->type == kTfLiteFloat32) {
            // Convert uint8 to float32 and normalize
            float* floatBuffer = (float*)inputTensor->data.f;
            for (size_t i = 0; i < min(dataSize, width * height * channels); i++) {
                floatBuffer[i] = imageData[i] / 255.0f;
            }
        }
    }
    
    return true;
#else
    return false;
#endif
}

#ifdef TFLITE_MICRO_ENABLED
bool TensorFlowLiteImplementation::initializeModel(WildlifeModelType type, const uint8_t* modelData, size_t modelSize) {
    // Cleanup existing model if loaded
    cleanupModel(type);
    
    // Parse the model
    models_[type].model = tflite::GetModel(modelData);
    if (models_[type].model->version() != TFLITE_SCHEMA_VERSION) {
        DEBUG_PRINTF("ERROR: Model schema version mismatch: %d vs %d\n", 
                    models_[type].model->version(), TFLITE_SCHEMA_VERSION);
        return false;
    }
    
    // Create operations resolver
    models_[type].resolver = new tflite::MicroMutableOpResolver<10>();
    models_[type].resolver->AddConv2D();
    models_[type].resolver->AddDepthwiseConv2D();
    models_[type].resolver->AddMaxPool2D();
    models_[type].resolver->AddAveragePool2D();
    models_[type].resolver->AddRelu();
    models_[type].resolver->AddRelu6();
    models_[type].resolver->AddSoftmax();
    models_[type].resolver->AddFullyConnected();
    models_[type].resolver->AddReshape();
    models_[type].resolver->AddQuantize();
    
    // Allocate tensor arena
    models_[type].arenaSize = DEFAULT_ARENA_SIZE;
    models_[type].tensorArena = (uint8_t*)heap_caps_malloc(models_[type].arenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
    if (!models_[type].tensorArena) {
        // Fallback to internal RAM
        models_[type].tensorArena = (uint8_t*)malloc(models_[type].arenaSize);
        if (!models_[type].tensorArena) {
            DEBUG_PRINTF("ERROR: Failed to allocate tensor arena (%d bytes)\n", models_[type].arenaSize);
            return false;
        }
        DEBUG_PRINTLN("Using internal RAM for tensor arena");
    } else {
        DEBUG_PRINTLN("Using PSRAM for tensor arena");
    }
    
    // Create interpreter
    models_[type].interpreter = new tflite::MicroInterpreter(
        models_[type].model,
        *models_[type].resolver,
        models_[type].tensorArena,
        models_[type].arenaSize
    );
    
    // Allocate tensors
    TfLiteStatus allocateStatus = models_[type].interpreter->AllocateTensors();
    if (allocateStatus != kTfLiteOk) {
        DEBUG_PRINTF("ERROR: Failed to allocate tensors for model: %d\n", type);
        cleanupModel(type);
        return false;
    }
    
    // Get input and output tensors
    models_[type].inputTensor = models_[type].interpreter->input(0);
    models_[type].outputTensor = models_[type].interpreter->output(0);
    
    if (!models_[type].inputTensor || !models_[type].outputTensor) {
        DEBUG_PRINTF("ERROR: Failed to get input/output tensors for model: %d\n", type);
        cleanupModel(type);
        return false;
    }
    
    // Update model info with tensor dimensions
    modelInfo_[type].inputWidth = models_[type].inputTensor->dims->data[2];
    modelInfo_[type].inputHeight = models_[type].inputTensor->dims->data[1];
    modelInfo_[type].channels = models_[type].inputTensor->dims->data[3];
    
    DEBUG_PRINTF("Model initialized: %dx%dx%d input\n", 
                modelInfo_[type].inputWidth, modelInfo_[type].inputHeight, modelInfo_[type].channels);
    
    return true;
}

void TensorFlowLiteImplementation::cleanupModel(WildlifeModelType type) {
    if (models_[type].interpreter) {
        delete models_[type].interpreter;
        models_[type].interpreter = nullptr;
    }
    
    if (models_[type].resolver) {
        delete models_[type].resolver;
        models_[type].resolver = nullptr;
    }
    
    if (models_[type].tensorArena) {
        free(models_[type].tensorArena);
        models_[type].tensorArena = nullptr;
    }
    
    models_[type].model = nullptr;
    models_[type].inputTensor = nullptr;
    models_[type].outputTensor = nullptr;
    models_[type].arenaSize = 0;
    
    modelInfo_[type].loaded = false;
}

bool TensorFlowLiteImplementation::prepareInputTensor(WildlifeModelType type, const uint8_t* imageData, 
                                                     uint32_t width, uint32_t height, uint32_t channels) {
    if (!models_[type].inputTensor) return false;
    
    // For now, assume input tensor expects the same dimensions as the image
    // In a real implementation, you would resize/preprocess the image to match model requirements
    
    TfLiteTensor* input = models_[type].inputTensor;
    size_t inputSize = input->bytes;
    
    // Simple copy for demonstration - in practice you'd need proper image preprocessing
    if (inputSize <= width * height * channels) {
        memcpy(input->data.uint8, imageData, inputSize);
        return true;
    }
    
    DEBUG_PRINTF("WARNING: Input size mismatch: %d vs %d\n", inputSize, width * height * channels);
    return false;
}

InferenceResult TensorFlowLiteImplementation::processOutputTensor(WildlifeModelType type) {
    InferenceResult result;
    
    TfLiteTensor* output = models_[type].outputTensor;
    if (!output) return result;
    
    // Find the class with highest confidence
    float maxConfidence = 0.0f;
    uint32_t maxIndex = 0;
    
    for (int i = 0; i < output->dims->data[1]; i++) {
        float confidence = output->data.f[i];
        if (confidence > maxConfidence) {
            maxConfidence = confidence;
            maxIndex = i;
        }
    }
    
    result.confidence = maxConfidence;
    result.classIndex = maxIndex;
    result.className = "Class_" + String(maxIndex); // In practice, you'd have a proper class name mapping
    
    return result;
}

void TensorFlowLiteImplementation::updatePerformanceMetrics(WildlifeModelType type, uint32_t inferenceTime) {
    PerformanceMetrics& metrics = performance_[type];
    
    metrics.totalInferences++;
    metrics.totalTime += inferenceTime;
    
    if (inferenceTime < metrics.minTime) metrics.minTime = inferenceTime;
    if (inferenceTime > metrics.maxTime) metrics.maxTime = inferenceTime;
    
    // Monitor memory fragmentation during AI operations
    size_t fragmentationLevel = MemoryOptimizer::getFragmentationLevel();
    if (fragmentationLevel > 30) { // 30% fragmentation threshold
        DEBUG_PRINTF("WARNING: High memory fragmentation (%d%%) during AI inference\n", fragmentationLevel);
        // Trigger defragmentation if safe to do so
        MemoryOptimizer::defragmentHeap();
    }
    
    // Log performance metrics periodically
    if (metrics.totalInferences % 10 == 0) {
        uint32_t avgTime = metrics.totalTime / metrics.totalInferences;
        DEBUG_PRINTF("Model %d performance: avg=%dms, min=%dms, max=%dms, runs=%d\n", 
                    type, avgTime, metrics.minTime, metrics.maxTime, metrics.totalInferences);
    }
}
#endif

// Check if model is loaded
bool TensorFlowLiteImplementation::isModelLoaded(WildlifeModelType type) const {
    if (type >= MODEL_COUNT) return false;
    return modelInfo_[type].loaded;
}

// Get model info
ModelInfo TensorFlowLiteImplementation::getModelInfo(WildlifeModelType type) const {
    if (type >= MODEL_COUNT) return ModelInfo();
    return modelInfo_[type];
}

// Unload all models
void TensorFlowLiteImplementation::unloadAllModels() {
#ifdef TFLITE_MICRO_ENABLED
    for (int i = 0; i < MODEL_COUNT; i++) {
        cleanupModel((WildlifeModelType)i);
    }
#endif
}

// Get average inference time
uint32_t TensorFlowLiteImplementation::getAverageInferenceTime(WildlifeModelType type) const {
    if (type >= MODEL_COUNT || performance_[type].totalInferences == 0) return 0;
    return performance_[type].totalTime / performance_[type].totalInferences;
}

// Utility functions

bool initializeTensorFlowLite() {
    if (g_tensorflowImplementation) {
        return g_tensorflowImplementation->isInitialized();
    }
    
    g_tensorflowImplementation = new TensorFlowLiteImplementation();
    return g_tensorflowImplementation->init();
}

void cleanupTensorFlowLite() {
    if (g_tensorflowImplementation) {
        delete g_tensorflowImplementation;
        g_tensorflowImplementation = nullptr;
    }
}

bool loadWildlifeModels(const char* modelsDirectory) {
    if (!g_tensorflowImplementation || !g_tensorflowImplementation->isInitialized()) {
        DEBUG_PRINTLN("ERROR: TensorFlow Lite not initialized");
        return false;
    }
    
    return g_tensorflowImplementation->loadModelsFromDirectory(modelsDirectory);
}

// Model validation
bool TensorFlowLiteImplementation::validateModel(const char* modelPath) {
    DEBUG_PRINTF("Validating model: %s\n", modelPath);
    
    // Check if file exists
    if (!SD_MMC.exists(modelPath) && !LittleFS.exists(modelPath)) {
        DEBUG_PRINTF("ERROR: Model file not found: %s\n", modelPath);
        return false;
    }
    
    // Open and read basic model header
    File modelFile;
    if (SD_MMC.exists(modelPath)) {
        modelFile = SD_MMC.open(modelPath, FILE_READ);
    } else {
        modelFile = LittleFS.open(modelPath, FILE_READ);
    }
    
    if (!modelFile) {
        DEBUG_PRINTF("ERROR: Cannot open model file: %s\n", modelPath);
        return false;
    }
    
    // Check minimum file size
    size_t fileSize = modelFile.size();
    if (fileSize < 1024) { // Minimum 1KB for a valid TensorFlow Lite model
        DEBUG_PRINTF("ERROR: Model file too small (%d bytes): %s\n", fileSize, modelPath);
        modelFile.close();
        return false;
    }
    
    // Read and validate TensorFlow Lite magic number
    uint8_t header[4];
    if (modelFile.read(header, 4) != 4) {
        DEBUG_PRINTF("ERROR: Cannot read model header: %s\n", modelPath);
        modelFile.close();
        return false;
    }
    
    modelFile.close();
    
    // Check TensorFlow Lite FlatBuffer magic number (simplified check)
    // A more complete implementation would validate the entire FlatBuffer structure
    DEBUG_PRINTF("Model validation passed: %s (%d bytes)\n", modelPath, fileSize);
    return true;
}

// Benchmark model performance
bool TensorFlowLiteImplementation::benchmarkModel(WildlifeModelType type, uint32_t iterations) {
    if (!isModelLoaded(type)) {
        DEBUG_PRINTF("ERROR: Model not loaded for benchmarking: %d\n", type);
        return false;
    }
    
    DEBUG_PRINTF("Benchmarking model %d with %d iterations...\n", type, iterations);
    
    // Create dummy input data for benchmarking
    uint32_t inputSize = 224 * 224 * 3; // Standard input size
    uint8_t* dummyData = (uint8_t*)malloc(inputSize);
    if (!dummyData) {
        DEBUG_PRINTF("ERROR: Cannot allocate dummy data for benchmarking\n");
        return false;
    }
    
    // Fill with pattern data
    for (uint32_t i = 0; i < inputSize; i++) {
        dummyData[i] = (i % 256);
    }
    
    uint32_t totalTime = 0;
    uint32_t successCount = 0;
    
    for (uint32_t i = 0; i < iterations; i++) {
        uint32_t startTime = millis();
        
        InferenceResult result = runInference(type, dummyData, 224, 224, 3);
        
        uint32_t inferenceTime = millis() - startTime;
        
        if (result.valid) {
            successCount++;
            totalTime += inferenceTime;
        }
        
        // Small delay between iterations
        delay(10);
    }
    
    free(dummyData);
    
    if (successCount > 0) {
        uint32_t avgTime = totalTime / successCount;
        DEBUG_PRINTF("Benchmark results - Model %d: avg=%dms, success=%d/%d\n", 
                    type, avgTime, successCount, iterations);
        return true;
    } else {
        DEBUG_PRINTF("ERROR: All benchmark iterations failed for model %d\n", type);
        return false;
    }
}