#include "wildlife_classifier.h"
#include "../utils/logger.h"
#include <esp_heap_caps.h>

bool WildlifeClassifier::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing wildlife classifier...");

    // Allocate tensor arena for TensorFlow Lite
    tensorArena = heap_caps_malloc(arenaSize, MALLOC_CAP_8BIT);
    if (tensorArena == nullptr) {
        LOG_ERROR("Failed to allocate tensor arena");
        return false;
    }

    // Load TensorFlow Lite model
    if (!loadModel()) {
        LOG_ERROR("Failed to load classification model");
        cleanup();
        return false;
    }

    // Initialize statistics
    resetStatistics();

    initialized = true;
    enabled = true;

    LOG_INFO("Wildlife classifier initialized successfully");
    LOG_INFO("Arena size: " + String(arenaSize) + " bytes");
    LOG_INFO("Confidence threshold: " + String(confidenceThreshold, 2));

    return true;
}

WildlifeClassifier::ClassificationResult WildlifeClassifier::classifyFrame(camera_fb_t* frame) {
    ClassificationResult result = {};
    result.isValid = false;

    if (!initialized || !enabled || frame == nullptr) {
        return result;
    }

    uint32_t startTime = millis();

    // For demonstration, use simplified classification
    // In a real implementation, this would decode JPEG and run TensorFlow Lite inference
    result = performSimplifiedClassification(frame->buf, frame->len);
    
    result.inferenceTime = millis() - startTime;
    updateStatistics(result);

    if (result.isValid) {
        LOG_DEBUG("Species classified: " + result.speciesName + 
                 " (confidence: " + String(result.confidence, 2) + ")");
    }

    return result;
}

WildlifeClassifier::ClassificationResult WildlifeClassifier::classifyImage(const uint8_t* imageData, 
                                                                          size_t imageSize, 
                                                                          uint16_t width, 
                                                                          uint16_t height) {
    ClassificationResult result = {};
    result.isValid = false;

    if (!initialized || !enabled || imageData == nullptr) {
        return result;
    }

    uint32_t startTime = millis();

    // Simplified classification for demonstration
    result = performSimplifiedClassification(imageData, imageSize);
    
    result.inferenceTime = millis() - startTime;
    updateStatistics(result);

    return result;
}

void WildlifeClassifier::setConfidenceThreshold(float threshold) {
    confidenceThreshold = constrain(threshold, 0.0f, 1.0f);
    LOG_INFO("Confidence threshold set to: " + String(confidenceThreshold, 2));
}

void WildlifeClassifier::resetStatistics() {
    stats = {};
    LOG_INFO("Classifier statistics reset");
}

String WildlifeClassifier::getSpeciesName(SpeciesType species) {
    switch (species) {
        case SpeciesType::WHITE_TAILED_DEER: return "White-tailed Deer";
        case SpeciesType::BLACK_BEAR: return "Black Bear";
        case SpeciesType::RED_FOX: return "Red Fox";
        case SpeciesType::GRAY_WOLF: return "Gray Wolf";
        case SpeciesType::MOUNTAIN_LION: return "Mountain Lion";
        case SpeciesType::ELK: return "Elk";
        case SpeciesType::MOOSE: return "Moose";
        case SpeciesType::RACCOON: return "Raccoon";
        case SpeciesType::COYOTE: return "Coyote";
        case SpeciesType::BOBCAT: return "Bobcat";
        case SpeciesType::WILD_TURKEY: return "Wild Turkey";
        case SpeciesType::BALD_EAGLE: return "Bald Eagle";
        case SpeciesType::RED_TAILED_HAWK: return "Red-tailed Hawk";
        case SpeciesType::GREAT_BLUE_HERON: return "Great Blue Heron";
        case SpeciesType::HUMAN: return "Human";
        default: return "Unknown";
    }
}

String WildlifeClassifier::getConfidenceLevelDescription(ConfidenceLevel level) {
    switch (level) {
        case ConfidenceLevel::VERY_LOW: return "Very Low";
        case ConfidenceLevel::LOW: return "Low";
        case ConfidenceLevel::MEDIUM: return "Medium";
        case ConfidenceLevel::HIGH: return "High";
        case ConfidenceLevel::VERY_HIGH: return "Very High";
        default: return "Unknown";
    }
}

bool WildlifeClassifier::isDangerousSpecies(SpeciesType species) {
    switch (species) {
        case SpeciesType::BLACK_BEAR:
        case SpeciesType::GRAY_WOLF:
        case SpeciesType::MOUNTAIN_LION:
        case SpeciesType::COYOTE:
            return true;
        default:
            return false;
    }
}

void WildlifeClassifier::cleanup() {
    if (tensorArena != nullptr) {
        heap_caps_free(tensorArena);
        tensorArena = nullptr;
    }

    if (modelBuffer != nullptr) {
        heap_caps_free(modelBuffer);
        modelBuffer = nullptr;
    }

    initialized = false;
    LOG_INFO("Wildlife classifier cleaned up");
}

bool WildlifeClassifier::loadModel() {
    // In a real implementation, this would:
    // 1. Load the TensorFlow Lite model from SD card or flash
    // 2. Initialize the TensorFlow Lite interpreter
    // 3. Allocate input/output tensors
    
    LOG_INFO("Loading wildlife classification model...");
    
    // Placeholder implementation
    // Real model loading would use TensorFlow Lite Micro APIs
    modelBuffer = heap_caps_malloc(512 * 1024, MALLOC_CAP_8BIT); // 512KB model
    if (modelBuffer == nullptr) {
        LOG_ERROR("Failed to allocate model buffer");
        return false;
    }

    LOG_INFO("Model loaded successfully (placeholder)");
    return true;
}

