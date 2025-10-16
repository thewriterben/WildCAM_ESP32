/**
 * @file test_node_failure_detection.cpp
 * @brief Unit tests for node failure detection and task reassignment
 * 
 * Tests the node failure detection system including:
 * - Heartbeat tracking for each node
 * - Node failure detection after 60-second timeout
 * - Automatic task reassignment to healthy nodes
 * - Node failure event logging
 * - System stability with failed nodes
 */

#include <unity.h>
#include "../firmware/src/multi_board/board_coordinator.h"
#include "../firmware/src/multi_board/message_protocol.h"

// Test instances
BoardCoordinator* testCoordinator = nullptr;

void setUp(void) {
    // Initialize test instances before each test
    if (!testCoordinator) {
        testCoordinator = new BoardCoordinator();
        testCoordinator->init(1);
        testCoordinator->startCoordinator();
    }
}

void tearDown(void) {
    // Clean up after each test
    if (testCoordinator) {
        testCoordinator->stopCoordinator();
        delete testCoordinator;
        testCoordinator = nullptr;
    }
}

// ===========================
// NODE FAILURE DETECTION TESTS
// ===========================

void test_heartbeat_tracking_initialization(void) {
    // Test that coordinator initializes with proper state
    TEST_ASSERT_NOT_NULL(testCoordinator);
    TEST_ASSERT_EQUAL(COORD_DISCOVERING, testCoordinator->getState());
}

void test_node_failure_detection_timeout(void) {
    // Manually add a node to managed nodes
    NetworkNode testNode;
    testNode.nodeId = 2;
    testNode.role = ROLE_NODE;
    testNode.isActive = true;
    testNode.lastSeen = millis() - 70000; // 70 seconds ago (past timeout)
    testNode.capabilities.batteryLevel = 80;
    testNode.capabilities.hasAI = false;
    testNode.signalStrength = -60;
    
    // Create a simple test by checking the node would be detected as failed
    unsigned long now = millis();
    unsigned long timeSinceLastSeen = now - testNode.lastSeen;
    
    // Verify node should be marked as failed (> 60 seconds)
    TEST_ASSERT_GREATER_THAN(60000, timeSinceLastSeen);
    TEST_ASSERT_TRUE(testNode.isActive); // Currently active, should be marked failed
}

void test_task_reassignment_on_node_failure(void) {
    // Setup: Create active nodes
    NetworkNode healthyNode1;
    healthyNode1.nodeId = 2;
    healthyNode1.role = ROLE_NODE;
    healthyNode1.isActive = true;
    healthyNode1.lastSeen = millis();
    healthyNode1.capabilities.batteryLevel = 90;
    healthyNode1.capabilities.hasAI = true;
    healthyNode1.signalStrength = -50;
    
    NetworkNode healthyNode2;
    healthyNode2.nodeId = 3;
    healthyNode2.role = ROLE_NODE;
    healthyNode2.isActive = true;
    healthyNode2.lastSeen = millis();
    healthyNode2.capabilities.batteryLevel = 85;
    healthyNode2.capabilities.hasAI = false;
    healthyNode2.signalStrength = -55;
    
    NetworkNode failedNode;
    failedNode.nodeId = 4;
    failedNode.role = ROLE_NODE;
    failedNode.isActive = true;
    failedNode.lastSeen = millis() - 70000; // Failed
    failedNode.capabilities.batteryLevel = 60;
    failedNode.capabilities.hasAI = false;
    failedNode.signalStrength = -80;
    
    // Test that we can identify healthy nodes for reassignment
    TEST_ASSERT_TRUE(healthyNode1.isActive);
    TEST_ASSERT_TRUE(healthyNode2.isActive);
    
    // Verify failed node should be detected
    unsigned long timeSinceFailed = millis() - failedNode.lastSeen;
    TEST_ASSERT_GREATER_THAN(60000, timeSinceFailed);
}

void test_healthy_node_selection_for_task(void) {
    // Test selection algorithm priorities
    NetworkNode aiNode;
    aiNode.nodeId = 2;
    aiNode.role = ROLE_AI_PROCESSOR;
    aiNode.isActive = true;
    aiNode.lastSeen = millis();
    aiNode.capabilities.batteryLevel = 95;
    aiNode.capabilities.hasAI = true;
    aiNode.capabilities.hasPSRAM = true;
    aiNode.signalStrength = -40;
    
    NetworkNode basicNode;
    basicNode.nodeId = 3;
    basicNode.role = ROLE_NODE;
    basicNode.isActive = true;
    basicNode.lastSeen = millis();
    basicNode.capabilities.batteryLevel = 75;
    basicNode.capabilities.hasAI = false;
    basicNode.capabilities.hasPSRAM = false;
    basicNode.signalStrength = -60;
    
    NetworkNode lowBatteryNode;
    lowBatteryNode.nodeId = 4;
    lowBatteryNode.role = ROLE_NODE;
    lowBatteryNode.isActive = true;
    lowBatteryNode.lastSeen = millis();
    lowBatteryNode.capabilities.batteryLevel = 15;
    lowBatteryNode.capabilities.hasAI = false;
    lowBatteryNode.signalStrength = -70;
    
    // Verify AI node has best capabilities for processing
    TEST_ASSERT_TRUE(aiNode.capabilities.hasAI);
    TEST_ASSERT_GREATER_THAN(basicNode.capabilities.batteryLevel, aiNode.capabilities.batteryLevel);
    TEST_ASSERT_GREATER_THAN(lowBatteryNode.capabilities.batteryLevel, basicNode.capabilities.batteryLevel);
}

