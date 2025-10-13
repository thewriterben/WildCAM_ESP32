import 'dart:math' as math;
import 'package:vector_math/vector_math_64.dart';

/// AR Measurement Tool
/// Provides measurement utilities for AR wildlife observations
class ARMeasurementTool {
  /// Calculate distance between two AR points
  static double calculateDistance(Vector3 point1, Vector3 point2) {
    return (point2 - point1).length;
  }

  /// Calculate angle between two vectors
  static double calculateAngle(Vector3 vector1, Vector3 vector2) {
    final dot = vector1.normalized().dot(vector2.normalized());
    return math.acos(dot.clamp(-1.0, 1.0)) * (180 / math.pi);
  }

  /// Estimate animal size from AR measurements
  static AnimalSizeEstimate estimateAnimalSize(
    Vector3 topPoint,
    Vector3 bottomPoint,
    Vector3 frontPoint,
    Vector3 backPoint,
    double cameraDistance,
  ) {
    final height = calculateDistance(topPoint, bottomPoint);
    final length = calculateDistance(frontPoint, backPoint);
    
    // Apply distance correction factor
    final correctionFactor = cameraDistance / 5.0; // Baseline at 5m
    final correctedHeight = height * correctionFactor;
    final correctedLength = length * correctionFactor;
    
    return AnimalSizeEstimate(
      heightMeters: correctedHeight,
      lengthMeters: correctedLength,
      widthMeters: correctedLength * 0.6, // Estimate width as 60% of length
      confidence: _calculateMeasurementConfidence(cameraDistance),
      measurementMethod: MeasurementMethod.arPoints,
    );
  }

  /// Calculate measurement confidence based on conditions
  static double _calculateMeasurementConfidence(double distance) {
    // Confidence decreases with distance
    if (distance < 3) return 0.95;
    if (distance < 5) return 0.9;
    if (distance < 10) return 0.8;
    if (distance < 15) return 0.7;
    return 0.6;
  }

  /// Estimate distance to animal using camera field of view
  static double estimateDistanceFromPixelSize(
    double pixelHeight,
    double imageHeight,
    double cameraFOV,
    double knownAnimalHeight,
  ) {
    // Calculate apparent size
    final apparentSize = (pixelHeight / imageHeight) * cameraFOV;
    
    // Estimate distance using trigonometry
    final distance = knownAnimalHeight / math.tan(apparentSize * (math.pi / 180));
    
    return distance;
  }

  /// Project 3D point to 2D screen coordinates
  static Vector2 projectToScreen(
    Vector3 worldPoint,
    Matrix4 viewMatrix,
    Matrix4 projectionMatrix,
    double screenWidth,
    double screenHeight,
  ) {
    // Transform world point to clip space
    final clipSpace = projectionMatrix * viewMatrix * Vector4(
      worldPoint.x,
      worldPoint.y,
      worldPoint.z,
      1.0,
    );

    // Perspective divide
    final ndc = Vector3(
      clipSpace.x / clipSpace.w,
      clipSpace.y / clipSpace.w,
      clipSpace.z / clipSpace.w,
    );

    // Convert to screen coordinates
    final screenX = (ndc.x + 1.0) * 0.5 * screenWidth;
    final screenY = (1.0 - ndc.y) * 0.5 * screenHeight;

    return Vector2(screenX, screenY);
  }

  /// Calculate bounding box from AR points
  static BoundingBox3D calculateBoundingBox(List<Vector3> points) {
    if (points.isEmpty) {
      return BoundingBox3D(
        min: Vector3.zero(),
        max: Vector3.zero(),
        center: Vector3.zero(),
        size: Vector3.zero(),
      );
    }

    var minX = points[0].x, minY = points[0].y, minZ = points[0].z;
    var maxX = points[0].x, maxY = points[0].y, maxZ = points[0].z;

    for (final point in points) {
      minX = math.min(minX, point.x);
      minY = math.min(minY, point.y);
      minZ = math.min(minZ, point.z);
      maxX = math.max(maxX, point.x);
      maxY = math.max(maxY, point.y);
      maxZ = math.max(maxZ, point.z);
    }

    final min = Vector3(minX, minY, minZ);
    final max = Vector3(maxX, maxY, maxZ);
    final size = max - min;
    final center = (min + max) / 2;

    return BoundingBox3D(
      min: min,
      max: max,
      center: center,
      size: size,
    );
  }

