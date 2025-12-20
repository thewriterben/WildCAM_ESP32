/**
 * WildCAM Mobile - Configuration Service
 * Handles remote configuration and local settings
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import AsyncStorage from '@react-native-async-storage/async-storage';
import APIService from './APIService';

/**
 * Configuration keys for local storage
 */
const CONFIG_KEYS = {
  API_BASE_URL: 'config_api_base_url',
  WEBSOCKET_URL: 'config_websocket_url',
  CAPTURE_INTERVAL: 'config_capture_interval',
  MOTION_SENSITIVITY: 'config_motion_sensitivity',
  ENABLE_NIGHT_MODE: 'config_enable_night_mode',
  ENABLE_CLOUD_UPLOAD: 'config_enable_cloud_upload',
  JPEG_QUALITY: 'config_jpeg_quality',
  FRAME_SIZE: 'config_frame_size',
  REMOTE_CONFIG: 'config_remote',
  REMOTE_CONFIG_TIME: 'config_remote_time',
  SETTINGS_VERSION: 'config_settings_version',
};

/**
 * Default configuration values
 */
const DEFAULT_CONFIG = {
  apiBaseURL: 'https://api.wildlife-monitor.com',
  websocketURL: 'wss://api.wildlife-monitor.com/ws',
  captureInterval: 300,
  motionSensitivity: 50,
  enableNightMode: true,
  enableCloudUpload: true,
  jpegQuality: 10,
  frameSize: 'UXGA',
};

/**
 * Configuration presets
 */
const PRESETS = {
  highQualityResearch: {
    name: 'High Quality Research',
    description: 'Best quality images, shorter intervals',
    captureInterval: 300,
    motionSensitivity: 70,
    enableNightMode: true,
    jpegQuality: 8,
    frameSize: 'UXGA',
  },
  longTermMonitoring: {
    name: 'Long-term Monitoring',
    description: 'Balanced quality and battery life',
    captureInterval: 600,
    motionSensitivity: 50,
    enableNightMode: true,
    jpegQuality: 15,
    frameSize: 'SXGA',
  },
  maximumBatteryLife: {
    name: 'Maximum Battery Life',
    description: 'Extended battery operation',
    captureInterval: 1800,
    motionSensitivity: 40,
    enableNightMode: false,
    jpegQuality: 20,
    frameSize: 'SVGA',
  },
  quickSurvey: {
    name: 'Quick Survey',
    description: 'Fast capture, moderate quality',
    captureInterval: 60,
    motionSensitivity: 80,
    enableNightMode: true,
    jpegQuality: 12,
    frameSize: 'XGA',
  },
};

/**
 * Frame size options with resolution info
 */
const FRAME_SIZES = {
  UXGA: { name: 'UXGA', resolution: '1600x1200', description: 'Highest quality' },
  SXGA: { name: 'SXGA', resolution: '1280x1024', description: 'High quality' },
  XGA: { name: 'XGA', resolution: '1024x768', description: 'Good quality' },
  SVGA: { name: 'SVGA', resolution: '800x600', description: 'Standard quality' },
  VGA: { name: 'VGA', resolution: '640x480', description: 'Basic quality' },
};

/**
 * ConfigurationService class for managing app and device configuration
 */
class ConfigurationService {
  constructor() {
    this.config = { ...DEFAULT_CONFIG };
    this.remoteConfig = null;
    this.lastRemoteConfigTime = null;
    this.configCacheDuration = 3600000; // 1 hour in milliseconds
    this.listeners = [];
  }

  /**
   * Initialize the configuration service
   */
  async initialize() {
    await this.loadLocalConfig();
    await this.loadCachedRemoteConfig();
    
    // Try to fetch remote config (non-blocking)
    this.fetchRemoteConfig().catch(console.error);
  }

