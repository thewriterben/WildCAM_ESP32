#include "WebServer.h"
#include "StorageManager.h"
#include "CameraManager.h"
#include "PowerManager.h"
#include "config.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP.h>
#include <WiFi.h>
#include <SD_MMC.h>

// Store constant strings in PROGMEM to save RAM
static const char TAG_ERROR_INIT[] PROGMEM = "ERROR: WebServer init failed - null reference(s) provided";
static const char TAG_SUCCESS_INIT[] PROGMEM = "WebServer initialized successfully";
static const char TAG_SERVER_HEADER[] PROGMEM = "==============================";
static const char TAG_SERVER_START[] PROGMEM = "Web Server Started";
static const char TAG_SERVER_URL[] PROGMEM = "Server URL: http://";
static const char JSON_ERROR_NOT_FOUND[] PROGMEM = "{\"error\":\"Not found\"}";
static const char JSON_ERROR_STORAGE[] PROGMEM = "{\"error\":\"Storage not available\"}";
static const char JSON_ERROR_NO_IMAGES[] PROGMEM = "{\"error\":\"No images found\"}";
static const char JSON_ERROR_CAMERA[] PROGMEM = "{\"error\":\"Camera not initialized\"}";
static const char JSON_ERROR_CAPTURE[] PROGMEM = "{\"error\":\"Capture failed\"}";
static const char JSON_ERROR_SAVE[] PROGMEM = "{\"error\":\"Failed to save image\"}";
static const char JSON_REBOOT[] PROGMEM = "{\"message\":\"Rebooting...\"}";

WebServer::WebServer(int serverPort)
    : port(serverPort), storage(nullptr), camera(nullptr), power(nullptr),
      captureInterval(30), motionSensitivity(MOTION_SENSITIVITY), 
      nightMode(false), cloudUpload(false) {
    server = new AsyncWebServer(port);
}

WebServer::~WebServer() {
    if (server) {
        delete server;
        server = nullptr;
    }
}

bool WebServer::init(StorageManager* storageRef, CameraManager* cameraRef, PowerManager* powerRef) {
    storage = storageRef;
    camera = cameraRef;
    power = powerRef;
    
    // Validate all references are not null
    if (!storage || !camera || !power) {
        Serial.println(FPSTR(TAG_ERROR_INIT));
        return false;
    }
    
    Serial.println(FPSTR(TAG_SUCCESS_INIT));
    return true;
}

void WebServer::begin() {
    // Enable CORS for all routes
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Serve main pages
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleIndex(request);
    });
    
    server->on("/gallery", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGallery(request);
    });
    
    server->on("/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleConfig(request);
    });
    
    // Serve CSS
    server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/css", WebServer::getStyleCSS());
    });
    
    // Setup RESTful API endpoints
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    // Legacy endpoint for backward compatibility
    server->on("/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleStatus(request);
    });
    
    server->on("/latest", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLatestImage(request);
    });
    
    server->on("/api/images", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleImagesList(request);
    });
    
    server->on("/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleCapture(request);
    });
    
    server->on("/api/capture", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleCapture(request);
    });
    
    server->on("/reboot", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleReboot(request);
    });
    
    server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGetConfig(request);
    });
    
    // Handle POST config with body
    server->on("/api/config", HTTP_POST, 
        [](AsyncWebServerRequest* request) {},
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            handlePostConfig(request, data, len);
        }
    );
    
    // Serve images from SD card
    server->on("^\\/images\\/(.+)$", HTTP_GET, [this](AsyncWebServerRequest* request) {
        String imagePath = "/" + request->pathArg(0);
        handleImage(request, imagePath);
    });
    
    // Serve thumbnails
    server->on("^\\/thumbnail\\/(.+)$", HTTP_GET, [this](AsyncWebServerRequest* request) {
        String imagePath = "/" + request->pathArg(0);
        handleThumbnail(request, imagePath);
    });
    
    // Handle 404
    server->onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json", FPSTR(JSON_ERROR_NOT_FOUND));
    });
    
    server->begin();
    
    // Print server URL
    Serial.println(FPSTR(TAG_SERVER_HEADER));
    Serial.println(FPSTR(TAG_SERVER_START));
    Serial.print(FPSTR(TAG_SERVER_URL));
    Serial.print(WiFi.localIP().toString());
    Serial.print(":");
    Serial.println(port);
    Serial.println(FPSTR(TAG_SERVER_HEADER));
}

