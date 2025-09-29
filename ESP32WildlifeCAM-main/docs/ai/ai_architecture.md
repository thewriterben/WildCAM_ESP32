# AI Architecture Documentation

## Overview

The ESP32 Wildlife Camera AI system implements a comprehensive machine learning pipeline optimized for edge computing. The architecture combines TensorFlow Lite Micro with custom optimizations for wildlife monitoring applications.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    ESP32 Wildlife Camera AI System              │
├─────────────────────────────────────────────────────────────────┤
│  Application Layer                                              │
│  ┌─────────────────┐ ┌─────────────────┐ ┌─────────────────┐   │
│  │  Main App       │ │  MicroPython    │ │  Web Interface  │   │
│  │  (C++)          │ │  Scripting      │ │  (Optional)     │   │
│  └─────────────────┘ └─────────────────┘ └─────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│  AI/ML Framework Layer                                          │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │              AIWildlifeSystem                               │ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐   │ │
│  │  │  Species    │ │  Behavior   │ │  Intelligent        │   │ │
│  │  │  Classifier │ │  Analyzer   │ │  Trigger System     │   │ │
│  │  └─────────────┘ └─────────────┘ └─────────────────────┘   │ │
│  └─────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  TinyML Engine Layer                                            │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │               InferenceEngine                               │ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐   │ │
│  │  │  Model      │ │  Memory     │ │  Performance        │   │ │
│  │  │  Manager    │ │  Manager    │ │  Optimizer          │   │ │
│  │  └─────────────┘ └─────────────┘ └─────────────────────┘   │ │
│  └─────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  TensorFlow Lite Micro Layer                                   │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │  ┌─────────────┐ ┌─────────────┐ ┌─────────────────────┐   │ │
│  │  │  Micro      │ │  ESP32      │ │  Memory             │   │ │
│  │  │  Interpreter│ │  Optimized  │ │  Allocator          │   │ │
│  │  │             │ │  Operators  │ │                     │   │ │
│  │  └─────────────┘ └─────────────┘ └─────────────────────┘   │ │
│  └─────────────────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────────────────┤
│  Hardware Abstraction Layer                                    │
│  ┌─────────────────┐ ┌─────────────┐ ┌─────────────────────┐   │
│  │  Camera         │ │  PSRAM      │ │  Flash Storage      │   │
│  │  Interface      │ │  Manager    │ │  Manager            │   │
│  └─────────────────┘ └─────────────┘ └─────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. AIWildlifeSystem

**Purpose**: Main coordinator for all AI operations
**Location**: `firmware/src/ai/ai_wildlife_system.h`

**Key Responsibilities**:
- Orchestrate species classification and behavior analysis
- Manage intelligent triggering system
- Coordinate power optimization
- Handle continuous learning and model updates

**APIs**:
```cpp
WildlifeAnalysisResult analyzeFrame(const CameraFrame& frame);
bool shouldTriggerCapture();
void updatePowerState(float batteryVoltage, bool isCharging);
```

### 2. InferenceEngine

**Purpose**: TensorFlow Lite model execution engine
**Location**: `firmware/src/ai/tinyml/inference_engine.h`

**Key Responsibilities**:
- Load and manage TensorFlow Lite models
- Execute inference with memory optimization
- Handle model-specific preprocessing/postprocessing
- Performance monitoring and metrics

**APIs**:
```cpp
bool loadModel(const uint8_t* modelData, size_t modelSize, ModelType type);
AIResult runInference(const CameraFrame& frame, ModelType type);
AIMetrics getPerformanceMetrics();
```

### 3. Species Classifier

**Purpose**: Wildlife species identification
**Location**: `firmware/src/ai/vision/species_classifier.h`

**Features**:
- 50+ species support with regional adaptation
- Multi-model ensemble for improved accuracy
- Size and age estimation capabilities
- Temporal tracking across frames

**Performance**:
- Target Accuracy: >90%
- Inference Time: <1.5 seconds
- Memory Usage: 512KB arena

### 4. Behavior Analyzer

**Purpose**: Animal behavior classification and pattern analysis
**Location**: `firmware/src/ai/vision/behavior_analyzer.h`

**Features**:
- 12 behavior categories
- Stress level detection
- Social interaction analysis
- Temporal pattern recognition

**Performance**:
- Target Accuracy: >85%
- Inference Time: <1.0 seconds
- Memory Usage: 384KB arena

## Memory Architecture

### Memory Layout

```
ESP32-S3 Memory Map (AI Optimized)
┌─────────────────────────────────────┐
│  Internal SRAM (512KB)              │
│  ┌─────────────────────────────────┐ │
│  │  System Stack & Heap (200KB)   │ │
│  ├─────────────────────────────────┤ │
│  │  AI Common Data (50KB)         │ │
│  ├─────────────────────────────────┤ │
│  │  Motion Detector Arena (128KB) │ │
│  ├─────────────────────────────────┤ │
│  │  Camera Buffers (134KB)        │ │
│  └─────────────────────────────────┘ │
├─────────────────────────────────────┤
│  External PSRAM (2MB-8MB)           │
│  ┌─────────────────────────────────┐ │
│  │  Species Classifier (512KB)    │ │
│  ├─────────────────────────────────┤ │
│  │  Behavior Analyzer (384KB)     │ │
│  ├─────────────────────────────────┤ │
│  │  Model Storage Cache (512KB)   │ │
│  ├─────────────────────────────────┤ │
│  │  Image Processing (256KB)      │ │
│  ├─────────────────────────────────┤ │
│  │  Available Space (≥360KB)      │ │
│  └─────────────────────────────────┘ │
└─────────────────────────────────────┘
```

