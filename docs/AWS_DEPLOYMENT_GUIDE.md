# AWS Deployment Guide for WildCAM ESP32

## Overview

This guide walks through deploying WildCAM ESP32 with Amazon Web Services (AWS) cloud platform integration. AWS provides enterprise-grade infrastructure for wildlife monitoring at any scale.

## Prerequisites

- AWS Account with billing enabled
- AWS CLI installed and configured
- Basic knowledge of AWS services
- ESP32-CAM hardware with internet connectivity

## Architecture

```
┌─────────────┐
│  ESP32-CAM  │
│   Device    │
└──────┬──────┘
       │ HTTPS/MQTT
       │
┌──────▼──────────────────────────────────┐
│           AWS Cloud Platform            │
├─────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐            │
│  │ IoT Core │  │ API GW   │            │
│  └────┬─────┘  └────┬─────┘            │
│       │             │                   │
│  ┌────▼─────────────▼─────┐            │
│  │     Lambda Functions    │            │
│  └────┬────────────────────┘            │
│       │                                 │
│  ┌────▼────┐  ┌────────┐  ┌─────────┐ │
│  │   S3    │  │DynamoDB│  │SageMaker│ │
│  │ Storage │  │        │  │   ML    │ │
│  └─────────┘  └────────┘  └─────────┘ │
└─────────────────────────────────────────┘
```

## Step-by-Step Setup

### 1. Create S3 Bucket for Data Storage

#### Using AWS Console

1. Navigate to S3 console
2. Click "Create bucket"
3. Bucket name: `wildlife-camera-data-[your-unique-id]`
4. Region: Select closest to deployment (e.g., `us-west-2`)
5. Enable "Bucket Versioning" for data protection
6. Enable "Default encryption" with SSE-S3
7. Click "Create bucket"

#### Using AWS CLI

```bash
# Create bucket
aws s3 mb s3://wildlife-camera-data-unique-id --region us-west-2

# Enable versioning
aws s3api put-bucket-versioning \
    --bucket wildlife-camera-data-unique-id \
    --versioning-configuration Status=Enabled

# Enable encryption
aws s3api put-bucket-encryption \
    --bucket wildlife-camera-data-unique-id \
    --server-side-encryption-configuration '{
        "Rules": [{
            "ApplyServerSideEncryptionByDefault": {
                "SSEAlgorithm": "AES256"
            }
        }]
    }'

# Create folder structure
aws s3api put-object --bucket wildlife-camera-data-unique-id --key images/
aws s3api put-object --bucket wildlife-camera-data-unique-id --key videos/
aws s3api put-object --bucket wildlife-camera-data-unique-id --key detections/
aws s3api put-object --bucket wildlife-camera-data-unique-id --key telemetry/
```

### 2. Create IAM User and Permissions

#### Create IAM Policy

Create file `wildlife-camera-policy.json`:

```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "s3:PutObject",
                "s3:GetObject",
                "s3:DeleteObject",
                "s3:ListBucket"
            ],
            "Resource": [
                "arn:aws:s3:::wildlife-camera-data-unique-id",
                "arn:aws:s3:::wildlife-camera-data-unique-id/*"
            ]
        },
        {
            "Effect": "Allow",
            "Action": [
                "iot:Connect",
                "iot:Publish",
                "iot:Subscribe",
                "iot:Receive"
            ],
            "Resource": "*"
        }
    ]
}
```

#### Create IAM User

```bash
# Create IAM user
aws iam create-user --user-name wildlife-camera-device

# Create and attach policy
aws iam create-policy \
    --policy-name WildlifeCameraPolicy \
    --policy-document file://wildlife-camera-policy.json

# Attach policy to user
aws iam attach-user-policy \
    --user-name wildlife-camera-device \
    --policy-arn arn:aws:iam::ACCOUNT_ID:policy/WildlifeCameraPolicy

# Create access keys
aws iam create-access-key --user-name wildlife-camera-device
```

Save the `AccessKeyId` and `SecretAccessKey` - you'll need these for ESP32 configuration.

### 3. Configure AWS IoT Core (Optional but Recommended)

#### Create Thing