// Page handlers
void WebServer::handleIndex(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getIndexHTML());
}

void WebServer::handleGallery(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getGalleryHTML());
}

void WebServer::handleConfig(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getConfigHTML());
}

void WebServer::handleStatus(AsyncWebServerRequest* request) {
    // Use StaticJsonDocument with adequate size for status data
    // JSON keys: 'uptime', 'freeHeap', 'batteryVoltage', 'batteryPercentage',
    // 'sdCardFreeSpace', 'sdCardUsedSpace', 'imageCount' (~100 bytes)
    // Plus values (up to 8 bytes each) and JSON overhead (~50%)
    // Total: ~300 bytes, using 512 bytes for safety margin
    StaticJsonDocument<512> doc;
    
    // System status
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    
    // Power status
    if (power) {
        doc["batteryVoltage"] = power->getBatteryVoltage();
        doc["batteryPercentage"] = power->getBatteryPercentage();
    } else {
        doc["batteryVoltage"] = 0.0;
        doc["batteryPercentage"] = 0;
    }
    
    // Storage status
    if (storage) {
        doc["sdCardFreeSpace"] = storage->getFreeSpace();
        doc["sdCardUsedSpace"] = storage->getUsedSpace();
        doc["imageCount"] = storage->getImageCount();
    } else {
        doc["sdCardFreeSpace"] = 0;
        doc["sdCardUsedSpace"] = 0;
        doc["imageCount"] = 0;
    }
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleLatestImage(AsyncWebServerRequest* request) {
    if (!storage) {
        request->send(503, "application/json", FPSTR(JSON_ERROR_STORAGE));
        return;
    }
    
    // Get list of image files from SD card
    std::vector<String> imageFiles = storage->getImageFiles();
    
    // Check if any images exist
    if (imageFiles.empty()) {
        request->send(404, "application/json", FPSTR(JSON_ERROR_NO_IMAGES));
        return;
    }
    
    // Get the most recent image (first in the sorted list)
    String latestImagePath = imageFiles[0];
    
    // Send file with image/jpeg content type
    request->send(SD_MMC, latestImagePath, "image/jpeg");
}

void WebServer::handleCapture(AsyncWebServerRequest* request) {
    if (!camera || !camera->isInitialized()) {
        request->send(503, "application/json", FPSTR(JSON_ERROR_CAMERA));
        return;
    }
    
    // Trigger immediate image capture
    camera_fb_t* fb = camera->captureImage();
    if (!fb) {
        request->send(500, "application/json", FPSTR(JSON_ERROR_CAPTURE));
        return;
    }
    
    String imagePath;
    size_t imageSize = fb->len;
    
    // Save image to SD card
    if (storage) {
        imagePath = storage->saveImage(fb);
        if (imagePath.length() == 0) {
            camera->releaseFrameBuffer(fb);
            request->send(500, "application/json", FPSTR(JSON_ERROR_SAVE));
            return;
        }
    }
    
    camera->releaseFrameBuffer(fb);
    
    // Return JSON with image path and size - use StaticJsonDocument with adequate size
    // JSON keys: 'success', 'path', 'size' (~20 bytes)
    // Image path can be long (e.g., '/images/2024/01/15/IMG_20240115_143022.jpg' = ~45 bytes)
    // Plus values and JSON overhead (~50%)
    // Total: ~130 bytes, using 256 bytes for safety margin
    StaticJsonDocument<256> doc;
    doc["success"] = true;
    doc["path"] = imagePath;
    doc["size"] = imageSize;
    
    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void WebServer::handleReboot(AsyncWebServerRequest* request) {
    // Send confirmation response
    request->send(200, "application/json", FPSTR(JSON_REBOOT));
    
    // Delay 1 second
    delay(1000);
    
    // Call ESP.restart()
    ESP.restart();
}

void WebServer::handleImagesList(AsyncWebServerRequest* request) {
    if (!storage) {
        request->send(503, "application/json", FPSTR(JSON_ERROR_STORAGE));
        return;
    }
    
    // Get pagination parameters
    int page = 1;
    int perPage = 20;
    if (request->hasParam("page")) {
        page = request->getParam("page")->value().toInt();
        if (page < 1) page = 1;
    }
    if (request->hasParam("perPage")) {
        perPage = request->getParam("perPage")->value().toInt();
        if (perPage < 1) perPage = 1;
        if (perPage > 100) perPage = 100;
    }
    
    std::vector<String> imageFiles = storage->getImageFiles();
    
    // Calculate pagination
    int totalImages = imageFiles.size();
    int totalPages = (totalImages + perPage - 1) / perPage;
    int startIndex = (page - 1) * perPage;
    int endIndex = startIndex + perPage;
    if (endIndex > totalImages) endIndex = totalImages;
    
    // Build JSON response
    StaticJsonDocument<2048> doc;
    doc["total"] = totalImages;
    doc["page"] = page;
    doc["perPage"] = perPage;
    doc["totalPages"] = totalPages;
    
    JsonArray images = doc.createNestedArray("images");
    for (int i = startIndex; i < endIndex && i < (int)imageFiles.size(); i++) {
        JsonObject img = images.createNestedObject();
        img["path"] = imageFiles[i];
        img["thumbnail"] = String("/thumbnail") + imageFiles[i];
        // Extract filename from path
        int lastSlash = imageFiles[i].lastIndexOf('/');
        img["name"] = (lastSlash >= 0) ? imageFiles[i].substring(lastSlash + 1) : imageFiles[i];
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServer::handleImage(AsyncWebServerRequest* request, const String& imagePath) {
    if (!storage) {
        request->send(503, "application/json", FPSTR(JSON_ERROR_STORAGE));
        return;
    }
    
    String fullPath = "/images" + imagePath;
    if (SD_MMC.exists(fullPath)) {
        request->send(SD_MMC, fullPath, "image/jpeg");
    } else {
        request->send(404, "application/json", FPSTR(JSON_ERROR_NOT_FOUND));
    }
}

void WebServer::handleThumbnail(AsyncWebServerRequest* request, const String& imagePath) {
    // For ESP32-CAM, we serve the original image as thumbnail
    // A more advanced implementation could generate actual thumbnails
    // or serve smaller versions stored separately
    handleImage(request, imagePath);
}

void WebServer::handleGetConfig(AsyncWebServerRequest* request) {
    StaticJsonDocument<512> doc;
    doc["captureInterval"] = captureInterval;
    doc["motionSensitivity"] = motionSensitivity;
    doc["nightMode"] = nightMode;
    doc["cloudUpload"] = cloudUpload;
    doc["firmwareVersion"] = FIRMWARE_VERSION;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebServer::handlePostConfig(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    // Update configuration values
    if (doc.containsKey("captureInterval")) {
        captureInterval = doc["captureInterval"].as<int>();
        if (captureInterval < 1) captureInterval = 1;
        if (captureInterval > 3600) captureInterval = 3600;
    }
    if (doc.containsKey("motionSensitivity")) {
        motionSensitivity = doc["motionSensitivity"].as<int>();
        if (motionSensitivity < 0) motionSensitivity = 0;
        if (motionSensitivity > 100) motionSensitivity = 100;
    }
    if (doc.containsKey("nightMode")) {
        nightMode = doc["nightMode"].as<bool>();
    }
    if (doc.containsKey("cloudUpload")) {
        cloudUpload = doc["cloudUpload"].as<bool>();
    }
    
    // Return updated configuration
    handleGetConfig(request);
}

// Static HTML content - Mobile responsive dashboard
const char* WebServer::getIndexHTML() {
    return R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WildCAM ESP32 - Dashboard</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-brand">🦌 WildCAM ESP32</div>
        <div class="nav-links">
            <a href="/" class="active">Dashboard</a>
            <a href="/gallery">Gallery</a>
            <a href="/config">Settings</a>
        </div>
    </nav>
    
    <main class="container">
        <h1>Real-Time Dashboard</h1>
        
        <div class="stats-grid">
            <div class="stat-card">
                <div class="stat-icon">⏱️</div>
                <div class="stat-value" id="uptime">--</div>
                <div class="stat-label">Uptime</div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">🔋</div>
                <div class="stat-value" id="battery">--%</div>
                <div class="stat-label">Battery</div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">💾</div>
                <div class="stat-value" id="storage">-- MB</div>
                <div class="stat-label">Free Storage</div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">📷</div>
                <div class="stat-value" id="imageCount">--</div>
                <div class="stat-label">Images</div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">🧠</div>
                <div class="stat-value" id="freeHeap">-- KB</div>
                <div class="stat-label">Free Memory</div>
            </div>
            <div class="stat-card">
                <div class="stat-icon">⚡</div>
                <div class="stat-value" id="voltage">-- V</div>
                <div class="stat-label">Voltage</div>
            </div>
        </div>
        
        <div class="action-section">
            <h2>Quick Actions</h2>
            <div class="action-buttons">
                <button class="btn btn-primary" onclick="captureImage()">📸 Capture Now</button>
                <button class="btn btn-secondary" onclick="refreshStats()">🔄 Refresh</button>
                <button class="btn btn-danger" onclick="rebootDevice()">🔌 Reboot</button>
            </div>
        </div>
        
        <div class="latest-image-section">
            <h2>Latest Capture</h2>
            <div class="image-container">
                <img id="latestImage" src="/latest" alt="Latest capture" onerror="this.style.display='none'">
                <p id="noImage" style="display:none">No images captured yet</p>
            </div>
        </div>
    </main>
    
    <script>
        function formatUptime(ms) {
            const seconds = Math.floor(ms / 1000);
            const minutes = Math.floor(seconds / 60);
            const hours = Math.floor(minutes / 60);
            const days = Math.floor(hours / 24);
            
            if (days > 0) return days + 'd ' + (hours % 24) + 'h';
            if (hours > 0) return hours + 'h ' + (minutes % 60) + 'm';
            if (minutes > 0) return minutes + 'm ' + (seconds % 60) + 's';
            return seconds + 's';
        }
        
        function formatBytes(bytes) {
            if (bytes === 0) return '0 B';
            const k = 1024;
            const sizes = ['B', 'KB', 'MB', 'GB'];
            const i = Math.floor(Math.log(bytes) / Math.log(k));
            return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
        }
        
        async function refreshStats() {
            try {
                const response = await fetch('/api/status');
                const data = await response.json();
                
                document.getElementById('uptime').textContent = formatUptime(data.uptime);
                document.getElementById('battery').textContent = data.batteryPercentage + '%';
                document.getElementById('storage').textContent = formatBytes(data.sdCardFreeSpace);
                document.getElementById('imageCount').textContent = data.imageCount;
                document.getElementById('freeHeap').textContent = formatBytes(data.freeHeap);
                document.getElementById('voltage').textContent = data.batteryVoltage.toFixed(2) + ' V';
            } catch (error) {
                console.error('Failed to fetch status:', error);
            }
        }
        
        async function captureImage() {
            try {
                const btn = event.target;
                btn.disabled = true;
                btn.textContent = '⏳ Capturing...';
                
                const response = await fetch('/api/capture', { method: 'POST' });
                const data = await response.json();
                
                if (data.success) {
                    alert('Image captured: ' + data.path);
                    document.getElementById('latestImage').src = '/latest?' + Date.now();
                    document.getElementById('latestImage').style.display = 'block';
                    document.getElementById('noImage').style.display = 'none';
                    refreshStats();
                } else {
                    alert('Capture failed');
                }
            } catch (error) {
                alert('Error: ' + error.message);
            } finally {
                const btn = event.target;
                btn.disabled = false;
                btn.textContent = '📸 Capture Now';
            }
        }
        
        async function rebootDevice() {
            if (confirm('Are you sure you want to reboot the device?')) {
                try {
                    await fetch('/reboot', { method: 'POST' });
                    alert('Device is rebooting...');
                } catch (error) {
                    console.error('Reboot request sent');
                }
            }
        }
        
        // Initial load and auto-refresh
        refreshStats();
        setInterval(refreshStats, 5000);
    </script>
</body>
</html>)rawliteral";
}

// Gallery page HTML
const char* WebServer::getGalleryHTML() {
    return R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WildCAM ESP32 - Gallery</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-brand">🦌 WildCAM ESP32</div>
        <div class="nav-links">
            <a href="/">Dashboard</a>
            <a href="/gallery" class="active">Gallery</a>
            <a href="/config">Settings</a>
        </div>
    </nav>
    
    <main class="container">
        <h1>Image Gallery</h1>
        
        <div class="gallery-controls">
            <button class="btn btn-primary" onclick="captureImage()">📸 Capture New</button>
            <button class="btn btn-secondary" onclick="loadImages()">🔄 Refresh</button>
        </div>
        
        <div class="gallery-grid" id="galleryGrid">
            <p class="loading">Loading images...</p>
        </div>
        
        <div class="pagination" id="pagination"></div>
        
        <!-- Image Modal -->
        <div class="modal" id="imageModal" onclick="closeModal()">
            <span class="modal-close">&times;</span>
            <img class="modal-content" id="modalImage">
            <div class="modal-caption" id="modalCaption"></div>
        </div>
    </main>
    
    <script>
        let currentPage = 1;
        const perPage = 12;
        
        async function loadImages(page = 1) {
            currentPage = page;
            const grid = document.getElementById('galleryGrid');
            grid.innerHTML = '<p class="loading">Loading images...</p>';
            
            try {
                const response = await fetch('/api/images?page=' + page + '&perPage=' + perPage);
                const data = await response.json();
                
                if (data.images.length === 0) {
                    grid.innerHTML = '<p class="no-images">No images found. Capture some wildlife!</p>';
                    document.getElementById('pagination').innerHTML = '';
                    return;
                }
                
                grid.innerHTML = '';
                data.images.forEach(img => {
                    const card = document.createElement('div');
                    card.className = 'gallery-card';
                    card.innerHTML = '<img src="' + img.thumbnail + '" alt="' + img.name + '" onclick="openModal(\'' + img.path + '\', \'' + img.name + '\')"><div class="gallery-card-title">' + img.name + '</div>';
                    grid.appendChild(card);
                });
                
                renderPagination(data.page, data.totalPages);
            } catch (error) {
                grid.innerHTML = '<p class="error">Failed to load images: ' + error.message + '</p>';
            }
        }
        
        function renderPagination(current, total) {
            const pagination = document.getElementById('pagination');
            if (total <= 1) {
                pagination.innerHTML = '';
                return;
            }
            
            let html = '';
            if (current > 1) {
                html += '<button class="btn btn-small" onclick="loadImages(' + (current - 1) + ')">← Prev</button>';
            }
            html += '<span class="page-info">Page ' + current + ' of ' + total + '</span>';
            if (current < total) {
                html += '<button class="btn btn-small" onclick="loadImages(' + (current + 1) + ')">Next →</button>';
            }
            pagination.innerHTML = html;
        }
        
        function openModal(path, name) {
            document.getElementById('modalImage').src = '/images' + path;
            document.getElementById('modalCaption').textContent = name;
            document.getElementById('imageModal').style.display = 'flex';
        }
        
        function closeModal() {
            document.getElementById('imageModal').style.display = 'none';
        }
        
        async function captureImage() {
            try {
                const response = await fetch('/api/capture', { method: 'POST' });
                const data = await response.json();
                if (data.success) {
                    alert('Image captured!');
                    loadImages(1);
                }
            } catch (error) {
                alert('Capture failed: ' + error.message);
            }
        }
        
        // Close modal on escape key
        document.addEventListener('keydown', (e) => {
            if (e.key === 'Escape') closeModal();
        });
        
        // Initial load
        loadImages();
    </script>
</body>
</html>)rawliteral";
}

// Configuration page HTML
const char* WebServer::getConfigHTML() {
    return R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WildCAM ESP32 - Settings</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <nav class="navbar">
        <div class="nav-brand">🦌 WildCAM ESP32</div>
        <div class="nav-links">
            <a href="/">Dashboard</a>
            <a href="/gallery">Gallery</a>
            <a href="/config" class="active">Settings</a>
        </div>
    </nav>
    
    <main class="container">
        <h1>Device Configuration</h1>
        
        <form id="configForm" class="config-form">
            <div class="form-group">
                <label for="captureInterval">Capture Interval (seconds)</label>
                <input type="number" id="captureInterval" name="captureInterval" min="1" max="3600" value="30">
                <small>Time between automatic captures (1-3600 seconds)</small>
            </div>
            
            <div class="form-group">
                <label for="motionSensitivity">Motion Sensitivity</label>
                <input type="range" id="motionSensitivity" name="motionSensitivity" min="0" max="100" value="50">
                <span id="sensitivityValue">50</span>%
                <small>Higher values are more sensitive to motion</small>
            </div>
            
            <div class="form-group">
                <label class="checkbox-label">
                    <input type="checkbox" id="nightMode" name="nightMode">
                    <span>Night Mode</span>
                </label>
                <small>Enable IR LEDs for night vision</small>
            </div>
            
            <div class="form-group">
                <label class="checkbox-label">
                    <input type="checkbox" id="cloudUpload" name="cloudUpload">
                    <span>Cloud Upload</span>
                </label>
                <small>Automatically upload images to cloud storage</small>
            </div>
            
            <div class="form-actions">
                <button type="submit" class="btn btn-primary">💾 Save Settings</button>
                <button type="button" class="btn btn-secondary" onclick="loadConfig()">🔄 Reset</button>
            </div>
        </form>
        
        <div class="device-info">
            <h2>Device Information</h2>
            <table class="info-table">
                <tr><td>Firmware Version</td><td id="firmwareVersion">--</td></tr>
                <tr><td>IP Address</td><td id="ipAddress">--</td></tr>
            </table>
        </div>
    </main>
    
    <script>
        const sensitivitySlider = document.getElementById('motionSensitivity');
        const sensitivityValue = document.getElementById('sensitivityValue');
        
        sensitivitySlider.addEventListener('input', () => {
            sensitivityValue.textContent = sensitivitySlider.value;
        });
        
        async function loadConfig() {
            try {
                const response = await fetch('/api/config');
                const data = await response.json();
                
                document.getElementById('captureInterval').value = data.captureInterval;
                document.getElementById('motionSensitivity').value = data.motionSensitivity;
                sensitivityValue.textContent = data.motionSensitivity;
                document.getElementById('nightMode').checked = data.nightMode;
                document.getElementById('cloudUpload').checked = data.cloudUpload;
                document.getElementById('firmwareVersion').textContent = data.firmwareVersion;
                document.getElementById('ipAddress').textContent = window.location.host;
            } catch (error) {
                console.error('Failed to load config:', error);
            }
        }
        
        document.getElementById('configForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            
            const config = {
                captureInterval: parseInt(document.getElementById('captureInterval').value),
                motionSensitivity: parseInt(document.getElementById('motionSensitivity').value),
                nightMode: document.getElementById('nightMode').checked,
                cloudUpload: document.getElementById('cloudUpload').checked
            };
            
            try {
                const response = await fetch('/api/config', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(config)
                });
                
                if (response.ok) {
                    alert('Settings saved successfully!');
                } else {
                    alert('Failed to save settings');
                }
            } catch (error) {
                alert('Error: ' + error.message);
            }
        });
        
        // Load config on page load
        loadConfig();
    </script>
</body>
</html>)rawliteral";
}

