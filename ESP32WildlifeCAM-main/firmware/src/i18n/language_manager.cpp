/**
 * @file language_manager.cpp
 * @brief Multi-language Support Manager Implementation
 * 
 * @author ESP32WildlifeCAM Project
 * @version 1.0.0
 */

#include "language_manager.h"
#include <time.h>

// Static constants
const char* LanguageManager::PREFERENCE_NAMESPACE = "language";
const char* LanguageManager::LANGUAGE_PREFERENCE_KEY = "current_lang";

// Global instance
LanguageManager g_languageManager;

// Language information stored in PROGMEM
const PROGMEM LanguageInfo languageInfos[] = {
    {LanguageCode::EN, "English", "English", "🇺🇸", false, "%Y-%m-%d", "%H:%M:%S"},
    {LanguageCode::ES, "Spanish", "Español", "🇪🇸", false, "%d/%m/%Y", "%H:%M:%S"},
    {LanguageCode::FR, "French", "Français", "🇫🇷", false, "%d/%m/%Y", "%H:%M:%S"},
    {LanguageCode::DE, "German", "Deutsch", "🇩🇪", false, "%d.%m.%Y", "%H:%M:%S"}
};

// ===============================
// BUILT-IN TRANSLATIONS - ENGLISH
// ===============================

// System category - English
const PROGMEM TranslationEntry systemTranslationsEN[] = {
    {"status_online", "Online"},
    {"status_offline", "Offline"},
    {"status_error", "Error"},
    {"status_warning", "Warning"},
    {"status_ok", "OK"},
    {"status_loading", "Loading..."},
    {"error_network", "Network error"},
    {"error_camera", "Camera error"},
    {"error_storage", "Storage error"},
    {"error_power", "Power error"},
    {"message_startup", "System starting up..."},
    {"message_shutdown", "System shutting down..."},
    {"message_ready", "System ready"},
    {"uptime", "Uptime"},
    {"version", "Version"},
    {"device_id", "Device ID"}
};

// UI category - English
const PROGMEM TranslationEntry uiTranslationsEN[] = {
    {"home", "Home"},
    {"dashboard", "Dashboard"},
    {"gallery", "Gallery"},
    {"settings", "Settings"},
    {"analytics", "Analytics"},
    {"live_view", "Live View"},
    {"capture", "Capture"},
    {"download", "Download"},
    {"delete", "Delete"},
    {"save", "Save"},
    {"cancel", "Cancel"},
    {"ok", "OK"},
    {"yes", "Yes"},
    {"no", "No"},
    {"refresh", "Refresh"},
    {"loading", "Loading"},
    {"search", "Search"},
    {"filter", "Filter"},
    {"sort", "Sort"},
    {"export", "Export"},
    {"import", "Import"},
    {"language", "Language"},
    {"theme", "Theme"},
    {"about", "About"},
    {"help", "Help"},
    {"close", "Close"},
    {"next", "Next"},
    {"previous", "Previous"},
    {"first", "First"},
    {"last", "Last"}
};

// Wildlife category - English
const PROGMEM TranslationEntry wildlifeTranslationsEN[] = {
    {"species_detected", "Species detected"},
    {"confidence", "Confidence"},
    {"behavior_feeding", "Feeding"},
    {"behavior_resting", "Resting"},
    {"behavior_moving", "Moving"},
    {"behavior_alert", "Alert"},
    {"detection_count", "Detections"},
    {"last_seen", "Last seen"},
    {"activity_high", "High activity"},
    {"activity_normal", "Normal activity"},
    {"activity_low", "Low activity"},
    {"activity_none", "No activity"}
};

// Power category - English
const PROGMEM TranslationEntry powerTranslationsEN[] = {
    {"battery_level", "Battery Level"},
    {"battery_charging", "Charging"},
    {"battery_discharging", "Discharging"},
    {"battery_full", "Full"},
    {"battery_low", "Low Battery"},
    {"battery_critical", "Critical Battery"},
    {"solar_voltage", "Solar Voltage"},
    {"power_consumption", "Power Consumption"},
    {"estimated_runtime", "Estimated Runtime"},
    {"power_mode_normal", "Normal Mode"},
    {"power_mode_eco", "Eco Mode"},
    {"power_mode_sleep", "Sleep Mode"}
};

// Network category - English
const PROGMEM TranslationEntry networkTranslationsEN[] = {
    {"wifi_connected", "WiFi Connected"},
    {"wifi_disconnected", "WiFi Disconnected"},
    {"wifi_connecting", "Connecting to WiFi"},
    {"signal_strength", "Signal Strength"},
    {"ip_address", "IP Address"},
    {"lora_status", "LoRa Status"},
    {"mesh_nodes", "Mesh Nodes"},
    {"data_transmitted", "Data Transmitted"},
    {"connection_quality", "Connection Quality"}
};

