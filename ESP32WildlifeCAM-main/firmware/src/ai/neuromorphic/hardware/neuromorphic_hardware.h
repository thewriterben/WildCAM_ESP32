/**
 * @file neuromorphic_hardware.h
 * @brief Neuromorphic Hardware Integration Layer
 * 
 * Provides abstraction layer for integrating with various neuromorphic
 * hardware platforms (Intel Loihi, IBM TrueNorth, BrainChip Akida).
 */

#ifndef NEUROMORPHIC_HARDWARE_H
#define NEUROMORPHIC_HARDWARE_H

#include "../neuromorphic_common.h"
#include <vector>
#include <memory>

/**
 * Neuromorphic Hardware Platform Types
 */
enum class NeuromorphicPlatform {
    SIMULATION,           // Software simulation on ESP32
    INTEL_LOIHI,         // Intel Loihi neuromorphic chip
    IBM_TRUENORTH,       // IBM TrueNorth architecture
    BRAINCHIP_AKIDA,     // BrainChip Akida processor
    FPGA_CUSTOM,         // Custom FPGA implementation
    MEMRISTOR_ARRAY      // Memristor-based neuromorphic circuit
};

/**
 * Hardware Capabilities
 */
struct HardwareCapabilities {
    uint32_t max_neurons;
    uint32_t max_synapses;
    float min_time_resolution;     // Microseconds
    float max_power_consumption;   // Watts
    bool supports_online_learning;
    bool supports_stdp;
    bool supports_event_driven;
    uint32_t memory_bytes;
    
    HardwareCapabilities()
        : max_neurons(1000)
        , max_synapses(100000)
        , min_time_resolution(1.0f)
        , max_power_consumption(0.5f)
        , supports_online_learning(false)
        , supports_stdp(false)
        , supports_event_driven(false)
        , memory_bytes(0) {}
};

/**
 * Hardware Configuration
 */
struct HardwareConfig {
    NeuromorphicPlatform platform;
    String device_id;
    String firmware_version;
    bool power_gating_enabled;
    float clock_frequency;         // MHz
    uint8_t num_cores;
    
    HardwareConfig()
        : platform(NeuromorphicPlatform::SIMULATION)
        , power_gating_enabled(true)
        , clock_frequency(240.0f)
        , num_cores(1) {}
};

/**
 * Base Hardware Interface
 */
class NeuromorphicHardwareInterface {
public:
    virtual ~NeuromorphicHardwareInterface() = default;
    
    /**
     * Initialize hardware platform
     */
    virtual bool init(const HardwareConfig& config) = 0;
    
    /**
     * Check if hardware is available and functional
     */
    virtual bool isAvailable() const = 0;
    
    /**
     * Get hardware capabilities
     */
    virtual HardwareCapabilities getCapabilities() const = 0;
    
    /**
     * Load SNN model to hardware
     */
    virtual bool loadModel(const NeuromorphicConfig& model_config) = 0;
    
    /**
     * Execute inference on hardware
     */
    virtual NeuromorphicDetection execute(const float* input_data, size_t length) = 0;
    
    /**
     * Execute on event data
     */
    virtual NeuromorphicDetection executeEvents(const std::vector<EventSensorData>& events) = 0;
    
    /**
     * Train model on hardware
     */
    virtual bool train(const float* input_data, size_t length, uint8_t label) = 0;
    
    /**
     * Get current power consumption
     */
    virtual float getPowerConsumption() const = 0;
    
    /**
     * Reset hardware state
     */
    virtual void reset() = 0;
    
    /**
     * Put hardware in low-power mode
     */
    virtual void enterLowPowerMode() = 0;
    
    /**
     * Wake hardware from low-power mode
     */
    virtual void exitLowPowerMode() = 0;
};

/**
 * Intel Loihi Integration
 */
class LoihiInterface : public NeuromorphicHardwareInterface {
public:
    LoihiInterface();
    ~LoihiInterface() override;
    
    bool init(const HardwareConfig& config) override;
    bool isAvailable() const override;
    HardwareCapabilities getCapabilities() const override;
    bool loadModel(const NeuromorphicConfig& model_config) override;
    NeuromorphicDetection execute(const float* input_data, size_t length) override;
    NeuromorphicDetection executeEvents(const std::vector<EventSensorData>& events) override;
    bool train(const float* input_data, size_t length, uint8_t label) override;
    float getPowerConsumption() const override;
    void reset() override;
    void enterLowPowerMode() override;
    void exitLowPowerMode() override;
    
private:
    bool initialized_;
    HardwareConfig config_;
    float current_power_;
    
    // Loihi-specific methods
    bool configureLoihiChip();
    void programNeuronCores();
    void setupRoutingNetwork();
};

/**
 * IBM TrueNorth Integration
 */
class TrueNorthInterface : public NeuromorphicHardwareInterface {
public:
    TrueNorthInterface();
    ~TrueNorthInterface() override;
    