// CSS Styles - Mobile responsive
const char* WebServer::getStyleCSS() {
    return R"rawliteral(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
    background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
    color: #e0e0e0;
    min-height: 100vh;
}

.navbar {
    background: rgba(0, 0, 0, 0.3);
    padding: 1rem;
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 1rem;
}

.nav-brand {
    font-size: 1.5rem;
    font-weight: bold;
    color: #4ade80;
}

.nav-links {
    display: flex;
    gap: 1rem;
    flex-wrap: wrap;
}

.nav-links a {
    color: #e0e0e0;
    text-decoration: none;
    padding: 0.5rem 1rem;
    border-radius: 0.5rem;
    transition: background 0.3s;
}

.nav-links a:hover, .nav-links a.active {
    background: rgba(74, 222, 128, 0.2);
    color: #4ade80;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 2rem 1rem;
}

h1 {
    margin-bottom: 2rem;
    color: #4ade80;
    font-size: 2rem;
}

h2 {
    margin: 1.5rem 0 1rem;
    color: #4ade80;
    font-size: 1.5rem;
}

/* Stats Grid */
.stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
    gap: 1rem;
    margin-bottom: 2rem;
}

.stat-card {
    background: rgba(255, 255, 255, 0.05);
    border-radius: 1rem;
    padding: 1.5rem;
    text-align: center;
    border: 1px solid rgba(255, 255, 255, 0.1);
    transition: transform 0.3s, box-shadow 0.3s;
}

