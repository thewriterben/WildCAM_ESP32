# WiFi Credentials Storage Implementation

This document describes the WiFi credential storage functionality implemented using the ESP32 Preferences library (NVS - Non-Volatile Storage).

## Overview

The WiFi credential storage system provides persistent storage of WiFi SSID and password using the ESP32's built-in Non-Volatile Storage (NVS). This allows the device to:

- Remember WiFi credentials across power cycles
- Automatically reconnect to saved networks on boot
- Support factory reset functionality
- Protect passwords with basic encryption

## Features

### 1. Credential Storage
- Stores WiFi SSID and password in NVS namespace `wifi_config`
- SSID stored under key `ssid`
- Password stored under key `password`
- Password is encrypted using XOR obfuscation before storage
- Survives device reboots and power loss

### 2. Credential Loading
- Automatically loads credentials from NVS on WiFiManager initialization
- Falls back to config.h values if NVS credentials are empty
- Returns empty strings for missing credentials
- Gracefully handles corrupted data

### 3. Factory Reset
- `clearWiFiCredentials()` removes all stored credentials
- Clears entire `wifi_config` namespace
- Useful for device reset or credential changes

### 4. Password Encryption
- Simple XOR-based encryption for password obfuscation
- Protects against casual inspection of NVS data
- Symmetric encryption (same operation for encrypt/decrypt)
- Custom encryption key can be modified in implementation

**Note:** XOR encryption provides obfuscation, not cryptographic security. For production systems requiring high security, consider:
- ESP32 hardware encryption features
- Flash encryption (requires secure boot)
- Key management systems

## API Reference

### WiFiManager Class Methods

#### `bool saveWiFiCredentials(const char* ssid, const char* password)`

Saves WiFi credentials to non-volatile storage.

**Parameters:**
- `ssid` - WiFi network SSID (required, non-empty)
- `password` - WiFi network password (can be empty for open networks)

**Returns:**
- `true` if credentials saved successfully
- `false` if SSID is empty or write failed

**Example:**
```cpp
WiFiManager wifiManager;
wifiManager.init();

if (wifiManager.saveWiFiCredentials("MyNetwork", "MyPassword123")) {
    Serial.println("Credentials saved!");
}
```

---

#### `bool loadWiFiCredentials(String& ssid, String& password)`

Loads WiFi credentials from non-volatile storage.

**Parameters:**
- `ssid` - Reference to String to receive loaded SSID
- `password` - Reference to String to receive loaded password

**Returns:**
- `true` if credentials loaded successfully
- `false` if no credentials found or read failed

**Example:**
```cpp
WiFiManager wifiManager;
wifiManager.init();

String ssid, password;
if (wifiManager.loadWiFiCredentials(ssid, password)) {
    Serial.printf("Loaded: %s\n", ssid.c_str());
    // Connect using loaded credentials
    wifiManager.connect(ssid, password);
}
```

---

#### `void clearWiFiCredentials()`

Clears all WiFi credentials from non-volatile storage (factory reset).

**Parameters:** None

**Returns:** None

**Example:**
```cpp
WiFiManager wifiManager;
wifiManager.init();

// Factory reset
wifiManager.clearWiFiCredentials();
Serial.println("All credentials cleared!");
```

## Usage Examples

### Example 1: Basic Save and Load

```cpp
#include "wifi_manager.h"

WiFiManager wifiManager;

void setup() {
    Serial.begin(115200);
    wifiManager.init();
    
    // Save credentials
    wifiManager.saveWiFiCredentials("HomeNetwork", "password123");
    
    // Later... load and connect
    String ssid, password;
    if (wifiManager.loadWiFiCredentials(ssid, password)) {
        wifiManager.connect(ssid, password);
    }
}
```

### Example 2: Automatic Connection on Boot

