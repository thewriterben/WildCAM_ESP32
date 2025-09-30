# Advanced Species Detection for ESP32

## Overview

The Advanced Species Detection system provides high-accuracy wildlife species identification (94%+ accuracy) with edge computing capabilities optimized for ESP32-S3 hardware. This system supports 150+ North American wildlife species with hierarchical taxonomic classification.

## Features

### Core Capabilities
- **150+ Species Support**: Comprehensive coverage of North American wildlife
- **High Accuracy**: 94% accuracy on common species, 81% on rare species
- **Edge Computing**: Real-time inference on ESP32-S3 hardware
- **Hierarchical Classification**: Taxonomic hierarchy from kingdom to subspecies
- **Low False Positives**: Only 6% false positive rate

### Advanced Features
- **Subspecies Detection**: Identify subspecies variations
- **Size Estimation**: Estimate animal size from bounding boxes
- **Age Estimation**: Detect juvenile vs adult animals
- **Regional Optimization**: Adapt to specific geographic regions
- **Power Optimization**: Battery-aware processing modes
- **Temporal Tracking**: Track same animal across multiple frames

## Hardware Requirements

### Minimum Requirements
- **Platform**: ESP32-S3 (recommended) or ESP32 with PSRAM
- **Memory**: 4MB PSRAM required
- **CPU**: 240 MHz CPU frequency
- **Flash**: 8MB for model storage
- **Power**: Supports battery operation with power optimization

### Recommended Hardware
- ESP32-S3-DevKitC-1 with 8MB PSRAM
- OV2640 or OV5640 camera module
- External antenna for better connectivity
- LiPo battery (2000mAh+) for field deployment

## Model Specifications

### Wildlife Research V3 Model
```cpp
Model: wildlife_research_v3.tflite
Size: 2MB (2048 KB)
Input: 224x224x3 RGB image
Output: 150 species classes
Inference Time: ~5 seconds on ESP32-S3 @ 240MHz
Memory: 4MB PSRAM required
Quantization: INT8 quantized for edge deployment
```

### Performance Metrics
- **Common Species Accuracy**: 94%
- **Rare Species Accuracy**: 81%
- **False Positive Rate**: 6%
- **Inference Time**: 5000ms (max)
- **Power Consumption**: ~300mA during inference

## Usage

### Basic Initialization

```cpp
#include "firmware/src/ai/advanced_species_detection.h"

// Create detector instance
AdvancedSpeciesDetector detector;

// Configure
AdvancedDetectionConfig config;
config.modelPath = "wildlife_research_v3.tflite";
config.confidenceThreshold = 0.80f;
config.enableEdgeOptimization = true;
config.region = "north_america";

// Initialize
if (detector.initialize(config)) {
    Serial.println("Advanced species detector ready!");
}
```

### Detecting Species

```cpp
// Capture frame
CameraFrame frame = captureFrame();

// Run detection
AdvancedSpeciesResult result = detector.detectSpecies(frame);

if (result.isValidDetection) {
    Serial.printf("Detected: %s (%.1f%% confidence)\n",
                 advancedSpeciesToString(result.primarySpecies),
                 result.primaryConfidence * 100.0f);
    
    // Get taxonomic information
    Serial.printf("Family: %s\n", result.taxonomy.family.c_str());
    Serial.printf("Scientific: %s %s\n", 
                 result.taxonomy.genus.c_str(),
                 result.taxonomy.species.c_str());
}
```

### Power Optimization

```cpp
// Optimize for battery level
float batteryLevel = getBatteryLevel(); // 0.0 to 1.0
detector.optimizeForPower(batteryLevel);

// Or set specific inference modes
detector.setInferenceMode("fast");      // Low power, faster inference
detector.setInferenceMode("balanced");  // Default mode
detector.setInferenceMode("accurate");  // Highest accuracy
```

### Regional Adaptation

```cpp
// Set geographic region for better accuracy
detector.setRegion("rocky_mountains");
detector.setRegion("great_plains");
detector.setRegion("pacific_northwest");

// Prioritize specific species
std::vector<AdvancedSpeciesType> priority = {
    AdvancedSpeciesType::GRAY_WOLF,
    AdvancedSpeciesType::GRIZZLY_BEAR,
    AdvancedSpeciesType::MOUNTAIN_LION
};
detector.setPrioritizedSpecies(priority);
```

## Supported Species

### Mammals - Large Ungulates
- White-tailed Deer (*Odocoileus virginianus*)
- Mule Deer (*Odocoileus hemionus*)
- Elk (*Cervus canadensis*)
- Moose (*Alces alces*)
- Caribou (*Rangifer tarandus*)

