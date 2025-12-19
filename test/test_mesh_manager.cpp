/**
 * @file test_mesh_manager.cpp
 * @brief Unit tests for MeshManager LoRa mesh networking
 * @version 1.0.0
 * 
 * Tests the mesh networking functionality including:
 * - Initialization and cleanup
 * - Node configuration
 * - Network status
 * - Signal quality classification
 * - Packet structure validation
 */

#include <unity.h>
#include "MeshManager.h"
#include "config.h"

// Test instance
MeshManager testMesh;

void setUp(void) {
    // Reset test instance state before each test
}

void tearDown(void) {
    // Cleanup after each test
}

//==============================================================================
// INITIALIZATION TESTS
//==============================================================================

void test_mesh_manager_initial_state(void) {
    MeshManager mesh;
    
    // Should not be initialized by default
    TEST_ASSERT_FALSE(mesh.isInitialized());
    
    // Node ID should be 0 before init
    TEST_ASSERT_EQUAL_UINT32(0, mesh.getNodeId());
    
    // Should not be coordinator
    TEST_ASSERT_FALSE(mesh.isCoordinator());
    
    // Coordinator ID should be 0
    TEST_ASSERT_EQUAL_UINT32(0, mesh.getCoordinatorId());
}

void test_mesh_manager_node_id_setting(void) {
    MeshManager mesh;
    
    // Set a specific node ID
    mesh.setNodeId(0x12345678);
    TEST_ASSERT_EQUAL_UINT32(0x12345678, mesh.getNodeId());
    
    // Set another ID
    mesh.setNodeId(0xABCDEF01);
    TEST_ASSERT_EQUAL_UINT32(0xABCDEF01, mesh.getNodeId());
}

void test_mesh_manager_node_name(void) {
    MeshManager mesh;
    
    // Set node name
    mesh.setNodeName("TestCam");
    TEST_ASSERT_EQUAL_STRING("TestCam", mesh.getNodeName().c_str());
    
    // Test truncation to 8 chars
    mesh.setNodeName("VeryLongNodeName");
    TEST_ASSERT_EQUAL_STRING("VeryLong", mesh.getNodeName().c_str());
    
    // Test short name
    mesh.setNodeName("CAM1");
    TEST_ASSERT_EQUAL_STRING("CAM1", mesh.getNodeName().c_str());
}

//==============================================================================
// NETWORK STATUS TESTS
//==============================================================================

void test_mesh_network_status_uninitialized(void) {
    MeshManager mesh;
    
    MeshNetworkStatus status = mesh.getNetworkStatus();
    
    TEST_ASSERT_FALSE(status.initialized);
    TEST_ASSERT_EQUAL_UINT32(0, status.nodeId);
    TEST_ASSERT_EQUAL(MESH_ROLE_NODE, status.role);
    TEST_ASSERT_EQUAL_UINT16(0, status.connectedNodes);
    TEST_ASSERT_FALSE(status.isCoordinator);
    TEST_ASSERT_EQUAL_UINT32(0, status.packetsReceived);
    TEST_ASSERT_EQUAL_UINT32(0, status.packetsSent);
}

void test_mesh_signal_quality_classification(void) {
    MeshManager mesh;
    
    SignalQuality quality;
    
    // Test excellent signal (RSSI > -70)
    quality = mesh.getSignalQuality();
    // Default RSSI is 0, which would be excellent
    TEST_ASSERT_EQUAL(SIGNAL_EXCELLENT, quality.strength);
}

void test_mesh_node_count(void) {
    MeshManager mesh;
    
    // No nodes initially
    TEST_ASSERT_EQUAL_UINT16(0, mesh.getNodeCount());
    
    // Get nodes list
    auto nodes = mesh.getNodes();
    TEST_ASSERT_EQUAL(0, nodes.size());
}

//==============================================================================
// COORDINATOR TESTS
//==============================================================================

void test_mesh_become_coordinator_without_init(void) {
    MeshManager mesh;
    
    // Should fail if not initialized
    TEST_ASSERT_FALSE(mesh.becomeCoordinator());
    TEST_ASSERT_FALSE(mesh.isCoordinator());
}

//==============================================================================
// ROUTING TABLE TESTS
//==============================================================================

void test_mesh_routing_table_empty(void) {
    MeshManager mesh;
    
    auto routes = mesh.getRoutingTable();
    TEST_ASSERT_EQUAL(0, routes.size());
    
    // No route to any destination
    TEST_ASSERT_EQUAL_UINT32(0, mesh.getNextHop(0x12345678));
}

//==============================================================================
// POWER MANAGEMENT TESTS
//==============================================================================