bool WildlifeClassifier::preprocessImage(const uint8_t* imageData, size_t imageSize, float* inputTensor) {
    // In a real implementation, this would:
    // 1. Decode JPEG image
    // 2. Resize to model input size (e.g., 224x224)
    // 3. Normalize pixel values to 0-1 range
    // 4. Convert to RGB format if needed
    
    // Placeholder preprocessing
    if (inputTensor == nullptr) {
        return false;
    }

    // Fill tensor with normalized dummy data
    size_t tensorSize = MODEL_INPUT_SIZE * MODEL_INPUT_SIZE * 3; // RGB
    for (size_t i = 0; i < tensorSize; i++) {
        inputTensor[i] = 0.5f; // Normalized gray
    }

    return true;
}

bool WildlifeClassifier::runInference(const float* inputTensor, float* outputTensor) {
    // In a real implementation, this would:
    // 1. Set input tensor data
    // 2. Run TensorFlow Lite inference
    // 3. Get output tensor data
    
    if (inputTensor == nullptr || outputTensor == nullptr) {
        return false;
    }

    // Placeholder inference - just copy some dummy probabilities
    for (int i = 0; i < MAX_SPECIES_COUNT; i++) {
        outputTensor[i] = 0.02f; // Low probability for all species
    }

    // Set higher probability for a random species (demonstration)
    uint32_t randomSpecies = (millis() % 15) + 1; // 1-15
    outputTensor[randomSpecies] = 0.8f;

    return true;
}

WildlifeClassifier::ClassificationResult WildlifeClassifier::processOutput(const float* outputTensor) {
    ClassificationResult result = {};
    
    if (outputTensor == nullptr) {
        return result;
    }

    // Find species with highest confidence
    float maxConfidence = 0.0f;
    int maxIndex = 0;
    
    for (int i = 0; i < MAX_SPECIES_COUNT; i++) {
        if (outputTensor[i] > maxConfidence) {
            maxConfidence = outputTensor[i];
            maxIndex = i;
        }
    }

    result.species = static_cast<SpeciesType>(maxIndex);
    result.confidence = maxConfidence;
    result.confidenceLevel = scoreToConfidenceLevel(maxConfidence);
    result.speciesName = getSpeciesName(result.species);
    result.animalCount = 1; // Simplified - assume single animal
    result.isValid = (maxConfidence >= confidenceThreshold);

    return result;
}

WildlifeClassifier::ConfidenceLevel WildlifeClassifier::scoreToConfidenceLevel(float confidence) {
    if (confidence >= 0.9f) return ConfidenceLevel::VERY_HIGH;
    if (confidence >= 0.75f) return ConfidenceLevel::HIGH;
    if (confidence >= 0.6f) return ConfidenceLevel::MEDIUM;
    if (confidence >= 0.4f) return ConfidenceLevel::LOW;
    return ConfidenceLevel::VERY_LOW;
}

void WildlifeClassifier::updateStatistics(const ClassificationResult& result) {
    stats.totalInferences++;
    
    if (result.isValid) {
        stats.successfulInferences++;
        
        // Update species count
        int speciesIndex = static_cast<int>(result.species);
        if (speciesIndex >= 0 && speciesIndex < 51) {
            stats.speciesDetectionCounts[speciesIndex]++;
        }
        
        // Update average confidence
        stats.averageConfidence = ((stats.averageConfidence * (stats.successfulInferences - 1)) + 
                                  result.confidence) / stats.successfulInferences;
    }
    
    // Update average inference time
    stats.averageInferenceTime = ((stats.averageInferenceTime * (stats.totalInferences - 1)) + 
                                 result.inferenceTime) / stats.totalInferences;
}

WildlifeClassifier::ClassificationResult WildlifeClassifier::performSimplifiedClassification(const uint8_t* imageData, 
                                                                                            size_t imageSize) {
    ClassificationResult result = {};
    
    // Simplified classification based on image characteristics
    // This is for demonstration - real implementation would use TensorFlow Lite
    
    uint32_t dataSum = 0;
    for (size_t i = 0; i < min(imageSize, (size_t)1000); i += 10) {
        dataSum += imageData[i];
    }
    
    // Use data characteristics to simulate species detection
    uint8_t speciesHint = (dataSum % 15) + 1;
    float confidence = 0.6f + (dataSum % 30) / 100.0f; // 0.6 to 0.89
    
    result.species = static_cast<SpeciesType>(speciesHint);
    result.confidence = confidence;
    result.confidenceLevel = scoreToConfidenceLevel(confidence);
    result.speciesName = getSpeciesName(result.species);
    result.animalCount = 1;
    result.isValid = (confidence >= confidenceThreshold);
    
    // Set bounding box (simplified)
    if (result.isValid) {
        result.boundingBoxes[0][0] = 0.2f; // x
        result.boundingBoxes[0][1] = 0.2f; // y
        result.boundingBoxes[0][2] = 0.6f; // width
        result.boundingBoxes[0][3] = 0.6f; // height
    }
    
    return result;
}