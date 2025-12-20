/**
 * WildCAM Mobile - Notification Service
 * Handles push notifications and real-time alerts
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import PushNotification from 'react-native-push-notification';
import PushNotificationIOS from '@react-native-community/push-notification-ios';
import { Platform, AppState } from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';
import APIService from './APIService';

/**
 * Notification channels for Android
 */
const NOTIFICATION_CHANNELS = {
  WILDLIFE_ALERTS: {
    channelId: 'wildlife-alerts',
    channelName: 'Wildlife Detection Alerts',
    channelDescription: 'Notifications for new wildlife detections',
    importance: 4,
    vibrate: true,
    playSound: true,
    soundName: 'default',
  },
  DEVICE_STATUS: {
    channelId: 'device-status',
    channelName: 'Device Status Updates',
    channelDescription: 'Notifications for camera status changes',
    importance: 3,
    vibrate: true,
    playSound: true,
    soundName: 'default',
  },
  LOW_BATTERY: {
    channelId: 'low-battery',
    channelName: 'Low Battery Warnings',
    channelDescription: 'Notifications for low battery alerts',
    importance: 4,
    vibrate: true,
    playSound: true,
    soundName: 'default',
  },
  SYSTEM: {
    channelId: 'system',
    channelName: 'System Notifications',
    channelDescription: 'General system notifications',
    importance: 2,
    vibrate: false,
    playSound: false,
  },
};

/**
 * Notification preferences storage keys
 */
const PREF_KEYS = {
  WILDLIFE_ALERTS: 'notif_wildlife_alerts',
  DEVICE_STATUS: 'notif_device_status',
  LOW_BATTERY: 'notif_low_battery',
  MIN_CONFIDENCE: 'notif_min_confidence',
  QUIET_HOURS_START: 'notif_quiet_start',
  QUIET_HOURS_END: 'notif_quiet_end',
};

/**
 * NotificationService class for managing push notifications
 */
class NotificationService {
  constructor() {
    this.isInitialized = false;
    this.deviceToken = null;
    this.onNotificationCallback = null;
    this.onRegistrationCallback = null;
  }

  /**
   * Initialize the notification service
   * @param {Object} options - Initialization options
   */
  initialize(options = {}) {
    if (this.isInitialized) {
      return;
    }

    const {
      onNotification,
      onRegistration,
      onRegistrationError,
    } = options;

    this.onNotificationCallback = onNotification;
    this.onRegistrationCallback = onRegistration;

    // Configure push notification library
    PushNotification.configure({
      // Called when a notification is received
      onNotification: (notification) => {
        this.handleNotification(notification);
      },

      // Called when device token is registered
      onRegister: (tokenData) => {
        this.handleRegistration(tokenData);
      },

      // Called when registration fails
      onRegistrationError: (error) => {
        console.error('Push notification registration error:', error);
        if (onRegistrationError) {
          onRegistrationError(error);
        }
      },

      // iOS permissions
      permissions: {
        alert: true,
        badge: true,
        sound: true,
      },

      // Request permissions automatically
      requestPermissions: Platform.OS === 'ios',

      // Android specific
      popInitialNotification: true,
      senderID: options.senderId || '', // Firebase sender ID
    });

    // Create notification channels for Android
    if (Platform.OS === 'android') {
      this.createNotificationChannels();
    }

    this.isInitialized = true;
  }

  /**
   * Create notification channels for Android
   */
  createNotificationChannels() {
    Object.values(NOTIFICATION_CHANNELS).forEach((channel) => {
      PushNotification.createChannel(channel, (created) => {
        console.log(`Notification channel '${channel.channelId}' created:`, created);
      });
    });
  }

  /**
   * Handle incoming notification
   * @param {Object} notification - Notification object
   */
  async handleNotification(notification) {
    console.log('Notification received:', notification);

    // Check if notification should be shown based on preferences
    const shouldShow = await this.shouldShowNotification(notification);
    if (!shouldShow) {
      // Use proper fallback for iOS fetch result
      const fetchResult = PushNotificationIOS?.FetchResult?.NoData ?? 0;
      if (notification.finish) {
        notification.finish(fetchResult);
      }
      return;
    }

    // Handle based on notification type
    const notificationType = notification.data?.type || notification.type;
    
    switch (notificationType) {
      case 'detection':
        await this.handleDetectionNotification(notification);
        break;
      case 'device_status':
        await this.handleDeviceStatusNotification(notification);
        break;
      case 'low_battery':
        await this.handleLowBatteryNotification(notification);
        break;
      default:
        // Generic notification handling
        break;
    }

    // Call user callback if provided
    if (this.onNotificationCallback) {
      this.onNotificationCallback(notification);
    }

    // Finish processing (iOS)
    notification.finish && notification.finish(PushNotificationIOS?.FetchResult?.NewData);
  }