void test_mesh_low_power_mode(void) {
    MeshManager mesh;
    
    // Not in low power mode initially
    TEST_ASSERT_FALSE(mesh.isLowPowerMode());
    
    // Enter low power mode
    mesh.enterLowPowerMode();
    TEST_ASSERT_TRUE(mesh.isLowPowerMode());
    
    // Exit low power mode
    mesh.exitLowPowerMode();
    TEST_ASSERT_FALSE(mesh.isLowPowerMode());
}

//==============================================================================
// MESSAGE TRANSMISSION TESTS (without hardware)
//==============================================================================

void test_mesh_send_message_uninitialized(void) {
    MeshManager mesh;
    
    uint8_t testData[] = {0x01, 0x02, 0x03};
    
    // Should fail when not initialized
    TEST_ASSERT_FALSE(mesh.sendMessage(0x12345678, testData, sizeof(testData)));
    TEST_ASSERT_FALSE(mesh.broadcastMessage(testData, sizeof(testData)));
    TEST_ASSERT_FALSE(mesh.sendText(0x12345678, "Hello"));
}

void test_mesh_send_wildlife_event_uninitialized(void) {
    MeshManager mesh;
    
    WildlifeEvent event;
    event.timestamp = millis();
    event.nodeId = 0x12345678;
    event.species = "Deer";
    event.confidence = 0.95f;
    event.hasImage = true;
    
    // Should fail when not initialized
    TEST_ASSERT_FALSE(mesh.sendWildlifeEvent(event));
}

void test_mesh_send_image_uninitialized(void) {
    MeshManager mesh;
    
    uint8_t fakeImage[] = {0xFF, 0xD8, 0xFF, 0xE0};  // JPEG header bytes
    
    // Should fail when not initialized
    TEST_ASSERT_FALSE(mesh.sendImage(fakeImage, sizeof(fakeImage), "test.jpg"));
}

//==============================================================================
// CALLBACK TESTS
//==============================================================================

static bool callbackCalled = false;
static uint32_t lastCallbackNodeId = 0;

void testMessageCallback(uint32_t fromNode, const uint8_t* data, size_t length) {
    callbackCalled = true;
    lastCallbackNodeId = fromNode;
}

void testWildlifeCallback(const WildlifeEvent& event) {
    callbackCalled = true;
    lastCallbackNodeId = event.nodeId;
}

void test_mesh_callback_registration(void) {
    MeshManager mesh;
    
    callbackCalled = false;
    lastCallbackNodeId = 0;
    
    // Should be able to set callbacks without crashing
    mesh.setMessageCallback(testMessageCallback);
    mesh.setWildlifeCallback(testWildlifeCallback);
    mesh.setNodeCallback(nullptr);
    
    // Callbacks should not have been triggered
    TEST_ASSERT_FALSE(callbackCalled);
}

//==============================================================================
// STRUCTURE VALIDATION TESTS
//==============================================================================

void test_signal_strength_enum_values(void) {
    TEST_ASSERT_EQUAL(0, SIGNAL_POOR);
    TEST_ASSERT_EQUAL(1, SIGNAL_FAIR);
    TEST_ASSERT_EQUAL(2, SIGNAL_GOOD);
    TEST_ASSERT_EQUAL(3, SIGNAL_EXCELLENT);
}

void test_mesh_role_enum_values(void) {
    TEST_ASSERT_EQUAL(0, MESH_ROLE_NODE);
    TEST_ASSERT_EQUAL(1, MESH_ROLE_COORDINATOR);
    TEST_ASSERT_EQUAL(2, MESH_ROLE_GATEWAY);
    TEST_ASSERT_EQUAL(3, MESH_ROLE_REPEATER);
}

void test_mesh_priority_enum_values(void) {
    TEST_ASSERT_EQUAL(0, PRIORITY_LOW);
    TEST_ASSERT_EQUAL(1, PRIORITY_NORMAL);
    TEST_ASSERT_EQUAL(2, PRIORITY_HIGH);
    TEST_ASSERT_EQUAL(3, PRIORITY_EMERGENCY);
}

void test_mesh_packet_type_enum_values(void) {
    TEST_ASSERT_EQUAL(0x01, PACKET_BEACON);
    TEST_ASSERT_EQUAL(0x02, PACKET_DATA);
    TEST_ASSERT_EQUAL(0x03, PACKET_ACK);
    TEST_ASSERT_EQUAL(0x04, PACKET_ROUTING);
    TEST_ASSERT_EQUAL(0x05, PACKET_WILDLIFE);
    TEST_ASSERT_EQUAL(0x06, PACKET_IMAGE);
    TEST_ASSERT_EQUAL(0x07, PACKET_TELEMETRY);
    TEST_ASSERT_EQUAL(0x08, PACKET_EMERGENCY);
}

//==============================================================================
// MESH NODE STRUCTURE TESTS
//==============================================================================

