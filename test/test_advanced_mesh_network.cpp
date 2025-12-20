/**
 * @file test_advanced_mesh_network.cpp
 * @brief Unit tests for Advanced LoRa Mesh Networking
 * @version 1.0.0
 * 
 * Tests the advanced mesh networking functionality including:
 * - Multi-camera coordination
 * - Extended range communication
 * - Shared detection alerts
 * - Federated learning over mesh
 */

#include <unity.h>
#include "AdvancedMeshNetwork.h"
#include "MeshManager.h"
#include "config.h"

// Test instances
MeshManager testMeshManager;
AdvancedMeshNetwork* testAdvancedMesh = nullptr;

void setUp(void) {
    // Reset test instance state before each test
}

void tearDown(void) {
    // Cleanup after each test
}

//==============================================================================
// INITIALIZATION TESTS
//==============================================================================

void test_advanced_mesh_initial_state(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Should not be initialized by default
    TEST_ASSERT_FALSE(mesh.isInitialized());
    
    // Camera role should be standalone
    TEST_ASSERT_EQUAL(CAMERA_ROLE_STANDALONE, mesh.getCameraRole());
    
    // Sync state should be unsync
    TEST_ASSERT_EQUAL(SYNC_STATE_UNSYNC, mesh.getSyncState());
    
    // FL should not be active
    TEST_ASSERT_FALSE(mesh.isFLActive());
    
    // Should not be FL coordinator
    TEST_ASSERT_FALSE(mesh.isFLCoordinator());
}

void test_advanced_mesh_cannot_init_without_mesh_manager(void) {
    MeshManager uninitMesh;
    AdvancedMeshNetwork advMesh(uninitMesh);
    
    // Should fail because MeshManager is not initialized
    TEST_ASSERT_FALSE(advMesh.init());
    TEST_ASSERT_FALSE(advMesh.isInitialized());
}

//==============================================================================
// CAMERA ROLE TESTS
//==============================================================================

void test_camera_role_setting(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Default should be standalone
    TEST_ASSERT_EQUAL(CAMERA_ROLE_STANDALONE, mesh.getCameraRole());
    
    // Set to primary
    mesh.setCameraRole(CAMERA_ROLE_PRIMARY);
    TEST_ASSERT_EQUAL(CAMERA_ROLE_PRIMARY, mesh.getCameraRole());
    
    // Set to secondary
    mesh.setCameraRole(CAMERA_ROLE_SECONDARY);
    TEST_ASSERT_EQUAL(CAMERA_ROLE_SECONDARY, mesh.getCameraRole());
    
    // Set to relay
    mesh.setCameraRole(CAMERA_ROLE_RELAY);
    TEST_ASSERT_EQUAL(CAMERA_ROLE_RELAY, mesh.getCameraRole());
    
    // Set to gateway
    mesh.setCameraRole(CAMERA_ROLE_GATEWAY);
    TEST_ASSERT_EQUAL(CAMERA_ROLE_GATEWAY, mesh.getCameraRole());
}

void test_camera_role_enum_values(void) {
    TEST_ASSERT_EQUAL(0, CAMERA_ROLE_STANDALONE);
    TEST_ASSERT_EQUAL(1, CAMERA_ROLE_PRIMARY);
    TEST_ASSERT_EQUAL(2, CAMERA_ROLE_SECONDARY);
    TEST_ASSERT_EQUAL(3, CAMERA_ROLE_RELAY);
    TEST_ASSERT_EQUAL(4, CAMERA_ROLE_GATEWAY);
}

void test_camera_sync_state_enum_values(void) {
    TEST_ASSERT_EQUAL(0, SYNC_STATE_UNSYNC);
    TEST_ASSERT_EQUAL(1, SYNC_STATE_SYNCING);
    TEST_ASSERT_EQUAL(2, SYNC_STATE_SYNCED);
    TEST_ASSERT_EQUAL(3, SYNC_STATE_LEADER);
}

