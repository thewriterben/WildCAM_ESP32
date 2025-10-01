# Azure Deployment Guide for WildCAM ESP32

## Overview

This guide provides step-by-step instructions for deploying WildCAM ESP32 with Microsoft Azure cloud platform. Azure offers robust IoT and AI services ideal for wildlife monitoring at scale.

## Prerequisites

- Azure subscription with billing enabled
- Azure CLI installed
- Basic knowledge of Azure services
- ESP32-CAM hardware

## Architecture

```
┌─────────────┐
│  ESP32-CAM  │
│   Device    │
└──────┬──────┘
       │ HTTPS/MQTT
       │
┌──────▼──────────────────────────────────┐
│          Azure Cloud Platform           │
├─────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐            │
│  │ IoT Hub  │  │ Event Hub│            │
│  └────┬─────┘  └────┬─────┘            │
│       │             │                   │
│  ┌────▼─────────────▼─────┐            │
│  │   Azure Functions       │            │
│  └────┬────────────────────┘            │
│       │                                 │
│  ┌────▼──────┐ ┌────────┐ ┌──────────┐│
│  │   Blob    │ │ Cosmos │ │ ML Studio││
│  │  Storage  │ │   DB   │ │ /Custom  ││
│  └───────────┘ └────────┘ └──────────┘│
└─────────────────────────────────────────┘
```

## Step-by-Step Setup

### 1. Create Resource Group

```bash
# Login to Azure
az login

# Create resource group
az group create \
    --name wildlife-camera-rg \
    --location westus2
```

### 2. Create Storage Account

```bash
# Create storage account
az storage account create \
    --name wildlifestorage$RANDOM \
    --resource-group wildlife-camera-rg \
    --location westus2 \
    --sku Standard_LRS \
    --encryption-services blob \
    --https-only true

# Get connection string
az storage account show-connection-string \
    --name wildlifestorage \
    --resource-group wildlife-camera-rg \
    --output tsv

# Create containers
az storage container create \
    --name images \
    --account-name wildlifestorage

az storage container create \
    --name videos \
    --account-name wildlifestorage

az storage container create \
    --name detections \
    --account-name wildlifestorage

az storage container create \
    --name telemetry \
    --account-name wildlifestorage
```

### 3. Create IoT Hub

```bash
# Create IoT Hub
az iot hub create \
    --name wildlife-iot-hub \
    --resource-group wildlife-camera-rg \
    --sku S1 \
    --location westus2

# Register device
az iot hub device-identity create \
    --hub-name wildlife-iot-hub \
    --device-id esp32-wildlife-cam-001

# Get device connection string
az iot hub device-identity connection-string show \
    --hub-name wildlife-iot-hub \
    --device-id esp32-wildlife-cam-001 \
    --output tsv
```

### 4. Configure ESP32 Device

```cpp
#include "production/enterprise/cloud_integrator.h"

// Azure Configuration
CloudConfig azureConfig;
azureConfig.platform = CLOUD_AZURE;
azureConfig.region = "westus2";
azureConfig.endpoint = "https://wildlifestorage.blob.core.windows.net";
azureConfig.accessKey = "STORAGE_ACCOUNT_NAME";
azureConfig.secretKey = "STORAGE_ACCOUNT_KEY";
azureConfig.bucketName = "images"; // Container name
azureConfig.useSSL = true;
azureConfig.compressData = true;
azureConfig.encryptData = true;
azureConfig.syncMode = SYNC_OFFLINE_FIRST;
azureConfig.syncInterval = 300;

// Initialize
if (!initializeCloudIntegration(azureConfig)) {
    Serial.println("Failed to initialize Azure integration");
    return;
}

Serial.println("Azure cloud integration initialized");
```

### 5. Set Up Azure Functions

Create `ImageProcessor/__init__.py`:

