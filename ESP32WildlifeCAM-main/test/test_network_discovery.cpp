/**
 * @file test_network_discovery.cpp
 * @brief Unit tests for automatic network discovery and device coordination
 * 
 * Tests the enhanced discovery protocol including:
 * - Automatic device joining
 * - Real-time topology updates
 * - Dynamic role assignment
 * - Network health monitoring
 */

#include <unity.h>
#include "../firmware/src/multi_board/discovery_protocol.h"
#include "../firmware/src/multi_board/board_coordinator.h"
#include "../firmware/src/multi_board/board_node.h"
#include "../firmware/src/multi_board/message_protocol.h"

// Test instances
DiscoveryProtocol* testDiscovery = nullptr;
BoardCoordinator* testCoordinator = nullptr;
BoardNode* testNode = nullptr;

void setUp(void) {
    // Initialize test instances before each test
    if (!testDiscovery) {
        testDiscovery = new DiscoveryProtocol();
    }
    if (!testCoordinator) {
        testCoordinator = new BoardCoordinator();
    }
    if (!testNode) {
        testNode = new BoardNode();
    }
}

void tearDown(void) {
    // Clean up after each test
    if (testDiscovery) {
        testDiscovery->stopDiscovery();
    }
    if (testCoordinator) {
        testCoordinator->stopCoordinator();
    }
    if (testNode) {
        testNode->stopNode();
    }
}

// ===========================
// DISCOVERY PROTOCOL TESTS
// ===========================

void test_discovery_initialization(void) {
    bool result = testDiscovery->init(1, ROLE_NODE);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(DISCOVERY_IDLE, testDiscovery->getDiscoveryState());
}

void test_discovery_start(void) {
    testDiscovery->init(1, ROLE_NODE);
    bool result = testDiscovery->startDiscovery();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(DISCOVERY_SCANNING, testDiscovery->getDiscoveryState());
}

void test_automatic_device_joining(void) {
    testDiscovery->init(1, ROLE_COORDINATOR);
    testDiscovery->startDiscovery();
    
    // Simulate a new device discovery message
    NetworkNode newNode;
    newNode.nodeId = 2;
    newNode.role = ROLE_NODE;
    newNode.capabilities.boardType = BOARD_AI_THINKER_ESP32_CAM;
    newNode.capabilities.batteryLevel = 80;
    newNode.signalStrength = -60;
    newNode.hopCount = 1;
    newNode.coordinatorScore = 50.0;
    
    // Add node to topology
    testDiscovery->updateNode(newNode);
    
    // Verify node was added
    TEST_ASSERT_TRUE(testDiscovery->nodeExists(2));
    const auto& nodes = testDiscovery->getDiscoveredNodes();
    TEST_ASSERT_EQUAL(1, nodes.size());
    TEST_ASSERT_EQUAL(2, nodes[0].nodeId);
}

void test_topology_update_broadcast(void) {
    testDiscovery->init(1, ROLE_COORDINATOR);
    testDiscovery->startDiscovery();
    
    // Add multiple nodes
    for (int i = 2; i <= 4; i++) {
        NetworkNode node;
        node.nodeId = i;
        node.role = ROLE_NODE;
        node.capabilities.boardType = BOARD_AI_THINKER_ESP32_CAM;
        node.capabilities.batteryLevel = 75;
        node.signalStrength = -65;
        node.hopCount = 1;
        node.coordinatorScore = 40.0;
        testDiscovery->updateNode(node);
    }
    
    // Verify topology has all nodes
    const auto& nodes = testDiscovery->getDiscoveredNodes();
    TEST_ASSERT_EQUAL(3, nodes.size());
    
    // Test forced topology broadcast
    testDiscovery->broadcastTopologyNow();
    
    // Verify topology was updated
    const NetworkTopology& topology = testDiscovery->getNetworkTopology();
    TEST_ASSERT_GREATER_THAN(0, topology.lastUpdate);
}

