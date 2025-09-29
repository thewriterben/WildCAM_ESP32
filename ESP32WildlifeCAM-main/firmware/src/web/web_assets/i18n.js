/**
 * @file i18n.js
 * @brief JavaScript Internationalization Framework for ESP32WildlifeCAM Web Interface
 * 
 * Provides client-side multi-language support with:
 * - Dynamic language switching without page reload
 * - Translation key-based text replacement
 * - Language preference persistence in localStorage
 * - Support for parameterized translations
 * - RTL text direction support
 * - Date/time formatting based on locale
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

class I18nManager {
    constructor() {
        this.currentLanguage = 'en';
        this.defaultLanguage = 'en';
        this.translations = {};
        this.languageInfo = {};
        this.translationCache = new Map();
        this.observers = [];
        
        // Supported languages with metadata
        this.supportedLanguages = {
            'en': { name: 'English', nativeName: 'English', flag: '🇺🇸', rtl: false },
            'es': { name: 'Spanish', nativeName: 'Español', flag: '🇪🇸', rtl: false },
            'fr': { name: 'French', nativeName: 'Français', flag: '🇫🇷', rtl: false },
            'de': { name: 'German', nativeName: 'Deutsch', flag: '🇩🇪', rtl: false }
        };
        
        this.dateFormats = {
            'en': { date: 'MM/dd/yyyy', time: 'HH:mm:ss', locale: 'en-US' },
            'es': { date: 'dd/MM/yyyy', time: 'HH:mm:ss', locale: 'es-ES' },
            'fr': { date: 'dd/MM/yyyy', time: 'HH:mm:ss', locale: 'fr-FR' },
            'de': { date: 'dd.MM.yyyy', time: 'HH:mm:ss', locale: 'de-DE' }
        };
        
        // DOM observer for dynamic content
        this.domObserver = null;
        
        // Initialize
        this.init();
    }
    
    /**
     * Initialize the i18n manager
     */
    async init() {
        try {
            // Load saved language preference
            this.loadLanguagePreference();
            
            // Load translations from server
            await this.loadTranslations(this.currentLanguage);
            
            // Apply initial language
            this.applyLanguage();
            
            // Set up DOM observer for dynamic content
            this.setupDOMObserver();
            
            // Set up event listeners
            this.setupEventListeners();
            
            console.log(`I18n Manager initialized - Current language: ${this.currentLanguage}`);
            
        } catch (error) {
            console.error('Failed to initialize I18n Manager:', error);
            this.currentLanguage = this.defaultLanguage;
        }
    }
    
    /**
     * Load translations from the server
     * @param {string} language - Language code to load
     */
    async loadTranslations(language) {
        try {
            const response = await fetch(`/api/translations?lang=${language}`);
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            const data = await response.json();
            this.translations[language] = data.translations || {};
            this.languageInfo[language] = {
                name: data.languageName || this.supportedLanguages[language].name,
                nativeName: data.nativeName || this.supportedLanguages[language].nativeName,
                flag: data.flag || this.supportedLanguages[language].flag,
                rtl: data.rtl || this.supportedLanguages[language].rtl
            };
            
            console.log(`Translations loaded for language: ${language}`);
            
        } catch (error) {
            console.warn(`Failed to load translations for ${language}:`, error);
            
            // Fallback to built-in translations
            if (!this.translations[language]) {
                this.translations[language] = this.getBuiltinTranslations(language);
                this.languageInfo[language] = this.supportedLanguages[language];
            }
        }
    }
    
    /**
     * Get built-in fallback translations
     * @param {string} language - Language code
     * @returns {Object} Built-in translations
     */
    getBuiltinTranslations(language) {
        const builtinTranslations = {
            'en': {
                'ui.loading': 'Loading...',
                'ui.save': 'Save',
                'ui.cancel': 'Cancel',
                'ui.ok': 'OK',
                'ui.error': 'Error',
                'ui.warning': 'Warning',
                'ui.language': 'Language',
                'system.status_online': 'Online',
                'system.status_offline': 'Offline'
            },
            'es': {
                'ui.loading': 'Cargando...',
                'ui.save': 'Guardar',
                'ui.cancel': 'Cancelar',
                'ui.ok': 'OK',
                'ui.error': 'Error',
                'ui.warning': 'Advertencia',
                'ui.language': 'Idioma',
                'system.status_online': 'En línea',
                'system.status_offline': 'Desconectado'
            },
            'fr': {
                'ui.loading': 'Chargement...',
                'ui.save': 'Enregistrer',
                'ui.cancel': 'Annuler',
                'ui.ok': 'OK',
                'ui.error': 'Erreur',
                'ui.warning': 'Avertissement',
                'ui.language': 'Langue',
                'system.status_online': 'En ligne',
                'system.status_offline': 'Hors ligne'
            },
            'de': {
                'ui.loading': 'Lädt...',
                'ui.save': 'Speichern',
                'ui.cancel': 'Abbrechen',
                'ui.ok': 'OK',
                'ui.error': 'Fehler',
                'ui.warning': 'Warnung',
                'ui.language': 'Sprache',
                'system.status_online': 'Online',
                'system.status_offline': 'Offline'
            }
        };
        
        return builtinTranslations[language] || builtinTranslations['en'];
    }
    
    /**
     * Change the current language
     * @param {string} language - Language code to switch to
     */
    async changeLanguage(language) {
        if (!this.supportedLanguages[language]) {
            console.error(`Unsupported language: ${language}`);
            return false;
        }
        
        if (language === this.currentLanguage) {
            return true; // No change needed
        }
        
        try {
            // Load translations if not already loaded
            if (!this.translations[language]) {
                await this.loadTranslations(language);
            }
            
            this.currentLanguage = language;
            this.saveLanguagePreference();
            this.applyLanguage();
            
            // Notify server about language change
            this.notifyServerLanguageChange(language);
            
            // Notify observers
            this.notifyObservers('languageChanged', { language });
            
            console.log(`Language changed to: ${language}`);
            return true;
            
        } catch (error) {
            console.error(`Failed to change language to ${language}:`, error);
            return false;
        }
    }
    
    /**
     * Translate a key
     * @param {string} key - Translation key (e.g., 'ui.save' or 'system.status_online')
     * @param {Object} params - Parameters for interpolation
     * @returns {string} Translated text
     */
    t(key, params = {}) {
        if (!key) return '';
        
        // Check cache first
        const cacheKey = `${this.currentLanguage}:${key}:${JSON.stringify(params)}`;
        if (this.translationCache.has(cacheKey)) {
            return this.translationCache.get(cacheKey);
        }
        
        let translation = this.getTranslation(key);
        
        // Perform parameter substitution
        if (params && Object.keys(params).length > 0) {
            translation = this.interpolateParameters(translation, params);
        }
        
        // Cache the result
        this.translationCache.set(cacheKey, translation);
        
        return translation;
    }
    
    /**
     * Get translation for a key
     * @param {string} key - Translation key
     * @returns {string} Translation or key if not found
     */
    getTranslation(key) {
        const currentTranslations = this.translations[this.currentLanguage];
        if (currentTranslations && currentTranslations[key]) {
            return currentTranslations[key];
        }
        
        // Fallback to default language
        if (this.currentLanguage !== this.defaultLanguage) {
            const defaultTranslations = this.translations[this.defaultLanguage];
            if (defaultTranslations && defaultTranslations[key]) {
                return defaultTranslations[key];
            }
        }
        
        // Return key if no translation found
        console.warn(`Translation not found for key: ${key}`);
        return key;
    }
    
    /**
     * Interpolate parameters in translation string
     * @param {string} translation - Translation string with placeholders
     * @param {Object} params - Parameters to substitute
     * @returns {string} Interpolated string
     */
    interpolateParameters(translation, params) {
        let result = translation;
        
        // Handle named parameters {paramName}
        for (const [key, value] of Object.entries(params)) {
            const placeholder = `{${key}}`;
            result = result.replace(new RegExp(placeholder, 'g'), value);
        }
        
        // Handle indexed parameters {0}, {1}, etc.
        if (Array.isArray(params)) {
            params.forEach((value, index) => {
                const placeholder = `{${index}}`;
                result = result.replace(new RegExp(placeholder, 'g'), value);
            });
        }
        
        return result;
    }
    
    /**
     * Apply current language to the page
     */
    applyLanguage() {
        // Update document language
        document.documentElement.lang = this.currentLanguage;
        
        // Update text direction
        const isRTL = this.languageInfo[this.currentLanguage]?.rtl || false;
        document.documentElement.dir = isRTL ? 'rtl' : 'ltr';
        
        // Translate all elements with data-i18n attribute
        this.translateElements();
        
        // Update language selector if present
        this.updateLanguageSelector();
        
        // Clear translation cache
        this.translationCache.clear();
    }
    
    /**
     * Translate all elements with data-i18n attributes
     */
    translateElements() {
        const elements = document.querySelectorAll('[data-i18n]');
        
        elements.forEach(element => {
            const key = element.getAttribute('data-i18n');
            const params = this.getElementParams(element);
            const translation = this.t(key, params);
            
            // Determine how to apply the translation
            const target = element.getAttribute('data-i18n-target') || 'textContent';
            
            switch (target) {
                case 'textContent':
                    element.textContent = translation;
                    break;
                case 'innerHTML':
                    element.innerHTML = translation;
                    break;
                case 'placeholder':
                    element.placeholder = translation;
                    break;
                case 'title':
                    element.title = translation;
                    break;
                case 'alt':
                    element.alt = translation;
                    break;
                default:
                    if (target.startsWith('attr:')) {
                        const attrName = target.substring(5);
                        element.setAttribute(attrName, translation);
                    } else {
                        element.textContent = translation;
                    }
            }
        });
    }
    
    /**
     * Get parameters from element data attributes
     * @param {Element} element - DOM element
     * @returns {Object} Parameters object
     */
    getElementParams(element) {
        const params = {};
        
        // Get parameters from data-i18n-params attribute
        const paramsAttr = element.getAttribute('data-i18n-params');
        if (paramsAttr) {
            try {
                Object.assign(params, JSON.parse(paramsAttr));
            } catch (error) {
                console.warn('Failed to parse i18n params:', paramsAttr);
            }
        }
        
        // Get parameters from individual data-i18n-param-* attributes
        Array.from(element.attributes).forEach(attr => {
            if (attr.name.startsWith('data-i18n-param-')) {
                const paramName = attr.name.substring(17); // Remove 'data-i18n-param-'
                params[paramName] = attr.value;
            }
        });
        
        return params;
    }
    
    /**
     * Update language selector dropdown
     */
    updateLanguageSelector() {
        const selector = document.getElementById('language-selector');
        if (!selector) return;
        
        // Update current value
        selector.value = this.currentLanguage;
        
        // Update options if needed
        if (selector.children.length === 0) {
            Object.entries(this.supportedLanguages).forEach(([code, info]) => {
                const option = document.createElement('option');
                option.value = code;
                option.textContent = `${info.flag} ${info.nativeName}`;
                if (code === this.currentLanguage) {
                    option.selected = true;
                }
                selector.appendChild(option);
            });
        }
    }
    
    /**
     * Format date according to current language settings
     * @param {Date|string|number} date - Date to format
     * @param {Object} options - Formatting options
     * @returns {string} Formatted date string
     */
    formatDate(date, options = {}) {
        const dateObj = new Date(date);
        const format = this.dateFormats[this.currentLanguage] || this.dateFormats[this.defaultLanguage];
        
        const defaultOptions = {
            year: 'numeric',
            month: '2-digit',
            day: '2-digit'
        };
        
        const formatOptions = { ...defaultOptions, ...options };
        
        try {
            return dateObj.toLocaleDateString(format.locale, formatOptions);
        } catch (error) {
            console.warn('Date formatting failed:', error);
            return dateObj.toLocaleDateString();
        }
    }
    
    /**
     * Format time according to current language settings
     * @param {Date|string|number} date - Date to format
     * @param {Object} options - Formatting options
     * @returns {string} Formatted time string
     */
    formatTime(date, options = {}) {
        const dateObj = new Date(date);
        const format = this.dateFormats[this.currentLanguage] || this.dateFormats[this.defaultLanguage];
        
        const defaultOptions = {
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        };
        
        const formatOptions = { ...defaultOptions, ...options };
        
        try {
            return dateObj.toLocaleTimeString(format.locale, formatOptions);
        } catch (error) {
            console.warn('Time formatting failed:', error);
            return dateObj.toLocaleTimeString();
        }
    }
    
    /**
     * Get current language info
     * @returns {Object} Language information
     */
    getCurrentLanguageInfo() {
        return {
            code: this.currentLanguage,
            ...this.languageInfo[this.currentLanguage],
            ...this.supportedLanguages[this.currentLanguage]
        };
    }
    
    /**
     * Get all supported languages
     * @returns {Object} Supported languages with metadata
     */
    getSupportedLanguages() {
        return this.supportedLanguages;
    }
    
    /**
     * Save language preference to localStorage
     */
    saveLanguagePreference() {
        try {
            localStorage.setItem('wildlife_cam_language', this.currentLanguage);
        } catch (error) {
            console.warn('Failed to save language preference:', error);
        }
    }
    
    /**
     * Load language preference from localStorage
     */
    loadLanguagePreference() {
        try {
            const saved = localStorage.getItem('wildlife_cam_language');
            if (saved && this.supportedLanguages[saved]) {
                this.currentLanguage = saved;
            } else {
                // Try to detect browser language
                const browserLang = navigator.language.substring(0, 2);
                if (this.supportedLanguages[browserLang]) {
                    this.currentLanguage = browserLang;
                }
            }
        } catch (error) {
            console.warn('Failed to load language preference:', error);
        }
    }
    
    /**
     * Notify server about language change
     * @param {string} language - New language code
     */
    async notifyServerLanguageChange(language) {
        try {
            await fetch('/api/language', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ language })
            });
        } catch (error) {
            console.warn('Failed to notify server about language change:', error);
        }
    }
    
    /**
     * Set up DOM observer for dynamic content
     */
    setupDOMObserver() {
        if (!window.MutationObserver) return;
        
        this.domObserver = new MutationObserver((mutations) => {
            let shouldTranslate = false;
            
            mutations.forEach((mutation) => {
                if (mutation.type === 'childList') {
                    mutation.addedNodes.forEach((node) => {
                        if (node.nodeType === Node.ELEMENT_NODE) {
                            if (node.hasAttribute && node.hasAttribute('data-i18n') || 
                                node.querySelector && node.querySelector('[data-i18n]')) {
                                shouldTranslate = true;
                            }
                        }
                    });
                }
            });
            
            if (shouldTranslate) {
                this.translateElements();
            }
        });
        
        this.domObserver.observe(document.body, {
            childList: true,
            subtree: true
        });
    }
    
    /**
     * Set up event listeners
     */
    setupEventListeners() {
        // Language selector change event
        document.addEventListener('change', (event) => {
            if (event.target.id === 'language-selector') {
                this.changeLanguage(event.target.value);
            }
        });
        
        // Custom language change events
        document.addEventListener('changeLanguage', (event) => {
            if (event.detail && event.detail.language) {
                this.changeLanguage(event.detail.language);
            }
        });
    }
    
    /**
     * Add observer for language changes
     * @param {Function} callback - Callback function
     */
    addObserver(callback) {
        this.observers.push(callback);
    }
    
    /**
     * Remove observer
     * @param {Function} callback - Callback function to remove
     */
    removeObserver(callback) {
        this.observers = this.observers.filter(obs => obs !== callback);
    }
    
    /**
     * Notify observers of events
     * @param {string} event - Event name
     * @param {Object} data - Event data
     */
    notifyObservers(event, data) {
        this.observers.forEach(callback => {
            try {
                callback(event, data);
            } catch (error) {
                console.error('Observer callback failed:', error);
            }
        });
    }
    
    /**
     * Clean up resources
     */
    destroy() {
        if (this.domObserver) {
            this.domObserver.disconnect();
            this.domObserver = null;
        }
        
        this.translationCache.clear();
        this.observers = [];
    }
}

// Global i18n instance
window.i18n = new I18nManager();

// Convenience functions
window.t = (key, params) => window.i18n.t(key, params);
window.formatDate = (date, options) => window.i18n.formatDate(date, options);
window.formatTime = (date, options) => window.i18n.formatTime(date, options);

// Auto-initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        if (window.i18n && !window.i18n.initialized) {
            window.i18n.init();
        }
    });
} else {
    // DOM is already ready
    if (window.i18n && !window.i18n.initialized) {
        window.i18n.init();
    }
}

export default I18nManager;