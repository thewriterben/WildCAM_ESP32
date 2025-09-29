# Federated Learning System for ESP32WildlifeCAM

## Overview

This implementation adds comprehensive federated learning capabilities to the ESP32WildlifeCAM system, enabling multiple devices to collaboratively improve AI models for wildlife detection and behavior analysis without sharing raw data.

## Architecture

### Core Components

#### 1. **Federated Learning System** (`federated_learning_system.h`)
- **Purpose**: Main system coordinator and high-level API
- **Features**: System lifecycle management, automatic mode operations, web interface integration
- **Integration**: Provides easy-to-use interface for the main application

#### 2. **Local Training Module** (`local_training_module.h`)
- **Purpose**: On-device training with incremental learning
- **Features**: Data augmentation, memory-efficient training, power-aware scheduling
- **Integration**: Extends existing TensorFlow Lite Micro framework

#### 3. **Privacy-Preserving Aggregation** (`privacy_preserving_aggregation.h`)
- **Purpose**: Secure model aggregation with differential privacy
- **Features**: FedAvg/FedProx algorithms, secure multi-party computation, robust aggregation
- **Integration**: Uses existing security infrastructure

#### 4. **Federated Learning Coordinator** (`federated_learning_coordinator.h`)
- **Purpose**: Round management and participant coordination
- **Features**: Round initialization, participant recruitment, model validation
- **Integration**: Supports star, mesh, and hybrid topologies

#### 5. **Model Update Protocol** (`model_update_protocol.h`)
- **Purpose**: Secure, bandwidth-efficient communication
- **Features**: Message compression, encryption, multi-protocol support
- **Integration**: Leverages existing networking infrastructure

#### 6. **Network Topology Manager** (`network_topology_manager.h`)
- **Purpose**: Dynamic network topology management
- **Features**: Node discovery, route optimization, fault tolerance
- **Integration**: Works with WiFi, LoRa, and Cellular networks

### Integration Components

#### 7. **AI Federated Integration** (`ai_federated_integration.h`)
- **Purpose**: Seamless integration with existing AI Wildlife System
- **Features**: Automatic contribution of high-confidence results, expert validation interface
- **Integration**: Maintains backward compatibility

#### 8. **Federated Web Interface** (`federated_web_interface.h`)
- **Purpose**: Web-based monitoring and control
- **Features**: Real-time dashboard, configuration management, expert validation UI
- **Integration**: Extends existing web interface

#### 9. **Federated Power Manager** (`federated_power_manager.h`)
- **Purpose**: Power-aware federated learning operations
- **Features**: Solar charging optimization, battery-aware scheduling, emergency management
- **Integration**: Works with existing power management system

## Key Features

### Privacy Protection
- **Differential Privacy**: Configurable epsilon values for privacy-utility trade-offs
- **Data Anonymization**: Location fuzzing, timestamp obfuscation
- **Secure Aggregation**: Multi-party computation without revealing individual updates
- **Privacy Budget Management**: Tracking and limiting privacy expenditure

### Network Adaptability
- **Multi-Protocol Support**: WiFi, LoRa, Cellular with automatic failover
- **Topology Flexibility**: Star, mesh, hierarchical, and hybrid configurations
- **Fault Tolerance**: Automatic coordinator election, partition handling
- **Bandwidth Optimization**: Compression, quantization, sparsification

### Power Optimization
- **Solar Integration**: Training scheduled during peak solar hours
- **Battery Awareness**: Operations suspended at low battery levels
- **Adaptive Scheduling**: Learning from power patterns
- **Emergency Management**: Graceful degradation during power emergencies

### Model Management
- **Incremental Learning**: Continuous improvement from new data
- **Model Versioning**: Checkpoint management and rollback capabilities
- **Validation Framework**: Automated and expert validation
- **Update Distribution**: Secure model update dissemination

## Configuration

### Basic Configuration Example

```cpp
// Initialize federated learning system
FLSystemConfig config;
config.federatedConfig.enableFederatedLearning = true;
config.federatedConfig.privacyLevel = PrivacyLevel::STANDARD;
config.federatedConfig.contributionThreshold = 0.9f;
config.topologyConfig.preferredTopology = NetworkTopology::MESH;
config.enablePowerManagement = true;
config.enableAutomaticMode = true;

// Initialize the system
initializeFederatedLearning(config);
```

