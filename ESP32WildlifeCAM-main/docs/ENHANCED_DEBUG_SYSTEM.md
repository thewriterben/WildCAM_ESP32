# Enhanced Debug System for ESP32 Wildlife Camera

The ESP32 Wildlife Camera now includes a comprehensive enhanced debug system that provides sophisticated debugging capabilities for complex embedded systems. This system is designed specifically for wildlife camera deployments where robust debugging is critical for field troubleshooting.

## 🎯 Key Features

### Categorized Logging
- **Component-specific debug levels**: Set different debug verbosity for camera, WiFi, LoRa, AI, power management, etc.
- **10 debug categories**: System, Camera, Motion, Power, WiFi, LoRa, AI, Memory, Sensor, Network
- **5 debug levels**: None, Error, Warning, Info, Debug, Verbose

### Timestamped Output
- **Precise timing**: All debug messages include millisecond timestamps
- **Performance analysis**: Easily identify timing issues and bottlenecks
- **Chronological tracking**: Follow the sequence of events during system operation

### Performance Monitoring
- **Timer tracking**: Measure execution time of critical operations
- **Memory monitoring**: Track heap usage, fragmentation, and PSRAM utilization
- **System health**: Monitor CPU temperature, stack usage, and task states

### Runtime Configuration
- **Serial commands**: Change debug levels without reflashing firmware
- **Debug presets**: Quick configuration for different deployment scenarios
- **Live monitoring**: Real-time system information and performance metrics

## 🚀 Quick Start

### Basic Usage

The enhanced debug system is fully backward compatible with existing `DEBUG_PRINT`, `DEBUG_PRINTLN`, and `DEBUG_PRINTF` macros. Simply include the new headers:

```cpp
#include "debug_utils.h"
#include "debug_config.h"

void setup() {
    Serial.begin(115200);
    EnhancedDebugSystem::init();
    
    // Your existing debug statements still work
    DEBUG_PRINTLN("System starting...");
    
    // New categorized debug statements
    DEBUG_CAMERA_INFO("Camera initialized");
    DEBUG_POWER_WARN("Battery low: %.1fV", batteryVoltage);
}
```

### Categorized Debug Macros

Each subsystem now has dedicated debug macros:

```cpp
// Camera debugging
DEBUG_CAMERA_ERROR("Camera initialization failed");
DEBUG_CAMERA_WARN("Low light conditions detected");
DEBUG_CAMERA_INFO("Image captured: %dx%d", width, height);
DEBUG_CAMERA_DEBUG("Camera settings: ISO=%d, exposure=%dms", iso, exposure);

// Motion detection
DEBUG_MOTION_INFO("Motion detected at sector %d", sector);
DEBUG_MOTION_DEBUG("PIR sensor reading: %d", pirValue);

// Power management
DEBUG_POWER_ERROR("Critical battery level: %.2fV", voltage);
DEBUG_POWER_INFO("Solar charging: %.1fW", power);

// LoRa networking
DEBUG_LORA_INFO("Message sent to node %d", nodeId);
DEBUG_LORA_DEBUG("RSSI: %d dBm, SNR: %.1f", rssi, snr);

// AI processing
DEBUG_AI_INFO("Species detected: %s (%.1f%% confidence)", species, confidence);
DEBUG_AI_DEBUG("Inference time: %dms", inferenceTime);
```

### Performance Timing

Measure execution times of critical operations:

```cpp
void captureAndAnalyzeImage() {
    DEBUG_TIMER_START("image_capture");
    camera_fb_t* fb = esp_camera_fb_get();
    DEBUG_TIMER_END("image_capture");
    
    DEBUG_TIMER_START("ai_analysis");
    AnalysisResult result = analyzeImage(fb);
    DEBUG_TIMER_END("ai_analysis");
    
    // Print timing summary
    DEBUG_TIMER_PRINT();
}
```

### Memory Monitoring

Track memory usage and system health:

```cpp
void checkSystemHealth() {
    DEBUG_PRINT_MEMORY();  // Detailed memory information
    DEBUG_PRINT_SYSTEM();  // System information (CPU, chip info, etc.)
    DEBUG_PRINT_NETWORK(); // Network status
    DEBUG_PRINT_TASKS();   // Task and stack information
}
```

## 🎛️ Runtime Configuration

The debug system can be configured at runtime using serial commands:

### Debug Commands

| Command | Description | Example |
|---------|-------------|---------|
| `debug status` | Show current debug configuration | `debug status` |
| `debug level <category> <level>` | Set debug level for category | `debug level cam 4` |
| `debug global <level>` | Set debug level for all categories | `debug global 2` |
| `debug preset <preset>` | Apply debug preset | `debug preset dev` |
| `debug memory` | Show memory information | `debug memory` |
| `debug system` | Show system information | `debug system` |
| `debug timers` | Show performance timers | `debug timers` |

### Categories

| Category | Code | Description |
|----------|------|-------------|
| System | `sys` | General system messages |
| Camera | `cam` | Camera operations |
| Motion | `mot` | Motion detection |
| Power | `pwr` | Power management |
| WiFi | `wifi` | WiFi networking |
| LoRa | `lora` | LoRa mesh networking |
| AI | `ai` | AI/ML processing |
| Memory | `mem` | Memory management |
| Sensor | `sen` | Sensor readings |
| Network | `net` | Network operations |

### Debug Levels

