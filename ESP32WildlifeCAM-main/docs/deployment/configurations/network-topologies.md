# Network Topologies Configuration Guide

## Overview

ESP32 Wildlife Cameras support multiple network architectures depending on deployment requirements, environmental constraints, and communication needs. This guide covers all supported network topologies with configuration examples and optimization strategies.

## Network Architecture Types

### 1. Star Network (Simple Hub-and-Spoke)

**Best For**: Small deployments (5-15 nodes), centralized data collection, simple management

```
        Central Hub
    ╭─────────┴─────────╮
   Node1  Node2  Node3  Node4
```

#### Configuration
```cpp
// Star network configuration
#define NETWORK_TOPOLOGY_STAR
#define STAR_HUB_NODE_ID 1
#define STAR_MAX_NODES 15
#define STAR_POLLING_INTERVAL 60000    // 1 minute polling

struct StarNetworkConfig {
    bool is_hub_node = false;           // Set true for central hub
    uint8_t hub_node_id = 1;           // ID of the hub node
    uint32_t reporting_interval = 300000; // 5 minutes for leaf nodes
    uint32_t hub_polling_interval = 60000; // 1 minute hub polling
    bool store_and_forward = true;      // Cache data if hub unavailable
    uint8_t max_retry_attempts = 3;     // Retry failed transmissions
};

class StarNetworkManager {
public:
    bool initStarNetwork(StarNetworkConfig config);
    bool registerWithHub();
    bool sendDataToHub(WildlifeData data);
    bool pollLeafNodes(); // Hub only
    bool handleNodeFailure(uint8_t node_id);
    
private:
    bool is_hub_node;
    std::vector<uint8_t> registered_nodes;
    std::queue<DataPacket> pending_data;
};
```

#### Advantages
- Simple setup and management
- Centralized data collection
- Easy troubleshooting
- Low complexity

#### Disadvantages
- Single point of failure (hub)
- Limited range (direct communication to hub required)
- Hub requires high power budget
- Not suitable for large areas

### 2. Mesh Network (Self-Healing)

**Best For**: Large deployments (15+ nodes), redundant paths, extended range

```
Node1 ── Node2 ── Node3
  │   ╲   │   ╱   │
  │    ╲  │  ╱    │
Node4 ── Node5 ── Node6
  │       │       │
Node7 ── Node8 ── Node9
```

#### Configuration
```cpp
// Mesh network configuration
#define NETWORK_TOPOLOGY_MESH
#define MESH_MAX_NODES 50
#define MESH_MAX_HOPS 6
#define MESH_ROUTING_PROTOCOL_AODV

struct MeshNetworkConfig {
    String mesh_id = "WildlifeMesh";
    String mesh_password = "SecureWildlife2024";
    uint8_t max_connections = 6;        // Maximum direct connections
    uint32_t route_discovery_timeout = 30000; // 30 seconds
    uint32_t route_maintenance_interval = 300000; // 5 minutes
    bool enable_load_balancing = true;   // Distribute traffic
    uint8_t max_hop_count = 6;          // Maximum hops to destination
    float signal_strength_threshold = -80; // dBm minimum signal
};

class MeshNetworkManager {
public:
    bool initMeshNetwork(MeshNetworkConfig config);
    bool joinMeshNetwork();
    bool discoverRoutes();
    bool sendMeshData(WildlifeData data, uint8_t destination_id);
    bool relayMeshData(DataPacket packet);
    
    // Self-healing capabilities
    bool detectNodeFailure(uint8_t node_id);
    bool findAlternativeRoute(uint8_t destination);
    bool updateRoutingTable();
    bool optimizeTopology();
    
private:
    MeshTopology current_topology;
    RoutingTable routing_table;
    std::map<uint8_t, NodeInfo> neighbor_nodes;
    std::queue<DataPacket> forwarding_queue;
};
```

#### Advantages
- Self-healing and redundant
- Extended range through multi-hop
- No single point of failure
- Scalable to large deployments

#### Disadvantages
- Complex setup and management
- Higher power consumption
- Routing overhead
- Potential for network loops

