# Neuromorphic Computing Quick Start Guide

## 🚀 Get Started in 5 Minutes

### 1. Include the Header

```cpp
#include "ai/neuromorphic/neuromorphic_system.h"
```

### 2. Create and Initialize System

**Option A: Quick Setup**
```cpp
NeuromorphicSystemConfig config;
config.power_budget_mw = 10.0f;  // Ultra-low power
config.online_learning_enabled = true;

NeuromorphicSystem neuro_system;
neuro_system.init(config);
```

**Option B: Builder Pattern (Recommended)**
```cpp
auto system = NeuromorphicSystemBuilder()
    .setPowerBudget(10.0f)
    .enableOnlineLearning()
    .setImageSize(224, 224)
    .setNumSpecies(20)
    .build();
```

### 3. Process Wildlife Image

```cpp
// Classify wildlife
auto result = system->classifyWildlife(
    image_data, 
    width, 
    height
);

if (result.is_valid) {
    Serial.printf("Species: %d\n", result.species);
    Serial.printf("Confidence: %.2f\n", result.confidence);
    Serial.printf("Power: %.2f mW\n", result.power_consumption_mw);
    Serial.printf("Time: %d μs\n", result.detection_time_us);
}
```

## 💡 Common Use Cases

### Ultra-Low Power Mode (Battery < 20%)

```cpp
auto system = NeuromorphicSystemBuilder()
    .setPowerBudget(5.0f)           // 5mW budget
    .enableOnlineLearning()
    .setProcessingMode(NeuromorphicMode::SPIKE_BASED)
    .build();

// Typical consumption: 3-8mW
// Battery life: Months instead of days
```

### Event-Based Motion Detection

```cpp
auto system = NeuromorphicSystemBuilder()
    .enableEventBasedProcessing()
    .enableDVSSensor()
    .build();

// Process DVS events
std::vector<EventSensorData> events = dvs.getEvents();
auto result = system->processEvents(events);

// Consumption: <5mW, Latency: <1ms
```

### Behavior Analysis

```cpp
// Collect detection history
std::vector<NeuromorphicWildlifeResult> history;

for (int frame = 0; frame < 10; frame++) {
    auto result = system->classifyWildlife(frames[frame], w, h);
    history.push_back(result);
}

// Analyze temporal patterns
BehaviorType behavior = system->analyzeBehavior(history);
Serial.printf("Detected behavior: %d\n", behavior);
```

### Adaptive Learning

```cpp
// Enable learning
system->enableOnlineLearning(true);

// Train with observations
system->train(
    image_data, 
    width, 
    height, 
    SpeciesType::MAMMAL_MEDIUM  // Ground truth
);

// Network automatically adapts to local species
```

## 🔧 Integration with Existing AI

```cpp
#include "ai/neuromorphic_ai_integration.h"

// Initialize integration
NeuromorphicAIIntegration integration;
NeuromorphicIntegrationConfig config;
config.enabled = true;
config.auto_switching = true;
config.battery_threshold_percent = 25.0f;

integration.init(config);

// Process automatically switches modes
AIResult result = integration.processWildlifeDetection(
    frame, 
    battery_level  // Automatically uses neuromorphic if low
);
```

## ⚙️ Configuration Options

### Power Budgets

| Budget | Use Case | Typical Power |
|--------|----------|---------------|
| 5mW | Extreme power saving | 3-8mW |
| 10mW | Standard low-power | 8-15mW |
| 25mW | Balanced | 15-30mW |
| 50mW | Performance mode | 30-60mW |

### Processing Modes

```cpp
// Pure neuromorphic (lowest power)
.setProcessingMode(NeuromorphicMode::SPIKE_BASED)

// Hybrid (better accuracy)
.setProcessingMode(NeuromorphicMode::HYBRID_ANN_SNN)

// Event-driven (fastest, lowest power)
.setProcessingMode(NeuromorphicMode::EVENT_DRIVEN)
```

### Network Sparsity

```cpp
// Higher sparsity = lower power, may reduce accuracy
NeuromorphicConfig config;
config.sparsity_target = 0.95;  // 95% sparsity (default)
config.sparsity_target = 0.98;  // 98% sparsity (ultra-low power)
config.sparsity_target = 0.90;  // 90% sparsity (better accuracy)
```

