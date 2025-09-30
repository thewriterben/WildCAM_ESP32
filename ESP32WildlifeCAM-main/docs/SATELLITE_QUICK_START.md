# Satellite Communication Quick Start Guide

## Overview

This guide will help you quickly set up satellite communication for your ESP32 Wildlife Camera, enabling global connectivity in remote areas without cellular or WiFi coverage.

## Quick Module Selection

| Module | Best For | Cost/Message | Setup Difficulty |
|--------|----------|--------------|------------------|
| **Swarm M138** | Regular monitoring, frequent updates | $0.05 | Easy |
| **Iridium 9603N** | Emergency alerts, critical data | $0.95 | Moderate |
| **RockBLOCK** | Professional deployments, reliability | $0.95 | Easy |

### Recommendation
- **Budget deployments**: Start with **Swarm M138** (lowest cost)
- **Critical deployments**: Use **Iridium/RockBLOCK** (proven reliability)
- **Hybrid approach**: Combine both for cost optimization

## Hardware Requirements

### Required Components
1. **ESP32-CAM Module** (any variant)
2. **Satellite Module** (choose one):
   - Swarm M138 Modem (~$119)
   - Iridium 9603N Module (~$200)
   - RockBLOCK 9603 (~$250 with antenna)
3. **Antenna** (specific to module)
4. **Jumper Wires** (5-6 wires)
5. **Power Supply** (3.3V capable, 3A recommended)

### Optional Components
- Level shifter (if using 5V power)
- External antenna mount
- Waterproof enclosure with RF-transparent window

## 5-Minute Setup

### Step 1: Physical Wiring

#### Swarm M138 Wiring
```
ESP32-CAM Pin  →  Swarm M138 Pin
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO 25        →  RX
GPIO 26        →  TX
GPIO 27        →  SLEEP/EN
3.3V           →  VCC
GND            →  GND
```

#### Iridium/RockBLOCK Wiring
```
ESP32-CAM Pin  →  Module Pin
━━━━━━━━━━━━━━━━━━━━━━━━━━
GPIO 25        →  RX
GPIO 26        →  TX
GPIO 27        →  SLEEP
GPIO 33        →  RING (optional)
3.3V           →  VCC
GND            →  GND
```

**⚠️ Important Notes:**
- Double-check voltage! Most modules are 3.3V only
- Ensure antenna has clear sky view (no metal obstructions)
- Keep antenna cable as short as possible

### Step 2: Software Configuration

#### Minimal Code Example
```cpp
#include <Arduino.h>
#include "satellite_integration.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize satellite communication
    if (SATELLITE_INIT()) {
        Serial.println("✅ Satellite ready!");
        
        // Configure for your deployment
        satelliteIntegration.configureForEnvironment(true); // true = remote
    } else {
        Serial.println("❌ Satellite init failed");
    }
}

void loop() {
    // Example: Send wildlife alert
    SATELLITE_WILDLIFE_ALERT("DEER", 0.85, nullptr, 0);
    
    delay(3600000); // Wait 1 hour between transmissions
}
```

#### Full Configuration Example
```cpp
#include "satellite_comm.h"
#include "satellite_config.h"

SatelliteComm satComm;
SatelliteConfig config;

void setup() {
    Serial.begin(115200);
    
    // Configure satellite settings
    config.enabled = true;
    config.module = MODULE_SWARM;  // or MODULE_IRIDIUM or MODULE_ROCKBLOCK
    config.transmissionInterval = 3600;  // 1 hour between transmissions
    config.maxDailyMessages = 24;        // Max 24 messages per day
    config.maxDailyCost = 5.00;          // Max $5 per day
    config.costOptimization = true;      // Enable smart scheduling
    config.emergencyThreshold = 20;      // Alert at 20% battery
    
    // Apply configuration
    satComm.configure(config);
    
    // Initialize the module
    if (satComm.initialize(config.module)) {
        Serial.println("✅ Satellite initialized successfully!");
    } else {
        Serial.println("❌ Satellite initialization failed");
        Serial.println("Check wiring and antenna placement");
    }
}

void loop() {
    // Your wildlife detection code here
    
    // Example: Check battery and send emergency if low
    uint8_t batteryLevel = readBatteryLevel(); // Your battery reading function
    if (batteryLevel < 20) {
        satComm.sendEmergencyAlert("LOW_BATTERY:" + String(batteryLevel) + "%");
    }
    
    // Send periodic status
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 3600000) {  // Every hour
        satComm.sendStatusReport();
        lastStatus = millis();
    }
    
    delay(1000);
}
```