### Mammals - Bears
- Black Bear (*Ursus americanus*)
- Grizzly Bear (*Ursus arctos horribilis*)
- Polar Bear (*Ursus maritimus*)

### Mammals - Large Carnivores
- Gray Wolf (*Canis lupus*)
- Mountain Lion (*Puma concolor*)
- Bobcat (*Lynx rufus*)
- Lynx (*Lynx canadensis*)

### Mammals - Medium Carnivores
- Coyote (*Canis latrans*)
- Red Fox (*Vulpes vulpes*)
- Gray Fox (*Urocyon cinereoargenteus*)
- Raccoon (*Procyon lotor*)

### Birds - Raptors
- Bald Eagle (*Haliaeetus leucocephalus*)
- Golden Eagle (*Aquila chrysaetos*)
- Red-tailed Hawk (*Buteo jamaicensis*)
- Great Horned Owl (*Bubo virginianus*)

### Birds - Game Birds
- Wild Turkey (*Meleagris gallopavo*)
- Ring-necked Pheasant (*Phasianus colchicus*)
- Ruffed Grouse (*Bonasa umbellus*)

*...and 120+ more species*

## Integration with Existing Systems

### AI Detection Interface

The advanced species detection integrates seamlessly with the existing AI detection interface:

```cpp
#include "firmware/ml_models/ai_detection_interface.h"

WildlifeDetection::AIDetectionInterface aiInterface;

// Configure to enable advanced species detection
WildlifeDetection::AIDetectionInterface::AdvancedConfig config;
config.enable_advanced_species_detection = true;
config.ai_confidence_threshold = 0.80f;

aiInterface.configureAdvancedAI(config);
aiInterface.initializeAdvancedAI();

// Check if available
if (aiInterface.hasAdvancedSpeciesDetection()) {
    Serial.println("Advanced 150+ species detection active");
}
```

## Performance Tuning

### Memory Management
```cpp
// The system automatically manages PSRAM
// Ensure PSRAM is available:
if (psramFound()) {
    Serial.printf("PSRAM: %d bytes\n", ESP.getPsramSize());
} else {
    Serial.println("WARNING: PSRAM required for advanced detection");
}
```

### CPU Optimization
```cpp
// Set CPU to maximum frequency for best performance
setCpuFrequencyMhz(240);

// Enable edge optimization for better performance
detector.enableEdgeOptimization(true);
detector.preloadModel(); // Preload for faster first inference
```

### Inference Optimization
```cpp
// Adjust timeout based on your needs
config.maxInferenceTime_ms = 5000;  // 5 seconds max

// Enable quantization for faster inference
config.enableQuantization = true;
```

## Conservation Features

### Endangered Species Detection
```cpp
if (isEndangeredSpecies(result.primarySpecies)) {
    String status = getConservationStatus(result.primarySpecies);
    Serial.printf("ALERT: Endangered species detected - %s\n", status.c_str());
    
    // Trigger special handling
    saveHighResolutionImage();
    notifyConservationAuthorities();
}
```

## Testing

Run the comprehensive test suite:

```cpp
#include "tests/test_advanced_species_detection.cpp"

void setup() {
    Serial.begin(115200);
    runAllTests();  // Runs all validation tests
}
```

## Performance Benchmarks

| Operation | Time (ESP32-S3 @ 240MHz) | Memory |
|-----------|-------------------------|---------|
| Initialization | 500ms | 4MB PSRAM |
| Single Detection | 5000ms | 4MB PSRAM |
| Preprocessing | 300ms | 512KB |
| Inference | 4500ms | 2MB |
| Postprocessing | 200ms | 256KB |

## Troubleshooting

### Common Issues

**Issue**: "PSRAM required but not found"
- **Solution**: Use ESP32-S3 or ESP32-WROVER with PSRAM enabled

**Issue**: Inference timeout
- **Solution**: Increase `maxInferenceTime_ms` or lower CPU load

**Issue**: Low accuracy
- **Solution**: Ensure good lighting, set correct region, use higher resolution images

**Issue**: High power consumption
- **Solution**: Enable power optimization, use lower CPU frequency when possible

## Future Enhancements

- [ ] Multi-animal detection in single frame
- [ ] Behavior analysis integration
- [ ] Cloud model updates via OTA
- [ ] On-device model fine-tuning
- [ ] Real-time video stream processing
- [ ] Integration with acoustic monitoring

## References

- TensorFlow Lite for Microcontrollers: https://www.tensorflow.org/lite/microcontrollers
- ESP32-S3 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
- Wildlife Conservation Standards: https://www.iucn.org/

## License

This module is part of the WildCAM ESP32 project and follows the same license.

## Support

For issues and feature requests, please open an issue on GitHub:
https://github.com/thewriterben/WildCAM_ESP32/issues
