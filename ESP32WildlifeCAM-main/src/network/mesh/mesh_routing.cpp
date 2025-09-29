/**
 * @file mesh_routing.cpp
 * @brief Intelligent Packet Routing for Wildlife Mesh Network
 * 
 * Implements advanced routing algorithms optimized for wildlife monitoring
 * including multi-hop communication, load balancing, and reliability optimization.
 */

#include "mesh_routing.h"
#include "../../include/network/mesh_config.h"
#include <Arduino.h>
#include <algorithm>

// ===========================
// CONSTRUCTOR & INITIALIZATION
// ===========================

MeshRouting::MeshRouting() 
    : initialized_(false)
    , nodeId_(0)
    , lastTableUpdate_(0)
    , lastOptimization_(0)
    , routeDiscoveryActive_(false)
{
    memset(&statistics_, 0, sizeof(statistics_));
    routingTable_.clear();
    linkQualityTable_.clear();
    routeCache_.clear();
}

MeshRouting::~MeshRouting() {
    cleanup();
}

bool MeshRouting::initialize(uint32_t nodeId, const MeshConfig& config) {
    nodeId_ = nodeId;
    config_ = config;
    
    // Initialize routing tables
    routingTable_.clear();
    linkQualityTable_.clear();
    routeCache_.clear();
    
    // Initialize statistics
    statistics_.startTime = millis();
    statistics_.routesCalculated = 0;
    statistics_.routeDiscoveries = 0;
    statistics_.loadBalanceOperations = 0;
    
    // Set timing
    lastTableUpdate_ = millis();
    lastOptimization_ = millis();
    
    initialized_ = true;
    Serial.printf("Mesh Routing initialized for node %08X\n", nodeId_);
    return true;
}

void MeshRouting::cleanup() {
    routingTable_.clear();
    linkQualityTable_.clear();
    routeCache_.clear();
    initialized_ = false;
}

// ===========================
// MAIN ROUTING PROCESSING
// ===========================

void MeshRouting::processRouting() {
    if (!initialized_) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Update routing table periodically
    if (currentTime - lastTableUpdate_ > MESH_ROUTING_UPDATE_INTERVAL) {
        updateRoutingTable();
        lastTableUpdate_ = currentTime;
    }
    
    // Perform route optimization
    if (currentTime - lastOptimization_ > MESH_OPTIMIZATION_INTERVAL) {
        optimizeRoutes();
        lastOptimization_ = currentTime;
    }
    
    // Clean up expired routes
    cleanupExpiredRoutes();
    
    // Update link quality metrics
    updateLinkQuality();
    
    // Update statistics
    updateStatistics();
}

void MeshRouting::updateRoutingTable() {
    Serial.println("Updating routing table...");
    
    // Run Dijkstra's algorithm for shortest path calculation
    calculateShortestPaths();
    
    // Apply wildlife-specific routing optimizations
    applyWildlifeOptimizations();
    
    // Update route cache
    updateRouteCache();
    
    statistics_.routesCalculated++;
}

void MeshRouting::calculateShortestPaths() {
    // Implementation of Dijkstra's algorithm for mesh routing
    std::map<uint32_t, float> distances;
    std::map<uint32_t, uint32_t> previous;
    std::vector<uint32_t> unvisited;
    
    // Initialize distances
    for (const auto& link : linkQualityTable_) {
        uint32_t nodeId = link.first;
        distances[nodeId] = std::numeric_limits<float>::infinity();
        unvisited.push_back(nodeId);
    }
    distances[nodeId_] = 0.0;
    
    while (!unvisited.empty()) {
        // Find node with minimum distance
        uint32_t currentNode = findMinimumDistanceNode(unvisited, distances);
        if (distances[currentNode] == std::numeric_limits<float>::infinity()) {
            break;  // No more reachable nodes
        }
        
        // Remove from unvisited
        unvisited.erase(std::find(unvisited.begin(), unvisited.end(), currentNode));
        
        // Update distances to neighbors
        updateNeighborDistances(currentNode, distances, previous);
    }
    
    // Build routing table from shortest paths
    buildRoutingTableFromPaths(distances, previous);
}

