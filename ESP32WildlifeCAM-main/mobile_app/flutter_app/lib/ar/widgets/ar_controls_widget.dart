import 'package:flutter/material.dart';
import '../screens/ar_main_screen.dart';

/// AR Controls Widget
/// Bottom control bar for AR functionality
class ARControlsWidget extends StatelessWidget {
  final ARMode currentMode;
  final Function(ARMode) onModeChanged;

  ARControlsWidget({
    required this.currentMode,
    required this.onModeChanged,
  });

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Container(
        padding: EdgeInsets.symmetric(horizontal: 20, vertical: 12),
        decoration: BoxDecoration(
          color: Colors.black.withOpacity(0.7),
          borderRadius: BorderRadius.circular(30),
        ),
        child: Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            _buildControlButton(
              Icons.camera_alt,
              'Capture',
              () => _handleCapture(context),
            ),
            SizedBox(width: 20),
            _buildControlButton(
              Icons.straighten,
              'Measure',
              () => _handleMeasure(context),
            ),
            SizedBox(width: 20),
            _buildControlButton(
              Icons.mic,
              'Voice',
              () => _handleVoice(context),
            ),
            SizedBox(width: 20),
            _buildControlButton(
              Icons.settings,
              'Settings',
              () => _handleSettings(context),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildControlButton(IconData icon, String label, VoidCallback onTap) {
    return InkWell(
      onTap: onTap,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Container(
            padding: EdgeInsets.all(12),
            decoration: BoxDecoration(
              color: Colors.white.withOpacity(0.2),
              shape: BoxShape.circle,
            ),
            child: Icon(icon, color: Colors.white, size: 24),
          ),
          SizedBox(height: 4),
          Text(
            label,
            style: TextStyle(
              color: Colors.white,
              fontSize: 10,
            ),
          ),
        ],
      ),
    );
  }

  void _handleCapture(BuildContext context) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('Capturing AR scene...'),
        duration: Duration(seconds: 2),
      ),
    );
  }

  void _handleMeasure(BuildContext context) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('Measurement tool activated'),
        duration: Duration(seconds: 2),
      ),
    );
  }

  void _handleVoice(BuildContext context) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text('Voice input activated'),
        duration: Duration(seconds: 2),
      ),
    );
  }

  void _handleSettings(BuildContext context) {
    showModalBottomSheet(
      context: context,
      backgroundColor: Colors.transparent,
      builder: (context) => _buildSettingsSheet(context),
    );
  }

  Widget _buildSettingsSheet(BuildContext context) {
    return Container(
      decoration: BoxDecoration(
        color: Colors.black.withOpacity(0.9),
        borderRadius: BorderRadius.vertical(top: Radius.circular(20)),
      ),
      padding: EdgeInsets.all(20),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'AR Settings',
            style: TextStyle(
              color: Colors.white,
              fontSize: 20,
              fontWeight: FontWeight.bold,
            ),
          ),
          SizedBox(height: 20),
          _buildSettingItem(
            'Plane Detection',
            'Detect horizontal and vertical surfaces',
            true,
          ),
          _buildSettingItem(
            'Environmental Lighting',
            'Adjust AR lighting based on environment',
            true,
          ),
          _buildSettingItem(
            'Depth Sensing',
            'Use depth sensors for better occlusion',
            false,
          ),
          _buildSettingItem(
            'Auto-Capture',
            'Automatically capture detected wildlife',
            false,
          ),
          SizedBox(height: 20),
          SizedBox(
            width: double.infinity,
            child: ElevatedButton(
              onPressed: () => Navigator.pop(context),
              style: ElevatedButton.styleFrom(
                backgroundColor: Colors.blue,
                padding: EdgeInsets.symmetric(vertical: 16),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(12),
                ),
              ),
              child: Text('Done'),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildSettingItem(String title, String subtitle, bool initialValue) {
    return Padding(
      padding: EdgeInsets.symmetric(vertical: 8),
      child: Row(
        children: [
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  title,
                  style: TextStyle(
                    color: Colors.white,
                    fontSize: 16,
                  ),
                ),
                SizedBox(height: 4),
                Text(
                  subtitle,
                  style: TextStyle(
                    color: Colors.white70,
                    fontSize: 12,
                  ),
                ),
              ],
            ),
          ),
          Switch(
            value: initialValue,
            onChanged: (value) {},
            activeColor: Colors.blue,
          ),
        ],
      ),
    );
  }
}