  /**
   * Load local configuration from storage
   */
  async loadLocalConfig() {
    try {
      const keys = Object.values(CONFIG_KEYS);
      const results = await AsyncStorage.multiGet(keys);
      
      const configMap = {};
      results.forEach(([key, value]) => {
        if (value !== null) {
          try {
            configMap[key] = JSON.parse(value);
          } catch {
            configMap[key] = value;
          }
        }
      });

      // Apply loaded values
      if (configMap[CONFIG_KEYS.API_BASE_URL]) {
        this.config.apiBaseURL = configMap[CONFIG_KEYS.API_BASE_URL];
        APIService.setBaseURL(this.config.apiBaseURL);
      }
      if (configMap[CONFIG_KEYS.WEBSOCKET_URL]) {
        this.config.websocketURL = configMap[CONFIG_KEYS.WEBSOCKET_URL];
      }
      if (configMap[CONFIG_KEYS.CAPTURE_INTERVAL] !== undefined) {
        this.config.captureInterval = configMap[CONFIG_KEYS.CAPTURE_INTERVAL];
      }
      if (configMap[CONFIG_KEYS.MOTION_SENSITIVITY] !== undefined) {
        this.config.motionSensitivity = configMap[CONFIG_KEYS.MOTION_SENSITIVITY];
      }
      if (configMap[CONFIG_KEYS.ENABLE_NIGHT_MODE] !== undefined) {
        this.config.enableNightMode = configMap[CONFIG_KEYS.ENABLE_NIGHT_MODE];
      }
      if (configMap[CONFIG_KEYS.ENABLE_CLOUD_UPLOAD] !== undefined) {
        this.config.enableCloudUpload = configMap[CONFIG_KEYS.ENABLE_CLOUD_UPLOAD];
      }
      if (configMap[CONFIG_KEYS.JPEG_QUALITY] !== undefined) {
        this.config.jpegQuality = configMap[CONFIG_KEYS.JPEG_QUALITY];
      }
      if (configMap[CONFIG_KEYS.FRAME_SIZE]) {
        this.config.frameSize = configMap[CONFIG_KEYS.FRAME_SIZE];
      }
    } catch (error) {
      console.error('Failed to load local config:', error);
    }
  }

  /**
   * Load cached remote configuration
   */
  async loadCachedRemoteConfig() {
    try {
      const cachedConfig = await AsyncStorage.getItem(CONFIG_KEYS.REMOTE_CONFIG);
      const cachedTime = await AsyncStorage.getItem(CONFIG_KEYS.REMOTE_CONFIG_TIME);
      
      if (cachedConfig && cachedTime) {
        this.remoteConfig = JSON.parse(cachedConfig);
        this.lastRemoteConfigTime = new Date(JSON.parse(cachedTime));
      }
    } catch (error) {
      console.error('Failed to load cached remote config:', error);
    }
  }

  /**
   * Fetch remote configuration from server
   * @returns {Promise<Object>} - Remote configuration
   */
  async fetchRemoteConfig() {
    // Check if cache is still valid
    if (this.remoteConfig && this.lastRemoteConfigTime) {
      const elapsed = Date.now() - this.lastRemoteConfigTime.getTime();
      if (elapsed < this.configCacheDuration) {
        return this.remoteConfig;
      }
    }

    try {
      const response = await APIService.getRemoteConfig();
      this.remoteConfig = response;
      this.lastRemoteConfigTime = new Date();

      // Cache the remote config
      await AsyncStorage.setItem(CONFIG_KEYS.REMOTE_CONFIG, JSON.stringify(response));
      await AsyncStorage.setItem(CONFIG_KEYS.REMOTE_CONFIG_TIME, JSON.stringify(new Date().toISOString()));

      // Apply remote config to local settings
      this.applyRemoteConfig(response);

      // Notify listeners
      this.notifyListeners();

      return response;
    } catch (error) {
      console.error('Failed to fetch remote config:', error);
      throw error;
    }
  }