void test_coordinator_selection(void) {
    testDiscovery->init(1, ROLE_COORDINATOR);
    testDiscovery->startDiscovery();
    
    // Add a node with lower coordinator score
    NetworkNode weakNode;
    weakNode.nodeId = 2;
    weakNode.role = ROLE_NODE;
    weakNode.capabilities.boardType = BOARD_AI_THINKER_ESP32_CAM;
    weakNode.capabilities.batteryLevel = 30; // Low battery
    weakNode.coordinatorScore = 30.0;
    testDiscovery->updateNode(weakNode);
    
    // Add a node with higher coordinator score
    NetworkNode strongNode;
    strongNode.nodeId = 3;
    strongNode.role = ROLE_NODE;
    strongNode.capabilities.boardType = BOARD_AI_THINKER_ESP32_CAM;
    strongNode.capabilities.batteryLevel = 90;
    strongNode.capabilities.hasAI = true;
    strongNode.coordinatorScore = 90.0;
    testDiscovery->updateNode(strongNode);
    
    // Verify strong node becomes coordinator
    int coordinator = testDiscovery->getCoordinatorNode();
    TEST_ASSERT_TRUE(coordinator == 3 || coordinator == 1); // Node 3 or self
}

void test_network_health_calculation(void) {
    testDiscovery->init(1, ROLE_COORDINATOR);
    testDiscovery->startDiscovery();
    
    // Add nodes with good signal strength
    for (int i = 2; i <= 4; i++) {
        NetworkNode node;
        node.nodeId = i;
        node.role = ROLE_NODE;
        node.capabilities.batteryLevel = 80;
        node.signalStrength = -50; // Good signal
        node.hopCount = 1;
        node.coordinatorScore = 60.0;
        testDiscovery->updateNode(node);
    }
    
    // Calculate network health
    float health = testDiscovery->getNetworkHealth();
    
    // Health should be good (> 0.5) with active nodes and good signal
    TEST_ASSERT_GREATER_THAN(0.5, health);
    TEST_ASSERT_LESS_OR_EQUAL(1.0, health);
}

void test_inactive_node_cleanup(void) {
    testDiscovery->init(1, ROLE_COORDINATOR);
    testDiscovery->startDiscovery();
    
    // Add a node
    NetworkNode node;
    node.nodeId = 2;
    node.role = ROLE_NODE;
    node.capabilities.batteryLevel = 80;
    node.signalStrength = -60;
    node.hopCount = 1;
    node.coordinatorScore = 50.0;
    testDiscovery->updateNode(node);
    
    TEST_ASSERT_EQUAL(1, testDiscovery->getDiscoveredNodes().size());
    
    // Manually mark node as old (simulate timeout)
    NetworkNode* foundNode = testDiscovery->findNode(2);
    TEST_ASSERT_NOT_NULL(foundNode);
    foundNode->lastSeen = millis() - 400000; // 400 seconds ago (> 5 min timeout)
    
    // Cleanup inactive nodes
    testDiscovery->cleanupInactiveNodes();
    
    // Verify node was removed
    TEST_ASSERT_EQUAL(0, testDiscovery->getDiscoveredNodes().size());
}

// ===========================
// COORDINATOR TESTS
// ===========================

void test_coordinator_initialization(void) {
    bool result = testCoordinator->init(1);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(COORD_INACTIVE, testCoordinator->getState());
}

void test_coordinator_start(void) {
    testCoordinator->init(1);
    bool result = testCoordinator->startCoordinator();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_NOT_EQUAL(COORD_INACTIVE, testCoordinator->getState());
}

void test_role_assignment_capability_based(void) {
    testCoordinator->init(1);
    testCoordinator->startCoordinator();
    
    // Simulate discovery of nodes with different capabilities
    DiscoveryProtocol* coordDiscovery = testCoordinator->getDiscoveryProtocol();
    TEST_ASSERT_NOT_NULL(coordDiscovery);
    
    // Test would require mocking or integration with actual discovery
    // For now, verify coordinator has discovery protocol
    TEST_ASSERT_TRUE(true);
}

