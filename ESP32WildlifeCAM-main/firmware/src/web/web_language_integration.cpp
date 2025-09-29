/**
 * @file web_language_integration.cpp
 * @brief Language Manager Integration with Web Server Implementation
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

#include "web_language_integration.h"
#include <esp_log.h>

static const char* TAG = "WebLanguageIntegration";

WebLanguageIntegration::WebLanguageIntegration(LanguageManager& languageManager)
    : languageManager_(languageManager) {
}

void WebLanguageIntegration::setupAPIEndpoints(AsyncWebServer& server) {
    // Get available languages
    server.on("/api/languages", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetLanguages(request);
    });
    
    // Get current language
    server.on("/api/language", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetCurrentLanguage(request);
    });
    
    // Set current language
    server.on("/api/language", HTTP_POST, [this](AsyncWebServerRequest* request) {
        this->handleSetLanguage(request);
    });
    
    // Get translations for current or specified language
    server.on("/api/translations", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetTranslations(request);
    });
    
    // Get language system status
    server.on("/api/language/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleLanguageStatus(request);
    });
    
    ESP_LOGI(TAG, "Language API endpoints configured");
}

void WebLanguageIntegration::handleGetLanguages(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    JsonObject root = doc.to<JsonObject>();
    
    JsonArray languages = root.createNestedArray("languages");
    
    auto availableLanguages = languageManager_.getAvailableLanguages();
    for (const auto& lang : availableLanguages) {
        JsonObject langObj = languages.createNestedObject();
        const LanguageInfo& info = languageManager_.getLanguageInfo(lang);
        
        langObj["code"] = languageManager_.getLanguageString(lang);
        langObj["name"] = info.name;
        langObj["nativeName"] = info.nativeName;
        langObj["flag"] = info.flag;
        langObj["rtl"] = info.rtl;
        langObj["dateFormat"] = info.dateFormat;
        langObj["timeFormat"] = info.timeFormat;
    }
    
    root["currentLanguage"] = languageManager_.getLanguageString();
    root["defaultLanguage"] = "en";
    root["success"] = true;
    
    String response;
    serializeJson(doc, response);
    sendJSONResponse(request, response);
}

void WebLanguageIntegration::handleGetCurrentLanguage(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(512);
    JsonObject root = doc.to<JsonObject>();
    
    const LanguageInfo& info = languageManager_.getLanguageInfo();
    
    root["language"] = languageManager_.getLanguageString();
    root["name"] = info.name;
    root["nativeName"] = info.nativeName;
    root["flag"] = info.flag;
    root["rtl"] = info.rtl;
    root["dateFormat"] = info.dateFormat;
    root["timeFormat"] = info.timeFormat;
    root["success"] = true;
    
    // Add cache statistics
    auto cacheStats = languageManager_.getCacheStats();
    JsonObject cache = root.createNestedObject("cacheStats");
    cache["totalEntries"] = cacheStats.totalEntries;
    cache["cacheHits"] = cacheStats.cacheHits;
    cache["cacheMisses"] = cacheStats.cacheMisses;
    cache["memoryUsed"] = cacheStats.memoryUsed;
    
    String response;
    serializeJson(doc, response);
    sendJSONResponse(request, response);
}

void WebLanguageIntegration::handleSetLanguage(AsyncWebServerRequest* request) {
    String languageCode = parseLanguageFromRequest(request);
    
    if (languageCode.isEmpty()) {
        String errorResponse = createErrorJSON("Missing or invalid language parameter", 400);
        sendJSONResponse(request, errorResponse, 400);
        return;
    }
    
    if (!isValidLanguageCode(languageCode)) {
        String errorResponse = createErrorJSON("Unsupported language code: " + languageCode, 400);
        sendJSONResponse(request, errorResponse, 400);
        return;
    }
    
    if (languageManager_.setLanguage(languageCode)) {
        // Save the preference
        languageManager_.saveLanguagePreference();
        
        // Create success response with new language info
        DynamicJsonDocument doc(512);
        JsonObject root = doc.to<JsonObject>();
        
        const LanguageInfo& info = languageManager_.getLanguageInfo();
        
        root["success"] = true;
        root["message"] = languageManager_.translate(TranslationCategory::SYSTEM, "message_ready");
        root["language"] = languageCode;
        root["name"] = info.name;
        root["nativeName"] = info.nativeName;
        root["flag"] = info.flag;
        root["rtl"] = info.rtl;
        
        String response;
        serializeJson(doc, response);
        sendJSONResponse(request, response);
        
        ESP_LOGI(TAG, "Language changed to: %s", languageCode.c_str());
    } else {
        String errorResponse = createErrorJSON("Failed to set language: " + languageCode, 500);
        sendJSONResponse(request, errorResponse, 500);
    }
}

void WebLanguageIntegration::handleGetTranslations(AsyncWebServerRequest* request) {
    String languageParam = "";
    String categoryParam = "";
    
    // Parse query parameters
    if (request->hasParam("lang")) {
        languageParam = request->getParam("lang")->value();
    }
    if (request->hasParam("category")) {
        categoryParam = request->getParam("category")->value();
    }
    
    // Set language temporarily if specified
    LanguageCode originalLang = languageManager_.getCurrentLanguage();
    if (!languageParam.isEmpty() && isValidLanguageCode(languageParam)) {
        languageManager_.setLanguage(languageParam);
    }
    
    // Determine category filter
    TranslationCategory category = TranslationCategory::COUNT; // All categories
    if (!categoryParam.isEmpty()) {
        if (categoryParam == "system") category = TranslationCategory::SYSTEM;
        else if (categoryParam == "ui") category = TranslationCategory::UI;
        else if (categoryParam == "wildlife") category = TranslationCategory::WILDLIFE;
        else if (categoryParam == "power") category = TranslationCategory::POWER;
        else if (categoryParam == "network") category = TranslationCategory::NETWORK;
        else if (categoryParam == "camera") category = TranslationCategory::CAMERA;
        else if (categoryParam == "storage") category = TranslationCategory::STORAGE;
        else if (categoryParam == "settings") category = TranslationCategory::SETTINGS;
    }
    
    // Get translations
    String translationsJSON = languageManager_.getTranslationsJSON(category);
    
    // Restore original language if changed
    if (!languageParam.isEmpty() && languageParam != languageManager_.getLanguageString(originalLang)) {
        languageManager_.setLanguage(originalLang);
    }
    
    sendJSONResponse(request, translationsJSON);
}

void WebLanguageIntegration::handleLanguageStatus(AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    JsonObject root = doc.to<JsonObject>();
    
    // Current language info
    const LanguageInfo& info = languageManager_.getLanguageInfo();
    JsonObject current = root.createNestedObject("currentLanguage");
    current["code"] = languageManager_.getLanguageString();
    current["name"] = info.name;
    current["nativeName"] = info.nativeName;
    current["flag"] = info.flag;
    current["rtl"] = info.rtl;
    
    // System info
    root["memoryUsage"] = languageManager_.getMemoryUsage();
    root["supportedLanguagesCount"] = static_cast<int>(LanguageCode::COUNT);
    
    // Cache statistics
    auto cacheStats = languageManager_.getCacheStats();
    JsonObject cache = root.createNestedObject("cache");
    cache["totalEntries"] = cacheStats.totalEntries;
    cache["cacheHits"] = cacheStats.cacheHits;
    cache["cacheMisses"] = cacheStats.cacheMisses;
    cache["memoryUsed"] = cacheStats.memoryUsed;
    cache["hitRatio"] = cacheStats.cacheHits + cacheStats.cacheMisses > 0 ? 
                       (float)cacheStats.cacheHits / (cacheStats.cacheHits + cacheStats.cacheMisses) : 0.0f;
    
    // System status messages
    JsonObject status = root.createNestedObject("systemStatus");
    status["online"] = languageManager_.translate(TranslationCategory::SYSTEM, "status_online");
    status["offline"] = languageManager_.translate(TranslationCategory::SYSTEM, "status_offline");
    status["ready"] = languageManager_.translate(TranslationCategory::SYSTEM, "message_ready");
    
    root["success"] = true;
    root["timestamp"] = time(nullptr);
    
    String response;
    serializeJson(doc, response);
    sendJSONResponse(request, response);
}

String WebLanguageIntegration::getLocalizedSystemStatus(int statusCode) {
    switch (statusCode) {
        case 0: return languageManager_.translate(TranslationCategory::SYSTEM, "status_offline");
        case 1: return languageManager_.translate(TranslationCategory::SYSTEM, "status_online");
        case 2: return languageManager_.translate(TranslationCategory::SYSTEM, "status_error");
        case 3: return languageManager_.translate(TranslationCategory::SYSTEM, "status_warning");
        default: return languageManager_.translate(TranslationCategory::SYSTEM, "status_ok");
    }
}

String WebLanguageIntegration::getLocalizedErrorMessage(int errorCode) {
    switch (errorCode) {
        case 1: return languageManager_.translate(TranslationCategory::SYSTEM, "error_network");
        case 2: return languageManager_.translate(TranslationCategory::SYSTEM, "error_camera");
        case 3: return languageManager_.translate(TranslationCategory::SYSTEM, "error_storage");
        case 4: return languageManager_.translate(TranslationCategory::SYSTEM, "error_power");
        default: return languageManager_.translate(TranslationCategory::SYSTEM, "status_error");
    }
}

String WebLanguageIntegration::getLocalizedUIStrings() {
    return languageManager_.getTranslationsJSON(TranslationCategory::UI);
}

String WebLanguageIntegration::formatLocalizedTimestamp(time_t timestamp, bool includeTime) {
    if (includeTime) {
        return languageManager_.formatDate(timestamp) + " " + languageManager_.formatTime(timestamp);
    } else {
        return languageManager_.formatDate(timestamp);
    }
}

// Private methods

void WebLanguageIntegration::sendJSONResponse(AsyncWebServerRequest* request, 
                                            const String& jsonContent, int statusCode) {
    AsyncWebServerResponse* response = request->beginResponse(statusCode, "application/json", jsonContent);
    
    // Add CORS headers
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    
    request->send(response);
}

String WebLanguageIntegration::createErrorJSON(const String& errorMessage, int errorCode) {
    DynamicJsonDocument doc(256);
    JsonObject root = doc.to<JsonObject>();
    
    root["success"] = false;
    root["error"] = errorMessage;
    if (errorCode > 0) {
        root["errorCode"] = errorCode;
    }
    root["timestamp"] = time(nullptr);
    
    String result;
    serializeJson(doc, result);
    return result;
}

String WebLanguageIntegration::createSuccessJSON(const String& data) {
    DynamicJsonDocument doc(256);
    JsonObject root = doc.to<JsonObject>();
    
    root["success"] = true;
    if (!data.isEmpty()) {
        root["data"] = data;
    }
    root["timestamp"] = time(nullptr);
    
    String result;
    serializeJson(doc, result);
    return result;
}

String WebLanguageIntegration::parseLanguageFromRequest(AsyncWebServerRequest* request) {
    // Try to get language from URL parameter
    if (request->hasParam("lang")) {
        return request->getParam("lang")->value();
    }
    
    // Try to get language from POST body (if available)
    if (request->hasParam("language", true)) {
        return request->getParam("language", true)->value();
    }
    
    // Try to parse JSON body if it's a POST request
    if (request->method() == HTTP_POST && request->hasParam("plain", true)) {
        String body = request->getParam("plain", true)->value();
        
        DynamicJsonDocument doc(256);
        if (deserializeJson(doc, body) == DeserializationError::Ok) {
            if (doc.containsKey("language")) {
                return doc["language"].as<String>();
            }
            if (doc.containsKey("lang")) {
                return doc["lang"].as<String>();
            }
        }
    }
    
    return String(); // Empty string if not found
}

bool WebLanguageIntegration::isValidLanguageCode(const String& languageCode) {
    // Convert string to language code using the static method in LanguageManager
    auto availableLanguages = languageManager_.getAvailableLanguages();
    for (const auto& lang : availableLanguages) {
        if (languageManager_.getLanguageString(lang) == languageCode) {
            return true;
        }
    }
    return false;
}