### Step 3: First Test

1. **Upload the code** to your ESP32-CAM
2. **Open Serial Monitor** (115200 baud)
3. **Watch for initialization messages**:
   ```
   Initializing satellite communication...
   Initializing Swarm M138 module
   Swarm module responding
   ✅ Satellite communication initialized successfully
   ```

4. **Test message sending**:
   ```cpp
   satComm.sendMessage("TEST", SAT_PRIORITY_HIGH);
   ```

5. **Check output** for confirmation:
   ```
   Message sent successfully on attempt 1. Daily count: 1/24, Cost: $0.05
   ```

## Cost Management

### Daily Cost Calculator

```cpp
// For Swarm M138
int messagesPerDay = 24;
float dailyCost = messagesPerDay * 0.05;  // $1.20/day
float monthlyCost = dailyCost * 30;       // $36/month

// For Iridium/RockBLOCK
int messagesPerDay = 24;
float dailyCost = messagesPerDay * 0.95;  // $22.80/day
float monthlyCost = dailyCost * 30;       // $684/month
```

### Cost Optimization Tips

1. **Use message priorities**: Emergency messages bypass limits, normal messages wait
2. **Enable cost optimization**: Automatically schedules messages during optimal windows
3. **Compress messages**: Built-in compression saves bandwidth
4. **Batch transmissions**: Store and send multiple messages in one session
5. **Set daily limits**: Prevent cost overruns with hard limits

```cpp
// Example cost optimization config
config.costOptimization = true;
config.maxDailyCost = 5.00;          // Hard limit
config.useScheduledTransmission = true;  // Wait for optimal windows
```

## Common Issues & Solutions

### Issue: Module Not Responding

**Symptoms**: "Failed to initialize satellite module" in serial output

**Solutions**:
1. ✓ Verify wiring (especially RX/TX - they are crossed!)
2. ✓ Check power supply (module needs 3A peak)
3. ✓ Ensure antenna is connected
4. ✓ Try different GPIO pins if conflicts exist
5. ✓ Measure voltage at module VCC pin (should be 3.0-3.6V)

### Issue: No Satellite Lock

**Symptoms**: "No satellite connectivity" messages

**Solutions**:
1. ✓ Move antenna to open area with clear sky view
2. ✓ Wait 5-10 minutes for initial satellite acquisition
3. ✓ Check antenna cable is not damaged
4. ✓ Verify module SIM/subscription is active (for Iridium/RockBLOCK)
5. ✓ Check signal quality: `satComm.checkSignalQuality()`

### Issue: Messages Not Sending

**Symptoms**: Messages stored but not transmitted

**Solutions**:
1. ✓ Check daily message limit: `satComm.getMessageCount()`
2. ✓ Check daily cost limit: `satComm.getTodayCost()`
3. ✓ Verify satellite availability: `satComm.checkSatelliteAvailability()`
4. ✓ Try manual transmission: `satComm.sendStoredMessages()`
5. ✓ Enable emergency mode: `satComm.enableEmergencyMode()`

### Issue: High Power Consumption

**Symptoms**: Battery draining quickly

**Solutions**:
1. ✓ Enable low power mode: `satComm.enterSleepMode()` between transmissions
2. ✓ Increase transmission interval (e.g., 2-4 hours)
3. ✓ Use scheduled transmissions with pass prediction
4. ✓ Reduce daily message count
5. ✓ Implement battery-aware transmission:
   ```cpp
   satComm.optimizePowerConsumption(batteryLevel);
   ```

## Advanced Features

### Emergency Mode

Automatically triggers when battery is critically low:
```cpp
// Configure emergency threshold
config.emergencyThreshold = 20;  // Trigger at 20% battery

// Or manually enable
satComm.enableEmergencyMode();

// Send emergency alert
satComm.sendEmergencyAlert("SYSTEM_CRITICAL:BATTERY_LOW");
```

### Message Prioritization