.stat-card:hover {
    transform: translateY(-5px);
    box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
}

.stat-icon {
    font-size: 2rem;
    margin-bottom: 0.5rem;
}

.stat-value {
    font-size: 1.5rem;
    font-weight: bold;
    color: #4ade80;
}

.stat-label {
    font-size: 0.875rem;
    color: #888;
    margin-top: 0.25rem;
}

/* Buttons */
.btn {
    padding: 0.75rem 1.5rem;
    border: none;
    border-radius: 0.5rem;
    cursor: pointer;
    font-size: 1rem;
    transition: transform 0.2s, box-shadow 0.2s;
}

.btn:hover {
    transform: translateY(-2px);
    box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
}

.btn:disabled {
    opacity: 0.5;
    cursor: not-allowed;
    transform: none;
}

.btn-primary {
    background: #4ade80;
    color: #1a1a2e;
}

.btn-secondary {
    background: #64748b;
    color: white;
}

.btn-danger {
    background: #ef4444;
    color: white;
}

.btn-small {
    padding: 0.5rem 1rem;
    font-size: 0.875rem;
}

.action-buttons {
    display: flex;
    gap: 1rem;
    flex-wrap: wrap;
}

/* Latest Image Section */
.latest-image-section {
    margin-top: 2rem;
}