// Camera category - English
const PROGMEM TranslationEntry cameraTranslationsEN[] = {
    {"resolution", "Resolution"},
    {"quality", "Quality"},
    {"frame_rate", "Frame Rate"},
    {"exposure", "Exposure"},
    {"brightness", "Brightness"},
    {"contrast", "Contrast"},
    {"saturation", "Saturation"},
    {"night_mode", "Night Mode"},
    {"motion_detection", "Motion Detection"},
    {"recording", "Recording"},
    {"streaming", "Streaming"},
    {"capture_photo", "Capture Photo"},
    {"start_recording", "Start Recording"},
    {"stop_recording", "Stop Recording"}
};

// Storage category - English
const PROGMEM TranslationEntry storageTranslationsEN[] = {
    {"storage_used", "Storage Used"},
    {"storage_free", "Storage Free"},
    {"total_files", "Total Files"},
    {"file_size", "File Size"},
    {"file_date", "Date"},
    {"file_type", "Type"},
    {"storage_full", "Storage Full"},
    {"storage_warning", "Storage Warning"},
    {"cleanup_required", "Cleanup Required"},
    {"auto_delete", "Auto Delete"}
};

// Settings category - English
const PROGMEM TranslationEntry settingsTranslationsEN[] = {
    {"general", "General"},
    {"camera_settings", "Camera Settings"},
    {"power_settings", "Power Settings"},
    {"network_settings", "Network Settings"},
    {"storage_settings", "Storage Settings"},
    {"time_zone", "Time Zone"},
    {"date_format", "Date Format"},
    {"time_format", "Time Format"},
    {"auto_capture", "Auto Capture"},
    {"motion_sensitivity", "Motion Sensitivity"},
    {"reset_settings", "Reset Settings"},
    {"factory_reset", "Factory Reset"},
    {"firmware_update", "Firmware Update"}
};

// ===============================
// BUILT-IN TRANSLATIONS - SPANISH
// ===============================

// System category - Spanish
const PROGMEM TranslationEntry systemTranslationsES[] = {
    {"status_online", "En línea"},
    {"status_offline", "Desconectado"},
    {"status_error", "Error"},
    {"status_warning", "Advertencia"},
    {"status_ok", "OK"},
    {"status_loading", "Cargando..."},
    {"error_network", "Error de red"},
    {"error_camera", "Error de cámara"},
    {"error_storage", "Error de almacenamiento"},
    {"error_power", "Error de energía"},
    {"message_startup", "Sistema iniciando..."},
    {"message_shutdown", "Sistema apagándose..."},
    {"message_ready", "Sistema listo"},
    {"uptime", "Tiempo activo"},
    {"version", "Versión"},
    {"device_id", "ID del dispositivo"}
};

// UI category - Spanish
const PROGMEM TranslationEntry uiTranslationsES[] = {
    {"home", "Inicio"},
    {"dashboard", "Panel"},
    {"gallery", "Galería"},
    {"settings", "Configuración"},
    {"analytics", "Análisis"},
    {"live_view", "Vista en vivo"},
    {"capture", "Capturar"},
    {"download", "Descargar"},
    {"delete", "Eliminar"},
    {"save", "Guardar"},
    {"cancel", "Cancelar"},
    {"ok", "OK"},
    {"yes", "Sí"},
    {"no", "No"},
    {"refresh", "Actualizar"},
    {"loading", "Cargando"},
    {"search", "Buscar"},
    {"filter", "Filtrar"},
    {"sort", "Ordenar"},
    {"export", "Exportar"},
    {"import", "Importar"},
    {"language", "Idioma"},
    {"theme", "Tema"},
    {"about", "Acerca de"},
    {"help", "Ayuda"},
    {"close", "Cerrar"},
    {"next", "Siguiente"},
    {"previous", "Anterior"},
    {"first", "Primero"},
    {"last", "Último"}
};

// Wildlife category - Spanish
const PROGMEM TranslationEntry wildlifeTranslationsES[] = {
    {"species_detected", "Especie detectada"},
    {"confidence", "Confianza"},
    {"behavior_feeding", "Alimentándose"},
    {"behavior_resting", "Descansando"},
    {"behavior_moving", "Moviéndose"},
    {"behavior_alert", "Alerta"},
    {"detection_count", "Detecciones"},
    {"last_seen", "Última vez visto"},
    {"activity_high", "Actividad alta"},
    {"activity_normal", "Actividad normal"},
    {"activity_low", "Actividad baja"},
    {"activity_none", "Sin actividad"}
};

// Power category - Spanish
const PROGMEM TranslationEntry powerTranslationsES[] = {
    {"battery_level", "Nivel de batería"},
    {"battery_charging", "Cargando"},
    {"battery_discharging", "Descargando"},
    {"battery_full", "Completa"},
    {"battery_low", "Batería baja"},
    {"battery_critical", "Batería crítica"},
    {"solar_voltage", "Voltaje solar"},
    {"power_consumption", "Consumo de energía"},
    {"estimated_runtime", "Tiempo estimado"},
    {"power_mode_normal", "Modo normal"},
    {"power_mode_eco", "Modo eco"},
    {"power_mode_sleep", "Modo suspensión"}
};

