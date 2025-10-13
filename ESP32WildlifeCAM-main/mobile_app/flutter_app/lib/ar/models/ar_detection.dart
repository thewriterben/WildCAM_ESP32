import 'package:vector_math/vector_math_64.dart';

/// Represents a wildlife detection in AR space
class ARWildlifeDetection {
  final String id;
  final String speciesName;
  final double confidence;
  final Vector3 position;
  final Vector3 boundingBox;
  final DateTime timestamp;
  final String? imageUrl;
  final Map<String, dynamic> metadata;

  ARWildlifeDetection({
    required this.id,
    required this.speciesName,
    required this.confidence,
    required this.position,
    required this.boundingBox,
    required this.timestamp,
    this.imageUrl,
    this.metadata = const {},
  });

  /// Check if detection is high confidence
  bool get isHighConfidence => confidence >= 0.8;

  /// Check if detection is recent (within last 5 minutes)
  bool get isRecent {
    final diff = DateTime.now().difference(timestamp);
    return diff.inMinutes < 5;
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'speciesName': speciesName,
        'confidence': confidence,
        'position': [position.x, position.y, position.z],
        'boundingBox': [boundingBox.x, boundingBox.y, boundingBox.z],
        'timestamp': timestamp.toIso8601String(),
        'imageUrl': imageUrl,
        'metadata': metadata,
      };

  factory ARWildlifeDetection.fromJson(Map<String, dynamic> json) =>
      ARWildlifeDetection(
        id: json['id'],
        speciesName: json['speciesName'],
        confidence: json['confidence'],
        position: Vector3(
          json['position'][0],
          json['position'][1],
          json['position'][2],
        ),
        boundingBox: Vector3(
          json['boundingBox'][0],
          json['boundingBox'][1],
          json['boundingBox'][2],
        ),
        timestamp: DateTime.parse(json['timestamp']),
        imageUrl: json['imageUrl'],
        metadata: json['metadata'] ?? {},
      );
}

/// AR measurement data for animals
class ARMeasurement {
  final String detectionId;
  final double distanceMeters;
  final double heightMeters;
  final double widthMeters;
  final double lengthMeters;
  final Vector3 anchorPosition;
  final DateTime measuredAt;

  ARMeasurement({
    required this.detectionId,
    required this.distanceMeters,
    required this.heightMeters,
    required this.widthMeters,
    required this.lengthMeters,
    required this.anchorPosition,
    required this.measuredAt,
  });

  String get formattedDistance => '${distanceMeters.toStringAsFixed(1)}m';
  String get formattedSize =>
      '${heightMeters.toStringAsFixed(2)}m × ${widthMeters.toStringAsFixed(2)}m';

  Map<String, dynamic> toJson() => {
        'detectionId': detectionId,
        'distanceMeters': distanceMeters,
        'heightMeters': heightMeters,
        'widthMeters': widthMeters,
        'lengthMeters': lengthMeters,
        'anchorPosition': [anchorPosition.x, anchorPosition.y, anchorPosition.z],
        'measuredAt': measuredAt.toIso8601String(),
      };
}