void MeshRouting::optimizeRoutes() {
    if (!config_.loadBalancingEnabled) {
        return;
    }
    
    Serial.println("Optimizing routes for load balancing...");
    
    // Identify congested routes
    std::vector<uint32_t> congestedRoutes = identifyCongestedRoutes();
    
    // Find alternative paths for congested routes
    for (uint32_t destination : congestedRoutes) {
        RouteEntry* primaryRoute = findRoute(destination);
        if (primaryRoute && primaryRoute->utilization > MESH_CONGESTION_THRESHOLD) {
            
            // Find alternative path
            RouteEntry alternativeRoute = findAlternativeRoute(destination, primaryRoute->nextHop);
            if (alternativeRoute.destination != 0) {
                
                // Implement load balancing
                implementLoadBalancing(destination, *primaryRoute, alternativeRoute);
                statistics_.loadBalanceOperations++;
            }
        }
    }
    
    lastOptimization_ = millis();
}

// ===========================
// ROUTE DISCOVERY
// ===========================

bool MeshRouting::discoverRoute(uint32_t destination) {
    if (findRoute(destination) != nullptr) {
        return true;  // Route already exists
    }
    
    Serial.printf("Discovering route to %08X\n", destination);
    
    // Create route discovery packet
    RouteDiscoveryPacket discovery;
    discovery.originId = nodeId_;
    discovery.destinationId = destination;
    discovery.requestId = generateRequestId();
    discovery.hopCount = 0;
    discovery.timestamp = millis();
    
    // Broadcast route discovery request
    broadcastRouteDiscovery(discovery);
    
    routeDiscoveryActive_ = true;
    statistics_.routeDiscoveries++;
    
    return waitForRouteDiscoveryResponse(discovery.requestId, MESH_ROUTE_DISCOVERY_TIMEOUT);
}

void MeshRouting::handleRouteDiscoveryRequest(const RouteDiscoveryPacket& request) {
    // Check if we are the destination
    if (request.destinationId == nodeId_) {
        // Send route discovery response
        sendRouteDiscoveryResponse(request);
        return;
    }
    
    // Check if we have already seen this request
    if (isDiscoveryRequestSeen(request.requestId, request.originId)) {
        return;  // Ignore duplicate request
    }
    
    // Add to seen requests
    addSeenDiscoveryRequest(request.requestId, request.originId);
    
    // Forward request if hop limit allows
    if (request.hopCount < MESH_MAX_HOPS) {
        forwardRouteDiscoveryRequest(request);
    }
}

void MeshRouting::handleRouteDiscoveryResponse(const RouteDiscoveryResponse& response) {
    // Update routing table with discovered route
    RouteEntry route;
    route.destination = response.destinationId;
    route.nextHop = response.nextHopId;
    route.hopCount = response.hopCount;
    route.metric = response.pathMetric;
    route.lastUsed = millis();
    route.reliability = 1.0;  // New route starts with full reliability
    route.utilization = 0.0;
    route.wildlifePriority = false;
    
    addRoute(route);
    
    Serial.printf("Route discovered: %08X via %08X (%d hops)\n", 
                 response.destinationId, response.nextHopId, response.hopCount);
}

// ===========================
// ROUTE MANAGEMENT
// ===========================

RouteEntry* MeshRouting::findRoute(uint32_t destination) {
    auto it = routingTable_.find(destination);
    return (it != routingTable_.end()) ? &it->second : nullptr;
}

bool MeshRouting::addRoute(const RouteEntry& route) {
    routingTable_[route.destination] = route;
    
    // Update route cache
    updateRouteCacheEntry(route.destination);
    
    // Notify of route change if callback is set
    if (routeChangeCallback_) {
        routeChangeCallback_(route.destination, route.nextHop, route.hopCount);
    }
    
    return true;
}

bool MeshRouting::removeRoute(uint32_t destination) {
    auto it = routingTable_.find(destination);
    if (it != routingTable_.end()) {
        routingTable_.erase(it);
        
        // Remove from route cache
        removeRouteCacheEntry(destination);
        
        Serial.printf("Route to %08X removed\n", destination);
        return true;
    }
    return false;
}

uint32_t MeshRouting::getNextHop(uint32_t destination) {
    // Check route cache first for performance
    auto cacheIt = routeCache_.find(destination);
    if (cacheIt != routeCache_.end() && 
        millis() - cacheIt->second.timestamp < MESH_ROUTE_CACHE_TIMEOUT) {
        return cacheIt->second.nextHop;
    }
    
    // Look up in routing table
    RouteEntry* route = findRoute(destination);
    if (route) {
        // Update route usage
        route->lastUsed = millis();
        route->utilization += 0.1;  // Increment utilization
        
        // Update cache
        updateRouteCacheEntry(destination);
        
        return route->nextHop;
    }
    
    // No route found - trigger route discovery
    if (!routeDiscoveryActive_) {
        discoverRoute(destination);
    }
    
    return 0;  // No route available
}