// Network category - Spanish
const PROGMEM TranslationEntry networkTranslationsES[] = {
    {"wifi_connected", "WiFi conectado"},
    {"wifi_disconnected", "WiFi desconectado"},
    {"wifi_connecting", "Conectando a WiFi"},
    {"signal_strength", "Fuerza de señal"},
    {"ip_address", "Dirección IP"},
    {"lora_status", "Estado LoRa"},
    {"mesh_nodes", "Nodos de malla"},
    {"data_transmitted", "Datos transmitidos"},
    {"connection_quality", "Calidad de conexión"}
};

// Camera category - Spanish
const PROGMEM TranslationEntry cameraTranslationsES[] = {
    {"resolution", "Resolución"},
    {"quality", "Calidad"},
    {"frame_rate", "Tasa de fotogramas"},
    {"exposure", "Exposición"},
    {"brightness", "Brillo"},
    {"contrast", "Contraste"},
    {"saturation", "Saturación"},
    {"night_mode", "Modo nocturno"},
    {"motion_detection", "Detección de movimiento"},
    {"recording", "Grabando"},
    {"streaming", "Transmitiendo"},
    {"capture_photo", "Capturar foto"},
    {"start_recording", "Iniciar grabación"},
    {"stop_recording", "Detener grabación"}
};

// Storage category - Spanish
const PROGMEM TranslationEntry storageTranslationsES[] = {
    {"storage_used", "Almacenamiento usado"},
    {"storage_free", "Almacenamiento libre"},
    {"total_files", "Total de archivos"},
    {"file_size", "Tamaño de archivo"},
    {"file_date", "Fecha"},
    {"file_type", "Tipo"},
    {"storage_full", "Almacenamiento lleno"},
    {"storage_warning", "Advertencia de almacenamiento"},
    {"cleanup_required", "Limpieza requerida"},
    {"auto_delete", "Eliminación automática"}
};

// Settings category - Spanish
const PROGMEM TranslationEntry settingsTranslationsES[] = {
    {"general", "General"},
    {"camera_settings", "Configuración de cámara"},
    {"power_settings", "Configuración de energía"},
    {"network_settings", "Configuración de red"},
    {"storage_settings", "Configuración de almacenamiento"},
    {"time_zone", "Zona horaria"},
    {"date_format", "Formato de fecha"},
    {"time_format", "Formato de hora"},
    {"auto_capture", "Captura automática"},
    {"motion_sensitivity", "Sensibilidad de movimiento"},
    {"reset_settings", "Restablecer configuración"},
    {"factory_reset", "Restablecimiento de fábrica"},
    {"firmware_update", "Actualización de firmware"}
};

// ===============================
// BUILT-IN TRANSLATIONS - FRENCH
// ===============================

// System category - French
const PROGMEM TranslationEntry systemTranslationsFR[] = {
    {"status_online", "En ligne"},
    {"status_offline", "Hors ligne"},
    {"status_error", "Erreur"},
    {"status_warning", "Avertissement"},
    {"status_ok", "OK"},
    {"status_loading", "Chargement..."},
    {"error_network", "Erreur réseau"},
    {"error_camera", "Erreur caméra"},
    {"error_storage", "Erreur stockage"},
    {"error_power", "Erreur alimentation"},
    {"message_startup", "Démarrage du système..."},
    {"message_shutdown", "Arrêt du système..."},
    {"message_ready", "Système prêt"},
    {"uptime", "Temps de fonctionnement"},
    {"version", "Version"},
    {"device_id", "ID de l'appareil"}
};

// UI category - French
const PROGMEM TranslationEntry uiTranslationsFR[] = {
    {"home", "Accueil"},
    {"dashboard", "Tableau de bord"},
    {"gallery", "Galerie"},
    {"settings", "Paramètres"},
    {"analytics", "Analytiques"},
    {"live_view", "Vue en direct"},
    {"capture", "Capturer"},
    {"download", "Télécharger"},
    {"delete", "Supprimer"},
    {"save", "Enregistrer"},
    {"cancel", "Annuler"},
    {"ok", "OK"},
    {"yes", "Oui"},
    {"no", "Non"},
    {"refresh", "Actualiser"},
    {"loading", "Chargement"},
    {"search", "Rechercher"},
    {"filter", "Filtrer"},
    {"sort", "Trier"},
    {"export", "Exporter"},
    {"import", "Importer"},
    {"language", "Langue"},
    {"theme", "Thème"},
    {"about", "À propos"},
    {"help", "Aide"},
    {"close", "Fermer"},
    {"next", "Suivant"},
    {"previous", "Précédent"},
    {"first", "Premier"},
    {"last", "Dernier"}
};

// Wildlife category - French
const PROGMEM TranslationEntry wildlifeTranslationsFR[] = {
    {"species_detected", "Espèce détectée"},
    {"confidence", "Confiance"},
    {"behavior_feeding", "Se nourrit"},
    {"behavior_resting", "Se repose"},
    {"behavior_moving", "En mouvement"},
    {"behavior_alert", "Alerte"},
    {"detection_count", "Détections"},
    {"last_seen", "Vu pour la dernière fois"},
    {"activity_high", "Activité élevée"},
    {"activity_normal", "Activité normale"},
    {"activity_low", "Activité faible"},
    {"activity_none", "Aucune activité"}
};

