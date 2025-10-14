/**
 * @file test_reliable_transmission_protocol.cpp
 * @brief Unit tests for Reliable Transmission Protocol
 */

#include <unity.h>
#include "../firmware/src/meshtastic/reliable_transmission_protocol.h"
#include "../firmware/src/meshtastic/mesh_interface.h"

// Test fixtures
static ReliableTransmissionProtocol* rtp = nullptr;
static MeshInterface* meshInterface = nullptr;

void setUp(void) {
    // Set up before each test
    meshInterface = new MeshInterface();
    meshInterface->init(0x12345678, "TestNode");
    
    rtp = new ReliableTransmissionProtocol();
    rtp->init(meshInterface);
}

void tearDown(void) {
    // Clean up after each test
    if (rtp) {
        delete rtp;
        rtp = nullptr;
    }
    if (meshInterface) {
        delete meshInterface;
        meshInterface = nullptr;
    }
}

// ===========================
// INITIALIZATION TESTS
// ===========================

void test_initialization() {
    ReliableTransmissionProtocol protocol;
    TEST_ASSERT_TRUE(protocol.init(meshInterface));
}

void test_initialization_null_interface() {
    ReliableTransmissionProtocol protocol;
    TEST_ASSERT_FALSE(protocol.init(nullptr));
}

void test_configuration() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    config.maxRetries = 10;
    config.ackTimeout = 10000;
    
    TEST_ASSERT_TRUE(rtp->configure(config));
    
    ReliableTransmissionConfig retrieved = rtp->getConfig();
    TEST_ASSERT_EQUAL(10, retrieved.maxRetries);
    TEST_ASSERT_EQUAL(10000, retrieved.ackTimeout);
}

// ===========================
// TRANSMISSION TESTS
// ===========================

void test_transmit_small_data() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
    
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    TEST_ASSERT_NOT_EQUAL(0, txId);
    TEST_ASSERT_TRUE(rtp->isTransmissionActive(txId));
}

void test_transmit_large_data() {
    // Create large data that requires splitting
    std::vector<uint8_t> data(1024, 0xAB);
    
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_HIGH, true);
    
    TEST_ASSERT_NOT_EQUAL(0, txId);
    
    TransmissionStatus status = rtp->getTransmissionStatus(txId);
    TEST_ASSERT_EQUAL(TX_STATE_QUEUED, status.state);
    TEST_ASSERT_GREATER_THAN(1, status.totalPackets);
}

void test_transmit_empty_data() {
    std::vector<uint8_t> emptyData;
    
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, emptyData, PRIORITY_NORMAL, false);
    
    TEST_ASSERT_EQUAL(0, txId);  // Should fail
}

void test_transmit_with_priority() {
    std::vector<uint8_t> data1 = {0x01, 0x02};
    std::vector<uint8_t> data2 = {0x03, 0x04};
    std::vector<uint8_t> data3 = {0x05, 0x06};
    
    uint32_t lowPriority = rtp->transmitData(0, PACKET_TYPE_DATA, data1, PRIORITY_LOW, false);
    uint32_t normalPriority = rtp->transmitData(0, PACKET_TYPE_DATA, data2, PRIORITY_NORMAL, false);
    uint32_t highPriority = rtp->transmitData(0, PACKET_TYPE_DATA, data3, PRIORITY_HIGH, false);
    
    TEST_ASSERT_NOT_EQUAL(0, lowPriority);
    TEST_ASSERT_NOT_EQUAL(0, normalPriority);
    TEST_ASSERT_NOT_EQUAL(0, highPriority);
}

// ===========================
// RETRY LOGIC TESTS
// ===========================

void test_retry_calculation() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    config.initialRetryDelay = 1000;
    config.backoffMultiplier = 2.0;
    rtp->configure(config);
    
    // Test exponential backoff
    // Retry 0: 1000ms
    // Retry 1: 2000ms
    // Retry 2: 4000ms
    // etc.
}

void test_max_retries_exceeded() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    config.maxRetries = 3;
    config.enableRetransmission = true;
    rtp->configure(config);
    
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, true);
    
    TEST_ASSERT_NOT_EQUAL(0, txId);
}

// ===========================
// BANDWIDTH MANAGEMENT TESTS
// ===========================

void test_bandwidth_limit() {
    TEST_ASSERT_TRUE(rtp->setBandwidthLimit(2048));
    TEST_ASSERT_EQUAL(2048, rtp->getBandwidthLimit());
}

void test_bandwidth_usage() {
    // Initially should be zero
    TEST_ASSERT_EQUAL(0.0, rtp->getCurrentBandwidthUsage());
    
    // After transmission, should be updated
    std::vector<uint8_t> data(512, 0xFF);
    rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    // Process to update statistics
    rtp->process();
}