//==============================================================================
// CAMERA GROUP TESTS
//==============================================================================

void test_camera_group_initial_state(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    CameraGroup group = mesh.getCameraGroup();
    
    TEST_ASSERT_EQUAL(0, group.groupId);
    TEST_ASSERT_EQUAL(0, group.primaryNodeId);
    TEST_ASSERT_EQUAL(0, group.memberCount);
    TEST_ASSERT_FALSE(group.isActive);
}

void test_camera_group_cannot_create_without_init(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Should fail because mesh is not initialized
    TEST_ASSERT_FALSE(mesh.createCameraGroup(1));
}

void test_camera_group_cannot_join_without_init(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Should fail because mesh is not initialized
    TEST_ASSERT_FALSE(mesh.joinCameraGroup(1));
}

//==============================================================================
// EXTENDED RANGE TESTS
//==============================================================================

void test_range_mode_setting(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Default should be adaptive
    TEST_ASSERT_EQUAL(RANGE_MODE_ADAPTIVE, mesh.getRangeMode());
    
    // Set to normal
    mesh.setRangeMode(RANGE_MODE_NORMAL);
    TEST_ASSERT_EQUAL(RANGE_MODE_NORMAL, mesh.getRangeMode());
    
    // Set to extended
    mesh.setRangeMode(RANGE_MODE_EXTENDED);
    TEST_ASSERT_EQUAL(RANGE_MODE_EXTENDED, mesh.getRangeMode());
    
    // Set to maximum
    mesh.setRangeMode(RANGE_MODE_MAXIMUM);
    TEST_ASSERT_EQUAL(RANGE_MODE_MAXIMUM, mesh.getRangeMode());
}

void test_range_mode_enum_values(void) {
    TEST_ASSERT_EQUAL(0, RANGE_MODE_NORMAL);
    TEST_ASSERT_EQUAL(1, RANGE_MODE_EXTENDED);
    TEST_ASSERT_EQUAL(2, RANGE_MODE_MAXIMUM);
    TEST_ASSERT_EQUAL(3, RANGE_MODE_ADAPTIVE);
}

void test_range_settings_normal(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    mesh.setRangeMode(RANGE_MODE_NORMAL);
    RangeSettings settings = mesh.getRangeSettings();
    
    // Normal mode should have balanced settings
    TEST_ASSERT_EQUAL(9, settings.spreadingFactor);
    TEST_ASSERT_EQUAL(125000, settings.bandwidth);
    TEST_ASSERT_EQUAL(5, settings.codingRate);
    TEST_ASSERT_EQUAL(14, settings.txPower);
}

void test_range_settings_extended(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    mesh.setRangeMode(RANGE_MODE_EXTENDED);
    RangeSettings settings = mesh.getRangeSettings();
    
    // Extended mode should have higher SF
    TEST_ASSERT_EQUAL(10, settings.spreadingFactor);
    TEST_ASSERT_EQUAL(17, settings.txPower);
}

void test_range_settings_maximum(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    mesh.setRangeMode(RANGE_MODE_MAXIMUM);
    RangeSettings settings = mesh.getRangeSettings();
    
    // Maximum mode should have highest SF and power
    TEST_ASSERT_EQUAL(12, settings.spreadingFactor);
    TEST_ASSERT_EQUAL(20, settings.txPower);
    TEST_ASSERT_EQUAL(8, settings.codingRate);
}

void test_relay_nodes_initially_empty(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    auto relays = mesh.getRelayNodes();
    TEST_ASSERT_EQUAL(0, relays.size());
}

//==============================================================================
// SHARED ALERT TESTS
//==============================================================================