// Power category - French
const PROGMEM TranslationEntry powerTranslationsFR[] = {
    {"battery_level", "Niveau de batterie"},
    {"battery_charging", "En charge"},
    {"battery_discharging", "Se décharge"},
    {"battery_full", "Complète"},
    {"battery_low", "Batterie faible"},
    {"battery_critical", "Batterie critique"},
    {"solar_voltage", "Tension solaire"},
    {"power_consumption", "Consommation d'énergie"},
    {"estimated_runtime", "Autonomie estimée"},
    {"power_mode_normal", "Mode normal"},
    {"power_mode_eco", "Mode éco"},
    {"power_mode_sleep", "Mode veille"}
};

// Network category - French
const PROGMEM TranslationEntry networkTranslationsFR[] = {
    {"wifi_connected", "WiFi connecté"},
    {"wifi_disconnected", "WiFi déconnecté"},
    {"wifi_connecting", "Connexion WiFi"},
    {"signal_strength", "Force du signal"},
    {"ip_address", "Adresse IP"},
    {"lora_status", "État LoRa"},
    {"mesh_nodes", "Nœuds maillés"},
    {"data_transmitted", "Données transmises"},
    {"connection_quality", "Qualité de connexion"}
};

// Camera category - French
const PROGMEM TranslationEntry cameraTranslationsFR[] = {
    {"resolution", "Résolution"},
    {"quality", "Qualité"},
    {"frame_rate", "Fréquence d'images"},
    {"exposure", "Exposition"},
    {"brightness", "Luminosité"},
    {"contrast", "Contraste"},
    {"saturation", "Saturation"},
    {"night_mode", "Mode nuit"},
    {"motion_detection", "Détection de mouvement"},
    {"recording", "Enregistrement"},
    {"streaming", "Diffusion"},
    {"capture_photo", "Capturer photo"},
    {"start_recording", "Démarrer enregistrement"},
    {"stop_recording", "Arrêter enregistrement"}
};

// Storage category - French
const PROGMEM TranslationEntry storageTranslationsFR[] = {
    {"storage_used", "Stockage utilisé"},
    {"storage_free", "Stockage libre"},
    {"total_files", "Total des fichiers"},
    {"file_size", "Taille du fichier"},
    {"file_date", "Date"},
    {"file_type", "Type"},
    {"storage_full", "Stockage plein"},
    {"storage_warning", "Avertissement stockage"},
    {"cleanup_required", "Nettoyage requis"},
    {"auto_delete", "Suppression automatique"}
};

// Settings category - French
const PROGMEM TranslationEntry settingsTranslationsFR[] = {
    {"general", "Général"},
    {"camera_settings", "Paramètres caméra"},
    {"power_settings", "Paramètres alimentation"},
    {"network_settings", "Paramètres réseau"},
    {"storage_settings", "Paramètres stockage"},
    {"time_zone", "Fuseau horaire"},
    {"date_format", "Format de date"},
    {"time_format", "Format d'heure"},
    {"auto_capture", "Capture automatique"},
    {"motion_sensitivity", "Sensibilité mouvement"},
    {"reset_settings", "Réinitialiser paramètres"},
    {"factory_reset", "Réinitialisation usine"},
    {"firmware_update", "Mise à jour firmware"}
};

// ===============================
// BUILT-IN TRANSLATIONS - GERMAN
// ===============================

// System category - German
const PROGMEM TranslationEntry systemTranslationsDE[] = {
    {"status_online", "Online"},
    {"status_offline", "Offline"},
    {"status_error", "Fehler"},
    {"status_warning", "Warnung"},
    {"status_ok", "OK"},
    {"status_loading", "Lädt..."},
    {"error_network", "Netzwerkfehler"},
    {"error_camera", "Kamerafehler"},
    {"error_storage", "Speicherfehler"},
    {"error_power", "Stromfehler"},
    {"message_startup", "System startet..."},
    {"message_shutdown", "System fährt herunter..."},
    {"message_ready", "System bereit"},
    {"uptime", "Betriebszeit"},
    {"version", "Version"},
    {"device_id", "Geräte-ID"}
};

// UI category - German
const PROGMEM TranslationEntry uiTranslationsDE[] = {
    {"home", "Startseite"},
    {"dashboard", "Dashboard"},
    {"gallery", "Galerie"},
    {"settings", "Einstellungen"},
    {"analytics", "Analytik"},
    {"live_view", "Live-Ansicht"},
    {"capture", "Aufnehmen"},
    {"download", "Herunterladen"},
    {"delete", "Löschen"},
    {"save", "Speichern"},
    {"cancel", "Abbrechen"},
    {"ok", "OK"},
    {"yes", "Ja"},
    {"no", "Nein"},
    {"refresh", "Aktualisieren"},
    {"loading", "Lädt"},
    {"search", "Suchen"},
    {"filter", "Filter"},
    {"sort", "Sortieren"},
    {"export", "Exportieren"},
    {"import", "Importieren"},
    {"language", "Sprache"},
    {"theme", "Design"},
    {"about", "Über"},
    {"help", "Hilfe"},
    {"close", "Schließen"},
    {"next", "Weiter"},
    {"previous", "Zurück"},
    {"first", "Erste"},
    {"last", "Letzte"}
};