```python
import logging
import azure.functions as func
from azure.storage.blob import BlobServiceClient
from azure.cognitiveservices.vision.customvision.prediction import CustomVisionPredictionClient
from msrest.authentication import ApiKeyCredentials
import json

def main(myblob: func.InputStream):
    logging.info(f"Processing blob: {myblob.name}")
    
    # Initialize services
    blob_service = BlobServiceClient.from_connection_string(
        os.environ['STORAGE_CONNECTION_STRING']
    )
    
    # Analyze image with Custom Vision
    credentials = ApiKeyCredentials(
        in_headers={"Prediction-key": os.environ['PREDICTION_KEY']}
    )
    predictor = CustomVisionPredictionClient(
        os.environ['PREDICTION_ENDPOINT'], 
        credentials
    )
    
    # Get image data
    image_data = myblob.read()
    
    # Detect wildlife
    results = predictor.detect_image(
        os.environ['PROJECT_ID'],
        os.environ['PUBLISHED_NAME'],
        image_data
    )
    
    # Process results
    detections = []
    for prediction in results.predictions:
        if prediction.probability > 0.7:
            detections.append({
                'species': prediction.tag_name,
                'confidence': prediction.probability,
                'bounding_box': {
                    'left': prediction.bounding_box.left,
                    'top': prediction.bounding_box.top,
                    'width': prediction.bounding_box.width,
                    'height': prediction.bounding_box.height
                }
            })
    
    # Store results
    results_blob = blob_service.get_blob_client(
        container='detections',
        blob=f"{myblob.name}.json"
    )
    results_blob.upload_blob(
        json.dumps(detections),
        overwrite=True
    )
    
    logging.info(f"Detected {len(detections)} animals")
```

Deploy function:

```bash
# Create function app
az functionapp create \
    --resource-group wildlife-camera-rg \
    --consumption-plan-location westus2 \
    --runtime python \
    --runtime-version 3.9 \
    --functions-version 4 \
    --name wildlife-image-processor \
    --storage-account wildlifestorage

# Deploy function
func azure functionapp publish wildlife-image-processor
```

### 6. Set Up Cosmos DB

```bash
# Create Cosmos DB account
az cosmosdb create \
    --name wildlife-cosmosdb \
    --resource-group wildlife-camera-rg \
    --kind GlobalDocumentDB \
    --locations regionName=westus2

# Create database
az cosmosdb sql database create \
    --account-name wildlife-cosmosdb \
    --resource-group wildlife-camera-rg \
    --name WildlifeData

# Create container
az cosmosdb sql container create \
    --account-name wildlife-cosmosdb \
    --resource-group wildlife-camera-rg \
    --database-name WildlifeData \
    --name Detections \
    --partition-key-path "/deviceId" \
    --throughput 400
```

### 7. Configure Custom Vision AI

```bash
# Create Custom Vision resources
az cognitiveservices account create \
    --name wildlife-vision-training \
    --resource-group wildlife-camera-rg \
    --kind CustomVision.Training \
    --sku F0 \
    --location westus2

az cognitiveservices account create \
    --name wildlife-vision-prediction \
    --resource-group wildlife-camera-rg \
    --kind CustomVision.Prediction \
    --sku F0 \
    --location westus2

# Get keys
az cognitiveservices account keys list \
    --name wildlife-vision-training \
    --resource-group wildlife-camera-rg
```

### 8. Set Up Monitoring with Application Insights

```bash
# Create Application Insights
az monitor app-insights component create \
    --app wildlife-camera-insights \
    --location westus2 \
    --resource-group wildlife-camera-rg \
    --application-type web

# Get instrumentation key
az monitor app-insights component show \
    --app wildlife-camera-insights \
    --resource-group wildlife-camera-rg \
    --query instrumentationKey
```

## Advanced Features

### IoT Hub Device Management

```cpp
// Send telemetry to IoT Hub
void sendTelemetry() {
    String telemetry = "{";
    telemetry += "\"temperature\":" + String(temperature) + ",";
    telemetry += "\"battery\":" + String(batteryLevel) + ",";
    telemetry += "\"signalStrength\":" + String(WiFi.RSSI());
    telemetry += "}";
    
    g_cloudIntegrator->uploadAnalyticsData(telemetry);
}

// Receive cloud-to-device messages
void receiveCloudCommands() {
    String command;
    if (g_cloudIntegrator->requestConfiguration()) {
        // Process configuration update
        updateDeviceConfiguration(command);
    }
}
```

