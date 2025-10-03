class SystemStatus {
  final bool connected;
  final String connectionType;
  final int batteryLevel;
  final bool isCharging;
  final int signalStrength;
  final int freeHeap;
  final int sdCardSpace;
  final int totalDetections;
  final String firmwareVersion;
  final int uptime;
  
  SystemStatus({
    required this.connected,
    required this.connectionType,
    required this.batteryLevel,
    required this.isCharging,
    required this.signalStrength,
    required this.freeHeap,
    required this.sdCardSpace,
    required this.totalDetections,
    required this.firmwareVersion,
    required this.uptime,
  });
  
  factory SystemStatus.fromJson(Map<String, dynamic> json) {
    return SystemStatus(
      connected: json['connected'] ?? false,
      connectionType: json['connectionType'] ?? 'none',
      batteryLevel: json['batteryLevel'] ?? 0,
      isCharging: json['isCharging'] ?? false,
      signalStrength: json['signalStrength'] ?? 0,
      freeHeap: json['freeHeap'] ?? 0,
      sdCardSpace: json['sdCardSpace'] ?? 0,
      totalDetections: json['totalDetections'] ?? 0,
      firmwareVersion: json['firmwareVersion'] ?? 'Unknown',
      uptime: json['uptime'] ?? 0,
    );
  }
  
  Map<String, dynamic> toJson() {
    return {
      'connected': connected,
      'connectionType': connectionType,
      'batteryLevel': batteryLevel,
      'isCharging': isCharging,
      'signalStrength': signalStrength,
      'freeHeap': freeHeap,
      'sdCardSpace': sdCardSpace,
      'totalDetections': totalDetections,
      'firmwareVersion': firmwareVersion,
      'uptime': uptime,
    };
  }
  
  String get batteryLevelString => '$batteryLevel%';
  String get connectionTypeString {
    switch (connectionType.toLowerCase()) {
      case 'wifi':
        return 'WiFi';
      case 'cellular':
        return 'Cellular';
      default:
        return 'Disconnected';
    }
  }
  
  String get uptimeString {
    final hours = uptime ~/ 3600;
    final minutes = (uptime % 3600) ~/ 60;
    if (hours > 24) {
      final days = hours ~/ 24;
      return '$days days';
    }
    return '${hours}h ${minutes}m';
  }
}
