/**
 * WildCAM iOS - Configuration Service
 * Handles remote configuration and local settings
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

import Foundation
import Combine

// MARK: - Configuration Service
class ConfigurationService: ObservableObject {
    static let shared = ConfigurationService()
    
    // API configuration
    @Published var apiBaseURL: String
    @Published var websocketURL: String
    
    // Remote configuration
    @Published var remoteConfig: RemoteConfiguration?
    @Published var lastConfigUpdate: Date?
    @Published var isConfigLoaded = false
    
    // Local settings
    @Published var captureInterval: Int {
        didSet {
            UserDefaults.standard.set(captureInterval, forKey: "captureInterval")
        }
    }
    @Published var motionSensitivity: Int {
        didSet {
            UserDefaults.standard.set(motionSensitivity, forKey: "motionSensitivity")
        }
    }
    @Published var enableNightMode: Bool {
        didSet {
            UserDefaults.standard.set(enableNightMode, forKey: "enableNightMode")
        }
    }
    @Published var enableCloudUpload: Bool {
        didSet {
            UserDefaults.standard.set(enableCloudUpload, forKey: "enableCloudUpload")
        }
    }
    @Published var jpegQuality: Int {
        didSet {
            UserDefaults.standard.set(jpegQuality, forKey: "jpegQuality")
        }
    }
    @Published var frameSize: String {
        didSet {
            UserDefaults.standard.set(frameSize, forKey: "frameSize")
        }
    }
    
    // Configuration cache
    private let configCacheKey = "cachedRemoteConfig"
    private let configCacheTimeKey = "cachedRemoteConfigTime"
    private let configCacheDuration: TimeInterval = 3600 // 1 hour
    
    private var cancellables = Set<AnyCancellable>()
    
    private init() {
        // Load API URLs from Info.plist or use defaults
        let bundle = Bundle.main
        self.apiBaseURL = bundle.object(forInfoDictionaryKey: "API_BASE_URL") as? String
            ?? "https://api.wildlife-monitor.com"
        self.websocketURL = bundle.object(forInfoDictionaryKey: "WEBSOCKET_URL") as? String
            ?? "wss://api.wildlife-monitor.com/ws"
        
        // Load local settings from UserDefaults
        captureInterval = UserDefaults.standard.integer(forKey: "captureInterval")
        motionSensitivity = UserDefaults.standard.integer(forKey: "motionSensitivity")
        enableNightMode = UserDefaults.standard.bool(forKey: "enableNightMode")
        enableCloudUpload = UserDefaults.standard.bool(forKey: "enableCloudUpload")
        jpegQuality = UserDefaults.standard.integer(forKey: "jpegQuality")
        frameSize = UserDefaults.standard.string(forKey: "frameSize") ?? "UXGA"
        
        // Set defaults if not configured
        if captureInterval == 0 { captureInterval = 300 }
        if motionSensitivity == 0 { motionSensitivity = 50 }
        if jpegQuality == 0 { jpegQuality = 10 }
        if !UserDefaults.standard.bool(forKey: "settingsInitialized") {
            enableCloudUpload = true
            UserDefaults.standard.set(true, forKey: "settingsInitialized")
        }
        
        // Load cached configuration
        loadCachedConfiguration()
    }
    
    // MARK: - Remote Configuration
    
    func fetchConfiguration() async {
        // Check if cache is still valid
        if let lastUpdate = lastConfigUpdate,
           Date().timeIntervalSince(lastUpdate) < configCacheDuration,
           remoteConfig != nil {
            return
        }
        
        do {
            let config = try await APIService.shared.getRemoteConfiguration()
            
            await MainActor.run {
                self.remoteConfig = config
                self.lastConfigUpdate = Date()
                self.isConfigLoaded = true
                self.applyRemoteConfiguration(config)
                self.cacheConfiguration(config)
            }
        } catch {
            print("Failed to fetch remote configuration: \(error.localizedDescription)")
            // Use cached configuration if available
        }
    }
    
    private func applyRemoteConfiguration(_ config: RemoteConfiguration) {
        // Apply capture settings
        if captureInterval != config.captureSettings.captureDelay {
            captureInterval = config.captureSettings.captureDelay
        }
        if jpegQuality != config.captureSettings.jpegQuality {
            jpegQuality = config.captureSettings.jpegQuality
        }
        if frameSize != config.captureSettings.frameSize {
            frameSize = config.captureSettings.frameSize
        }
        if enableNightMode != config.captureSettings.enableNightMode {
            enableNightMode = config.captureSettings.enableNightMode
        }
        
        // Apply upload settings
        if enableCloudUpload != config.uploadSettings.enableAutoUpload {
            enableCloudUpload = config.uploadSettings.enableAutoUpload
        }
    }
    
    private func cacheConfiguration(_ config: RemoteConfiguration) {
        if let data = try? JSONEncoder().encode(config) {
            UserDefaults.standard.set(data, forKey: configCacheKey)
            UserDefaults.standard.set(Date(), forKey: configCacheTimeKey)
        }
    }
    
    private func loadCachedConfiguration() {
        guard let data = UserDefaults.standard.data(forKey: configCacheKey),
              let config = try? JSONDecoder().decode(RemoteConfiguration.self, from: data) else {
            return
        }
        
        remoteConfig = config
        lastConfigUpdate = UserDefaults.standard.object(forKey: configCacheTimeKey) as? Date
        isConfigLoaded = true
    }
    
    // MARK: - Push Configuration to Device
    
    func pushConfigurationToDevice(deviceId: String) async throws {
        let configuration: [String: Any] = [
            "capture_interval": captureInterval,
            "motion_sensitivity": motionSensitivity,
            "enable_night_mode": enableNightMode,
            "enable_cloud_upload": enableCloudUpload,
            "jpeg_quality": jpegQuality,
            "frame_size": frameSize
        ]
        
        _ = try await APIService.shared.updateCameraConfiguration(
            deviceId: deviceId,
            configuration: configuration
        )
    }
    
    // MARK: - Configuration Presets
    
    struct ConfigurationPreset {
        let name: String
        let description: String
        let captureInterval: Int
        let motionSensitivity: Int
        let enableNightMode: Bool
        let jpegQuality: Int
        let frameSize: String
    }
    
    static let presets: [ConfigurationPreset] = [
        ConfigurationPreset(
            name: "High Quality Research",
            description: "Best quality images, shorter intervals",
            captureInterval: 300,
            motionSensitivity: 70,
            enableNightMode: true,
            jpegQuality: 8,
            frameSize: "UXGA"
        ),
        ConfigurationPreset(
            name: "Long-term Monitoring",
            description: "Balanced quality and battery life",
            captureInterval: 600,
            motionSensitivity: 50,
            enableNightMode: true,
            jpegQuality: 15,
            frameSize: "SXGA"
        ),
        ConfigurationPreset(
            name: "Maximum Battery Life",
            description: "Extended battery operation",
            captureInterval: 1800,
            motionSensitivity: 40,
            enableNightMode: false,
            jpegQuality: 20,
            frameSize: "SVGA"
        ),
        ConfigurationPreset(
            name: "Quick Survey",
            description: "Fast capture, moderate quality",
            captureInterval: 60,
            motionSensitivity: 80,
            enableNightMode: true,
            jpegQuality: 12,
            frameSize: "XGA"
        )
    ]
    
    func applyPreset(_ preset: ConfigurationPreset) {
        captureInterval = preset.captureInterval
        motionSensitivity = preset.motionSensitivity
        enableNightMode = preset.enableNightMode
        jpegQuality = preset.jpegQuality
        frameSize = preset.frameSize
    }
    
    // MARK: - Validation
    
    func validateConfiguration() -> [String] {
        var errors: [String] = []
        
        if captureInterval < 1 || captureInterval > 3600 {
            errors.append("Capture interval must be between 1 and 3600 seconds")
        }
        
        if motionSensitivity < 0 || motionSensitivity > 100 {
            errors.append("Motion sensitivity must be between 0 and 100")
        }
        
        if jpegQuality < 1 || jpegQuality > 63 {
            errors.append("JPEG quality must be between 1 and 63")
        }
        
        let validFrameSizes = ["UXGA", "SXGA", "XGA", "SVGA", "VGA"]
        if !validFrameSizes.contains(frameSize) {
            errors.append("Invalid frame size")
        }
        
        return errors
    }
    
    // MARK: - Export/Import
    
    func exportConfiguration() -> Data? {
        let config: [String: Any] = [
            "capture_interval": captureInterval,
            "motion_sensitivity": motionSensitivity,
            "enable_night_mode": enableNightMode,
            "enable_cloud_upload": enableCloudUpload,
            "jpeg_quality": jpegQuality,
            "frame_size": frameSize,
            "export_date": ISO8601DateFormatter().string(from: Date())
        ]
        
        return try? JSONSerialization.data(withJSONObject: config, options: .prettyPrinted)
    }
    
    func importConfiguration(from data: Data) throws {
        guard let config = try JSONSerialization.jsonObject(with: data) as? [String: Any] else {
            throw ConfigurationError.invalidFormat
        }
        
        if let interval = config["capture_interval"] as? Int {
            captureInterval = interval
        }
        if let sensitivity = config["motion_sensitivity"] as? Int {
            motionSensitivity = sensitivity
        }
        if let nightMode = config["enable_night_mode"] as? Bool {
            enableNightMode = nightMode
        }
        if let cloudUpload = config["enable_cloud_upload"] as? Bool {
            enableCloudUpload = cloudUpload
        }
        if let quality = config["jpeg_quality"] as? Int {
            jpegQuality = quality
        }
        if let size = config["frame_size"] as? String {
            frameSize = size
        }
    }
    
    enum ConfigurationError: LocalizedError {
        case invalidFormat
        case validationFailed([String])
        
        var errorDescription: String? {
            switch self {
            case .invalidFormat:
                return "Invalid configuration format"
            case .validationFailed(let errors):
                return "Configuration validation failed: \(errors.joined(separator: ", "))"
            }
        }
    }
}

// MARK: - Frame Size Options
enum FrameSize: String, CaseIterable {
    case uxga = "UXGA"
    case sxga = "SXGA"
    case xga = "XGA"
    case svga = "SVGA"
    case vga = "VGA"
    
    var resolution: String {
        switch self {
        case .uxga: return "1600x1200"
        case .sxga: return "1280x1024"
        case .xga: return "1024x768"
        case .svga: return "800x600"
        case .vga: return "640x480"
        }
    }
    
    var displayName: String {
        return "\(rawValue) (\(resolution))"
    }
}
