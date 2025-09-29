# Multi-Language Support Documentation

## Overview

The ESP32WildlifeCAM project includes comprehensive multi-language support to make the wildlife monitoring system accessible to users worldwide. The internationalization (i18n) framework supports both the embedded C++ firmware and the web interface.

## Supported Languages

Currently supported languages:
- **English (en)** - Default language
- **Spanish (es)** - Español
- **French (fr)** - Français  
- **German (de)** - Deutsch

## Architecture

### Backend (C++ Firmware)

The language system consists of several key components:

1. **LanguageManager** (`firmware/src/i18n/language_manager.h/cpp`)
   - Core language management system
   - PROGMEM-optimized translation storage
   - Language preference persistence using ESP32 Preferences
   - Translation caching for performance
   - Fallback mechanism to default language

2. **WebLanguageIntegration** (`firmware/src/web/web_language_integration.h/cpp`)
   - Web API endpoints for language management
   - Integration with AsyncWebServer
   - JSON response formatting
   - CORS support

3. **Translation Categories**
   - `SYSTEM` - System messages, errors, status
   - `UI` - User interface elements
   - `WILDLIFE` - Wildlife species, behaviors
   - `POWER` - Power management messages
   - `NETWORK` - Network and connectivity
   - `CAMERA` - Camera and imaging
   - `STORAGE` - Storage and file management
   - `SETTINGS` - Configuration and settings

### Frontend (JavaScript)

1. **I18nManager** (`firmware/src/web/web_assets/i18n.js`)
   - Client-side translation management
   - Dynamic language switching
   - Translation key-based text replacement
   - Parameter substitution support
   - Date/time formatting based on locale
   - DOM observer for dynamic content

2. **LanguageSelector** (`firmware/src/web/web_assets/language_selector.js`)
   - UI component for language selection
   - Flag emojis and native language names
   - Multiple display variants (inline, compact, default)
   - Auto-initialization from data attributes

## API Endpoints

### Language Management

- `GET /api/languages` - Get all available languages
- `GET /api/language` - Get current language info
- `POST /api/language` - Set current language
- `GET /api/translations?lang=<code>&category=<category>` - Get translations
- `GET /api/language/status` - Get language system status

### Example API Responses

#### Get Available Languages
```json
{
  "languages": [
    {
      "code": "en",
      "name": "English",
      "nativeName": "English",
      "flag": "🇺🇸",
      "rtl": false,
      "dateFormat": "%Y-%m-%d",
      "timeFormat": "%H:%M:%S"
    },
    {
      "code": "es",
      "name": "Spanish", 
      "nativeName": "Español",
      "flag": "🇪🇸",
      "rtl": false,
      "dateFormat": "%d/%m/%Y",
      "timeFormat": "%H:%M:%S"
    }
  ],
  "currentLanguage": "en",
  "defaultLanguage": "en",
  "success": true
}
```

#### Get Translations
```json
{
  "language": "en",
  "languageName": "English",
  "nativeName": "English",
  "flag": "🇺🇸",
  "rtl": false,
  "translations": {
    "system.status_online": "Online",
    "system.status_offline": "Offline",
    "ui.save": "Save",
    "ui.cancel": "Cancel"
  }
}
```

## Usage

### Backend (C++)

```cpp
#include "i18n/language_manager.h"

// Initialize language manager
g_languageManager.begin(LanguageCode::EN);

// Load saved language preference
g_languageManager.loadLanguagePreference();

// Translate messages
String onlineStatus = g_languageManager.translate(TranslationCategory::SYSTEM, "status_online");
String saveButton = g_languageManager.translate(TranslationCategory::UI, "save");

// Using convenience macros
String errorMsg = T(SYSTEM, "error_network");
String fullKey = TR("ui.loading");

// With parameter substitution
String message = g_languageManager.translatef(TranslationCategory::SYSTEM, "battery_level", {"75%"});

// Change language
g_languageManager.setLanguage(LanguageCode::ES);
g_languageManager.saveLanguagePreference();
```

### Frontend (JavaScript)

