/**
 * WildCAM Mobile - Device Detail Screen
 * Individual camera device management and configuration
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
  Alert,
  ActivityIndicator,
} from 'react-native';
import Slider from '@react-native-community/slider';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService, ConfigurationService } from '../services';

const DeviceDetailScreen = ({ route, navigation }) => {
  const { deviceId } = route.params || {};
  
  const [refreshing, setRefreshing] = useState(false);
  const [loading, setLoading] = useState(true);
  const [device, setDevice] = useState(null);
  const [recentDetections, setRecentDetections] = useState([]);
  const [isUpdating, setIsUpdating] = useState(false);
  
  // Configuration state
  const [captureInterval, setCaptureInterval] = useState(300);
  const [motionSensitivity, setMotionSensitivity] = useState(50);
  const [enableNightMode, setEnableNightMode] = useState(true);
  const [enableCloudUpload, setEnableCloudUpload] = useState(true);

  useEffect(() => {
    if (deviceId) {
      loadDeviceData();
    }
  }, [deviceId]);

  const loadDeviceData = async () => {
    setLoading(true);
    try {
      const [deviceResponse, detectionsResponse] = await Promise.all([
        APIService.getDevice(deviceId),
        APIService.getDetections({ device_id: deviceId, limit: 5 }),
      ]);

      setDevice(deviceResponse);
      setRecentDetections(detectionsResponse.data || []);

      // Load device configuration
      if (deviceResponse.config) {
        setCaptureInterval(deviceResponse.config.capture_interval || 300);
        setMotionSensitivity(deviceResponse.config.motion_sensitivity || 50);
        setEnableNightMode(deviceResponse.config.night_mode !== false);
        setEnableCloudUpload(deviceResponse.config.cloud_upload !== false);
      }
    } catch (error) {
      console.error('Failed to load device data:', error);
      Alert.alert('Error', 'Failed to load device information');
    } finally {
      setLoading(false);
    }
  };

  const onRefresh = useCallback(async () => {
    setRefreshing(true);
    await loadDeviceData();
    setRefreshing(false);
  }, [deviceId]);

  const updateConfiguration = async () => {
    setIsUpdating(true);
    try {
      await APIService.updateDeviceConfig(deviceId, {
        capture_interval: captureInterval,
        motion_sensitivity: motionSensitivity,
        night_mode: enableNightMode,
        cloud_upload: enableCloudUpload,
      });
      Alert.alert('Success', 'Configuration updated successfully');
    } catch (error) {
      console.error('Failed to update configuration:', error);
      Alert.alert('Error', 'Failed to update configuration');
    } finally {
      setIsUpdating(false);
    }
  };

  const triggerAction = async (action) => {
    try {
      await APIService.updateDeviceConfig(deviceId, {
        action,
        timestamp: new Date().toISOString(),
      });
      Alert.alert('Success', `${action} command sent to device`);
    } catch (error) {
      console.error(`Failed to trigger ${action}:`, error);
      Alert.alert('Error', `Failed to trigger ${action}`);
    }
  };

  const getStatusIcon = (status) => {
    switch (status) {
      case 'online':
        return { name: 'check-circle', color: '#4CAF50' };
      case 'offline':
        return { name: 'cancel', color: '#9E9E9E' };
      case 'low_battery':
        return { name: 'battery-alert', color: '#FF9800' };
      case 'error':
        return { name: 'error', color: '#F44336' };
      default:
        return { name: 'help', color: '#9E9E9E' };
    }
  };

  if (loading) {
    return (
      <View style={styles.loadingContainer}>
        <ActivityIndicator size="large" color="#4CAF50" />
        <Text style={styles.loadingText}>Loading device...</Text>
      </View>
    );
  }

  if (!device) {
    return (
      <View style={styles.errorContainer}>
        <Icon name="error-outline" size={64} color="#F44336" />
        <Text style={styles.errorText}>Device not found</Text>
        <TouchableOpacity
          style={styles.retryButton}
          onPress={() => navigation.goBack()}
        >
          <Text style={styles.retryButtonText}>Go Back</Text>
        </TouchableOpacity>
      </View>
    );
  }

  const statusIcon = getStatusIcon(device.status);

  return (
    <ScrollView
      style={styles.container}
      refreshControl={
        <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
      }
    >
      {/* Device Header */}
      <View style={styles.header}>
        <View style={styles.headerInfo}>
          <Text style={styles.deviceName}>
            {device.device_name || device.device_id}
          </Text>
          <View style={styles.statusBadge}>
            <Icon name={statusIcon.name} size={16} color={statusIcon.color} />
            <Text style={[styles.statusText, { color: statusIcon.color }]}>
              {device.status || 'Unknown'}
            </Text>
          </View>
        </View>
        {device.last_image_url && (
          <Image
            source={{ uri: device.last_image_url }}
            style={styles.previewImage}
          />
        )}
      </View>

      {/* Device Stats */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Device Status</Text>
        <View style={styles.statsGrid}>
          <StatCard
            icon="battery-full"
            label="Battery"
            value={`${device.battery_level || 0}%`}
            color={(device.battery_level || 0) > 20 ? '#4CAF50' : '#F44336'}
          />
          <StatCard
            icon="signal-wifi-4-bar"
            label="Signal"
            value={`${device.signal_strength || 0}%`}
            color="#2196F3"
          />
          <StatCard
            icon="sd-storage"
            label="Storage"
            value={`${device.storage_used || 0}%`}
            color={(device.storage_used || 0) < 80 ? '#4CAF50' : '#FF9800'}
          />
          <StatCard
            icon="thermostat"
            label="Temp"
            value={`${device.temperature || '--'}°C`}
            color="#9C27B0"
          />
        </View>
      </View>

      {/* Quick Actions */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Quick Actions</Text>
        <View style={styles.actionsRow}>
          <ActionButton
            icon="photo-camera"
            label="Capture"
            onPress={() => triggerAction('capture')}
          />
          <ActionButton
            icon="refresh"
            label="Restart"
            onPress={() => triggerAction('restart')}
          />
          <ActionButton
            icon="sync"
            label="Sync"
            onPress={() => triggerAction('sync')}
          />
          <ActionButton
            icon="power-settings-new"
            label="Sleep"
            onPress={() => triggerAction('sleep')}
          />
        </View>
      </View>

      {/* Configuration */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Configuration</Text>
        
        <View style={styles.configItem}>
          <View style={styles.configHeader}>
            <Icon name="timer" size={20} color="#4CAF50" />
            <Text style={styles.configLabel}>Capture Interval</Text>
            <Text style={styles.configValue}>{captureInterval}s</Text>
          </View>
          <Slider
            style={styles.slider}
            minimumValue={60}
            maximumValue={1800}
            step={60}
            value={captureInterval}
            onValueChange={setCaptureInterval}
            minimumTrackTintColor="#4CAF50"
            thumbTintColor="#4CAF50"
          />
        </View>

        <View style={styles.configItem}>
          <View style={styles.configHeader}>
            <Icon name="sensors" size={20} color="#4CAF50" />
            <Text style={styles.configLabel}>Motion Sensitivity</Text>
            <Text style={styles.configValue}>{motionSensitivity}%</Text>
          </View>
          <Slider
            style={styles.slider}
            minimumValue={0}
            maximumValue={100}
            step={5}
            value={motionSensitivity}
            onValueChange={setMotionSensitivity}
            minimumTrackTintColor="#4CAF50"
            thumbTintColor="#4CAF50"
          />
        </View>

        <TouchableOpacity
          style={styles.toggleItem}
          onPress={() => setEnableNightMode(!enableNightMode)}
        >
          <Icon name="nightlight" size={20} color="#4CAF50" />
          <Text style={styles.toggleLabel}>Night Mode</Text>
          <View
            style={[
              styles.toggleSwitch,
              enableNightMode && styles.toggleSwitchActive,
            ]}
          >
            <View
              style={[
                styles.toggleKnob,
                enableNightMode && styles.toggleKnobActive,
              ]}
            />
          </View>
        </TouchableOpacity>

        <TouchableOpacity
          style={styles.toggleItem}
          onPress={() => setEnableCloudUpload(!enableCloudUpload)}
        >
          <Icon name="cloud-upload" size={20} color="#4CAF50" />
          <Text style={styles.toggleLabel}>Cloud Upload</Text>
          <View
            style={[
              styles.toggleSwitch,
              enableCloudUpload && styles.toggleSwitchActive,
            ]}
          >
            <View
              style={[
                styles.toggleKnob,
                enableCloudUpload && styles.toggleKnobActive,
              ]}
            />
          </View>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.updateButton, isUpdating && styles.updateButtonDisabled]}
          onPress={updateConfiguration}
          disabled={isUpdating}
        >
          {isUpdating ? (
            <ActivityIndicator size="small" color="#fff" />
          ) : (
            <>
              <Icon name="save" size={20} color="#fff" />
              <Text style={styles.updateButtonText}>Save Configuration</Text>
            </>
          )}
        </TouchableOpacity>
      </View>

      {/* Recent Detections */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Recent Detections</Text>
        {recentDetections.length > 0 ? (
          recentDetections.map((detection) => (
            <TouchableOpacity
              key={detection.id}
              style={styles.detectionItem}
              onPress={() =>
                navigation.navigate('SpeciesDetail', { detectionId: detection.id })
              }
            >
              {detection.image_url && (
                <Image
                  source={{ uri: detection.image_url }}
                  style={styles.detectionThumb}
                />
              )}
              <View style={styles.detectionInfo}>
                <Text style={styles.detectionSpecies}>
                  {detection.species || 'Unknown'}
                </Text>
                <Text style={styles.detectionMeta}>
                  {Math.round((detection.confidence || 0) * 100)}% •{' '}
                  {new Date(detection.timestamp).toLocaleString()}
                </Text>
              </View>
              <Icon name="chevron-right" size={24} color="#666" />
            </TouchableOpacity>
          ))
        ) : (
          <Text style={styles.noDetectionsText}>No recent detections</Text>
        )}
      </View>

      {/* Device Info */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Device Information</Text>
        <InfoRow label="Device ID" value={device.device_id} />
        <InfoRow label="Firmware" value={device.firmware_version || 'Unknown'} />
        <InfoRow label="Hardware" value={device.hardware_type || 'ESP32-CAM'} />
        <InfoRow
          label="Last Seen"
          value={
            device.last_seen
              ? new Date(device.last_seen).toLocaleString()
              : 'Never'
          }
        />
        <InfoRow
          label="Location"
          value={
            device.latitude && device.longitude
              ? `${device.latitude.toFixed(4)}, ${device.longitude.toFixed(4)}`
              : 'Not set'
          }
        />
      </View>

      <View style={styles.bottomPadding} />
    </ScrollView>
  );
};

const StatCard = ({ icon, label, value, color }) => (
  <View style={styles.statCard}>
    <Icon name={icon} size={24} color={color} />
    <Text style={[styles.statValue, { color }]}>{value}</Text>
    <Text style={styles.statLabel}>{label}</Text>
  </View>
);

const ActionButton = ({ icon, label, onPress }) => (
  <TouchableOpacity style={styles.actionButton} onPress={onPress}>
    <Icon name={icon} size={24} color="#4CAF50" />
    <Text style={styles.actionLabel}>{label}</Text>
  </TouchableOpacity>
);

const InfoRow = ({ label, value }) => (
  <View style={styles.infoRow}>
    <Text style={styles.infoLabel}>{label}</Text>
    <Text style={styles.infoValue}>{value}</Text>
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
  errorContainer: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#121212',
    padding: 32,
  },
  errorText: {
    color: '#fff',
    fontSize: 18,
    marginTop: 16,
  },
  retryButton: {
    marginTop: 24,
    backgroundColor: '#4CAF50',
    paddingHorizontal: 24,
    paddingVertical: 12,
    borderRadius: 8,
  },
  retryButtonText: {
    color: '#fff',
    fontWeight: 'bold',
  },
  header: {
    padding: 16,
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'flex-start',
  },
  headerInfo: {
    flex: 1,
  },
  deviceName: {
    color: '#fff',
    fontSize: 24,
    fontWeight: 'bold',
  },
  statusBadge: {
    flexDirection: 'row',
    alignItems: 'center',
    marginTop: 8,
  },
  statusText: {
    marginLeft: 8,
    fontSize: 14,
    fontWeight: '500',
    textTransform: 'capitalize',
  },
  previewImage: {
    width: 100,
    height: 75,
    borderRadius: 8,
  },
  section: {
    padding: 16,
    borderTopWidth: 1,
    borderTopColor: '#333',
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 16,
    textTransform: 'uppercase',
    letterSpacing: 1,
  },
  statsGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    justifyContent: 'space-between',
  },
  statCard: {
    width: '48%',
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 16,
    alignItems: 'center',
    marginBottom: 8,
  },
  statValue: {
    fontSize: 20,
    fontWeight: 'bold',
    marginTop: 8,
  },
  statLabel: {
    color: '#888',
    fontSize: 12,
    marginTop: 4,
  },
  actionsRow: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  actionButton: {
    alignItems: 'center',
    padding: 12,
  },
  actionLabel: {
    color: '#888',
    fontSize: 12,
    marginTop: 4,
  },
  configItem: {
    marginBottom: 24,
  },
  configHeader: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 8,
  },
  configLabel: {
    flex: 1,
    color: '#fff',
    fontSize: 16,
    marginLeft: 12,
  },
  configValue: {
    color: '#4CAF50',
    fontSize: 16,
    fontWeight: 'bold',
  },
  slider: {
    marginHorizontal: -8,
  },
  toggleItem: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  toggleLabel: {
    flex: 1,
    color: '#fff',
    fontSize: 16,
    marginLeft: 12,
  },
  toggleSwitch: {
    width: 48,
    height: 28,
    borderRadius: 14,
    backgroundColor: '#333',
    padding: 2,
  },
  toggleSwitchActive: {
    backgroundColor: '#4CAF50',
  },
  toggleKnob: {
    width: 24,
    height: 24,
    borderRadius: 12,
    backgroundColor: '#888',
  },
  toggleKnobActive: {
    backgroundColor: '#fff',
    marginLeft: 20,
  },
  updateButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#4CAF50',
    borderRadius: 12,
    padding: 16,
    marginTop: 16,
  },
  updateButtonDisabled: {
    backgroundColor: '#666',
  },
  updateButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: 'bold',
    marginLeft: 8,
  },
  detectionItem: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 12,
    marginBottom: 8,
  },
  detectionThumb: {
    width: 50,
    height: 50,
    borderRadius: 8,
    marginRight: 12,
  },
  detectionInfo: {
    flex: 1,
  },
  detectionSpecies: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '500',
  },
  detectionMeta: {
    color: '#888',
    fontSize: 12,
    marginTop: 4,
  },
  noDetectionsText: {
    color: '#666',
    textAlign: 'center',
    padding: 24,
  },
  infoRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  infoLabel: {
    color: '#888',
    fontSize: 14,
  },
  infoValue: {
    color: '#fff',
    fontSize: 14,
  },
  bottomPadding: {
    height: 32,
  },
});

export default DeviceDetailScreen;