// Wildlife category - German
const PROGMEM TranslationEntry wildlifeTranslationsDE[] = {
    {"species_detected", "Art erkannt"},
    {"confidence", "Vertrauen"},
    {"behavior_feeding", "Fütterung"},
    {"behavior_resting", "Ruht"},
    {"behavior_moving", "Bewegt sich"},
    {"behavior_alert", "Alarm"},
    {"detection_count", "Erkennungen"},
    {"last_seen", "Zuletzt gesehen"},
    {"activity_high", "Hohe Aktivität"},
    {"activity_normal", "Normale Aktivität"},
    {"activity_low", "Niedrige Aktivität"},
    {"activity_none", "Keine Aktivität"}
};

// Power category - German
const PROGMEM TranslationEntry powerTranslationsDE[] = {
    {"battery_level", "Batteriestand"},
    {"battery_charging", "Lädt"},
    {"battery_discharging", "Entlädt"},
    {"battery_full", "Voll"},
    {"battery_low", "Batterie schwach"},
    {"battery_critical", "Batterie kritisch"},
    {"solar_voltage", "Solar-Spannung"},
    {"power_consumption", "Stromverbrauch"},
    {"estimated_runtime", "Geschätzte Laufzeit"},
    {"power_mode_normal", "Normaler Modus"},
    {"power_mode_eco", "Öko-Modus"},
    {"power_mode_sleep", "Schlafmodus"}
};

// Network category - German
const PROGMEM TranslationEntry networkTranslationsDE[] = {
    {"wifi_connected", "WiFi verbunden"},
    {"wifi_disconnected", "WiFi getrennt"},
    {"wifi_connecting", "WiFi verbinden"},
    {"signal_strength", "Signalstärke"},
    {"ip_address", "IP-Adresse"},
    {"lora_status", "LoRa-Status"},
    {"mesh_nodes", "Mesh-Knoten"},
    {"data_transmitted", "Daten übertragen"},
    {"connection_quality", "Verbindungsqualität"}
};

// Camera category - German
const PROGMEM TranslationEntry cameraTranslationsDE[] = {
    {"resolution", "Auflösung"},
    {"quality", "Qualität"},
    {"frame_rate", "Bildrate"},
    {"exposure", "Belichtung"},
    {"brightness", "Helligkeit"},
    {"contrast", "Kontrast"},
    {"saturation", "Sättigung"},
    {"night_mode", "Nachtmodus"},
    {"motion_detection", "Bewegungserkennung"},
    {"recording", "Aufnahme"},
    {"streaming", "Streaming"},
    {"capture_photo", "Foto aufnehmen"},
    {"start_recording", "Aufnahme starten"},
    {"stop_recording", "Aufnahme stoppen"}
};

// Storage category - German
const PROGMEM TranslationEntry storageTranslationsDE[] = {
    {"storage_used", "Speicher verwendet"},
    {"storage_free", "Speicher frei"},
    {"total_files", "Gesamt Dateien"},
    {"file_size", "Dateigröße"},
    {"file_date", "Datum"},
    {"file_type", "Typ"},
    {"storage_full", "Speicher voll"},
    {"storage_warning", "Speicher-Warnung"},
    {"cleanup_required", "Bereinigung erforderlich"},
    {"auto_delete", "Automatisches Löschen"}
};

// Settings category - German
const PROGMEM TranslationEntry settingsTranslationsDE[] = {
    {"general", "Allgemein"},
    {"camera_settings", "Kamera-Einstellungen"},
    {"power_settings", "Energie-Einstellungen"},
    {"network_settings", "Netzwerk-Einstellungen"},
    {"storage_settings", "Speicher-Einstellungen"},
    {"time_zone", "Zeitzone"},
    {"date_format", "Datumsformat"},
    {"time_format", "Zeitformat"},
    {"auto_capture", "Automatische Aufnahme"},
    {"motion_sensitivity", "Bewegungsempfindlichkeit"},
    {"reset_settings", "Einstellungen zurücksetzen"},
    {"factory_reset", "Werkseinstellungen"},
    {"firmware_update", "Firmware-Update"}
};

// Translation table arrays for each language and category
const TranslationEntry* const PROGMEM translationTables[][static_cast<int>(TranslationCategory::COUNT)] = {
    // English
    {
        systemTranslationsEN, uiTranslationsEN, wildlifeTranslationsEN,
        powerTranslationsEN, networkTranslationsEN, cameraTranslationsEN,
        storageTranslationsEN, settingsTranslationsEN
    },
    // Spanish
    {
        systemTranslationsES, uiTranslationsES, wildlifeTranslationsES,
        powerTranslationsES, networkTranslationsES, cameraTranslationsES,
        storageTranslationsES, settingsTranslationsES
    },
    // French
    {
        systemTranslationsFR, uiTranslationsFR, wildlifeTranslationsFR,
        powerTranslationsFR, networkTranslationsFR, cameraTranslationsFR,
        storageTranslationsFR, settingsTranslationsFR
    },
    // German
    {
        systemTranslationsDE, uiTranslationsDE, wildlifeTranslationsDE,
        powerTranslationsDE, networkTranslationsDE, cameraTranslationsDE,
        storageTranslationsDE, settingsTranslationsDE
    }
};