### 3. Tree Network (Hierarchical)

**Best For**: Structured deployments, geographical clustering, efficient data aggregation

```
         Root Hub
        ╱    │    ╲
   Hub-A    Hub-B   Hub-C
   ╱  ╲      │      ╱  ╲
Leaf1 Leaf2 Leaf3 Leaf4 Leaf5
```

#### Configuration
```cpp
// Tree network configuration  
#define NETWORK_TOPOLOGY_TREE
#define TREE_MAX_DEPTH 4
#define TREE_MAX_CHILDREN 5

struct TreeNetworkConfig {
    uint8_t node_level = 0;             // 0=root, 1=branch, 2+=leaf
    uint8_t parent_node_id = 0;         // Parent node ID
    std::vector<uint8_t> children_nodes; // Child node IDs
    uint32_t aggregation_interval = 600000; // 10 minutes
    bool enable_data_aggregation = true; // Aggregate before forwarding
    bool enable_load_balancing = true;   // Balance across children
    uint8_t max_tree_depth = 4;         // Maximum tree depth
};

class TreeNetworkManager {
public:
    bool initTreeNetwork(TreeNetworkConfig config);
    bool joinAsChild(uint8_t parent_id);
    bool acceptChild(uint8_t child_id);
    bool aggregateChildData();
    bool forwardToParent(AggregatedData data);
    
    // Tree maintenance
    bool detectParentFailure();
    bool findNewParent();
    bool rebalanceSubtree();
    bool pruneFailedBranches();
    
private:
    TreeNode node_info;
    std::vector<TreeNode> children;
    TreeNode parent;
    DataAggregator aggregator;
};
```

#### Advantages
- Efficient data aggregation
- Clear hierarchy and roles
- Scalable and organized
- Reduced network traffic

#### Disadvantages
- Parent nodes are critical points
- Limited redundancy
- Complex reconfiguration
- Unbalanced trees possible

### 4. Hybrid Network (Multi-Protocol)

**Best For**: Complex deployments, multiple communication methods, maximum reliability

```
Satellite Gateway
      │
   LoRa Hub ──── Cellular Hub
   ╱   │   ╲        │
WiFi  WiFi  LoRa   Cell
Mesh  Mesh  Node   Node
```

#### Configuration
```cpp
// Hybrid network configuration
#define NETWORK_TOPOLOGY_HYBRID
#define HYBRID_PROTOCOLS_ENABLED 4

struct HybridNetworkConfig {
    // Available protocols
    bool lora_enabled = true;
    bool wifi_enabled = true;
    bool cellular_enabled = true;
    bool satellite_enabled = true;
    
    // Protocol priorities
    uint8_t primary_protocol = PROTOCOL_LORA;
    uint8_t secondary_protocol = PROTOCOL_WIFI;
    uint8_t backup_protocol = PROTOCOL_CELLULAR;
    uint8_t emergency_protocol = PROTOCOL_SATELLITE;
    
    // Switching criteria
    float signal_strength_threshold = -85;  // dBm
    uint32_t connection_timeout = 30000;     // 30 seconds
    uint32_t protocol_switch_delay = 10000;  // 10 seconds
    bool adaptive_protocol_selection = true;
    
    // Data routing
    bool load_balancing_enabled = true;
    bool redundant_transmission = false;     // Send via multiple protocols
    bool protocol_specific_compression = true;
};

class HybridNetworkManager {
public:
    bool initHybridNetwork(HybridNetworkConfig config);
    bool selectOptimalProtocol();
    bool sendViaOptimalPath(WildlifeData data);
    bool switchProtocol(Protocol new_protocol);
    
    // Protocol management
    bool testProtocolConnectivity(Protocol protocol);
    float assessProtocolQuality(Protocol protocol);
    bool enableProtocol(Protocol protocol);
    bool disableProtocol(Protocol protocol);
    
    // Adaptive routing
    bool adaptToConditions();
    bool learnOptimalRouting();
    bool balanceLoadAcrossProtocols();
    
private:
    std::map<Protocol, ProtocolManager*> protocol_managers;
    ProtocolSelector protocol_selector;
    AdaptiveRouter adaptive_router;
    LoadBalancer load_balancer;
};
```

