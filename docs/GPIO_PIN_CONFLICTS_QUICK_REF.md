# GPIO Pin Conflicts - Quick Reference Card

**For WildCAM ESP32 Developers**

## 🚨 Critical Information

**AI-Thinker ESP32-CAM has only ~6 free GPIO pins after camera configuration!**

## ⚡ Quick Decision Guide

```
Need Camera + LoRa?  → Use ESP32-S3-CAM ($20)
Need Camera only?    → Use AI-Thinker ($10) ✓
Need LoRa only?      → Use AI-Thinker with camera disabled
Need Everything?     → Use ESP32-S3-CAM
```

## 📌 Pin Conflicts at a Glance

| Feature Combo | AI-Thinker | ESP32-S3 | Solution |
|--------------|-----------|----------|----------|
| Camera + LoRa | ❌ | ✅ | Upgrade or choose one |
| Camera + SD | ✅ | ✅ | Works fine |
| Camera + Sensors | ✅ | ✅ | Use I2C (GPIO 26/27) |
| SD + LEDs | ❌ | ✅ | Disable LEDs |
| Camera + Battery Monitor | ⚠️ | ✅ | Can share GPIO 34 |

## 🔌 Available Pins (AI-Thinker + Camera + SD)

```
GPIO 1  - PIR sensor / UART TX
GPIO 3  - UART RX  
GPIO 16 - LoRa RST / IR LED
GPIO 17 - PIR power / Servo
GPIO 33 - Battery/Light (ADC)
```

**That's it! Only 5 pins free!**

## 🚫 Never Use These

```
GPIO 6-11  - Flash memory (NEVER!)
GPIO 0     - Boot mode (camera XCLK)
GPIO 2,15  - Boot mode (SD card when enabled)
GPIO 12    - Flash voltage (SD card)
```

## ✅ Valid Configurations

### Config 1: Wildlife Camera (AI-Thinker)
```cpp
#define CAMERA_ENABLED true      ✓
#define SD_CARD_ENABLED true     ✓
#define PIR_SENSOR_ENABLED true  ✓
#define BME280_SENSOR_ENABLED true ✓
// LORA_ENABLED undefined      ✗
// LED_INDICATORS_ENABLED false ✗
```

### Config 2: Full System (ESP32-S3)
```cpp
#define CAMERA_ENABLED true      ✓
#define LORA_ENABLED true        ✓
#define SD_CARD_ENABLED true     ✓
#define LED_INDICATORS_ENABLED true ✓
// Everything works!
```

### Config 3: LoRa Node (AI-Thinker)
```cpp
#define CAMERA_ENABLED false     ✗
#define LORA_ENABLED true        ✓
#define SD_CARD_ENABLED true     ✓
// Many GPIO freed up
```

## ❌ Common Errors

### Error: "LoRa and Camera cannot be enabled"
**Fix**: Choose one OR upgrade to ESP32-S3

### Error: "LED indicators and SD card cannot be enabled"
**Fix**: Disable LEDs (set `LED_INDICATORS_ENABLED false`)

### Warning: "Solar voltage monitoring conflicts"
**Fix**: Use GPIO 33 instead of GPIO 32

## 📖 Full Documentation

- **Complete Guide**: `docs/GPIO_PIN_CONFLICTS.md`
- **Hardware Selection**: `docs/HARDWARE_SELECTION_GUIDE.md`
- **Config Templates**: `firmware/examples/configs/`

## 🛠️ Quick Setup

```bash
# Choose a template
cd firmware
cp examples/configs/config_ai_thinker_camera_only.h config.h

# Edit WiFi settings
nano config.h

# Compile
pio run -e esp32cam_advanced

# If compile error:
# Read the error message - it tells you exactly what to fix!
```

## 🧪 Test Your Config

```bash
cd firmware
./test_gpio_conflicts.sh
```

## 💡 Pro Tips

1. **Start with a template** - Don't build config from scratch
2. **Read compile errors** - They tell you exactly what's wrong
3. **Use I2C sensors** - They share pins (GPIO 26/27) with camera
4. **Disable debug in production** - Frees GPIO 1 for PIR
5. **Consider ESP32-S3** - Worth the extra $10 if you need features

## 🎯 Decision Tree

```
┌─ Camera needed?
│   ├─ Yes
│   │   ├─ LoRa needed?
│   │   │   ├─ Yes → ESP32-S3-CAM ($20) ✓
│   │   │   └─ No  → AI-Thinker ($10) ✓
│   │   └─ Budget?
│   │       ├─ Tight → AI-Thinker ($10) ✓
│   │       └─ Ok → ESP32-S3-CAM ($20) ✓ (future-proof)
│   └─ No
│       └─ LoRa needed?
│           ├─ Yes → ESP32-DevKit ($8) or AI-Thinker ✓
│           └─ No  → ESP32-DevKit ($8) ✓
```

## 📊 Cost Comparison

| Option | Hardware | Cost | Camera | LoRa | Notes |
|--------|----------|------|--------|------|-------|
| Basic | AI-Thinker | $10 | ✓ | ✗ | Best value |
| Full | ESP32-S3 | $20 | ✓ | ✓ | No limits |
| Mesh | Mixed | $12 | ✓ | ✓ | 8× camera + 2× relay |
| Node | ESP32-DevKit | $8 | ✗ | ✓ | Relay only |

## 🔧 Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Compile error | Feature conflict | Read error, follow solutions |
| Camera corrupted | LoRa enabled | Disable LoRa |
| SD mount fails | LEDs enabled | Disable LEDs |
| No free GPIO | Too many features | Upgrade to ESP32-S3 |
| Boot fails | Wrong pin state | Check GPIO 0,2,12,15 |

## 📞 Support

- **Issue**: Check `docs/GPIO_PIN_CONFLICTS.md` section for your error
- **Hardware**: Check `docs/HARDWARE_SELECTION_GUIDE.md`
- **Config**: Check `firmware/examples/configs/README.md`
- **GitHub**: Open issue with error message

## 🎓 Remember

> "The AI-Thinker ESP32-CAM is a great camera board but a poor general-purpose ESP32.
> If you need both camera AND many peripherals, the ESP32-S3-CAM is worth every penny."

---

**Print this page and keep it at your desk!** 📄

**Version**: 1.0 | **Date**: 2025-10-16
