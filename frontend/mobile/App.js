/**
 * WildCAM Mobile Companion App
 * React Native app for ESP32 WildCAM v2.0 wildlife monitoring
 * 
 * @author WildCAM ESP32 Team
 * @date 2025-09-29
 * @version 3.0.0
 */

import React, { useState, useEffect } from 'react';
import { NavigationContainer } from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';
import { createStackNavigator } from '@react-navigation/stack';
import {
  StyleSheet,
  View,
  Text,
  StatusBar,
  Alert,
  Platform,
  PermissionsAndroid
} from 'react-native';
import Icon from 'react-native-vector-icons/MaterialIcons';
import PushNotification from 'react-native-push-notification';
import io from 'socket.io-client';

// Import screens
import DashboardScreen from './src/screens/DashboardScreen';
import LiveFeedScreen from './src/screens/LiveFeedScreen';
import DetectionsScreen from './src/screens/DetectionsScreen';
import MapScreen from './src/screens/MapScreen';
import SettingsScreen from './src/screens/SettingsScreen';
import DeviceDetailScreen from './src/screens/DeviceDetailScreen';
import SpeciesDetailScreen from './src/screens/SpeciesDetailScreen';

// Import services
import APIService from './src/services/APIService';
import LocationService from './src/services/LocationService';

const Tab = createBottomTabNavigator();
const Stack = createStackNavigator();

// Main tab navigator
function MainTabs() {
  return (
    <Tab.Navigator
      screenOptions={({ route }) => ({
        tabBarIcon: ({ focused, color, size }) => {
          let iconName;

          switch (route.name) {
            case 'Dashboard':
              iconName = 'dashboard';
              break;
            case 'Live Feed':
              iconName = 'videocam';
              break;
            case 'Detections':
              iconName = 'photo-camera';
              break;
            case 'Map':
              iconName = 'map';
              break;
            case 'Settings':
              iconName = 'settings';
              break;
            default:
              iconName = 'help';
          }

          return <Icon name={iconName} size={size} color={color} />;
        },
        tabBarActiveTintColor: '#4CAF50',
        tabBarInactiveTintColor: '#757575',
        tabBarStyle: {
          backgroundColor: '#1e1e1e',
          borderTopColor: '#333333',
        },
        headerStyle: {
          backgroundColor: '#2e7d32',
        },
        headerTintColor: '#ffffff',
        headerTitleStyle: {
          fontWeight: 'bold',
        },
      })}
    >
      <Tab.Screen 
        name="Dashboard" 
        component={DashboardScreen}
        options={{
          title: '🦌 WildCAM Dashboard'
        }}
      />
      <Tab.Screen 
        name="Live Feed" 
        component={LiveFeedScreen}
        options={{
          title: 'Live Camera Feed'
        }}
      />
      <Tab.Screen 
        name="Detections" 
        component={DetectionsScreen}
        options={{
          title: 'Wildlife Detections',
          tabBarBadge: null // Will be updated with detection count
        }}
      />
      <Tab.Screen 
        name="Map" 
        component={MapScreen}
        options={{
          title: 'Camera Locations'
        }}
      />
      <Tab.Screen 
        name="Settings" 
        component={SettingsScreen}
        options={{
          title: 'Settings'
        }}
      />
    </Tab.Navigator>
  );
}