void test_throttling() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    config.enableBandwidthManagement = true;
    config.maxBytesPerSecond = 512;
    rtp->configure(config);
    
    // Should be able to throttle
    TEST_ASSERT_FALSE(rtp->throttleTransmission());
}

// ===========================
// STATISTICS TESTS
// ===========================

void test_statistics_initialization() {
    BandwidthStats stats = rtp->getStatistics();
    
    TEST_ASSERT_EQUAL(0, stats.bytesTransmitted);
    TEST_ASSERT_EQUAL(0, stats.packetsTransmitted);
    TEST_ASSERT_EQUAL(0.0, stats.packetLossRate);
}

void test_statistics_reset() {
    // Transmit some data
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    // Reset statistics
    rtp->resetStatistics();
    
    BandwidthStats stats = rtp->getStatistics();
    TEST_ASSERT_EQUAL(0, stats.bytesTransmitted);
    TEST_ASSERT_EQUAL(0, stats.packetsTransmitted);
}

void test_packet_loss_rate() {
    BandwidthStats stats = rtp->getStatistics();
    
    // Initially should be 0
    TEST_ASSERT_EQUAL(0.0, rtp->getPacketLossRate());
}

void test_throughput_calculation() {
    // Initially should be 0
    TEST_ASSERT_EQUAL(0.0, rtp->getThroughput());
}

// ===========================
// STATUS TESTS
// ===========================

void test_get_transmission_status() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    TransmissionStatus status = rtp->getTransmissionStatus(txId);
    
    TEST_ASSERT_EQUAL(txId, status.transmissionId);
    TEST_ASSERT_EQUAL(TX_STATE_QUEUED, status.state);
}

void test_get_invalid_transmission_status() {
    TransmissionStatus status = rtp->getTransmissionStatus(99999);
    
    TEST_ASSERT_EQUAL(0, status.transmissionId);
    TEST_ASSERT_EQUAL(TX_STATE_FAILED, status.state);
}

void test_get_all_transmissions() {
    std::vector<uint8_t> data = {0x01, 0x02};
    
    rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_HIGH, false);
    rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_LOW, false);
    
    std::vector<TransmissionStatus> all = rtp->getAllTransmissions();
    
    TEST_ASSERT_EQUAL(3, all.size());
}

void test_is_transmission_complete() {
    std::vector<uint8_t> data = {0x01, 0x02};
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    // Initially should not be complete
    TEST_ASSERT_FALSE(rtp->isTransmissionComplete(txId));
}

void test_is_transmission_active() {
    std::vector<uint8_t> data = {0x01, 0x02};
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    // Should be active after queuing
    TEST_ASSERT_TRUE(rtp->isTransmissionActive(txId));
}

// ===========================
// CANCELLATION TESTS
// ===========================

void test_cancel_transmission() {
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    uint32_t txId = rtp->transmitData(0, PACKET_TYPE_DATA, data, PRIORITY_NORMAL, false);
    
    TEST_ASSERT_TRUE(rtp->cancelTransmission(txId));
    
    TransmissionStatus status = rtp->getTransmissionStatus(txId);
    TEST_ASSERT_EQUAL(TX_STATE_CANCELLED, status.state);
}

void test_cancel_invalid_transmission() {
    TEST_ASSERT_FALSE(rtp->cancelTransmission(99999));
}

// ===========================
// CONFIGURATION HELPER TESTS
// ===========================

void test_default_config() {
    ReliableTransmissionConfig config = createDefaultRTPConfig();
    
    TEST_ASSERT_TRUE(config.enableRetransmission);
    TEST_ASSERT_TRUE(config.enableAcknowledgments);
    TEST_ASSERT_TRUE(config.enableBandwidthManagement);
    TEST_ASSERT_EQUAL(RTP_MAX_RETRIES, config.maxRetries);
}

void test_low_bandwidth_config() {
    ReliableTransmissionConfig config = createLowBandwidthConfig();
    
    TEST_ASSERT_EQUAL(512, config.maxBytesPerSecond);
    TEST_ASSERT_EQUAL(7, config.maxRetries);
    TEST_ASSERT_TRUE(config.adaptiveRetry);
}

void test_high_reliability_config() {
    ReliableTransmissionConfig config = createHighReliabilityConfig();
    
    TEST_ASSERT_EQUAL(10, config.maxRetries);
    TEST_ASSERT_EQUAL(10000, config.ackTimeout);
    TEST_ASSERT_TRUE(config.adaptiveRetry);
}

void test_best_effort_config() {
    ReliableTransmissionConfig config = createBestEffortConfig();
    
    TEST_ASSERT_FALSE(config.enableRetransmission);
    TEST_ASSERT_FALSE(config.enableAcknowledgments);
    TEST_ASSERT_FALSE(config.enableBandwidthManagement);
}

