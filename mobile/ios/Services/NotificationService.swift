/**
 * WildCAM iOS - Notification Service
 * Handles push notifications and real-time alerts
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

import Foundation
import UserNotifications
import Combine

// MARK: - Notification Service
class NotificationService: ObservableObject {
    static let shared = NotificationService()
    
    // Published properties for settings
    @Published var wildlifeAlertsEnabled: Bool {
        didSet {
            UserDefaults.standard.set(wildlifeAlertsEnabled, forKey: "wildlifeAlertsEnabled")
            updatePreferencesOnServer()
        }
    }
    @Published var deviceStatusEnabled: Bool {
        didSet {
            UserDefaults.standard.set(deviceStatusEnabled, forKey: "deviceStatusEnabled")
            updatePreferencesOnServer()
        }
    }
    @Published var lowBatteryEnabled: Bool {
        didSet {
            UserDefaults.standard.set(lowBatteryEnabled, forKey: "lowBatteryEnabled")
            updatePreferencesOnServer()
        }
    }
    @Published var isAuthorized = false
    @Published var pendingNotifications: [PendingNotification] = []
    
    // Notification handlers
    var onDetectionNotification: ((NotificationData) -> Void)?
    var onDeviceStatusNotification: ((NotificationData) -> Void)?
    
    private var deviceToken: String?
    private var cancellables = Set<AnyCancellable>()
    
    private init() {
        // Load settings from UserDefaults
        wildlifeAlertsEnabled = UserDefaults.standard.bool(forKey: "wildlifeAlertsEnabled")
        deviceStatusEnabled = UserDefaults.standard.bool(forKey: "deviceStatusEnabled")
        lowBatteryEnabled = UserDefaults.standard.bool(forKey: "lowBatteryEnabled")
        
        // Set defaults if not set
        if !UserDefaults.standard.bool(forKey: "notificationSettingsInitialized") {
            wildlifeAlertsEnabled = true
            deviceStatusEnabled = true
            lowBatteryEnabled = true
            UserDefaults.standard.set(true, forKey: "notificationSettingsInitialized")
        }
        
        checkAuthorizationStatus()
    }
    
    // MARK: - Authorization
    
    func requestAuthorization() {
        let center = UNUserNotificationCenter.current()
        center.requestAuthorization(options: [.alert, .sound, .badge]) { [weak self] granted, error in
            DispatchQueue.main.async {
                self?.isAuthorized = granted
                
                if granted {
                    self?.configureNotificationCategories()
                }
                
                if let error = error {
                    print("Notification authorization error: \(error.localizedDescription)")
                }
            }
        }
    }
    
    func checkAuthorizationStatus() {
        UNUserNotificationCenter.current().getNotificationSettings { [weak self] settings in
            DispatchQueue.main.async {
                self?.isAuthorized = settings.authorizationStatus == .authorized
            }
        }
    }
    
    // MARK: - Device Token Registration
    
    func registerDeviceToken(_ token: String) {
        self.deviceToken = token
        
        Task {
            do {
                try await APIService.shared.registerDeviceToken(token: token)
                print("Device token registered successfully")
            } catch {
                print("Failed to register device token: \(error.localizedDescription)")
            }
        }
    }
    
    // MARK: - Notification Categories
    
    private func configureNotificationCategories() {
        // Wildlife Detection Category
        let verifyAction = UNNotificationAction(
            identifier: "VERIFY_ACTION",
            title: "Verify",
            options: .foreground
        )
        let dismissAction = UNNotificationAction(
            identifier: "DISMISS_ACTION",
            title: "Dismiss",
            options: .destructive
        )
        let detectionCategory = UNNotificationCategory(
            identifier: "WILDLIFE_DETECTION",
            actions: [verifyAction, dismissAction],
            intentIdentifiers: [],
            options: .customDismissAction
        )
        
        // Device Status Category
        let viewDeviceAction = UNNotificationAction(
            identifier: "VIEW_DEVICE_ACTION",
            title: "View Device",
            options: .foreground
        )
        let deviceCategory = UNNotificationCategory(
            identifier: "DEVICE_STATUS",
            actions: [viewDeviceAction, dismissAction],
            intentIdentifiers: [],
            options: .customDismissAction
        )
        
        // Low Battery Category
        let lowBatteryCategory = UNNotificationCategory(
            identifier: "LOW_BATTERY",
            actions: [viewDeviceAction, dismissAction],
            intentIdentifiers: [],
            options: .customDismissAction
        )
        
        UNUserNotificationCenter.current().setNotificationCategories([
            detectionCategory,
            deviceCategory,
            lowBatteryCategory
        ])
    }
    
    // MARK: - Handle Notification Tap
    
    func handleNotificationTap(userInfo: [AnyHashable: Any]) {
        guard let typeString = userInfo["type"] as? String,
              let type = PushNotificationPayload.NotificationType(rawValue: typeString) else {
            return
        }
        
        let data = parseNotificationData(userInfo)
        
        switch type {
        case .detection:
            onDetectionNotification?(data)
        case .deviceStatus, .lowBattery, .deviceOffline:
            onDeviceStatusNotification?(data)
        case .systemAlert:
            // Handle system alerts
            break
        }
    }
    
    private func parseNotificationData(_ userInfo: [AnyHashable: Any]) -> NotificationData {
        return NotificationData(
            detectionId: userInfo["detection_id"] as? String,
            deviceId: userInfo["device_id"] as? String,
            species: userInfo["species"] as? String,
            confidence: (userInfo["confidence"] as? NSNumber)?.floatValue,
            imageUrl: userInfo["image_url"] as? String,
            batteryLevel: (userInfo["battery_level"] as? NSNumber)?.floatValue,
            timestamp: nil
        )
    }
    
    // MARK: - Local Notifications
    
    func scheduleLocalNotification(
        title: String,
        body: String,
        category: String,
        userInfo: [String: Any] = [:],
        delay: TimeInterval = 0
    ) {
        guard isAuthorized else { return }
        
        let content = UNMutableNotificationContent()
        content.title = title
        content.body = body
        content.sound = .default
        content.categoryIdentifier = category
        content.userInfo = userInfo
        
        let trigger: UNNotificationTrigger?
        if delay > 0 {
            trigger = UNTimeIntervalNotificationTrigger(timeInterval: delay, repeats: false)
        } else {
            trigger = nil
        }
        
        let request = UNNotificationRequest(
            identifier: UUID().uuidString,
            content: content,
            trigger: trigger
        )
        
        UNUserNotificationCenter.current().add(request) { error in
            if let error = error {
                print("Failed to schedule notification: \(error.localizedDescription)")
            }
        }
    }
    
    func showWildlifeDetectionNotification(detection: Detection) {
        guard wildlifeAlertsEnabled else { return }
        
        let confidence = Int(detection.confidence * 100)
        scheduleLocalNotification(
            title: "\(detection.species) Detected!",
            body: "Confidence: \(confidence)%",
            category: "WILDLIFE_DETECTION",
            userInfo: [
                "type": "detection",
                "detection_id": detection.id,
                "species": detection.species,
                "confidence": detection.confidence
            ]
        )
    }
    
    func showDeviceStatusNotification(deviceId: String, status: CameraStatus) {
        guard deviceStatusEnabled else { return }
        
        let title: String
        let body: String
        
        switch status {
        case .offline:
            title = "Device Offline"
            body = "Camera \(deviceId) is now offline"
        case .online:
            title = "Device Online"
            body = "Camera \(deviceId) is back online"
        case .lowBattery:
            guard lowBatteryEnabled else { return }
            title = "Low Battery Warning"
            body = "Camera \(deviceId) has low battery"
        case .error:
            title = "Device Error"
            body = "Camera \(deviceId) reported an error"
        }
        
        scheduleLocalNotification(
            title: title,
            body: body,
            category: "DEVICE_STATUS",
            userInfo: [
                "type": "device_status",
                "device_id": deviceId,
                "status": status.rawValue
            ]
        )
    }
    
    // MARK: - Update Preferences
    
    private func updatePreferencesOnServer() {
        let preferences = NotificationSettings(
            enablePushNotifications: isAuthorized,
            wildlifeAlerts: wildlifeAlertsEnabled,
            deviceStatusAlerts: deviceStatusEnabled,
            lowBatteryAlerts: lowBatteryEnabled,
            minConfidenceForAlert: 0.8,
            alertSpecies: [],
            quietHoursStart: nil,
            quietHoursEnd: nil
        )
        
        Task {
            do {
                try await APIService.shared.updateNotificationPreferences(preferences: preferences)
            } catch {
                print("Failed to update notification preferences: \(error.localizedDescription)")
            }
        }
    }
    
    // MARK: - Badge Management
    
    func updateBadgeCount(_ count: Int) {
        DispatchQueue.main.async {
            UNUserNotificationCenter.current().setBadgeCount(count)
        }
    }
    
    func clearBadge() {
        updateBadgeCount(0)
    }
    
    // MARK: - Pending Notifications
    
    func getPendingNotifications() {
        UNUserNotificationCenter.current().getPendingNotificationRequests { [weak self] requests in
            DispatchQueue.main.async {
                self?.pendingNotifications = requests.map { request in
                    PendingNotification(
                        identifier: request.identifier,
                        title: request.content.title,
                        body: request.content.body,
                        category: request.content.categoryIdentifier
                    )
                }
            }
        }
    }
    
    func cancelNotification(identifier: String) {
        UNUserNotificationCenter.current().removePendingNotificationRequests(withIdentifiers: [identifier])
        pendingNotifications.removeAll { $0.identifier == identifier }
    }
    
    func cancelAllNotifications() {
        UNUserNotificationCenter.current().removeAllPendingNotificationRequests()
        pendingNotifications.removeAll()
    }
}

// MARK: - Pending Notification Model

struct PendingNotification: Identifiable {
    let id = UUID()
    let identifier: String
    let title: String
    let body: String
    let category: String
}