Control when messages are sent based on importance:
```cpp
// Low priority - sent during optimal windows only
satComm.sendMessage("STATUS:OK", SAT_PRIORITY_LOW);

// Normal priority - sent during regular windows
satComm.sendMessage("WILDLIFE:DEER", SAT_PRIORITY_NORMAL);

// High priority - sent as soon as possible
satComm.sendMessage("RARE_SPECIES:WOLF", SAT_PRIORITY_HIGH);

// Emergency priority - bypasses all limits
satComm.sendMessage("EMERGENCY:FIRE", SAT_PRIORITY_EMERGENCY);
```

### Satellite Pass Prediction

Optimize transmission timing based on satellite orbits:
```cpp
// Enable auto pass prediction
config.autoPassPrediction = true;

// Check next satellite pass
satComm.predictNextPass();
time_t nextPass = satComm.getNextPassTime();
Serial.printf("Next satellite pass: %ld\n", nextPass);

// Get optimal transmission window
TransmissionWindow window = satComm.getOptimalTransmissionWindow();
```

### Message Compression

Automatically compress messages to save bandwidth and cost:
```cpp
// Enable compression (recommended)
config.enableMessageCompression = true;

// Messages are automatically compressed
// Example: "WILDLIFE: Deer detected" → "WL:Deer detected"
```

## Production Deployment Checklist

### Pre-Deployment
- [ ] Test satellite module indoors with antenna outside
- [ ] Verify all messages send successfully (10+ test messages)
- [ ] Check signal quality at deployment location
- [ ] Configure daily cost limits
- [ ] Set up emergency contact procedures
- [ ] Document module serial number and subscription details
- [ ] Test battery life (24 hour minimum)

### Deployment Day
- [ ] Final antenna placement test (clear sky view)
- [ ] Verify solar panel charging (if applicable)
- [ ] Send initial test message from site
- [ ] Configure check-in schedule
- [ ] Enable emergency mode settings
- [ ] Note GPS coordinates for reference

### Post-Deployment
- [ ] Monitor message delivery (first 48 hours)
- [ ] Check daily cost reports
- [ ] Verify battery levels daily (first week)
- [ ] Adjust transmission intervals if needed
- [ ] Plan maintenance schedule

## Getting Help

### Resources
- **Full Documentation**: See `SATELLITE_COMMUNICATION.md`
- **Wiring Diagrams**: See `SATELLITE_WIRING_DIAGRAMS.md`
- **Test Code**: See `examples/main_with_satellite.cpp`
- **Hardware Guide**: See `docs/HARDWARE_GUIDE.md`

### Support Contacts
- **Swarm Support**: support@swarm.space
- **Iridium Support**: support@iridium.com
- **RockBLOCK Support**: support@rock7.com

### Troubleshooting Script

Run the built-in validation script:
```bash
python3 validate_satellite_comm.py
```

This checks:
- ✓ All required files present
- ✓ Configuration constants defined
- ✓ Class structure correct
- ✓ Documentation complete
- ✓ Test coverage adequate

## Next Steps

1. **Review** the full `SATELLITE_COMMUNICATION.md` documentation
2. **Experiment** with different configurations
3. **Monitor** costs and adjust limits
4. **Optimize** transmission schedules for your use case
5. **Join** the community to share experiences

## Example Deployment Scenarios

### Scenario 1: Budget Wildlife Monitoring
- **Module**: Swarm M138
- **Frequency**: Every 4 hours
- **Daily Messages**: 6
- **Daily Cost**: $0.30
- **Monthly Cost**: $9.00
- **Best For**: Long-term studies, regular monitoring

### Scenario 2: Critical Species Tracking
- **Module**: Iridium 9603N
- **Frequency**: Every hour
- **Daily Messages**: 24
- **Daily Cost**: $22.80
- **Monthly Cost**: $684
- **Best For**: Endangered species, high-value research

### Scenario 3: Hybrid Approach
- **Primary**: Swarm M138 (regular updates)
- **Backup**: Iridium (emergencies only)
- **Daily Messages**: 12 Swarm + 2 Iridium
- **Daily Cost**: $0.60 + $1.90 = $2.50
- **Monthly Cost**: $75
- **Best For**: Professional deployments, reliability critical

---

**Ready to deploy?** Start with the minimal code example above and expand as needed. Remember to test thoroughly before field deployment!