### Privacy Configuration

```cpp
// High privacy configuration
config.federatedConfig.privacyLevel = PrivacyLevel::HIGH;
config.federatedConfig.privacyEpsilon = 0.5f; // More privacy
config.federatedConfig.anonymizeLocation = true;
config.federatedConfig.anonymizeTimestamps = true;
```

### Power Management Configuration

```cpp
// Solar-optimized power configuration
FLPowerConfig powerConfig;
powerConfig.enableSolarOptimization = true;
powerConfig.solarPeakStartHour = 8;
powerConfig.solarPeakEndHour = 16;
powerConfig.minSolarPower = 100.0f;
powerConfig.criticalBatteryLevel = 0.15f;
```

### Network Topology Configuration

```cpp
// Mesh network configuration
TopologyConfig topoConfig;
topoConfig.preferredTopology = NetworkTopology::MESH;
topoConfig.maxNodes = 20;
topoConfig.maxHops = 3;
topoConfig.enableAutoReconfiguration = true;
topoConfig.enableFaultTolerance = true;
```

## Usage Examples

### 1. Basic Federated Learning Participation

```cpp
// Enable federated learning
enableFederatedLearning(true);

// The system will automatically:
// - Discover nearby devices
// - Join federated rounds when invited
// - Contribute high-confidence observations
// - Download and apply model updates
```

### 2. Manual Training Session

```cpp
// Prepare training data
std::vector<TrainingSample> samples;
// ... populate samples ...

// Start local training
bool success = startTraining(MODEL_SPECIES_CLASSIFIER, samples);
if (success) {
    TrainingResult result = getLastTrainingResult();
    Serial.printf("Training completed with accuracy: %.2f%%\n", 
                  result.finalProgress.accuracy * 100);
}
```

### 3. Expert Validation Workflow

```cpp
// Set up expert validation callback
g_aiFederatedIntegration->setExpertValidationRequestCallback(
    [](const String& validationId) {
        Serial.printf("Expert validation requested: %s\n", validationId.c_str());
        // Notify expert through web interface or external system
    }
);

// Process expert feedback
bool validated = true;
AIResult correctedResult = /* expert-corrected result */;
g_aiFederatedIntegration->processExpertFeedback(validationId, validated, correctedResult);
```

### 4. Privacy-Aware Contribution

```cpp
// Configure privacy settings
g_aiFederatedIntegration->setPrivacyLevel(PrivacyLevel::HIGH);

// The system will automatically apply privacy protection
// when contributing to federated learning
FederatedWildlifeResult result = processWildlifeFrameWithFederation(frame);
if (result.contributedToFederation) {
    Serial.printf("Contributed to federation with privacy level: %s\n",
                  privacyLevelToString(result.privacyLevel));
}
```

### 5. Power-Aware Operation

```cpp
// Check power availability before training
if (isPowerAvailableForFederatedLearning()) {
    TrainingSchedule schedule = getNextOptimalTrainingWindow();
    if (schedule.startTime > 0) {
        Serial.printf("Next training window: %u minutes\n", 
                      (schedule.startTime - getCurrentTimestamp()) / 60000);
    }
}
```

## Web Interface

### Dashboard Features
- **Real-time Metrics**: Training progress, accuracy improvements, network status
- **Network Visualization**: Connected devices, topology view, signal strength
- **Privacy Status**: Current privacy level, remaining privacy budget
- **Expert Validation**: Queue of samples requiring validation

### API Endpoints
- `GET /api/federated/status` - System status and metrics
- `POST /api/federated/config` - Update configuration
- `GET /api/federated/network` - Network topology information
- `POST /api/expert/validate` - Submit expert validation

## Testing

### Unit Tests
Run the basic federated learning tests:
```bash
cd ESP32WildlifeCAM
g++ -std=c++17 tests/test_federated_learning_basic.cpp -o test_fl
./test_fl
```

### Integration Tests
The complete test suite includes:
- Configuration validation
- Privacy protection mechanisms
- Network topology algorithms
- Training sample management
- Model update protocols

## Performance Considerations

