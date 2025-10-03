class WildlifeDetection {
  final String id;
  final String species;
  final double confidence;
  final DateTime timestamp;
  final String imagePath;
  final String? thumbnailPath;
  final Map<String, dynamic>? environmentalData;
  final double? temperature;
  final double? humidity;
  final int? lightLevel;
  
  WildlifeDetection({
    required this.id,
    required this.species,
    required this.confidence,
    required this.timestamp,
    required this.imagePath,
    this.thumbnailPath,
    this.environmentalData,
    this.temperature,
    this.humidity,
    this.lightLevel,
  });
  
  factory WildlifeDetection.fromJson(Map<String, dynamic> json) {
    return WildlifeDetection(
      id: json['id'] ?? '',
      species: json['species'] ?? 'Unknown',
      confidence: (json['confidence'] ?? 0.0).toDouble(),
      timestamp: DateTime.parse(json['timestamp'] ?? DateTime.now().toIso8601String()),
      imagePath: json['imagePath'] ?? '',
      thumbnailPath: json['thumbnailPath'],
      environmentalData: json['environmental'],
      temperature: json['temperature']?.toDouble(),
      humidity: json['humidity']?.toDouble(),
      lightLevel: json['lightLevel']?.toInt(),
    );
  }
  
  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'species': species,
      'confidence': confidence,
      'timestamp': timestamp.toIso8601String(),
      'imagePath': imagePath,
      'thumbnailPath': thumbnailPath,
      'environmental': environmentalData,
      'temperature': temperature,
      'humidity': humidity,
      'lightLevel': lightLevel,
    };
  }
  
  String get confidencePercentage => '${(confidence * 100).toStringAsFixed(1)}%';
  
  String get timeAgo {
    final now = DateTime.now();
    final difference = now.difference(timestamp);
    
    if (difference.inSeconds < 60) {
      return '${difference.inSeconds}s ago';
    } else if (difference.inMinutes < 60) {
      return '${difference.inMinutes}m ago';
    } else if (difference.inHours < 24) {
      return '${difference.inHours}h ago';
    } else {
      return '${difference.inDays}d ago';
    }
  }
}
