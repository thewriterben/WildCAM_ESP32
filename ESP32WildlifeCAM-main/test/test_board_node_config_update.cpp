/**
 * @file test_board_node_config_update.cpp
 * @brief Unit tests for board node configuration update handling
 * 
 * Tests configuration update parsing, validation, and acknowledgment
 * for the multi-board coordination system.
 */

#include <unity.h>
#include "../firmware/src/multi_board/board_node.h"
#include "../firmware/src/multi_board/message_protocol.h"

// Mock node for testing
BoardNode testNode;
bool mockMessageQueued = false;
String lastQueuedMessage = "";

// Mock LoraMesh::queueMessage for testing
namespace LoraMesh {
    bool queueMessage(const String& message) {
        mockMessageQueued = true;
        lastQueuedMessage = message;
        return true;
    }
}

// Test setup and teardown
void setUp(void) {
    mockMessageQueued = false;
    lastQueuedMessage = "";
}

void tearDown(void) {
    // Cleanup after each test
}

/**
 * Test valid configuration update with all parameters
 */
void test_config_update_valid_all_parameters(void) {
    // Initialize node
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    
    // Create a valid configuration update message
    DynamicJsonDocument doc(1024);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["heartbeatInterval"] = 30000UL;      // 30 seconds - valid
    data["coordinatorTimeout"] = 300000UL;    // 5 minutes - valid
    data["taskTimeout"] = 120000UL;           // 2 minutes - valid
    data["maxRetries"] = 5;                   // valid
    data["enableAutonomousMode"] = true;
    data["enableTaskExecution"] = false;
    
    // Parse message
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    // Handle configuration update
    testNode.handleMessage(msg);
    
    // Verify that acknowledgment was sent
    TEST_ASSERT_TRUE(mockMessageQueued);
    
    // Verify configuration was applied
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(30000UL, config.heartbeatInterval);
    TEST_ASSERT_EQUAL(300000UL, config.coordinatorTimeout);
    TEST_ASSERT_EQUAL(120000UL, config.taskTimeout);
    TEST_ASSERT_EQUAL(5, config.maxRetries);
    TEST_ASSERT_TRUE(config.enableAutonomousMode);
    TEST_ASSERT_FALSE(config.enableTaskExecution);
    
    // Verify acknowledgment contains success status
    TEST_ASSERT_TRUE(lastQueuedMessage.indexOf("success") > 0);
}

/**
 * Test configuration update with partial parameters
 */
void test_config_update_partial_parameters(void) {
    // Initialize node with default config
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    // Create a configuration update with only some parameters
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["heartbeatInterval"] = 45000UL;      // 45 seconds - only update this
    data["maxRetries"] = 7;                   // and this
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify only specified parameters were updated
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(45000UL, config.heartbeatInterval);
    TEST_ASSERT_EQUAL(7, config.maxRetries);
    
    // Verify other parameters remain unchanged
    TEST_ASSERT_EQUAL(originalConfig.coordinatorTimeout, config.coordinatorTimeout);
    TEST_ASSERT_EQUAL(originalConfig.taskTimeout, config.taskTimeout);
    TEST_ASSERT_EQUAL(originalConfig.enableAutonomousMode, config.enableAutonomousMode);
    TEST_ASSERT_EQUAL(originalConfig.enableTaskExecution, config.enableTaskExecution);
}

/**
 * Test configuration update with invalid heartbeatInterval (out of range)
 */
void test_config_update_invalid_heartbeat_interval(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    // Create configuration update with invalid heartbeatInterval
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["heartbeatInterval"] = 5000UL;  // Too low - below 10 seconds minimum
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify configuration was NOT applied
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(originalConfig.heartbeatInterval, config.heartbeatInterval);
    
    // Verify acknowledgment contains failure status
    TEST_ASSERT_TRUE(lastQueuedMessage.indexOf("failed") > 0);
}

/**
 * Test configuration update with invalid coordinatorTimeout
 */
void test_config_update_invalid_coordinator_timeout(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    // Create configuration update with invalid coordinatorTimeout
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["coordinatorTimeout"] = 2000000UL;  // Too high - above 30 minutes maximum
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify configuration was NOT applied
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(originalConfig.coordinatorTimeout, config.coordinatorTimeout);
    
    // Verify acknowledgment contains failure status
    TEST_ASSERT_TRUE(lastQueuedMessage.indexOf("failed") > 0);
}

