import 'package:flutter/material.dart';
import '../screens/ar_main_screen.dart';
import '../services/ar_wildlife_identification_service.dart';
import '../services/ar_field_guide_service.dart';
import '../services/ar_navigation_service.dart';
import '../services/ar_data_visualization_service.dart';
import '../models/ar_detection.dart';

/// AR Overlay Widget
/// Displays AR information overlays based on current mode
class AROverlayWidget extends StatefulWidget {
  final ARMode mode;
  final ARWildlifeIdentificationService identificationService;
  final ARFieldGuideService fieldGuideService;
  final ARNavigationService navigationService;
  final ARDataVisualizationService visualizationService;

  AROverlayWidget({
    required this.mode,
    required this.identificationService,
    required this.fieldGuideService,
    required this.navigationService,
    required this.visualizationService,
  });

  @override
  _AROverlayWidgetState createState() => _AROverlayWidgetState();
}

class _AROverlayWidgetState extends State<AROverlayWidget> {
  ARWildlifeDetection? _currentDetection;

  @override
  void initState() {
    super.initState();
    _setupDetectionListener();
  }

  void _setupDetectionListener() {
    widget.identificationService.detectionStream.listen((detection) {
      setState(() {
        _currentDetection = detection;
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    switch (widget.mode) {
      case ARMode.identification:
        return _buildIdentificationOverlay();
      case ARMode.fieldGuide:
        return _buildFieldGuideOverlay();
      case ARMode.navigation:
        return _buildNavigationOverlay();
      case ARMode.visualization:
        return _buildVisualizationOverlay();
    }
  }

  Widget _buildIdentificationOverlay() {
    if (_currentDetection == null) {
      return Positioned(
        top: 100,
        left: 20,
        right: 20,
        child: Card(
          color: Colors.black.withOpacity(0.7),
          child: Padding(
            padding: EdgeInsets.all(16),
            child: Row(
              children: [
                Icon(Icons.camera_alt, color: Colors.white),
                SizedBox(width: 12),
                Expanded(
                  child: Text(
                    'Point camera at wildlife to identify',
                    style: TextStyle(color: Colors.white, fontSize: 14),
                  ),
                ),
              ],
            ),
          ),
        ),
      );
    }

    return Positioned(
      top: 100,
      left: 20,
      right: 20,
      child: Card(
        color: Colors.black.withOpacity(0.8),
        child: Padding(
          padding: EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                children: [
                  Icon(Icons.check_circle, color: Colors.green, size: 24),
                  SizedBox(width: 8),
                  Expanded(
                    child: Text(
                      _currentDetection!.speciesName,
                      style: TextStyle(
                        color: Colors.white,
                        fontSize: 20,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ),
                ],
              ),
              SizedBox(height: 8),
              _buildConfidenceMeter(_currentDetection!.confidence),
              SizedBox(height: 12),
              Text(
                'Distance: ${_currentDetection!.position.z.abs().toStringAsFixed(1)}m',
                style: TextStyle(color: Colors.white70, fontSize: 14),
              ),
              SizedBox(height: 4),
              Text(
                'Detected: ${_formatTimestamp(_currentDetection!.timestamp)}',
                style: TextStyle(color: Colors.white70, fontSize: 12),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildFieldGuideOverlay() {
    final species = widget.fieldGuideService.allSpecies;
    
    return Positioned(
      bottom: 120,
      left: 0,
      right: 0,
      child: Container(
        height: 150,
        child: ListView.builder(
          scrollDirection: Axis.horizontal,
          padding: EdgeInsets.symmetric(horizontal: 20),
          itemCount: species.length,
          itemBuilder: (context, index) {
            final model = species[index];
            return Card(
              color: Colors.black.withOpacity(0.7),
              margin: EdgeInsets.only(right: 12),
              child: Container(
                width: 120,
                padding: EdgeInsets.all(12),
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Icon(Icons.pets, color: Colors.white, size: 40),
                    SizedBox(height: 8),
                    Text(
                      model.speciesName,
                      style: TextStyle(
                        color: Colors.white,
                        fontSize: 12,
                      ),
                      textAlign: TextAlign.center,
                      maxLines: 2,
                      overflow: TextOverflow.ellipsis,
                    ),
                  ],
                ),
              ),
            );
          },
        ),
      ),
    );
  }

  Widget _buildNavigationOverlay() {
    final nearestWaypoint = widget.navigationService.getNearestWaypoint();
    
    if (nearestWaypoint == null) {
      return Positioned(
        top: 100,
        left: 20,
        right: 20,
        child: Card(
          color: Colors.black.withOpacity(0.7),
          child: Padding(
            padding: EdgeInsets.all(16),
            child: Text(
              'No waypoints set',
              style: TextStyle(color: Colors.white),
              textAlign: TextAlign.center,
            ),
          ),
        ),
      );
    }

    return Stack(
      children: [
        // Direction arrow
        Positioned(
          top: MediaQuery.of(context).size.height / 2 - 50,
          left: MediaQuery.of(context).size.width / 2 - 25,
          child: Icon(
            Icons.navigation,
            color: Colors.blue,
            size: 50,
          ),
        ),
        // Waypoint info
        Positioned(
          top: 100,
          left: 20,
          right: 20,
          child: Card(
            color: Colors.black.withOpacity(0.8),
            child: Padding(
              padding: EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    nearestWaypoint.name,
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  SizedBox(height: 8),
                  Row(
                    children: [
                      Icon(Icons.location_on, color: Colors.blue, size: 20),
                      SizedBox(width: 8),
                      Text(
                        nearestWaypoint.formattedDistance,
                        style: TextStyle(color: Colors.white, fontSize: 16),
                      ),
                    ],
                  ),
                ],
              ),
            ),
          ),
        ),
      ],
    );
  }

  Widget _buildVisualizationOverlay() {
    final networkStatus = widget.visualizationService.getNetworkStatus();
    
    return Positioned(
      top: 100,
      left: 20,
      right: 20,
      child: Card(
        color: Colors.black.withOpacity(0.8),
        child: Padding(
          padding: EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Text(
                'Camera Network Status',
                style: TextStyle(
                  color: Colors.white,
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                ),
              ),
              SizedBox(height: 12),
              _buildStatusRow(
                'Online',
                networkStatus.onlineCameras,
                Colors.green,
              ),
              _buildStatusRow(
                'Offline',
                networkStatus.offlineCameras,
                Colors.red,
              ),
              _buildStatusRow(
                'Low Battery',
                networkStatus.lowBatteryCameras,
                Colors.orange,
              ),
              SizedBox(height: 8),
              LinearProgressIndicator(
                value: networkStatus.averageBattery / 100,
                backgroundColor: Colors.grey[800],
                valueColor: AlwaysStoppedAnimation<Color>(Colors.green),
              ),
              SizedBox(height: 4),
              Text(
                'Average Battery: ${networkStatus.averageBattery.toStringAsFixed(0)}%',
                style: TextStyle(color: Colors.white70, fontSize: 12),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildStatusRow(String label, int count, Color color) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: 4),
      child: Row(
        children: [
          Container(
            width: 12,
            height: 12,
            decoration: BoxDecoration(
              color: color,
              shape: BoxShape.circle,
            ),
          ),
          SizedBox(width: 8),
          Text(
            '$label:',
            style: TextStyle(color: Colors.white70, fontSize: 14),
          ),
          SizedBox(width: 8),
          Text(
            count.toString(),
            style: TextStyle(
              color: Colors.white,
              fontSize: 14,
              fontWeight: FontWeight.bold,
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildConfidenceMeter(double confidence) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(
          'Confidence: ${(confidence * 100).toStringAsFixed(0)}%',
          style: TextStyle(color: Colors.white70, fontSize: 14),
        ),
        SizedBox(height: 4),
        LinearProgressIndicator(
          value: confidence,
          backgroundColor: Colors.grey[800],
          valueColor: AlwaysStoppedAnimation<Color>(
            confidence > 0.8 ? Colors.green : Colors.orange,
          ),
        ),
      ],
    );
  }

  String _formatTimestamp(DateTime timestamp) {
    final now = DateTime.now();
    final diff = now.difference(timestamp);
    
    if (diff.inSeconds < 60) {
      return 'Just now';
    } else if (diff.inMinutes < 60) {
      return '${diff.inMinutes}m ago';
    } else {
      return '${diff.inHours}h ago';
    }
  }
}
