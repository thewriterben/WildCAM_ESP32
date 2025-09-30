# Cloud Platform Integration - Quick Start Guide

## 5-Minute Setup

Get your WildCAM ESP32 connected to the cloud in just 5 minutes!

### Prerequisites Checklist

- [ ] ESP32-CAM module
- [ ] WiFi credentials
- [ ] Cloud account (AWS, Azure, or GCP)
- [ ] PlatformIO or Arduino IDE

### Step 1: Create Cloud Storage (2 minutes)

#### Option A: AWS S3

```bash
aws s3 mb s3://my-wildlife-bucket
```

#### Option B: Azure Blob

```bash
az storage account create --name mystore --resource-group wildlife-rg
az storage container create --name images --account-name mystore
```

#### Option C: Custom/Self-Hosted

Skip this step and use your own server endpoint.

### Step 2: Get Credentials (1 minute)

#### AWS

```bash
aws iam create-access-key --user-name wildlife-camera
```
Note the `AccessKeyId` and `SecretAccessKey`.

#### Azure

```bash
az storage account keys list --account-name mystore --query "[0].value" -o tsv
```
Note the account name and key.

### Step 3: Configure ESP32 (2 minutes)

Create `config.h` in your project:

```cpp
// WiFi Configuration
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"

// Cloud Configuration (choose one)

// For AWS:
#define CLOUD_PLATFORM CLOUD_AWS
#define CLOUD_REGION "us-west-2"
#define CLOUD_ENDPOINT "https://s3.us-west-2.amazonaws.com"
#define CLOUD_ACCESS_KEY "YOUR_AWS_ACCESS_KEY"
#define CLOUD_SECRET_KEY "YOUR_AWS_SECRET_KEY"
#define CLOUD_BUCKET "my-wildlife-bucket"

// For Azure:
// #define CLOUD_PLATFORM CLOUD_AZURE
// #define CLOUD_REGION "westus2"
// #define CLOUD_ENDPOINT "https://mystore.blob.core.windows.net"
// #define CLOUD_ACCESS_KEY "mystore"
// #define CLOUD_SECRET_KEY "YOUR_AZURE_KEY"
// #define CLOUD_BUCKET "images"
```

### Step 4: Add Code (30 seconds)

```cpp
#include <WiFi.h>
#include "config.h"
#include "production/enterprise/cloud_integrator.h"

void setup() {
    Serial.begin(115200);
    
    // Connect WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    
    // Configure cloud
    CloudConfig config;
    config.platform = CLOUD_PLATFORM;
    config.region = CLOUD_REGION;
    config.endpoint = CLOUD_ENDPOINT;
    config.accessKey = CLOUD_ACCESS_KEY;
    config.secretKey = CLOUD_SECRET_KEY;
    config.bucketName = CLOUD_BUCKET;
    config.useSSL = true;
    config.compressData = true;
    
    // Initialize cloud
    if (initializeCloudIntegration(config)) {
        Serial.println("Cloud connected! ✓");
    }
}

void loop() {
    // Your wildlife monitoring code here
    
    // Upload example
    if (imageReady) {
        uploadToCloud("/sdcard/image.jpg", DATA_IMAGE);
        Serial.println("Image uploaded!");
    }
    
    delay(1000);
}
```

### Step 5: Upload and Test (30 seconds)

1. Connect ESP32 via USB
2. Upload code using PlatformIO or Arduino IDE
3. Open Serial Monitor
4. Watch for "Cloud connected! ✓"
5. Your first image will upload automatically!

## Next Steps

### Enable Advanced Features

```cpp
// Add to setup():

// Intelligent synchronization
SyncConfig syncConfig;
syncConfig.enableAdaptiveSync = true;
syncConfig.enableCompression = true;
g_intelligentSyncManager->initialize(syncConfig);

// Cloud analytics
g_cloudAnalyticsEngine->initialize();
g_cloudAnalyticsEngine->enableRealtimeAnalytics(true);

// Wildlife detection pipeline
g_wildlifeCloudPipeline->initialize();
g_wildlifeCloudPipeline->setDetectionThreshold(0.8);
```

### Set Up Monitoring

```cpp
void loop() {
    // Report health every 5 minutes
    if (millis() % 300000 == 0) {
        String health = "{\"battery\":80,\"signal\":" + String(WiFi.RSSI()) + "}";
        g_cloudIntegrator->reportDeviceHealth(health);
    }
}
```

### Add Multi-Cloud Backup

```cpp
// Add to setup():
EnhancedCloudConfig multiCloud;

// Primary cloud
CloudConfig primary;
primary.platform = CLOUD_AWS;
// ... configure
multiCloud.providers.push_back(primary);

// Backup cloud
CloudConfig backup;
backup.platform = CLOUD_AZURE;
// ... configure
multiCloud.providers.push_back(backup);

initializeEnhancedCloudManager(multiCloud);
```

## Troubleshooting

### "Cloud not connected"

1. Check WiFi connection
2. Verify cloud credentials
3. Test endpoint: `ping your-endpoint.com`

### "Upload failed"

1. Check internet connection
2. Verify bucket/container exists
3. Check IAM permissions

### "Out of memory"

1. Enable compression: `config.compressData = true;`
2. Reduce image quality
3. Clear sync queue regularly

## Common Patterns

### Pattern 1: Upload on Detection

```cpp
void onWildlifeDetected(String species, float confidence) {
    // Save image
    camera_fb_t* fb = esp_camera_fb_get();
    String filename = "/detection_" + String(millis()) + ".jpg";
    saveImage(filename, fb);
    
    // Upload immediately
    UploadRequest req;
    req.dataType = DATA_DETECTION;
    req.localFilePath = filename;
    req.priority = 1; // High priority
    req.urgent = true;
    g_cloudIntegrator->uploadFileAsync(req);
    
    // Send real-time event
    sendCloudEvent("detection", "{\"species\":\"" + species + "\"}");
}
```

### Pattern 2: Batch Upload

```cpp
void batchUpload() {
    File dir = SD.open("/");
    while (File file = dir.openNextFile()) {
        if (String(file.name()).endsWith(".jpg")) {
            uploadToCloud(file.name(), DATA_IMAGE);
        }
    }
}
```

### Pattern 3: Scheduled Sync

```cpp
void loop() {
    static uint32_t lastSync = 0;
    
    if (millis() - lastSync > 600000) { // Every 10 minutes
        syncAllData();
        lastSync = millis();
    }
}
```

## Resources

- **Full Documentation**: [CLOUD_PLATFORM_LAUNCH.md](../CLOUD_PLATFORM_LAUNCH.md)
- **AWS Guide**: [AWS_DEPLOYMENT_GUIDE.md](AWS_DEPLOYMENT_GUIDE.md)
- **Azure Guide**: [AZURE_DEPLOYMENT_GUIDE.md](AZURE_DEPLOYMENT_GUIDE.md)
- **Monitoring**: [CLOUD_MONITORING_GUIDE.md](CLOUD_MONITORING_GUIDE.md)
- **Examples**: [examples/cloud_integration/](../ESP32WildlifeCAM-main/examples/)

## Support

Need help? Open an issue on [GitHub](https://github.com/thewriterben/WildCAM_ESP32/issues)

---

**You're now cloud-enabled! Happy wildlife monitoring! 🦌📸☁️**