// Translation counts for each language and category
const size_t PROGMEM translationCounts[][static_cast<int>(TranslationCategory::COUNT)] = {
    // English
    {
        sizeof(systemTranslationsEN) / sizeof(TranslationEntry),
        sizeof(uiTranslationsEN) / sizeof(TranslationEntry),
        sizeof(wildlifeTranslationsEN) / sizeof(TranslationEntry),
        sizeof(powerTranslationsEN) / sizeof(TranslationEntry),
        sizeof(networkTranslationsEN) / sizeof(TranslationEntry),
        sizeof(cameraTranslationsEN) / sizeof(TranslationEntry),
        sizeof(storageTranslationsEN) / sizeof(TranslationEntry),
        sizeof(settingsTranslationsEN) / sizeof(TranslationEntry)
    },
    // Spanish
    {
        sizeof(systemTranslationsES) / sizeof(TranslationEntry),
        sizeof(uiTranslationsES) / sizeof(TranslationEntry),
        sizeof(wildlifeTranslationsES) / sizeof(TranslationEntry),
        sizeof(powerTranslationsES) / sizeof(TranslationEntry),
        sizeof(networkTranslationsES) / sizeof(TranslationEntry),
        sizeof(cameraTranslationsES) / sizeof(TranslationEntry),
        sizeof(storageTranslationsES) / sizeof(TranslationEntry),
        sizeof(settingsTranslationsES) / sizeof(TranslationEntry)
    },
    // French
    {
        sizeof(systemTranslationsFR) / sizeof(TranslationEntry),
        sizeof(uiTranslationsFR) / sizeof(TranslationEntry),
        sizeof(wildlifeTranslationsFR) / sizeof(TranslationEntry),
        sizeof(powerTranslationsFR) / sizeof(TranslationEntry),
        sizeof(networkTranslationsFR) / sizeof(TranslationEntry),
        sizeof(cameraTranslationsFR) / sizeof(TranslationEntry),
        sizeof(storageTranslationsFR) / sizeof(TranslationEntry),
        sizeof(settingsTranslationsFR) / sizeof(TranslationEntry)
    },
    // German
    {
        sizeof(systemTranslationsDE) / sizeof(TranslationEntry),
        sizeof(uiTranslationsDE) / sizeof(TranslationEntry),
        sizeof(wildlifeTranslationsDE) / sizeof(TranslationEntry),
        sizeof(powerTranslationsDE) / sizeof(TranslationEntry),
        sizeof(networkTranslationsDE) / sizeof(TranslationEntry),
        sizeof(cameraTranslationsDE) / sizeof(TranslationEntry),
        sizeof(storageTranslationsDE) / sizeof(TranslationEntry),
        sizeof(settingsTranslationsDE) / sizeof(TranslationEntry)
    }
};

// ===============================
// LANGUAGE MANAGER IMPLEMENTATION
// ===============================

LanguageManager::LanguageManager() 
    : currentLanguage_(LanguageCode::EN)
    , defaultLanguage_(LanguageCode::EN)
    , cacheStats_{0, 0, 0, 0}
{
}

bool LanguageManager::begin(LanguageCode defaultLang) {
    defaultLanguage_ = defaultLang;
    currentLanguage_ = defaultLang;
    
    // Initialize preferences
    if (!preferences_.begin(PREFERENCE_NAMESPACE, false)) {
        Serial.println("Failed to initialize language preferences");
        return false;
    }
    
    // Load saved language preference
    loadLanguagePreference();
    
    Serial.printf("Language Manager initialized - Current: %s\n", 
                  languageCodeToString(currentLanguage_));
    
    return true;
}

bool LanguageManager::setLanguage(LanguageCode lang) {
    if (!isLanguageSupported(lang)) {
        Serial.printf("Unsupported language code: %d\n", static_cast<int>(lang));
        return false;
    }
    
    currentLanguage_ = lang;
    clearCache(); // Clear cache when language changes
    
    Serial.printf("Language changed to: %s\n", languageCodeToString(lang));
    
    return true;
}

bool LanguageManager::setLanguage(const String& langCode) {
    LanguageCode lang = stringToLanguageCode(langCode);
    if (lang == LanguageCode::COUNT) {
        Serial.printf("Invalid language code string: %s\n", langCode.c_str());
        return false;
    }
    
    return setLanguage(lang);
}

String LanguageManager::getLanguageString(LanguageCode lang) const {
    if (lang == static_cast<LanguageCode>(-1)) {
        lang = currentLanguage_;
    }
    return String(languageCodeToString(lang));
}

const LanguageInfo& LanguageManager::getLanguageInfo(LanguageCode lang) const {
    if (lang == static_cast<LanguageCode>(-1)) {
        lang = currentLanguage_;
    }
    
    if (static_cast<int>(lang) >= 0 && lang < LanguageCode::COUNT) {
        return languageInfos[static_cast<int>(lang)];
    }
    
    return languageInfos[0]; // Default to English
}