```bash
# Create IoT thing
aws iot create-thing --thing-name esp32-wildlife-cam-001

# Create certificate
aws iot create-keys-and-certificate \
    --set-as-active \
    --certificate-pem-outfile esp32-cert.pem \
    --public-key-outfile esp32-public.key \
    --private-key-outfile esp32-private.key

# Create IoT policy
aws iot create-policy \
    --policy-name WildlifeCameraIoTPolicy \
    --policy-document '{
        "Version": "2012-10-17",
        "Statement": [{
            "Effect": "Allow",
            "Action": ["iot:*"],
            "Resource": ["*"]
        }]
    }'

# Attach policy to certificate
aws iot attach-policy \
    --policy-name WildlifeCameraIoTPolicy \
    --target CERTIFICATE_ARN
```

### 4. Set Up Lambda Functions for Image Processing

#### Create Lambda Function

Create file `image-processor.py`:

```python
import json
import boto3
import base64
from datetime import datetime

s3 = boto3.client('s3')
rekognition = boto3.client('rekognition')

def lambda_handler(event, context):
    try:
        # Extract image data from event
        bucket = event['bucket']
        key = event['key']
        
        # Analyze image with Rekognition
        response = rekognition.detect_labels(
            Image={'S3Object': {'Bucket': bucket, 'Name': key}},
            MaxLabels=10,
            MinConfidence=70
        )
        
        # Extract wildlife labels
        wildlife_labels = []
        for label in response['Labels']:
            if label['Name'] in ['Animal', 'Wildlife', 'Mammal', 'Bird']:
                wildlife_labels.append({
                    'name': label['Name'],
                    'confidence': label['Confidence']
                })
        
        # Store results
        result_key = f"analysis/{key}.json"
        s3.put_object(
            Bucket=bucket,
            Key=result_key,
            Body=json.dumps(wildlife_labels),
            ContentType='application/json'
        )
        
        return {
            'statusCode': 200,
            'body': json.dumps({
                'message': 'Analysis complete',
                'labels': wildlife_labels
            })
        }
    except Exception as e:
        print(f"Error: {str(e)}")
        return {
            'statusCode': 500,
            'body': json.dumps({'error': str(e)})
        }
```

#### Deploy Lambda Function

```bash
# Create deployment package
zip lambda-function.zip image-processor.py

# Create Lambda function
aws lambda create-function \
    --function-name wildlife-image-processor \
    --runtime python3.9 \
    --role arn:aws:iam::ACCOUNT_ID:role/lambda-execution-role \
    --handler image-processor.lambda_handler \
    --zip-file fileb://lambda-function.zip \
    --timeout 30 \
    --memory-size 512

# Add S3 trigger
aws lambda add-permission \
    --function-name wildlife-image-processor \
    --statement-id s3-trigger \
    --action lambda:InvokeFunction \
    --principal s3.amazonaws.com \
    --source-arn arn:aws:s3:::wildlife-camera-data-unique-id
```

### 5. Configure ESP32 Device

#### Update Configuration

Edit your ESP32 code to include AWS credentials:

```cpp
#include "production/enterprise/cloud_integrator.h"

// AWS Configuration
CloudConfig awsConfig;
awsConfig.platform = CLOUD_AWS;
awsConfig.region = "us-west-2";
awsConfig.endpoint = "https://s3.us-west-2.amazonaws.com";
awsConfig.accessKey = "AKIAIOSFODNN7EXAMPLE"; // Replace with your key
awsConfig.secretKey = "wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY"; // Replace
awsConfig.bucketName = "wildlife-camera-data-unique-id";
awsConfig.useSSL = true;
awsConfig.compressData = true;
awsConfig.encryptData = true;
awsConfig.syncMode = SYNC_OFFLINE_FIRST;
awsConfig.syncInterval = 300; // 5 minutes

// Initialize cloud integration
if (!initializeCloudIntegration(awsConfig)) {
    Serial.println("Failed to initialize AWS cloud integration");
    return;
}

Serial.println("AWS cloud integration initialized successfully");
```

#### Upload Test Image