  /**
   * Handle device registration
   * @param {Object} tokenData - Token data object
   */
  async handleRegistration(tokenData) {
    console.log('Device registered for push notifications:', tokenData);
    this.deviceToken = tokenData.token;

    try {
      // Register token with backend
      await APIService.registerPushToken(tokenData.token);
      console.log('Push token registered with backend');
    } catch (error) {
      console.error('Failed to register push token with backend:', error);
    }

    if (this.onRegistrationCallback) {
      this.onRegistrationCallback(tokenData);
    }
  }

  /**
   * Check if notification should be shown based on preferences
   * @param {Object} notification - Notification object
   * @returns {Promise<boolean>} - Whether to show notification
   */
  async shouldShowNotification(notification) {
    const notificationType = notification.data?.type || notification.type;

    // Check quiet hours
    if (await this.isQuietHours()) {
      return false;
    }

    // Check notification preferences
    switch (notificationType) {
      case 'detection':
        const wildlifeEnabled = await this.getPreference(PREF_KEYS.WILDLIFE_ALERTS, true);
        if (!wildlifeEnabled) return false;
        
        // Check minimum confidence threshold
        const minConfidence = await this.getPreference(PREF_KEYS.MIN_CONFIDENCE, 0.8);
        const confidence = notification.data?.confidence || 0;
        return confidence >= minConfidence;

      case 'device_status':
        return await this.getPreference(PREF_KEYS.DEVICE_STATUS, true);

      case 'low_battery':
        return await this.getPreference(PREF_KEYS.LOW_BATTERY, true);

      default:
        return true;
    }
  }

  /**
   * Check if current time is within quiet hours
   * @returns {Promise<boolean>} - Whether quiet hours are active
   */
  async isQuietHours() {
    const startHour = await this.getPreference(PREF_KEYS.QUIET_HOURS_START, null);
    const endHour = await this.getPreference(PREF_KEYS.QUIET_HOURS_END, null);

    if (startHour === null || endHour === null) {
      return false;
    }

    const now = new Date();
    const currentHour = now.getHours();

    if (startHour <= endHour) {
      return currentHour >= startHour && currentHour < endHour;
    } else {
      // Handles overnight quiet hours (e.g., 22:00 - 07:00)
      return currentHour >= startHour || currentHour < endHour;
    }
  }

  /**
   * Handle wildlife detection notification
   * @param {Object} notification - Notification object
   */
  async handleDetectionNotification(notification) {
    const { species, confidence, camera_id, image_url } = notification.data || {};
    
    // Update badge count
    this.incrementBadgeCount();

    // Store for later reference
    await this.storeRecentNotification('detection', notification.data);
  }

  /**
   * Handle device status notification
   * @param {Object} notification - Notification object
   */
  async handleDeviceStatusNotification(notification) {
    const { device_id, status } = notification.data || {};
    
    // Store for later reference
    await this.storeRecentNotification('device_status', notification.data);
  }

  /**
   * Handle low battery notification
   * @param {Object} notification - Notification object
   */
  async handleLowBatteryNotification(notification) {
    const { device_id, battery_level } = notification.data || {};
    
    // Store for later reference
    await this.storeRecentNotification('low_battery', notification.data);
  }

  /**
   * Store recent notification for reference
   * @param {string} type - Notification type
   * @param {Object} data - Notification data
   */
  async storeRecentNotification(type, data) {
    try {
      const key = 'recent_notifications';
      const stored = await AsyncStorage.getItem(key);
      let notifications = stored ? JSON.parse(stored) : [];
      
      notifications.unshift({
        type,
        data,
        timestamp: new Date().toISOString(),
      });

      // Keep only last 50 notifications
      notifications = notifications.slice(0, 50);
      
      await AsyncStorage.setItem(key, JSON.stringify(notifications));
    } catch (error) {
      console.error('Failed to store notification:', error);
    }
  }

  /**
   * Get recent notifications
   * @returns {Promise<Array>} - Array of recent notifications
   */
  async getRecentNotifications() {
    try {
      const stored = await AsyncStorage.getItem('recent_notifications');
      return stored ? JSON.parse(stored) : [];
    } catch (error) {
      console.error('Failed to get recent notifications:', error);
      return [];
    }
  }

  // ==================== Local Notifications ====================

  /**
   * Show a local notification for wildlife detection
   * @param {Object} detection - Detection data
   */
  showDetectionNotification(detection) {
    const { species, confidence, camera_id } = detection;
    const confidencePercent = Math.round((confidence || 0) * 100);

    PushNotification.localNotification({
      channelId: NOTIFICATION_CHANNELS.WILDLIFE_ALERTS.channelId,
      title: `${species || 'Wildlife'} Detected!`,
      message: `Confidence: ${confidencePercent}% at ${camera_id || 'Unknown Camera'}`,
      data: {
        type: 'detection',
        ...detection,
      },
      smallIcon: 'ic_notification',
      largeIcon: 'ic_launcher',
      priority: 'high',
      visibility: 'public',
      autoCancel: true,
    });
  }