.image-container {
    background: rgba(255, 255, 255, 0.05);
    border-radius: 1rem;
    padding: 1rem;
    text-align: center;
}

.image-container img {
    max-width: 100%;
    max-height: 400px;
    border-radius: 0.5rem;
}

/* Gallery */
.gallery-controls {
    margin-bottom: 1.5rem;
    display: flex;
    gap: 1rem;
    flex-wrap: wrap;
}

.gallery-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
    gap: 1rem;
}

.gallery-card {
    background: rgba(255, 255, 255, 0.05);
    border-radius: 0.5rem;
    overflow: hidden;
    cursor: pointer;
    transition: transform 0.3s;
}

.gallery-card:hover {
    transform: scale(1.05);
}

.gallery-card img {
    width: 100%;
    height: 150px;
    object-fit: cover;
}

.gallery-card-title {
    padding: 0.5rem;
    font-size: 0.75rem;
    color: #888;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
}

.pagination {
    margin-top: 2rem;
    display: flex;
    justify-content: center;
    align-items: center;
    gap: 1rem;
}

.page-info {
    color: #888;
}

/* Modal */
.modal {
    display: none;
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0, 0, 0, 0.9);
    justify-content: center;
    align-items: center;
    flex-direction: column;
    z-index: 1000;
}

