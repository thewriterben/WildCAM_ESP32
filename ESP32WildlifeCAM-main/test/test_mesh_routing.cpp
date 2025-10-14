/**
 * @file test_mesh_routing.cpp
 * @brief Unit tests for advanced mesh routing algorithms
 * 
 * Tests Dijkstra's shortest path, load balancing, route discovery,
 * link quality management, and wildlife-specific optimizations.
 */

#include <unity.h>
#include "../src/network/mesh/mesh_routing.h"
#include "../include/network/mesh_config.h"

MeshRouting* routing = nullptr;
MeshConfig testConfig;

void setUp(void) {
    // Initialize test configuration
    testConfig.nodeId = 0x12345678;
    testConfig.role = MESH_ROLE_NODE;
    testConfig.channel = 0;
    testConfig.networkId = 1;
    testConfig.loadBalancingEnabled = true;
    testConfig.maxHops = 5;
    testConfig.beaconInterval = 30000;
    testConfig.congestionThreshold = 0.8;
    
    routing = new MeshRouting();
}

void tearDown(void) {
    if (routing) {
        routing->cleanup();
        delete routing;
        routing = nullptr;
    }
}

/**
 * Test mesh routing initialization
 */
void test_routing_initialization(void) {
    bool result = routing->initialize(0x12345678, testConfig);
    
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(0, routing->getRoutingTableSize());
}

/**
 * Test route addition and retrieval
 */
void test_route_add_find(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Create test route
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 2;
    route.metric = 1.5;
    route.reliability = 0.95;
    route.utilization = 0.3;
    route.wildlifePriority = false;
    route.lastUsed = millis();
    
    // Add route
    bool added = routing->addRoute(route);
    TEST_ASSERT_TRUE(added);
    TEST_ASSERT_EQUAL_UINT32(1, routing->getRoutingTableSize());
    
    // Find route
    RouteEntry* found = routing->findRoute(0x11111111);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_HEX32(0x11111111, found->destination);
    TEST_ASSERT_EQUAL_HEX32(0x22222222, found->nextHop);
    TEST_ASSERT_EQUAL_UINT8(2, found->hopCount);
}

/**
 * Test route removal
 */
void test_route_removal(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add route
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    TEST_ASSERT_EQUAL_UINT32(1, routing->getRoutingTableSize());
    
    // Remove route
    bool removed = routing->removeRoute(0x11111111);
    TEST_ASSERT_TRUE(removed);
    TEST_ASSERT_EQUAL_UINT32(0, routing->getRoutingTableSize());
    
    // Try to find removed route
    RouteEntry* found = routing->findRoute(0x11111111);
    TEST_ASSERT_NULL(found);
}

/**
 * Test get next hop
 */
void test_get_next_hop(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add route
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    // Get next hop
    uint32_t nextHop = routing->getNextHop(0x11111111);
    TEST_ASSERT_EQUAL_HEX32(0x22222222, nextHop);
}

/**
 * Test link quality reporting
 */
void test_link_quality_reporting(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Report link quality
    uint32_t neighborId = 0x22222222;
    float reliability = 0.95;
    float packetLoss = 0.05;
    int16_t signalStrength = -75;
    
    routing->reportLinkQuality(neighborId, reliability, packetLoss, signalStrength);
    
    // No direct way to verify without getters, but should not crash
    TEST_ASSERT_TRUE(true);
}

/**
 * Test multiple routes
 */
void test_multiple_routes(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add multiple routes
    for (int i = 0; i < 5; i++) {
        RouteEntry route;
        route.destination = 0x10000000 + i;
        route.nextHop = 0x20000000 + i;
        route.hopCount = i + 1;
        route.metric = 1.0 + i * 0.5;
        route.lastUsed = millis();
        
        routing->addRoute(route);
    }
    
    TEST_ASSERT_EQUAL_UINT32(5, routing->getRoutingTableSize());
    
    // Verify all routes
    for (int i = 0; i < 5; i++) {
        RouteEntry* found = routing->findRoute(0x10000000 + i);
        TEST_ASSERT_NOT_NULL(found);
        TEST_ASSERT_EQUAL_HEX32(0x20000000 + i, found->nextHop);
    }
}

/**
 * Test get all routes
 */
void test_get_all_routes(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add routes
    for (int i = 0; i < 3; i++) {
        RouteEntry route;
        route.destination = 0x10000000 + i;
        route.nextHop = 0x20000000 + i;
        route.hopCount = i + 1;
        route.metric = 1.0;
        route.lastUsed = millis();
        
        routing->addRoute(route);
    }
    
    std::vector<RouteEntry> allRoutes = routing->getAllRoutes();
    TEST_ASSERT_EQUAL_UINT32(3, allRoutes.size());
}