#### HTML with i18n attributes
```html
<!-- Basic translation -->
<h1 data-i18n="system.device_name">Wildlife Camera</h1>

<!-- Translation with target attribute -->
<button data-i18n="ui.save" data-i18n-target="textContent">Save</button>

<!-- Translation for placeholder -->
<input type="text" data-i18n="ui.search" data-i18n-target="placeholder" placeholder="Search">

<!-- Translation with parameters -->
<span data-i18n="wildlife.confidence" 
      data-i18n-params='{"value": "85%"}'>Confidence: {value}</span>

<!-- Language selector -->
<div id="language-selector" data-language-selector data-variant="inline"></div>
```

#### JavaScript usage
```javascript
// Use global translation function
const saveText = t('ui.save');
const loadingText = t('ui.loading');

// With parameters
const batteryText = t('power.battery_level', {level: '75%'});

// Change language programmatically
await window.i18n.changeLanguage('es');

// Format dates/times
const formattedDate = formatDate(new Date());
const formattedTime = formatTime(new Date());

// Listen for language changes
window.i18n.addObserver((event, data) => {
  if (event === 'languageChanged') {
    console.log('Language changed to:', data.language);
  }
});
```

## Adding New Languages

### 1. Backend Translation Tables

Add translation tables to `language_manager.cpp`:

```cpp
// System category - Italian
const PROGMEM TranslationEntry systemTranslationsIT[] = {
    {"status_online", "Online"},
    {"status_offline", "Offline"},
    {"error_network", "Errore di rete"},
    // ... more translations
};

// Add to translation tables array
const TranslationEntry* const PROGMEM translationTables[][static_cast<int>(TranslationCategory::COUNT)] = {
    // ... existing languages
    // Italian
    {
        systemTranslationsIT, uiTranslationsIT, wildlifeTranslationsIT,
        powerTranslationsIT, networkTranslationsIT, cameraTranslationsIT,
        storageTranslationsIT, settingsTranslationsIT
    }
};
```

### 2. Update Language Enum

In `language_manager.h`:

```cpp
enum class LanguageCode {
    EN = 0, // English (default)
    ES = 1, // Spanish
    FR = 2, // French  
    DE = 3, // German
    IT = 4, // Italian - NEW
    COUNT   // Number of supported languages
};
```

### 3. Add Language Info

In `language_manager.cpp`:

```cpp
const PROGMEM LanguageInfo languageInfos[] = {
    // ... existing languages
    {LanguageCode::IT, "Italian", "Italiano", "🇮🇹", false, "%d/%m/%Y", "%H:%M:%S"}
};
```

### 4. Update String Conversion

In `language_manager.cpp`:

```cpp
const char* LanguageManager::languageCodeToString(LanguageCode lang) {
    switch (lang) {
        // ... existing cases
        case LanguageCode::IT: return "it";
        default: return "en";
    }
}

LanguageCode LanguageManager::stringToLanguageCode(const String& langStr) {
    String lower = langStr;
    lower.toLowerCase();
    
    // ... existing checks
    if (lower == "it") return LanguageCode::IT;
    
    return LanguageCode::COUNT; // Invalid
}
```

### 5. Frontend Support

Update `i18n.js`:

```javascript
this.supportedLanguages = {
    // ... existing languages
    'it': { name: 'Italian', nativeName: 'Italiano', flag: '🇮🇹', rtl: false }
};

this.dateFormats = {
    // ... existing formats
    'it': { date: 'dd/MM/yyyy', time: 'HH:mm:ss', locale: 'it-IT' }
};
```

### 6. Translation File

Create `firmware/data/translations/it.json`:

```json
{
  "language": "it",
  "languageName": "Italian",
  "nativeName": "Italiano",
  "flag": "🇮🇹",
  "rtl": false,
  "dateFormat": "%d/%m/%Y",
  "timeFormat": "%H:%M:%S",
  "translations": {
    "system.status_online": "Online",
    "system.status_offline": "Offline",
    // ... all translations
  }
}
```

## Translation Guidelines

### Key Naming Convention

Translation keys follow the format: `category.key`

