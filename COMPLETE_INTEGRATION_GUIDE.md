# Complete System Integration Guide
## WildCAM ESP32 - Cloud, Mobile, and Environmental Monitoring

This guide provides comprehensive instructions for deploying the complete WildCAM ESP32 wildlife monitoring system with all advanced features integrated.

## 📋 Table of Contents

1. [System Overview](#system-overview)
2. [Prerequisites](#prerequisites)
3. [Hardware Setup](#hardware-setup)
4. [Firmware Configuration](#firmware-configuration)
5. [Cloud Platform Setup](#cloud-platform-setup)
6. [Mobile App Deployment](#mobile-app-deployment)
7. [Environmental Sensors Configuration](#environmental-sensors-configuration)
8. [Testing and Validation](#testing-and-validation)
9. [Troubleshooting](#troubleshooting)

## 🎯 System Overview

The complete WildCAM ESP32 system integrates:

### Core Features
- ✅ **Dual Connectivity**: WiFi with automatic cellular fallback
- ✅ **Cloud Integration**: AWS, Azure, or GCP with intelligent sync
- ✅ **Environmental Monitoring**: Temperature, humidity, pressure, air quality, light levels
- ✅ **Wildlife Detection**: AI-powered species identification
- ✅ **Mobile App**: Real-time monitoring and push notifications
- ✅ **Security**: AES-256 encryption and secure authentication
- ✅ **OTA Updates**: Remote firmware updates over-the-air

### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    WildCAM ESP32 Device                     │
│  ┌────────────┐  ┌────────────┐  ┌────────────────────┐   │
│  │  Camera    │  │   PIR      │  │ Environmental      │   │
│  │  Module    │  │  Sensor    │  │ Sensor Suite       │   │
│  └────────────┘  └────────────┘  └────────────────────┘   │
│         │              │                    │               │
│  ┌──────┴──────────────┴────────────────────┴──────┐       │
│  │         ESP32 Main Controller                    │       │
│  │  ┌──────────────────────────────────────────┐   │       │
│  │  │    Connectivity Orchestrator             │   │       │
│  │  │  ┌──────────────┐  ┌──────────────────┐ │   │       │
│  │  │  │ WiFi Manager │  │ Cellular Manager │ │   │       │
│  │  │  └──────────────┘  └──────────────────┘ │   │       │
│  │  └──────────────────────────────────────────┘   │       │
│  └──────────────────────────────────────────────────┘       │
└─────────────────────────────────────────────────────────────┘
                            │
                     ┌──────┴──────┐
                     │             │
              ┌──────▼─────┐ ┌────▼─────────┐
              │   WiFi     │ │   Cellular   │
              │  Network   │ │   Network    │
              └──────┬─────┘ └────┬─────────┘
                     │             │
                     └──────┬──────┘
                            │
                   ┌────────▼─────────┐
                   │  Cloud Platform  │
                   │ (AWS/Azure/GCP)  │
                   └────────┬─────────┘
                            │
                 ┌──────────┴───────────┐
                 │                      │
         ┌───────▼──────┐      ┌───────▼────────┐
         │  Web         │      │  Mobile App    │
         │  Dashboard   │      │  (iOS/Android) │
         └──────────────┘      └────────────────┘
```

## 🔧 Prerequisites

### Hardware Requirements

#### Essential Components
| Component | Specification | Quantity |
|-----------|---------------|----------|
| ESP32-CAM | AI-Thinker or ESP32-S3 with PSRAM | 1 |
| Camera Module | OV2640 or OV5640 | 1 |
| PIR Sensor | HC-SR501 | 1 |
| SD Card | 32GB Class 10 or higher | 1 |
| Power Supply | 5V 2A or 18650 Li-ion battery | 1 |

#### Environmental Sensors
| Sensor | Purpose | Interface |
|--------|---------|-----------|
| BME280/BME680 | Temperature, humidity, pressure | I2C |
| SGP30/CCS811 | Air quality (CO2, VOC) | I2C |
| BH1750/TSL2561 | Light level | I2C |
| DHT22 (optional) | Backup temp/humidity | Digital |

#### Connectivity Modules
| Module | Purpose | Notes |
|--------|---------|-------|
| Built-in WiFi | Primary connectivity | ESP32 built-in |
| SIM7600/SIM7000 | Cellular backup | Optional, for remote deployments |

### Software Requirements

- **Arduino IDE** 1.8.19+ or **PlatformIO** 6.0+
- **Python** 3.8+ (for backend)
- **Node.js** 16+ (for web dashboard)
- **Flutter** 3.10+ (for mobile app)
- **Git** (for version control)

## 🔨 Hardware Setup

### 1. ESP32-CAM Wiring

#### Camera and Basic Components
The ESP32-CAM has the camera module built-in, so you only need to connect:
- PIR sensor to GPIO 13
- SD card to built-in SD slot
- Power to 5V and GND

#### Environmental Sensor Connections (I2C)

```
ESP32-CAM Pin  →  Sensor Connection
──────────────────────────────────
GPIO 14 (SDA)  →  All sensors SDA
GPIO 15 (SCL)  →  All sensors SCL
3.3V           →  All sensors VCC
GND            →  All sensors GND
```

#### Optional: Cellular Module (SIM7600)

```
ESP32-CAM Pin  →  SIM7600 Pin
─────────────────────────────
GPIO 16 (RX2)  →  TX
GPIO 17 (TX2)  →  RX
GPIO 18        →  POWER KEY
GPIO 19        →  RESET
5V             →  VCC
GND            →  GND
```

### 2. Power Configuration

#### Option A: USB Power (Development)
- Connect via USB FTDI programmer
- 5V @ 500mA minimum

#### Option B: Battery Power (Deployment)
- 18650 Li-ion 3.7V with boost converter to 5V
- Solar panel (5W) with MPPT charge controller
- Recommended capacity: 3000mAh+

### 3. Weatherproof Enclosure

For outdoor deployment:
- IP65 or higher rated enclosure
- Ventilation for environmental sensors
- Clear window for camera
- Cable glands for wiring
- Mounting hardware

## 💻 Firmware Configuration

### 1. Clone the Repository

```bash
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/firmware
```

### 2. Install Dependencies

#### Using PlatformIO (Recommended)

```bash
# Install PlatformIO
pip install platformio

# Install project dependencies
pio lib install

# Build firmware
pio run -e esp32cam_advanced
```

#### Using Arduino IDE

1. Install ESP32 board support
2. Install required libraries from Library Manager:
   - ESP32 Camera
   - ArduinoJson
   - Adafruit BME280
   - Adafruit SGP30
   - PubSubClient (for MQTT)

### 3. Configure Settings

Edit `firmware/examples/complete_system_integration.cpp`:

```cpp
// WiFi Configuration
const char* WIFI_SSID = "YourWiFiNetwork";
const char* WIFI_PASSWORD = "YourWiFiPassword";

// Cellular Configuration
const bool ENABLE_CELLULAR = true;  // Set false if not using cellular
const size_t CELLULAR_DATA_LIMIT_MB = 100;  // Daily limit

// Cloud Configuration
const char* CLOUD_ENDPOINT = "https://your-cloud-endpoint.com";
const char* CLOUD_ACCESS_KEY = "your-access-key";
const char* CLOUD_SECRET_KEY = "your-secret-key";
const char* CLOUD_BUCKET = "your-bucket-name";

// Device Configuration
const char* DEVICE_ID = "WILDCAM-001";  // Unique ID for your device
const float LATITUDE = 37.7749;         // Your location
const float LONGITUDE = -122.4194;
```

### 4. Upload Firmware

```bash
# Using PlatformIO
pio run -e esp32cam_advanced --target upload --upload-port /dev/ttyUSB0

# Or using Arduino IDE
# Select "AI Thinker ESP32-CAM" board and upload
```

## ☁️ Cloud Platform Setup

### Option A: AWS Setup

#### 1. Create S3 Bucket

```bash
aws s3 mb s3://wildlife-camera-data --region us-west-2
```

#### 2. Configure IAM User

Create an IAM user with permissions:
- S3: PutObject, GetObject, ListBucket
- IoT: Connect, Publish, Subscribe

```json
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": [
        "s3:PutObject",
        "s3:GetObject",
        "s3:ListBucket"
      ],
      "Resource": [
        "arn:aws:s3:::wildlife-camera-data",
        "arn:aws:s3:::wildlife-camera-data/*"
      ]
    }
  ]
}
```

#### 3. Setup IoT Core (Optional for MQTT)

```bash
# Create IoT thing
aws iot create-thing --thing-name WildCAM-001

# Create and attach certificate
aws iot create-keys-and-certificate \
  --set-as-active \
  --certificate-pem-outfile cert.pem \
  --public-key-outfile public.key \
  --private-key-outfile private.key
```

### Option B: Azure Setup

#### 1. Create Storage Account

```bash
az storage account create \
  --name wildcamstorage \
  --resource-group wildlife-monitoring \
  --location westus2 \
  --sku Standard_LRS
```

#### 2. Create Blob Container

```bash
az storage container create \
  --name wildlife-images \
  --account-name wildcamstorage
```

#### 3. Get Connection String

```bash
az storage account show-connection-string \
  --name wildcamstorage \
  --resource-group wildlife-monitoring
```

### Option C: Google Cloud Platform Setup

#### 1. Create Storage Bucket

```bash
gsutil mb -l us-west2 gs://wildlife-camera-data
```

#### 2. Setup Service Account

```bash
gcloud iam service-accounts create wildlife-camera \
  --display-name="Wildlife Camera Service Account"

gcloud projects add-iam-policy-binding YOUR_PROJECT_ID \
  --member="serviceAccount:wildlife-camera@YOUR_PROJECT_ID.iam.gserviceaccount.com" \
  --role="roles/storage.objectAdmin"
```

## 📱 Mobile App Deployment

### Flutter Mobile App Setup

#### 1. Navigate to Mobile App Directory

```bash
cd ESP32WildlifeCAM-main/mobile_app/flutter_app
```

#### 2. Install Dependencies

```bash
flutter pub get
```

#### 3. Configure Firebase (for Push Notifications)

1. Create a Firebase project at https://console.firebase.google.com
2. Add Android and iOS apps to your Firebase project
3. Download configuration files:
   - `google-services.json` for Android → `android/app/`
   - `GoogleService-Info.plist` for iOS → `ios/Runner/`

#### 4. Configure API Endpoint

Edit `lib/config/api_config.dart`:

```dart
class ApiConfig {
  static const String baseUrl = 'http://your-device-ip:80';
  static const String wsUrl = 'ws://your-device-ip:81';
  static const String deviceId = 'WILDCAM-001';
}
```

#### 5. Build and Deploy

##### For Android

```bash
# Debug build
flutter run

# Release build
flutter build apk --release
# APK will be at: build/app/outputs/flutter-apk/app-release.apk
```

##### For iOS

```bash
# Requires macOS and Xcode
flutter build ios --release

# Or use Xcode to build and deploy
open ios/Runner.xcworkspace
```

### Progressive Web App (PWA)

The PWA is automatically available when accessing the device's web interface:

```
http://<device-ip>/mobile/
```

Features:
- Works offline with service worker
- Can be installed as app on mobile devices
- No app store required
- Auto-updates

## 🌡️ Environmental Sensors Configuration

### Sensor Calibration

#### 1. Initial Calibration

Run calibration on first boot:

```cpp
envSuite->performCalibration();
```

This will:
- Calibrate BME280/680 for altitude
- Baseline SGP30/CCS811 air quality sensors (24h required for full baseline)
- Calibrate light sensors for ambient conditions

#### 2. Configure Thresholds

Adjust alert thresholds in `setupEnvironmentalSensors()`:

```cpp
EnvironmentalConfig envConfig;
envConfig.minTemperature = -10.0;  // Minimum safe temperature
envConfig.maxTemperature = 45.0;   // Maximum safe temperature
envConfig.minHumidity = 10.0;      // Minimum humidity
envConfig.maxHumidity = 95.0;      // Maximum humidity
envConfig.maxAQI = 150;            // Air quality alert threshold
```

### Sensor Placement Guidelines

#### Temperature/Humidity (BME280/BME680)
- Place in shaded, well-ventilated area
- Protect from direct sunlight and rain
- Allow air circulation around sensor
- Typical accuracy: ±0.5°C, ±3% RH

#### Air Quality (SGP30/CCS811)
- Place in ventilated area
- Away from direct emissions or contaminants
- Requires 24-hour burn-in for accurate readings
- Baseline stored in EEPROM for power cycles

#### Light Sensor (BH1750)
- Place where it can detect ambient light
- Can be used for day/night detection
- Useful for correlating wildlife activity with light levels

### Data Logging

Environmental data is logged:
- Locally to SD card (CSV format)
- Cloud storage (JSON format)
- Real-time to mobile app via WebSocket

Format example:
```json
{
  "timestamp": 1632847200,
  "temperature": 22.5,
  "humidity": 65.3,
  "pressure": 1013.25,
  "airQualityIndex": 42,
  "lightLevel": 450,
  "wildlifeComfortIndex": 7.8
}
```

## 🧪 Testing and Validation

### 1. Connectivity Testing

```cpp
// Test WiFi connectivity
if (connectivityOrch->connect()) {
    Serial.println("✓ WiFi connected");
}

// Test cellular fallback
connectivityOrch->forceConnectionType(CONN_CELLULAR);
if (connectivityOrch->isConnected()) {
    Serial.println("✓ Cellular connected");
}

// Test automatic fallback
connectivityOrch->enableAutoFallback(true);
connectivityOrch->update();  // Will switch based on signal quality
```

### 2. Cloud Upload Testing

```cpp
// Test image upload
uint8_t testData[] = "Test wildlife detection data";
UploadRequest request;
request.dataType = DATA_IMAGE;
request.data = (const char*)testData;
request.metadata = "{\"test\":true}";

if (g_cloudIntegrator->uploadDataAsync(request)) {
    Serial.println("✓ Cloud upload successful");
}
```

### 3. Environmental Sensor Testing

```cpp
// Read and display sensor data
envSuite->readAllSensors();
EnvironmentalData data = envSuite->getCurrentData();

Serial.printf("Temperature: %.1f°C\n", data.temperature);
Serial.printf("Humidity: %.1f%%\n", data.humidity);
Serial.printf("Pressure: %.1f hPa\n", data.pressure);
Serial.printf("Air Quality: %.1f\n", data.airQualityIndex);
```

### 4. Mobile App Testing

1. Ensure device and mobile phone are on same network
2. Open mobile app
3. Enter device IP address in settings
4. Check real-time updates on dashboard
5. Trigger a test detection to verify push notifications

### 5. End-to-End Integration Test

Complete workflow test:

1. **Trigger Motion Detection** → PIR sensor activates
2. **Capture Image** → Camera takes photo
3. **AI Detection** → Wildlife classifier runs
4. **Environmental Correlation** → Current conditions logged
5. **Cloud Upload** → Data synced to cloud (WiFi or cellular)
6. **Mobile Notification** → Push notification sent
7. **Data Visualization** → Results appear in mobile app and web dashboard

## 🔍 Troubleshooting

### Connectivity Issues

#### WiFi Not Connecting

```
Symptom: "WiFi connection failed" messages
Solutions:
1. Verify SSID and password are correct
2. Check WiFi signal strength (should be > -75 dBm)
3. Ensure ESP32 is within range of access point
4. Try different WiFi channel (avoid congested channels)
5. Check if MAC address filtering is enabled on router
```

#### Cellular Fallback Not Working

```
Symptom: Device doesn't switch to cellular when WiFi fails
Solutions:
1. Verify cellular module is properly connected
2. Check SIM card is inserted and activated
3. Verify APN settings for your carrier
4. Check cellular signal strength
5. Ensure cellular data limit not exceeded
```

### Cloud Upload Issues

#### Upload Failures

```
Symptom: "Cloud upload failed" errors
Solutions:
1. Verify internet connectivity
2. Check cloud credentials (access key, secret key)
3. Verify bucket/container exists
4. Check IAM permissions
5. Monitor connection quality
6. Check if data limit reached
```

#### Slow Uploads

```
Symptom: Uploads taking very long
Solutions:
1. Enable compression in cloud config
2. Use cellular only for essential data
3. Reduce image quality/size
4. Batch uploads during good connection periods
5. Use intelligent sync manager's adaptive algorithms
```

### Environmental Sensor Issues

#### Incorrect Readings

```
Symptom: Sensor readings seem wrong
Solutions:
1. Run calibration: envSuite->performCalibration()
2. Check I2C connections (SDA, SCL)
3. Verify sensor power supply (3.3V)
4. Check I2C pull-up resistors (4.7kΩ typically)
5. Ensure sensors are not in direct sunlight
6. Allow BME680/SGP30 burn-in time (24 hours)
```

#### Sensors Not Detected

```
Symptom: "Sensor initialization failed" messages
Solutions:
1. Check I2C address (use I2C scanner)
2. Verify wiring connections
3. Check power supply voltage
4. Try different I2C pins if conflicts exist
5. Reduce I2C clock speed if needed
```

### Mobile App Issues

#### Can't Connect to Device

```
Symptom: Mobile app shows "Connection failed"
Solutions:
1. Verify device and phone on same network
2. Check device IP address is correct
3. Ensure firewall not blocking connections
4. Test device API endpoint in browser first
5. Check if device web server is running
```

#### No Push Notifications

```
Symptom: Not receiving wildlife detection alerts
Solutions:
1. Enable notifications in app settings
2. Check phone notification permissions
3. Verify Firebase configuration
4. Check WebSocket connection status
5. Test notification system manually
```

## 📊 Performance Optimization

### Battery Life Optimization

```cpp
// Enable power optimization
envSuite->enablePowerOptimization(true);

// Adjust reading frequency based on battery
envSuite->adjustReadingFrequency(batteryLevel);

// Use cellular only when necessary
if (batteryLevel < 20) {
    connectivityOrch->preferWiFi();
}
```

### Bandwidth Optimization

```cpp
// Enable intelligent compression
SyncConfig config;
config.enableCompression = true;
config.enableDeltaSync = true;  // Only sync changes
g_intelligentSyncManager->initialize(config);

// Prefer WiFi for large uploads
connectivityOrch->sendDataOptimized(data, size, true);
```

### Memory Management

```cpp
// Monitor heap usage
if (ESP.getFreeHeap() < 50000) {
    Serial.println("WARNING: Low memory");
    // Reduce image quality or batch size
}

// Clean up periodically
if (millis() % 3600000 == 0) {  // Every hour
    g_intelligentSyncManager->processOfflineQueue();
    gc();  // Garbage collection if available
}
```

## 🔐 Security Best Practices

1. **Change Default Credentials**
   - Update WiFi password
   - Change cloud API keys
   - Set strong device passwords

2. **Enable Encryption**
   - Use SSL/TLS for all cloud connections
   - Enable AES-256 encryption for local storage
   - Encrypt sensitive configuration data

3. **Regular Updates**
   - Enable OTA updates for security patches
   - Monitor for firmware updates
   - Update mobile app regularly

4. **Network Security**
   - Use WPA3 WiFi encryption if available
   - Implement VPN for cellular connections
   - Use private APNs when available

## 📈 Monitoring and Maintenance

### Health Monitoring

The system automatically reports health metrics every 30 minutes:

```
- Connectivity status and quality
- Battery level and charging state
- Memory usage and system uptime
- Detection count and cloud sync status
- Environmental sensor status
- Error and warning counts
```

### Log Files

Logs are stored on SD card:
```
/logs/system_YYYYMMDD.log       - System events
/logs/detections_YYYYMMDD.log   - Wildlife detections
/logs/environmental_YYYYMMDD.log - Sensor readings
/logs/errors_YYYYMMDD.log       - Error messages
```

### Maintenance Schedule

| Task | Frequency | Notes |
|------|-----------|-------|
| Check battery level | Weekly | Replace/charge if <20% |
| Clean camera lens | Monthly | Use microfiber cloth |
| Check sensor calibration | Monthly | Recalibrate if needed |
| Update firmware | As available | Use OTA updates |
| Review logs | Weekly | Check for errors |
| Clean solar panel | Monthly | If using solar power |

## 🎓 Additional Resources

- [Full API Documentation](./docs/API_REFERENCE.md)
- [Cloud Platform Guide](./CLOUD_PLATFORM_LAUNCH.md)
- [Mobile App Development Guide](./ESP32WildlifeCAM-main/mobile_app/README.md)
- [Environmental Sensors Guide](./docs/ENVIRONMENTAL_SENSORS.md)
- [Troubleshooting FAQ](./docs/TROUBLESHOOTING_FAQ.md)

## 🤝 Support

For additional help:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
- Discussions: https://github.com/thewriterben/WildCAM_ESP32/discussions
- Email: wildlife-tech@example.com

---

**Last Updated**: 2025-01-02
**Version**: 3.0.0
**Status**: Production Ready ✅
