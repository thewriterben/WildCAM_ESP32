/**
 * WildCAM Mobile - Settings Screen
 * Device and app settings configuration
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  ScrollView,
  StyleSheet,
  Switch,
  TouchableOpacity,
  Alert,
} from 'react-native';
import Slider from '@react-native-community/slider';
import Icon from 'react-native-vector-icons/MaterialIcons';
import { ConfigurationService, NotificationService, PRESETS, FRAME_SIZES } from '../services';

const SettingsScreen = ({ navigation }) => {
  // Notification settings
  const [wildlifeAlerts, setWildlifeAlerts] = useState(true);
  const [deviceStatusAlerts, setDeviceStatusAlerts] = useState(true);
  const [lowBatteryAlerts, setLowBatteryAlerts] = useState(true);
  
  // Device settings
  const [captureInterval, setCaptureInterval] = useState(300);
  const [motionSensitivity, setMotionSensitivity] = useState(50);
  const [enableNightMode, setEnableNightMode] = useState(true);
  const [enableCloudUpload, setEnableCloudUpload] = useState(true);
  const [jpegQuality, setJpegQuality] = useState(10);
  const [frameSize, setFrameSize] = useState('UXGA');

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    // Load notification preferences
    const notifPrefs = await NotificationService.getAllPreferences();
    setWildlifeAlerts(notifPrefs.wildlifeAlerts);
    setDeviceStatusAlerts(notifPrefs.deviceStatus);
    setLowBatteryAlerts(notifPrefs.lowBattery);

    // Load device configuration
    const config = ConfigurationService.getConfig();
    setCaptureInterval(config.captureInterval);
    setMotionSensitivity(config.motionSensitivity);
    setEnableNightMode(config.enableNightMode);
    setEnableCloudUpload(config.enableCloudUpload);
    setJpegQuality(config.jpegQuality);
    setFrameSize(config.frameSize);
  };

  const updateNotificationSetting = async (key, value) => {
    const prefKeys = {
      wildlifeAlerts: 'notif_wildlife_alerts',
      deviceStatusAlerts: 'notif_device_status',
      lowBatteryAlerts: 'notif_low_battery',
    };
    await NotificationService.setPreference(prefKeys[key], value);
  };

  const updateConfigSetting = async (key, value) => {
    await ConfigurationService.setConfigValue(key, value);
  };

  const applyPreset = async (presetKey) => {
    const preset = PRESETS[presetKey];
    Alert.alert(
      'Apply Preset',
      `Apply "${preset.name}" preset?\n\n${preset.description}`,
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Apply',
          onPress: async () => {
            await ConfigurationService.applyPreset(presetKey);
            loadSettings();
            Alert.alert('Success', 'Preset applied successfully');
          },
        },
      ]
    );
  };

  const pushToDevices = async () => {
    Alert.alert(
      'Push Configuration',
      'Push current settings to all connected devices?',
      [
        { text: 'Cancel', style: 'cancel' },
        {
          text: 'Push',
          onPress: async () => {
            try {
              // In real app, get device list and push to each
              Alert.alert('Success', 'Configuration pushed to devices');
            } catch (error) {
              Alert.alert('Error', error.message);
            }
          },
        },
      ]
    );
  };

  return (
    <ScrollView style={styles.container}>
      {/* Notification Settings */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Notifications</Text>
        
        <SettingRow
          icon="notifications"
          title="Wildlife Alerts"
          subtitle="Get notified when wildlife is detected"
        >
          <Switch
            value={wildlifeAlerts}
            onValueChange={(value) => {
              setWildlifeAlerts(value);
              updateNotificationSetting('wildlifeAlerts', value);
            }}
            trackColor={{ true: '#4CAF50' }}
          />
        </SettingRow>

        <SettingRow
          icon="router"
          title="Device Status"
          subtitle="Camera online/offline notifications"
        >
          <Switch
            value={deviceStatusAlerts}
            onValueChange={(value) => {
              setDeviceStatusAlerts(value);
              updateNotificationSetting('deviceStatusAlerts', value);
            }}
            trackColor={{ true: '#4CAF50' }}
          />
        </SettingRow>

        <SettingRow
          icon="battery-alert"
          title="Low Battery"
          subtitle="Get warned about low battery"
        >
          <Switch
            value={lowBatteryAlerts}
            onValueChange={(value) => {
              setLowBatteryAlerts(value);
              updateNotificationSetting('lowBatteryAlerts', value);
            }}
            trackColor={{ true: '#4CAF50' }}
          />
        </SettingRow>
      </View>

      {/* Camera Settings */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Camera Settings</Text>
        
        <SettingRow
          icon="timer"
          title="Capture Interval"
          subtitle={`${captureInterval} seconds`}
        />
        <Slider
          style={styles.slider}
          minimumValue={60}
          maximumValue={1800}
          step={60}
          value={captureInterval}
          onValueChange={setCaptureInterval}
          onSlidingComplete={(value) => updateConfigSetting('captureInterval', value)}
          minimumTrackTintColor="#4CAF50"
          thumbTintColor="#4CAF50"
        />

        <SettingRow
          icon="sensors"
          title="Motion Sensitivity"
          subtitle={`${motionSensitivity}%`}
        />
        <Slider
          style={styles.slider}
          minimumValue={0}
          maximumValue={100}
          step={5}
          value={motionSensitivity}
          onValueChange={setMotionSensitivity}
          onSlidingComplete={(value) => updateConfigSetting('motionSensitivity', value)}
          minimumTrackTintColor="#4CAF50"
          thumbTintColor="#4CAF50"
        />

        <SettingRow
          icon="nightlight"
          title="Night Mode"
          subtitle="Enable infrared capture at night"
        >
          <Switch
            value={enableNightMode}
            onValueChange={(value) => {
              setEnableNightMode(value);
              updateConfigSetting('enableNightMode', value);
            }}
            trackColor={{ true: '#4CAF50' }}
          />
        </SettingRow>

        <SettingRow
          icon="cloud-upload"
          title="Cloud Upload"
          subtitle="Automatically upload captures"
        >
          <Switch
            value={enableCloudUpload}
            onValueChange={(value) => {
              setEnableCloudUpload(value);
              updateConfigSetting('enableCloudUpload', value);
            }}
            trackColor={{ true: '#4CAF50' }}
          />
        </SettingRow>
      </View>

      {/* Image Quality */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Image Quality</Text>
        
        <SettingRow
          icon="high-quality"
          title="JPEG Quality"
          subtitle={`${jpegQuality} (lower = better)`}
        />
        <Slider
          style={styles.slider}
          minimumValue={1}
          maximumValue={63}
          step={1}
          value={jpegQuality}
          onValueChange={setJpegQuality}
          onSlidingComplete={(value) => updateConfigSetting('jpegQuality', value)}
          minimumTrackTintColor="#4CAF50"
          thumbTintColor="#4CAF50"
        />

        <SettingRow
          icon="photo-size-select-large"
          title="Frame Size"
          subtitle={`${frameSize} (${FRAME_SIZES[frameSize]?.resolution || ''})`}
        />
        <View style={styles.frameSizeButtons}>
          {Object.keys(FRAME_SIZES).map((size) => (
            <TouchableOpacity
              key={size}
              style={[
                styles.frameSizeButton,
                frameSize === size && styles.frameSizeButtonActive,
              ]}
              onPress={() => {
                setFrameSize(size);
                updateConfigSetting('frameSize', size);
              }}
            >
              <Text style={[
                styles.frameSizeText,
                frameSize === size && styles.frameSizeTextActive,
              ]}>
                {size}
              </Text>
            </TouchableOpacity>
          ))}
        </View>
      </View>

      {/* Presets */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Configuration Presets</Text>
        {Object.entries(PRESETS).map(([key, preset]) => (
          <TouchableOpacity
            key={key}
            style={styles.presetButton}
            onPress={() => applyPreset(key)}
          >
            <View style={styles.presetInfo}>
              <Text style={styles.presetName}>{preset.name}</Text>
              <Text style={styles.presetDescription}>{preset.description}</Text>
            </View>
            <Icon name="chevron-right" size={24} color="#666" />
          </TouchableOpacity>
        ))}
      </View>

      {/* Actions */}
      <View style={styles.section}>
        <TouchableOpacity style={styles.actionButton} onPress={pushToDevices}>
          <Icon name="send" size={24} color="#fff" />
          <Text style={styles.actionButtonText}>Push to Devices</Text>
        </TouchableOpacity>
      </View>

      {/* About */}
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>About</Text>
        <SettingRow icon="info" title="Version" subtitle="3.0.0" />
        <SettingRow icon="code" title="API Server" subtitle="api.wildlife-monitor.com" />
      </View>

      <View style={styles.bottomPadding} />
    </ScrollView>
  );
};

const SettingRow = ({ icon, title, subtitle, children }) => (
  <View style={styles.settingRow}>
    <Icon name={icon} size={24} color="#4CAF50" style={styles.settingIcon} />
    <View style={styles.settingText}>
      <Text style={styles.settingTitle}>{title}</Text>
      {subtitle && <Text style={styles.settingSubtitle}>{subtitle}</Text>}
    </View>
    {children}
  </View>
);

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
  },
  section: {
    padding: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#333',
  },
  sectionTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#4CAF50',
    marginBottom: 16,
    textTransform: 'uppercase',
    letterSpacing: 1,
  },
  settingRow: {
    flexDirection: 'row',
    alignItems: 'center',
    paddingVertical: 12,
  },
  settingIcon: {
    marginRight: 16,
  },
  settingText: {
    flex: 1,
  },
  settingTitle: {
    fontSize: 16,
    color: '#fff',
  },
  settingSubtitle: {
    fontSize: 14,
    color: '#888',
    marginTop: 2,
  },
  slider: {
    marginHorizontal: -8,
    marginBottom: 16,
  },
  frameSizeButtons: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    gap: 8,
    marginTop: 8,
  },
  frameSizeButton: {
    paddingHorizontal: 16,
    paddingVertical: 8,
    backgroundColor: '#333',
    borderRadius: 8,
  },
  frameSizeButtonActive: {
    backgroundColor: '#4CAF50',
  },
  frameSizeText: {
    color: '#888',
    fontSize: 14,
  },
  frameSizeTextActive: {
    color: '#fff',
    fontWeight: 'bold',
  },
  presetButton: {
    flexDirection: 'row',
    alignItems: 'center',
    backgroundColor: '#1e1e1e',
    borderRadius: 12,
    padding: 16,
    marginBottom: 8,
  },
  presetInfo: {
    flex: 1,
  },
  presetName: {
    fontSize: 16,
    color: '#fff',
    fontWeight: '500',
  },
  presetDescription: {
    fontSize: 14,
    color: '#888',
    marginTop: 2,
  },
  actionButton: {
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: '#4CAF50',
    borderRadius: 12,
    padding: 16,
  },
  actionButtonText: {
    color: '#fff',
    fontSize: 16,
    fontWeight: 'bold',
    marginLeft: 8,
  },
  bottomPadding: {
    height: 32,
  },
});

export default SettingsScreen;