```cpp
#include "wifi_manager.h"

WiFiManager wifiManager;

void setup() {
    Serial.begin(115200);
    wifiManager.init();
    
    // WiFiManager automatically loads saved credentials during init
    // Just call connect() without parameters to use saved credentials
    if (wifiManager.connect()) {
        Serial.println("Connected using saved credentials!");
    } else {
        // No saved credentials or connection failed
        // Enter AP mode for configuration
        wifiManager.startAccessPoint("WildCAM-Setup");
    }
}
```

### Example 3: Credential Update Flow

```cpp
#include "wifi_manager.h"

WiFiManager wifiManager;

void updateCredentials(const char* newSSID, const char* newPassword) {
    // Disconnect from current network
    wifiManager.disconnect();
    
    // Save new credentials
    if (wifiManager.saveWiFiCredentials(newSSID, newPassword)) {
        Serial.println("Credentials updated!");
        
        // Connect to new network
        if (wifiManager.connect(newSSID, newPassword)) {
            Serial.println("Connected to new network!");
        }
    }
}
```

### Example 4: Factory Reset

```cpp
#include "wifi_manager.h"

WiFiManager wifiManager;

void factoryReset() {
    Serial.println("Performing factory reset...");
    
    // Disconnect from WiFi
    wifiManager.disconnect();
    
    // Clear all saved credentials
    wifiManager.clearWiFiCredentials();
    
    // Optionally, reset other settings
    wifiManager.resetSettings();
    
    Serial.println("Factory reset complete!");
    
    // Restart device
    ESP.restart();
}
```

### Example 5: Credential Migration from Config

```cpp
#include "wifi_manager.h"
#include "config.h"

WiFiManager wifiManager;

void migrateToNVS() {
    wifiManager.init();
    
    // Check if NVS has credentials
    String savedSSID, savedPassword;
    if (!wifiManager.loadWiFiCredentials(savedSSID, savedPassword)) {
        // No NVS credentials, migrate from config.h
        if (strlen(WIFI_SSID) > 0) {
            Serial.println("Migrating credentials to NVS...");
            wifiManager.saveWiFiCredentials(WIFI_SSID, WIFI_PASSWORD);
            Serial.println("Migration complete!");
        }
    }
}
```

## Integration with Existing Code

The credential storage functionality is integrated into the existing `WiFiManager` class:

1. **Automatic Loading**: WiFiManager automatically loads saved credentials during initialization if config.h values are empty
2. **Backward Compatible**: Existing code continues to work without changes
3. **Optional Usage**: Credential storage is optional; hardcoded config.h values still work

### Initialization Flow

```
WiFiManager::init()
  └─> applyConfigurationSettings()
      ├─> Read WIFI_SSID and WIFI_PASSWORD from config.h
      └─> If config values are empty:
          └─> loadWiFiCredentials() from NVS
```

## Testing

### Unit Tests

The implementation includes comprehensive unit tests in `test/test_wifi_credentials.cpp`:

- `test_save_wifi_credentials` - Verify basic save operation
- `test_save_empty_ssid` - Validate error handling for empty SSID
- `test_save_empty_password` - Test open network support
- `test_load_wifi_credentials` - Verify basic load operation
- `test_load_no_credentials` - Test behavior with no stored credentials
- `test_clear_wifi_credentials` - Verify factory reset
- `test_special_characters_in_password` - Test special character handling
- `test_long_credentials` - Verify maximum length support
- `test_overwrite_credentials` - Test credential updates
- `test_password_encryption` - Verify encryption/decryption
- `test_multiple_save_load_cycles` - Test reliability over multiple operations

### Running Tests

```bash
# Using PlatformIO
pio test -e native

# Or run specific test
pio test -e native -f test_wifi_credentials
```

### Interactive Example

Run the interactive example to test functionality:

```bash
pio run -e esp32cam -t upload
pio device monitor
```

Then use the menu to:
1. Save credentials
2. Load credentials
3. Clear credentials
4. Connect to WiFi

## Security Considerations

### Current Implementation

The current implementation uses XOR-based password obfuscation:

**Pros:**
- Simple and lightweight
- No external dependencies
- Sufficient for casual protection
- Low memory and CPU overhead

