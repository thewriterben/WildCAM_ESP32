import 'dart:async';
import 'package:flutter/foundation.dart';
import 'package:device_info_plus/device_info_plus.dart';

/// Core AR platform service that abstracts ARCore (Android) and ARKit (iOS)
/// Provides unified interface for AR capabilities across platforms
class ARPlatformService {
  static final ARPlatformService _instance = ARPlatformService._internal();
  factory ARPlatformService() => _instance;
  ARPlatformService._internal();

  bool _isInitialized = false;
  bool _isSupported = false;
  ARPlatform _platform = ARPlatform.unsupported;
  
  bool get isInitialized => _isInitialized;
  bool get isSupported => _isSupported;
  ARPlatform get platform => _platform;

  /// Initialize AR platform and check device capabilities
  Future<bool> initialize() async {
    if (_isInitialized) return _isSupported;

    try {
      final deviceInfo = DeviceInfoPlugin();
      
      if (defaultTargetPlatform == TargetPlatform.android) {
        final androidInfo = await deviceInfo.androidInfo;
        // ARCore requires Android 7.0+ (API level 24+)
        _isSupported = androidInfo.version.sdkInt >= 24;
        _platform = _isSupported ? ARPlatform.arcore : ARPlatform.unsupported;
      } else if (defaultTargetPlatform == TargetPlatform.iOS) {
        final iosInfo = await deviceInfo.iosInfo;
        // ARKit requires iOS 11+
        final version = iosInfo.systemVersion.split('.').first;
        _isSupported = int.tryParse(version) != null && int.parse(version) >= 11;
        _platform = _isSupported ? ARPlatform.arkit : ARPlatform.unsupported;
      } else {
        _isSupported = false;
        _platform = ARPlatform.unsupported;
      }

      _isInitialized = true;
      return _isSupported;
    } catch (e) {
      debugPrint('AR Platform initialization error: $e');
      _isInitialized = true;
      _isSupported = false;
      return false;
    }
  }

  /// Check if specific AR feature is available
  bool hasFeature(ARFeature feature) {
    if (!_isSupported) return false;
    
    switch (feature) {
      case ARFeature.planeDetection:
      case ARFeature.hitTesting:
      case ARFeature.worldTracking:
        return true;
      case ARFeature.faceTracking:
        // Face tracking typically requires specific hardware
        return _platform == ARPlatform.arkit;
      case ARFeature.imageTracking:
      case ARFeature.objectDetection:
        return true;
      case ARFeature.environmentalHDR:
        return _platform == ARPlatform.arkit;
      case ARFeature.peopleOcclusion:
        return _platform == ARPlatform.arkit;
      default:
        return false;
    }
  }

  /// Get AR camera configuration
  ARCameraConfig getCameraConfig() {
    return ARCameraConfig(
      enableAutoFocus: true,
      enableDepth: hasFeature(ARFeature.worldTracking),
      enableLighting: true,
      preferredResolution: ARResolution.high,
    );
  }

  /// Reset AR session
  Future<void> reset() async {
    // Platform-specific reset logic would go here
    debugPrint('AR session reset');
  }

  void dispose() {
    _isInitialized = false;
  }
}

enum ARPlatform {
  arcore,
  arkit,
  unsupported,
}

enum ARFeature {
  planeDetection,
  hitTesting,
  worldTracking,
  faceTracking,
  imageTracking,
  objectDetection,
  environmentalHDR,
  peopleOcclusion,
}

class ARCameraConfig {
  final bool enableAutoFocus;
  final bool enableDepth;
  final bool enableLighting;
  final ARResolution preferredResolution;

  ARCameraConfig({
    required this.enableAutoFocus,
    required this.enableDepth,
    required this.enableLighting,
    required this.preferredResolution,
  });
}

enum ARResolution {
  low,
  medium,
  high,
  auto,
}
