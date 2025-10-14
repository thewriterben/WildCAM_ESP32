/**
 * @file swarm_navigation.h
 * @brief Advanced navigation and positioning system for swarm members
 * 
 * Provides:
 * - GPS/GNSS integration with RTK precision
 * - Visual-inertial odometry
 * - SLAM (Simultaneous Localization and Mapping)
 * - Swarm-based relative positioning
 * - Geofencing and no-fly zones
 * - Dynamic path planning
 * - Collision avoidance
 */

#ifndef SWARM_NAVIGATION_H
#define SWARM_NAVIGATION_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "swarm_coordinator.h"

// Navigation mode
enum NavigationMode {
    NAV_GPS = 0,
    NAV_GPS_RTK = 1,
    NAV_VISUAL_INERTIAL = 2,
    NAV_SLAM = 3,
    NAV_RELATIVE = 4,
    NAV_HYBRID = 5
};

// GPS accuracy level
enum GPSAccuracy {
    GPS_STANDARD = 0,      // ~5-10m accuracy
    GPS_DIFFERENTIAL = 1,  // ~1-3m accuracy
    GPS_RTK_FLOAT = 2,     // ~0.5-1m accuracy
    GPS_RTK_FIXED = 3      // ~0.01-0.05m accuracy (centimeter)
};

// Obstacle type
enum ObstacleType {
    OBSTACLE_STATIC = 0,
    OBSTACLE_DYNAMIC = 1,
    OBSTACLE_NO_FLY_ZONE = 2,
    OBSTACLE_TERRAIN = 3,
    OBSTACLE_WEATHER = 4
};

// GPS coordinate
struct GPSCoordinate {
    double latitude;   // degrees
    double longitude;  // degrees
    double altitude;   // meters above sea level
    GPSAccuracy accuracy;
    unsigned long timestamp;
};

// Obstacle information
struct Obstacle {
    int obstacleId;
    ObstacleType type;
    Position3D position;
    Position3D velocity;  // For dynamic obstacles
    float radius;         // Collision radius (meters)
    float height;         // Height (meters)
    bool isActive;
    unsigned long detectedTime;
};

// Geofence boundary
struct Geofence {
    int fenceId;
    std::vector<GPSCoordinate> boundary;
    float minAltitude;
    float maxAltitude;
    bool isExclusionZone;  // true = no-fly, false = must-stay-within
    String description;
};

// Path segment
struct PathSegment {
    Position3D start;
    Position3D end;
    float cost;
    float distance;
    std::vector<Obstacle> obstacles;
};

// Navigation waypoint
struct Waypoint {
    int waypointId;
    Position3D position;
    float arrivalRadius;  // meters
    float speed;          // desired speed at waypoint
    unsigned long duration; // time to stay at waypoint (ms)
    String action;        // action to perform at waypoint
};

// Map representation for SLAM
struct SLAMMap {
    std::vector<Position3D> landmarks;
    std::vector<Obstacle> obstacles;
    Position3D boundMin;
    Position3D boundMax;
    float resolution;     // meters per grid cell
    unsigned long lastUpdate;
};

// Navigation statistics
struct NavigationStats {
    int totalWaypoints;
    int completedWaypoints;
    float distanceTraveled;
    float averageSpeed;
    int collisionAvoidances;
    int geofenceViolations;
    GPSAccuracy currentAccuracy;
    float positionConfidence;
};

class SwarmNavigation {
public:
    SwarmNavigation();
    ~SwarmNavigation();
    
    // ===========================
    // INITIALIZATION
    // ===========================
    bool init(NavigationMode mode);
    void cleanup();
    bool isInitialized() const { return initialized_; }
    
    // ===========================
    // GPS/GNSS POSITIONING
    // ===========================
    bool updateGPSPosition(int memberId, const GPSCoordinate& gps);
    GPSCoordinate getGPSPosition(int memberId) const;
    Position3D gpsToLocal(const GPSCoordinate& gps) const;
    GPSCoordinate localToGPS(const Position3D& local) const;
    bool enableRTK(bool enable);
    GPSAccuracy getGPSAccuracy(int memberId) const;
    
    // ===========================
    // VISUAL-INERTIAL ODOMETRY
    // ===========================
    bool updateVisualOdometry(int memberId, const Position3D& position, 
                             const Velocity3D& velocity);
    bool calibrateVIO(int memberId);
    float getVIOConfidence(int memberId) const;
    
    // ===========================
    // SLAM
    // ===========================
    bool initSLAM();
    bool updateSLAM(int memberId, const std::vector<Position3D>& observations);
    SLAMMap getSLAMMap() const;
    bool addLandmark(const Position3D& landmark);
    std::vector<Position3D> getLandmarks() const;
    
