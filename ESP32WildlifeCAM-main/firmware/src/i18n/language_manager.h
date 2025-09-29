/**
 * @file language_manager.h
 * @brief Multi-language Support Manager for ESP32WildlifeCAM
 * 
 * Provides comprehensive internationalization support with:
 * - Efficient memory usage optimized for ESP32 constraints
 * - PROGMEM storage for translation strings to save RAM
 * - Language preference persistence in EEPROM
 * - Fallback mechanism to default language
 * - Support for adding new languages dynamically
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <map>
#include <vector>

/**
 * Supported language codes (ISO 639-1)
 */
enum class LanguageCode {
    EN = 0, // English (default)
    ES = 1, // Spanish
    FR = 2, // French  
    DE = 3, // German
    COUNT   // Number of supported languages
};

/**
 * Translation category for organizing translations
 */
enum class TranslationCategory {
    SYSTEM,      // System messages, errors, status
    UI,          // User interface elements
    WILDLIFE,    // Wildlife species, behaviors
    POWER,       // Power management messages
    NETWORK,     // Network and connectivity
    CAMERA,      // Camera and imaging
    STORAGE,     // Storage and file management
    SETTINGS,    // Configuration and settings
    COUNT        // Number of categories
};

/**
 * Translation key structure for efficient lookup
 */
struct TranslationKey {
    TranslationCategory category;
    const char* key;
    
    TranslationKey(TranslationCategory cat, const char* k) : category(cat), key(k) {}
};

/**
 * Language information structure
 */
struct LanguageInfo {
    LanguageCode code;
    const char* name;        // Language name in English
    const char* nativeName;  // Language name in native language
    const char* flag;        // Unicode flag emoji
    bool rtl;               // Right-to-left text direction
    const char* dateFormat; // Date format pattern
    const char* timeFormat; // Time format pattern
};

/**
 * Translation storage structure optimized for ESP32 memory
 */
struct TranslationEntry {
    const char* key;
    const char* translation;
};

/**
 * Language Manager Class
 * 
 * Manages multi-language support with efficient memory usage
 * and seamless integration with ESP32 web interface.
 */
class LanguageManager {
public:
    /**
     * Constructor
     */
    LanguageManager();
    
    /**
     * Initialize the language manager
     * @param defaultLang Default language code
     * @return true if initialization successful
     */
    bool begin(LanguageCode defaultLang = LanguageCode::EN);
    
    /**
     * Set current language
     * @param lang Language code to set
     * @return true if language was set successfully
     */
    bool setLanguage(LanguageCode lang);
    
    /**
     * Set current language by string code
     * @param langCode Language code string (e.g., "en", "es", "fr", "de")
     * @return true if language was set successfully
     */
    bool setLanguage(const String& langCode);
    
    /**
     * Get current language code
     * @return Current language code
     */
    LanguageCode getCurrentLanguage() const { return currentLanguage_; }
    
    /**
     * Get language code as string
     * @param lang Language code (optional, uses current if not specified)
     * @return Language code string
     */
    String getLanguageString(LanguageCode lang = static_cast<LanguageCode>(-1)) const;
    
    /**
     * Get language information
     * @param lang Language code (optional, uses current if not specified)
     * @return Language information structure
     */
    const LanguageInfo& getLanguageInfo(LanguageCode lang = static_cast<LanguageCode>(-1)) const;
    
    /**
     * Translate a text key
     * @param category Translation category
     * @param key Translation key
     * @return Translated text (or original key if not found)
     */
    String translate(TranslationCategory category, const char* key) const;
    
    /**
     * Translate a text key with category prefix
     * @param fullKey Full key in format "category.key"
     * @return Translated text (or original key if not found)
     */
    String translate(const char* fullKey) const;
    
    /**
     * Translate with parameter substitution
     * @param category Translation category
     * @param key Translation key
     * @param params Parameters to substitute in format {0}, {1}, etc.
     * @return Translated text with parameters substituted
     */
    String translatef(TranslationCategory category, const char* key, const std::vector<String>& params) const;
    