#### Advantages
- Maximum reliability and redundancy
- Adaptive to conditions
- Multiple communication options
- Optimal protocol selection

#### Disadvantages
- Complex configuration
- Higher power consumption
- Multiple hardware requirements
- Increased cost

## Protocol-Specific Configurations

### LoRa Mesh Configuration
```cpp
// LoRa-specific mesh networking
#define LORA_MESH_ENABLED
#define LORA_FREQUENCY 915E6              // US band (433E6 for EU)
#define LORA_BANDWIDTH 125E3
#define LORA_SPREADING_FACTOR 7           // Balance of range vs data rate

struct LoRaMeshConfig {
    float frequency = 915E6;              // Operating frequency
    uint32_t bandwidth = 125000;          // Signal bandwidth
    uint8_t spreading_factor = 7;         // SF7-12 (higher = longer range)
    uint8_t coding_rate = 5;              // Error correction (5-8)
    int8_t tx_power = 14;                 // Transmission power (dBm)
    
    // Mesh-specific settings
    uint16_t sync_word = 0x34;            // Network sync word
    uint32_t preamble_length = 8;         // Preamble symbols
    bool enable_crc = true;               // Cyclic redundancy check
    bool enable_implicit_header = false;  // Use explicit header
    
    // Network topology
    uint8_t mesh_id = 1;                  // Mesh network identifier
    uint32_t beacon_interval = 60000;     // 1 minute beacon interval
    uint32_t route_timeout = 300000;      // 5 minutes route timeout
    uint8_t max_retransmissions = 3;      // Maximum retry attempts
};

class LoRaMeshNetwork {
public:
    bool initLoRaMesh(LoRaMeshConfig config);
    bool sendLoRaPacket(LoRaPacket packet);
    bool receiveLoRaPacket(LoRaPacket& packet);
    bool broadcastBeacon();
    bool updateMeshTopology();
    
    // Performance optimization
    bool optimizeForRange();              // Maximize range
    bool optimizeForDataRate();           // Maximize data rate
    bool optimizeForPower();              // Minimize power consumption
    bool adaptiveParameterTuning();       // Automatic optimization
    
private:
    LoRaRadio lora_radio;
    MeshTopology mesh_topology;
    RoutingTable routing_table;
    BeaconManager beacon_manager;
};
```

### WiFi Mesh Configuration
```cpp
// WiFi mesh networking (ESP-MESH)
#define WIFI_MESH_ENABLED
#define WIFI_MESH_MAX_LAYERS 6
#define WIFI_MESH_MAX_CONNECTIONS 10

struct WiFiMeshConfig {
    String mesh_id = "WildlifeMesh";
    String mesh_password = "WildlifeSecure123";
    uint8_t channel = 1;                  // WiFi channel (1-13)
    uint8_t max_connection = 10;          // Max connections per node
    uint8_t max_layer = 6;                // Maximum mesh layers
    
    // Performance settings
    bool enable_root_healing = true;      // Automatic root healing
    bool enable_parent_switch = true;     // Switch parents for optimization
    uint32_t vote_percentage = 90;        // Voting threshold for root
    uint32_t reconnect_timeout = 10000;   // Reconnection timeout
    
    // Security settings
    wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA2_PSK;
    bool enable_mesh_ie_crypto = true;    // Encrypt mesh information
    
    // Power management
    bool enable_ps = true;                // Power save mode
    uint32_t ps_inactive_time = 300000;   // 5 minutes inactive time
};

class WiFiMeshNetwork {
public:
    bool initWiFiMesh(WiFiMeshConfig config);
    bool startMeshNetwork();
    bool connectToMesh();
    bool sendMeshData(wifi_mesh_data_t* data);
    bool receiveMeshData(wifi_mesh_data_t* data);
    
    // Mesh management
    bool promoteToRoot();
    bool electNewRoot();
    bool handleNodeJoin(uint8_t node_id);
    bool handleNodeLeave(uint8_t node_id);
    
    // Performance monitoring
    MeshPerformance getMeshPerformance();
    TopologyInfo getCurrentTopology();
    bool optimizeMeshStructure();
    
private:
    wifi_mesh_config_t mesh_config;
    MeshEventHandler event_handler;
    PerformanceMonitor performance_monitor;
};
```

