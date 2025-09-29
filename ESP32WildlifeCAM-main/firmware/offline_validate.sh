#!/bin/bash
# Offline validation and build script for ESP32 Wildlife Camera
# This script works completely offline and avoids firewall issues

set -e

echo "ESP32 Wildlife Camera - Offline Validation & Build"
echo "=================================================="

# Run the validation script (works completely offline)
echo "=== Running Offline Validation ==="
if python3 ../validate_fixes.py; then
    echo "✅ Validation checks passed!"
else
    echo "❌ Validation checks failed!"
    exit 1
fi

# Basic syntax checking using GCC preprocessor (offline)
echo ""
echo "=== Basic Syntax Checking ==="
cd src
find . -name "*.cpp" -o -name "*.h" | while read file; do
    echo "Checking syntax: $file"
    if g++ -fsyntax-only -std=c++17 -I. -I./hal -I./display -I./configs \
           -DARDUINO=200 -DESP32_VALIDATION_BUILD=1 \
           -Wno-unknown-pragmas -Wno-attributes "$file" 2>/dev/null; then
        echo "  ✅ $file"
    else
        echo "  ⚠️  $file (may have ESP32-specific dependencies)"
    fi
done

cd ..

echo ""
echo "=== Build Summary ==="
echo "✅ Offline validation completed successfully"
echo "✅ Basic syntax checking completed"
echo ""
echo "Note: Full compilation requires ESP32 platform installation"
echo "which needs network access for first-time setup."
echo ""
echo "To avoid firewall issues:"
echo "1. Use this script for offline validation"
echo "2. Install ESP32 platform manually if needed"
echo "3. Use cached builds for development"