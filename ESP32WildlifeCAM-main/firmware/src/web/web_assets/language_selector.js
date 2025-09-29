/**
 * @file language_selector.js
 * @brief Language Selector Component for ESP32WildlifeCAM Web Interface
 * 
 * Creates a dropdown language selector with flags and native names
 * that integrates with the i18n system.
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

class LanguageSelector {
    constructor(containerId, options = {}) {
        this.containerId = containerId;
        this.options = {
            showFlags: true,
            showNativeNames: true,
            showLanguageNames: false,
            position: 'bottom-right',
            ...options
        };
        
        this.container = null;
        this.selector = null;
        
        this.init();
    }
    
    /**
     * Initialize the language selector
     */
    init() {
        this.container = document.getElementById(this.containerId);
        if (!this.container) {
            console.error(`Language selector container not found: ${this.containerId}`);
            return;
        }
        
        this.render();
        this.setupEventListeners();
        
        // Update when language changes
        if (window.i18n) {
            window.i18n.addObserver((event, data) => {
                if (event === 'languageChanged') {
                    this.updateSelection(data.language);
                }
            });
        }
    }
    
    /**
     * Render the language selector
     */
    render() {
        const languages = window.i18n ? window.i18n.getSupportedLanguages() : {
            'en': { name: 'English', nativeName: 'English', flag: '🇺🇸' },
            'es': { name: 'Spanish', nativeName: 'Español', flag: '🇪🇸' },
            'fr': { name: 'French', nativeName: 'Français', flag: '🇫🇷' },
            'de': { name: 'German', nativeName: 'Deutsch', flag: '🇩🇪' }
        };
        
        const currentLang = window.i18n ? window.i18n.currentLanguage : 'en';
        
        this.container.innerHTML = `
            <div class="language-selector-wrapper">
                <label for="language-selector" class="language-selector-label" data-i18n="ui.language">
                    Language
                </label>
                <div class="language-selector-dropdown">
                    <select id="language-selector" class="language-selector">
                        ${Object.entries(languages).map(([code, info]) => `
                            <option value="${code}" ${code === currentLang ? 'selected' : ''}>
                                ${this.options.showFlags ? info.flag + ' ' : ''}${this.getDisplayName(info)}
                            </option>
                        `).join('')}
                    </select>
                    <div class="language-selector-icon">▼</div>
                </div>
            </div>
        `;
        
        this.selector = this.container.querySelector('#language-selector');
        
        // Add CSS if not already present
        this.addStyles();
    }
    
    /**
     * Get display name for language
     * @param {Object} languageInfo - Language information object
     * @returns {string} Display name
     */
    getDisplayName(languageInfo) {
        if (this.options.showNativeNames && this.options.showLanguageNames) {
            return `${languageInfo.nativeName} (${languageInfo.name})`;
        } else if (this.options.showNativeNames) {
            return languageInfo.nativeName;
        } else {
            return languageInfo.name;
        }
    }
    
    /**
     * Set up event listeners
     */
    setupEventListeners() {
        if (this.selector) {
            this.selector.addEventListener('change', (event) => {
                const selectedLanguage = event.target.value;
                if (window.i18n) {
                    window.i18n.changeLanguage(selectedLanguage);
                }
                
                // Trigger custom event
                this.container.dispatchEvent(new CustomEvent('languageChanged', {
                    detail: { language: selectedLanguage },
                    bubbles: true
                }));
            });
        }
    }
    
    /**
     * Update the selected language
     * @param {string} language - Language code
     */
    updateSelection(language) {
        if (this.selector && this.selector.value !== language) {
            this.selector.value = language;
        }
    }
    
    /**
     * Add CSS styles for the language selector
     */
    addStyles() {
        const styleId = 'language-selector-styles';
        if (document.getElementById(styleId)) {
            return; // Styles already added
        }
        
        const styles = document.createElement('style');
        styles.id = styleId;
        styles.textContent = `
            .language-selector-wrapper {
                display: flex;
                flex-direction: column;
                gap: 0.5rem;
                min-width: 200px;
            }
            
            .language-selector-label {
                font-size: 0.875rem;
                font-weight: 500;
                color: var(--text-primary, #333);
                margin-bottom: 0.25rem;
            }
            
            .language-selector-dropdown {
                position: relative;
                display: inline-block;
            }
            
            .language-selector {
                appearance: none;
                background: var(--surface-color, #fff);
                border: 1px solid var(--border-color, #ddd);
                border-radius: 0.5rem;
                padding: 0.5rem 2rem 0.5rem 0.75rem;
                font-size: 0.875rem;
                color: var(--text-primary, #333);
                cursor: pointer;
                transition: all 0.2s ease;
                width: 100%;
                min-width: 180px;
            }
            
            .language-selector:hover {
                border-color: var(--accent-color, #007bff);
                box-shadow: 0 0 0 2px var(--accent-color-alpha, rgba(0, 123, 255, 0.1));
            }
            
            .language-selector:focus {
                outline: none;
                border-color: var(--accent-color, #007bff);
                box-shadow: 0 0 0 3px var(--accent-color-alpha, rgba(0, 123, 255, 0.2));
            }
            
            .language-selector-icon {
                position: absolute;
                right: 0.75rem;
                top: 50%;
                transform: translateY(-50%);
                color: var(--text-muted, #666);
                pointer-events: none;
                font-size: 0.75rem;
                transition: transform 0.2s ease;
            }
            
            .language-selector:focus + .language-selector-icon {
                transform: translateY(-50%) rotate(180deg);
            }
            
            /* Dark theme support */
            @media (prefers-color-scheme: dark) {
                .language-selector {
                    background: var(--surface-dark, #2a2a2a);
                    border-color: var(--border-dark, #444);
                    color: var(--text-primary-dark, #fff);
                }
                
                .language-selector-label {
                    color: var(--text-primary-dark, #fff);
                }
                
                .language-selector-icon {
                    color: var(--text-muted-dark, #aaa);
                }
            }
            
            /* Responsive design */
            @media (max-width: 768px) {
                .language-selector-wrapper {
                    min-width: 150px;
                }
                
                .language-selector {
                    min-width: 150px;
                    font-size: 0.8rem;
                    padding: 0.4rem 1.8rem 0.4rem 0.6rem;
                }
            }
            
            /* RTL support */
            [dir="rtl"] .language-selector {
                padding: 0.5rem 0.75rem 0.5rem 2rem;
            }
            
            [dir="rtl"] .language-selector-icon {
                right: auto;
                left: 0.75rem;
            }
            
            /* Compact variant */
            .language-selector-wrapper.compact {
                flex-direction: row;
                align-items: center;
                gap: 0.75rem;
            }
            
            .language-selector-wrapper.compact .language-selector-label {
                margin-bottom: 0;
                white-space: nowrap;
            }
            
            .language-selector-wrapper.compact .language-selector {
                min-width: 120px;
            }
            
            /* Inline variant */
            .language-selector-wrapper.inline {
                flex-direction: row;
                align-items: center;
                gap: 0.5rem;
            }
            
            .language-selector-wrapper.inline .language-selector-label {
                margin-bottom: 0;
                font-size: 0.8rem;
            }
            
            .language-selector-wrapper.inline .language-selector {
                min-width: 100px;
                font-size: 0.8rem;
                padding: 0.3rem 1.5rem 0.3rem 0.5rem;
            }
        `;
        
        document.head.appendChild(styles);
    }
    
    /**
     * Set the variant of the language selector
     * @param {string} variant - 'default', 'compact', or 'inline'
     */
    setVariant(variant) {
        if (this.container) {
            this.container.querySelector('.language-selector-wrapper').className = 
                `language-selector-wrapper ${variant}`;
        }
    }
    
    /**
     * Show or hide the language selector
     * @param {boolean} visible - Whether to show the selector
     */
    setVisible(visible) {
        if (this.container) {
            this.container.style.display = visible ? 'block' : 'none';
        }
    }
    
    /**
     * Destroy the language selector
     */
    destroy() {
        if (this.container) {
            this.container.innerHTML = '';
        }
        
        this.container = null;
        this.selector = null;
    }
}

// Auto-initialize language selectors with data-language-selector attribute
document.addEventListener('DOMContentLoaded', () => {
    const selectors = document.querySelectorAll('[data-language-selector]');
    selectors.forEach(element => {
        const options = {};
        
        // Parse options from data attributes
        if (element.dataset.variant) {
            options.variant = element.dataset.variant;
        }
        if (element.dataset.showFlags !== undefined) {
            options.showFlags = element.dataset.showFlags !== 'false';
        }
        if (element.dataset.showNativeNames !== undefined) {
            options.showNativeNames = element.dataset.showNativeNames !== 'false';
        }
        if (element.dataset.showLanguageNames !== undefined) {
            options.showLanguageNames = element.dataset.showLanguageNames === 'true';
        }
        
        const selector = new LanguageSelector(element.id, options);
        
        // Apply variant if specified
        if (options.variant) {
            selector.setVariant(options.variant);
        }
        
        // Store reference for later access
        element._languageSelector = selector;
    });
});

// Export for module usage
if (typeof module !== 'undefined' && module.exports) {
    module.exports = LanguageSelector;
}

window.LanguageSelector = LanguageSelector;