// ===========================
// LOAD BALANCING
// ===========================

std::vector<uint32_t> MeshRouting::identifyCongestedRoutes() {
    std::vector<uint32_t> congested;
    
    for (const auto& route : routingTable_) {
        if (route.second.utilization > MESH_CONGESTION_THRESHOLD) {
            congested.push_back(route.first);
        }
    }
    
    return congested;
}

RouteEntry MeshRouting::findAlternativeRoute(uint32_t destination, uint32_t excludeNextHop) {
    RouteEntry alternative;
    alternative.destination = 0;  // Invalid route marker
    
    float bestMetric = std::numeric_limits<float>::infinity();
    
    // Look through all possible next hops
    for (const auto& link : linkQualityTable_) {
        uint32_t candidateNextHop = link.first;
        
        if (candidateNextHop == excludeNextHop || candidateNextHop == nodeId_) {
            continue;  // Skip excluded next hop and self
        }
        
        // Calculate metric for this path
        float pathMetric = calculatePathMetric(candidateNextHop, destination);
        
        if (pathMetric < bestMetric && pathMetric < std::numeric_limits<float>::infinity()) {
            bestMetric = pathMetric;
            alternative.destination = destination;
            alternative.nextHop = candidateNextHop;
            alternative.metric = pathMetric;
            alternative.hopCount = estimateHopCount(candidateNextHop, destination);
            alternative.reliability = link.second.reliability;
            alternative.utilization = 0.0;  // New path starts with zero utilization
            alternative.lastUsed = millis();
            alternative.wildlifePriority = false;
        }
    }
    
    return alternative;
}

void MeshRouting::implementLoadBalancing(uint32_t destination, 
                                       RouteEntry& primaryRoute, 
                                       const RouteEntry& alternativeRoute) {
    // Calculate traffic split ratio based on link quality
    float primaryWeight = primaryRoute.reliability * (1.0 - primaryRoute.utilization);
    float alternativeWeight = alternativeRoute.reliability;
    
    float totalWeight = primaryWeight + alternativeWeight;
    if (totalWeight > 0) {
        float alternativeRatio = alternativeWeight / totalWeight;
        
        // Implement traffic splitting
        if (alternativeRatio > 0.3) {  // Use alternative for at least 30% of traffic
            // Add alternative route to routing table
            RouteEntry balancedRoute = alternativeRoute;
            balancedRoute.loadBalancePartner = primaryRoute.nextHop;
            balancedRoute.trafficRatio = alternativeRatio;
            
            // Create load-balanced route entry
            uint32_t alternativeKey = destination | 0x80000000;  // Mark as alternative
            routingTable_[alternativeKey] = balancedRoute;
            
            // Update primary route
            primaryRoute.loadBalancePartner = alternativeRoute.nextHop;
            primaryRoute.trafficRatio = 1.0 - alternativeRatio;
            
            Serial.printf("Load balancing implemented for %08X: %.0f%% primary, %.0f%% alternative\n", 
                         destination, primaryRoute.trafficRatio * 100, alternativeRatio * 100);
        }
    }
}

// ===========================
// LINK QUALITY MANAGEMENT
// ===========================

void MeshRouting::updateLinkQuality() {
    for (auto& link : linkQualityTable_) {
        LinkQuality& quality = link.second;
        
        // Decay reliability over time if no recent updates
        if (millis() - quality.lastUpdate > MESH_LINK_QUALITY_TIMEOUT) {
            quality.reliability *= 0.9;  // 10% decay
            quality.packetLoss = std::min(quality.packetLoss + 0.05f, 1.0f);
        }
        
        // Update link metric based on quality factors
        updateLinkMetric(quality);
    }
}

void MeshRouting::reportLinkQuality(uint32_t neighborId, float reliability, 
                                  float packetLoss, int16_t signalStrength) {
    LinkQuality& quality = linkQualityTable_[neighborId];
    
    quality.neighborId = neighborId;
    quality.reliability = reliability;
    quality.packetLoss = packetLoss;
    quality.signalStrength = signalStrength;
    quality.lastUpdate = millis();
    
    // Calculate link metric (lower is better)
    updateLinkMetric(quality);
    
    // Trigger routing table update if significant change
    if (isSignificantQualityChange(quality)) {
        updateRoutingTable();
    }
}