### Azure Stream Analytics

Create Stream Analytics job:

```bash
# Create Stream Analytics job
az stream-analytics job create \
    --resource-group wildlife-camera-rg \
    --name wildlife-stream-analytics \
    --location westus2 \
    --sku Standard

# Define input (IoT Hub)
az stream-analytics input create \
    --resource-group wildlife-camera-rg \
    --job-name wildlife-stream-analytics \
    --name IoTHubInput \
    --type Stream \
    --datasource @iothub-input.json

# Define output (Cosmos DB)
az stream-analytics output create \
    --resource-group wildlife-camera-rg \
    --job-name wildlife-stream-analytics \
    --name CosmosDBOutput \
    --datasource @cosmosdb-output.json
```

Query for wildlife detections:

```sql
SELECT
    deviceId,
    species,
    confidence,
    location,
    System.Timestamp AS detectionTime
INTO
    CosmosDBOutput
FROM
    IoTHubInput
WHERE
    confidence > 0.8
```

## Cost Optimization

### Estimated Monthly Costs (100 devices)

- **Blob Storage (100 GB)**: ~$2.00
- **IoT Hub (S1 tier)**: ~$25.00
- **Functions (1M executions)**: ~$0.20
- **Cosmos DB (400 RU/s)**: ~$23.00
- **Custom Vision**: ~$5.00
- **Application Insights**: ~$2.00
- **Total**: ~$57.20/month

### Cost Reduction Tips

1. **Use Cool Storage Tier** for archival data:
```bash
az storage blob set-tier \
    --container-name images \
    --name old-image.jpg \
    --tier Cool \
    --account-name wildlifestorage
```

2. **Implement Lifecycle Management**:
```bash
az storage account management-policy create \
    --account-name wildlifestorage \
    --policy @lifecycle-policy.json
```

3. **Use Serverless Cosmos DB**:
```bash
az cosmosdb create \
    --name wildlife-cosmosdb \
    --resource-group wildlife-camera-rg \
    --capabilities EnableServerless
```

## Security Best Practices

1. **Enable Azure AD Authentication**
2. **Use Managed Identities** for services
3. **Implement Network Security Groups**
4. **Enable Advanced Threat Protection**
5. **Use Azure Key Vault** for secrets
6. **Enable audit logging**

### Store Secrets in Key Vault

```bash
# Create Key Vault
az keyvault create \
    --name wildlife-keyvault \
    --resource-group wildlife-camera-rg \
    --location westus2

# Store secrets
az keyvault secret set \
    --vault-name wildlife-keyvault \
    --name StorageAccountKey \
    --value "YOUR_STORAGE_KEY"

# Retrieve in ESP32
String secret = g_cloudIntegrator->getSecret("StorageAccountKey");
```

## Monitoring and Alerts

### Create Alert Rules

```bash
# Alert on high detection rate
az monitor metrics alert create \
    --name HighDetectionRate \
    --resource-group wildlife-camera-rg \
    --scopes /subscriptions/SUBSCRIPTION_ID/resourceGroups/wildlife-camera-rg/providers/Microsoft.Devices/IotHubs/wildlife-iot-hub \
    --condition "avg EventHubMessages > 1000" \
    --window-size 5m \
    --evaluation-frequency 1m \
    --action email your-email@example.com
```

## Troubleshooting

### Connection Issues

```cpp
// Test Azure connection
if (!g_cloudIntegrator->testConnection()) {
    Serial.println("Azure connection failed");
    
    // Check credentials
    if (azureConfig.accessKey.isEmpty()) {
        Serial.println("Storage account name not set");
    }
    
    // Check network
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
    }
}
```

### View Logs

```bash
# Function logs
az functionapp log tail \
    --name wildlife-image-processor \
    --resource-group wildlife-camera-rg

# Application Insights logs
az monitor app-insights query \
    --app wildlife-camera-insights \
    --analytics-query "traces | take 100"
```

## Support

- Azure Support: https://azure.microsoft.com/support/
- Azure Documentation: https://docs.microsoft.com/azure/
- Community: https://techcommunity.microsoft.com/
