#!/bin/bash

# WildCAM_ESP32 Setup Script
# Quick setup for ESP32 Wildlife Camera development

echo "🦌 WildCAM_ESP32 Setup Script"
echo "================================="

# Check if PlatformIO is installed
echo "📋 Checking PlatformIO installation..."
if ! command -v pio &> /dev/null; then
    echo "❌ PlatformIO not found. Installing..."
    pip install platformio
    if [ $? -eq 0 ]; then
        echo "✅ PlatformIO installed successfully"
    else
        echo "❌ Failed to install PlatformIO. Please install manually:"
        echo "   pip install platformio"
        exit 1
    fi
else
    echo "✅ PlatformIO found"
fi

# Initialize PlatformIO project
echo "🔧 Initializing PlatformIO project..."
pio project init --ide=vscode --silent

# Update libraries
echo "📦 Installing dependencies..."
pio lib install --silent

# Check project configuration
echo "🔍 Validating project configuration..."
pio project config

echo ""
echo "✅ Setup complete!"
echo ""
echo "🚀 Next steps:"
echo "1. Update wifi_config.h with your WiFi credentials"
echo "2. Adjust settings in config.h for your hardware"
echo "3. Build the project: pio run -e esp32cam"
echo "4. Upload to device: pio run -e esp32cam --target upload"
echo ""
echo "📚 Supported boards:"
echo "   - esp32cam      (AI-Thinker ESP32-CAM - default)"
echo "   - esp32s3cam    (ESP32-S3-CAM)" 
echo "   - ttgo-t-camera (TTGO T-Camera)"
echo ""
echo "🔧 Build for specific board:"
echo "   pio run -e [board_name]"
echo ""
echo "📊 Monitor serial output:"
echo "   pio device monitor"
echo ""
echo "Happy wildlife monitoring! 🦌📸"