.modal-close {
    position: absolute;
    top: 1rem;
    right: 2rem;
    color: white;
    font-size: 2rem;
    cursor: pointer;
}

.modal-content {
    max-width: 90%;
    max-height: 80%;
    border-radius: 0.5rem;
}

.modal-caption {
    color: #888;
    margin-top: 1rem;
}

/* Config Form */
.config-form {
    background: rgba(255, 255, 255, 0.05);
    border-radius: 1rem;
    padding: 2rem;
}

.form-group {
    margin-bottom: 1.5rem;
}

.form-group label {
    display: block;
    margin-bottom: 0.5rem;
    font-weight: 500;
}

.form-group input[type="number"],
.form-group input[type="text"] {
    width: 100%;
    padding: 0.75rem;
    border: 1px solid rgba(255, 255, 255, 0.2);
    border-radius: 0.5rem;
    background: rgba(0, 0, 0, 0.3);
    color: white;
    font-size: 1rem;
}

.form-group input[type="range"] {
    width: calc(100% - 4rem);
}

.form-group small {
    display: block;
    margin-top: 0.25rem;
    color: #888;
    font-size: 0.875rem;
}

.checkbox-label {
    display: flex;
    align-items: center;
    gap: 0.5rem;
    cursor: pointer;
}

.checkbox-label input[type="checkbox"] {
    width: 1.25rem;
    height: 1.25rem;
}

