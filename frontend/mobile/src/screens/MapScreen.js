/**
 * WildCAM Mobile - Map Screen
 * Camera locations and wildlife detection map
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import React, { useState, useEffect, useRef } from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  ActivityIndicator,
  Dimensions,
  Modal,
  ScrollView,
  Image,
} from 'react-native';
import MapView, { Marker, Callout, Circle, Heatmap } from 'react-native-maps';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService } from '../services';

const { width, height } = Dimensions.get('window');

const MapScreen = ({ navigation }) => {
  const mapRef = useRef(null);
  const [loading, setLoading] = useState(true);
  const [devices, setDevices] = useState([]);
  const [detections, setDetections] = useState([]);
  const [selectedDevice, setSelectedDevice] = useState(null);
  const [mapType, setMapType] = useState('standard');
  const [showHeatmap, setShowHeatmap] = useState(false);
  const [showLegend, setShowLegend] = useState(false);
  const [region, setRegion] = useState({
    latitude: 43.6532,
    longitude: -79.3832,
    latitudeDelta: 0.5,
    longitudeDelta: 0.5,
  });

  useEffect(() => {
    loadMapData();
  }, []);

  const loadMapData = async () => {
    setLoading(true);
    try {
      const [devicesResponse, detectionsResponse] = await Promise.all([
        APIService.getDevices(),
        APIService.getDetections({ limit: 100, has_location: true }),
      ]);

      const deviceList = devicesResponse.devices || [];
      const detectionList = detectionsResponse.data || [];

      setDevices(deviceList);
      setDetections(detectionList);

      // Center map on devices if available
      if (deviceList.length > 0) {
        const validDevices = deviceList.filter(
          (d) => d.latitude && d.longitude
        );
        if (validDevices.length > 0) {
          const centerLat =
            validDevices.reduce((sum, d) => sum + d.latitude, 0) /
            validDevices.length;
          const centerLng =
            validDevices.reduce((sum, d) => sum + d.longitude, 0) /
            validDevices.length;
          setRegion({
            latitude: centerLat,
            longitude: centerLng,
            latitudeDelta: 0.5,
            longitudeDelta: 0.5,
          });
        }
      }
    } catch (error) {
      console.error('Failed to load map data:', error);
    } finally {
      setLoading(false);
    }
  };

  const getDeviceMarkerColor = (device) => {
    switch (device.status) {
      case 'online':
        return '#4CAF50';
      case 'offline':
        return '#9E9E9E';
      case 'low_battery':
        return '#FF9800';
      case 'error':
        return '#F44336';
      default:
        return '#9E9E9E';
    }
  };

  const getHeatmapPoints = () => {
    return detections
      .filter((d) => d.latitude && d.longitude)
      .map((d) => ({
        latitude: d.latitude,
        longitude: d.longitude,
        weight: d.confidence || 0.5,
      }));
  };

  const centerOnDevice = (device) => {
    if (device.latitude && device.longitude) {
      mapRef.current?.animateToRegion({
        latitude: device.latitude,
        longitude: device.longitude,
        latitudeDelta: 0.05,
        longitudeDelta: 0.05,
      });
    }
  };

  const toggleMapType = () => {
    const types = ['standard', 'satellite', 'hybrid', 'terrain'];
    const currentIndex = types.indexOf(mapType);
    const nextIndex = (currentIndex + 1) % types.length;
    setMapType(types[nextIndex]);
  };

  if (loading) {
    return (
      <View style={styles.loadingContainer}>
        <ActivityIndicator size="large" color="#4CAF50" />
        <Text style={styles.loadingText}>Loading map data...</Text>
      </View>
    );
  }

  return (
    <View style={styles.container}>
      <MapView
        ref={mapRef}
        style={styles.map}
        region={region}
        onRegionChangeComplete={setRegion}
        mapType={mapType}
        showsUserLocation
        showsCompass
        showsScale
      >
        {/* Device Markers */}
        {devices
          .filter((d) => d.latitude && d.longitude)
          .map((device) => (
            <Marker
              key={device.device_id}
              coordinate={{
                latitude: device.latitude,
                longitude: device.longitude,
              }}
              pinColor={getDeviceMarkerColor(device)}
              onPress={() => setSelectedDevice(device)}
            >
              <View style={styles.customMarker}>
                <Icon
                  name="videocam"
                  size={24}
                  color={getDeviceMarkerColor(device)}
                />
              </View>
              <Callout
                onPress={() =>
                  navigation.navigate('DeviceDetail', {
                    deviceId: device.device_id,
                  })
                }
              >
                <View style={styles.callout}>
                  <Text style={styles.calloutTitle}>
                    {device.device_name || device.device_id}
                  </Text>
                  <Text style={styles.calloutText}>
                    Status: {device.status}
                  </Text>
                  <Text style={styles.calloutText}>
                    Battery: {device.battery_level || 0}%
                  </Text>
                  <Text style={styles.calloutAction}>Tap for details</Text>
                </View>
              </Callout>
            </Marker>
          ))}

        {/* Detection Markers (when heatmap is off) */}
        {!showHeatmap &&
          detections
            .filter((d) => d.latitude && d.longitude)
            .slice(0, 50)
            .map((detection, index) => (
              <Marker
                key={`detection-${detection.id || index}`}
                coordinate={{
                  latitude: detection.latitude,
                  longitude: detection.longitude,
                }}
                opacity={0.7}
              >
                <View style={styles.detectionMarker}>
                  <Icon name="pets" size={16} color="#FF9800" />
                </View>
              </Marker>
            ))}

        {/* Heatmap */}
        {showHeatmap && getHeatmapPoints().length > 0 && (
          <Heatmap
            points={getHeatmapPoints()}
            radius={50}
            opacity={0.7}
            gradient={{
              colors: ['#00FF00', '#FFFF00', '#FF0000'],
              startPoints: [0.2, 0.5, 0.8],
              colorMapSize: 256,
            }}
          />
        )}

        {/* Detection radius circles around cameras */}
        {devices
          .filter((d) => d.latitude && d.longitude && d.status === 'online')
          .map((device) => (
            <Circle
              key={`circle-${device.device_id}`}
              center={{
                latitude: device.latitude,
                longitude: device.longitude,
              }}
              radius={device.detection_radius || 50}
              fillColor="rgba(76, 175, 80, 0.1)"
              strokeColor="rgba(76, 175, 80, 0.3)"
              strokeWidth={1}
            />
          ))}
      </MapView>

      {/* Map Controls */}
      <View style={styles.controls}>
        <TouchableOpacity style={styles.controlButton} onPress={toggleMapType}>
          <Icon name="layers" size={24} color="#fff" />
        </TouchableOpacity>

        <TouchableOpacity
          style={[
            styles.controlButton,
            showHeatmap && styles.controlButtonActive,
          ]}
          onPress={() => setShowHeatmap(!showHeatmap)}
        >
          <Icon name="whatshot" size={24} color="#fff" />
        </TouchableOpacity>

        <TouchableOpacity
          style={styles.controlButton}
          onPress={() => setShowLegend(!showLegend)}
        >
          <Icon name="info" size={24} color="#fff" />
        </TouchableOpacity>

        <TouchableOpacity style={styles.controlButton} onPress={loadMapData}>
          <Icon name="refresh" size={24} color="#fff" />
        </TouchableOpacity>
      </View>

      {/* Device Quick Select */}
      <ScrollView
        horizontal
        style={styles.deviceScroll}
        showsHorizontalScrollIndicator={false}
      >
        {devices.map((device) => (
          <TouchableOpacity
            key={device.device_id}
            style={[
              styles.deviceChip,
              selectedDevice?.device_id === device.device_id &&
                styles.deviceChipSelected,
            ]}
            onPress={() => {
              setSelectedDevice(device);
              centerOnDevice(device);
            }}
          >
            <View
              style={[
                styles.statusDot,
                { backgroundColor: getDeviceMarkerColor(device) },
              ]}
            />
            <Text style={styles.deviceChipText}>
              {device.device_name || device.device_id}
            </Text>
          </TouchableOpacity>
        ))}
      </ScrollView>

      {/* Legend Modal */}
      <Modal
        visible={showLegend}
        transparent
        animationType="fade"
        onRequestClose={() => setShowLegend(false)}
      >
        <TouchableOpacity
          style={styles.legendBackdrop}
          onPress={() => setShowLegend(false)}
          activeOpacity={1}
        >
          <View style={styles.legendContent}>
            <Text style={styles.legendTitle}>Map Legend</Text>

            <View style={styles.legendSection}>
              <Text style={styles.legendSectionTitle}>Camera Status</Text>
              <LegendItem color="#4CAF50" label="Online" />
              <LegendItem color="#9E9E9E" label="Offline" />
              <LegendItem color="#FF9800" label="Low Battery" />
              <LegendItem color="#F44336" label="Error" />
            </View>

            <View style={styles.legendSection}>
              <Text style={styles.legendSectionTitle}>Detections</Text>
              <View style={styles.legendItem}>
                <Icon name="pets" size={16} color="#FF9800" />
                <Text style={styles.legendLabel}>Wildlife Detection</Text>
              </View>
              <View style={styles.legendItem}>
                <Icon name="whatshot" size={16} color="#FF5722" />
                <Text style={styles.legendLabel}>Activity Heatmap</Text>
              </View>
            </View>

            <TouchableOpacity
              style={styles.legendCloseButton}
              onPress={() => setShowLegend(false)}
            >
              <Text style={styles.legendCloseText}>Close</Text>
            </TouchableOpacity>
          </View>
        </TouchableOpacity>
      </Modal>

      {/* Stats Bar */}
      <View style={styles.statsBar}>
        <View style={styles.statItem}>
          <Text style={styles.statValue}>{devices.length}</Text>
          <Text style={styles.statLabel}>Cameras</Text>
        </View>
        <View style={styles.statItem}>
          <Text style={styles.statValue}>
            {devices.filter((d) => d.status === 'online').length}
          </Text>
          <Text style={styles.statLabel}>Online</Text>
        </View>
        <View style={styles.statItem}>
          <Text style={styles.statValue}>{detections.length}</Text>
          <Text style={styles.statLabel}>Detections</Text>
        </View>
      </View>
    </View>
  );
};