  /**
   * Apply remote configuration to local settings
   * @param {Object} remoteConfig - Remote configuration object
   */
  applyRemoteConfig(remoteConfig) {
    if (remoteConfig.capture_settings) {
      const { capture_settings } = remoteConfig;
      if (capture_settings.capture_delay !== undefined) {
        this.setConfigValue('captureInterval', capture_settings.capture_delay);
      }
      if (capture_settings.jpeg_quality !== undefined) {
        this.setConfigValue('jpegQuality', capture_settings.jpeg_quality);
      }
      if (capture_settings.frame_size !== undefined) {
        this.setConfigValue('frameSize', capture_settings.frame_size);
      }
      if (capture_settings.enable_night_mode !== undefined) {
        this.setConfigValue('enableNightMode', capture_settings.enable_night_mode);
      }
    }

    if (remoteConfig.upload_settings) {
      const { upload_settings } = remoteConfig;
      if (upload_settings.enable_auto_upload !== undefined) {
        this.setConfigValue('enableCloudUpload', upload_settings.enable_auto_upload);
      }
    }
  }

  // ==================== Getters ====================

  /**
   * Get all configuration values
   * @returns {Object} - Configuration object
   */
  getConfig() {
    return { ...this.config };
  }

  /**
   * Get a specific configuration value
   * @param {string} key - Configuration key
   * @returns {*} - Configuration value
   */
  getValue(key) {
    return this.config[key];
  }

  /**
   * Get API base URL
   * @returns {string} - API base URL
   */
  getAPIBaseURL() {
    return this.config.apiBaseURL;
  }

  /**
   * Get WebSocket URL
   * @returns {string} - WebSocket URL
   */
  getWebSocketURL() {
    return this.config.websocketURL;
  }

  /**
   * Get remote configuration
   * @returns {Object|null} - Remote configuration
   */
  getRemoteConfig() {
    return this.remoteConfig;
  }

  // ==================== Setters ====================

  /**
   * Set a configuration value
   * @param {string} key - Configuration key
   * @param {*} value - Configuration value
   */
  async setConfigValue(key, value) {
    this.config[key] = value;
    
    // Map to storage key and persist
    const storageKeyMap = {
      apiBaseURL: CONFIG_KEYS.API_BASE_URL,
      websocketURL: CONFIG_KEYS.WEBSOCKET_URL,
      captureInterval: CONFIG_KEYS.CAPTURE_INTERVAL,
      motionSensitivity: CONFIG_KEYS.MOTION_SENSITIVITY,
      enableNightMode: CONFIG_KEYS.ENABLE_NIGHT_MODE,
      enableCloudUpload: CONFIG_KEYS.ENABLE_CLOUD_UPLOAD,
      jpegQuality: CONFIG_KEYS.JPEG_QUALITY,
      frameSize: CONFIG_KEYS.FRAME_SIZE,
    };

    const storageKey = storageKeyMap[key];
    if (storageKey) {
      await AsyncStorage.setItem(storageKey, JSON.stringify(value));
    }

    // Special handling for API URL
    if (key === 'apiBaseURL') {
      APIService.setBaseURL(value);
    }

    // Notify listeners
    this.notifyListeners();
  }

  /**
   * Set multiple configuration values
   * @param {Object} values - Configuration values object
   */
  async setMultipleValues(values) {
    for (const [key, value] of Object.entries(values)) {
      await this.setConfigValue(key, value);
    }
  }

  // ==================== Device Configuration ====================

  /**
   * Push configuration to a specific device
   * @param {string} deviceId - Device ID
   * @returns {Promise<boolean>} - Success status
   */
  async pushConfigToDevice(deviceId) {
    try {
      const deviceConfig = {
        capture_interval: this.config.captureInterval,
        motion_sensitivity: this.config.motionSensitivity,
        enable_night_mode: this.config.enableNightMode,
        enable_cloud_upload: this.config.enableCloudUpload,
        jpeg_quality: this.config.jpegQuality,
        frame_size: this.config.frameSize,
      };

      await APIService.updateDeviceConfig(deviceId, deviceConfig);
      return true;
    } catch (error) {
      console.error('Failed to push config to device:', error);
      throw error;
    }
  }