void test_alert_priority_enum_values(void) {
    TEST_ASSERT_EQUAL(0, ALERT_PRIORITY_LOW);
    TEST_ASSERT_EQUAL(1, ALERT_PRIORITY_NORMAL);
    TEST_ASSERT_EQUAL(2, ALERT_PRIORITY_HIGH);
    TEST_ASSERT_EQUAL(3, ALERT_PRIORITY_URGENT);
    TEST_ASSERT_EQUAL(4, ALERT_PRIORITY_EMERGENCY);
}

void test_alert_type_enum_values(void) {
    TEST_ASSERT_EQUAL(0, ALERT_TYPE_DETECTION);
    TEST_ASSERT_EQUAL(1, ALERT_TYPE_MOTION);
    TEST_ASSERT_EQUAL(2, ALERT_TYPE_SPECIES);
    TEST_ASSERT_EQUAL(3, ALERT_TYPE_BEHAVIOR);
    TEST_ASSERT_EQUAL(4, ALERT_TYPE_INTRUSION);
    TEST_ASSERT_EQUAL(5, ALERT_TYPE_SYSTEM);
}

void test_recent_alerts_initially_empty(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    auto alerts = mesh.getRecentAlerts();
    TEST_ASSERT_EQUAL(0, alerts.size());
    TEST_ASSERT_EQUAL(0, mesh.getPendingAlertCount());
}

void test_alert_id_generation(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    uint32_t id1 = mesh.generateAlertId();
    uint32_t id2 = mesh.generateAlertId();
    uint32_t id3 = mesh.generateAlertId();
    
    // IDs should be unique
    TEST_ASSERT_NOT_EQUAL(id1, id2);
    TEST_ASSERT_NOT_EQUAL(id2, id3);
    TEST_ASSERT_NOT_EQUAL(id1, id3);
}

void test_shared_alert_structure(void) {
    SharedAlert alert;
    
    alert.alertId = 0x12345678;
    alert.originNodeId = 0xABCDEF01;
    alert.timestamp = 1000;
    alert.priority = ALERT_PRIORITY_HIGH;
    alert.type = ALERT_TYPE_SPECIES;
    alert.species = "Deer";
    alert.confidence = 0.95f;
    alert.latitude = 45.5f;
    alert.longitude = -122.5f;
    alert.hopCount = 2;
    alert.hasImage = true;
    alert.imageSize = 50000;
    alert.acknowledged = false;
    
    TEST_ASSERT_EQUAL_UINT32(0x12345678, alert.alertId);
    TEST_ASSERT_EQUAL_UINT32(0xABCDEF01, alert.originNodeId);
    TEST_ASSERT_EQUAL_UINT32(1000, alert.timestamp);
    TEST_ASSERT_EQUAL(ALERT_PRIORITY_HIGH, alert.priority);
    TEST_ASSERT_EQUAL(ALERT_TYPE_SPECIES, alert.type);
    TEST_ASSERT_EQUAL_STRING("Deer", alert.species.c_str());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.95f, alert.confidence);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 45.5f, alert.latitude);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, -122.5f, alert.longitude);
    TEST_ASSERT_EQUAL_UINT8(2, alert.hopCount);
    TEST_ASSERT_TRUE(alert.hasImage);
    TEST_ASSERT_EQUAL_UINT32(50000, alert.imageSize);
    TEST_ASSERT_FALSE(alert.acknowledged);
}

void test_duplicate_alert_detection(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Initially no duplicates
    TEST_ASSERT_FALSE(mesh.isDuplicateAlert(0x1234, 0xABCD));
    TEST_ASSERT_FALSE(mesh.isDuplicateAlert(0x5678, 0xEF01));
}

//==============================================================================
// FEDERATED LEARNING TESTS
//==============================================================================

void test_fl_node_role_enum_values(void) {
    TEST_ASSERT_EQUAL(0, FL_ROLE_PARTICIPANT);
    TEST_ASSERT_EQUAL(1, FL_ROLE_AGGREGATOR);
    TEST_ASSERT_EQUAL(2, FL_ROLE_RELAY);
    TEST_ASSERT_EQUAL(3, FL_ROLE_OBSERVER);
}

