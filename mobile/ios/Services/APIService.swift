/**
 * WildCAM iOS - API Service
 * Handles all API communication with backend server
 * 
 * @author WildCAM ESP32 Team
 * @version 1.0.0
 * @date 2025
 */

import Foundation

// MARK: - API Errors
enum APIError: LocalizedError {
    case invalidURL
    case invalidResponse
    case httpError(statusCode: Int, message: String?)
    case decodingError(Error)
    case networkError(Error)
    case unauthorized
    case serverError(String)
    
    var errorDescription: String? {
        switch self {
        case .invalidURL:
            return "Invalid URL"
        case .invalidResponse:
            return "Invalid response from server"
        case .httpError(let statusCode, let message):
            return message ?? "HTTP Error: \(statusCode)"
        case .decodingError(let error):
            return "Failed to decode response: \(error.localizedDescription)"
        case .networkError(let error):
            return "Network error: \(error.localizedDescription)"
        case .unauthorized:
            return "Unauthorized. Please check your credentials."
        case .serverError(let message):
            return "Server error: \(message)"
        }
    }
}

// MARK: - API Service
class APIService: ObservableObject {
    static let shared = APIService()
    
    private let baseURL: String
    private let session: URLSession
    private var authToken: String?
    
    private let jsonDecoder: JSONDecoder = {
        let decoder = JSONDecoder()
        decoder.dateDecodingStrategy = .iso8601
        return decoder
    }()
    
    private let jsonEncoder: JSONEncoder = {
        let encoder = JSONEncoder()
        encoder.dateEncodingStrategy = .iso8601
        return encoder
    }()
    
    private init() {
        // Load base URL from configuration
        self.baseURL = ConfigurationService.shared.apiBaseURL
        
        let configuration = URLSessionConfiguration.default
        configuration.timeoutIntervalForRequest = 30
        configuration.timeoutIntervalForResource = 60
        self.session = URLSession(configuration: configuration)
    }
    
    // MARK: - Authentication
    
    func setAuthToken(_ token: String?) {
        self.authToken = token
    }
    
    // MARK: - Cameras API
    
    func getCameras() async throws -> [Camera] {
        let url = try buildURL(path: "/api/v1/devices")
        let response: DevicesResponse = try await performRequest(url: url, method: "GET")
        return response.devices
    }
    
    func getCamera(deviceId: String) async throws -> Camera {
        let url = try buildURL(path: "/api/v1/devices/\(deviceId)")
        return try await performRequest(url: url, method: "GET")
    }
    
    func getCameraStatus(deviceId: String) async throws -> DeviceStatus {
        let url = try buildURL(path: "/api/v1/devices/\(deviceId)/status")
        return try await performRequest(url: url, method: "GET")
    }
    
    func updateCameraConfiguration(
        deviceId: String,
        configuration: [String: Any]
    ) async throws -> Bool {
        let url = try buildURL(path: "/api/v1/devices/\(deviceId)/config")
        let body = try JSONSerialization.data(withJSONObject: configuration)
        let _: APIResponse<EmptyResponse> = try await performRequest(url: url, method: "POST", body: body)
        return true
    }
    
    // MARK: - Detections API
    
    func getDetections(
        limit: Int = 20,
        page: Int = 1,
        startDate: Date? = nil,
        endDate: Date? = nil,
        deviceId: String? = nil,
        species: String? = nil
    ) async throws -> [Detection] {
        var queryItems: [URLQueryItem] = [
            URLQueryItem(name: "limit", value: String(limit)),
            URLQueryItem(name: "page", value: String(page))
        ]
        
        if let startDate = startDate {
            queryItems.append(URLQueryItem(name: "start", value: ISO8601DateFormatter().string(from: startDate)))
        }
        if let endDate = endDate {
            queryItems.append(URLQueryItem(name: "end", value: ISO8601DateFormatter().string(from: endDate)))
        }
        if let deviceId = deviceId {
            queryItems.append(URLQueryItem(name: "device_id", value: deviceId))
        }
        if let species = species {
            queryItems.append(URLQueryItem(name: "species", value: species))
        }
        
        let url = try buildURL(path: "/api/v1/detections", queryItems: queryItems)
        let response: DetectionsResponse = try await performRequest(url: url, method: "GET")
        return response.detections
    }
    
