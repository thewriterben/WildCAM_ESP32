# Quick Start Guide - Advanced Wildlife Monitoring Features

## 🚀 Get Started in 15 Minutes

This guide will help you quickly set up and test the advanced wildlife monitoring features including AI classification, dangerous species alerts, and motion pattern analysis.

## Prerequisites

### Hardware Required
- ESP32-S3 or ESP32-CAM board with PSRAM
- Camera module (OV2640 or OV5640)
- PIR motion sensor (HC-SR501)
- LoRa module (SX1276/SX1278) - optional
- Power supply (3.7V LiPo battery or USB)

### Software Required
- PlatformIO or Arduino IDE
- USB to Serial adapter (for ESP32-CAM)

---

## Step 1: Upload Basic Firmware (5 minutes)

### Option A: Using PlatformIO (Recommended)

```bash
# Clone repository
git clone https://github.com/thewriterben/WildCAM_ESP32.git
cd WildCAM_ESP32/ESP32WildlifeCAM-main

# Upload firmware
pio run -t upload -e esp32s3cam

# Monitor serial output
pio device monitor
```

### Option B: Using Arduino IDE

1. Open `examples/advanced_wildlife_monitoring_example.cpp`
2. Select board: `ESP32S3 Dev Module` or `AI Thinker ESP32-CAM`
3. Configure PSRAM: `OPI PSRAM` (for ESP32-S3)
4. Upload and open Serial Monitor (115200 baud)

---

## Step 2: Verify System Initialization (2 minutes)

After upload, you should see:

```
╔══════════════════════════════════════════╗
║  WildCAM ESP32 - Advanced Monitoring    ║
╚══════════════════════════════════════════╝

Initializing camera system...
✅ Camera system ready

Initializing AI wildlife classifier...
✅ AI classifier ready
   Supported species: 20+
   Confidence threshold: 70%

Initializing dangerous species alert system...
✅ Alert system ready
   Alert threshold: 70%
   Min consecutive detections: 2

Initializing motion pattern analyzer...
✅ Motion pattern analyzer ready
   Temporal analysis enabled
   False positive filtering active

Initializing hybrid motion detector...
✅ Motion detector ready

╔══════════════════════════════════════════╗
║  System Ready - Monitoring Wildlife      ║
╚══════════════════════════════════════════╝
```

**If you see errors:**
- Check camera connection (GPIO pins)
- Verify PSRAM is enabled in board settings
- Ensure sufficient power supply (500mA+)

---

## Step 3: Test Motion Detection (3 minutes)

Wave your hand in front of the camera or PIR sensor:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Motion Detected! (Confidence: 0.85)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
📊 Pattern Analysis: Animal Walking - Steady rhythmic movement
   Confidence: 75.2%
   Animal Motion: Yes
```

**Expected Results:**
- PIR sensor triggers on movement
- Frame analysis confirms motion
- Pattern analyzer identifies motion type
- System proceeds to AI classification

---

## Step 4: Test AI Classification (3 minutes)

Show the camera an image of wildlife (phone screen or printed photo):

```
📸 Capturing image for AI classification...

🦌 Wildlife Classification Results:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   Species: White-tailed Deer
   Confidence: 82.3%
   Confidence Level: High (75-90%)
   Inference Time: 178 ms
   Animals Detected: 1
```

**Test Species:**
- Use wildlife photos from your phone
- Test with different species
- Verify confidence scores

---

## Step 5: Test Dangerous Species Alert (2 minutes)

Show an image of a bear, wolf, or mountain lion:

```
   ⚠️  DANGEROUS SPECIES DETECTED!

🚨 ALERT GENERATED:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   ALERT: Black Bear detected (Confidence: 92.5%)
   CRITICAL: Large predator detected! Exercise extreme caution.
   Priority: CRITICAL
   Immediate Action Required: YES
   Detection Count: 2
   ✅ Alert transmitted via LoRa mesh
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**Alert Verification:**
- Confirm consecutive detection requirement
- Check alert cooldown works (60 seconds)
- Verify priority levels
- Test LoRa transmission (if equipped)

---

## Configuration Options

### Adjust Sensitivity

```cpp
// In your code or config.h

// AI Confidence (0.0 to 1.0)
classifier.setConfidenceThreshold(0.70f);  // Default: 70%

// Alert Threshold (0.0 to 1.0)
alertSystem.setConfidenceThreshold(0.70f); // Default: 70%

// Consecutive Detections (1 to 10)
alertSystem.setMinConsecutiveDetections(2); // Default: 2

// Pattern Sensitivity (0.0 to 1.0)
patternAnalyzer.setSensitivity(0.6f);      // Default: 0.6
```

### Power Saving Mode

```cpp
// Enable low power mode
motionDetector.setLowPowerMode(true);

// Adjust motion check interval
#define MOTION_CHECK_INTERVAL 5000  // 5 seconds (default: 1000ms)
```

### LoRa Configuration