void test_fl_round_state_enum_values(void) {
    TEST_ASSERT_EQUAL(0, FL_STATE_IDLE);
    TEST_ASSERT_EQUAL(1, FL_STATE_ANNOUNCING);
    TEST_ASSERT_EQUAL(2, FL_STATE_TRAINING);
    TEST_ASSERT_EQUAL(3, FL_STATE_COLLECTING);
    TEST_ASSERT_EQUAL(4, FL_STATE_AGGREGATING);
    TEST_ASSERT_EQUAL(5, FL_STATE_DISTRIBUTING);
    TEST_ASSERT_EQUAL(6, FL_STATE_COMPLETE);
}

void test_fl_compression_enum_values(void) {
    TEST_ASSERT_EQUAL(0, FL_COMPRESS_NONE);
    TEST_ASSERT_EQUAL(1, FL_COMPRESS_QUANTIZE_8BIT);
    TEST_ASSERT_EQUAL(2, FL_COMPRESS_QUANTIZE_4BIT);
    TEST_ASSERT_EQUAL(3, FL_COMPRESS_SPARSE);
    TEST_ASSERT_EQUAL(4, FL_COMPRESS_COMBINED);
}

void test_fl_initial_state(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Default role should be participant
    TEST_ASSERT_EQUAL(FL_ROLE_PARTICIPANT, mesh.getFLRole());
    
    // FL should not be active
    TEST_ASSERT_FALSE(mesh.isFLActive());
    
    // Should not be coordinator
    TEST_ASSERT_FALSE(mesh.isFLCoordinator());
    
    // Participants should be empty
    auto participants = mesh.getFLParticipants();
    TEST_ASSERT_EQUAL(0, participants.size());
}

void test_fl_role_setting(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    mesh.setFLRole(FL_ROLE_AGGREGATOR);
    TEST_ASSERT_EQUAL(FL_ROLE_AGGREGATOR, mesh.getFLRole());
    
    mesh.setFLRole(FL_ROLE_RELAY);
    TEST_ASSERT_EQUAL(FL_ROLE_RELAY, mesh.getFLRole());
    
    mesh.setFLRole(FL_ROLE_OBSERVER);
    TEST_ASSERT_EQUAL(FL_ROLE_OBSERVER, mesh.getFLRole());
    
    mesh.setFLRole(FL_ROLE_PARTICIPANT);
    TEST_ASSERT_EQUAL(FL_ROLE_PARTICIPANT, mesh.getFLRole());
}

void test_fl_compression_setting(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    mesh.setFLCompression(FL_COMPRESS_NONE);
    // Note: No getter, but we test that it doesn't crash
    
    mesh.setFLCompression(FL_COMPRESS_QUANTIZE_8BIT);
    mesh.setFLCompression(FL_COMPRESS_QUANTIZE_4BIT);
    mesh.setFLCompression(FL_COMPRESS_SPARSE);
    mesh.setFLCompression(FL_COMPRESS_COMBINED);
}

void test_fl_round_cannot_start_without_init(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Should return 0 (failure) because mesh is not initialized
    uint32_t roundId = mesh.startFLRound("wildlife_classifier", 4, 60000);
    TEST_ASSERT_EQUAL(0, roundId);
}

void test_fl_round_cannot_join_without_init(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Should fail because mesh is not initialized
    TEST_ASSERT_FALSE(mesh.joinFLRound(0x12345678));
}

void test_fl_model_update_cannot_submit_without_init(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    uint8_t dummyGradients[] = {0x01, 0x02, 0x03, 0x04};
    
    // Should fail because mesh is not initialized
    TEST_ASSERT_FALSE(mesh.submitModelUpdate(dummyGradients, sizeof(dummyGradients), 100, 0.5f));
}

