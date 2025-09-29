/**
 * @file example_language_demo.cpp
 * @brief Simple demonstration of multi-language support functionality
 * 
 * This example shows how to use the language manager and demonstrates
 * key features like language switching, translation retrieval, and
 * parameter substitution.
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

#include <Arduino.h>
#include "i18n/language_manager.h"
#include "web/web_language_integration.h"

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== ESP32WildlifeCAM Multi-Language Demo ===\n");
    
    // Initialize language manager
    Serial.println("Initializing language manager...");
    if (!g_languageManager.begin(LanguageCode::EN)) {
        Serial.println("ERROR: Failed to initialize language manager!");
        return;
    }
    
    Serial.printf("Language manager initialized successfully!\n");
    Serial.printf("Default language: %s\n\n", g_languageManager.getLanguageString().c_str());
    
    // Demonstrate basic translation
    Serial.println("=== Basic Translation Demo ===");
    Serial.printf("English: %s\n", g_languageManager.translate(TranslationCategory::SYSTEM, "status_online").c_str());
    Serial.printf("English: %s\n", g_languageManager.translate(TranslationCategory::UI, "save").c_str());
    
    // Test language switching
    Serial.println("\n=== Language Switching Demo ===");
    
    // Switch to Spanish
    Serial.println("Switching to Spanish...");
    g_languageManager.setLanguage(LanguageCode::ES);
    Serial.printf("Spanish: %s\n", g_languageManager.translate(TranslationCategory::SYSTEM, "status_online").c_str());
    Serial.printf("Spanish: %s\n", g_languageManager.translate(TranslationCategory::UI, "save").c_str());
    
    // Switch to French  
    Serial.println("Switching to French...");
    g_languageManager.setLanguage(LanguageCode::FR);
    Serial.printf("French: %s\n", g_languageManager.translate(TranslationCategory::SYSTEM, "status_online").c_str());
    Serial.printf("French: %s\n", g_languageManager.translate(TranslationCategory::UI, "save").c_str());
    
    // Switch to German
    Serial.println("Switching to German...");
    g_languageManager.setLanguage(LanguageCode::DE);
    Serial.printf("German: %s\n", g_languageManager.translate(TranslationCategory::SYSTEM, "status_online").c_str());
    Serial.printf("German: %s\n", g_languageManager.translate(TranslationCategory::UI, "save").c_str());
    
    // Demonstrate convenience macros
    Serial.println("\n=== Convenience Macros Demo ===");
    g_languageManager.setLanguage(LanguageCode::EN);
    Serial.printf("Using T() macro: %s\n", T(SYSTEM, "message_ready").c_str());
    Serial.printf("Using TR() macro: %s\n", TR("ui.loading").c_str());
    
    // Test parameter substitution
    Serial.println("\n=== Parameter Substitution Demo ===");
    std::vector<String> params = {"85%"};
    String batteryMsg = g_languageManager.translatef(TranslationCategory::POWER, "battery_level", params);
    Serial.printf("Battery message: %s\n", batteryMsg.c_str());
    
    // Test available languages
    Serial.println("\n=== Available Languages ===");
    auto languages = g_languageManager.getAvailableLanguages();
    for (const auto& lang : languages) {
        const LanguageInfo& info = g_languageManager.getLanguageInfo(lang);
        Serial.printf("%s %s (%s) - %s\n", 
                     info.flag, 
                     info.nativeName, 
                     g_languageManager.getLanguageString(lang).c_str(),
                     info.name);
    }
    
    // Test fallback behavior
    Serial.println("\n=== Fallback Behavior Demo ===");
    String nonexistent = g_languageManager.translate(TranslationCategory::SYSTEM, "nonexistent_key");
    Serial.printf("Nonexistent key result: '%s'\n", nonexistent.c_str());
    
    // Date/time formatting demo
    Serial.println("\n=== Date/Time Formatting Demo ===");
    time_t now = time(nullptr);
    
    g_languageManager.setLanguage(LanguageCode::EN);
    Serial.printf("English: %s %s\n", 
                 g_languageManager.formatDate(now).c_str(),
                 g_languageManager.formatTime(now).c_str());
    
    g_languageManager.setLanguage(LanguageCode::DE);
    Serial.printf("German: %s %s\n", 
                 g_languageManager.formatDate(now).c_str(),
                 g_languageManager.formatTime(now).c_str());
    
    // Memory usage statistics
    Serial.println("\n=== Memory Usage Statistics ===");
    Serial.printf("Total memory usage: %d bytes\n", g_languageManager.getMemoryUsage());
    
    auto cacheStats = g_languageManager.getCacheStats();
    Serial.printf("Cache entries: %d\n", cacheStats.totalEntries);
    Serial.printf("Cache hits: %d\n", cacheStats.cacheHits);
    Serial.printf("Cache misses: %d\n", cacheStats.cacheMisses);
    Serial.printf("Cache memory: %d bytes\n", cacheStats.memoryUsed);
    
    if (cacheStats.cacheHits + cacheStats.cacheMisses > 0) {
        float hitRatio = (float)cacheStats.cacheHits / (cacheStats.cacheHits + cacheStats.cacheMisses) * 100.0f;
        Serial.printf("Cache hit ratio: %.1f%%\n", hitRatio);
    }
    
    // Test JSON export
    Serial.println("\n=== JSON Export Demo ===");
    g_languageManager.setLanguage(LanguageCode::ES);
    String jsonTranslations = g_languageManager.getTranslationsJSON(TranslationCategory::UI);
    Serial.println("UI translations JSON (first 200 chars):");
    Serial.println(jsonTranslations.substring(0, 200) + "...");
    
    // Save language preference
    Serial.println("\n=== Persistence Demo ===");
    Serial.printf("Saving current language (%s) to preferences...\n", 
                 g_languageManager.getLanguageString().c_str());
    if (g_languageManager.saveLanguagePreference()) {
        Serial.println("Language preference saved successfully!");
    } else {
        Serial.println("Failed to save language preference!");
    }
    
    Serial.println("\n=== Demo Complete ===");
    Serial.println("Language system is ready for use!");
    Serial.println("Use the web interface to test dynamic language switching.");
}

void loop() {
    // Demonstrate dynamic translation access in the loop
    static unsigned long lastDemo = 0;
    static int demoStep = 0;
    
    if (millis() - lastDemo > 5000) { // Every 5 seconds
        lastDemo = millis();
        
        switch (demoStep % 4) {
            case 0:
                g_languageManager.setLanguage(LanguageCode::EN);
                Serial.printf("[%s] %s\n", 
                             g_languageManager.getLanguageString().c_str(),
                             T(SYSTEM, "status_online").c_str());
                break;
                
            case 1:
                g_languageManager.setLanguage(LanguageCode::ES);
                Serial.printf("[%s] %s\n", 
                             g_languageManager.getLanguageString().c_str(),
                             T(SYSTEM, "status_online").c_str());
                break;
                
            case 2:
                g_languageManager.setLanguage(LanguageCode::FR);
                Serial.printf("[%s] %s\n", 
                             g_languageManager.getLanguageString().c_str(),
                             T(SYSTEM, "status_online").c_str());
                break;
                
            case 3:
                g_languageManager.setLanguage(LanguageCode::DE);
                Serial.printf("[%s] %s\n", 
                             g_languageManager.getLanguageString().c_str(),
                             T(SYSTEM, "status_online").c_str());
                break;
        }
        
        demoStep++;
    }
    
    delay(100);
}