| Level | Name | Code | Description |
|-------|------|------|-------------|
| 0 | None | `none` | No debug output |
| 1 | Error | `error` | Critical errors only |
| 2 | Warning | `warn` | Warnings and errors |
| 3 | Info | `info` | General information |
| 4 | Debug | `debug` | Detailed debugging |
| 5 | Verbose | `verbose` | Maximum verbosity |

## 📋 Debug Presets

Predefined configurations for different scenarios:

### Development Mode (`debug preset dev`)
- **All categories**: Debug level (4)
- **Features**: Full timestamps, categories, memory info
- **Use case**: Software development and testing

### Production Mode (`debug preset prod`)
- **All categories**: Warning level (2)
- **Features**: Minimal overhead, essential info only
- **Use case**: Deployed wildlife cameras

### Field Mode (`debug preset field`) - Default
- **Most categories**: Info level (3)
- **WiFi/Network**: Warning level (2)
- **Features**: Balanced debugging for troubleshooting
- **Use case**: Field deployment and maintenance

### Performance Mode (`debug preset perf`)
- **Most categories**: Error level (1)
- **Memory/System**: Info level (3)
- **Features**: Focus on performance metrics
- **Use case**: Performance optimization and testing

### Silent Mode (`debug preset silent`)
- **All categories**: Error level (1)
- **Features**: Minimal output, emergencies only
- **Use case**: Critical power situations

## 🔧 Configuration

### Compile-time Configuration

In `config.h`, configure the enhanced debug system:

```cpp
// Enhanced Debug System Configuration
#define DEBUG_ENHANCED_ENABLED true      // Enable enhanced debug features
#define DEBUG_TIMESTAMP_ENABLED true     // Enable timestamps in debug output
#define DEBUG_CATEGORY_ENABLED true      // Enable category tags in debug output
#define DEBUG_MEMORY_TRACKING true       // Enable memory usage tracking
#define DEBUG_PERFORMANCE_TIMING true    // Enable performance timing features

// Debug Mode Presets (uncomment one)
// #define DEBUG_DEVELOPMENT_MODE        // Verbose debugging for development
// #define DEBUG_PRODUCTION_MODE         // Minimal debugging for production
// #define DEBUG_PERFORMANCE_MODE        // Focus on performance metrics
// Default: Field deployment mode (balanced)
```

### Runtime Configuration

Configure debug system behavior in your code:

```cpp
void configureDebugSystem() {
    // Set specific debug levels
    EnhancedDebugSystem::setLevel(DEBUG_CAMERA, DEBUG_LEVEL_DEBUG);
    EnhancedDebugSystem::setLevel(DEBUG_WIFI, DEBUG_LEVEL_WARN);
    
    // Configure debug formatting
    DebugConfig config = EnhancedDebugSystem::getConfig();
    config.enableTimestamp = true;
    config.enableCategory = true;
    config.enableMemoryInfo = false;  // Reduce overhead
    EnhancedDebugSystem::setConfig(config);
}
```

## 📊 Example Output

### Categorized Debug Output
```
[    12345] INFO  [SYS] System initialization complete
[    12456] INFO  [CAM] Camera module initialized: OV2640
[    12567] DEBUG [MOT] PIR sensor calibrated, threshold: 512
[    12678] WARN  [PWR] Battery voltage low: 3.4V
[    12789] INFO  [LoRa] Joined mesh network, Node ID: 3
[    15234] INFO  [AI] Species detected: White-tailed Deer (87.3%)
```

### Performance Timing Output
```
=== Performance Timers ===
image_capture       : 15 calls, 2340 ms total, 156 ms avg
ai_analysis        : 12 calls, 8760 ms total, 730 ms avg
lora_transmission  : 8 calls, 3200 ms total, 400 ms avg
===========================
```

### Memory Information Output
```
=== Memory Information ===
Free heap: 157824 bytes
Min free heap: 142336 bytes
Heap size: 327680 bytes
Free PSRAM: 8126464 bytes
PSRAM size: 8388608 bytes
Largest free block: 113792 bytes
Total allocated: 169856 bytes
Total free: 157824 bytes
Heap fragmentation: 27.9%
==========================
```

## 🛠️ Integration with Existing Code

The enhanced debug system is designed for seamless integration:

1. **Backward Compatibility**: All existing `DEBUG_PRINT*` macros continue to work
2. **Minimal Overhead**: Debug statements are completely removed when `DEBUG_ENABLED` is false
3. **Easy Migration**: Gradually replace existing debug statements with categorized versions
4. **No Breaking Changes**: Existing code compiles and runs without modifications

## 📝 Best Practices

### Wildlife Camera Debugging
1. **Use appropriate categories**: Match debug category to the subsystem being debugged
2. **Set appropriate levels**: Use INFO for normal operation, DEBUG for troubleshooting
3. **Monitor performance**: Use timers for critical operations like image capture and AI analysis
4. **Track memory usage**: Regular memory monitoring helps identify leaks in long-running deployments
5. **Use field preset**: Default configuration balances debugging capability with performance

### Field Deployment
1. **Start with field preset**: Provides good balance of information and performance
2. **Use serial commands**: Adjust debug levels without firmware updates
3. **Monitor memory regularly**: Wildlife cameras run for extended periods
4. **Track timing metrics**: Identify performance degradation over time
5. **Enable remote debugging**: Use WiFi or LoRa for remote debug access when possible

The enhanced debug system transforms the ESP32 Wildlife Camera into a professional monitoring platform with enterprise-grade debugging capabilities, making field deployment and maintenance significantly more manageable.