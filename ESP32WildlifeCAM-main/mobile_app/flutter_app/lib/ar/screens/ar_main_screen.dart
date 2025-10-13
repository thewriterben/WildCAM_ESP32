import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import '../core/ar_platform_service.dart';
import '../services/ar_wildlife_identification_service.dart';
import '../services/ar_field_guide_service.dart';
import '../services/ar_navigation_service.dart';
import '../services/ar_data_visualization_service.dart';
import '../widgets/ar_overlay_widget.dart';
import '../widgets/ar_controls_widget.dart';

/// Main AR Screen
/// Entry point for all AR functionality
class ARMainScreen extends StatefulWidget {
  @override
  _ARMainScreenState createState() => _ARMainScreenState();
}

class _ARMainScreenState extends State<ARMainScreen> {
  final ARPlatformService _platformService = ARPlatformService();
  final ARWildlifeIdentificationService _identificationService =
      ARWildlifeIdentificationService();
  final ARFieldGuideService _fieldGuideService = ARFieldGuideService();
  final ARNavigationService _navigationService = ARNavigationService();
  final ARDataVisualizationService _visualizationService =
      ARDataVisualizationService();

  bool _isInitialized = false;
  bool _isSupported = false;
  String _errorMessage = '';
  ARMode _currentMode = ARMode.identification;

  @override
  void initState() {
    super.initState();
    _initializeAR();
  }

  Future<void> _initializeAR() async {
    try {
      // Initialize AR platform
      final platformSupported = await _platformService.initialize();
      if (!platformSupported) {
        setState(() {
          _isSupported = false;
          _errorMessage = 'AR is not supported on this device';
        });
        return;
      }

      // Initialize services
      await Future.wait([
        _identificationService.initialize(),
        _fieldGuideService.initialize(),
        _navigationService.initialize(),
        _visualizationService.initialize(),
      ]);

      setState(() {
        _isInitialized = true;
        _isSupported = true;
      });
    } catch (e) {
      setState(() {
        _isSupported = false;
        _errorMessage = 'Failed to initialize AR: $e';
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    if (!_isSupported) {
      return _buildUnsupportedScreen();
    }

    if (!_isInitialized) {
      return _buildLoadingScreen();
    }

    return Scaffold(
      body: AnnotatedRegion<SystemUiOverlayStyle>(
        value: SystemUiOverlayStyle.light,
        child: Stack(
          children: [
            // AR View (placeholder - would use actual AR plugin)
            _buildARView(),

            // AR Overlays
            AROverlayWidget(
              mode: _currentMode,
              identificationService: _identificationService,
              fieldGuideService: _fieldGuideService,
              navigationService: _navigationService,
              visualizationService: _visualizationService,
            ),

            // AR Controls
            Positioned(
              bottom: 20,
              left: 0,
              right: 0,
              child: ARControlsWidget(
                currentMode: _currentMode,
                onModeChanged: (mode) {
                  setState(() {
                    _currentMode = mode;
                  });
                },
              ),
            ),

            // Mode Selector
            Positioned(
              top: MediaQuery.of(context).padding.top + 10,
              left: 10,
              right: 10,
              child: _buildModeSelector(),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildARView() {
    // Placeholder for AR view - would integrate ARCore/ARKit plugin
    return Container(
      color: Colors.black,
      child: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(
              Icons.camera_alt,
              size: 100,
              color: Colors.white.withOpacity(0.3),
            ),
            SizedBox(height: 20),
            Text(
              'AR Camera View',
              style: TextStyle(
                color: Colors.white.withOpacity(0.5),
                fontSize: 18,
              ),
            ),
            SizedBox(height: 10),
            Text(
              'Platform: ${_platformService.platform.toString().split('.').last}',
              style: TextStyle(
                color: Colors.white.withOpacity(0.3),
                fontSize: 14,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildModeSelector() {
    return Card(
      color: Colors.black.withOpacity(0.7),
      child: Padding(
        padding: EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            _buildModeButton(
              ARMode.identification,
              Icons.camera_enhance,
              'Identify',
            ),
            _buildModeButton(
              ARMode.fieldGuide,
              Icons.menu_book,
              'Guide',
            ),
            _buildModeButton(
              ARMode.navigation,
              Icons.explore,
              'Navigate',
            ),
            _buildModeButton(
              ARMode.visualization,
              Icons.analytics,
              'Data',
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildModeButton(ARMode mode, IconData icon, String label) {
    final isActive = _currentMode == mode;
    return InkWell(
      onTap: () {
        setState(() {
          _currentMode = mode;
        });
      },
      child: Container(
        padding: EdgeInsets.all(8),
        decoration: BoxDecoration(
          color: isActive ? Colors.blue : Colors.transparent,
          borderRadius: BorderRadius.circular(8),
        ),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            Icon(
              icon,
              color: Colors.white,
              size: 24,
            ),
            SizedBox(height: 4),
            Text(
              label,
              style: TextStyle(
                color: Colors.white,
                fontSize: 12,
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildLoadingScreen() {
    return Scaffold(
      backgroundColor: Colors.black,
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            CircularProgressIndicator(color: Colors.white),
            SizedBox(height: 20),
            Text(
              'Initializing AR...',
              style: TextStyle(color: Colors.white, fontSize: 18),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildUnsupportedScreen() {
    return Scaffold(
      appBar: AppBar(
        title: Text('AR Not Supported'),
      ),
      body: Center(
        child: Padding(
          padding: EdgeInsets.all(20),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Icon(
                Icons.error_outline,
                size: 80,
                color: Colors.red,
              ),
              SizedBox(height: 20),
              Text(
                _errorMessage,
                textAlign: TextAlign.center,
                style: TextStyle(fontSize: 16),
              ),
              SizedBox(height: 20),
              Text(
                'AR requires:\n• iOS 11+ (ARKit)\n• Android 7.0+ (ARCore)',
                textAlign: TextAlign.center,
                style: TextStyle(fontSize: 14, color: Colors.grey),
              ),
            ],
          ),
        ),
      ),
    );
  }

  @override
  void dispose() {
    _platformService.dispose();
    _identificationService.dispose();
    _fieldGuideService.dispose();
    _navigationService.dispose();
    _visualizationService.dispose();
    super.dispose();
  }
}

enum ARMode {
  identification,
  fieldGuide,
  navigation,
  visualization,
}
