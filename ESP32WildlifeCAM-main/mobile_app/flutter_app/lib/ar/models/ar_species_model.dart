import 'package:vector_math/vector_math_64.dart';

/// Represents a 3D species model for AR visualization
class ARSpeciesModel {
  final String id;
  final String speciesName;
  final String scientificName;
  final String modelPath;
  final String? thumbnailPath;
  final Vector3 scale;
  final Vector3 position;
  final Vector3 rotation;
  final List<String> animations;
  final ARModelDetail detail;
  final ConservationStatus conservationStatus;

  ARSpeciesModel({
    required this.id,
    required this.speciesName,
    required this.scientificName,
    required this.modelPath,
    this.thumbnailPath,
    Vector3? scale,
    Vector3? position,
    Vector3? rotation,
    this.animations = const [],
    this.detail = ARModelDetail.medium,
    this.conservationStatus = ConservationStatus.unknown,
  })  : scale = scale ?? Vector3(1.0, 1.0, 1.0),
        position = position ?? Vector3.zero(),
        rotation = rotation ?? Vector3.zero();

  /// Get model size in meters (approximate)
  double get estimatedSize {
    switch (detail) {
      case ARModelDetail.low:
        return 0.5;
      case ARModelDetail.medium:
        return 1.0;
      case ARModelDetail.high:
        return 1.5;
    }
  }

  /// Check if model has animation
  bool get hasAnimation => animations.isNotEmpty;

  /// Get conservation status color
  String get conservationColor {
    switch (conservationStatus) {
      case ConservationStatus.extinct:
        return '#000000';
      case ConservationStatus.criticallyEndangered:
        return '#D81E05';
      case ConservationStatus.endangered:
        return '#FC7F3F';
      case ConservationStatus.vulnerable:
        return '#F9E814';
      case ConservationStatus.nearThreatened:
        return '#CCE226';
      case ConservationStatus.leastConcern:
        return '#60C659';
      case ConservationStatus.unknown:
        return '#999999';
    }
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'speciesName': speciesName,
        'scientificName': scientificName,
        'modelPath': modelPath,
        'thumbnailPath': thumbnailPath,
        'scale': [scale.x, scale.y, scale.z],
        'position': [position.x, position.y, position.z],
        'rotation': [rotation.x, rotation.y, rotation.z],
        'animations': animations,
        'detail': detail.toString(),
        'conservationStatus': conservationStatus.toString(),
      };

  factory ARSpeciesModel.fromJson(Map<String, dynamic> json) => ARSpeciesModel(
        id: json['id'],
        speciesName: json['speciesName'],
        scientificName: json['scientificName'],
        modelPath: json['modelPath'],
        thumbnailPath: json['thumbnailPath'],
        scale: Vector3(json['scale'][0], json['scale'][1], json['scale'][2]),
        position: Vector3(json['position'][0], json['position'][1], json['position'][2]),
        rotation: Vector3(json['rotation'][0], json['rotation'][1], json['rotation'][2]),
        animations: List<String>.from(json['animations'] ?? []),
        detail: ARModelDetail.values.firstWhere(
          (e) => e.toString() == json['detail'],
          orElse: () => ARModelDetail.medium,
        ),
        conservationStatus: ConservationStatus.values.firstWhere(
          (e) => e.toString() == json['conservationStatus'],
          orElse: () => ConservationStatus.unknown,
        ),
      );
}

enum ARModelDetail {
  low,
  medium,
  high,
}

enum ConservationStatus {
  extinct,
  criticallyEndangered,
  endangered,
  vulnerable,
  nearThreatened,
  leastConcern,
  unknown,
}

/// Species information overlay for AR
class ARSpeciesOverlay {
  final String speciesName;
  final String scientificName;
  final double confidence;
  final String behavior;
  final Map<String, dynamic> environmentalData;
  final List<String> characteristics;
  final String habitat;
  final String diet;

  ARSpeciesOverlay({
    required this.speciesName,
    required this.scientificName,
    required this.confidence,
    required this.behavior,
    this.environmentalData = const {},
    this.characteristics = const [],
    this.habitat = '',
    this.diet = '',
  });

  Map<String, dynamic> toJson() => {
        'speciesName': speciesName,
        'scientificName': scientificName,
        'confidence': confidence,
        'behavior': behavior,
        'environmentalData': environmentalData,
        'characteristics': characteristics,
        'habitat': habitat,
        'diet': diet,
      };
}
