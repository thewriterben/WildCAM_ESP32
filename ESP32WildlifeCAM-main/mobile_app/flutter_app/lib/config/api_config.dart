class ApiConfig {
  // Device API Configuration
  static String baseUrl = 'http://192.168.1.100';
  static String wsUrl = 'ws://192.168.1.100:81';
  
  // API Endpoints
  static const String statusEndpoint = '/api/mobile/status';
  static const String captureEndpoint = '/api/mobile/capture';
  static const String previewEndpoint = '/api/mobile/preview';
  static const String imagesEndpoint = '/api/mobile/images';
  static const String thumbnailEndpoint = '/api/mobile/thumbnail';
  static const String settingsEndpoint = '/api/mobile/settings';
  static const String burstEndpoint = '/api/mobile/burst';
  static const String environmentalEndpoint = '/api/mobile/environmental';
  static const String healthEndpoint = '/api/health';
  
  // Device Configuration
  static String deviceId = 'WILDCAM-001';
  
  // Request Timeouts
  static const Duration connectionTimeout = Duration(seconds: 10);
  static const Duration receiveTimeout = Duration(seconds: 30);
  
  // WebSocket Configuration
  static const Duration reconnectDelay = Duration(seconds: 5);
  static const int maxReconnectAttempts = 5;
  
  // Image Configuration
  static const int thumbnailSize = 200;
  static const int previewSize = 800;
  static const int thumbnailQuality = 60;
  
  // Update configuration from settings
  static void updateBaseUrl(String url) {
    baseUrl = url;
    wsUrl = url.replaceFirst('http', 'ws') + ':81';
  }
  
  static void updateDeviceId(String id) {
    deviceId = id;
  }
  
  // Build full URL
  static String getUrl(String endpoint) {
    return '$baseUrl$endpoint';
  }
  
  // Build WebSocket URL
  static String getWsUrl() {
    return wsUrl;
  }
}