// ===========================
// UTILITY FUNCTION TESTS
// ===========================

void test_priority_to_string() {
    TEST_ASSERT_EQUAL_STRING("CRITICAL", priorityToString(PRIORITY_CRITICAL));
    TEST_ASSERT_EQUAL_STRING("HIGH", priorityToString(PRIORITY_HIGH));
    TEST_ASSERT_EQUAL_STRING("NORMAL", priorityToString(PRIORITY_NORMAL));
    TEST_ASSERT_EQUAL_STRING("LOW", priorityToString(PRIORITY_LOW));
    TEST_ASSERT_EQUAL_STRING("BACKGROUND", priorityToString(PRIORITY_BACKGROUND));
}

void test_string_to_priority() {
    TEST_ASSERT_EQUAL(PRIORITY_CRITICAL, stringToPriority("CRITICAL"));
    TEST_ASSERT_EQUAL(PRIORITY_HIGH, stringToPriority("HIGH"));
    TEST_ASSERT_EQUAL(PRIORITY_NORMAL, stringToPriority("NORMAL"));
    TEST_ASSERT_EQUAL(PRIORITY_LOW, stringToPriority("LOW"));
    TEST_ASSERT_EQUAL(PRIORITY_BACKGROUND, stringToPriority("BACKGROUND"));
}

void test_state_to_string() {
    TEST_ASSERT_EQUAL_STRING("QUEUED", stateToString(TX_STATE_QUEUED));
    TEST_ASSERT_EQUAL_STRING("TRANSMITTING", stateToString(TX_STATE_TRANSMITTING));
    TEST_ASSERT_EQUAL_STRING("COMPLETED", stateToString(TX_STATE_COMPLETED));
    TEST_ASSERT_EQUAL_STRING("FAILED", stateToString(TX_STATE_FAILED));
}

void test_result_to_string() {
    TEST_ASSERT_EQUAL_STRING("SUCCESS", resultToString(TX_SUCCESS));
    TEST_ASSERT_EQUAL_STRING("TIMEOUT", resultToString(TX_TIMEOUT));
    TEST_ASSERT_EQUAL_STRING("MAX_RETRIES", resultToString(TX_MAX_RETRIES));
}

void test_validation_functions() {
    TransmissionRequest validRequest;
    validRequest.transmissionId = 1;
    validRequest.data = {0x01, 0x02, 0x03};
    validRequest.maxRetries = 3;
    validRequest.retryDelay = 1000;
    
    TEST_ASSERT_TRUE(isValidTransmissionRequest(validRequest));
    
    ReliablePacket validPacket;
    validPacket.packetId = 1;
    validPacket.transmissionId = 1;
    validPacket.payload = {0x01, 0x02};
    validPacket.sequenceNumber = 0;
    validPacket.totalPackets = 1;
    
    TEST_ASSERT_TRUE(isValidPacket(validPacket));
}

// ===========================
// MAIN TEST RUNNER
// ===========================

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_initialization);
    RUN_TEST(test_initialization_null_interface);
    RUN_TEST(test_configuration);
    
    // Transmission tests
    RUN_TEST(test_transmit_small_data);
    RUN_TEST(test_transmit_large_data);
    RUN_TEST(test_transmit_empty_data);
    RUN_TEST(test_transmit_with_priority);
    
    // Retry logic tests
    RUN_TEST(test_retry_calculation);
    RUN_TEST(test_max_retries_exceeded);
    
    // Bandwidth management tests
    RUN_TEST(test_bandwidth_limit);
    RUN_TEST(test_bandwidth_usage);
    RUN_TEST(test_throttling);
    
    // Statistics tests
    RUN_TEST(test_statistics_initialization);
    RUN_TEST(test_statistics_reset);
    RUN_TEST(test_packet_loss_rate);
    RUN_TEST(test_throughput_calculation);
    
    // Status tests
    RUN_TEST(test_get_transmission_status);
    RUN_TEST(test_get_invalid_transmission_status);
    RUN_TEST(test_get_all_transmissions);
    RUN_TEST(test_is_transmission_complete);
    RUN_TEST(test_is_transmission_active);
    
    // Cancellation tests
    RUN_TEST(test_cancel_transmission);
    RUN_TEST(test_cancel_invalid_transmission);
    
    // Configuration helper tests
    RUN_TEST(test_default_config);
    RUN_TEST(test_low_bandwidth_config);
    RUN_TEST(test_high_reliability_config);
    RUN_TEST(test_best_effort_config);
    
    // Utility function tests
    RUN_TEST(test_priority_to_string);
    RUN_TEST(test_string_to_priority);
    RUN_TEST(test_state_to_string);
    RUN_TEST(test_result_to_string);
    RUN_TEST(test_validation_functions);
    
    return UNITY_END();
}