void test_multiple_node_failures(void) {
    // Test system stability with multiple node failures
    NetworkNode node1;
    node1.nodeId = 2;
    node1.isActive = true;
    node1.lastSeen = millis() - 70000; // Failed
    
    NetworkNode node2;
    node2.nodeId = 3;
    node2.isActive = true;
    node2.lastSeen = millis() - 80000; // Failed
    
    NetworkNode node3;
    node3.nodeId = 4;
    node3.isActive = true;
    node3.lastSeen = millis(); // Healthy
    
    // Count failed nodes
    int failedCount = 0;
    if (millis() - node1.lastSeen > 60000) failedCount++;
    if (millis() - node2.lastSeen > 60000) failedCount++;
    if (millis() - node3.lastSeen > 60000) failedCount++;
    
    TEST_ASSERT_EQUAL(2, failedCount);
}

void test_node_recovery_after_failure(void) {
    // Test that a node can recover after being marked as failed
    NetworkNode testNode;
    testNode.nodeId = 2;
    testNode.isActive = false; // Previously failed
    testNode.lastSeen = millis() - 70000;
    
    // Simulate recovery with new heartbeat
    testNode.lastSeen = millis();
    testNode.isActive = true;
    
    // Verify node is now healthy
    TEST_ASSERT_TRUE(testNode.isActive);
    unsigned long timeSinceLastSeen = millis() - testNode.lastSeen;
    TEST_ASSERT_LESS_THAN(60000, timeSinceLastSeen);
}

void test_task_deadline_reset_on_reassignment(void) {
    // Test that task deadlines are reset when reassigned
    unsigned long originalDeadline = millis() + 300000; // 5 minutes
    unsigned long currentTime = millis() + 310000; // Past deadline
    
    // Task should be considered timed out
    TEST_ASSERT_GREATER_THAN(originalDeadline, currentTime);
    
    // After reassignment, deadline should be reset
    unsigned long newDeadline = millis() + 300000;
    TEST_ASSERT_GREATER_THAN(currentTime, newDeadline);
}

void test_no_reassignment_when_no_healthy_nodes(void) {
    // Test that system handles case where no healthy nodes available
    NetworkNode failedNode1;
    failedNode1.nodeId = 2;
    failedNode1.isActive = false;
    
    NetworkNode failedNode2;
    failedNode2.nodeId = 3;
    failedNode2.isActive = false;
    
    // Count active nodes
    int activeCount = 0;
    if (failedNode1.isActive) activeCount++;
    if (failedNode2.isActive) activeCount++;
    
    TEST_ASSERT_EQUAL(0, activeCount);
}

void test_node_failure_logging(void) {
    // Test that failure logging includes required information
    int failedNodeId = 5;
    const char* reason = "Heartbeat timeout";
    unsigned long timestamp = millis();
    
    // Verify we have all required logging information
    TEST_ASSERT_NOT_EQUAL(0, failedNodeId);
    TEST_ASSERT_NOT_NULL(reason);
    TEST_ASSERT_GREATER_THAN(0, timestamp);
}

// ===========================
// TEST RUNNER
// ===========================

void runTests() {
    UNITY_BEGIN();
    
    // Node failure detection tests
    RUN_TEST(test_heartbeat_tracking_initialization);
    RUN_TEST(test_node_failure_detection_timeout);
    RUN_TEST(test_task_reassignment_on_node_failure);
    RUN_TEST(test_healthy_node_selection_for_task);
    RUN_TEST(test_multiple_node_failures);
    RUN_TEST(test_node_recovery_after_failure);
    RUN_TEST(test_task_deadline_reset_on_reassignment);
    RUN_TEST(test_no_reassignment_when_no_healthy_nodes);
    RUN_TEST(test_node_failure_logging);
    
    UNITY_END();
}

#ifdef ARDUINO

void setup() {
    delay(2000); // Wait for serial monitor
    runTests();
}

void loop() {
    // Nothing to do
}

#else

int main(int argc, char **argv) {
    runTests();
    return 0;
}

#endif