### Memory Management Strategy

1. **PSRAM Priority**: Large models loaded into PSRAM when available
2. **Fallback Strategy**: Graceful degradation to internal RAM
3. **Dynamic Allocation**: Models loaded/unloaded based on power state
4. **Memory Defragmentation**: Periodic cleanup to prevent fragmentation

## Processing Pipeline

### Image Analysis Flow

```
Camera Frame Capture
         │
         ▼
   Preprocessing
    ┌─────────┐
    │ Resize  │
    │ Normalize│ 
    │ Format  │
    └─────────┘
         │
         ▼
   Motion Detection
    ┌─────────┐
    │ TinyML  │
    │ Model   │
    └─────────┘
         │
         ▼
   Species Classification
    ┌─────────┐
    │ CNN     │
    │ Model   │
    └─────────┘
         │
         ▼
   Behavior Analysis
    ┌─────────┐
    │ Temporal│
    │ CNN     │
    └─────────┘
         │
         ▼
   Result Fusion
    ┌─────────┐
    │ Combine │
    │ Analyze │
    │ Decide  │
    └─────────┘
         │
         ▼
   Action Decision
```

### Power-Optimized Processing

The system adapts processing complexity based on power state:

**High Power Mode** (Battery >50%, Charging):
- All models active
- 1-second inference intervals
- Maximum accuracy mode

**Balanced Mode** (Battery 20-50%):
- Priority models only
- 2-second inference intervals
- Balanced accuracy/power

**Power Saving Mode** (Battery <20%):
- Motion detection only
- 5-second inference intervals
- Minimum power consumption

## Model Optimization Techniques

### 1. Quantization

**INT8 Post-Training Quantization**:
- Reduces model size by 75%
- Decreases memory usage by 75%
- Minimal accuracy loss (<2%)

```cpp
// Example quantization configuration
quantization_config = {
    .input_type = kTfLiteInt8,
    .output_type = kTfLiteInt8,
    .activations_type = kTfLiteInt8,
    .weights_type = kTfLiteInt8
};
```

### 2. Model Pruning

**Structured Pruning**:
- Remove entire neurons/channels
- 30-50% parameter reduction
- Hardware-friendly optimizations

### 3. Knowledge Distillation

**Teacher-Student Framework**:
- Large cloud model (teacher)
- Compact edge model (student)
- Maintains accuracy with reduced complexity

## Edge Impulse Integration

### Workflow Integration

```
Data Collection → Edge Impulse → Model Training → Deployment
      │                │              │            │
      ▼                ▼              ▼            ▼
ESP32 Camera → Cloud Processing → Optimized Model → ESP32
```

### Key Features

1. **Automatic Data Collection**: Cameras upload training data
2. **Cloud Training**: GPU-accelerated model training
3. **Edge Optimization**: Automatic hardware optimization
4. **Over-the-Air Updates**: Seamless model deployment

## Performance Metrics

### Inference Performance

| Model Type | Size | Inference Time | Memory | Accuracy |
|------------|------|----------------|--------|----------|
| Species Classifier | 512KB | 1.2s | 512KB | 92% |
| Behavior Analyzer | 300KB | 0.8s | 384KB | 87% |
| Motion Detector | 100KB | 0.2s | 128KB | 95% |

### Power Consumption

| Processing Mode | Current Draw | Battery Life* |
|----------------|--------------|---------------|
| AI Disabled | 80mA | 48 hours |
| Power Saving | 120mA | 32 hours |
| Balanced | 180mA | 21 hours |
| High Performance | 250mA | 15 hours |

*Based on 3000mAh battery

### Memory Utilization

| Component | Internal RAM | PSRAM | Flash |
|-----------|-------------|-------|-------|
| System | 200KB | - | 1MB |
| AI Framework | 50KB | 1.4MB | 500KB |
| Models | 128KB | 1.4MB | 2MB |
| Buffers | 134KB | 256KB | - |
| **Total** | **512KB** | **3.0MB** | **3.5MB** |

## Future Enhancements

### Short Term (v2.1)
- Multi-object detection
- Improved power optimization
- Enhanced Edge Impulse integration

### Medium Term (v2.5)
- Audio analysis capabilities
- Federated learning
- Advanced behavior patterns

### Long Term (v3.0)
- Real-time video analysis
- Multi-camera coordination
- Cloud AI acceleration

## Development Guidelines

### Adding New Models

1. **Model Requirements**:
   - TensorFlow Lite format (.tflite)
   - INT8 quantization preferred
   - Memory budget <512KB

2. **Integration Steps**:
   - Add model type to `ModelType` enum
   - Implement preprocessing/postprocessing
   - Update model manifest
   - Add performance benchmarks

3. **Testing Requirements**:
   - Accuracy validation
   - Memory usage verification
   - Performance benchmarking
   - Power consumption testing

### Performance Optimization

1. **Memory Optimization**:
   - Use PSRAM for large allocations
   - Implement memory pooling
   - Profile memory usage patterns

2. **Speed Optimization**:
   - Leverage ESP32-S3 SIMD instructions
   - Optimize preprocessing pipelines
   - Use hardware acceleration when available

3. **Power Optimization**:
   - Dynamic CPU frequency scaling
   - Model-based power budgeting
   - Intelligent sleep scheduling