void test_mesh_node_structure(void) {
    MeshNode node;
    
    // Set values
    node.nodeId = 0x12345678;
    node.shortName = "TestNode";
    node.latitude = 45.5f;
    node.longitude = -122.5f;
    node.batteryLevel = 85;
    node.lastRssi = -75;
    node.lastSnr = 8.5f;
    node.hopCount = 2;
    node.lastSeen = 1000;
    node.role = MESH_ROLE_COORDINATOR;
    node.isOnline = true;
    
    // Verify values
    TEST_ASSERT_EQUAL_UINT32(0x12345678, node.nodeId);
    TEST_ASSERT_EQUAL_STRING("TestNode", node.shortName.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 45.5f, node.latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -122.5f, node.longitude);
    TEST_ASSERT_EQUAL_UINT8(85, node.batteryLevel);
    TEST_ASSERT_EQUAL_INT16(-75, node.lastRssi);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 8.5f, node.lastSnr);
    TEST_ASSERT_EQUAL_UINT8(2, node.hopCount);
    TEST_ASSERT_EQUAL_UINT32(1000, node.lastSeen);
    TEST_ASSERT_EQUAL(MESH_ROLE_COORDINATOR, node.role);
    TEST_ASSERT_TRUE(node.isOnline);
}

void test_wildlife_event_structure(void) {
    WildlifeEvent event;
    
    event.timestamp = 12345678;
    event.nodeId = 0xABCD1234;
    event.species = "Mountain Lion";
    event.confidence = 0.89f;
    event.latitude = 37.7749f;
    event.longitude = -122.4194f;
    event.imageSize = 65536;
    event.hasImage = true;
    
    TEST_ASSERT_EQUAL_UINT32(12345678, event.timestamp);
    TEST_ASSERT_EQUAL_UINT32(0xABCD1234, event.nodeId);
    TEST_ASSERT_EQUAL_STRING("Mountain Lion", event.species.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.89f, event.confidence);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 37.7749f, event.latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -122.4194f, event.longitude);
    TEST_ASSERT_EQUAL_UINT32(65536, event.imageSize);
    TEST_ASSERT_TRUE(event.hasImage);
}

//==============================================================================
// CONFIGURATION TESTS
//==============================================================================

void test_lora_config_defaults(void) {
    // Verify config.h defaults are reasonable
    TEST_ASSERT_EQUAL_UINT32(915000000, LORA_FREQUENCY);  // US ISM band
    TEST_ASSERT_EQUAL_UINT32(125000, LORA_BANDWIDTH);
    TEST_ASSERT_TRUE(LORA_SPREADING_FACTOR >= 7 && LORA_SPREADING_FACTOR <= 12);
    TEST_ASSERT_TRUE(LORA_CODING_RATE >= 5 && LORA_CODING_RATE <= 8);
    TEST_ASSERT_TRUE(LORA_TX_POWER >= 2 && LORA_TX_POWER <= 20);
    TEST_ASSERT_TRUE(MESH_MAX_HOPS >= 1 && MESH_MAX_HOPS <= 10);
    TEST_ASSERT_TRUE(MESH_BEACON_INTERVAL_MS >= 5000);
    TEST_ASSERT_TRUE(MESH_NODE_TIMEOUT_MS >= 60000);
    TEST_ASSERT_TRUE(MESH_MAX_NODES >= 4 && MESH_MAX_NODES <= 64);
}

//==============================================================================
// TEST RUNNER
//==============================================================================

void setup() {
    delay(2000);  // Wait for serial monitor
    
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_mesh_manager_initial_state);
    RUN_TEST(test_mesh_manager_node_id_setting);
    RUN_TEST(test_mesh_manager_node_name);
    
    // Network status tests
    RUN_TEST(test_mesh_network_status_uninitialized);
    RUN_TEST(test_mesh_signal_quality_classification);
    RUN_TEST(test_mesh_node_count);
    
    // Coordinator tests
    RUN_TEST(test_mesh_become_coordinator_without_init);
    
    // Routing tests
    RUN_TEST(test_mesh_routing_table_empty);
    
    // Power management tests
    RUN_TEST(test_mesh_low_power_mode);
    
    // Message transmission tests
    RUN_TEST(test_mesh_send_message_uninitialized);
    RUN_TEST(test_mesh_send_wildlife_event_uninitialized);
    RUN_TEST(test_mesh_send_image_uninitialized);
    
    // Callback tests
    RUN_TEST(test_mesh_callback_registration);
    
    // Structure validation tests
    RUN_TEST(test_signal_strength_enum_values);
    RUN_TEST(test_mesh_role_enum_values);
    RUN_TEST(test_mesh_priority_enum_values);
    RUN_TEST(test_mesh_packet_type_enum_values);
    RUN_TEST(test_mesh_node_structure);
    RUN_TEST(test_wildlife_event_structure);
    
    // Configuration tests
    RUN_TEST(test_lora_config_defaults);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}