.form-actions {
    display: flex;
    gap: 1rem;
    margin-top: 2rem;
}

/* Device Info */
.device-info {
    margin-top: 2rem;
}

.info-table {
    width: 100%;
    border-collapse: collapse;
    background: rgba(255, 255, 255, 0.05);
    border-radius: 0.5rem;
    overflow: hidden;
}

.info-table td {
    padding: 1rem;
    border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.info-table td:first-child {
    color: #888;
    width: 50%;
}

/* Loading & Messages */
.loading, .no-images, .error {
    text-align: center;
    padding: 2rem;
    color: #888;
}

.error {
    color: #ef4444;
}

/* Mobile Responsive */
@media (max-width: 768px) {
    .navbar {
        flex-direction: column;
        text-align: center;
    }
    
    .nav-links {
        justify-content: center;
    }
    
    h1 {
        font-size: 1.5rem;
    }
    
    .stats-grid {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .action-buttons {
        flex-direction: column;
    }
    
    .btn {
        width: 100%;
    }
    
    .gallery-grid {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .form-actions {
        flex-direction: column;
    }
}

@media (max-width: 480px) {
    .stats-grid {
        grid-template-columns: 1fr;
    }
    
    .gallery-grid {
        grid-template-columns: 1fr;
    }
}
)rawliteral";
}
