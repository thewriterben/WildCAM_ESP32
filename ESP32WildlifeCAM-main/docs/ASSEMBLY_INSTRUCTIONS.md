# 🔧 ESP32 Wildlife CAM - Assembly Instructions

## Overview
This comprehensive guide provides step-by-step assembly instructions for the ESP32 Wildlife CAM system, including safety procedures, best practices, and troubleshooting guidance.

## ⚠️ Safety First - Required Precautions

### Electrical Safety
- [ ] **Anti-static wrist strap** - Prevent component damage
- [ ] **Proper ventilation** - Soldering fumes management
- [ ] **Fire extinguisher nearby** - Soldering safety
- [ ] **First aid kit accessible** - Burns and cuts
- [ ] **Proper lighting** - Reduce eye strain
- [ ] **Clean workspace** - Prevent component loss

### Component Handling
- [ ] **Handle ESP32 by edges only** - Avoid pin damage
- [ ] **Store components in anti-static bags** - Prevent ESD
- [ ] **Check voltage levels before connection** - Prevent damage
- [ ] **Use proper gauge wire** - Current capacity
- [ ] **Double-check polarity** - Battery and sensor connections

## 🛠️ Assembly Phases

### Phase 1: Preparation & Planning (30 minutes)

#### 1.1 Workspace Setup
1. **Clear work surface** - Minimum 2ft x 2ft area
2. **Organize tools** - Layout as per tool checklist
3. **Sort components** - Group by assembly phase
4. **Prepare documentation** - Print pin diagrams
5. **Test tools** - Verify multimeter and soldering iron

#### 1.2 Component Verification
1. **Inventory check** - Verify all parts from hardware requirements
2. **Visual inspection** - Check for damage or defects
3. **Basic functionality test** - Power ESP32 with USB
4. **Camera module test** - Verify image capture capability

```bash
# Quick component test script
./scripts/hardware_test.sh --component all --simulation
```

### Phase 2: Core Module Assembly (45 minutes)

#### 2.1 ESP32-CAM Preparation
1. **Inspect ESP32-CAM module**
   - Check for bent pins
   - Verify camera connector is secure
   - Test basic power-on with USB programmer

2. **Install MicroSD card**
   - Format card as FAT32
   - Test write/read operations
   - Label card with project info

#### 2.2 Pin Header Installation
```
ESP32-CAM Pinout Reference:
┌─────────────────┐
│  5V  GND  IO12  │ ← Power & SD
│  IO13 IO15 IO14 │ ← Reserved pins  
│  IO2  IO4  IO16 │ ← User pins
│  IO17 IO5  IO18 │ ← Camera interface
│  IO19 IO21     │
│  IO22 TX  RX   │ ← UART
│  IO23 IO25     │
│  IO26 IO27     │ ← I2C capable
│  IO32 IO33     │ ← Analog capable
│  IO34 IO35     │ ← Input only
│  IO36 IO39     │ ← Input only
└─────────────────┘
```

3. **Solder pin headers**
   - Use flux for clean joints
   - Heat joint, apply solder
   - Remove iron, let cool
   - Inspect for cold joints

### Phase 3: Sensor Integration (60 minutes)

#### 3.1 PIR Motion Sensor (GPIO 1)
1. **Prepare PIR sensor HC-SR501**
   - Adjust sensitivity potentiometer to middle position
   - Set time delay to minimum (3 seconds)
   - Note pin configuration: VCC, OUT, GND

2. **Wire connections**
   ```
   PIR Sensor → ESP32-CAM
   VCC (Red)  → 5V
   GND (Black)→ GND  
   OUT (Yellow)→ GPIO 1 (PIR_PIN)
   ```

3. **Test motion detection**
   ```cpp
   // Quick test code
   #define PIR_PIN 1
   void setup() {
     pinMode(PIR_PIN, INPUT);
     Serial.begin(115200);
   }
   void loop() {
     if(digitalRead(PIR_PIN)) {
       Serial.println("Motion detected!");
     }
     delay(100);
   }
   ```

#### 3.2 Environmental Sensors (I2C Bus)
1. **BME280 Temperature/Humidity Sensor**
   ```
   BME280 → ESP32-CAM
   VCC → 3.3V
   GND → GND
   SDA → GPIO 26 (SIOD_GPIO_NUM)
   SCL → GPIO 27 (SIOC_GPIO_NUM)
   ```

2. **Light Sensor (Optional)**
   ```
   BH1750 → ESP32-CAM  
   VCC → 3.3V
   GND → GND
   SDA → GPIO 26 (shared I2C)
   SCL → GPIO 27 (shared I2C)
   ```

3. **I2C Bus Testing**
   ```bash
   # Test I2C device detection
   python3 scripts/validate_component.py sensors --simulation
   ```

### Phase 4: Power System Assembly (45 minutes)

#### 4.1 Battery Management System
1. **TP4056 Charging Module Setup**
   - **B+ (Red)** → LiPo Battery Positive
   - **B- (Black)** → LiPo Battery Negative  
   - **OUT+ (Red)** → ESP32 5V Input
   - **OUT- (Black)** → ESP32 GND

2. **Solar Panel Connection**
   - **Solar + (Red)** → TP4056 IN+
   - **Solar - (Black)** → TP4056 IN-
   - Add blocking diode if not integrated

#### 4.2 Power Distribution
1. **Create power bus**
   ```
   Power Bus Layout:
   TP4056 OUT+ ────┬── ESP32 5V
                   ├── PIR VCC
                   └── Sensor VCC
   
   TP4056 OUT- ────┬── ESP32 GND
                   ├── PIR GND  
                   └── Sensor GND
   ```