### Cellular Network Configuration
```cpp
// Cellular communication configuration
#define CELLULAR_ENABLED
#define CELLULAR_APN "wildlife.data"
#define CELLULAR_BACKUP_ENABLED true

struct CellularConfig {
    String apn = "wildlife.data";
    String username = "";
    String password = "";
    String operator_selection = "auto";    // auto or specific operator
    
    // Connection settings
    uint32_t connection_timeout = 120000;  // 2 minutes
    uint32_t data_timeout = 60000;         // 1 minute
    uint8_t max_retry_attempts = 5;
    uint32_t retry_delay = 30000;          // 30 seconds
    
    // Data optimization
    bool enable_compression = true;
    bool enable_header_compression = true;
    uint32_t max_packet_size = 1500;       // MTU size
    bool enable_tcp_nodelay = true;
    
    // Power management
    bool enable_power_save = true;
    uint32_t idle_timeout = 300000;        // 5 minutes
    bool enable_deep_sleep_cellular = true;
};

class CellularNetwork {
public:
    bool initCellular(CellularConfig config);
    bool connectToCellular();
    bool sendCellularData(CellularPacket packet);
    bool checkSignalStrength();
    bool optimizeConnection();
    
    // Data management
    bool compressData(uint8_t* data, size_t& length);
    bool queueDataForTransmission(CellularPacket packet);
    bool batchTransmissions();
    
    // Power optimization
    bool enterPowerSaveMode();
    bool exitPowerSaveMode();
    bool scheduleTransmissions();
    
private:
    CellularModem cellular_modem;
    DataCompressor data_compressor;
    TransmissionQueue transmission_queue;
    PowerManager power_manager;
};
```

## Network Optimization Strategies

### Adaptive Protocol Selection
```cpp
// Intelligent protocol selection based on conditions
class AdaptiveProtocolSelector {
public:
    Protocol selectOptimalProtocol(NetworkConditions conditions) {
        // Evaluate each available protocol
        std::map<Protocol, float> protocol_scores;
        
        for (auto protocol : available_protocols) {
            float score = evaluateProtocol(protocol, conditions);
            protocol_scores[protocol] = score;
        }
        
        // Select highest scoring protocol
        return getBestProtocol(protocol_scores);
    }
    
private:
    float evaluateProtocol(Protocol protocol, NetworkConditions conditions) {
        float score = 0.0;
        
        // Signal strength factor
        score += conditions.signal_strength[protocol] * 0.3;
        
        // Power consumption factor (lower is better)
        score += (1.0 - conditions.power_consumption[protocol]) * 0.25;
        
        // Data rate factor
        score += conditions.data_rate[protocol] * 0.2;
        
        // Reliability factor
        score += conditions.reliability[protocol] * 0.15;
        
        // Cost factor (lower is better)
        score += (1.0 - conditions.cost[protocol]) * 0.1;
        
        return score;
    }
};
```

### Load Balancing
```cpp
// Network load balancing across multiple paths/protocols
class NetworkLoadBalancer {
public:
    bool distributeLoad(std::vector<DataPacket> packets) {
        // Assess current load on each path
        std::map<NetworkPath, float> path_loads;
        for (auto path : available_paths) {
            path_loads[path] = assessPathLoad(path);
        }
        
        // Distribute packets based on path capacity
        for (auto packet : packets) {
            NetworkPath optimal_path = selectLeastLoadedPath(path_loads);
            sendViaPath(packet, optimal_path);
            updatePathLoad(optimal_path, packet.size);
        }
        
        return true;
    }
    
private:
    float assessPathLoad(NetworkPath path) {
        // Consider bandwidth utilization, queue length, latency
        float bandwidth_utilization = path.getBandwidthUtilization();
        float queue_load = path.getQueueLoad();
        float latency = path.getAverageLatency();
        
        return (bandwidth_utilization * 0.5) + (queue_load * 0.3) + (latency * 0.2);
    }
};
```

