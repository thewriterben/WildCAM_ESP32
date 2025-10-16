# WiFi Credentials Storage - Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         WiFiManager Class                                │
│                                                                           │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │                    Public API Methods                              │  │
│  │                                                                     │  │
│  │  bool saveWiFiCredentials(ssid, password)                         │  │
│  │    ├─ Validate SSID (non-empty)                                   │  │
│  │    ├─ Encrypt password (XOR)                                      │  │
│  │    ├─ Open Preferences("wifi_config", write)                      │  │
│  │    ├─ Write ssid and encrypted password                           │  │
│  │    └─ Return success/failure                                      │  │
│  │                                                                     │  │
│  │  bool loadWiFiCredentials(ssid&, password&)                       │  │
│  │    ├─ Open Preferences("wifi_config", read-only)                  │  │
│  │    ├─ Read ssid and encrypted password                            │  │
│  │    ├─ Decrypt password (XOR)                                      │  │
│  │    └─ Return success/failure                                      │  │
│  │                                                                     │  │
│  │  void clearWiFiCredentials()                                       │  │
│  │    ├─ Open Preferences("wifi_config", write)                      │  │
│  │    ├─ Clear all data in namespace                                 │  │
│  │    └─ Close Preferences                                           │  │
│  └───────────────────────────────────────────────────────────────────┘  │
│                                                                           │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │                  Private Helper Methods                            │  │
│  │                                                                     │  │
│  │  String encryptPassword(password)                                 │  │
│  │    └─ XOR with key: {0xA5,0x3C,0x7E,0x91,0x5D,0xB2,0x48,0xF6}   │  │
│  │                                                                     │  │
│  │  String decryptPassword(encryptedPassword)                        │  │
│  │    └─ XOR with same key (symmetric)                              │  │
│  └───────────────────────────────────────────────────────────────────┘  │
│                                                                           │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │              Automatic Credential Loading                          │  │
│  │                                                                     │  │
│  │  void applyConfigurationSettings()                                │  │
│  │    ├─ Load config.h values (WIFI_SSID, WIFI_PASSWORD)            │  │
│  │    └─ If config.ssid is empty:                                    │  │
│  │       └─ loadWiFiCredentials() from NVS                           │  │
│  └───────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                                    │ Uses
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     ESP32 Preferences Library                            │
│                       (NVS - Non-Volatile Storage)                       │
│                                                                           │
│  Namespace: "wifi_config"                                                │
│  ┌────────────────────────────────────────────────────────────────┐    │
│  │  Key: "ssid"      │  Value: "MyNetwork"                        │    │
│  ├────────────────────────────────────────────────────────────────┤    │
│  │  Key: "password"  │  Value: "\xC9\x5E\x2A..." (encrypted)      │    │
│  └────────────────────────────────────────────────────────────────┘    │
│                                                                           │
│  Storage: 20KB NVS partition (persistent across reboots)                 │
└─────────────────────────────────────────────────────────────────────────┘


┌─────────────────────────────────────────────────────────────────────────┐
│                          Usage Flow                                      │
└─────────────────────────────────────────────────────────────────────────┘

Boot Sequence:
═══════════════

1. WiFiManager::WiFiManager()
   └─> applyConfigurationSettings()
       ├─> Read config.h values
       └─> If WIFI_SSID empty:
           └─> loadWiFiCredentials()
               └─> Read from NVS


Save Credentials Flow:
══════════════════════

User → saveWiFiCredentials("Network", "Pass123")
  │
  ├─> Validate SSID (non-empty) ✓
  │
  ├─> encryptPassword("Pass123")
  │   └─> XOR: "Pass123" ⊕ key = "\xE5\x0E..."
  │
  ├─> Preferences.begin("wifi_config", write)
  │
  ├─> Preferences.putString("ssid", "Network")
  ├─> Preferences.putString("password", "\xE5\x0E...")
  │
  └─> Preferences.end()
      └─> Return true ✓


Load Credentials Flow:
══════════════════════

User → loadWiFiCredentials(ssid, password)
  │
  ├─> Preferences.begin("wifi_config", read-only)
  │
  ├─> ssid = Preferences.getString("ssid")
  │   └─> "Network"
  │
  ├─> encrypted = Preferences.getString("password")
  │   └─> "\xE5\x0E..."
  │
  ├─> Preferences.end()
  │
  ├─> password = decryptPassword(encrypted)
  │   └─> XOR: "\xE5\x0E..." ⊕ key = "Pass123"
  │
  └─> Return true ✓


Factory Reset Flow:
═══════════════════

User → clearWiFiCredentials()
  │
  ├─> Preferences.begin("wifi_config", write)
  │
  ├─> Preferences.clear()
  │   └─> Removes all keys in namespace
  │
  └─> Preferences.end()


┌─────────────────────────────────────────────────────────────────────────┐
│                         Error Handling                                   │
└─────────────────────────────────────────────────────────────────────────┘

saveWiFiCredentials():
  • Empty SSID → Return false, log error
  • Preferences.begin() fails → Return false, log error
  • putString() fails → Return false, log error

loadWiFiCredentials():
  • Preferences.begin() fails → Return false, empty strings
  • SSID not found → Return false, empty strings
  • Password not found → Return false, empty strings
  • Corrupted data → Gracefully handled, empty strings

clearWiFiCredentials():
  • Preferences.begin() fails → Log error, return
  • Clear operation successful → Log success


┌─────────────────────────────────────────────────────────────────────────┐
│                    Integration Example                                   │
└─────────────────────────────────────────────────────────────────────────┘

#include "wifi_manager.h"

void setup() {
    WiFiManager wifiManager;
    wifiManager.init();  // Auto-loads saved credentials
    
    // Option 1: Use saved credentials
    if (wifiManager.connect()) {
        Serial.println("Connected!");
    }
    
    // Option 2: Save new credentials
    wifiManager.saveWiFiCredentials("NewNetwork", "NewPass");
    wifiManager.connect();
    
    // Option 3: Factory reset
    wifiManager.clearWiFiCredentials();
}


┌─────────────────────────────────────────────────────────────────────────┐
│                         Security Model                                   │
└─────────────────────────────────────────────────────────────────────────┘

Password Storage:
  Plain Text: "MyPassword123"
           ↓ XOR Encryption
  Encrypted: "\xE7\x0E\x12..." (obfuscated)
           ↓ Store in NVS
  NVS Data: Key-Value pair in flash memory

Password Retrieval:
  NVS Data: Key-Value pair in flash memory
           ↓ Read from NVS
  Encrypted: "\xE7\x0E\x12..." (obfuscated)
           ↓ XOR Decryption
  Plain Text: "MyPassword123"

Security Level:
  ✓ Protects against casual inspection
  ✓ Prevents clear-text password storage
  ✗ Not cryptographically secure
  ✗ Vulnerable to firmware reverse engineering

Enhancement Options:
  • ESP32 Flash Encryption (hardware-level)
  • AES-256 encryption (software-level)
  • Secure Element (hardware key storage)