  /**
   * Push configuration to multiple devices
   * @param {Array<string>} deviceIds - Array of device IDs
   * @returns {Promise<Object>} - Results for each device
   */
  async pushConfigToMultipleDevices(deviceIds) {
    const results = {};
    for (const deviceId of deviceIds) {
      try {
        await this.pushConfigToDevice(deviceId);
        results[deviceId] = { success: true };
      } catch (error) {
        results[deviceId] = { success: false, error: error.message };
      }
    }
    return results;
  }

  // ==================== Presets ====================

  /**
   * Get available presets
   * @returns {Object} - Available presets
   */
  getPresets() {
    return PRESETS;
  }

  /**
   * Apply a configuration preset
   * @param {string} presetKey - Preset key
   */
  async applyPreset(presetKey) {
    const preset = PRESETS[presetKey];
    if (!preset) {
      throw new Error(`Unknown preset: ${presetKey}`);
    }

    await this.setMultipleValues({
      captureInterval: preset.captureInterval,
      motionSensitivity: preset.motionSensitivity,
      enableNightMode: preset.enableNightMode,
      jpegQuality: preset.jpegQuality,
      frameSize: preset.frameSize,
    });
  }

  // ==================== Validation ====================

  /**
   * Validate configuration values
   * @returns {Array<string>} - Array of validation errors
   */
  validateConfig() {
    const errors = [];

    if (this.config.captureInterval < 1 || this.config.captureInterval > 3600) {
      errors.push('Capture interval must be between 1 and 3600 seconds');
    }

    if (this.config.motionSensitivity < 0 || this.config.motionSensitivity > 100) {
      errors.push('Motion sensitivity must be between 0 and 100');
    }

    if (this.config.jpegQuality < 1 || this.config.jpegQuality > 63) {
      errors.push('JPEG quality must be between 1 and 63');
    }

    if (!Object.keys(FRAME_SIZES).includes(this.config.frameSize)) {
      errors.push('Invalid frame size');
    }

    return errors;
  }

  // ==================== Export/Import ====================

  /**
   * Export configuration to JSON
   * @returns {string} - JSON configuration string
   */
  exportConfig() {
    return JSON.stringify({
      ...this.config,
      exportDate: new Date().toISOString(),
      version: '1.0',
    }, null, 2);
  }

  /**
   * Import configuration from JSON
   * @param {string} jsonString - JSON configuration string
   */
  async importConfig(jsonString) {
    try {
      const imported = JSON.parse(jsonString);
      
      // Validate and apply imported values
      const validKeys = ['captureInterval', 'motionSensitivity', 'enableNightMode', 
                         'enableCloudUpload', 'jpegQuality', 'frameSize'];
      
      for (const key of validKeys) {
        if (imported[key] !== undefined) {
          await this.setConfigValue(key, imported[key]);
        }
      }
    } catch (error) {
      throw new Error('Invalid configuration format');
    }
  }

  // ==================== Listeners ====================

  /**
   * Add a configuration change listener
   * @param {Function} listener - Listener function
   */
  addListener(listener) {
    this.listeners.push(listener);
  }

  /**
   * Remove a configuration change listener
   * @param {Function} listener - Listener function
   */
  removeListener(listener) {
    const index = this.listeners.indexOf(listener);
    if (index > -1) {
      this.listeners.splice(index, 1);
    }
  }

  /**
   * Notify all listeners of configuration change
   */
  notifyListeners() {
    this.listeners.forEach(listener => listener(this.config));
  }

  // ==================== Reset ====================

  /**
   * Reset configuration to defaults
   */
  async resetToDefaults() {
    this.config = { ...DEFAULT_CONFIG };
    
    const keys = Object.values(CONFIG_KEYS);
    await AsyncStorage.multiRemove(keys);
    
    this.notifyListeners();
  }
}

// Export singleton instance and utilities
const configService = new ConfigurationService();
export default configService;
export { CONFIG_KEYS, DEFAULT_CONFIG, PRESETS, FRAME_SIZES };