### Memory Usage
- **Training Arena**: 64-512KB depending on model size
- **Network Buffers**: 32KB for message handling
- **Model Storage**: 100KB-1MB per model
- **Privacy Overhead**: 10-20% for differential privacy

### Power Consumption
- **Training**: 500-800mW during active training
- **Communication**: 150-300mW during model updates
- **Idle**: 20-50mW when participating in network
- **Solar Optimization**: Training scheduled during peak solar hours

### Network Bandwidth
- **Model Updates**: 50-200KB per update (compressed)
- **Heartbeats**: 100-500 bytes every 30 seconds
- **Discovery**: 1-5KB during node discovery
- **Compression**: 60-80% reduction with quantization

## Security Considerations

### Data Protection
- **No Raw Data Sharing**: Only model updates are transmitted
- **Encryption**: All communication encrypted with AES-256
- **Authentication**: Secure device identification and authorization
- **Integrity**: Cryptographic checksums for all updates

### Privacy Guarantees
- **Differential Privacy**: Mathematically proven privacy bounds
- **k-Anonymity**: Location and temporal anonymization
- **Secure Aggregation**: Individual contributions cannot be extracted
- **Privacy Budget**: Tracked and limited privacy expenditure

### Attack Resistance
- **Poisoning Attacks**: Robust aggregation with outlier detection
- **Inference Attacks**: Differential privacy prevents membership inference
- **Communication Attacks**: Encrypted channels with authentication
- **Sybil Attacks**: Device registration and reputation systems

## Future Enhancements

### Planned Features
1. **Advanced Aggregation**: Implementation of additional federated learning algorithms
2. **Cross-Model Learning**: Transfer learning between different wildlife models
3. **Seasonal Adaptation**: Automatic model adaptation to seasonal changes
4. **Edge Computing**: Integration with edge computing infrastructure
5. **Blockchain Integration**: Decentralized reputation and incentive systems

### Research Directions
1. **Hierarchical Federated Learning**: Multi-level aggregation for large networks
2. **Asynchronous Federated Learning**: Non-synchronized training rounds
3. **Personalized Models**: Device-specific model customization
4. **Energy Harvesting Optimization**: Advanced solar charging integration
5. **Wildlife Behavior Prediction**: Long-term pattern analysis

## Troubleshooting

### Common Issues

#### 1. **High Memory Usage**
- Reduce batch size in training configuration
- Enable model compression
- Use smaller model variants

#### 2. **Network Connectivity Problems**
- Check protocol availability (WiFi/LoRa/Cellular)
- Verify network configuration
- Review firewall settings

#### 3. **Battery Drain**
- Enable power optimization mode
- Adjust training schedules
- Check solar charging configuration

#### 4. **Privacy Budget Exhausted**
- Increase privacy epsilon (less privacy, more utility)
- Reset privacy budget
- Reduce contribution frequency

#### 5. **Training Failures**
- Verify training data quality
- Check memory constraints
- Review model compatibility

### Debug Commands

```cpp
// Enable debug logging
config.logLevel = "DEBUG";

// Check system status
FLSystemStats stats = g_federatedLearningSystem->getSystemStats();
Serial.printf("Status: %d, Rounds: %u, Success Rate: %.1f%%\n",
              stats.status, stats.totalRounds, 
              stats.successfulRounds * 100.0f / max(stats.totalRounds, 1u));

// Monitor network
NetworkMetrics network = getCurrentNetworkMetrics();
Serial.printf("Connected Nodes: %u, Reliability: %.2f\n",
              network.connectedNodes, network.averageReliability);

// Check power status
FLPowerMetrics power = g_federatedPowerManager->getPowerMetrics();
Serial.printf("Battery: %.1f%%, Solar: %.1fW\n",
              power.currentBatteryLevel * 100, power.currentSolarPower);
```

## Contributing

When contributing to the federated learning system:

1. **Follow Existing Patterns**: Maintain consistency with existing ESP32WildlifeCAM architecture
2. **Test Thoroughly**: Include unit tests for new components
3. **Document Changes**: Update this README for new features
4. **Consider Privacy**: Ensure new features maintain privacy guarantees
5. **Optimize Power**: Consider power consumption in all implementations

## License

This federated learning implementation follows the same license as the main ESP32WildlifeCAM project.