    // ===========================
    // SWARM-BASED RELATIVE POSITIONING
    // ===========================
    bool updateRelativePosition(int memberId, int referenceId, 
                               const Position3D& relativePos);
    Position3D calculateRelativePosition(int memberA, int memberB) const;
    bool triangulateMember(int memberId);
    std::vector<int> getVisibleNeighbors(int memberId, float range) const;
    
    // ===========================
    // GEOFENCING
    // ===========================
    bool addGeofence(const Geofence& fence);
    bool removeGeofence(int fenceId);
    bool isWithinGeofence(const Position3D& position, int fenceId) const;
    bool checkGeofenceViolation(int memberId) const;
    std::vector<Geofence> getActiveGeofences() const;
    Position3D getNearestSafePosition(const Position3D& position) const;
    
    // ===========================
    // OBSTACLE DETECTION & AVOIDANCE
    // ===========================
    bool addObstacle(const Obstacle& obstacle);
    bool removeObstacle(int obstacleId);
    bool updateObstacle(int obstacleId, const Position3D& position);
    std::vector<Obstacle> detectObstacles(int memberId, float range) const;
    Position3D calculateAvoidanceVector(int memberId, const Position3D& target);
    bool isPathClear(const Position3D& start, const Position3D& end) const;
    float calculateCollisionTime(int memberId, const Obstacle& obstacle) const;
    
    // ===========================
    // PATH PLANNING
    // ===========================
    std::vector<Position3D> planPath(const Position3D& start, 
                                     const Position3D& goal);
    std::vector<Position3D> planPathAStar(const Position3D& start, 
                                         const Position3D& goal);
    std::vector<Position3D> planPathRRT(const Position3D& start, 
                                       const Position3D& goal);
    std::vector<Position3D> optimizePath(const std::vector<Position3D>& path);
    float calculatePathCost(const std::vector<Position3D>& path) const;
    
    // ===========================
    // WAYPOINT NAVIGATION
    // ===========================
    bool addWaypoint(int memberId, const Waypoint& waypoint);
    bool clearWaypoints(int memberId);
    Waypoint getCurrentWaypoint(int memberId) const;
    bool advanceToNextWaypoint(int memberId);
    float distanceToWaypoint(int memberId) const;
    bool hasReachedWaypoint(int memberId) const;
    
    // ===========================
    // COLLISION AVOIDANCE
    // ===========================
    bool enableCollisionAvoidance(bool enable);
    Velocity3D calculateCollisionAvoidanceVelocity(int memberId);
    bool predictCollision(int memberA, int memberB, float timeHorizon) const;
    Position3D calculateSafePosition(int memberId, const Position3D& desired);
    
    // ===========================
    // WEATHER & TERRAIN ADAPTATION
    // ===========================
    bool setWeatherConditions(float windSpeed, float windDirection, float visibility);
    Position3D adjustForWind(const Position3D& desired, const Velocity3D& wind) const;
    bool isWeatherSafe() const;
    float getTerrainAltitude(float x, float y) const;
    bool maintainTerrainFollowing(int memberId, float altitude);
    
    // ===========================
    // STATISTICS & MONITORING
    // ===========================
    NavigationStats getStatistics(int memberId) const;
    void logNavigationEvent(const String& event, const String& details = "");
    
private:
    // State
    bool initialized_;
    NavigationMode mode_;
    bool rtkEnabled_;
    bool collisionAvoidanceEnabled_;
    
    // GPS data
    std::map<int, GPSCoordinate> memberGPS_;
    GPSCoordinate referencePoint_;  // Local coordinate origin
    
    // VIO data
    std::map<int, float> vioConfidence_;
    
    // SLAM
    SLAMMap slamMap_;
    bool slamInitialized_;
    
    // Geofences
    std::vector<Geofence> geofences_;
    
    // Obstacles
    std::vector<Obstacle> obstacles_;
    int nextObstacleId_;
    
    // Waypoints
    std::map<int, std::vector<Waypoint>> memberWaypoints_;
    std::map<int, int> currentWaypointIndex_;
    
    // Weather
    float windSpeed_;
    float windDirection_;
    float visibility_;
    
    // Statistics
    std::map<int, NavigationStats> memberStats_;
    
    // Private helper methods
    float calculateDistance2D(const Position3D& a, const Position3D& b) const;
    float calculateDistance3D(const Position3D& a, const Position3D& b) const;
    bool lineIntersectsObstacle(const Position3D& start, const Position3D& end,
                               const Obstacle& obstacle) const;
    Position3D interpolate(const Position3D& a, const Position3D& b, float t) const;
    float heuristicCost(const Position3D& a, const Position3D& b) const;
    bool isPositionValid(const Position3D& position) const;
    void updateStatistics(int memberId);
};

// Global navigation instance
extern SwarmNavigation* g_swarmNavigation;

// Utility functions
bool initializeSwarmNavigation(NavigationMode mode);
void processSwarmNavigation();
Position3D getNavigationPosition(int memberId);
void cleanupSwarmNavigation();

#endif // SWARM_NAVIGATION_H
