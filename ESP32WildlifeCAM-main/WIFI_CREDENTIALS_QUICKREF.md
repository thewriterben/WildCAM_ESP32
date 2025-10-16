# WiFi Credentials Storage - Quick Reference

## TL;DR

Three functions to manage WiFi credentials in ESP32 non-volatile storage:

```cpp
// Save credentials
wifiManager.saveWiFiCredentials("MyNetwork", "MyPassword");

// Load credentials
String ssid, password;
wifiManager.loadWiFiCredentials(ssid, password);

// Clear credentials (factory reset)
wifiManager.clearWiFiCredentials();
```

## Quick Start

### 1. Basic Usage

```cpp
#include "wifi_manager.h"

WiFiManager wifiManager;

void setup() {
    Serial.begin(115200);
    wifiManager.init();
    
    // Auto-connects using saved credentials
    if (wifiManager.connect()) {
        Serial.println("Connected!");
    }
}
```

### 2. Save Credentials

```cpp
if (wifiManager.saveWiFiCredentials("NetworkName", "Password123")) {
    Serial.println("Saved!");
}
```

### 3. Load Credentials

```cpp
String ssid, password;
if (wifiManager.loadWiFiCredentials(ssid, password)) {
    Serial.printf("SSID: %s\n", ssid.c_str());
    // Password is available but should not be printed
}
```

### 4. Factory Reset

```cpp
wifiManager.clearWiFiCredentials();
Serial.println("All credentials cleared!");
```

## Common Patterns

### Pattern 1: Auto-Connect on Boot

```cpp
void setup() {
    WiFiManager mgr;
    mgr.init();  // Automatically loads saved credentials
    
    if (mgr.connect()) {
        // Connected using saved credentials
    } else {
        // No saved credentials or connection failed
        mgr.startAccessPoint("Setup");
    }
}
```

### Pattern 2: Save and Connect

```cpp
void connectToNewNetwork(const char* ssid, const char* password) {
    wifiManager.saveWiFiCredentials(ssid, password);
    wifiManager.connect(ssid, password);
}
```

### Pattern 3: Check for Saved Credentials

```cpp
String ssid, password;
if (wifiManager.loadWiFiCredentials(ssid, password)) {
    // Credentials exist, use them
    wifiManager.connect(ssid, password);
} else {
    // No credentials, enter setup mode
    wifiManager.startAccessPoint("Setup");
}
```

### Pattern 4: Update Credentials

```cpp
void updateCredentials(const char* newSSID, const char* newPassword) {
    wifiManager.disconnect();
    wifiManager.saveWiFiCredentials(newSSID, newPassword);
    wifiManager.connect(newSSID, newPassword);
}
```

### Pattern 5: Conditional Save

```cpp
void connectAndSave(const char* ssid, const char* password, bool save) {
    if (wifiManager.connect(ssid, password)) {
        if (save) {
            wifiManager.saveWiFiCredentials(ssid, password);
        }
    }
}
```

## Function Reference

### saveWiFiCredentials()

**Signature:**
```cpp
bool saveWiFiCredentials(const char* ssid, const char* password)
```

**Parameters:**
- `ssid` - WiFi network name (required, non-empty)
- `password` - WiFi password (can be empty for open networks)

**Returns:**
- `true` - Credentials saved successfully
- `false` - Failed (empty SSID or storage error)

**Example:**
```cpp
if (wifiManager.saveWiFiCredentials("HomeWiFi", "secret123")) {
    Serial.println("✓ Saved");
} else {
    Serial.println("✗ Failed");
}
```

---

### loadWiFiCredentials()

**Signature:**
```cpp
bool loadWiFiCredentials(String& ssid, String& password)
```

**Parameters:**
- `ssid` - Reference to String to receive SSID
- `password` - Reference to String to receive password

**Returns:**
- `true` - Credentials loaded successfully
- `false` - No credentials found or error

**Example:**
```cpp
String ssid, password;
if (wifiManager.loadWiFiCredentials(ssid, password)) {
    Serial.printf("Found: %s\n", ssid.c_str());
} else {
    Serial.println("No saved credentials");
}
```

---

### clearWiFiCredentials()

**Signature:**
```cpp
void clearWiFiCredentials()
```

**Parameters:** None

**Returns:** None (always succeeds)

**Example:**
```cpp
wifiManager.clearWiFiCredentials();
Serial.println("Factory reset complete");
```

## Storage Details

