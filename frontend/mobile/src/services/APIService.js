/**
 * WildCAM Mobile - API Service
 * Handles all API communication with backend server
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import axios from 'axios';
import AsyncStorage from '@react-native-async-storage/async-storage';
import { Platform } from 'react-native';

// API Configuration
const API_CONFIG = {
  baseURL: 'https://api.wildlife-monitor.com',
  timeout: 30000,
  headers: {
    'Content-Type': 'application/json',
    'Accept': 'application/json',
  },
};

// Create axios instance
const apiClient = axios.create(API_CONFIG);

// Request interceptor for auth token
apiClient.interceptors.request.use(
  async (config) => {
    const token = await AsyncStorage.getItem('authToken');
    if (token) {
      config.headers.Authorization = `Bearer ${token}`;
    }
    return config;
  },
  (error) => {
    return Promise.reject(error);
  }
);

// Response interceptor for error handling
apiClient.interceptors.response.use(
  (response) => response,
  async (error) => {
    if (error.response?.status === 401) {
      // Handle unauthorized - clear token and redirect to login
      await AsyncStorage.removeItem('authToken');
      // Emit auth error event
    }
    return Promise.reject(error);
  }
);

/**
 * API Service class for WildCAM mobile app
 */
class APIService {
  /**
   * Set the API base URL
   * @param {string} url - The base URL for API calls
   */
  static setBaseURL(url) {
    apiClient.defaults.baseURL = url;
  }

  /**
   * Set authentication token
   * @param {string} token - JWT auth token
   */
  static async setAuthToken(token) {
    await AsyncStorage.setItem('authToken', token);
  }

  /**
   * Clear authentication token
   */
  static async clearAuthToken() {
    await AsyncStorage.removeItem('authToken');
  }

  // ==================== Device/Camera APIs ====================

  /**
   * Get all registered devices/cameras
   * @returns {Promise<Object>} - List of devices
   */
  static async getDevices() {
    const response = await apiClient.get('/api/v1/devices');
    return response.data;
  }

  /**
   * Get a specific device by ID
   * @param {string} deviceId - Device ID
   * @returns {Promise<Object>} - Device details
   */
  static async getDevice(deviceId) {
    const response = await apiClient.get(`/api/v1/devices/${deviceId}`);
    return response.data;
  }

  /**
   * Get device status
   * @param {string} deviceId - Device ID
   * @returns {Promise<Object>} - Device status
   */
  static async getDeviceStatus(deviceId) {
    const response = await apiClient.get(`/api/v1/devices/${deviceId}/status`);
    return response.data;
  }

  /**
   * Update device configuration
   * @param {string} deviceId - Device ID
   * @param {Object} config - Configuration object
   * @returns {Promise<Object>} - Updated configuration
   */
  static async updateDeviceConfig(deviceId, config) {
    const response = await apiClient.post(`/api/v1/devices/${deviceId}/config`, config);
    return response.data;
  }

  /**
   * Register a new device
   * @param {Object} deviceData - Device registration data
   * @returns {Promise<Object>} - Registration result
   */
  static async registerDevice(deviceData) {
    const response = await apiClient.post('/api/v1/devices', deviceData);
    return response.data;
  }

  // ==================== Detection APIs ====================

  /**
   * Get wildlife detections
   * @param {Object} params - Query parameters
   * @returns {Promise<Object>} - List of detections
   */
  static async getDetections(params = {}) {
    const response = await apiClient.get('/api/v1/detections', { params });
    return response.data;
  }

  /**
   * Get a specific detection by ID
   * @param {string} detectionId - Detection ID
   * @returns {Promise<Object>} - Detection details
   */
  static async getDetection(detectionId) {
    const response = await apiClient.get(`/api/v1/detections/${detectionId}`);
    return response.data;
  }

  /**
   * Verify a detection
   * @param {string} detectionId - Detection ID
   * @param {Object} verificationData - Verification data
   * @returns {Promise<Object>} - Verification result
   */
  static async verifyDetection(detectionId, verificationData) {
    const response = await apiClient.post(
      `/api/v1/detections/${detectionId}/verify`,
      verificationData
    );
    return response.data;
  }