const LegendItem = ({ color, label }) => (
  <View style={styles.legendItem}>
    <View style={[styles.legendDot, { backgroundColor: color }]} />
    <Text style={styles.legendLabel}>{label}</Text>
  </View>
);

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
  },
  loadingContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#121212',
  },
  loadingText: {
    color: '#888',
    marginTop: 12,
  },
  map: {
    flex: 1,
  },
  customMarker: {
    backgroundColor: '#1e1e1e',
    borderRadius: 20,
    padding: 8,
    borderWidth: 2,
    borderColor: '#4CAF50',
  },
  detectionMarker: {
    backgroundColor: 'rgba(255, 152, 0, 0.3)',
    borderRadius: 12,
    padding: 4,
  },
  callout: {
    padding: 8,
    minWidth: 150,
  },
  calloutTitle: {
    fontWeight: 'bold',
    fontSize: 14,
    marginBottom: 4,
  },
  calloutText: {
    fontSize: 12,
    color: '#666',
  },
  calloutAction: {
    fontSize: 11,
    color: '#4CAF50',
    marginTop: 8,
    fontWeight: '500',
  },
  controls: {
    position: 'absolute',
    right: 16,
    top: 16,
    gap: 8,
  },
  controlButton: {
    backgroundColor: 'rgba(30, 30, 30, 0.9)',
    borderRadius: 8,
    padding: 12,
    marginBottom: 8,
  },
  controlButtonActive: {
    backgroundColor: '#4CAF50',
  },
  deviceScroll: {
    position: 'absolute',
    bottom: 80,
    left: 0,
    right: 0,
    paddingHorizontal: 16,
    paddingVertical: 8,
  },
  deviceChip: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: 'rgba(30, 30, 30, 0.9)',
    borderRadius: 20,
    paddingHorizontal: 16,
    paddingVertical: 8,
    marginRight: 8,
  },
  deviceChipSelected: {
    backgroundColor: '#4CAF50',
  },
  statusDot: {
    width: 8,
    height: 8,
    borderRadius: 4,
    marginRight: 8,
  },
  deviceChipText: {
    color: '#fff',
    fontSize: 14,
  },
  statsBar: {
    position: 'absolute',
    bottom: 0,
    left: 0,
    right: 0,
    backgroundColor: 'rgba(30, 30, 30, 0.95)',
    flexDirection: 'row',
    justifyContent: 'space-around',
    padding: 16,
  },
  statItem: {
    alignItems: 'center',
  },
  statValue: {
    color: '#4CAF50',
    fontSize: 20,
    fontWeight: 'bold',
  },
  statLabel: {
    color: '#888',
    fontSize: 12,
    marginTop: 4,
  },
  legendBackdrop: {
    flex: 1,
    backgroundColor: 'rgba(0, 0, 0, 0.7)',
    justifyContent: 'center',
    alignItems: 'center',
  },
  legendContent: {
    backgroundColor: '#1e1e1e',
    borderRadius: 16,
    padding: 20,
    width: width - 64,
  },
  legendTitle: {
    color: '#fff',
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 16,
    textAlign: 'center',
  },
  legendSection: {
    marginBottom: 16,
  },
  legendSectionTitle: {
    color: '#4CAF50',
    fontSize: 14,
    fontWeight: 'bold',
    marginBottom: 8,
  },
  legendItem: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 4,
  },
  legendDot: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginRight: 12,
  },
  legendLabel: {
    color: '#fff',
    fontSize: 14,
    marginLeft: 8,
  },
  legendCloseButton: {
    backgroundColor: '#4CAF50',
    borderRadius: 8,
    padding: 12,
    alignItems: 'center',
    marginTop: 8,
  },
  legendCloseText: {
    color: '#fff',
    fontWeight: 'bold',
  },
});

export default MapScreen;