void MeshRouting::updateLinkMetric(LinkQuality& quality) {
    // Calculate composite metric considering multiple factors
    float reliabilityFactor = 1.0 - quality.reliability;
    float packetLossFactor = quality.packetLoss;
    float signalFactor = (quality.signalStrength < -80) ? 0.5 : 0.1;
    
    quality.metric = reliabilityFactor + packetLossFactor + signalFactor;
    
    // Wildlife-specific adjustments
    if (quality.wildlifePriority) {
        quality.metric *= 0.8;  // 20% bonus for wildlife priority links
    }
}

// ===========================
// WILDLIFE-SPECIFIC OPTIMIZATIONS
// ===========================

void MeshRouting::applyWildlifeOptimizations() {
    // Prioritize routes for nodes with active wildlife detections
    for (auto& route : routingTable_) {
        RouteEntry& entry = route.second;
        
        // Check if destination has wildlife detection
        if (hasWildlifeDetection(entry.destination)) {
            entry.wildlifePriority = true;
            entry.metric *= 0.7;  // 30% bonus for wildlife nodes
            
            // Ensure higher reliability for wildlife routes
            if (entry.reliability < 0.8) {
                findHighReliabilityPath(entry.destination);
            }
        }
    }
}

void MeshRouting::prioritizeWildlifeRoute(uint32_t destination) {
    RouteEntry* route = findRoute(destination);
    if (route) {
        route->wildlifePriority = true;
        route->metric *= 0.5;  // Strong priority for wildlife data
        
        // Update link quality for wildlife priority
        auto linkIt = linkQualityTable_.find(route->nextHop);
        if (linkIt != linkQualityTable_.end()) {
            linkIt->second.wildlifePriority = true;
        }
        
        Serial.printf("Wildlife priority applied to route %08X\n", destination);
    }
}

bool MeshRouting::optimizeForImageTransmission(uint32_t destination, size_t imageSize) {
    RouteEntry* route = findRoute(destination);
    if (!route) {
        return false;
    }
    
    // Check if current route can handle large image transmission
    float requiredBandwidth = calculateRequiredBandwidth(imageSize);
    float availableBandwidth = estimateAvailableBandwidth(route->nextHop);
    
    if (availableBandwidth < requiredBandwidth) {
        // Find route with better bandwidth
        RouteEntry betterRoute = findHighBandwidthRoute(destination, requiredBandwidth);
        if (betterRoute.destination != 0) {
            // Temporarily use better route for image transmission
            addRoute(betterRoute);
            Serial.printf("Optimized route for image transmission: %08X via %08X\n", 
                         destination, betterRoute.nextHop);
            return true;
        }
    }
    
    return false;
}

// ===========================
// UTILITY METHODS
// ===========================

MeshRoutingStatistics MeshRouting::getStatistics() const {
    return statistics_;
}

size_t MeshRouting::getRoutingTableSize() const {
    return routingTable_.size();
}

std::vector<RouteEntry> MeshRouting::getAllRoutes() const {
    std::vector<RouteEntry> routes;
    for (const auto& entry : routingTable_) {
        routes.push_back(entry.second);
    }
    return routes;
}

void MeshRouting::setRouteChangeCallback(RouteChangeCallback callback) {
    routeChangeCallback_ = callback;
}

void MeshRouting::updateStatistics() {
    statistics_.uptime = millis() - statistics_.startTime;
    statistics_.routingTableSize = routingTable_.size();
    statistics_.linkQualityEntries = linkQualityTable_.size();
    statistics_.cacheHitRate = calculateCacheHitRate();
}

// ===========================
// PRIVATE HELPER METHODS
// ===========================

uint32_t MeshRouting::findMinimumDistanceNode(const std::vector<uint32_t>& unvisited,
                                            const std::map<uint32_t, float>& distances) {
    uint32_t minNode = 0;
    float minDistance = std::numeric_limits<float>::infinity();
    
    for (uint32_t node : unvisited) {
        auto it = distances.find(node);
        if (it != distances.end() && it->second < minDistance) {
            minDistance = it->second;
            minNode = node;
        }
    }
    
    return minNode;
}

