/**
 * WildCAM iOS - WebSocket Service
 * Real-time communication with backend server
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

import Foundation
import Combine

// MARK: - WebSocket Message Types
enum WebSocketMessageType: String, Codable {
    case connect = "connect"
    case disconnect = "disconnect"
    case newDetection = "new_detection"
    case deviceStatusUpdate = "device_status_update"
    case subscribeDevice = "subscribe_device"
    case unsubscribeDevice = "unsubscribe_device"
    case ping = "ping"
    case pong = "pong"
    case error = "error"
}

struct WebSocketMessage: Codable {
    let type: WebSocketMessageType
    let data: [String: AnyCodable]?
    let timestamp: Date?
}

// MARK: - WebSocket Service
class WebSocketService: ObservableObject {
    static let shared = WebSocketService()
    
    @Published var isConnected = false
    @Published var lastError: String?
    
    // Event publishers
    let detectionSubject = PassthroughSubject<Detection, Never>()
    let deviceStatusSubject = PassthroughSubject<DeviceStatus, Never>()
    let connectionSubject = PassthroughSubject<Bool, Never>()
    
    private var webSocketTask: URLSessionWebSocketTask?
    private var session: URLSession
    private var pingTimer: Timer?
    private var reconnectTimer: Timer?
    private var subscribedDevices: Set<String> = []
    
    private let pingInterval: TimeInterval = 30
    private let reconnectDelay: TimeInterval = 5
    private var reconnectAttempts = 0
    private let maxReconnectAttempts = 5
    
    private init() {
        let configuration = URLSessionConfiguration.default
        session = URLSession(configuration: configuration)
    }
    
    // MARK: - Connection Management
    
    func connect() {
        guard !isConnected else { return }
        
        let urlString = ConfigurationService.shared.websocketURL
        guard let url = URL(string: urlString) else {
            lastError = "Invalid WebSocket URL"
            return
        }
        
        webSocketTask = session.webSocketTask(with: url)
        webSocketTask?.resume()
        
        receiveMessage()
        startPingTimer()
        
        DispatchQueue.main.async {
            self.isConnected = true
            self.reconnectAttempts = 0
            self.connectionSubject.send(true)
        }
        
        // Resubscribe to previously subscribed devices
        for deviceId in subscribedDevices {
            sendSubscribeDevice(deviceId: deviceId)
        }
    }
    
    func disconnect() {
        stopPingTimer()
        stopReconnectTimer()
        
        webSocketTask?.cancel(with: .goingAway, reason: nil)
        webSocketTask = nil
        
        DispatchQueue.main.async {
            self.isConnected = false
            self.connectionSubject.send(false)
        }
    }
    
    private func reconnect() {
        guard reconnectAttempts < maxReconnectAttempts else {
            lastError = "Max reconnection attempts reached"
            return
        }
        
        reconnectAttempts += 1
        
        reconnectTimer = Timer.scheduledTimer(withTimeInterval: reconnectDelay, repeats: false) { [weak self] _ in
            self?.connect()
        }
    }
    
    // MARK: - Message Handling
    
    private func receiveMessage() {
        webSocketTask?.receive { [weak self] result in
            switch result {
            case .success(let message):
                self?.handleMessage(message)
                self?.receiveMessage() // Continue receiving
                
            case .failure(let error):
                DispatchQueue.main.async {
                    self?.isConnected = false
                    self?.lastError = error.localizedDescription
                    self?.connectionSubject.send(false)
                }
                self?.reconnect()
            }
        }
    }
    
    private func handleMessage(_ message: URLSessionWebSocketTask.Message) {
        switch message {
        case .string(let text):
            parseMessage(text)
        case .data(let data):
            if let text = String(data: data, encoding: .utf8) {
                parseMessage(text)
            }
        @unknown default:
            break
        }
    }
    
    private func parseMessage(_ text: String) {
        guard let data = text.data(using: .utf8) else { return }
        
        do {
            let decoder = JSONDecoder()
            decoder.dateDecodingStrategy = .iso8601
            
            // Try to decode as generic message first
            if let json = try JSONSerialization.jsonObject(with: data) as? [String: Any],
               let typeString = json["type"] as? String,
               let messageType = WebSocketMessageType(rawValue: typeString) {
                
                switch messageType {
                case .newDetection:
                    if let detectionData = json["data"] as? [String: Any] {
                        let detectionJson = try JSONSerialization.data(withJSONObject: detectionData)
                        let detection = try decoder.decode(Detection.self, from: detectionJson)
                        DispatchQueue.main.async {
                            self.detectionSubject.send(detection)
                        }
                    }
                    
                case .deviceStatusUpdate:
                    if let statusData = json["data"] as? [String: Any] {
                        let statusJson = try JSONSerialization.data(withJSONObject: statusData)
                        let status = try decoder.decode(DeviceStatus.self, from: statusJson)
                        DispatchQueue.main.async {
                            self.deviceStatusSubject.send(status)
                        }
                    }
                    
                case .pong:
                    // Ping response received
                    break
                    
                case .error:
                    if let errorMessage = json["message"] as? String {
                        DispatchQueue.main.async {
                            self.lastError = errorMessage
                        }
                    }
                    
                default:
                    break
                }
            }
        } catch {
            print("Failed to parse WebSocket message: \(error.localizedDescription)")
        }
    }
    
    // MARK: - Send Messages
    
    func sendMessage(_ message: [String: Any]) {
        guard let data = try? JSONSerialization.data(withJSONObject: message),
              let text = String(data: data, encoding: .utf8) else {
            return
        }
        
        webSocketTask?.send(.string(text)) { [weak self] error in
            if let error = error {
                DispatchQueue.main.async {
                    self?.lastError = error.localizedDescription
                }
            }
        }
    }
    
    func subscribeToDevice(deviceId: String) {
        subscribedDevices.insert(deviceId)
        if isConnected {
            sendSubscribeDevice(deviceId: deviceId)
        }
    }
    
    func unsubscribeFromDevice(deviceId: String) {
        subscribedDevices.remove(deviceId)
        if isConnected {
            sendUnsubscribeDevice(deviceId: deviceId)
        }
    }
    
    private func sendSubscribeDevice(deviceId: String) {
        sendMessage([
            "type": "subscribe_device",
            "device_id": deviceId
        ])
    }
    
    private func sendUnsubscribeDevice(deviceId: String) {
        sendMessage([
            "type": "unsubscribe_device",
            "device_id": deviceId
        ])
    }
    
    // MARK: - Ping/Pong
    
    private func startPingTimer() {
        pingTimer = Timer.scheduledTimer(withTimeInterval: pingInterval, repeats: true) { [weak self] _ in
            self?.sendPing()
        }
    }
    
    private func stopPingTimer() {
        pingTimer?.invalidate()
        pingTimer = nil
    }
    
    private func stopReconnectTimer() {
        reconnectTimer?.invalidate()
        reconnectTimer = nil
    }
    
    private func sendPing() {
        webSocketTask?.sendPing { [weak self] error in
            if let error = error {
                DispatchQueue.main.async {
                    self?.isConnected = false
                    self?.lastError = error.localizedDescription
                }
                self?.reconnect()
            }
        }
    }
}

// MARK: - AnyCodable Helper
struct AnyCodable: Codable {
    let value: Any
    
    init(_ value: Any) {
        self.value = value
    }
    
    init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()
        
        if let bool = try? container.decode(Bool.self) {
            value = bool
        } else if let int = try? container.decode(Int.self) {
            value = int
        } else if let double = try? container.decode(Double.self) {
            value = double
        } else if let string = try? container.decode(String.self) {
            value = string
        } else if let array = try? container.decode([AnyCodable].self) {
            value = array.map { $0.value }
        } else if let dictionary = try? container.decode([String: AnyCodable].self) {
            value = dictionary.mapValues { $0.value }
        } else {
            value = NSNull()
        }
    }
    
    func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()
        
        switch value {
        case let bool as Bool:
            try container.encode(bool)
        case let int as Int:
            try container.encode(int)
        case let double as Double:
            try container.encode(double)
        case let string as String:
            try container.encode(string)
        case let array as [Any]:
            try container.encode(array.map { AnyCodable($0) })
        case let dictionary as [String: Any]:
            try container.encode(dictionary.mapValues { AnyCodable($0) })
        default:
            try container.encodeNil()
        }
    }
}
