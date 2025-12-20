/**
 * WildCAM Mobile - Dashboard Screen
 * Main dashboard displaying camera overview and recent detections
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
} from 'react-native';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { APIService } from '../services';

const DashboardScreen = ({ navigation }) => {
  const [refreshing, setRefreshing] = useState(false);
  const [devices, setDevices] = useState([]);
  const [detections, setDetections] = useState([]);
  const [stats, setStats] = useState({
    totalCameras: 0,
    onlineCameras: 0,
    totalDetections: 0,
    todayDetections: 0,
  });

  useEffect(() => {
    loadDashboardData();
  }, []);

  const loadDashboardData = async () => {
    try {
      const [devicesResponse, detectionsResponse] = await Promise.all([
        APIService.getDevices(),
        APIService.getDetections({ limit: 10 }),
      ]);

      const deviceList = devicesResponse.devices || [];
      const detectionList = detectionsResponse.data || [];

      setDevices(deviceList);
      setDetections(detectionList);

      // Calculate stats
      const onlineCount = deviceList.filter(d => d.status === 'online').length;
      setStats({
        totalCameras: deviceList.length,
        onlineCameras: onlineCount,
        totalDetections: detectionList.length,
        todayDetections: detectionList.filter(d => isToday(d.timestamp)).length,
      });
    } catch (error) {
      console.error('Failed to load dashboard data:', error);
    }
  };

  const onRefresh = useCallback(async () => {
    setRefreshing(true);
    await loadDashboardData();
    setRefreshing(false);
  }, []);

  const isToday = (timestamp) => {
    const today = new Date();
    const date = new Date(timestamp);
    return date.toDateString() === today.toDateString();
  };

  return (
    <ScrollView
      style={styles.container}
      refreshControl={
        <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
      }
    >
      {/* Stats Cards */}
      <View style={styles.statsContainer}>
        <StatCard
          icon="videocam"
          title="Cameras"
          value={`${stats.onlineCameras}/${stats.totalCameras}`}
          subtitle="Online"
          color="#4CAF50"
        />
        <StatCard
          icon="photo-camera"
          title="Detections"
          value={stats.todayDetections.toString()}
          subtitle="Today"
          color="#2196F3"
        />
      </View>

      {/* Quick Actions */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Quick Actions</Text>
        <View style={styles.actionsContainer}>
          <ActionButton
            icon="add-a-photo"
            title="Capture"
            onPress={() => navigation.navigate('DeviceDetail')}
          />
          <ActionButton
            icon="settings"
            title="Configure"
            onPress={() => navigation.navigate('Settings')}
          />
          <ActionButton
            icon="map"
            title="Map"
            onPress={() => navigation.navigate('Map')}
          />
        </View>
      </View>

      {/* Camera Status */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Camera Status</Text>
        {devices.slice(0, 3).map((device) => (
          <TouchableOpacity
            key={device.device_id}
            style={styles.deviceCard}
            onPress={() => navigation.navigate('DeviceDetail', { deviceId: device.device_id })}
          >
            <View style={[styles.statusDot, { backgroundColor: getStatusColor(device.status) }]} />
            <View style={styles.deviceInfo}>
              <Text style={styles.deviceName}>{device.device_name || device.device_id}</Text>
              <Text style={styles.deviceStatus}>
                Battery: {device.battery_level || 0}%
              </Text>
            </View>
            <Icon name="chevron-right" size={24} color="#666" />
          </TouchableOpacity>
        ))}
      </View>

      {/* Recent Detections */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Recent Detections</Text>
        {detections.slice(0, 5).map((detection) => (
          <DetectionCard
            key={detection.id}
            detection={detection}
            onPress={() => navigation.navigate('SpeciesDetail', { detectionId: detection.id })}
          />
        ))}
      </View>
    </ScrollView>
  );
};

// Helper Components
const StatCard = ({ icon, title, value, subtitle, color }) => (
  <View style={styles.statCard}>
    <Icon name={icon} size={32} color={color} />
    <Text style={styles.statValue}>{value}</Text>
    <Text style={styles.statTitle}>{title}</Text>
    <Text style={styles.statSubtitle}>{subtitle}</Text>
  </View>
);

const ActionButton = ({ icon, title, onPress }) => (
  <TouchableOpacity style={styles.actionButton} onPress={onPress}>
    <Icon name={icon} size={28} color="#4CAF50" />
    <Text style={styles.actionTitle}>{title}</Text>
  </TouchableOpacity>
);

const DetectionCard = ({ detection, onPress }) => (
  <TouchableOpacity style={styles.detectionCard} onPress={onPress}>
    {detection.image_url && (
      <Image source={{ uri: detection.image_url }} style={styles.detectionImage} />
    )}
    <View style={styles.detectionInfo}>
      <Text style={styles.detectionSpecies}>{detection.species || 'Unknown'}</Text>
      <Text style={styles.detectionConfidence}>
        Confidence: {Math.round((detection.confidence || 0) * 100)}%
      </Text>
      <Text style={styles.detectionTime}>
        {new Date(detection.timestamp).toLocaleString()}
      </Text>
    </View>
    {detection.verified && (
      <Icon name="check-circle" size={20} color="#4CAF50" />
    )}
  </TouchableOpacity>
);

const getStatusColor = (status) => {
  switch (status) {
    case 'online': return '#4CAF50';
    case 'offline': return '#9E9E9E';
    case 'low_battery': return '#FF9800';
    case 'error': return '#F44336';
    default: return '#9E9E9E';
  }
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
  },
  statsContainer: {
    flexDirection: 'row',
    padding: 16,
    justifyContent: 'space-between',
  },
  statCard: {
    flex: 1,
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 16,
    alignItems: 'center',
    marginHorizontal: 4,
  },
  statValue: {
    fontSize: 28,
    fontWeight: 'bold',
    color: '#fff',
    marginTop: 8,
  },
  statTitle: {
    fontSize: 14,
    color: '#fff',
    marginTop: 4,
  },
  statSubtitle: {
    fontSize: 12,
    color: '#888',
  },
  section: {
    padding: 16,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#fff',
    marginBottom: 12,
  },
  actionsContainer: {
    flexDirection: 'row',
    justifyContent: 'space-around',
  },
  actionButton: {
    alignItems: 'center',
    padding: 16,
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    minWidth: 80,
  },
  actionTitle: {
    color: '#fff',
    marginTop: 8,
    fontSize: 12,
  },
  deviceCard: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 16,
    marginBottom: 8,
  },
  statusDot: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginRight: 12,
  },
  deviceInfo: {
    flex: 1,
  },
  deviceName: {
    color: '#fff',
    fontSize: 16,
    fontWeight: '500',
  },
  deviceStatus: {
    color: '#888',
    fontSize: 14,
  },
  detectionCard: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 12,
    marginBottom: 8,
  },
  detectionImage: {
    width: 60,
    height: 60,
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
  detectionConfidence: {
    color: '#888',
    fontSize: 14,
  },
  detectionTime: {
    color: '#666',
    fontSize: 12,
  },
});

export default DashboardScreen;