  /// Compare measured size with known species dimensions
  static SpeciesMatchResult compareWithKnownSpecies(
    AnimalSizeEstimate measurement,
    Map<String, SpeciesDimensions> speciesDatabase,
  ) {
    final matches = <SpeciesMatch>[];

    for (final entry in speciesDatabase.entries) {
      final species = entry.key;
      final dimensions = entry.value;

      final heightMatch = _calculateDimensionMatch(
        measurement.heightMeters,
        dimensions.averageHeight,
        dimensions.heightRange,
      );

      final lengthMatch = _calculateDimensionMatch(
        measurement.lengthMeters,
        dimensions.averageLength,
        dimensions.lengthRange,
      );

      final overallMatch = (heightMatch + lengthMatch) / 2;

      if (overallMatch > 0.5) {
        matches.add(SpeciesMatch(
          speciesName: species,
          matchConfidence: overallMatch * measurement.confidence,
          heightMatch: heightMatch,
          lengthMatch: lengthMatch,
        ));
      }
    }

    // Sort by confidence
    matches.sort((a, b) => b.matchConfidence.compareTo(a.matchConfidence));

    return SpeciesMatchResult(
      matches: matches,
      bestMatch: matches.isNotEmpty ? matches.first : null,
    );
  }

  static double _calculateDimensionMatch(
    double measured,
    double average,
    DimensionRange range,
  ) {
    if (measured < range.min || measured > range.max) {
      // Outside range
      final minDiff = (measured - range.min).abs();
      final maxDiff = (measured - range.max).abs();
      final closestDiff = math.min(minDiff, maxDiff);
      return math.max(0, 1 - (closestDiff / average));
    } else {
      // Within range - calculate how close to average
      final diff = (measured - average).abs();
      return 1 - (diff / (range.max - range.min));
    }
  }
}

/// Animal size estimate
class AnimalSizeEstimate {
  final double heightMeters;
  final double lengthMeters;
  final double widthMeters;
  final double confidence;
  final MeasurementMethod measurementMethod;

  AnimalSizeEstimate({
    required this.heightMeters,
    required this.lengthMeters,
    required this.widthMeters,
    required this.confidence,
    required this.measurementMethod,
  });

  String get formattedHeight => '${heightMeters.toStringAsFixed(2)}m';
  String get formattedLength => '${lengthMeters.toStringAsFixed(2)}m';
  String get formattedWidth => '${widthMeters.toStringAsFixed(2)}m';
}

enum MeasurementMethod {
  arPoints,
  pixelSize,
  comparison,
  manual,
}

/// 3D Bounding Box
class BoundingBox3D {
  final Vector3 min;
  final Vector3 max;
  final Vector3 center;
  final Vector3 size;

  BoundingBox3D({
    required this.min,
    required this.max,
    required this.center,
    required this.size,
  });

  double get volume => size.x * size.y * size.z;
}

/// Species dimensions
class SpeciesDimensions {
  final double averageHeight;
  final double averageLength;
  final DimensionRange heightRange;
  final DimensionRange lengthRange;

  SpeciesDimensions({
    required this.averageHeight,
    required this.averageLength,
    required this.heightRange,
    required this.lengthRange,
  });
}

class DimensionRange {
  final double min;
  final double max;

  DimensionRange(this.min, this.max);
}

/// Species match result
class SpeciesMatch {
  final String speciesName;
  final double matchConfidence;
  final double heightMatch;
  final double lengthMatch;

  SpeciesMatch({
    required this.speciesName,
    required this.matchConfidence,
    required this.heightMatch,
    required this.lengthMatch,
  });
}

class SpeciesMatchResult {
  final List<SpeciesMatch> matches;
  final SpeciesMatch? bestMatch;

  SpeciesMatchResult({
    required this.matches,
    this.bestMatch,
  });
}

/// Known species dimensions database
class SpeciesDimensionsDatabase {
  static final Map<String, SpeciesDimensions> database = {
    'White-tailed Deer': SpeciesDimensions(
      averageHeight: 1.0,
      averageLength: 1.8,
      heightRange: DimensionRange(0.85, 1.1),
      lengthRange: DimensionRange(1.5, 2.1),
    ),
    'Black Bear': SpeciesDimensions(
      averageHeight: 1.0,
      averageLength: 1.8,
      heightRange: DimensionRange(0.9, 1.2),
      lengthRange: DimensionRange(1.5, 2.1),
    ),
    'Gray Wolf': SpeciesDimensions(
      averageHeight: 0.8,
      averageLength: 1.3,
      heightRange: DimensionRange(0.66, 0.86),
      lengthRange: DimensionRange(1.05, 1.6),
    ),
    'Red Fox': SpeciesDimensions(
      averageHeight: 0.4,
      averageLength: 0.7,
      heightRange: DimensionRange(0.35, 0.5),
      lengthRange: DimensionRange(0.6, 0.9),
    ),
  };
}