String LanguageManager::translate(TranslationCategory category, const char* key) const {
    if (!key) return String();
    
    // Create cache key
    String cacheKey = String(static_cast<int>(currentLanguage_)) + ":" + 
                     String(static_cast<int>(category)) + ":" + String(key);
    
    // Check cache first
    auto it = translationCache_.find(cacheKey);
    if (it != translationCache_.end()) {
        cacheStats_.cacheHits++;
        return it->second;
    }
    
    cacheStats_.cacheMisses++;
    
    // Look up in built-in translations
    const TranslationEntry* translations = getBuiltinTranslations(currentLanguage_, category);
    size_t count = getBuiltinTranslationCount(currentLanguage_, category);
    
    if (translations && count > 0) {
        for (size_t i = 0; i < count; i++) {
            if (strcmp_P(key, translations[i].key) == 0) {
                String translation = String(FPSTR(translations[i].translation));
                addToCache(cacheKey, translation);
                return translation;
            }
        }
    }
    
    // Fallback to default language if current is not English
    if (currentLanguage_ != defaultLanguage_) {
        const TranslationEntry* defaultTranslations = getBuiltinTranslations(defaultLanguage_, category);
        size_t defaultCount = getBuiltinTranslationCount(defaultLanguage_, category);
        
        if (defaultTranslations && defaultCount > 0) {
            for (size_t i = 0; i < defaultCount; i++) {
                if (strcmp_P(key, defaultTranslations[i].key) == 0) {
                    String translation = String(FPSTR(defaultTranslations[i].translation));
                    addToCache(cacheKey, translation);
                    return translation;
                }
            }
        }
    }
    
    // Return key if no translation found
    String fallback = String(key);
    addToCache(cacheKey, fallback);
    return fallback;
}

String LanguageManager::translate(const char* fullKey) const {
    if (!fullKey) return String();
    
    TranslationCategory category = parseCategoryFromKey(fullKey);
    String key = extractKeyFromFullKey(fullKey);
    
    return translate(category, key.c_str());
}

String LanguageManager::translatef(TranslationCategory category, const char* key, 
                                  const std::vector<String>& params) const {
    String translation = translate(category, key);
    
    // Replace parameters {0}, {1}, etc.
    for (size_t i = 0; i < params.size(); i++) {
        String placeholder = "{" + String(i) + "}";
        translation.replace(placeholder, params[i]);
    }
    
    return translation;
}

std::vector<LanguageCode> LanguageManager::getAvailableLanguages() const {
    std::vector<LanguageCode> languages;
    for (int i = 0; i < static_cast<int>(LanguageCode::COUNT); i++) {
        languages.push_back(static_cast<LanguageCode>(i));
    }
    return languages;
}

bool LanguageManager::isLanguageSupported(LanguageCode lang) const {
    return static_cast<int>(lang) >= 0 && lang < LanguageCode::COUNT;
}