    func getDetection(detectionId: String) async throws -> Detection {
        let url = try buildURL(path: "/api/v1/detections/\(detectionId)")
        return try await performRequest(url: url, method: "GET")
    }
    
    func verifyDetection(
        detectionId: String,
        verification: VerificationData
    ) async throws -> Bool {
        let url = try buildURL(path: "/api/v1/detections/\(detectionId)/verify")
        let body = try jsonEncoder.encode(verification)
        let _: APIResponse<EmptyResponse> = try await performRequest(url: url, method: "POST", body: body)
        return true
    }
    
    // MARK: - Field Observations API
    
    func submitFieldObservation(
        observation: FieldObservation,
        image: Data?
    ) async throws -> Bool {
        let url = try buildURL(path: "/api/v1/observations")
        
        // Create multipart form data
        let boundary = "Boundary-\(UUID().uuidString)"
        var body = Data()
        
        // Add metadata
        let metadataJson = try jsonEncoder.encode(observation)
        body.appendMultipartFormData(name: "metadata", value: metadataJson, boundary: boundary)
        
        // Add image if provided
        if let image = image {
            body.appendMultipartFormData(
                name: "image",
                filename: "observation.jpg",
                mimeType: "image/jpeg",
                data: image,
                boundary: boundary
            )
        }
        
        body.append("--\(boundary)--\r\n".data(using: .utf8)!)
        
        let _: APIResponse<EmptyResponse> = try await performRequest(
            url: url,
            method: "POST",
            body: body,
            contentType: "multipart/form-data; boundary=\(boundary)"
        )
        return true
    }
    
    // MARK: - Configuration API
    
    func getRemoteConfiguration() async throws -> RemoteConfiguration {
        let url = try buildURL(path: "/api/v1/config")
        return try await performRequest(url: url, method: "GET")
    }
    
    func updateRemoteConfiguration(configuration: RemoteConfiguration) async throws -> Bool {
        let url = try buildURL(path: "/api/v1/config")
        let body = try jsonEncoder.encode(configuration)
        let _: APIResponse<EmptyResponse> = try await performRequest(url: url, method: "POST", body: body)
        return true
    }
    
    // MARK: - Push Notifications API
    
    func registerDeviceToken(token: String, platform: String = "ios") async throws -> Bool {
        let url = try buildURL(path: "/api/v1/push/register")
        let body: [String: Any] = [
            "token": token,
            "platform": platform,
            "device_id": UIDevice.current.identifierForVendor?.uuidString ?? ""
        ]
        let bodyData = try JSONSerialization.data(withJSONObject: body)
        let _: APIResponse<EmptyResponse> = try await performRequest(url: url, method: "POST", body: bodyData)
        return true
    }
    
    func updateNotificationPreferences(preferences: NotificationSettings) async throws -> Bool {
        let url = try buildURL(path: "/api/v1/push/preferences")
        let body = try jsonEncoder.encode(preferences)
        let _: APIResponse<EmptyResponse> = try await performRequest(url: url, method: "POST", body: body)
        return true
    }
    
    // MARK: - Analytics API
    
    func getSpeciesAnalytics(deviceId: String? = nil, days: Int = 7) async throws -> [String: Int] {
        var queryItems: [URLQueryItem] = [
            URLQueryItem(name: "days", value: String(days))
        ]
        
        if let deviceId = deviceId {
            queryItems.append(URLQueryItem(name: "device_id", value: deviceId))
        }
        
        let url = try buildURL(path: "/api/v1/analytics/species-count", queryItems: queryItems)
        let response: SpeciesAnalyticsResponse = try await performRequest(url: url, method: "GET")
        return response.speciesCounts
    }
    