- `category` - One of: system, ui, wildlife, power, network, camera, storage, settings
- `key` - Descriptive name in snake_case

Examples:
- `system.status_online`
- `ui.save_button`
- `power.battery_level`
- `wildlife.species_detected`

### Text Guidelines

1. **Keep text concise** - UI space is limited
2. **Use consistent terminology** - Maintain vocabulary across the interface
3. **Consider context** - Some words may have different meanings in different contexts
4. **Use proper capitalization** - Follow language conventions
5. **Handle plurals carefully** - Consider using parameter substitution for dynamic counts

### Parameter Substitution

Use `{paramName}` or `{0}`, `{1}` for parameters:

```json
{
  "power.battery_status": "Battery: {level}% ({status})",
  "wildlife.detection_summary": "Detected {0} animals in {1} images"
}
```

### Date and Time Formats

Each language should specify appropriate date/time formats:

- `dateFormat` - strftime format for dates
- `timeFormat` - strftime format for times  
- `locale` - JavaScript locale identifier for formatting

## Memory Considerations

### PROGMEM Usage

All built-in translations are stored in PROGMEM (flash memory) to conserve RAM:

```cpp
const PROGMEM TranslationEntry translations[] = {
    {"key", "translation"}
};
```

### Translation Cache

The system includes a translation cache (default 100 entries) to improve performance:
- LRU eviction when cache is full
- Cache is cleared when language changes
- Cache statistics available via API

### Memory Monitoring

Monitor memory usage:

```cpp
size_t memoryUsed = g_languageManager.getMemoryUsage();
auto cacheStats = g_languageManager.getCacheStats();
```

## Testing

### Unit Tests

Test language functionality:

```cpp
// Test language switching
ASSERT_TRUE(g_languageManager.setLanguage(LanguageCode::ES));
ASSERT_EQ("En línea", g_languageManager.translate(TranslationCategory::SYSTEM, "status_online"));

// Test fallback
ASSERT_EQ("nonexistent_key", g_languageManager.translate(TranslationCategory::SYSTEM, "nonexistent_key"));
```

### Web Interface Testing

1. Load web interface
2. Change language using selector
3. Verify all text updates
4. Test dynamic content (charts, notifications)
5. Verify persistence after page reload

## Troubleshooting

### Common Issues

1. **Translation not found**
   - Check key spelling and category
   - Verify translation exists in language table
   - Check fallback to default language

2. **Memory issues**
   - Monitor cache size
   - Check PROGMEM usage
   - Consider reducing translation table size

3. **Web interface not updating**
   - Verify i18n.js is loaded
   - Check data-i18n attributes
   - Ensure language selector is properly initialized

4. **API errors**
   - Check CORS headers
   - Verify JSON formatting
   - Check language code validity

### Debug Information

Enable debug logging:

```cpp
// Check language manager status
DEBUG_SYSTEM_INFO("Current language: %s", g_languageManager.getLanguageString().c_str());
DEBUG_SYSTEM_INFO("Memory usage: %d bytes", g_languageManager.getMemoryUsage());

// Get cache statistics
auto stats = g_languageManager.getCacheStats();
DEBUG_SYSTEM_INFO("Cache: %d entries, %d hits, %d misses", 
                  stats.totalEntries, stats.cacheHits, stats.cacheMisses);
```

## Performance Optimization

1. **Use translation cache effectively**
2. **Minimize frequent language switches**
3. **Group related translations**
4. **Use PROGMEM for all static translations**
5. **Monitor memory usage regularly**

## Contributing New Translations

1. Fork the repository
2. Add translations following the guidelines above
3. Test thoroughly with the web interface
4. Submit a pull request with:
   - Translation tables in C++
   - JSON translation files
   - Updated language enumeration
   - Test cases

## Future Enhancements

Planned improvements:
- Support for more languages (Chinese, Japanese, Arabic, etc.)
- Plural form handling
- Number and currency formatting
- Dynamic translation loading from files
- Translation validation tools
- Automatic translation detection

For questions or contributions, please open an issue or submit a pull request on GitHub.