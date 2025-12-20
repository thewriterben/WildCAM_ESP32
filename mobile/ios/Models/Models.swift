/**
 * WildCAM iOS - Data Models
 * Core data structures for the mobile app
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

import Foundation
import CoreLocation

// MARK: - Camera Model
struct Camera: Identifiable, Codable {
    let id: String
    let name: String
    let location: CameraLocation
    let batteryLevel: Float
    let lastSeen: Date
    let status: CameraStatus
    let firmwareVersion: String?
    
    enum CodingKeys: String, CodingKey {
        case id = "device_id"
        case name = "device_name"
        case location
        case batteryLevel = "battery_level"
        case lastSeen = "last_seen"
        case status
        case firmwareVersion = "firmware_version"
    }
}

struct CameraLocation: Codable {
    let lat: Double
    let lon: Double
    
    var coordinate: CLLocationCoordinate2D {
        CLLocationCoordinate2D(latitude: lat, longitude: lon)
    }
}

enum CameraStatus: String, Codable {
    case online = "online"
    case offline = "offline"
    case lowBattery = "low_battery"
    case error = "error"
    
    var displayName: String {
        switch self {
        case .online: return "Online"
        case .offline: return "Offline"
        case .lowBattery: return "Low Battery"
        case .error: return "Error"
        }
    }
    
    var color: String {
        switch self {
        case .online: return "green"
        case .offline: return "gray"
        case .lowBattery: return "yellow"
        case .error: return "red"
        }
    }
}

// MARK: - Detection Model
struct Detection: Identifiable, Codable {
    let id: String
    let cameraId: String
    let timestamp: Date
    let species: String
    let confidence: Float
    let imageUrl: String
    let verified: Bool
    let location: CameraLocation?
    let weatherConditions: WeatherConditions?
    let behavioralNotes: String?
    
    enum CodingKeys: String, CodingKey {
        case id
        case cameraId = "camera_id"
        case timestamp
        case species
        case confidence
        case imageUrl = "image_url"
        case verified
        case location
        case weatherConditions = "weather_conditions"
        case behavioralNotes = "behavioral_notes"
    }
}

struct WeatherConditions: Codable {
    let temperature: Float?
    let humidity: Float?
    let pressure: Float?
    let conditions: String?
}

// MARK: - Verification Model
struct VerificationData: Codable {
    let actualSpecies: String
    let verifiedBy: String
    let confidence: Float
    let notes: String?
    
    enum CodingKeys: String, CodingKey {
        case actualSpecies = "actual_species"
        case verifiedBy = "verified_by"
        case confidence
        case notes
    }
}

// MARK: - Field Observation Model
struct FieldObservation: Codable {
    let location: CameraLocation
    let species: String
    let notes: String
    let timestamp: Date
    let observer: String
    let imageData: Data?
    
    enum CodingKeys: String, CodingKey {
        case location
        case species
        case notes
        case timestamp
        case observer
        case imageData = "image_data"
    }
}

// MARK: - Device Status Model
struct DeviceStatus: Codable {
    let deviceId: String
    let batteryLevel: Float
    let solarPower: Float
    let temperature: Float
    let humidity: Float
    let lastSeen: Date
    let firmwareVersion: String
    let networkStrength: Int
    let storageUsed: Int64
    let storageTotal: Int64
    
    enum CodingKeys: String, CodingKey {
        case deviceId = "device_id"
        case batteryLevel = "battery_level"
        case solarPower = "solar_power"
        case temperature
        case humidity
        case lastSeen = "last_seen"
        case firmwareVersion = "firmware_version"
        case networkStrength = "network_strength"
        case storageUsed = "storage_used"
        case storageTotal = "storage_total"
    }
    
    var storagePercentage: Double {
        guard storageTotal > 0 else { return 0 }
        return Double(storageUsed) / Double(storageTotal) * 100
    }
}

// MARK: - Configuration Models
struct RemoteConfiguration: Codable {
    let version: String
    let captureSettings: CaptureSettings
    let detectionSettings: DetectionSettings
    let notificationSettings: NotificationSettings
    let uploadSettings: UploadSettings
    let powerSettings: PowerSettings
    
    enum CodingKeys: String, CodingKey {
        case version
        case captureSettings = "capture_settings"
        case detectionSettings = "detection_settings"
        case notificationSettings = "notification_settings"
        case uploadSettings = "upload_settings"
        case powerSettings = "power_settings"
    }
}

struct CaptureSettings: Codable {
    let jpegQuality: Int
    let frameSize: String
    let captureDelay: Int
    let motionCooldown: Int
    let enableNightMode: Bool
    
    enum CodingKeys: String, CodingKey {
        case jpegQuality = "jpeg_quality"
        case frameSize = "frame_size"
        case captureDelay = "capture_delay"
        case motionCooldown = "motion_cooldown"
        case enableNightMode = "enable_night_mode"
    }
}

struct DetectionSettings: Codable {
    let minConfidence: Float
    let enableSpeciesDetection: Bool
    let targetSpecies: [String]
    let enableTwoFactorDetection: Bool
    
    enum CodingKeys: String, CodingKey {
        case minConfidence = "min_confidence"
        case enableSpeciesDetection = "enable_species_detection"
        case targetSpecies = "target_species"
        case enableTwoFactorDetection = "enable_two_factor_detection"
    }
}

struct NotificationSettings: Codable {
    let enablePushNotifications: Bool
    let wildlifeAlerts: Bool
    let deviceStatusAlerts: Bool
    let lowBatteryAlerts: Bool
    let minConfidenceForAlert: Float
    let alertSpecies: [String]
    let quietHoursStart: String?
    let quietHoursEnd: String?
    
    enum CodingKeys: String, CodingKey {
        case enablePushNotifications = "enable_push_notifications"
        case wildlifeAlerts = "wildlife_alerts"
        case deviceStatusAlerts = "device_status_alerts"
        case lowBatteryAlerts = "low_battery_alerts"
        case minConfidenceForAlert = "min_confidence_for_alert"
        case alertSpecies = "alert_species"
        case quietHoursStart = "quiet_hours_start"
        case quietHoursEnd = "quiet_hours_end"
    }
}

struct UploadSettings: Codable {
    let enableAutoUpload: Bool
    let uploadOnWifiOnly: Bool
    let maxQueueSize: Int
    let retryAttempts: Int
    let compressionQuality: Int
    
    enum CodingKeys: String, CodingKey {
        case enableAutoUpload = "enable_auto_upload"
        case uploadOnWifiOnly = "upload_on_wifi_only"
        case maxQueueSize = "max_queue_size"
        case retryAttempts = "retry_attempts"
        case compressionQuality = "compression_quality"
    }
}

struct PowerSettings: Codable {
    let deepSleepDuration: Int
    let lowBatteryThreshold: Float
    let criticalBatteryThreshold: Float
    let enableSolarOptimization: Bool
    
    enum CodingKeys: String, CodingKey {
        case deepSleepDuration = "deep_sleep_duration"
        case lowBatteryThreshold = "low_battery_threshold"
        case criticalBatteryThreshold = "critical_battery_threshold"
        case enableSolarOptimization = "enable_solar_optimization"
    }
}

// MARK: - API Response Models
struct APIResponse<T: Codable>: Codable {
    let status: String
    let message: String?
    let data: T?
}

struct DevicesResponse: Codable {
    let devices: [Camera]
}

struct DetectionsResponse: Codable {
    let detections: [Detection]
    let total: Int
    let page: Int
    let perPage: Int
    
    enum CodingKeys: String, CodingKey {
        case detections
        case total
        case page
        case perPage = "per_page"
    }
}

// MARK: - Push Notification Payload
struct PushNotificationPayload: Codable {
    let type: NotificationType
    let title: String
    let body: String
    let data: NotificationData?
    
    enum NotificationType: String, Codable {
        case detection = "detection"
        case deviceStatus = "device_status"
        case lowBattery = "low_battery"
        case deviceOffline = "device_offline"
        case systemAlert = "system_alert"
    }
}

struct NotificationData: Codable {
    let detectionId: String?
    let deviceId: String?
    let species: String?
    let confidence: Float?
    let imageUrl: String?
    let batteryLevel: Float?
    let timestamp: Date?
    
    enum CodingKeys: String, CodingKey {
        case detectionId = "detection_id"
        case deviceId = "device_id"
        case species
        case confidence
        case imageUrl = "image_url"
        case batteryLevel = "battery_level"
        case timestamp
    }
}
