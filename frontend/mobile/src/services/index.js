/**
 * WildCAM Mobile - Services Index
 * Central export for all service modules
 * 
 * @author WildCAM ESP32 Team
 * @version 3.0.0
 * @date 2025
 */

import APIService from './APIService';
import NotificationService from './NotificationService';
import ConfigurationService, { PRESETS, FRAME_SIZES } from './ConfigurationService';

export {
  APIService,
  NotificationService,
  ConfigurationService,
  PRESETS,
  FRAME_SIZES,
};