### Quality of Service (QoS)
```cpp
// QoS implementation for wildlife data prioritization
enum DataPriority {
    PRIORITY_EMERGENCY = 1,      // Conservation alerts
    PRIORITY_HIGH = 2,           // Research data
    PRIORITY_NORMAL = 3,         // Regular monitoring
    PRIORITY_LOW = 4             // System maintenance
};

struct QoSConfig {
    std::map<DataPriority, uint32_t> bandwidth_allocation = {
        {PRIORITY_EMERGENCY, 50},    // 50% for emergency alerts
        {PRIORITY_HIGH, 30},         // 30% for research data
        {PRIORITY_NORMAL, 15},       // 15% for regular monitoring
        {PRIORITY_LOW, 5}            // 5% for maintenance
    };
    
    std::map<DataPriority, uint32_t> max_latency_ms = {
        {PRIORITY_EMERGENCY, 1000},  // 1 second max latency
        {PRIORITY_HIGH, 10000},      // 10 seconds max latency
        {PRIORITY_NORMAL, 60000},    // 1 minute max latency
        {PRIORITY_LOW, 300000}       // 5 minutes max latency
    };
    
    bool enable_traffic_shaping = true;
    bool enable_priority_queuing = true;
    bool enable_admission_control = true;
};

class QoSManager {
public:
    bool classifyTraffic(DataPacket& packet);
    bool applyQoSPolicy(DataPacket packet);
    bool manageTrafficQueues();
    bool enforceLatencyRequirements();
    
    // QoS monitoring
    QoSMetrics getQoSMetrics();
    bool adaptQoSPolicy(NetworkConditions conditions);
    
private:
    std::map<DataPriority, PacketQueue> priority_queues;
    TrafficShaper traffic_shaper;
    LatencyMonitor latency_monitor;
    BandwidthManager bandwidth_manager;
};
```

## Deployment Examples

### Research Deployment Network
```cpp
// Large-scale research deployment network configuration
void configureResearchNetwork() {
    // Hybrid network with mesh backbone
    HybridNetworkConfig research_config;
    research_config.lora_enabled = true;      // Primary mesh backbone
    research_config.cellular_enabled = true;  // Data uplink
    research_config.satellite_enabled = true; // Emergency backup
    research_config.adaptive_protocol_selection = true;
    
    // LoRa mesh for local communication
    LoRaMeshConfig lora_config;
    lora_config.spreading_factor = 8;         // Good range/speed balance
    lora_config.tx_power = 17;                // Higher power for research
    lora_config.max_retransmissions = 5;      // Reliable delivery
    
    // Cellular for research data uplink
    CellularConfig cellular_config;
    cellular_config.enable_compression = true;
    cellular_config.max_packet_size = 1500;
    cellular_config.enable_power_save = true;
    
    // QoS for research priorities
    QoSConfig qos_config;
    qos_config.bandwidth_allocation[PRIORITY_HIGH] = 60; // Research priority
    qos_config.enable_priority_queuing = true;
    
    initHybridNetwork(research_config);
}
```

### Budget Conservation Network
```cpp
// Cost-effective conservation network
void configureBudgetNetwork() {
    // Simple star network with LoRa
    StarNetworkConfig budget_config;
    budget_config.is_hub_node = false;        // Most nodes are leaf nodes
    budget_config.reporting_interval = 600000; // 10 minutes to save power
    budget_config.store_and_forward = true;   // Cache data locally
    
    // Basic LoRa configuration
    LoRaMeshConfig lora_config;
    lora_config.spreading_factor = 10;        // Longer range, lower data rate
    lora_config.tx_power = 14;                // Standard power
    lora_config.max_retransmissions = 3;      // Basic reliability
    
    initStarNetwork(budget_config);
}
```