    /**
     * Get all available languages
     * @return Vector of supported language codes
     */
    std::vector<LanguageCode> getAvailableLanguages() const;
    
    /**
     * Check if a language is supported
     * @param lang Language code to check
     * @return true if language is supported
     */
    bool isLanguageSupported(LanguageCode lang) const;
    
    /**
     * Get translations for web interface as JSON
     * @param category Optional category filter
     * @return JSON string with translations
     */
    String getTranslationsJSON(TranslationCategory category = TranslationCategory::COUNT) const;
    
    /**
     * Load translations from JSON string
     * @param json JSON string containing translations
     * @param lang Language code to load for
     * @return true if loaded successfully
     */
    bool loadTranslationsFromJSON(const String& json, LanguageCode lang);
    
    /**
     * Save current language preference to persistent storage
     * @return true if saved successfully
     */
    bool saveLanguagePreference();
    
    /**
     * Load language preference from persistent storage
     * @return true if loaded successfully
     */
    bool loadLanguagePreference();
    
    /**
     * Format date according to current language settings
     * @param timestamp Unix timestamp
     * @return Formatted date string
     */
    String formatDate(time_t timestamp) const;
    
    /**
     * Format time according to current language settings
     * @param timestamp Unix timestamp
     * @return Formatted time string
     */
    String formatTime(time_t timestamp) const;
    
    /**
     * Get memory usage information
     * @return Memory usage in bytes
     */
    size_t getMemoryUsage() const;
    
    /**
     * Get translation cache statistics
     */
    struct CacheStats {
        size_t totalEntries;
        size_t cacheHits;
        size_t cacheMisses;
        size_t memoryUsed;
    };
    
    CacheStats getCacheStats() const { return cacheStats_; }

private:
    // Current language state
    LanguageCode currentLanguage_;
    LanguageCode defaultLanguage_;
    
    // Preferences for persistent storage
    Preferences preferences_;
    
    // Translation cache for performance
    mutable std::map<String, String> translationCache_;
    mutable CacheStats cacheStats_;
    
    // Constants
    static const char* PREFERENCE_NAMESPACE;
    static const char* LANGUAGE_PREFERENCE_KEY;
    static const size_t MAX_CACHE_SIZE = 100; // Limit cache size for memory management
    
    /**
     * Get built-in translations from PROGMEM
     * @param lang Language code
     * @param category Translation category
     * @return Pointer to translation entries array
     */
    const TranslationEntry* getBuiltinTranslations(LanguageCode lang, TranslationCategory category) const;
    
    /**
     * Get number of built-in translations for category
     * @param lang Language code
     * @param category Translation category
     * @return Number of translation entries
     */
    size_t getBuiltinTranslationCount(LanguageCode lang, TranslationCategory category) const;
    
    /**
     * Parse category from full key
     * @param fullKey Full key in format "category.key"
     * @return Translation category
     */
    TranslationCategory parseCategoryFromKey(const char* fullKey) const;
    
    /**
     * Extract key from full key
     * @param fullKey Full key in format "category.key"
     * @return Key part
     */
    String extractKeyFromFullKey(const char* fullKey) const;
    
    /**
     * Add entry to translation cache
     * @param key Cache key
     * @param translation Translation value
     */
    void addToCache(const String& key, const String& translation) const;
    
    /**
     * Clear translation cache
     */
    void clearCache();
    
    /**
     * Convert language code to string
     * @param lang Language code
     * @return Language string
     */
    static const char* languageCodeToString(LanguageCode lang);
    
    /**
     * Convert string to language code
     * @param langStr Language string
     * @return Language code
     */
    static LanguageCode stringToLanguageCode(const String& langStr);
};

// Global language manager instance
extern LanguageManager g_languageManager;

/**
 * Convenience macros for translation
 */
#define T(category, key) g_languageManager.translate(TranslationCategory::category, key)
#define TR(key) g_languageManager.translate(key)
#define TF(category, key, ...) g_languageManager.translatef(TranslationCategory::category, key, {__VA_ARGS__})

#endif // LANGUAGE_MANAGER_H