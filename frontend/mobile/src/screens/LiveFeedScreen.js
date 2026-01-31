/**
 * WildCAM Mobile - Live Feed Screen
 * Real-time camera streaming and snapshot viewing
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import React, { useState, useEffect, useCallback } from 'react';
import {
  View,
  Text,
  ScrollView,
  RefreshControl,
  StyleSheet,
  TouchableOpacity,
  Image,
  ActivityIndicator,
  Alert,
  FlatList,
} from 'react-native';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService } from '../services';

const LiveFeedScreen = ({ navigation }) => {
  const [refreshing, setRefreshing] = useState(false);
  const [devices, setDevices] = useState([]);
  const [selectedDevice, setSelectedDevice] = useState(null);
  const [latestImage, setLatestImage] = useState(null);
  const [isLoading, setIsLoading] = useState(false);
  const [isCapturing, setIsCapturing] = useState(false);

  useEffect(() => {
    loadDevices();
  }, []);

  useEffect(() => {
    if (selectedDevice) {
      loadLatestImage(selectedDevice.device_id);
    }
  }, [selectedDevice]);

  const loadDevices = async () => {
    try {
      const response = await APIService.getDevices();
      const deviceList = response.devices || [];
      setDevices(deviceList);
      if (deviceList.length > 0 && !selectedDevice) {
        setSelectedDevice(deviceList[0]);
      }
    } catch (error) {
      console.error('Failed to load devices:', error);
      Alert.alert('Error', 'Failed to load camera devices');
    }
  };

  const loadLatestImage = async (deviceId) => {
    setIsLoading(true);
    try {
      const response = await APIService.getDetections({
        device_id: deviceId,
        limit: 1,
      });
      const detections = response.data || [];
      if (detections.length > 0) {
        setLatestImage(detections[0]);
      } else {
        setLatestImage(null);
      }
    } catch (error) {
      console.error('Failed to load latest image:', error);
    } finally {
      setIsLoading(false);
    }
  };

  const onRefresh = useCallback(async () => {
    setRefreshing(true);
    await loadDevices();
    if (selectedDevice) {
      await loadLatestImage(selectedDevice.device_id);
    }
    setRefreshing(false);
  }, [selectedDevice]);

  const triggerCapture = async () => {
    if (!selectedDevice) {
      Alert.alert('No Device', 'Please select a camera first');
      return;
    }

    setIsCapturing(true);
    try {
      // Trigger remote capture
      await APIService.updateDeviceConfig(selectedDevice.device_id, {
        action: 'capture',
        timestamp: new Date().toISOString(),
      });
      
      Alert.alert('Success', 'Capture command sent. Refreshing in 5 seconds...');
      
      // Wait and refresh to get new image
      setTimeout(async () => {
        await loadLatestImage(selectedDevice.device_id);
        setIsCapturing(false);
      }, 5000);
    } catch (error) {
      console.error('Failed to trigger capture:', error);
      Alert.alert('Error', 'Failed to trigger capture');
      setIsCapturing(false);
    }
  };

  const renderDeviceItem = ({ item }) => (
    <TouchableOpacity
      style={[
        styles.deviceChip,
        selectedDevice?.device_id === item.device_id && styles.deviceChipSelected,
      ]}
      onPress={() => setSelectedDevice(item)}
    >
      <View
        style={[
          styles.statusIndicator,
          { backgroundColor: item.status === 'online' ? '#4CAF50' : '#9E9E9E' },
        ]}
      />
      <Text
        style={[
          styles.deviceChipText,
          selectedDevice?.device_id === item.device_id && styles.deviceChipTextSelected,
        ]}
      >
        {item.device_name || item.device_id}
      </Text>
    </TouchableOpacity>
  );

  return (
    <ScrollView
      style={styles.container}
      refreshControl={
        <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
      }
    >
      {/* Device Selector */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Select Camera</Text>
        <FlatList
          horizontal
          data={devices}
          keyExtractor={(item) => item.device_id}
          renderItem={renderDeviceItem}
          showsHorizontalScrollIndicator={false}
          contentContainerStyle={styles.deviceList}
        />
      </View>

      {/* Live Feed / Latest Image */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Camera Feed</Text>
        <View style={styles.feedContainer}>
          {isLoading ? (
            <View style={styles.loadingContainer}>
              <ActivityIndicator size="large" color="#4CAF50" />
              <Text style={styles.loadingText}>Loading...</Text>
            </View>
          ) : latestImage ? (
            <TouchableOpacity
              onPress={() =>
                navigation.navigate('SpeciesDetail', {
                  detectionId: latestImage.id,
                })
              }
            >
              <Image
                source={{ uri: latestImage.image_url }}
                style={styles.feedImage}
                resizeMode="cover"
              />
              <View style={styles.imageOverlay}>
                <Text style={styles.overlayText}>
                  {new Date(latestImage.timestamp).toLocaleString()}
                </Text>
                {latestImage.species && (
                  <View style={styles.speciesBadge}>
                    <Text style={styles.speciesBadgeText}>
                      {latestImage.species} ({Math.round(latestImage.confidence * 100)}%)
                    </Text>
                  </View>
                )}
              </View>
            </TouchableOpacity>
          ) : (
            <View style={styles.noImageContainer}>
              <Icon name="camera-alt" size={64} color="#666" />
              <Text style={styles.noImageText}>No images available</Text>
            </View>
          )}
        </View>
      </View>

      {/* Controls */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Camera Controls</Text>
        <View style={styles.controlsContainer}>
          <TouchableOpacity
            style={[styles.controlButton, isCapturing && styles.controlButtonDisabled]}
            onPress={triggerCapture}
            disabled={isCapturing}
          >
            {isCapturing ? (
              <ActivityIndicator size="small" color="#fff" />
            ) : (
              <Icon name="photo-camera" size={28} color="#fff" />
            )}
            <Text style={styles.controlButtonText}>
              {isCapturing ? 'Capturing...' : 'Capture Now'}
            </Text>
          </TouchableOpacity>

          <TouchableOpacity
            style={styles.controlButton}
            onPress={onRefresh}
          >
            <Icon name="refresh" size={28} color="#fff" />
            <Text style={styles.controlButtonText}>Refresh</Text>
          </TouchableOpacity>

          <TouchableOpacity
            style={styles.controlButton}
            onPress={() =>
              selectedDevice &&
              navigation.navigate('DeviceDetail', { deviceId: selectedDevice.device_id })
            }
          >
            <Icon name="settings" size={28} color="#fff" />
            <Text style={styles.controlButtonText}>Configure</Text>
          </TouchableOpacity>
        </View>
      </View>

      {/* Device Status */}
      {selectedDevice && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Device Status</Text>
          <View style={styles.statusCard}>
            <StatusRow
              icon="power"
              label="Status"
              value={selectedDevice.status || 'Unknown'}
              valueColor={selectedDevice.status === 'online' ? '#4CAF50' : '#9E9E9E'}
            />
            <StatusRow
              icon="battery-full"
              label="Battery"
              value={`${selectedDevice.battery_level || 0}%`}
              valueColor={
                (selectedDevice.battery_level || 0) > 20 ? '#4CAF50' : '#F44336'
              }
            />
            <StatusRow
              icon="signal-wifi-4-bar"
              label="Signal"
              value={`${selectedDevice.signal_strength || 0}%`}
            />
            <StatusRow
              icon="sd-storage"
              label="Storage"
              value={`${selectedDevice.storage_used || 0}% used`}
            />
            <StatusRow
              icon="schedule"
              label="Last Seen"
              value={
                selectedDevice.last_seen
                  ? new Date(selectedDevice.last_seen).toLocaleString()
                  : 'Never'
              }
            />
          </View>
        </View>
      )}

      <View style={styles.bottomPadding} />
    </ScrollView>
  );
};