void test_fl_round_structure(void) {
    FLRound round;
    
    round.roundId = 0x12345678;
    round.coordinatorId = 0xABCDEF01;
    round.startTime = 1000;
    round.deadline = 61000;
    round.state = FL_STATE_TRAINING;
    round.participantCount = 5;
    round.updatesReceived = 3;
    round.targetParticipants = 8;
    round.minParticipationRatio = 0.6f;
    round.modelType = "wildlife_classifier";
    
    TEST_ASSERT_EQUAL_UINT32(0x12345678, round.roundId);
    TEST_ASSERT_EQUAL_UINT32(0xABCDEF01, round.coordinatorId);
    TEST_ASSERT_EQUAL_UINT32(1000, round.startTime);
    TEST_ASSERT_EQUAL_UINT32(61000, round.deadline);
    TEST_ASSERT_EQUAL(FL_STATE_TRAINING, round.state);
    TEST_ASSERT_EQUAL_UINT8(5, round.participantCount);
    TEST_ASSERT_EQUAL_UINT8(3, round.updatesReceived);
    TEST_ASSERT_EQUAL_UINT8(8, round.targetParticipants);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.6f, round.minParticipationRatio);
    TEST_ASSERT_EQUAL_STRING("wildlife_classifier", round.modelType.c_str());
}

void test_fl_participant_structure(void) {
    FLParticipant participant;
    
    participant.nodeId = 0x12345678;
    participant.role = FL_ROLE_PARTICIPANT;
    participant.batteryLevel = 85;
    participant.availableMemory = 65536;
    participant.reliability = 0.95f;
    participant.lastSeen = 1000;
    participant.updateSubmitted = true;
    participant.signalStrength = -75;
    
    TEST_ASSERT_EQUAL_UINT32(0x12345678, participant.nodeId);
    TEST_ASSERT_EQUAL(FL_ROLE_PARTICIPANT, participant.role);
    TEST_ASSERT_EQUAL_UINT8(85, participant.batteryLevel);
    TEST_ASSERT_EQUAL_UINT32(65536, participant.availableMemory);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.95f, participant.reliability);
    TEST_ASSERT_EQUAL_UINT32(1000, participant.lastSeen);
    TEST_ASSERT_TRUE(participant.updateSubmitted);
    TEST_ASSERT_EQUAL_INT16(-75, participant.signalStrength);
}

void test_fl_model_chunk_structure(void) {
    FLModelChunk chunk;
    
    chunk.roundId = 0x12345678;
    chunk.originNodeId = 0xABCDEF01;
    chunk.chunkIndex = 5;
    chunk.totalChunks = 10;
    chunk.modelVersion = 2;
    chunk.compression = FL_COMPRESS_QUANTIZE_8BIT;
    chunk.dataSize = 100;
    memset(chunk.data, 0xAB, chunk.dataSize);
    
    TEST_ASSERT_EQUAL_UINT32(0x12345678, chunk.roundId);
    TEST_ASSERT_EQUAL_UINT32(0xABCDEF01, chunk.originNodeId);
    TEST_ASSERT_EQUAL_UINT16(5, chunk.chunkIndex);
    TEST_ASSERT_EQUAL_UINT16(10, chunk.totalChunks);
    TEST_ASSERT_EQUAL_UINT32(2, chunk.modelVersion);
    TEST_ASSERT_EQUAL(FL_COMPRESS_QUANTIZE_8BIT, chunk.compression);
    TEST_ASSERT_EQUAL_UINT16(100, chunk.dataSize);
    TEST_ASSERT_EQUAL_UINT8(0xAB, chunk.data[0]);
    TEST_ASSERT_EQUAL_UINT8(0xAB, chunk.data[chunk.dataSize - 1]);
}