```cpp
void uploadTestImage() {
    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }
    
    // Save to SD card
    File file = SD.open("/test_image.jpg", FILE_WRITE);
    file.write(fb->buf, fb->len);
    file.close();
    
    // Upload to AWS
    UploadRequest request;
    request.requestId = "test-001";
    request.dataType = DATA_IMAGE;
    request.localFilePath = "/test_image.jpg";
    request.cloudPath = "images/test_image.jpg";
    request.metadata = "{\"test\":true}";
    request.priority = 2;
    
    if (g_cloudIntegrator->uploadFileAsync(request)) {
        Serial.println("Image upload queued");
    }
    
    esp_camera_fb_return(fb);
}
```

### 6. Set Up DynamoDB for Metadata

#### Create DynamoDB Table

```bash
# Create table for wildlife detections
aws dynamodb create-table \
    --table-name wildlife-detections \
    --attribute-definitions \
        AttributeName=deviceId,AttributeType=S \
        AttributeName=timestamp,AttributeType=N \
    --key-schema \
        AttributeName=deviceId,KeyType=HASH \
        AttributeName=timestamp,KeyType=RANGE \
    --billing-mode PAY_PER_REQUEST \
    --region us-west-2
```

#### Store Detection Metadata

```cpp
bool storeDetectionMetadata(const String& species, float confidence) {
    String payload = "{";
    payload += "\"deviceId\":\"" + WiFi.macAddress() + "\",";
    payload += "\"timestamp\":" + String(millis()) + ",";
    payload += "\"species\":\"" + species + "\",";
    payload += "\"confidence\":" + String(confidence) + ",";
    payload += "\"location\":{\"lat\":45.5231,\"lon\":-122.6765}";
    payload += "}";
    
    return g_cloudIntegrator->insertRecord("wildlife-detections", payload);
}
```

### 7. Set Up CloudWatch for Monitoring

#### Create CloudWatch Dashboard

```bash
# Create dashboard
aws cloudwatch put-dashboard \
    --dashboard-name wildlife-camera-monitoring \
    --dashboard-body file://dashboard.json
```

Dashboard configuration (`dashboard.json`):

```json
{
    "widgets": [
        {
            "type": "metric",
            "properties": {
                "metrics": [
                    ["AWS/S3", "NumberOfObjects", {"stat": "Average"}]
                ],
                "period": 300,
                "stat": "Average",
                "region": "us-west-2",
                "title": "Images Uploaded"
            }
        },
        {
            "type": "metric",
            "properties": {
                "metrics": [
                    ["AWS/Lambda", "Invocations", {"stat": "Sum"}]
                ],
                "period": 300,
                "stat": "Sum",
                "region": "us-west-2",
                "title": "Image Processing"
            }
        }
    ]
}
```

### 8. Configure Alerts

#### Create SNS Topic for Alerts

```bash
# Create SNS topic
aws sns create-topic --name wildlife-camera-alerts

# Subscribe email
aws sns subscribe \
    --topic-arn arn:aws:sns:us-west-2:ACCOUNT_ID:wildlife-camera-alerts \
    --protocol email \
    --notification-endpoint your-email@example.com

# Create CloudWatch alarm
aws cloudwatch put-metric-alarm \
    --alarm-name wildlife-detection-alert \
    --alarm-description "Alert on wildlife detection" \
    --actions-enabled \
    --alarm-actions arn:aws:sns:us-west-2:ACCOUNT_ID:wildlife-camera-alerts \
    --metric-name Invocations \
    --namespace AWS/Lambda \
    --statistic Sum \
    --period 300 \
    --evaluation-periods 1 \
    --threshold 10 \
    --comparison-operator GreaterThanThreshold
```

## Advanced Features

### Using AWS SageMaker for ML

#### Deploy Custom Model

```bash
# Create SageMaker model
aws sagemaker create-model \
    --model-name wildlife-classifier \
    --primary-container Image=ACCOUNT_ID.dkr.ecr.us-west-2.amazonaws.com/wildlife-model:latest,ModelDataUrl=s3://bucket/model.tar.gz \
    --execution-role-arn arn:aws:iam::ACCOUNT_ID:role/SageMakerRole

# Create endpoint configuration
aws sagemaker create-endpoint-config \
    --endpoint-config-name wildlife-classifier-config \
    --production-variants VariantName=default,ModelName=wildlife-classifier,InitialInstanceCount=1,InstanceType=ml.t2.medium

# Create endpoint
aws sagemaker create-endpoint \
    --endpoint-name wildlife-classifier \
    --endpoint-config-name wildlife-classifier-config
```

