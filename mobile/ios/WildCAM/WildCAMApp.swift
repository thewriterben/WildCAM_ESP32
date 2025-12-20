/**
 * WildCAM iOS Companion App
 * Main application entry point
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

import SwiftUI
import UserNotifications

@main
struct WildCAMApp: App {
    @UIApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    @StateObject private var appState = AppState.shared
    @StateObject private var notificationService = NotificationService.shared
    @StateObject private var configService = ConfigurationService.shared
    
    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(appState)
                .environmentObject(notificationService)
                .environmentObject(configService)
                .onAppear {
                    setupApp()
                }
        }
    }
    
    private func setupApp() {
        // Request notification permissions
        notificationService.requestAuthorization()
        
        // Load remote configuration
        Task {
            await configService.fetchConfiguration()
        }
    }
}

// MARK: - App Delegate
class AppDelegate: NSObject, UIApplicationDelegate, UNUserNotificationCenterDelegate {
    func application(
        _ application: UIApplication,
        didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]? = nil
    ) -> Bool {
        // Configure notification center
        UNUserNotificationCenter.current().delegate = self
        
        // Register for remote notifications
        application.registerForRemoteNotifications()
        
        return true
    }
    
    func application(
        _ application: UIApplication,
        didRegisterForRemoteNotificationsWithDeviceToken deviceToken: Data
    ) {
        let tokenString = deviceToken.map { String(format: "%02.2hhx", $0) }.joined()
        NotificationService.shared.registerDeviceToken(tokenString)
    }
    
    func application(
        _ application: UIApplication,
        didFailToRegisterForRemoteNotificationsWithError error: Error
    ) {
        print("Failed to register for remote notifications: \(error.localizedDescription)")
    }
    
    // MARK: - UNUserNotificationCenterDelegate
    
    func userNotificationCenter(
        _ center: UNUserNotificationCenter,
        willPresent notification: UNNotification,
        withCompletionHandler completionHandler: @escaping (UNNotificationPresentationOptions) -> Void
    ) {
        // Show notification banner even when app is in foreground
        completionHandler([.banner, .sound, .badge])
    }
    
    func userNotificationCenter(
        _ center: UNUserNotificationCenter,
        didReceive response: UNNotificationResponse,
        withCompletionHandler completionHandler: @escaping () -> Void
    ) {
        // Handle notification tap
        let userInfo = response.notification.request.content.userInfo
        NotificationService.shared.handleNotificationTap(userInfo: userInfo)
        completionHandler()
    }
}

// MARK: - App State
class AppState: ObservableObject {
    static let shared = AppState()
    
    @Published var isConnected = false
    @Published var selectedTab: Tab = .dashboard
    @Published var recentDetections: [Detection] = []
    @Published var devices: [Camera] = []
    @Published var isLoading = false
    @Published var errorMessage: String?
    
    enum Tab {
        case dashboard
        case liveFeed
        case detections
        case map
        case settings
    }
    
    private init() {}
    
    func loadInitialData() async {
        await MainActor.run {
            isLoading = true
        }
        
        do {
            let detections = try await APIService.shared.getDetections(limit: 20)
            let devices = try await APIService.shared.getCameras()
            
            await MainActor.run {
                self.recentDetections = detections
                self.devices = devices
                self.isLoading = false
            }
        } catch {
            await MainActor.run {
                self.errorMessage = error.localizedDescription
                self.isLoading = false
            }
        }
    }
}

// MARK: - Content View
struct ContentView: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var notificationService: NotificationService
    
    var body: some View {
        TabView(selection: $appState.selectedTab) {
            DashboardView()
                .tabItem {
                    Label("Dashboard", systemImage: "square.grid.2x2")
                }
                .tag(AppState.Tab.dashboard)
            
            LiveFeedView()
                .tabItem {
                    Label("Live Feed", systemImage: "video")
                }
                .tag(AppState.Tab.liveFeed)
            
            DetectionsView()
                .tabItem {
                    Label("Detections", systemImage: "camera")
                }
                .tag(AppState.Tab.detections)
            
            MapView()
                .tabItem {
                    Label("Map", systemImage: "map")
                }
                .tag(AppState.Tab.map)
            
            SettingsView()
                .tabItem {
                    Label("Settings", systemImage: "gear")
                }
                .tag(AppState.Tab.settings)
        }
        .accentColor(Color("PrimaryGreen"))
        .onAppear {
            Task {
                await appState.loadInitialData()
            }
        }
    }
}

// MARK: - Placeholder Views
struct DashboardView: View {
    @EnvironmentObject var appState: AppState
    
    var body: some View {
        NavigationView {
            ScrollView {
                VStack(spacing: 16) {
                    // Status cards
                    HStack(spacing: 16) {
                        StatusCard(title: "Cameras", value: "\(appState.devices.count)", icon: "camera")
                        StatusCard(title: "Detections", value: "\(appState.recentDetections.count)", icon: "photo")
                    }
                    .padding(.horizontal)
                    
                    // Recent detections
                    VStack(alignment: .leading) {
                        Text("Recent Detections")
                            .font(.headline)
                            .padding(.horizontal)
                        
                        ForEach(appState.recentDetections.prefix(5)) { detection in
                            DetectionCard(detection: detection)
                        }
                    }
                }
                .padding(.vertical)
            }
            .navigationTitle("🦌 WildCAM")
            .refreshable {
                await appState.loadInitialData()
            }
        }
    }
}

struct StatusCard: View {
    let title: String
    let value: String
    let icon: String
    
    var body: some View {
        VStack {
            Image(systemName: icon)
                .font(.title)
                .foregroundColor(Color("PrimaryGreen"))
            Text(value)
                .font(.title2)
                .fontWeight(.bold)
            Text(title)
                .font(.caption)
                .foregroundColor(.secondary)
        }
        .frame(maxWidth: .infinity)
        .padding()
        .background(Color(.systemBackground))
        .cornerRadius(12)
        .shadow(radius: 2)
    }
}

struct DetectionCard: View {
    let detection: Detection
    
    var body: some View {
        HStack {
            AsyncImage(url: URL(string: detection.imageUrl)) { image in
                image
                    .resizable()
                    .aspectRatio(contentMode: .fill)
            } placeholder: {
                Color.gray
            }
            .frame(width: 60, height: 60)
            .cornerRadius(8)
            
            VStack(alignment: .leading) {
                Text(detection.species)
                    .font(.headline)
                Text("Confidence: \(Int(detection.confidence * 100))%")
                    .font(.caption)
                    .foregroundColor(.secondary)
                Text(detection.timestamp, style: .relative)
                    .font(.caption2)
                    .foregroundColor(.secondary)
            }
            
            Spacer()
            
            if detection.verified {
                Image(systemName: "checkmark.circle.fill")
                    .foregroundColor(.green)
            }
        }
        .padding()
        .background(Color(.systemBackground))
        .cornerRadius(12)
        .padding(.horizontal)
    }
}

struct LiveFeedView: View {
    var body: some View {
        NavigationView {
            Text("Live Feed")
                .navigationTitle("Live Camera Feed")
        }
    }
}

struct DetectionsView: View {
    var body: some View {
        NavigationView {
            Text("Detections List")
                .navigationTitle("Wildlife Detections")
        }
    }
}

struct MapView: View {
    var body: some View {
        NavigationView {
            Text("Camera Map")
                .navigationTitle("Camera Locations")
        }
    }
}

struct SettingsView: View {
    @EnvironmentObject var notificationService: NotificationService
    @EnvironmentObject var configService: ConfigurationService
    
    var body: some View {
        NavigationView {
            Form {
                Section("Notifications") {
                    Toggle("Wildlife Alerts", isOn: $notificationService.wildlifeAlertsEnabled)
                    Toggle("Device Status", isOn: $notificationService.deviceStatusEnabled)
                    Toggle("Low Battery Warnings", isOn: $notificationService.lowBatteryEnabled)
                }
                
                Section("Configuration") {
                    NavigationLink("Camera Settings") {
                        CameraSettingsView()
                    }
                    NavigationLink("Detection Settings") {
                        DetectionSettingsView()
                    }
                }
                
                Section("About") {
                    HStack {
                        Text("Version")
                        Spacer()
                        Text("1.0.0")
                            .foregroundColor(.secondary)
                    }
                }
            }
            .navigationTitle("Settings")
        }
    }
}

struct CameraSettingsView: View {
    var body: some View {
        Text("Camera Settings")
    }
}

struct DetectionSettingsView: View {
    var body: some View {
        Text("Detection Settings")
    }
}