export default function App() {
  const [isConnected, setIsConnected] = useState(false);
  const [recentDetections, setRecentDetections] = useState([]);
  const [deviceCount, setDeviceCount] = useState(0);
  const [socket, setSocket] = useState(null);

  useEffect(() => {
    initializeApp();
    return () => {
      if (socket) {
        socket.disconnect();
      }
    };
  }, []);

  const initializeApp = async () => {
    try {
      // Request permissions
      await requestPermissions();
      
      // Initialize push notifications
      configurePushNotifications();
      
      // Connect to WebSocket
      initializeWebSocket();
      
      // Load initial data
      await loadInitialData();
      
    } catch (error) {
      console.error('App initialization failed:', error);
      Alert.alert(
        'Initialization Error',
        'Failed to initialize the app. Please check your network connection.',
        [{ text: 'OK' }]
      );
    }
  };

  const requestPermissions = async () => {
    if (Platform.OS === 'android') {
      try {
        const grants = await PermissionsAndroid.requestMultiple([
          PermissionsAndroid.PERMISSIONS.CAMERA,
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          PermissionsAndroid.PERMISSIONS.ACCESS_COARSE_LOCATION,
          PermissionsAndroid.PERMISSIONS.WRITE_EXTERNAL_STORAGE,
          PermissionsAndroid.PERMISSIONS.READ_EXTERNAL_STORAGE,
        ]);

        const allGranted = Object.values(grants).every(
          grant => grant === PermissionsAndroid.RESULTS.GRANTED
        );

        if (!allGranted) {
          Alert.alert(
            'Permissions Required',
            'This app requires camera and location permissions to function properly.',
            [{ text: 'OK' }]
          );
        }
      } catch (err) {
        console.warn('Permission request error:', err);
      }
    }
  };

  const configurePushNotifications = () => {
    PushNotification.configure({
      onNotification: function(notification) {
        console.log('Notification received:', notification);
        
        if (notification.userInteraction && notification.data) {
          // Handle notification tap
          handleNotificationTap(notification.data);
        }
      },
      requestPermissions: Platform.OS === 'ios',
    });

    // Create notification channel for Android
    PushNotification.createChannel(
      {
        channelId: "wildlife-alerts",
        channelName: "Wildlife Detection Alerts",
        channelDescription: "Notifications for new wildlife detections",
        playSound: true,
        soundName: "default",
        importance: 4,
        vibrate: true,
      },
      (created) => console.log(`Notification channel created: ${created}`)
    );
  };

  const initializeWebSocket = () => {
    const newSocket = io('ws://your-api-server:5000', {
      transports: ['websocket'],
      autoConnect: true,
    });

    newSocket.on('connect', () => {
      console.log('Connected to WildCAM API');
      setIsConnected(true);
    });

    newSocket.on('disconnect', () => {
      console.log('Disconnected from WildCAM API');
      setIsConnected(false);
    });

    newSocket.on('new_detection', (detection) => {
      console.log('New wildlife detection:', detection);
      
      // Update recent detections
      setRecentDetections(prev => [detection, ...prev.slice(0, 49)]);
      
      // Show push notification for significant detections
      if (detection.confidence > 0.8) {
        PushNotification.localNotification({
          channelId: "wildlife-alerts",
          title: `${detection.species} Detected!`,
          message: `Confidence: ${(detection.confidence * 100).toFixed(1)}% at ${detection.device_name}`,
          data: {
            type: 'detection',
            detection_id: detection.id,
            species: detection.species
          },
          smallIcon: "ic_notification",
          largeIcon: "ic_launcher",
        });
      }
    });

    newSocket.on('device_status_update', (status) => {
      console.log('Device status update:', status);
      // Handle device status updates
    });

    setSocket(newSocket);
  };

  const loadInitialData = async () => {
    try {
      // Load recent detections
      const detections = await APIService.getDetections({ limit: 20 });
      setRecentDetections(detections.data || []);
      
      // Load device count
      const devices = await APIService.getDevices();
      setDeviceCount(devices.devices?.length || 0);
      
    } catch (error) {
      console.error('Failed to load initial data:', error);
    }
  };

  const handleNotificationTap = (data) => {
    if (data.type === 'detection') {
      // Navigate to detection details
      // This would need to be implemented with navigation context
      console.log('Navigate to detection:', data.detection_id);
    }
  };

  return (
    <NavigationContainer>
      <StatusBar barStyle="light-content" backgroundColor="#2e7d32" />
      <Stack.Navigator>
        <Stack.Screen 
          name="Main" 
          component={MainTabs} 
          options={{ headerShown: false }}
        />
        <Stack.Screen 
          name="DeviceDetail" 
          component={DeviceDetailScreen}
          options={{
            title: 'Device Details',
            headerStyle: { backgroundColor: '#2e7d32' },
            headerTintColor: '#ffffff'
          }}
        />
        <Stack.Screen 
          name="SpeciesDetail" 
          component={SpeciesDetailScreen}
          options={{
            title: 'Species Information',
            headerStyle: { backgroundColor: '#2e7d32' },
            headerTintColor: '#ffffff'
          }}
        />
      </Stack.Navigator>
      
      {/* Connection status indicator */}
      {!isConnected && (
        <View style={styles.connectionBanner}>
          <Text style={styles.connectionText}>
            ⚠️ Not connected to WildCAM system
          </Text>
        </View>
      )}
    </NavigationContainer>
  );
}

const styles = StyleSheet.create({
  connectionBanner: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    backgroundColor: '#f44336',
    padding: 8,
    alignItems: 'center',
    zIndex: 1000,
  },
  connectionText: {
    color: '#ffffff',
    fontSize: 12,
    fontWeight: 'bold',
  },
});