void test_fl_aggregation_stats_structure(void) {
    FLAggregationStats stats;
    
    stats.roundId = 0x12345678;
    stats.contributorCount = 6;
    stats.totalSamples = 1200;
    stats.averageLoss = 0.25f;
    stats.accuracyImprovement = 0.03f;
    stats.aggregationTime = 500;
    stats.distributionTime = 1200;
    
    TEST_ASSERT_EQUAL_UINT32(0x12345678, stats.roundId);
    TEST_ASSERT_EQUAL_UINT8(6, stats.contributorCount);
    TEST_ASSERT_EQUAL_UINT32(1200, stats.totalSamples);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, stats.averageLoss);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.03f, stats.accuracyImprovement);
    TEST_ASSERT_EQUAL_UINT32(500, stats.aggregationTime);
    TEST_ASSERT_EQUAL_UINT32(1200, stats.distributionTime);
}

//==============================================================================
// CALLBACK TESTS
//==============================================================================

static bool coordCallbackCalled = false;
static bool alertCallbackCalled = false;
static bool rangeCallbackCalled = false;
static bool flRoundCallbackCalled = false;
static bool flUpdateCallbackCalled = false;
static bool flAggregationCallbackCalled = false;

void testCoordCallback(const CameraStatus& status, bool isNew) {
    coordCallbackCalled = true;
}

void testAlertCallback(const SharedAlert& alert) {
    alertCallbackCalled = true;
}

void testRangeCallback(ExtendedRangeMode mode) {
    rangeCallbackCalled = true;
}

void testFLRoundCallback(const FLRound& round) {
    flRoundCallbackCalled = true;
}

void testFLUpdateCallback(uint32_t nodeId, uint32_t roundId, bool success) {
    flUpdateCallbackCalled = true;
}

void testFLAggregationCallback(const FLAggregationStats& stats) {
    flAggregationCallbackCalled = true;
}

void test_callback_registration(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    // Reset flags
    coordCallbackCalled = false;
    alertCallbackCalled = false;
    rangeCallbackCalled = false;
    flRoundCallbackCalled = false;
    flUpdateCallbackCalled = false;
    flAggregationCallbackCalled = false;
    
    // Should be able to set callbacks without crashing
    mesh.setCameraCoordCallback(testCoordCallback);
    mesh.setSharedAlertCallback(testAlertCallback);
    mesh.setRangeChangeCallback(testRangeCallback);
    mesh.setFLRoundCallback(testFLRoundCallback);
    mesh.setFLModelUpdateCallback(testFLUpdateCallback);
    mesh.setFLAggregationCallback(testFLAggregationCallback);
    
    // Callbacks should not have been triggered
    TEST_ASSERT_FALSE(coordCallbackCalled);
    TEST_ASSERT_FALSE(alertCallbackCalled);
    TEST_ASSERT_FALSE(rangeCallbackCalled);
    TEST_ASSERT_FALSE(flRoundCallbackCalled);
    TEST_ASSERT_FALSE(flUpdateCallbackCalled);
    TEST_ASSERT_FALSE(flAggregationCallbackCalled);
}

//==============================================================================
// CONFIGURATION TESTS
//==============================================================================

void test_mesh_config_defaults(void) {
    // Verify configuration defaults are reasonable
    TEST_ASSERT_TRUE(MESH_MULTI_CAMERA_ENABLED);
    TEST_ASSERT_TRUE(MESH_EXTENDED_RANGE_ENABLED);
    TEST_ASSERT_TRUE(MESH_SHARED_ALERTS_ENABLED);
    TEST_ASSERT_TRUE(MESH_FEDERATED_LEARNING_ENABLED);
    
    TEST_ASSERT_TRUE(MESH_MAX_CAMERAS >= 4 && MESH_MAX_CAMERAS <= 16);
    TEST_ASSERT_TRUE(MESH_COORD_SYNC_INTERVAL_MS >= 1000);
    TEST_ASSERT_TRUE(MESH_ALERT_DEDUP_WINDOW_MS >= 1000);
    TEST_ASSERT_TRUE(MESH_MAX_RECENT_ALERTS >= 10);
    TEST_ASSERT_TRUE(MESH_EXTENDED_RANGE_RSSI_THRESHOLD <= -70);
    
    TEST_ASSERT_TRUE(MESH_FL_MAX_CHUNK_SIZE >= 100 && MESH_FL_MAX_CHUNK_SIZE <= 240);
    TEST_ASSERT_TRUE(MESH_FL_UPDATE_TIMEOUT_MS >= 10000);
    TEST_ASSERT_TRUE(MESH_FL_MAX_PARTICIPANTS >= 4);
}