    func getActivityPatterns(deviceId: String? = nil, days: Int = 7) async throws -> ActivityPatterns {
        var queryItems: [URLQueryItem] = [
            URLQueryItem(name: "days", value: String(days))
        ]
        
        if let deviceId = deviceId {
            queryItems.append(URLQueryItem(name: "device_id", value: deviceId))
        }
        
        let url = try buildURL(path: "/api/v1/analytics/activity-patterns", queryItems: queryItems)
        return try await performRequest(url: url, method: "GET")
    }
    
    // MARK: - Private Methods
    
    private func buildURL(path: String, queryItems: [URLQueryItem]? = nil) throws -> URL {
        guard var components = URLComponents(string: baseURL + path) else {
            throw APIError.invalidURL
        }
        components.queryItems = queryItems
        guard let url = components.url else {
            throw APIError.invalidURL
        }
        return url
    }
    
    private func performRequest<T: Decodable>(
        url: URL,
        method: String,
        body: Data? = nil,
        contentType: String = "application/json"
    ) async throws -> T {
        var request = URLRequest(url: url)
        request.httpMethod = method
        request.httpBody = body
        request.setValue(contentType, forHTTPHeaderField: "Content-Type")
        request.setValue("application/json", forHTTPHeaderField: "Accept")
        
        if let token = authToken {
            request.setValue("Bearer \(token)", forHTTPHeaderField: "Authorization")
        }
        
        do {
            let (data, response) = try await session.data(for: request)
            
            guard let httpResponse = response as? HTTPURLResponse else {
                throw APIError.invalidResponse
            }
            
            switch httpResponse.statusCode {
            case 200...299:
                do {
                    return try jsonDecoder.decode(T.self, from: data)
                } catch {
                    throw APIError.decodingError(error)
                }
            case 401:
                throw APIError.unauthorized
            case 400...499:
                let message = try? JSONDecoder().decode(ErrorResponse.self, from: data).message
                throw APIError.httpError(statusCode: httpResponse.statusCode, message: message)
            case 500...599:
                let message = try? JSONDecoder().decode(ErrorResponse.self, from: data).message
                throw APIError.serverError(message ?? "Internal server error")
            default:
                throw APIError.httpError(statusCode: httpResponse.statusCode, message: nil)
            }
        } catch let error as APIError {
            throw error
        } catch {
            throw APIError.networkError(error)
        }
    }
}

// MARK: - Helper Types

struct EmptyResponse: Codable {}

struct ErrorResponse: Codable {
    let status: String
    let message: String
}

struct SpeciesAnalyticsResponse: Codable {
    let speciesCounts: [String: Int]
    let periodDays: Int
    
    enum CodingKeys: String, CodingKey {
        case speciesCounts = "species_counts"
        case periodDays = "period_days"
    }
}

struct ActivityPatterns: Codable {
    let hourlyActivity: [Int: Int]
    let dailyActivity: [String: Int]
    let speciesActivity: [String: [Int: Int]]
    
    enum CodingKeys: String, CodingKey {
        case hourlyActivity = "hourly_activity"
        case dailyActivity = "daily_activity"
        case speciesActivity = "species_activity"
    }
}

// MARK: - Data Extensions

extension Data {
    mutating func appendMultipartFormData(
        name: String,
        value: Data,
        boundary: String
    ) {
        append("--\(boundary)\r\n".data(using: .utf8)!)
        append("Content-Disposition: form-data; name=\"\(name)\"\r\n".data(using: .utf8)!)
        append("Content-Type: application/json\r\n\r\n".data(using: .utf8)!)
        append(value)
        append("\r\n".data(using: .utf8)!)
    }
    
    mutating func appendMultipartFormData(
        name: String,
        filename: String,
        mimeType: String,
        data: Data,
        boundary: String
    ) {
        append("--\(boundary)\r\n".data(using: .utf8)!)
        append("Content-Disposition: form-data; name=\"\(name)\"; filename=\"\(filename)\"\r\n".data(using: .utf8)!)
        append("Content-Type: \(mimeType)\r\n\r\n".data(using: .utf8)!)
        append(data)
        append("\r\n".data(using: .utf8)!)
    }
}