  /**
   * Show a local notification for device status change
   * @param {string} deviceId - Device ID
   * @param {string} status - New status
   * @param {string} message - Optional message
   */
  showDeviceStatusNotification(deviceId, status, message = null) {
    const statusMessages = {
      online: 'Camera is now online',
      offline: 'Camera is now offline',
      error: 'Camera reported an error',
    };

    PushNotification.localNotification({
      channelId: NOTIFICATION_CHANNELS.DEVICE_STATUS.channelId,
      title: `Device Status: ${status.charAt(0).toUpperCase() + status.slice(1)}`,
      message: message || statusMessages[status] || `Status changed for ${deviceId}`,
      data: {
        type: 'device_status',
        device_id: deviceId,
        status,
      },
      smallIcon: 'ic_notification',
      priority: 'default',
      autoCancel: true,
    });
  }

  /**
   * Show a local notification for low battery
   * @param {string} deviceId - Device ID
   * @param {number} batteryLevel - Battery level percentage
   */
  showLowBatteryNotification(deviceId, batteryLevel) {
    PushNotification.localNotification({
      channelId: NOTIFICATION_CHANNELS.LOW_BATTERY.channelId,
      title: 'Low Battery Warning',
      message: `Camera ${deviceId} battery at ${batteryLevel}%`,
      data: {
        type: 'low_battery',
        device_id: deviceId,
        battery_level: batteryLevel,
      },
      smallIcon: 'ic_notification',
      priority: 'high',
      autoCancel: true,
    });
  }

  // ==================== Badge Management ====================

  /**
   * Set app badge count
   * @param {number} count - Badge count
   */
  setBadgeCount(count) {
    PushNotification.setApplicationIconBadgeNumber(count);
  }

  /**
   * Increment badge count
   */
  async incrementBadgeCount() {
    if (Platform.OS === 'ios') {
      PushNotificationIOS?.getApplicationIconBadgeNumber((current) => {
        PushNotification.setApplicationIconBadgeNumber(current + 1);
      });
    }
  }

  /**
   * Clear badge count
   */
  clearBadge() {
    this.setBadgeCount(0);
  }

  // ==================== Preferences ====================

  /**
   * Get notification preference
   * @param {string} key - Preference key
   * @param {*} defaultValue - Default value
   * @returns {Promise<*>} - Preference value
   */
  async getPreference(key, defaultValue) {
    try {
      const value = await AsyncStorage.getItem(key);
      if (value === null) return defaultValue;
      return JSON.parse(value);
    } catch (error) {
      return defaultValue;
    }
  }

  /**
   * Set notification preference
   * @param {string} key - Preference key
   * @param {*} value - Preference value
   */
  async setPreference(key, value) {
    try {
      await AsyncStorage.setItem(key, JSON.stringify(value));
      // Sync preferences with backend
      await this.syncPreferences();
    } catch (error) {
      console.error('Failed to set preference:', error);
    }
  }

  /**
   * Get all notification preferences
   * @returns {Promise<Object>} - All preferences
   */
  async getAllPreferences() {
    return {
      wildlifeAlerts: await this.getPreference(PREF_KEYS.WILDLIFE_ALERTS, true),
      deviceStatus: await this.getPreference(PREF_KEYS.DEVICE_STATUS, true),
      lowBattery: await this.getPreference(PREF_KEYS.LOW_BATTERY, true),
      minConfidence: await this.getPreference(PREF_KEYS.MIN_CONFIDENCE, 0.8),
      quietHoursStart: await this.getPreference(PREF_KEYS.QUIET_HOURS_START, null),
      quietHoursEnd: await this.getPreference(PREF_KEYS.QUIET_HOURS_END, null),
    };
  }

  /**
   * Sync preferences with backend
   */
  async syncPreferences() {
    try {
      const preferences = await this.getAllPreferences();
      await APIService.updateNotificationPreferences({
        enable_push_notifications: true,
        wildlife_alerts: preferences.wildlifeAlerts,
        device_status_alerts: preferences.deviceStatus,
        low_battery_alerts: preferences.lowBattery,
        min_confidence_for_alert: preferences.minConfidence,
        quiet_hours_start: preferences.quietHoursStart,
        quiet_hours_end: preferences.quietHoursEnd,
      });
    } catch (error) {
      console.error('Failed to sync preferences:', error);
    }
  }

  // ==================== Cleanup ====================

  /**
   * Cancel all notifications
   */
  cancelAllNotifications() {
    PushNotification.cancelAllLocalNotifications();
  }

  /**
   * Cancel a specific notification by ID
   * @param {string} notificationId - Notification ID
   */
  cancelNotification(notificationId) {
    PushNotification.cancelLocalNotification(notificationId);
  }

  /**
   * Cleanup and unregister
   */
  async cleanup() {
    try {
      await APIService.unregisterPushToken();
    } catch (error) {
      console.error('Failed to unregister push token:', error);
    }
    this.deviceToken = null;
    this.isInitialized = false;
  }
}

// Export singleton instance
export default new NotificationService();
export { NOTIFICATION_CHANNELS, PREF_KEYS };