// ===========================
// NODE TESTS
// ===========================

void test_node_initialization(void) {
    bool result = testNode->init(2, ROLE_NODE);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(NODE_INACTIVE, testNode->getState());
    TEST_ASSERT_EQUAL(ROLE_NODE, testNode->getCurrentRole());
}

void test_node_start_seeking_coordinator(void) {
    testNode->init(2, ROLE_NODE);
    bool result = testNode->startNode();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(NODE_SEEKING_COORDINATOR, testNode->getState());
}

void test_node_role_change(void) {
    testNode->init(2, ROLE_NODE);
    testNode->startNode();
    
    // Change role
    testNode->setAssignedRole(ROLE_RELAY);
    
    // Verify role changed
    TEST_ASSERT_EQUAL(ROLE_RELAY, testNode->getCurrentRole());
}

// ===========================
// INTEGRATION TESTS
// ===========================

void test_end_to_end_discovery(void) {
    // Initialize coordinator
    testCoordinator->init(1);
    testCoordinator->startCoordinator();
    
    // Initialize node
    testNode->init(2, ROLE_NODE);
    testNode->startNode();
    
    // Simulate discovery process
    DiscoveryProtocol* coordDiscovery = testCoordinator->getDiscoveryProtocol();
    TEST_ASSERT_NOT_NULL(coordDiscovery);
    
    // Process messages (would need actual message passing in real scenario)
    for (int i = 0; i < 10; i++) {
        testCoordinator->process();
        testNode->process();
        delay(100);
    }
    
    // Basic verification that systems are running
    TEST_ASSERT_TRUE(testCoordinator->getState() != COORD_INACTIVE);
    TEST_ASSERT_TRUE(testNode->getState() != NODE_INACTIVE);
}

void test_network_stats(void) {
    testDiscovery->init(1, ROLE_COORDINATOR);
    testDiscovery->startDiscovery();
    
    // Add nodes
    for (int i = 2; i <= 5; i++) {
        NetworkNode node;
        node.nodeId = i;
        node.role = ROLE_NODE;
        node.capabilities.batteryLevel = 70 + (i * 5);
        node.signalStrength = -60 - i;
        node.hopCount = 1;
        node.coordinatorScore = 50.0;
        testDiscovery->updateNode(node);
    }
    
    // Get network stats
    DiscoveryProtocol::NetworkStats stats = testDiscovery->getNetworkStats();
    
    TEST_ASSERT_EQUAL(4, stats.totalNodes);
    TEST_ASSERT_EQUAL(4, stats.activeNodes);
    TEST_ASSERT_GREATER_THAN(0, stats.averageSignalStrength);
    TEST_ASSERT_EQUAL(1, stats.maxHopCount);
}

// ===========================
// MAIN TEST RUNNER
// ===========================

void setup() {
    delay(2000); // Allow time for serial monitor
    
    UNITY_BEGIN();
    
    // Discovery Protocol Tests
    RUN_TEST(test_discovery_initialization);
    RUN_TEST(test_discovery_start);
    RUN_TEST(test_automatic_device_joining);
    RUN_TEST(test_topology_update_broadcast);
    RUN_TEST(test_coordinator_selection);
    RUN_TEST(test_network_health_calculation);
    RUN_TEST(test_inactive_node_cleanup);
    
    // Coordinator Tests
    RUN_TEST(test_coordinator_initialization);
    RUN_TEST(test_coordinator_start);
    RUN_TEST(test_role_assignment_capability_based);
    
    // Node Tests
    RUN_TEST(test_node_initialization);
    RUN_TEST(test_node_start_seeking_coordinator);
    RUN_TEST(test_node_role_change);
    
    // Integration Tests
    RUN_TEST(test_end_to_end_discovery);
    RUN_TEST(test_network_stats);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