void MeshRouting::updateNeighborDistances(uint32_t currentNode,
                                        std::map<uint32_t, float>& distances,
                                        std::map<uint32_t, uint32_t>& previous) {
    // Get neighbors of current node
    std::vector<uint32_t> neighbors = getNeighbors(currentNode);
    
    for (uint32_t neighbor : neighbors) {
        float edgeWeight = getLinkMetric(currentNode, neighbor);
        float altDistance = distances[currentNode] + edgeWeight;
        
        if (altDistance < distances[neighbor]) {
            distances[neighbor] = altDistance;
            previous[neighbor] = currentNode;
        }
    }
}

void MeshRouting::buildRoutingTableFromPaths(const std::map<uint32_t, float>& distances,
                                           const std::map<uint32_t, uint32_t>& previous) {
    for (const auto& dist : distances) {
        uint32_t destination = dist.first;
        if (destination == nodeId_ || dist.second == std::numeric_limits<float>::infinity()) {
            continue;
        }
        
        // Build route entry
        RouteEntry route;
        route.destination = destination;
        route.metric = dist.second;
        route.lastUsed = millis();
        route.reliability = 1.0;
        route.utilization = 0.0;
        route.wildlifePriority = false;
        
        // Find next hop by tracing back from destination
        uint32_t current = destination;
        while (previous.find(current) != previous.end() && previous.at(current) != nodeId_) {
            current = previous.at(current);
        }
        route.nextHop = current;
        
        // Calculate hop count
        route.hopCount = calculateHopCount(destination, previous);
        
        // Add to routing table
        routingTable_[destination] = route;
    }
}

std::vector<uint32_t> MeshRouting::getNeighbors(uint32_t nodeId) {
    std::vector<uint32_t> neighbors;
    
    if (nodeId == nodeId_) {
        // Return direct neighbors
        for (const auto& link : linkQualityTable_) {
            neighbors.push_back(link.first);
        }
    }
    
    return neighbors;
}

float MeshRouting::getLinkMetric(uint32_t from, uint32_t to) {
    if (from == nodeId_) {
        auto it = linkQualityTable_.find(to);
        return (it != linkQualityTable_.end()) ? it->second.metric : std::numeric_limits<float>::infinity();
    }
    
    // For non-direct links, use default metric
    return 1.0;
}

uint8_t MeshRouting::calculateHopCount(uint32_t destination, const std::map<uint32_t, uint32_t>& previous) {
    uint8_t hops = 0;
    uint32_t current = destination;
    
    while (previous.find(current) != previous.end() && current != nodeId_) {
        current = previous.at(current);
        hops++;
        if (hops >= MESH_MAX_HOPS) break;  // Prevent infinite loop
    }
    
    return hops;
}

float MeshRouting::calculateCacheHitRate() {
    // Simplified cache hit rate calculation
    return (statistics_.cacheHits > 0) ? 
           (float)statistics_.cacheHits / (statistics_.cacheHits + statistics_.cacheMisses) : 0.0;
}

bool MeshRouting::hasWildlifeDetection(uint32_t nodeId) {
    // This would interface with the wildlife detection system
    // For now, return false as placeholder
    return false;
}

void MeshRouting::findHighReliabilityPath(uint32_t destination) {
    // Implementation would find path with highest reliability
    Serial.printf("Finding high reliability path to %08X\n", destination);
}

float MeshRouting::calculateRequiredBandwidth(size_t imageSize) {
    // Calculate bandwidth needed for image transmission
    return (float)imageSize / MESH_IMAGE_TRANSMISSION_TIME;
}

float MeshRouting::estimateAvailableBandwidth(uint32_t nextHop) {
    auto it = linkQualityTable_.find(nextHop);
    if (it != linkQualityTable_.end()) {
        return MESH_MAX_BANDWIDTH * (1.0 - it->second.packetLoss) * it->second.reliability;
    }
    return MESH_MAX_BANDWIDTH * 0.5;  // Conservative estimate
}

RouteEntry MeshRouting::findHighBandwidthRoute(uint32_t destination, float requiredBandwidth) {
    RouteEntry bestRoute;
    bestRoute.destination = 0;  // Invalid marker
    
    float bestBandwidth = 0;
    
    for (const auto& link : linkQualityTable_) {
        float bandwidth = estimateAvailableBandwidth(link.first);
        if (bandwidth >= requiredBandwidth && bandwidth > bestBandwidth) {
            bestBandwidth = bandwidth;
            bestRoute.destination = destination;
            bestRoute.nextHop = link.first;
            bestRoute.metric = 1.0 / bandwidth;  // Inverse of bandwidth
        }
    }
    
    return bestRoute;
}

// Additional helper method implementations would continue here...