//==============================================================================
// STATISTICS TESTS
//==============================================================================

void test_network_statistics_format(void) {
    AdvancedMeshNetwork mesh(testMeshManager);
    
    String stats = mesh.getNetworkStatistics();
    
    // Should be valid JSON-like format
    TEST_ASSERT_TRUE(stats.startsWith("{"));
    TEST_ASSERT_TRUE(stats.endsWith("}"));
    TEST_ASSERT_TRUE(stats.indexOf("initialized") > 0);
    TEST_ASSERT_TRUE(stats.indexOf("cameraRole") > 0);
    TEST_ASSERT_TRUE(stats.indexOf("syncState") > 0);
    TEST_ASSERT_TRUE(stats.indexOf("rangeMode") > 0);
}

//==============================================================================
// TEST RUNNER
//==============================================================================

void setup() {
    delay(2000);  // Wait for serial monitor
    
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_advanced_mesh_initial_state);
    RUN_TEST(test_advanced_mesh_cannot_init_without_mesh_manager);
    
    // Camera role tests
    RUN_TEST(test_camera_role_setting);
    RUN_TEST(test_camera_role_enum_values);
    RUN_TEST(test_camera_sync_state_enum_values);
    
    // Camera group tests
    RUN_TEST(test_camera_group_initial_state);
    RUN_TEST(test_camera_group_cannot_create_without_init);
    RUN_TEST(test_camera_group_cannot_join_without_init);
    
    // Extended range tests
    RUN_TEST(test_range_mode_setting);
    RUN_TEST(test_range_mode_enum_values);
    RUN_TEST(test_range_settings_normal);
    RUN_TEST(test_range_settings_extended);
    RUN_TEST(test_range_settings_maximum);
    RUN_TEST(test_relay_nodes_initially_empty);
    
    // Shared alert tests
    RUN_TEST(test_alert_priority_enum_values);
    RUN_TEST(test_alert_type_enum_values);
    RUN_TEST(test_recent_alerts_initially_empty);
    RUN_TEST(test_alert_id_generation);
    RUN_TEST(test_shared_alert_structure);
    RUN_TEST(test_duplicate_alert_detection);
    
    // Federated learning tests
    RUN_TEST(test_fl_node_role_enum_values);
    RUN_TEST(test_fl_round_state_enum_values);
    RUN_TEST(test_fl_compression_enum_values);
    RUN_TEST(test_fl_initial_state);
    RUN_TEST(test_fl_role_setting);
    RUN_TEST(test_fl_compression_setting);
    RUN_TEST(test_fl_round_cannot_start_without_init);
    RUN_TEST(test_fl_round_cannot_join_without_init);
    RUN_TEST(test_fl_model_update_cannot_submit_without_init);
    RUN_TEST(test_fl_round_structure);
    RUN_TEST(test_fl_participant_structure);
    RUN_TEST(test_fl_model_chunk_structure);
    RUN_TEST(test_fl_aggregation_stats_structure);
    
    // Callback tests
    RUN_TEST(test_callback_registration);
    
    // Configuration tests
    RUN_TEST(test_mesh_config_defaults);
    
    // Statistics tests
    RUN_TEST(test_network_statistics_format);
    
    UNITY_END();
}

void loop() {
    // Nothing to do
}
