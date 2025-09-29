/**
 * @file web_language_integration.h
 * @brief Language Manager Integration with Web Server
 * 
 * Provides web API endpoints and integration for multi-language support
 * in the ESP32WildlifeCAM web interface.
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

#ifndef WEB_LANGUAGE_INTEGRATION_H
#define WEB_LANGUAGE_INTEGRATION_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "../i18n/language_manager.h"

/**
 * Web Language Integration Class
 * 
 * Handles web API endpoints for language management and provides
 * localized content for the web interface.
 */
class WebLanguageIntegration {
public:
    /**
     * Constructor
     * @param languageManager Reference to the language manager
     */
    WebLanguageIntegration(LanguageManager& languageManager);
    
    /**
     * Setup language-related API endpoints
     * @param server Reference to the AsyncWebServer
     */
    void setupAPIEndpoints(AsyncWebServer& server);
    
    /**
     * Handle language API requests
     */
    void handleGetLanguages(AsyncWebServerRequest* request);
    void handleGetCurrentLanguage(AsyncWebServerRequest* request);
    void handleSetLanguage(AsyncWebServerRequest* request);
    void handleGetTranslations(AsyncWebServerRequest* request);
    void handleLanguageStatus(AsyncWebServerRequest* request);
    
    /**
     * Get localized system status message
     * @param statusCode System status code
     * @return Localized status message
     */
    String getLocalizedSystemStatus(int statusCode);
    
    /**
     * Get localized error message
     * @param errorCode Error code
     * @return Localized error message
     */
    String getLocalizedErrorMessage(int errorCode);
    
    /**
     * Get localized web interface strings as JSON
     * @return JSON string with UI translations
     */
    String getLocalizedUIStrings();
    
    /**
     * Format timestamp according to current language
     * @param timestamp Unix timestamp
     * @param includeTime Whether to include time
     * @return Formatted date/time string
     */
    String formatLocalizedTimestamp(time_t timestamp, bool includeTime = true);

private:
    LanguageManager& languageManager_;
    
    /**
     * Create JSON response with CORS headers
     * @param request HTTP request
     * @param jsonContent JSON content string
     * @param statusCode HTTP status code
     */
    void sendJSONResponse(AsyncWebServerRequest* request, const String& jsonContent, 
                         int statusCode = 200);
    
    /**
     * Create error JSON response
     * @param errorMessage Error message
     * @param errorCode Error code (optional)
     * @return JSON error string
     */
    String createErrorJSON(const String& errorMessage, int errorCode = 0);
    
    /**
     * Create success JSON response
     * @param data Optional data object
     * @return JSON success string
     */
    String createSuccessJSON(const String& data = "");
    
    /**
     * Parse language code from request
     * @param request HTTP request
     * @return Language code or empty string if invalid
     */
    String parseLanguageFromRequest(AsyncWebServerRequest* request);
    
    /**
     * Validate language code
     * @param languageCode Language code to validate
     * @return true if valid
     */
    bool isValidLanguageCode(const String& languageCode);
};

#endif // WEB_LANGUAGE_INTEGRATION_H