```cpp
// Set your location
LoRaWildlifeAlerts::setLocation(45.5231, -122.6765);

// Configure LoRa parameters
LoRa.setFrequency(915E6);      // 915 MHz (US)
LoRa.setTxPower(17);           // 17 dBm
LoRa.setSpreadingFactor(7);    // SF7 (fastest)
```

---

## Troubleshooting

### Camera Not Working

```cpp
// Check camera initialization
if (!cameraManager.initialize()) {
    Serial.println("Camera failed!");
    // Check:
    // - GPIO pin configuration
    // - Camera module connection
    // - Power supply (needs 3.3V regulated)
}
```

**Common Fixes:**
1. Press camera reset button
2. Check ribbon cable connection
3. Verify camera model in config (`CAMERA_MODEL_AI_THINKER`)
4. Try different camera module

### Out of Memory

```
ERROR: Failed to allocate tensor arena
```

**Solutions:**
1. Enable PSRAM in board settings
2. Reduce tensor arena size:
   ```cpp
   #define TFLITE_ARENA_SIZE (200 * 1024)  // Reduce from 256KB
   ```
3. Use ESP32-S3 with 8MB PSRAM

### High False Positive Rate

**Adjust thresholds:**
```cpp
// Increase confidence thresholds
classifier.setConfidenceThreshold(0.80f);
alertSystem.setConfidenceThreshold(0.80f);

// Require more consecutive detections
alertSystem.setMinConsecutiveDetections(3);

// Increase pattern sensitivity
patternAnalyzer.setSensitivity(0.7f);
```

### LoRa Not Connecting

**Check configuration:**
```cpp
// Verify frequency band for your region
// US: 915 MHz
// EU: 868 MHz
// Asia: 433 MHz

LoRa.setFrequency(915E6);  // Set correctly for region

// Check SPI pins
// MISO, MOSI, SCK, SS
```

---

## Next Steps

### 1. Field Deployment

- Mount camera in weatherproof enclosure
- Position at wildlife trail or feeding area
- Ensure clear camera view (no branches)
- Test PIR sensor range (5-7 meters)

### 2. Advanced Configuration

- Read [Advanced Features Guide](ADVANCED_FEATURES_GUIDE.md)
- Customize species list for your region
- Adjust power management settings
- Configure multi-camera mesh network

### 3. Data Analysis

- Review detection statistics
- Analyze wildlife patterns
- Export data for research
- Generate behavior reports

### 4. Integration

- Connect to cloud services
- Set up email/SMS notifications
- Create mobile app connection
- Enable remote monitoring

---

## Example Output

### Successful Detection Sequence

```
Motion Detected! (Confidence: 0.85)
📊 Pattern Analysis: Animal Walking
   Animal Motion: Yes

📸 Capturing image...

🦌 Classification: White-tailed Deer
   Confidence: 82.3%
   Inference: 178ms
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Dangerous Species Alert

```
Motion Detected! (Confidence: 0.92)
📊 Pattern Analysis: Animal Walking
   Animal Motion: Yes

📸 Capturing image...

🦌 Classification: Black Bear
   Confidence: 92.5%
   ⚠️ DANGEROUS SPECIES!

🚨 CRITICAL ALERT
   Large predator detected!
   Exercise extreme caution.
   ✅ Alert sent via LoRa
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### False Positive Filtered

```
Motion Detected! (Confidence: 0.45)
📊 Pattern Analysis: Environmental
   Wind/vegetation/shadows
⚠️ False positive - Skipping classification
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## Performance Metrics

After running for a while, check statistics:

```
╔══════════════════════════════════════════╗
║         System Statistics                ║
╚══════════════════════════════════════════╝
System Uptime: 3600 seconds (1 hour)
Total Detections: 45
Animal Detections: 32
Dangerous Species: 3
Alerts Sent: 2
False Positives Filtered: 13

AI Classifier:
  Total Inferences: 32
  Successful: 30
  Avg Confidence: 78.5%
  Avg Inference Time: 182 ms

Alert System:
  Total Alerts: 2
  Critical Alerts: 1

Pattern Analyzer:
  Patterns Analyzed: 45
  False Positives: 13
══════════════════════════════════════════
```

---

## Support Resources

- **Documentation**: [Advanced Features Guide](ADVANCED_FEATURES_GUIDE.md)
- **Examples**: `/examples/advanced_wildlife_monitoring_example.cpp`
- **Tests**: `/test/test_dangerous_species_alerts.cpp`
- **GitHub Issues**: Report bugs or request features
- **Community**: Join discussions for tips and help

---

## What's Next?

You now have a working advanced wildlife monitoring system! 

**Explore more features:**
- Set up multi-camera mesh network
- Configure cloud integration
- Add GPS tracking
- Enable satellite backup
- Create custom alerts
- Analyze behavioral patterns

**Ready for production:**
- Deploy in field location
- Monitor for 24 hours
- Adjust settings based on results
- Scale to multiple cameras

---

**Happy Wildlife Monitoring! 🦌📸**