### Using AWS Rekognition

```cpp
bool analyzeImageWithRekognition(const String& imagePath) {
    // Upload image to S3 first
    CloudResponse response = g_cloudIntegrator->uploadFile(
        imagePath, 
        "analysis/" + imagePath, 
        DATA_IMAGE
    );
    
    if (!response.success) {
        return false;
    }
    
    // Trigger Rekognition via Lambda
    String payload = "{";
    payload += "\"bucket\":\"" + awsConfig.bucketName + "\",";
    payload += "\"key\":\"analysis/" + imagePath + "\"";
    payload += "}";
    
    return g_cloudIntegrator->requestInference(payload, analysisResult);
}
```

## Cost Optimization

### Estimated Monthly Costs (100 devices)

- **S3 Storage (100 GB)**: ~$2.30
- **S3 Requests (100K)**: ~$0.50
- **Lambda (1M invocations)**: ~$0.20
- **IoT Core (1M messages)**: ~$1.00
- **Data Transfer (50 GB)**: ~$4.50
- **CloudWatch**: ~$1.00
- **Total**: ~$9.50/month

### Cost Reduction Strategies

1. **S3 Lifecycle Policies**:
```bash
aws s3api put-bucket-lifecycle-configuration \
    --bucket wildlife-camera-data-unique-id \
    --lifecycle-configuration file://lifecycle.json
```

2. **Compress Data**:
```cpp
awsConfig.compressData = true; // Reduces storage by 60-70%
```

3. **Use Infrequent Access Storage**:
```bash
# Move old data to IA
aws s3 cp s3://bucket/old-data/ s3://bucket/old-data/ \
    --storage-class STANDARD_IA --recursive
```

4. **Reserved Capacity** for predictable workloads

## Troubleshooting

### Connection Issues

```cpp
// Test AWS connection
if (!g_cloudIntegrator->testConnection()) {
    Serial.println("AWS connection failed");
    Serial.println("Checking credentials...");
    
    if (awsConfig.accessKey.isEmpty()) {
        Serial.println("ERROR: Access key not set");
    }
    
    if (awsConfig.secretKey.isEmpty()) {
        Serial.println("ERROR: Secret key not set");
    }
    
    Serial.println("Checking network...");
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("ERROR: WiFi not connected");
    }
}
```

### Upload Failures

Check CloudWatch logs:

```bash
aws logs tail /aws/lambda/wildlife-image-processor --follow
```

## Security Best Practices

1. **Rotate Access Keys** regularly (every 90 days)
2. **Use IAM Roles** instead of access keys when possible
3. **Enable MFA** for AWS account
4. **Encrypt Data** in transit and at rest
5. **Use VPC** for additional network isolation
6. **Monitor Access** with CloudTrail
7. **Implement Least Privilege** access

## Monitoring and Alerts

### Key Metrics to Monitor

1. S3 bucket size and object count
2. Lambda function errors and duration
3. IoT Core connection status
4. DynamoDB read/write capacity
5. Network data transfer

### Creating Custom Metrics

```cpp
void reportCustomMetrics() {
    String payload = "{";
    payload += "\"namespace\":\"WildlifeCamera\",";
    payload += "\"metricName\":\"DetectionRate\",";
    payload += "\"value\":\"" + String(detectionCount) + "\",";
    payload += "\"unit\":\"Count\"";
    payload += "}";
    
    // Send to CloudWatch via API
}
```

## Next Steps

1. Set up automated backups
2. Configure multi-region replication
3. Implement disaster recovery plan
4. Set up CI/CD pipeline for Lambda functions
5. Create custom dashboards for monitoring
6. Integrate with third-party tools

## Support

For AWS-specific issues:
- AWS Support: https://console.aws.amazon.com/support/
- AWS Forums: https://forums.aws.amazon.com/
- AWS Documentation: https://docs.aws.amazon.com/

For WildCAM integration issues:
- GitHub Issues: https://github.com/thewriterben/WildCAM_ESP32/issues
