import 'dart:async';
import 'package:flutter/foundation.dart';
import '../models/ar_species_model.dart';

/// AR Field Guide Service
/// Provides interactive 3D field guide with species models and information
class ARFieldGuideService {
  static final ARFieldGuideService _instance = ARFieldGuideService._internal();
  factory ARFieldGuideService() => _instance;
  ARFieldGuideService._internal();

  final Map<String, ARSpeciesModel> _speciesModels = {};
  final Map<String, List<String>> _behaviorAnimations = {};
  bool _isInitialized = false;

  bool get isInitialized => _isInitialized;
  List<ARSpeciesModel> get allSpecies => _speciesModels.values.toList();

  /// Initialize field guide with species data
  Future<bool> initialize() async {
    if (_isInitialized) return true;

    try {
      await _loadSpeciesDatabase();
      await _loadBehaviorAnimations();
      _isInitialized = true;
      return true;
    } catch (e) {
      debugPrint('Failed to initialize AR Field Guide: $e');
      return false;
    }
  }

  /// Get species model by name
  ARSpeciesModel? getSpeciesModel(String speciesName) {
    return _speciesModels[speciesName];
  }

  /// Search species by name
  List<ARSpeciesModel> searchSpecies(String query) {
    if (query.isEmpty) return allSpecies;

    return allSpecies.where((species) {
      return species.speciesName.toLowerCase().contains(query.toLowerCase()) ||
          species.scientificName.toLowerCase().contains(query.toLowerCase());
    }).toList();
  }

  /// Get species by conservation status
  List<ARSpeciesModel> getSpeciesByConservationStatus(
      ConservationStatus status) {
    return allSpecies
        .where((species) => species.conservationStatus == status)
        .toList();
  }

  /// Get behavior animations for species
  List<String> getBehaviorAnimations(String speciesName) {
    return _behaviorAnimations[speciesName] ?? [];
  }

  /// Get comparative species for side-by-side analysis
  List<ARSpeciesModel> getComparativeSpecies(String speciesName) {
    final baseSpecies = getSpeciesModel(speciesName);
    if (baseSpecies == null) return [];

    // Return similar species for comparison
    return allSpecies
        .where((species) =>
            species.speciesName != speciesName &&
            _areSimilarSpecies(baseSpecies, species))
        .take(3)
        .toList();
  }

  /// Check if two species are similar (for comparison)
  bool _areSimilarSpecies(ARSpeciesModel species1, ARSpeciesModel species2) {
    // Simplified similarity check - in real implementation, use taxonomy
    return species1.conservationStatus == species2.conservationStatus;
  }

  /// Load species database
  Future<void> _loadSpeciesDatabase() async {
    // Simulate loading from local database or API
    await Future.delayed(Duration(milliseconds: 300));

    // Sample species data
    _speciesModels['White-tailed Deer'] = ARSpeciesModel(
      id: 'deer_001',
      speciesName: 'White-tailed Deer',
      scientificName: 'Odocoileus virginianus',
      modelPath: 'assets/models/deer.glb',
      thumbnailPath: 'assets/thumbnails/deer.png',
      animations: ['walk', 'graze', 'alert', 'run'],
      detail: ARModelDetail.high,
      conservationStatus: ConservationStatus.leastConcern,
    );

    _speciesModels['Black Bear'] = ARSpeciesModel(
      id: 'bear_001',
      speciesName: 'Black Bear',
      scientificName: 'Ursus americanus',
      modelPath: 'assets/models/bear.glb',
      thumbnailPath: 'assets/thumbnails/bear.png',
      animations: ['walk', 'forage', 'stand', 'scratch'],
      detail: ARModelDetail.high,
      conservationStatus: ConservationStatus.leastConcern,
    );

    _speciesModels['Gray Wolf'] = ARSpeciesModel(
      id: 'wolf_001',
      speciesName: 'Gray Wolf',
      scientificName: 'Canis lupus',
      modelPath: 'assets/models/wolf.glb',
      thumbnailPath: 'assets/thumbnails/wolf.png',
      animations: ['walk', 'howl', 'hunt', 'rest'],
      detail: ARModelDetail.high,
      conservationStatus: ConservationStatus.leastConcern,
    );

    _speciesModels['Red Fox'] = ARSpeciesModel(
      id: 'fox_001',
      speciesName: 'Red Fox',
      scientificName: 'Vulpes vulpes',
      modelPath: 'assets/models/fox.glb',
      thumbnailPath: 'assets/thumbnails/fox.png',
      animations: ['walk', 'pounce', 'dig', 'rest'],
      detail: ARModelDetail.medium,
      conservationStatus: ConservationStatus.leastConcern,
    );

    _speciesModels['Bald Eagle'] = ARSpeciesModel(
      id: 'eagle_001',
      speciesName: 'Bald Eagle',
      scientificName: 'Haliaeetus leucocephalus',
      modelPath: 'assets/models/eagle.glb',
      thumbnailPath: 'assets/thumbnails/eagle.png',
      animations: ['perch', 'fly', 'dive', 'feed'],
      detail: ARModelDetail.high,
      conservationStatus: ConservationStatus.leastConcern,
    );

    debugPrint('Loaded ${_speciesModels.length} species models');
  }

  /// Load behavior animations
  Future<void> _loadBehaviorAnimations() async {
    _behaviorAnimations['White-tailed Deer'] = [
      'grazing',
      'walking',
      'running',
      'alert',
      'drinking'
    ];
    _behaviorAnimations['Black Bear'] = [
      'foraging',
      'walking',
      'climbing',
      'fishing',
      'hibernating'
    ];
    _behaviorAnimations['Gray Wolf'] = [
      'hunting',
      'howling',
      'running',
      'resting',
      'playing'
    ];
    _behaviorAnimations['Red Fox'] = [
      'stalking',
      'pouncing',
      'digging',
      'resting',
      'playing'
    ];
    _behaviorAnimations['Bald Eagle'] = [
      'soaring',
      'diving',
      'perching',
      'feeding',
      'nesting'
    ];
  }

  void dispose() {
    _speciesModels.clear();
    _behaviorAnimations.clear();
    _isInitialized = false;
  }
}