const StatusRow = ({ icon, label, value, valueColor }) => (
  <View style={styles.statusRow}>
    <Icon name={icon} size={20} color="#4CAF50" style={styles.statusIcon} />
    <Text style={styles.statusLabel}>{label}</Text>
    <Text style={[styles.statusValue, valueColor && { color: valueColor }]}>
      {value}
    </Text>
  </View>
);

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
  },
  section: {
    padding: 16,
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 12,
    textTransform: 'uppercase',
    letterSpacing: 1,
  },
  deviceList: {
    paddingVertical: 4,
  },
  deviceChip: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 20,
    paddingHorizontal: 16,
    paddingVertical: 8,
    marginRight: 8,
    borderWidth: 1,
    borderColor: '#333',
  },
  deviceChipSelected: {
    backgroundColor: '#4CAF50',
    borderColor: '#4CAF50',
  },
  statusIndicator: {
    width: 8,
    height: 8,
    borderRadius: 4,
    marginRight: 8,
  },
  deviceChipText: {
    color: '#888',
    fontSize: 14,
  },
  deviceChipTextSelected: {
    color: '#fff',
    fontWeight: 'bold',
  },
  feedContainer: {
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    overflow: 'hidden',
    minHeight: 250,
  },
  loadingContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    minHeight: 250,
  },
  loadingText: {
    color: '#888',
    marginTop: 12,
  },
  feedImage: {
    width: '100%',
    height: 250,
  },
  imageOverlay: {
    position: 'absolute',
    bottom: 0,
    left: 0,
    right: 0,
    padding: 12,
    backgroundColor: 'rgba(0, 0, 0, 0.6)',
  },
  overlayText: {
    color: '#fff',
    fontSize: 12,
  },
  speciesBadge: {
    marginTop: 4,
    backgroundColor: '#4CAF50',
    paddingHorizontal: 8,
    paddingVertical: 4,
    borderRadius: 4,
    alignSelf: 'flex-start',
  },
  speciesBadgeText: {
    color: '#fff',
    fontSize: 12,
    fontWeight: 'bold',
  },
  noImageContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    minHeight: 250,
  },
  noImageText: {
    color: '#666',
    marginTop: 12,
  },
  controlsContainer: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  controlButton: {
    alignItems: 'center',
    backgroundColor: '#4CAF50',
    borderRadius: 12,
    padding: 16,
    minWidth: 100,
  },
  controlButtonDisabled: {
    backgroundColor: '#666',
  },
  controlButtonText: {
    color: '#fff',
    marginTop: 8,
    fontSize: 12,
    fontWeight: '500',
  },
  statusCard: {
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 16,
  },
  statusRow: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 8,
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  statusIcon: {
    marginRight: 12,
  },
  statusLabel: {
    flex: 1,
    color: '#888',
    fontSize: 14,
  },
  statusValue: {
    color: '#fff',
    fontSize: 14,
    fontWeight: '500',
  },
  bottomPadding: {
    height: 32,
  },
});

export default LiveFeedScreen;
