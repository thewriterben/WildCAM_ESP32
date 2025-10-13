import 'dart:async';
import 'dart:math' as math;
import 'package:flutter/foundation.dart';
import 'package:vector_math/vector_math_64.dart';

/// AR Audio Visualizer
/// Visualizes wildlife sounds and calls in AR space
class ARAudioVisualizer {
  static final ARAudioVisualizer _instance = ARAudioVisualizer._internal();
  factory ARAudioVisualizer() => _instance;
  ARAudioVisualizer._internal();

  final Map<String, AudioWaveform> _activeWaveforms = {};
  final StreamController<AudioVisualizationUpdate> _updateController =
      StreamController<AudioVisualizationUpdate>.broadcast();

  Stream<AudioVisualizationUpdate> get updateStream => _updateController.stream;

  /// Start visualizing audio from a source
  void startVisualization(String sourceId, Vector3 position, AudioData data) {
    final waveform = AudioWaveform(
      sourceId: sourceId,
      position: position,
      frequency: data.frequency,
      amplitude: data.amplitude,
      duration: data.duration,
      waveformType: data.waveformType,
    );

    _activeWaveforms[sourceId] = waveform;
    _updateController.add(AudioVisualizationUpdate(
      action: AudioVisualizationAction.start,
      waveform: waveform,
    ));
  }

  /// Update audio visualization
  void updateVisualization(String sourceId, AudioData data) {
    final waveform = _activeWaveforms[sourceId];
    if (waveform != null) {
      waveform.updateData(data);
      _updateController.add(AudioVisualizationUpdate(
        action: AudioVisualizationAction.update,
        waveform: waveform,
      ));
    }
  }

  /// Stop visualizing audio
  void stopVisualization(String sourceId) {
    final waveform = _activeWaveforms.remove(sourceId);
    if (waveform != null) {
      _updateController.add(AudioVisualizationUpdate(
        action: AudioVisualizationAction.stop,
        waveform: waveform,
      ));
    }
  }

  /// Get all active waveforms
  List<AudioWaveform> getActiveWaveforms() {
    return _activeWaveforms.values.toList();
  }

  /// Generate waveform points for visualization
  List<Vector3> generateWaveformPoints(AudioWaveform waveform, int resolution) {
    final points = <Vector3>[];
    final time = DateTime.now().millisecondsSinceEpoch / 1000.0;

    for (int i = 0; i < resolution; i++) {
      final t = i / resolution;
      final angle = t * 2 * math.pi;
      
      // Generate wave based on type
      double value;
      switch (waveform.waveformType) {
        case WaveformType.sine:
          value = math.sin(angle * waveform.frequency + time);
          break;
        case WaveformType.square:
          value = math.sin(angle * waveform.frequency + time) > 0 ? 1.0 : -1.0;
          break;
        case WaveformType.sawtooth:
          value = 2 * (t * waveform.frequency - (t * waveform.frequency).floor()) - 1;
          break;
        case WaveformType.triangle:
          value = 4 * ((t * waveform.frequency) % 1 - 0.5).abs() - 1;
          break;
      }

      final x = waveform.position.x + math.cos(angle) * 0.5;
      final y = waveform.position.y + value * waveform.amplitude * 0.3;
      final z = waveform.position.z + math.sin(angle) * 0.5;

      points.add(Vector3(x, y, z));
    }

    return points;
  }

  void dispose() {
    _activeWaveforms.clear();
    _updateController.close();
  }
}

/// Audio waveform data
class AudioWaveform {
  final String sourceId;
  final Vector3 position;
  double frequency;
  double amplitude;
  double duration;
  WaveformType waveformType;
  DateTime startTime;

  AudioWaveform({
    required this.sourceId,
    required this.position,
    required this.frequency,
    required this.amplitude,
    required this.duration,
    required this.waveformType,
  }) : startTime = DateTime.now();

  void updateData(AudioData data) {
    frequency = data.frequency;
    amplitude = data.amplitude;
    duration = data.duration;
    waveformType = data.waveformType;
  }

  double get elapsedTime {
    return DateTime.now().difference(startTime).inMilliseconds / 1000.0;
  }

  bool get isExpired => elapsedTime > duration;
}

/// Audio data for visualization
class AudioData {
  final double frequency;
  final double amplitude;
  final double duration;
  final WaveformType waveformType;
  final String? speciesName;
  final String? callType;

  AudioData({
    required this.frequency,
    required this.amplitude,
    this.duration = 2.0,
    this.waveformType = WaveformType.sine,
    this.speciesName,
    this.callType,
  });
}

enum WaveformType {
  sine,
  square,
  sawtooth,
  triangle,
}

/// Audio visualization update
class AudioVisualizationUpdate {
  final AudioVisualizationAction action;
  final AudioWaveform waveform;

  AudioVisualizationUpdate({
    required this.action,
    required this.waveform,
  });
}

enum AudioVisualizationAction {
  start,
  update,
  stop,
}

/// Wildlife call library
class WildlifeCallLibrary {
  static final Map<String, List<AudioData>> _callDatabase = {
    'White-tailed Deer': [
      AudioData(
        frequency: 1.2,
        amplitude: 0.6,
        duration: 1.5,
        waveformType: WaveformType.sine,
        callType: 'Alert snort',
      ),
      AudioData(
        frequency: 0.8,
        amplitude: 0.4,
        duration: 2.0,
        waveformType: WaveformType.sine,
        callType: 'Maternal grunt',
      ),
    ],
    'Black Bear': [
      AudioData(
        frequency: 0.5,
        amplitude: 0.8,
        duration: 2.5,
        waveformType: WaveformType.square,
        callType: 'Growl',
      ),
    ],
    'Gray Wolf': [
      AudioData(
        frequency: 1.5,
        amplitude: 1.0,
        duration: 3.0,
        waveformType: WaveformType.sine,
        callType: 'Howl',
      ),
    ],
    'Red Fox': [
      AudioData(
        frequency: 2.0,
        amplitude: 0.7,
        duration: 1.0,
        waveformType: WaveformType.sawtooth,
        callType: 'Bark',
      ),
    ],
    'Bald Eagle': [
      AudioData(
        frequency: 3.0,
        amplitude: 0.8,
        duration: 1.5,
        waveformType: WaveformType.triangle,
        callType: 'Screech',
      ),
    ],
  };

  static List<AudioData> getCallsForSpecies(String speciesName) {
    return _callDatabase[speciesName] ?? [];
  }

  static AudioData? getPrimaryCall(String speciesName) {
    final calls = getCallsForSpecies(speciesName);
    return calls.isNotEmpty ? calls.first : null;
  }
}