String LanguageManager::getTranslationsJSON(TranslationCategory category) const {
    DynamicJsonDocument doc(4096);
    JsonObject root = doc.to<JsonObject>();
    
    // Add language info
    const LanguageInfo& langInfo = getLanguageInfo(currentLanguage_);
    root["language"] = languageCodeToString(currentLanguage_);
    root["languageName"] = langInfo.name;
    root["nativeName"] = langInfo.nativeName;
    root["flag"] = langInfo.flag;
    root["rtl"] = langInfo.rtl;
    
    JsonObject translations = root.createNestedObject("translations");
    
    if (category == TranslationCategory::COUNT) {
        // Include all categories
        for (int cat = 0; cat < static_cast<int>(TranslationCategory::COUNT); cat++) {
            TranslationCategory currentCat = static_cast<TranslationCategory>(cat);
            const TranslationEntry* entries = getBuiltinTranslations(currentLanguage_, currentCat);
            size_t count = getBuiltinTranslationCount(currentLanguage_, currentCat);
            
            if (entries && count > 0) {
                for (size_t i = 0; i < count; i++) {
                    String fullKey = String(static_cast<int>(currentCat)) + "." + 
                                   String(FPSTR(entries[i].key));
                    translations[fullKey] = String(FPSTR(entries[i].translation));
                }
            }
        }
    } else {
        // Include specific category
        const TranslationEntry* entries = getBuiltinTranslations(currentLanguage_, category);
        size_t count = getBuiltinTranslationCount(currentLanguage_, category);
        
        if (entries && count > 0) {
            for (size_t i = 0; i < count; i++) {
                String fullKey = String(static_cast<int>(category)) + "." + 
                               String(FPSTR(entries[i].key));
                translations[fullKey] = String(FPSTR(entries[i].translation));
            }
        }
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}

bool LanguageManager::saveLanguagePreference() {
    if (preferences_.putUChar(LANGUAGE_PREFERENCE_KEY, static_cast<uint8_t>(currentLanguage_))) {
        Serial.printf("Language preference saved: %s\n", languageCodeToString(currentLanguage_));
        return true;
    }
    return false;
}

bool LanguageManager::loadLanguagePreference() {
    uint8_t savedLang = preferences_.getUChar(LANGUAGE_PREFERENCE_KEY, 
                                             static_cast<uint8_t>(defaultLanguage_));
    
    LanguageCode lang = static_cast<LanguageCode>(savedLang);
    if (isLanguageSupported(lang)) {
        currentLanguage_ = lang;
        Serial.printf("Language preference loaded: %s\n", languageCodeToString(lang));
        return true;
    }
    
    return false;
}

String LanguageManager::formatDate(time_t timestamp) const {
    const LanguageInfo& langInfo = getLanguageInfo(currentLanguage_);
    struct tm* timeInfo = localtime(&timestamp);
    char buffer[32];
    
    strftime(buffer, sizeof(buffer), langInfo.dateFormat, timeInfo);
    return String(buffer);
}

String LanguageManager::formatTime(time_t timestamp) const {
    const LanguageInfo& langInfo = getLanguageInfo(currentLanguage_);
    struct tm* timeInfo = localtime(&timestamp);
    char buffer[32];
    
    strftime(buffer, sizeof(buffer), langInfo.timeFormat, timeInfo);
    return String(buffer);
}

size_t LanguageManager::getMemoryUsage() const {
    size_t usage = sizeof(*this);
    
    // Add cache memory usage
    for (const auto& pair : translationCache_) {
        usage += pair.first.length() + pair.second.length() + 
                sizeof(std::pair<String, String>);
    }
    
    return usage;
}

// Private methods

const TranslationEntry* LanguageManager::getBuiltinTranslations(LanguageCode lang, 
                                                               TranslationCategory category) const {
    if (!isLanguageSupported(lang) || 
        static_cast<int>(category) >= static_cast<int>(TranslationCategory::COUNT)) {
        return nullptr;
    }
    
    return pgm_read_ptr(&translationTables[static_cast<int>(lang)][static_cast<int>(category)]);
}

size_t LanguageManager::getBuiltinTranslationCount(LanguageCode lang, 
                                                  TranslationCategory category) const {
    if (!isLanguageSupported(lang) || 
        static_cast<int>(category) >= static_cast<int>(TranslationCategory::COUNT)) {
        return 0;
    }
    
    return pgm_read_word(&translationCounts[static_cast<int>(lang)][static_cast<int>(category)]);
}

TranslationCategory LanguageManager::parseCategoryFromKey(const char* fullKey) const {
    if (!fullKey) return TranslationCategory::SYSTEM;
    
    String key = String(fullKey);
    int dotIndex = key.indexOf('.');
    if (dotIndex < 0) return TranslationCategory::SYSTEM;
    
    String categoryStr = key.substring(0, dotIndex);
    
    // Map category names to enum values
    if (categoryStr == "system") return TranslationCategory::SYSTEM;
    if (categoryStr == "ui") return TranslationCategory::UI;
    if (categoryStr == "wildlife") return TranslationCategory::WILDLIFE;
    if (categoryStr == "power") return TranslationCategory::POWER;
    if (categoryStr == "network") return TranslationCategory::NETWORK;
    if (categoryStr == "camera") return TranslationCategory::CAMERA;
    if (categoryStr == "storage") return TranslationCategory::STORAGE;
    if (categoryStr == "settings") return TranslationCategory::SETTINGS;
    
    // Try numeric category
    int categoryNum = categoryStr.toInt();
    if (categoryNum >= 0 && categoryNum < static_cast<int>(TranslationCategory::COUNT)) {
        return static_cast<TranslationCategory>(categoryNum);
    }
    
    return TranslationCategory::SYSTEM;
}

String LanguageManager::extractKeyFromFullKey(const char* fullKey) const {
    if (!fullKey) return String();
    
    String key = String(fullKey);
    int dotIndex = key.indexOf('.');
    if (dotIndex < 0) return key;
    
    return key.substring(dotIndex + 1);
}

void LanguageManager::addToCache(const String& key, const String& translation) const {
    if (translationCache_.size() >= MAX_CACHE_SIZE) {
        // Remove oldest entry (simple LRU approximation)
        auto it = translationCache_.begin();
        translationCache_.erase(it);
    }
    
    translationCache_[key] = translation;
    cacheStats_.totalEntries = translationCache_.size();
    cacheStats_.memoryUsed = getMemoryUsage();
}

void LanguageManager::clearCache() {
    translationCache_.clear();
    cacheStats_.totalEntries = 0;
    cacheStats_.cacheHits = 0;
    cacheStats_.cacheMisses = 0;
    cacheStats_.memoryUsed = getMemoryUsage();
}

const char* LanguageManager::languageCodeToString(LanguageCode lang) {
    switch (lang) {
        case LanguageCode::EN: return "en";
        case LanguageCode::ES: return "es";
        case LanguageCode::FR: return "fr";
        case LanguageCode::DE: return "de";
        default: return "en";
    }
}

LanguageCode LanguageManager::stringToLanguageCode(const String& langStr) {
    String lower = langStr;
    lower.toLowerCase();
    
    if (lower == "en") return LanguageCode::EN;
    if (lower == "es") return LanguageCode::ES;
    if (lower == "fr") return LanguageCode::FR;
    if (lower == "de") return LanguageCode::DE;
    
    return LanguageCode::COUNT; // Invalid
}