/**
 * Test configuration update with invalid taskTimeout
 */
void test_config_update_invalid_task_timeout(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["taskTimeout"] = 15000UL;  // Too low - below 30 seconds minimum
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify configuration was NOT applied
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(originalConfig.taskTimeout, config.taskTimeout);
}

/**
 * Test configuration update with invalid maxRetries
 */
void test_config_update_invalid_max_retries(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["maxRetries"] = 15;  // Too high - above 10 maximum
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify configuration was NOT applied
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(originalConfig.maxRetries, config.maxRetries);
}

/**
 * Test configuration update with invalid boolean type
 */
void test_config_update_invalid_boolean_type(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["enableAutonomousMode"] = "not_a_boolean";  // Invalid type
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify configuration was NOT applied
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(originalConfig.enableAutonomousMode, config.enableAutonomousMode);
    
    // Verify acknowledgment contains failure status
    TEST_ASSERT_TRUE(lastQueuedMessage.indexOf("failed") > 0);
}

/**
 * Test configuration update with mixed valid and invalid parameters
 */
void test_config_update_mixed_valid_invalid(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    const NodeConfig& originalConfig = testNode.getNodeConfig();
    
    DynamicJsonDocument doc(1024);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["heartbeatInterval"] = 40000UL;  // Valid
    data["maxRetries"] = 20;              // Invalid - too high
    data["enableAutonomousMode"] = false; // Valid
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Verify ENTIRE configuration was rejected (all-or-nothing validation)
    const NodeConfig& config = testNode.getNodeConfig();
    TEST_ASSERT_EQUAL(originalConfig.heartbeatInterval, config.heartbeatInterval);
    TEST_ASSERT_EQUAL(originalConfig.maxRetries, config.maxRetries);
    TEST_ASSERT_EQUAL(originalConfig.enableAutonomousMode, config.enableAutonomousMode);
    
    // Verify acknowledgment contains failure status
    TEST_ASSERT_TRUE(lastQueuedMessage.indexOf("failed") > 0);
}

/**
 * Test acknowledgment message format
 */
void test_config_update_acknowledgment_format(void) {
    TEST_ASSERT_TRUE(testNode.init(1, ROLE_NODE));
    
    DynamicJsonDocument doc(512);
    doc["type"] = MSG_CONFIG_UPDATE;
    doc["source_node"] = 2;
    doc["target_node"] = 1;
    doc["timestamp"] = millis();
    
    JsonObject data = doc.createNestedObject("data");
    data["heartbeatInterval"] = 30000UL;
    
    MultiboardMessage msg;
    msg.type = MSG_CONFIG_UPDATE;
    msg.sourceNode = 2;
    msg.targetNode = 1;
    msg.timestamp = millis();
    msg.data = data.as<JsonObject>();
    
    testNode.handleMessage(msg);
    
    // Parse acknowledgment message
    DynamicJsonDocument ackDoc(512);
    DeserializationError error = deserializeJson(ackDoc, lastQueuedMessage);
    TEST_ASSERT_FALSE(error);
    
    // Verify acknowledgment structure
    TEST_ASSERT_EQUAL(MSG_STATUS, ackDoc["type"].as<int>());
    TEST_ASSERT_EQUAL(1, ackDoc["source_node"].as<int>());  // From test node
    TEST_ASSERT_EQUAL(2, ackDoc["target_node"].as<int>());  // To coordinator
    TEST_ASSERT_TRUE(ackDoc.containsKey("data"));
    TEST_ASSERT_TRUE(ackDoc["data"].containsKey("config_update_status"));
}

// Main test runner
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_config_update_valid_all_parameters);
    RUN_TEST(test_config_update_partial_parameters);
    RUN_TEST(test_config_update_invalid_heartbeat_interval);
    RUN_TEST(test_config_update_invalid_coordinator_timeout);
    RUN_TEST(test_config_update_invalid_task_timeout);
    RUN_TEST(test_config_update_invalid_max_retries);
    RUN_TEST(test_config_update_invalid_boolean_type);
    RUN_TEST(test_config_update_mixed_valid_invalid);
    RUN_TEST(test_config_update_acknowledgment_format);
    
    return UNITY_END();
}