  // ==================== Field Observation APIs ====================

  /**
   * Submit a field observation
   * @param {Object} observation - Observation data
   * @param {Object} image - Image file (optional)
   * @returns {Promise<Object>} - Submission result
   */
  static async submitFieldObservation(observation, image = null) {
    const formData = new FormData();
    formData.append('metadata', JSON.stringify(observation));
    
    if (image) {
      formData.append('image', {
        uri: image.uri,
        type: image.type || 'image/jpeg',
        name: image.fileName || 'observation.jpg',
      });
    }

    const response = await apiClient.post('/api/v1/observations', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    return response.data;
  }

  // ==================== Configuration APIs ====================

  /**
   * Get remote configuration
   * @returns {Promise<Object>} - Configuration object
   */
  static async getRemoteConfig() {
    const response = await apiClient.get('/api/v1/config');
    return response.data;
  }

  /**
   * Update remote configuration
   * @param {Object} config - Configuration object
   * @returns {Promise<Object>} - Updated configuration
   */
  static async updateRemoteConfig(config) {
    const response = await apiClient.post('/api/v1/config', config);
    return response.data;
  }

  // ==================== Push Notification APIs ====================

  /**
   * Register device push token
   * @param {string} token - Push notification token
   * @returns {Promise<Object>} - Registration result
   */
  static async registerPushToken(token) {
    const response = await apiClient.post('/api/v1/push/register', {
      token,
      platform: Platform.OS,
      device_id: await getUniqueDeviceId(),
    });
    return response.data;
  }

  /**
   * Update notification preferences
   * @param {Object} preferences - Notification preferences
   * @returns {Promise<Object>} - Updated preferences
   */
  static async updateNotificationPreferences(preferences) {
    const response = await apiClient.post('/api/v1/push/preferences', preferences);
    return response.data;
  }

  /**
   * Unregister push token
   * @returns {Promise<Object>} - Unregistration result
   */
  static async unregisterPushToken() {
    const response = await apiClient.delete('/api/v1/push/unregister');
    return response.data;
  }

  // ==================== Analytics APIs ====================

  /**
   * Get species analytics
   * @param {Object} params - Query parameters
   * @returns {Promise<Object>} - Species analytics data
   */
  static async getSpeciesAnalytics(params = {}) {
    const response = await apiClient.get('/api/v1/analytics/species-count', { params });
    return response.data;
  }

  /**
   * Get activity patterns
   * @param {Object} params - Query parameters
   * @returns {Promise<Object>} - Activity pattern data
   */
  static async getActivityPatterns(params = {}) {
    const response = await apiClient.get('/api/v1/analytics/activity-patterns', { params });
    return response.data;
  }

  // ==================== Image APIs ====================

  /**
   * Get image URL
   * @param {string} imageId - Image ID
   * @returns {string} - Full image URL
   */
  static getImageURL(imageId) {
    return `${apiClient.defaults.baseURL}/api/v1/images/${imageId}`;
  }

  /**
   * Get thumbnail URL
   * @param {string} imageId - Image ID
   * @returns {string} - Full thumbnail URL
   */
  static getThumbnailURL(imageId) {
    return `${apiClient.defaults.baseURL}/api/v1/thumbnails/${imageId}`;
  }

  // ==================== Health Check ====================

  /**
   * Check API health
   * @returns {Promise<Object>} - Health status
   */
  static async healthCheck() {
    const response = await apiClient.get('/api/v1/health');
    return response.data;
  }
}

/**
 * Get unique device identifier
 * @returns {Promise<string>} - Device ID
 */
async function getUniqueDeviceId() {
  let deviceId = await AsyncStorage.getItem('deviceId');
  if (!deviceId) {
    deviceId = `${Platform.OS}-${Date.now()}-${Math.random().toString(36).slice(2, 11)}`;
    await AsyncStorage.setItem('deviceId', deviceId);
  }
  return deviceId;
}

export default APIService;
export { apiClient, getUniqueDeviceId };
