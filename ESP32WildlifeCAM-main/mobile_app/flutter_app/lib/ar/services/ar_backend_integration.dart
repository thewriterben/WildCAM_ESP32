import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:http/http.dart' as http;
import '../models/ar_detection.dart';
import '../models/ar_species_model.dart';

/// AR Backend Integration Service
/// Connects AR features with WildCAM ESP32 backend
class ARBackendIntegration {
  static final ARBackendIntegration _instance = ARBackendIntegration._internal();
  factory ARBackendIntegration() => _instance;
  ARBackendIntegration._internal();

  String _baseUrl = 'http://192.168.1.100'; // Default ESP32 IP
  String? _authToken;
  final http.Client _httpClient = http.Client();

  /// Initialize backend connection
  Future<bool> initialize(String baseUrl, {String? authToken}) async {
    _baseUrl = baseUrl;
    _authToken = authToken;

    try {
      final response = await _httpClient.get(
        Uri.parse('$_baseUrl/api/status'),
        headers: _getHeaders(),
      ).timeout(Duration(seconds: 5));

      return response.statusCode == 200;
    } catch (e) {
      debugPrint('Failed to connect to backend: $e');
      return false;
    }
  }

  /// Submit AR wildlife detection to backend
  Future<bool> submitARDetection(ARWildlifeDetection detection) async {
    try {
      final response = await _httpClient.post(
        Uri.parse('$_baseUrl/api/ar/detections'),
        headers: _getHeaders(),
        body: jsonEncode({
          'detection': detection.toJson(),
          'timestamp': DateTime.now().toIso8601String(),
        }),
      );

      return response.statusCode == 200 || response.statusCode == 201;
    } catch (e) {
      debugPrint('Failed to submit AR detection: $e');
      return false;
    }
  }

  /// Submit AR measurement data
  Future<bool> submitARMeasurement(ARMeasurement measurement) async {
    try {
      final response = await _httpClient.post(
        Uri.parse('$_baseUrl/api/ar/measurements'),
        headers: _getHeaders(),
        body: jsonEncode(measurement.toJson()),
      );

      return response.statusCode == 200 || response.statusCode == 201;
    } catch (e) {
      debugPrint('Failed to submit AR measurement: $e');
      return false;
    }
  }

  /// Get camera locations for AR navigation
  Future<List<CameraLocation>> getCameraLocations() async {
    try {
      final response = await _httpClient.get(
        Uri.parse('$_baseUrl/api/cameras/locations'),
        headers: _getHeaders(),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body) as List;
        return data.map((json) => CameraLocation.fromJson(json)).toList();
      }

      return [];
    } catch (e) {
      debugPrint('Failed to get camera locations: $e');
      return [];
    }
  }

  /// Get wildlife activity data for heat maps
  Future<List<ActivityDataPoint>> getWildlifeActivity({
    DateTime? startTime,
    DateTime? endTime,
    String? species,
  }) async {
    try {
      final queryParams = <String, String>{};
      if (startTime != null) {
        queryParams['start'] = startTime.toIso8601String();
      }
      if (endTime != null) {
        queryParams['end'] = endTime.toIso8601String();
      }
      if (species != null) {
        queryParams['species'] = species;
      }

      final uri = Uri.parse('$_baseUrl/api/activity')
          .replace(queryParameters: queryParams);

      final response = await _httpClient.get(uri, headers: _getHeaders());

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body) as List;
        return data.map((json) => ActivityDataPoint.fromJson(json)).toList();
      }

      return [];
    } catch (e) {
      debugPrint('Failed to get wildlife activity: $e');
      return [];
    }
  }

  /// Get environmental data
  Future<EnvironmentalData?> getEnvironmentalData() async {
    try {
      final response = await _httpClient.get(
        Uri.parse('$_baseUrl/api/environmental'),
        headers: _getHeaders(),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        return EnvironmentalData.fromJson(data);
      }

      return null;
    } catch (e) {
      debugPrint('Failed to get environmental data: $e');
      return null;
    }
  }

  /// Upload AR session recording
  Future<bool> uploadARSession(ARSessionData session) async {
    try {
      final response = await _httpClient.post(
        Uri.parse('$_baseUrl/api/ar/sessions'),
        headers: _getHeaders(),
        body: jsonEncode(session.toJson()),
      );

      return response.statusCode == 200 || response.statusCode == 201;
    } catch (e) {
      debugPrint('Failed to upload AR session: $e');
      return false;
    }
  }

  /// Get species information from backend
  Future<ARSpeciesOverlay?> getSpeciesInfo(String speciesName) async {
    try {
      final response = await _httpClient.get(
        Uri.parse('$_baseUrl/api/species/${Uri.encodeComponent(speciesName)}'),
        headers: _getHeaders(),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        return ARSpeciesOverlay(
          speciesName: data['name'],
          scientificName: data['scientific_name'],
          confidence: 1.0,
          behavior: data['behavior'] ?? '',
          environmentalData: data['environmental_requirements'] ?? {},
          characteristics: List<String>.from(data['characteristics'] ?? []),
          habitat: data['habitat'] ?? '',
          diet: data['diet'] ?? '',
        );
      }

      return null;
    } catch (e) {
      debugPrint('Failed to get species info: $e');
      return null;
    }
  }

  /// Submit voice annotation
  Future<bool> submitVoiceAnnotation(VoiceAnnotation annotation) async {
    try {
      final response = await _httpClient.post(
        Uri.parse('$_baseUrl/api/ar/annotations/voice'),
        headers: _getHeaders(),
        body: jsonEncode(annotation.toJson()),
      );

      return response.statusCode == 200 || response.statusCode == 201;
    } catch (e) {
      debugPrint('Failed to submit voice annotation: $e');
      return false;
    }
  }

  Map<String, String> _getHeaders() {
    final headers = {
      'Content-Type': 'application/json',
      'Accept': 'application/json',
    };

    if (_authToken != null) {
      headers['Authorization'] = 'Bearer $_authToken';
    }

    return headers;
  }

  void dispose() {
    _httpClient.close();
  }
}

