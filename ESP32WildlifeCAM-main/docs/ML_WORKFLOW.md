# 🤖 Machine Learning Workflow Guide

## Table of Contents
1. [Overview](#overview)
2. [AI Pipeline Architecture](#ai-pipeline-architecture)
3. [Wildlife Classification](#wildlife-classification)
4. [Federated Learning System](#federated-learning-system)
5. [Model Training](#model-training)
6. [Deployment and Updates](#deployment-and-updates)
7. [Performance Optimization](#performance-optimization)
8. [Monitoring and Evaluation](#monitoring-and-evaluation)

## Overview

The ESP32WildlifeCam implements a sophisticated ML workflow that combines edge AI processing with federated learning capabilities. This system enables real-time wildlife classification while continuously improving through distributed learning across multiple camera deployments.

### Key Features
- **Edge AI Processing**: Real-time species classification on ESP32
- **Federated Learning**: Privacy-preserving collaborative model improvement
- **Adaptive Models**: Environment-specific model variants
- **Continuous Learning**: Automatic model updates and refinement

## AI Pipeline Architecture

### System Components
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Image         │    │   Preprocessing │    │   Feature       │
│   Capture       │───▶│   & Enhancement │───▶│   Extraction    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Motion        │    │   Environmental │    │   Edge AI       │
│   Detection     │    │   Context       │    │   Inference     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Classification│    │   Confidence    │    │   Result        │
│   Pipeline      │───▶│   Scoring       │───▶│   Processing    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Data Flow
1. **Motion Trigger**: PIR sensor or advanced motion detection triggers capture
2. **Image Preprocessing**: Resize, normalize, and enhance captured images
3. **Feature Extraction**: Extract relevant features for classification
4. **AI Inference**: Run species classification on edge device
5. **Confidence Evaluation**: Assess classification reliability
6. **Result Processing**: Store results, trigger actions, contribute to federated learning

## Wildlife Classification

### Supported Species Categories
The system supports classification of major wildlife categories:

```cpp
enum SpeciesCategory {
    MAMMALS_LARGE,      // Deer, elk, bear, etc.
    MAMMALS_SMALL,      // Squirrels, rabbits, etc.
    BIRDS_LARGE,        // Hawks, eagles, owls
    BIRDS_SMALL,        // Songbirds, etc.
    REPTILES,           // Snakes, lizards
    UNKNOWN,            // Unclassified or low confidence
    FALSE_POSITIVE      // Non-animal triggers
};
```

### Classification Pipeline
```cpp
class WildlifeClassifier {
    // Primary classification workflow
    ClassificationResult classifyImage(uint8_t* imageData, size_t length) {
        // 1. Preprocess image
        ProcessedImage processed = preprocessImage(imageData, length);
        
        // 2. Extract features
        FeatureVector features = extractFeatures(processed);
        
        // 3. Run inference
        InferenceResult inference = runInference(features);
        
        // 4. Post-process results
        return postProcessResults(inference);
    }
};
```

### Model Architecture
The classification model uses a lightweight CNN optimized for ESP32:

```
Input Layer (224x224x3)
    ↓
Convolutional Blocks (MobileNet-based)
    ↓
Global Average Pooling
    ↓
Dense Layer (512 units)
    ↓
Dropout (0.3)
    ↓
Output Layer (Species Classes)
```

## Federated Learning System

### Architecture Overview
```cpp
class FederatedLearningSystem {
public:
    // Core federated learning functionality
    bool initialize(const FederatedConfig& config);
    bool contributeTrainingData(const TrainingData& data);
    bool requestModelUpdate();
    bool applyModelUpdate(const ModelUpdate& update);
    
    // Privacy-preserving aggregation
    LocalModel trainLocalModel(const TrainingDataset& localData);
    ModelUpdate createModelUpdate(const LocalModel& model);
    bool aggregateUpdates(const std::vector<ModelUpdate>& updates);
};
```

### Privacy-Preserving Training
The system implements differential privacy to protect sensitive data:

```cpp
class PrivacyPreservingAggregation {
    // Add noise to gradients for privacy
    ModelUpdate addDifferentialPrivacy(const ModelUpdate& update) {
        float epsilon = 1.0; // Privacy budget  
        float noise_scale = 2.0 / epsilon;
        
        for (auto& gradient : update.gradients) {
            gradient += generateGaussianNoise(0.0, noise_scale);
        }
        
        return update;
    }
};
```

### Federated Learning Workflow
1. **Local Data Collection**: Each camera collects labeled training data
2. **Local Model Training**: Train model updates on local device
3. **Privacy Protection**: Apply differential privacy to model updates
4. **Secure Aggregation**: Combine updates from multiple devices
5. **Global Model Update**: Distribute improved model to all devices
6. **Model Deployment**: Update edge AI models automatically

## Model Training

### Training Data Structure
```cpp
struct TrainingData {
    uint8_t* imageData;
    size_t imageLength;
    String speciesLabel;
    float confidence;
    EnvironmentalContext context;
    unsigned long timestamp;
    String deviceId;
};

struct EnvironmentalContext {
    float temperature;
    float humidity;
    int lightLevel;
    String location;
    String timeOfDay;
    String season;
};
```

### Local Training Process
```cpp
class LocalTrainingModule {
public:
    LocalModel trainModel(const TrainingDataset& data) {
        // 1. Data preprocessing
        auto processedData = preprocessTrainingData(data);
        
        // 2. Model initialization
        LocalModel model = initializeModel();
        
        // 3. Training loop
        for (int epoch = 0; epoch < MAX_EPOCHS; epoch++) {
            for (const auto& batch : processedData.getBatches()) {
                // Forward pass
                auto predictions = model.forward(batch.inputs);
                
                // Compute loss
                float loss = computeLoss(predictions, batch.labels);
                
                // Backward pass
                auto gradients = model.backward(loss);
                
                // Update weights
                model.updateWeights(gradients, LEARNING_RATE);
            }
        }
        
        return model;
    }
};
```

### Model Optimization for ESP32
```cpp
class EdgeOptimizer {
public:
    // Quantize model for edge deployment
    QuantizedModel quantizeModel(const Model& fullModel) {
        QuantizedModel quantized;
        
        // Convert 32-bit floats to 8-bit integers
        for (const auto& layer : fullModel.layers) {
            quantized.layers.push_back(quantizeLayer(layer));
        }
        
        return quantized;
    }
    
    // Prune unnecessary connections
    PrunedModel pruneModel(const Model& model, float threshold = 0.01) {
        PrunedModel pruned = model;
        
        for (auto& layer : pruned.layers) {
            for (auto& weight : layer.weights) {
                if (abs(weight) < threshold) {
                    weight = 0.0; // Remove small weights
                }
            }
        }
        
        return pruned;
    }
};
```

## Deployment and Updates

### Model Deployment Pipeline
```bash
# 1. Train model on cloud infrastructure
python train_wildlife_model.py --config production_config.yaml

# 2. Optimize for edge deployment
python optimize_for_edge.py --input model.h5 --output model_optimized.tflite

# 3. Generate C++ header for ESP32
python convert_to_header.py --input model_optimized.tflite --output model.h

# 4. Deploy to devices
platformio run --target upload --environment esp32cam
```

### Over-the-Air (OTA) Model Updates
```cpp
class ModelUpdateManager {
public:
    bool checkForUpdates() {
        String serverURL = "https://api.wildcam.org/models/latest";
        HTTPClient http;
        http.begin(serverURL);
        
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String response = http.getString();
            return parseUpdateResponse(response);
        }
        
        return false;
    }
    
    bool downloadAndInstallUpdate(const String& updateURL) {
        // Download new model
        if (!downloadModel(updateURL)) {
            return false;
        }
        
        // Validate model integrity
        if (!validateModel("new_model.bin")) {
            return false;
        }
        
        // Install new model
        return installModel("new_model.bin");
    }
};
```

## Performance Optimization

### Memory Optimization
```cpp
// Use memory-efficient data structures
class MemoryEfficientClassifier {
private:
    // Use quantized weights (8-bit instead of 32-bit)
    std::vector<int8_t> quantized_weights;
    
    // Reuse buffers to minimize allocations
    static uint8_t inference_buffer[INFERENCE_BUFFER_SIZE];
    
public:
    ClassificationResult classify(uint8_t* image) {
        // Minimize memory allocations during inference
        return runInferenceInPlace(image, inference_buffer);
    }
};
```

### Processing Speed Optimization
```cpp
// Use ESP32-specific optimizations
class FastInference {
    // Leverage ESP32 dual-core architecture
    void parallelInference(const ImageBatch& batch) {
        xTaskCreatePinnedToCore(
            processImageTask,   // Task function
            "ImageProcessing",  // Task name
            8192,              // Stack size
            &batch,            // Parameters
            1,                 // Priority
            nullptr,           // Task handle
            1                  // Core 1
        );
    }
};
```

### Power Optimization
```cpp
class PowerEfficientAI {
public:
    void optimizeForBattery() {
        // Reduce inference frequency when battery is low
        if (powerManager.getBatteryPercentage() < 20) {
            setInferenceInterval(30000); // 30 second intervals
        } else {
            setInferenceInterval(5000);  // 5 second intervals
        }
        
        // Use lighter models for power saving
        if (powerManager.getCurrentMode() == POWER_SAVE_MODE) {
            loadLightweightModel();
        }
    }
};
```

## Monitoring and Evaluation

### Performance Metrics
```cpp
struct ModelMetrics {
    float accuracy;
    float precision;
    float recall;
    float f1_score;
    float inference_time_ms;
    size_t memory_usage_bytes;
    float power_consumption_mw;
    unsigned long total_inferences;
    unsigned long correct_predictions;
};
```

### Continuous Evaluation
```cpp
class ModelEvaluator {
public:
    void evaluateModel() {
        ModelMetrics metrics = calculateMetrics();
        
        // Log performance data
        logMetrics(metrics);
        
        // Check if model needs retraining
        if (metrics.accuracy < MINIMUM_ACCURACY_THRESHOLD) {
            requestModelRetrain();
        }
        
        // Report to federated learning coordinator
        reportToCoordinator(metrics);
    }
    
private:
    void logMetrics(const ModelMetrics& metrics) {
        Serial.printf("Model Performance:\n");
        Serial.printf("  Accuracy: %.2f%%\n", metrics.accuracy * 100);
        Serial.printf("  Inference Time: %.1fms\n", metrics.inference_time_ms);
        Serial.printf("  Memory Usage: %zu bytes\n", metrics.memory_usage_bytes);
        Serial.printf("  Power: %.1fmW\n", metrics.power_consumption_mw);
    }
};
```

### Data Quality Assessment
```cpp
class DataQualityMonitor {
    void assessImageQuality(uint8_t* image, size_t length) {
        // Check image sharpness
        float sharpness = calculateSharpness(image);
        
        // Check brightness and contrast
        float brightness = calculateBrightness(image);
        float contrast = calculateContrast(image);
        
        // Assess noise levels
        float noiseLevel = estimateNoise(image);
        
        // Only use high-quality images for training
        if (sharpness > MIN_SHARPNESS && 
            brightness > MIN_BRIGHTNESS && 
            contrast > MIN_CONTRAST && 
            noiseLevel < MAX_NOISE) {
            markForTraining(image);
        }
    }
};
```

## Configuration Examples

### Basic ML Configuration
```cpp
// ml_config.h
#define ML_ENABLED true
#define CLASSIFICATION_THRESHOLD 0.8
#define MAX_INFERENCE_TIME_MS 500
#define FEDERATED_LEARNING_ENABLED true
#define MODEL_UPDATE_INTERVAL_HOURS 24
```

### Environment-Specific Settings
```cpp
// tropical_ml_config.h (for tropical deployments)
struct TropicalMLConfig {
    float humidity_compensation = 0.85;
    float temperature_adjustment = 1.1;
    std::vector<String> expected_species = {
        "toucan", "jaguar", "sloth", "macaw"
    };
    float confidence_threshold = 0.75; // Lower due to humid conditions
};
```

---

## Related Documentation
- [API Reference](api_reference.md) - Complete API documentation
- [Setup Guide](setup_guide.md) - Installation and configuration
- [Performance Optimization](../ai/edge_ai_optimization_guide.md) - Advanced optimization techniques
- [Federated Learning Setup](../ai/federated_learning_setup.md) - Detailed federated learning configuration