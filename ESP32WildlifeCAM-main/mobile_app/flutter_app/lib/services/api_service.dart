import 'dart:convert';
import 'package:dio/dio.dart';
import '../config/api_config.dart';
import '../models/system_status.dart';
import '../models/wildlife_detection.dart';
import '../models/environmental_data.dart';

class ApiService {
  late Dio _dio;
  
  ApiService() {
    _dio = Dio(BaseOptions(
      baseUrl: ApiConfig.baseUrl,
      connectTimeout: ApiConfig.connectionTimeout,
      receiveTimeout: ApiConfig.receiveTimeout,
      headers: {
        'Content-Type': 'application/json',
        'X-Device-ID': ApiConfig.deviceId,
      },
    ));
    
    // Add interceptors for logging
    _dio.interceptors.add(LogInterceptor(
      requestBody: true,
      responseBody: true,
    ));
  }
  
  // Get system status
  Future<SystemStatus> getSystemStatus() async {
    try {
      final response = await _dio.get(ApiConfig.statusEndpoint);
      return SystemStatus.fromJson(response.data);
    } catch (e) {
      throw Exception('Failed to get system status: $e');
    }
  }
  
  // Capture image
  Future<Map<String, dynamic>> captureImage({
    int? quality,
    String? size,
    bool? enableFlash,
  }) async {
    try {
      final response = await _dio.post(
        ApiConfig.captureEndpoint,
        data: {
          'quality': quality ?? 85,
          'size': size ?? 'UXGA',
          'flash': enableFlash ?? false,
        },
      );
      return response.data;
    } catch (e) {
      throw Exception('Failed to capture image: $e');
    }
  }
  
  // Start burst mode
  Future<Map<String, dynamic>> startBurstMode({
    required int count,
    required int interval,
  }) async {
    try {
      final response = await _dio.post(
        ApiConfig.burstEndpoint,
        data: {
          'action': 'start',
          'count': count,
          'interval': interval,
        },
      );
      return response.data;
    } catch (e) {
      throw Exception('Failed to start burst mode: $e');
    }
  }
  
  // Get wildlife detections
  Future<List<WildlifeDetection>> getDetections({
    int? limit,
    int? offset,
  }) async {
    try {
      final response = await _dio.get(
        ApiConfig.imagesEndpoint,
        queryParameters: {
          'limit': limit ?? 20,
          'offset': offset ?? 0,
        },
      );
      
      final List<dynamic> data = response.data['detections'] ?? [];
      return data.map((json) => WildlifeDetection.fromJson(json)).toList();
    } catch (e) {
      throw Exception('Failed to get detections: $e');
    }
  }
  
  // Get environmental data
  Future<EnvironmentalData> getEnvironmentalData() async {
    try {
      final response = await _dio.get(ApiConfig.environmentalEndpoint);
      return EnvironmentalData.fromJson(response.data);
    } catch (e) {
      throw Exception('Failed to get environmental data: $e');
    }
  }
  
  // Get camera settings
  Future<Map<String, dynamic>> getSettings() async {
    try {
      final response = await _dio.get(ApiConfig.settingsEndpoint);
      return response.data;
    } catch (e) {
      throw Exception('Failed to get settings: $e');
    }
  }
  
  // Update camera settings
  Future<void> updateSettings(Map<String, dynamic> settings) async {
    try {
      await _dio.post(
        ApiConfig.settingsEndpoint,
        data: settings,
      );
    } catch (e) {
      throw Exception('Failed to update settings: $e');
    }
  }
  
  // Get image thumbnail URL
  String getThumbnailUrl(String imagePath) {
    return '${ApiConfig.baseUrl}${ApiConfig.thumbnailEndpoint}?path=$imagePath&size=${ApiConfig.thumbnailSize}';
  }
  
  // Get full image URL
  String getImageUrl(String imagePath) {
    return '${ApiConfig.baseUrl}/images/$imagePath';
  }
  
  // Test connection
  Future<bool> testConnection() async {
    try {
      final response = await _dio.get(
        ApiConfig.healthEndpoint,
        options: Options(
          receiveTimeout: Duration(seconds: 5),
        ),
      );
      return response.statusCode == 200;
    } catch (e) {
      return false;
    }
  }
}