- **Technology**: ESP32 Preferences (NVS)
- **Namespace**: `wifi_config`
- **Keys**: `ssid`, `password`
- **Encryption**: XOR obfuscation
- **Persistence**: Survives reboots and power loss
- **Size**: ~20KB NVS partition

## Error Handling

### Save Errors

```cpp
if (!wifiManager.saveWiFiCredentials("", "pass")) {
    // Error: Empty SSID not allowed
}

if (!wifiManager.saveWiFiCredentials(nullptr, "pass")) {
    // Error: NULL SSID not allowed
}
```

### Load Errors

```cpp
String ssid, password;
if (!wifiManager.loadWiFiCredentials(ssid, password)) {
    // No credentials stored
    // ssid and password are empty strings
}
```

### Clear Never Fails

```cpp
// Always safe to call
wifiManager.clearWiFiCredentials();
```

## Testing

### Run Unit Tests

```bash
# Run all tests
pio test -e native

# Run specific test
pio test -e native -f test_wifi_credentials
```

### Interactive Testing

```bash
# Flash example to device
pio run -e esp32cam -t upload

# Open serial monitor
pio device monitor
```

## Debugging

Enable debug output in `config.h`:

```cpp
#define DEBUG_ENABLED true
```

Debug messages:
- `"WiFi credentials saved successfully (SSID: xxx)"`
- `"Loaded WiFi credentials from NVS storage"`
- `"No WiFi credentials found in storage"`
- `"WiFi credentials cleared from storage"`

## Migration

### From Hardcoded Config

```cpp
// Old code (config.h)
#define WIFI_SSID "HardcodedSSID"
#define WIFI_PASSWORD "HardcodedPass"

// New code (migrate to NVS)
void migrateToNVS() {
    wifiManager.init();
    
    String saved;
    if (!wifiManager.loadWiFiCredentials(saved, saved)) {
        // No NVS credentials, save from config
        wifiManager.saveWiFiCredentials(WIFI_SSID, WIFI_PASSWORD);
    }
}
```

### From Other Storage

```cpp
// Migrate from custom storage to NVS
void migrateCredentials() {
    String oldSSID = readFromCustomStorage();
    String oldPassword = readPasswordFromCustomStorage();
    
    wifiManager.saveWiFiCredentials(oldSSID.c_str(), oldPassword.c_str());
    deleteCustomStorage();
}
```

## Best Practices

✅ **DO:**
- Always check return values
- Handle missing credentials gracefully
- Clear credentials on factory reset
- Use saved credentials for auto-connect

❌ **DON'T:**
- Print passwords to serial
- Save credentials on every boot
- Ignore return values
- Store passwords in plain text elsewhere

## Security Notes

**Current Implementation:**
- XOR obfuscation (not cryptographically secure)
- Protects against casual inspection
- Suitable for non-critical applications

**For Production:**
- Enable ESP32 Flash Encryption
- Use AES-256 encryption
- Implement secure boot
- Consider hardware secure element

See `WIFI_CREDENTIALS_STORAGE.md` for details.

## Troubleshooting

### Credentials Not Saving

**Check:**
1. SSID is not empty
2. NVS partition exists in `partitions.csv`
3. Serial output for errors
4. Return value is `true`

### Credentials Not Loading

**Check:**
1. Credentials were saved successfully
2. Device not reflashed (erases NVS)
3. Serial output for debug messages
4. Return value is `true`

### Connection Fails

**Check:**
1. Network is in range
2. Password is correct
3. Network supports device
4. Try manual connection first

## Examples

### Full Files

- `examples/wifi_credentials_storage_example.cpp` - Interactive menu
- `test/test_wifi_credentials.cpp` - Unit tests

### Code Snippets

See `WIFI_CREDENTIALS_STORAGE.md` for more examples:
- Automatic credential management
- Credential migration
- Web interface integration
- Multiple network support

## Documentation

- **Quick Reference**: This file
- **Complete Guide**: `WIFI_CREDENTIALS_STORAGE.md`
- **Architecture**: `WIFI_CREDENTIALS_ARCHITECTURE.md`
- **Implementation**: `IMPLEMENTATION_SUMMARY_WIFI_CREDENTIALS.md`

## Support

**Issues?**
1. Check troubleshooting section
2. Review documentation
3. Run verification script
4. Check unit tests
5. Open GitHub issue

## Version

- **Version**: 1.0.0
- **Date**: 2025-10-16
- **Status**: Production Ready ✓
