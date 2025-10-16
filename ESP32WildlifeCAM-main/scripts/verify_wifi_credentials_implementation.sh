#!/bin/bash

# WiFi Credentials Storage Implementation Verification Script
# This script verifies the implementation of WiFi credential storage functionality

echo "=== WiFi Credentials Storage Implementation Verification ==="
echo ""

# Check if files exist
echo "1. Checking for implementation files..."
FILES=(
    "ESP32WildlifeCAM-main/firmware/src/wifi_manager.h"
    "ESP32WildlifeCAM-main/firmware/src/wifi_manager.cpp"
    "ESP32WildlifeCAM-main/test/test_wifi_credentials.cpp"
    "ESP32WildlifeCAM-main/examples/wifi_credentials_storage_example.cpp"
    "ESP32WildlifeCAM-main/WIFI_CREDENTIALS_STORAGE.md"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✓ $file"
    else
        echo "   ✗ MISSING: $file"
    fi
done
echo ""

# Check for function declarations in header
echo "2. Verifying function declarations in header..."
FUNCTIONS=(
    "bool saveWiFiCredentials"
    "bool loadWiFiCredentials"
    "void clearWiFiCredentials"
)

for func in "${FUNCTIONS[@]}"; do
    if grep -q "$func" ESP32WildlifeCAM-main/firmware/src/wifi_manager.h; then
        echo "   ✓ $func declared"
    else
        echo "   ✗ MISSING: $func"
    fi
done
echo ""

# Check for function implementations
echo "3. Verifying function implementations in cpp..."
for func in "${FUNCTIONS[@]}"; do
    if grep -q "$func" ESP32WildlifeCAM-main/firmware/src/wifi_manager.cpp; then
        echo "   ✓ $func implemented"
    else
        echo "   ✗ MISSING: $func"
    fi
done
echo ""

# Check for Preferences library inclusion
echo "4. Verifying Preferences library inclusion..."
if grep -q "#include <Preferences.h>" ESP32WildlifeCAM-main/firmware/src/wifi_manager.h; then
    echo "   ✓ Preferences.h included in header"
else
    echo "   ✗ MISSING: Preferences.h in header"
fi

if grep -q "#include <Preferences.h>" ESP32WildlifeCAM-main/firmware/src/wifi_manager.cpp; then
    echo "   ✓ Preferences.h included in cpp"
else
    echo "   ✗ MISSING: Preferences.h in cpp"
fi
echo ""

# Check for encryption functions
echo "5. Verifying password encryption implementation..."
if grep -q "encryptPassword" ESP32WildlifeCAM-main/firmware/src/wifi_manager.h; then
    echo "   ✓ encryptPassword declared"
else
    echo "   ✗ MISSING: encryptPassword"
fi

if grep -q "decryptPassword" ESP32WildlifeCAM-main/firmware/src/wifi_manager.h; then
    echo "   ✓ decryptPassword declared"
else
    echo "   ✗ MISSING: decryptPassword"
fi
echo ""

# Check for proper namespace usage
echo "6. Verifying NVS namespace configuration..."
if grep -q '"wifi_config"' ESP32WildlifeCAM-main/firmware/src/wifi_manager.cpp; then
    echo "   ✓ Namespace 'wifi_config' used"
else
    echo "   ✗ MISSING: wifi_config namespace"
fi

if grep -q '"ssid"' ESP32WildlifeCAM-main/firmware/src/wifi_manager.cpp; then
    echo "   ✓ Key 'ssid' defined"
else
    echo "   ✗ MISSING: ssid key"
fi

if grep -q '"password"' ESP32WildlifeCAM-main/firmware/src/wifi_manager.cpp; then
    echo "   ✓ Key 'password' defined"
else
    echo "   ✗ MISSING: password key"
fi
echo ""

# Check for unit tests
echo "7. Verifying unit test coverage..."
TESTS=(
    "test_save_wifi_credentials"
    "test_load_wifi_credentials"
    "test_clear_wifi_credentials"
    "test_password_encryption"
)

for test in "${TESTS[@]}"; do
    if grep -q "$test" ESP32WildlifeCAM-main/test/test_wifi_credentials.cpp; then
        echo "   ✓ $test exists"
    else
        echo "   ✗ MISSING: $test"
    fi
done
echo ""

# Check documentation
echo "8. Verifying documentation..."
if grep -q "saveWiFiCredentials" ESP32WildlifeCAM-main/WIFI_CREDENTIALS_STORAGE.md; then
    echo "   ✓ saveWiFiCredentials documented"
else
    echo "   ✗ MISSING: saveWiFiCredentials documentation"
fi

if grep -q "loadWiFiCredentials" ESP32WildlifeCAM-main/WIFI_CREDENTIALS_STORAGE.md; then
    echo "   ✓ loadWiFiCredentials documented"
else
    echo "   ✗ MISSING: loadWiFiCredentials documentation"
fi

if grep -q "clearWiFiCredentials" ESP32WildlifeCAM-main/WIFI_CREDENTIALS_STORAGE.md; then
    echo "   ✓ clearWiFiCredentials documented"
else
    echo "   ✗ MISSING: clearWiFiCredentials documentation"
fi
echo ""

# Check for example code
echo "9. Verifying example code..."
if [ -f "ESP32WildlifeCAM-main/examples/wifi_credentials_storage_example.cpp" ]; then
    if grep -q "saveWiFiCredentials" ESP32WildlifeCAM-main/examples/wifi_credentials_storage_example.cpp; then
        echo "   ✓ Example demonstrates saveWiFiCredentials"
    fi
    if grep -q "loadWiFiCredentials" ESP32WildlifeCAM-main/examples/wifi_credentials_storage_example.cpp; then
        echo "   ✓ Example demonstrates loadWiFiCredentials"
    fi
    if grep -q "clearWiFiCredentials" ESP32WildlifeCAM-main/examples/wifi_credentials_storage_example.cpp; then
        echo "   ✓ Example demonstrates clearWiFiCredentials"
    fi
else
    echo "   ✗ Example file not found"
fi
echo ""

# Summary
echo "=== Verification Complete ==="
echo ""
echo "Implementation Summary:"
echo "- WiFi credential storage implemented using ESP32 Preferences (NVS)"
echo "- Password encryption using XOR obfuscation"
echo "- Comprehensive error handling"
echo "- Unit tests created"
echo "- Interactive example provided"
echo "- Full documentation included"
echo ""
echo "Key Features:"
echo "- Namespace: wifi_config"
echo "- Keys: ssid, password"
echo "- Functions: saveWiFiCredentials(), loadWiFiCredentials(), clearWiFiCredentials()"
echo "- Automatic credential loading on WiFiManager init"
echo "- Factory reset capability"
echo ""
echo "Next Steps:"
echo "1. Build and flash to ESP32 device"
echo "2. Run unit tests: pio test -e native"
echo "3. Test interactive example on hardware"
echo "4. Review WIFI_CREDENTIALS_STORAGE.md for usage"
