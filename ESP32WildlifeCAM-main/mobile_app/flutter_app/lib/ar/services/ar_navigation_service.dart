import 'dart:async';
import 'dart:math' as math;
import 'package:flutter/foundation.dart';
import 'package:geolocator/geolocator.dart';
import 'package:flutter_compass/flutter_compass.dart';
import 'package:vector_math/vector_math_64.dart';

/// AR Navigation Service
/// GPS-based navigation with AR waypoints to wildlife cameras
class ARNavigationService {
  static final ARNavigationService _instance = ARNavigationService._internal();
  factory ARNavigationService() => _instance;
  ARNavigationService._internal();

  Position? _currentPosition;
  double? _currentHeading;
  final StreamController<ARWaypoint> _waypointController =
      StreamController<ARWaypoint>.broadcast();
  final List<ARWaypoint> _waypoints = [];

  Stream<ARWaypoint> get waypointStream => _waypointController.stream;
  List<ARWaypoint> get waypoints => _waypoints;
  Position? get currentPosition => _currentPosition;
  double? get currentHeading => _currentHeading;

  /// Initialize navigation service
  Future<bool> initialize() async {
    try {
      // Request location permissions
      final permission = await Geolocator.checkPermission();
      if (permission == LocationPermission.denied) {
        await Geolocator.requestPermission();
      }

      // Start location updates
      Geolocator.getPositionStream(
        locationSettings: LocationSettings(
          accuracy: LocationAccuracy.high,
          distanceFilter: 5, // Update every 5 meters
        ),
      ).listen((position) {
        _currentPosition = position;
        _updateWaypoints();
      });

      // Start compass updates
      FlutterCompass.events?.listen((event) {
        _currentHeading = event.heading;
        _updateWaypoints();
      });

      return true;
    } catch (e) {
      debugPrint('Failed to initialize AR Navigation: $e');
      return false;
    }
  }

  /// Add waypoint for camera location
  void addCameraWaypoint(ARWaypoint waypoint) {
    _waypoints.add(waypoint);
    _updateWaypoints();
  }

  /// Remove waypoint
  void removeWaypoint(String id) {
    _waypoints.removeWhere((w) => w.id == id);
  }

  /// Clear all waypoints
  void clearWaypoints() {
    _waypoints.clear();
  }

  /// Get nearest waypoint
  ARWaypoint? getNearestWaypoint() {
    if (_currentPosition == null || _waypoints.isEmpty) return null;

    ARWaypoint? nearest;
    double minDistance = double.infinity;

    for (final waypoint in _waypoints) {
      final distance = _calculateDistance(
        _currentPosition!.latitude,
        _currentPosition!.longitude,
        waypoint.latitude,
        waypoint.longitude,
      );

      if (distance < minDistance) {
        minDistance = distance;
        nearest = waypoint;
      }
    }

    return nearest;
  }

  /// Get AR direction vector to waypoint
  Vector3 getARDirection(ARWaypoint waypoint) {
    if (_currentPosition == null || _currentHeading == null) {
      return Vector3(0, 0, -1);
    }

    final bearing = _calculateBearing(
      _currentPosition!.latitude,
      _currentPosition!.longitude,
      waypoint.latitude,
      waypoint.longitude,
    );

    // Convert bearing to AR space vector
    final relativeBearing = bearing - _currentHeading!;
    final radians = relativeBearing * (math.pi / 180);

    return Vector3(
      math.sin(radians),
      0,
      -math.cos(radians),
    ).normalized();
  }

  /// Calculate distance between two coordinates (Haversine formula)
  double _calculateDistance(
      double lat1, double lon1, double lat2, double lon2) {
    const R = 6371000; // Earth's radius in meters
    final dLat = (lat2 - lat1) * (math.pi / 180);
    final dLon = (lon2 - lon1) * (math.pi / 180);

    final a = math.sin(dLat / 2) * math.sin(dLat / 2) +
        math.cos(lat1 * (math.pi / 180)) *
            math.cos(lat2 * (math.pi / 180)) *
            math.sin(dLon / 2) *
            math.sin(dLon / 2);

    final c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a));
    return R * c;
  }

  /// Calculate bearing between two coordinates
  double _calculateBearing(double lat1, double lon1, double lat2, double lon2) {
    final dLon = (lon2 - lon1) * (math.pi / 180);
    final y = math.sin(dLon) * math.cos(lat2 * (math.pi / 180));
    final x = math.cos(lat1 * (math.pi / 180)) *
            math.sin(lat2 * (math.pi / 180)) -
        math.sin(lat1 * (math.pi / 180)) *
            math.cos(lat2 * (math.pi / 180)) *
            math.cos(dLon);

    final bearing = math.atan2(y, x) * (180 / math.pi);
    return (bearing + 360) % 360;
  }

  /// Update waypoint distances and directions
  void _updateWaypoints() {
    if (_currentPosition == null) return;

    for (final waypoint in _waypoints) {
      waypoint.updateFromPosition(_currentPosition!, _currentHeading);
      _waypointController.add(waypoint);
    }
  }

  void dispose() {
    _waypointController.close();
    _waypoints.clear();
  }
}

/// AR Waypoint for navigation
class ARWaypoint {
  final String id;
  final String name;
  final double latitude;
  final double longitude;
  final double? altitude;
  final String type;
  final Map<String, dynamic> metadata;

  double? distanceMeters;
  double? bearing;
  Vector3? arDirection;

  ARWaypoint({
    required this.id,
    required this.name,
    required this.latitude,
    required this.longitude,
    this.altitude,
    required this.type,
    this.metadata = const {},
  });

  void updateFromPosition(Position position, double? heading) {
    // Calculate distance
    const R = 6371000;
    final dLat = (latitude - position.latitude) * (math.pi / 180);
    final dLon = (longitude - position.longitude) * (math.pi / 180);

    final a = math.sin(dLat / 2) * math.sin(dLat / 2) +
        math.cos(position.latitude * (math.pi / 180)) *
            math.cos(latitude * (math.pi / 180)) *
            math.sin(dLon / 2) *
            math.sin(dLon / 2);

    final c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a));
    distanceMeters = R * c;

    // Calculate bearing
    final dLonRad = (longitude - position.longitude) * (math.pi / 180);
    final y = math.sin(dLonRad) * math.cos(latitude * (math.pi / 180));
    final x = math.cos(position.latitude * (math.pi / 180)) *
            math.sin(latitude * (math.pi / 180)) -
        math.sin(position.latitude * (math.pi / 180)) *
            math.cos(latitude * (math.pi / 180)) *
            math.cos(dLonRad);

    bearing = (math.atan2(y, x) * (180 / math.pi) + 360) % 360;

    // Calculate AR direction
    if (heading != null) {
      final relativeBearing = bearing! - heading;
      final radians = relativeBearing * (math.pi / 180);
      arDirection = Vector3(
        math.sin(radians),
        0,
        -math.cos(radians),
      ).normalized();
    }
  }

  String get formattedDistance {
    if (distanceMeters == null) return 'Unknown';
    if (distanceMeters! < 1000) {
      return '${distanceMeters!.toStringAsFixed(0)}m';
    } else {
      return '${(distanceMeters! / 1000).toStringAsFixed(1)}km';
    }
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'name': name,
        'latitude': latitude,
        'longitude': longitude,
        'altitude': altitude,
        'type': type,
        'metadata': metadata,
      };
}