/// Camera location data
class CameraLocation {
  final String id;
  final String name;
  final double latitude;
  final double longitude;
  final double? altitude;
  final String status;
  final double batteryLevel;

  CameraLocation({
    required this.id,
    required this.name,
    required this.latitude,
    required this.longitude,
    this.altitude,
    required this.status,
    required this.batteryLevel,
  });

  factory CameraLocation.fromJson(Map<String, dynamic> json) => CameraLocation(
        id: json['id'],
        name: json['name'],
        latitude: json['latitude'],
        longitude: json['longitude'],
        altitude: json['altitude'],
        status: json['status'],
        batteryLevel: json['battery_level'],
      );

  Map<String, dynamic> toJson() => {
        'id': id,
        'name': name,
        'latitude': latitude,
        'longitude': longitude,
        'altitude': altitude,
        'status': status,
        'battery_level': batteryLevel,
      };
}

/// Activity data point for heat maps
class ActivityDataPoint {
  final double latitude;
  final double longitude;
  final DateTime timestamp;
  final String species;
  final double confidence;

  ActivityDataPoint({
    required this.latitude,
    required this.longitude,
    required this.timestamp,
    required this.species,
    required this.confidence,
  });

  factory ActivityDataPoint.fromJson(Map<String, dynamic> json) =>
      ActivityDataPoint(
        latitude: json['latitude'],
        longitude: json['longitude'],
        timestamp: DateTime.parse(json['timestamp']),
        species: json['species'],
        confidence: json['confidence'],
      );

  Map<String, dynamic> toJson() => {
        'latitude': latitude,
        'longitude': longitude,
        'timestamp': timestamp.toIso8601String(),
        'species': species,
        'confidence': confidence,
      };
}

/// Environmental data
class EnvironmentalData {
  final double temperature;
  final double humidity;
  final double windSpeed;
  final double pressure;
  final String weatherCondition;

  EnvironmentalData({
    required this.temperature,
    required this.humidity,
    required this.windSpeed,
    required this.pressure,
    required this.weatherCondition,
  });

  factory EnvironmentalData.fromJson(Map<String, dynamic> json) =>
      EnvironmentalData(
        temperature: json['temperature'],
        humidity: json['humidity'],
        windSpeed: json['wind_speed'],
        pressure: json['pressure'],
        weatherCondition: json['weather_condition'],
      );

  Map<String, dynamic> toJson() => {
        'temperature': temperature,
        'humidity': humidity,
        'wind_speed': windSpeed,
        'pressure': pressure,
        'weather_condition': weatherCondition,
      };
}

/// AR session data
class ARSessionData {
  final String sessionId;
  final DateTime startTime;
  final DateTime endTime;
  final int detectionsCount;
  final int measurementsCount;
  final List<String> modesUsed;
  final Map<String, dynamic> metadata;

  ARSessionData({
    required this.sessionId,
    required this.startTime,
    required this.endTime,
    required this.detectionsCount,
    required this.measurementsCount,
    required this.modesUsed,
    this.metadata = const {},
  });

  Map<String, dynamic> toJson() => {
        'session_id': sessionId,
        'start_time': startTime.toIso8601String(),
        'end_time': endTime.toIso8601String(),
        'detections_count': detectionsCount,
        'measurements_count': measurementsCount,
        'modes_used': modesUsed,
        'metadata': metadata,
      };
}

/// Voice annotation
class VoiceAnnotation {
  final String id;
  final DateTime timestamp;
  final String transcription;
  final String? detectionId;
  final Map<String, dynamic> metadata;

  VoiceAnnotation({
    required this.id,
    required this.timestamp,
    required this.transcription,
    this.detectionId,
    this.metadata = const {},
  });

  Map<String, dynamic> toJson() => {
        'id': id,
        'timestamp': timestamp.toIso8601String(),
        'transcription': transcription,
        'detection_id': detectionId,
        'metadata': metadata,
      };
}