### Urban Monitoring Network
```cpp
// Urban deployment with stealth and redundancy
void configureUrbanNetwork() {
    // WiFi mesh with cellular backup
    HybridNetworkConfig urban_config;
    urban_config.wifi_enabled = true;         // Primary for urban areas
    urban_config.cellular_enabled = true;     // Backup/uplink
    urban_config.lora_enabled = false;        // May interfere in urban RF
    urban_config.adaptive_protocol_selection = true;
    
    // WiFi mesh configuration
    WiFiMeshConfig wifi_config;
    wifi_config.channel = 6;                  // Less congested channel
    wifi_config.enable_mesh_ie_crypto = true; // Security for urban
    wifi_config.max_connection = 6;           // Limited connections
    
    // High-priority emergency alerts
    QoSConfig qos_config;
    qos_config.bandwidth_allocation[PRIORITY_EMERGENCY] = 70;
    qos_config.max_latency_ms[PRIORITY_EMERGENCY] = 500; // 0.5 seconds
    
    initHybridNetwork(urban_config);
}
```

## Network Monitoring and Diagnostics

### Network Health Monitoring
```cpp
struct NetworkHealthMetrics {
    // Connectivity metrics
    float network_uptime_percentage = 0.0;
    uint32_t successful_transmissions = 0;
    uint32_t failed_transmissions = 0;
    float packet_loss_rate = 0.0;
    
    // Performance metrics
    uint32_t average_latency_ms = 0;
    uint32_t peak_latency_ms = 0;
    float throughput_kbps = 0.0;
    float network_efficiency = 0.0;
    
    // Quality metrics
    float signal_strength_dbm = 0.0;
    float signal_to_noise_ratio = 0.0;
    uint32_t bit_error_rate = 0;
    float jitter_ms = 0.0;
    
    // Topology metrics
    uint8_t active_nodes = 0;
    uint8_t network_diameter = 0;          // Maximum hops
    float network_density = 0.0;           // Connectivity ratio
    uint8_t critical_nodes = 0;            // Single points of failure
};

class NetworkHealthMonitor {
public:
    void updateHealthMetrics();
    NetworkHealthMetrics getCurrentHealth();
    bool detectNetworkIssues();
    std::vector<String> getDiagnosticRecommendations();
    
    // Proactive monitoring
    bool predictNetworkFailures();
    bool recommendTopologyChanges();
    bool suggestPerformanceOptimizations();
    
private:
    HealthMetricsCollector metrics_collector;
    IssueDetector issue_detector;
    PredictiveAnalyzer predictive_analyzer;
    RecommendationEngine recommendation_engine;
};
```

### Network Troubleshooting Tools
```cpp
class NetworkDiagnostics {
public:
    // Connectivity testing
    bool pingTest(uint8_t target_node_id);
    bool traceroute(uint8_t target_node_id);
    bool bandwidthTest(uint8_t target_node_id);
    bool latencyTest(uint8_t target_node_id);
    
    // Signal analysis
    float measureSignalStrength(uint8_t target_node_id);
    float measureSignalQuality(uint8_t target_node_id);
    bool analyzeInterference();
    bool scanAvailableChannels();
    
    // Topology analysis
    std::vector<uint8_t> discoverActiveNodes();
    TopologyMap generateTopologyMap();
    std::vector<uint8_t> findCriticalNodes();
    std::vector<NetworkPath> findRedundantPaths();
    
    // Performance analysis
    ThroughputMetrics measureThroughput();
    LatencyDistribution analyzeLatency();
    PacketLossAnalysis analyzePacketLoss();
    JitterAnalysis analyzeJitter();
    
private:
    ConnectivityTester connectivity_tester;
    SignalAnalyzer signal_analyzer;
    TopologyAnalyzer topology_analyzer;
    PerformanceAnalyzer performance_analyzer;
};
```

---

*Network topology selection and optimization is critical for successful wildlife monitoring deployments. The choice depends on deployment scale, environmental constraints, power budgets, and data requirements. Proper network design ensures reliable data collection while minimizing power consumption and maintenance requirements.*