## 📊 Monitoring Performance

```cpp
// Get statistics
auto stats = system->getStatistics();

Serial.printf("Inferences: %d\n", stats.total_inferences);
Serial.printf("Avg time: %.1f μs\n", stats.average_inference_time_us);
Serial.printf("Avg power: %.2f mW\n", stats.average_power_consumption_mw);
Serial.printf("Avg confidence: %.2f\n", stats.average_confidence);
Serial.printf("Sparsity: %.1f%%\n", stats.average_sparsity * 100);

// Get current metrics
auto metrics = system->getMetrics();
Serial.printf("Total spikes: %d\n", metrics.total_spikes);
Serial.printf("Active neurons: %d\n", metrics.active_neurons);
```

## 🎯 Expected Performance

### Power Consumption
- **Neuromorphic**: 5-50mW (10-100x reduction)
- **Traditional CNN**: 500-2000mW

### Inference Time
- **Neuromorphic**: 50-500 microseconds
- **Traditional CNN**: 50-200 milliseconds

### Accuracy
- **Pure SNN**: 80-90%
- **Hybrid**: 90-95%

## 🐛 Troubleshooting

### High Power Consumption
```cpp
// Increase sparsity
system->setSparsityTarget(0.98);

// Reduce power budget (forces adaptation)
system->setPowerBudget(5.0f);
```

### Low Accuracy
```cpp
// Use hybrid mode
system->enableHybridProcessing(true);

// Increase simulation time
config.network_config.simulation_time = 200.0f;

// Enable online learning
system->enableOnlineLearning(true);
```

### Slow Inference
```cpp
// Reduce simulation time
config.network_config.simulation_time = 50.0f;

// Enable hardware acceleration
system->switchHardwarePlatform(NeuromorphicPlatform::INTEL_LOIHI);
```

## 📚 Next Steps

1. **Read Full Documentation**: See [README.md](README.md)
2. **Try Examples**: Check `examples/neuromorphic_wildlife_detection/`
3. **Run Tests**: See `test/neuromorphic_system_test.cpp`
4. **Review Implementation**: See [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)

## 💻 Complete Example

```cpp
#include <Arduino.h>
#include "ai/neuromorphic/neuromorphic_system.h"

NeuromorphicSystem* system = nullptr;

void setup() {
    Serial.begin(115200);
    
    // Build system
    system = NeuromorphicSystemBuilder()
        .setPowerBudget(10.0f)
        .enableOnlineLearning()
        .setImageSize(224, 224)
        .setNumSpecies(20)
        .build()
        .release();
    
    if (!system) {
        Serial.println("Failed to initialize!");
        return;
    }
    
    Serial.println("Neuromorphic system ready!");
}

void loop() {
    if (!system) return;
    
    // Get image from camera
    uint8_t* image = captureImage();
    
    // Classify
    auto result = system->classifyWildlife(image, 224, 224);
    
    if (result.is_valid) {
        Serial.printf("Species: %d | Confidence: %.2f | Power: %.2f mW\n",
                     result.species, result.confidence, 
                     result.power_consumption_mw);
    }
    
    delay(1000);
}
```

## 🎓 Learning Resources

### Concepts
- **Spiking Neural Networks**: Brain-inspired neural networks that communicate via spikes
- **STDP**: Learning rule based on timing of pre/post-synaptic spikes
- **Event-Driven**: Only process when events occur (motion, sound, etc.)
- **Sparse Coding**: Only a small fraction of neurons active at once

### Key Benefits
1. **Ultra-Low Power**: 10-100x reduction vs traditional AI
2. **Real-Time**: Sub-millisecond latency possible
3. **Adaptive**: Learns from field observations
4. **Brain-Inspired**: Natural temporal processing

## 🤝 Support

- **Documentation**: Full docs in [README.md](README.md)
- **Examples**: See `examples/neuromorphic_wildlife_detection/`
- **Tests**: Comprehensive tests in `test/neuromorphic_system_test.cpp`
- **Issues**: Report via GitHub issues

---

**Ready to revolutionize wildlife monitoring with neuromorphic computing!** 🧠⚡🦌