    bool init(const HardwareConfig& config) override;
    bool isAvailable() const override;
    HardwareCapabilities getCapabilities() const override;
    bool loadModel(const NeuromorphicConfig& model_config) override;
    NeuromorphicDetection execute(const float* input_data, size_t length) override;
    NeuromorphicDetection executeEvents(const std::vector<EventSensorData>& events) override;
    bool train(const float* input_data, size_t length, uint8_t label) override;
    float getPowerConsumption() const override;
    void reset() override;
    void enterLowPowerMode() override;
    void exitLowPowerMode() override;
    
private:
    bool initialized_;
    HardwareConfig config_;
    float current_power_;
    
    // TrueNorth-specific methods
    bool configureCorelets();
    void mapNetworkToChip();
    void setupSpikeRouting();
};

/**
 * BrainChip Akida Integration
 */
class AkidaInterface : public NeuromorphicHardwareInterface {
public:
    AkidaInterface();
    ~AkidaInterface() override;
    
    bool init(const HardwareConfig& config) override;
    bool isAvailable() const override;
    HardwareCapabilities getCapabilities() const override;
    bool loadModel(const NeuromorphicConfig& model_config) override;
    NeuromorphicDetection execute(const float* input_data, size_t length) override;
    NeuromorphicDetection executeEvents(const std::vector<EventSensorData>& events) override;
    bool train(const float* input_data, size_t length, uint8_t label) override;
    float getPowerConsumption() const override;
    void reset() override;
    void enterLowPowerMode() override;
    void exitLowPowerMode() override;
    
private:
    bool initialized_;
    HardwareConfig config_;
    float current_power_;
    
    // Akida-specific methods
    bool configureAkidaDevice();
    void loadNeuralProcessor();
    void enableEdgeLearning();
};

/**
 * FPGA Custom Implementation
 */
class FPGAInterface : public NeuromorphicHardwareInterface {
public:
    FPGAInterface();
    ~FPGAInterface() override;
    
    bool init(const HardwareConfig& config) override;
    bool isAvailable() const override;
    HardwareCapabilities getCapabilities() const override;
    bool loadModel(const NeuromorphicConfig& model_config) override;
    NeuromorphicDetection execute(const float* input_data, size_t length) override;
    NeuromorphicDetection executeEvents(const std::vector<EventSensorData>& events) override;
    bool train(const float* input_data, size_t length, uint8_t label) override;
    float getPowerConsumption() const override;
    void reset() override;
    void enterLowPowerMode() override;
    void exitLowPowerMode() override;
    
private:
    bool initialized_;
    HardwareConfig config_;
    float current_power_;
    
    // FPGA-specific methods
    bool configureFPGABitstream();
    void programNeuronModules();
    void setupInterconnect();
};

/**
 * Neuromorphic Hardware Manager
 * 
 * Manages multiple hardware platforms and provides unified interface
 */
class NeuromorphicHardwareManager {
public:
    NeuromorphicHardwareManager();
    ~NeuromorphicHardwareManager();
    
    /**
     * Initialize manager and detect available hardware
     */
    bool init();
    
    /**
     * Get list of available platforms
     */
    std::vector<NeuromorphicPlatform> getAvailablePlatforms() const;
    
    /**
     * Select active platform
     */
    bool selectPlatform(NeuromorphicPlatform platform);
    
    /**
     * Get active platform
     */
    NeuromorphicPlatform getActivePlatform() const { return active_platform_; }
    
    /**
     * Execute inference on active platform
     */
    NeuromorphicDetection execute(const float* input_data, size_t length);
    
    /**
     * Execute on event data
     */
    NeuromorphicDetection executeEvents(const std::vector<EventSensorData>& events);
    
    /**
     * Load model to active platform
     */
    bool loadModel(const NeuromorphicConfig& model_config);
    
    /**
     * Train on active platform
     */
    bool train(const float* input_data, size_t length, uint8_t label);
    
    /**
     * Get capabilities of active platform
     */
    HardwareCapabilities getCapabilities() const;
    
    /**
     * Get power consumption
     */
    float getPowerConsumption() const;
    
    /**
     * Enable automatic platform selection based on power/performance
     */
    void enableAutomaticSelection(bool enable) { auto_selection_enabled_ = enable; }
    
    /**
     * Set power budget for automatic selection
     */
    void setPowerBudget(float power_mw) { power_budget_ = power_mw; }
    
private:
    std::map<NeuromorphicPlatform, std::unique_ptr<NeuromorphicHardwareInterface>> platforms_;
    NeuromorphicPlatform active_platform_;
    bool auto_selection_enabled_;
    float power_budget_;
    
    // Automatic platform selection
    NeuromorphicPlatform selectOptimalPlatform(size_t input_size);
    
    // Platform detection
    void detectAvailableHardware();
    bool detectLoihi();
    bool detectTrueNorth();
    bool detectAkida();
    bool detectFPGA();
};

#endif // NEUROMORPHIC_HARDWARE_H
