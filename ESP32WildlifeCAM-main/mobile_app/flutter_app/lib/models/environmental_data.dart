class EnvironmentalData {
  final double temperature;
  final double humidity;
  final double pressure;
  final int lightLevel;
  final double airQualityIndex;
  final double wildlifeComfortIndex;
  final DateTime timestamp;
  
  EnvironmentalData({
    required this.temperature,
    required this.humidity,
    required this.pressure,
    required this.lightLevel,
    required this.airQualityIndex,
    required this.wildlifeComfortIndex,
    required this.timestamp,
  });
  
  factory EnvironmentalData.fromJson(Map<String, dynamic> json) {
    return EnvironmentalData(
      temperature: (json['temperature'] ?? 0.0).toDouble(),
      humidity: (json['humidity'] ?? 0.0).toDouble(),
      pressure: (json['pressure'] ?? 0.0).toDouble(),
      lightLevel: json['lightLevel'] ?? 0,
      airQualityIndex: (json['airQualityIndex'] ?? 0.0).toDouble(),
      wildlifeComfortIndex: (json['wildlifeComfortIndex'] ?? 0.0).toDouble(),
      timestamp: DateTime.parse(json['timestamp'] ?? DateTime.now().toIso8601String()),
    );
  }
  
  Map<String, dynamic> toJson() {
    return {
      'temperature': temperature,
      'humidity': humidity,
      'pressure': pressure,
      'lightLevel': lightLevel,
      'airQualityIndex': airQualityIndex,
      'wildlifeComfortIndex': wildlifeComfortIndex,
      'timestamp': timestamp.toIso8601String(),
    };
  }
  
  String get temperatureString => '${temperature.toStringAsFixed(1)}°C';
  String get humidityString => '${humidity.toStringAsFixed(1)}%';
  String get pressureString => '${pressure.toStringAsFixed(1)} hPa';
  String get lightLevelString => '$lightLevel lux';
  String get airQualityString {
    if (airQualityIndex < 50) return 'Good';
    if (airQualityIndex < 100) return 'Moderate';
    if (airQualityIndex < 150) return 'Unhealthy for Sensitive Groups';
    if (airQualityIndex < 200) return 'Unhealthy';
    if (airQualityIndex < 300) return 'Very Unhealthy';
    return 'Hazardous';
  }
}