**Cons:**
- Not cryptographically secure
- Vulnerable to determined attackers with physical access
- Key is embedded in firmware

### Enhanced Security Options

For production deployments requiring stronger security:

#### Option 1: ESP32 Flash Encryption

Enable flash encryption in ESP32:

```cpp
// In platformio.ini
build_flags = 
    -DCONFIG_SECURE_FLASH_ENC_ENABLED
```

This encrypts the entire NVS partition at the hardware level.

#### Option 2: AES Encryption

Replace XOR with AES encryption:

```cpp
#include "mbedtls/aes.h"

String encryptPasswordAES(const String& password, const uint8_t* key) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 256);
    // ... encryption logic
    mbedtls_aes_free(&aes);
    return encrypted;
}
```

#### Option 3: Secure Element

Use ESP32's secure element (if available):

- Store encryption keys in secure element
- Hardware-protected key storage
- Resistance to physical attacks

## Storage Details

### NVS Namespace

- **Namespace**: `wifi_config`
- **Keys**:
  - `ssid` - WiFi SSID (String)
  - `password` - Encrypted password (String)

### Storage Limits

- Maximum SSID length: 32 characters (WiFi standard)
- Maximum password length: 63 characters (WPA2 standard)
- NVS key length: 15 characters maximum
- NVS namespace: 15 characters maximum
- Total NVS partition size: Typically 20KB (configurable)

### NVS Partition Configuration

The NVS partition is defined in `partitions.csv`:

```csv
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x5000
```

## Error Handling

The implementation includes comprehensive error handling:

1. **Empty SSID**: Returns false, logs error
2. **NVS Open Failure**: Returns false, logs error
3. **Write Failure**: Returns false, logs error
4. **Read Failure**: Returns false, returns empty strings
5. **Missing Data**: Returns false, returns empty strings
6. **Corrupted Data**: Gracefully handled, returns empty strings

## Troubleshooting

### Credentials Not Saving

**Problem**: `saveWiFiCredentials()` returns false

**Solutions:**
- Check that SSID is not empty
- Verify NVS partition exists in partitions.csv
- Ensure sufficient NVS space available
- Check serial output for error messages

### Credentials Not Loading

**Problem**: `loadWiFiCredentials()` returns false

**Solutions:**
- Verify credentials were saved successfully
- Check that NVS partition hasn't been erased
- Look for corruption messages in serial output
- Try clearing and re-saving credentials

### Connection Fails After Loading

**Problem**: WiFi connection fails with loaded credentials

**Solutions:**
- Verify credentials are correct (check SSID spelling)
- Ensure network is in range
- Check password was saved correctly
- Try manual connection to verify credentials

### Factory Reset Not Working

**Problem**: Credentials persist after `clearWiFiCredentials()`

**Solutions:**
- Verify clear operation completed successfully
- Check for multiple WiFiManager instances
- Ensure correct namespace is being cleared
- Try erasing entire NVS partition (development only)

## Future Enhancements

Potential improvements for future versions:

1. **Multiple Network Support**
   - Store multiple SSIDs with priority
   - Automatic best network selection
   - Fallback network configuration

2. **Enhanced Encryption**
   - AES-256 encryption
   - Hardware-backed key storage
   - Per-device unique keys

3. **Credential Management API**
   - REST API for remote configuration
   - Web interface for credential management
   - Bluetooth provisioning support

4. **Advanced Features**
   - Enterprise WiFi support (WPA2-Enterprise)
   - Certificate storage
   - Network profile management
   - Automatic credential backup

## References

- [ESP32 Preferences Library](https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences)
- [ESP32 NVS Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html)
- [ESP32 Flash Encryption](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/security/flash-encryption.html)
- [WiFi Security Standards](https://www.wi-fi.org/discover-wi-fi/security)

## License

This implementation is part of the WildCAM ESP32 project and follows the same license terms.

## Support

For issues, questions, or contributions:
- Open an issue on GitHub
- Check existing documentation
- Review test cases for usage examples