2. **Add power monitoring**
   ```
   Battery Voltage Monitor:
   Battery + ──[10kΩ]──┬── GPIO 33 (BATTERY_VOLTAGE_PIN)
                       │
                      [10kΩ]
                       │
                      GND
   ```

### Phase 5: Storage System (30 minutes)

#### 5.1 SD Card Interface
1. **Verify existing SD card slot on ESP32-CAM**
   - Most AI-Thinker boards have built-in slot
   - Test with formatted 32GB card

2. **Alternative external SD module (if needed)**
   ```
   SD Module → ESP32-CAM
   VCC → 3.3V
   GND → GND
   MISO → GPIO 2
   MOSI → GPIO 15  
   SCK → GPIO 14
   CS → GPIO 12
   ```

### Phase 6: Enclosure Assembly (60 minutes)

#### 6.1 Enclosure Preparation
1. **Plan component layout**
   - ESP32-CAM positioned for camera view
   - Solar panel external mounting
   - Access to programming port
   - Ventilation considerations

2. **Drill mounting holes**
   - Use step bits for clean holes
   - Deburr all holes
   - Test fit before final assembly

#### 6.2 Weatherproofing
1. **Cable glands**
   - Install appropriate size glands
   - Apply thread sealant
   - Route cables carefully

2. **Seal joints**
   - Use silicone sealant on all joints
   - Ensure camera lens area is clear
   - Test enclosure with water before electronics

### Phase 7: System Integration & Testing (45 minutes)

#### 7.1 Initial Power-On Test
1. **Pre-power checklist**
   - [ ] Verify all connections
   - [ ] Check polarity on all components
   - [ ] Ensure no short circuits
   - [ ] Battery charged to >3.7V

2. **First power-on**
   ```bash
   # Monitor serial output during boot
   # Look for successful component initialization
   ```

#### 7.2 Component Function Tests
1. **Camera test**
   ```bash
   ./scripts/hardware_test.sh --component camera
   ```

2. **Sensor validation**
   ```bash
   ./scripts/hardware_test.sh --component sensors
   ```

3. **Power system test**
   ```bash
   ./scripts/hardware_test.sh --component power
   ```

## 🔍 Assembly Verification Checklist

### Electrical Verification
- [ ] **Continuity test** - All connections secure
- [ ] **Voltage test** - Correct voltages at all points
- [ ] **Isolation test** - No short circuits
- [ ] **Load test** - System stable under load

### Functional Verification  
- [ ] **Camera captures images** - Clear, focused images
- [ ] **Motion detection works** - Triggers on movement
- [ ] **Sensors respond** - Environmental data updating
- [ ] **Storage works** - Images save to SD card
- [ ] **Power system functions** - Charging and monitoring

### Environmental Verification
- [ ] **Enclosure sealed** - No water ingress
- [ ] **Temperature range** - Functions -10°C to +50°C
- [ ] **Vibration test** - Secure under movement
- [ ] **Solar charging** - Power generation verified

## 🚨 Troubleshooting Common Assembly Issues

### Power Problems
| Issue | Symptoms | Solution |
|-------|----------|----------|
| **No power** | Dead system | Check battery voltage, connections |
| **Brown-out resets** | Intermittent operation | Verify current capacity, clean connections |
| **Charging failure** | Battery not charging | Test solar panel output, TP4056 module |

### Camera Issues
| Issue | Symptoms | Solution |
|-------|----------|----------|
| **No image** | Black images | Check camera connector, GPIO assignments |
| **Blurry images** | Poor focus | Adjust camera lens, clean lens |
| **Corrupted files** | Partial images | Test SD card, check file system |

### Sensor Issues
| Issue | Symptoms | Solution |
|-------|----------|----------|
| **PIR not triggering** | No motion detection | Adjust sensitivity, check wiring |
| **I2C sensors offline** | No environmental data | Scan I2C bus, check pull-up resistors |
| **False readings** | Erratic sensor data | Check power stability, sensor placement |

## ⚡ Advanced Assembly Options

### High-Performance Configuration
- **ESP32-S3-CAM upgrade** - Better performance
- **OV5640 camera** - Higher resolution
- **Dual battery system** - Extended runtime
- **Professional enclosure** - Better weatherproofing

### Multi-Node Deployment
- **LoRa communication** - Long-range networking
- **Mesh networking** - Redundant communication
- **Synchronized triggers** - Coordinated capture
- **Central data aggregation** - Unified monitoring

## 📋 Post-Assembly Tasks

### 1. Firmware Installation
```bash
# Upload production firmware
./scripts/prepare_production.sh --upload
```

### 2. Field Testing Preparation
```bash
# Run comprehensive validation
./scripts/integration_test.sh --full
```

### 3. Documentation
- [ ] **Label all components** - For future maintenance
- [ ] **Record serial numbers** - For tracking
- [ ] **Create assembly photos** - For reference
- [ ] **Update configuration files** - Reflect actual hardware

## 📖 Next Steps

After successful assembly:
1. **[FIELD_TEST_PLAN.md](FIELD_TEST_PLAN.md)** - Prepare for field deployment
2. **[TEST_SCENARIOS.md](TEST_SCENARIOS.md)** - Execute validation tests
3. **[DEPLOYMENT_CHECKLIST.md](DEPLOYMENT_CHECKLIST.md)** - Final deployment preparation

---

**Assembly Time**: 4-6 hours (first-time assembly)  
**Skill Level**: Intermediate electronics  
**Tools Required**: Soldering iron, multimeter, drill  
**Safety Rating**: Medium - Electrical and mechanical hazards

**Document Version**: 1.0  
**Last Updated**: 2025-09-01  
**Compatibility**: ESP32 Wildlife CAM v2.0+