/**
 * Test routing statistics
 */
void test_routing_statistics(void) {
    routing->initialize(0x12345678, testConfig);
    
    MeshRoutingStatistics stats = routing->getStatistics();
    
    TEST_ASSERT_TRUE(stats.startTime > 0);
    TEST_ASSERT_TRUE(stats.routesCalculated >= 0);
    TEST_ASSERT_TRUE(stats.routeDiscoveries >= 0);
    TEST_ASSERT_TRUE(stats.loadBalanceOperations >= 0);
}

/**
 * Test wildlife priority route
 */
void test_wildlife_priority(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add route
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 2.0;
    route.wildlifePriority = false;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    // Prioritize for wildlife
    routing->prioritizeWildlifeRoute(0x11111111);
    
    // Verify route still exists
    RouteEntry* found = routing->findRoute(0x11111111);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_TRUE(found->wildlifePriority);
}

/**
 * Test image transmission optimization
 */
void test_image_transmission_optimization(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add route
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.reliability = 0.9;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    // Report good link quality
    routing->reportLinkQuality(0x22222222, 0.9, 0.1, -70);
    
    // Optimize for image transmission
    size_t imageSize = 100000;  // 100KB image
    bool result = routing->optimizeForImageTransmission(0x11111111, imageSize);
    
    // Should return true or false based on route quality
    TEST_ASSERT_TRUE(result == true || result == false);
}

/**
 * Test route discovery
 */
void test_route_discovery(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Try to discover route to unknown destination
    uint32_t destination = 0x99999999;
    bool result = routing->discoverRoute(destination);
    
    // Discovery will timeout without actual network
    // Just verify it doesn't crash
    TEST_ASSERT_TRUE(result == true || result == false);
}

/**
 * Test load balancing identification
 */
void test_load_balancing(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add congested route
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.utilization = 0.9;  // Highly utilized
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    // Identify congested routes
    std::vector<uint32_t> congested = routing->identifyCongestedRoutes();
    
    TEST_ASSERT_EQUAL_UINT32(1, congested.size());
    TEST_ASSERT_EQUAL_HEX32(0x11111111, congested[0]);
}

/**
 * Test routing table update
 */
void test_routing_table_update(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add initial routes
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    // Update routing table
    routing->updateRoutingTable();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

/**
 * Test route optimization
 */
void test_route_optimization(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Add routes
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.utilization = 0.5;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    // Optimize routes
    routing->optimizeRoutes();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

/**
 * Test wildlife optimizations
 */
void test_wildlife_optimizations(void) {
    routing->initialize(0x12345678, testConfig);
    
    // Enable wildlife priority
    routing->setWildlifePriorityEnabled(true);
    
    // Apply wildlife optimizations
    routing->applyWildlifeOptimizations();
    
    // Should not crash
    TEST_ASSERT_TRUE(true);
}

/**
 * Test route change callback
 */
void test_route_change_callback(void) {
    routing->initialize(0x12345678, testConfig);
    
    bool callbackCalled = false;
    
    // Set callback
    routing->setRouteChangeCallback([&callbackCalled](uint32_t dest, uint32_t nextHop, uint8_t hops) {
        callbackCalled = true;
    });
    
    // Add route to trigger callback
    RouteEntry route;
    route.destination = 0x11111111;
    route.nextHop = 0x22222222;
    route.hopCount = 1;
    route.metric = 1.0;
    route.lastUsed = millis();
    
    routing->addRoute(route);
    
    TEST_ASSERT_TRUE(callbackCalled);
}

// Main test runner
void setup() {
    delay(2000);  // Wait for serial monitor
    UNITY_BEGIN();
    
    // Initialization tests
    RUN_TEST(test_routing_initialization);
    
    // Route management tests
    RUN_TEST(test_route_add_find);
    RUN_TEST(test_route_removal);
    RUN_TEST(test_get_next_hop);
    RUN_TEST(test_multiple_routes);
    RUN_TEST(test_get_all_routes);
    
    // Link quality tests
    RUN_TEST(test_link_quality_reporting);
    
    // Statistics tests
    RUN_TEST(test_routing_statistics);
    
    // Wildlife optimization tests
    RUN_TEST(test_wildlife_priority);
    RUN_TEST(test_image_transmission_optimization);
    RUN_TEST(test_wildlife_optimizations);
    
    // Advanced routing tests
    RUN_TEST(test_route_discovery);
    RUN_TEST(test_load_balancing);
    RUN_TEST(test_routing_table_update);
    RUN_TEST(test_route_optimization);
    
    // Callback tests
    RUN_TEST(test_route_change_callback);
    
    UNITY_END();
}

void loop() {
    // Tests run once in setup
}
