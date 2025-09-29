# HMI System Architecture

## Overview

The Human Machine Interface (HMI) system provides a comprehensive display and user interface solution for ESP32-based wildlife cameras. It supports multiple display types and provides an intuitive menu system for configuration and monitoring.

## Architecture

```
HMI System
├── Display Interface Layer
│   ├── DisplayInterface (Abstract Base)
│   ├── SSD1306Display (OLED Driver)
│   └── ST7789Display (TFT Driver)
├── Display Manager
│   ├── Auto-detection
│   ├── Driver Management
│   └── Configuration
├── HMI Framework
│   ├── Menu System
│   ├── Status Display
│   ├── Input Handling
│   └── Power Management
└── Board Integration
    ├── Board Profiles
    ├── GPIO Configuration
    └── Hardware Detection
```

## Components

### Display Interface Layer

**Purpose**: Provides hardware abstraction for different display types

**Key Classes**:
- `DisplayInterface`: Abstract base class defining common display operations
- `SSD1306Display`: Driver for I2C OLED displays (128x64, monochrome)
- `ST7789Display`: Driver for SPI TFT displays (240x240, color)

**Features**:
- Unified API across display types
- Hardware-specific optimizations
- Power management integration
- Brightness control

### Display Manager

**Purpose**: Manages display detection, initialization, and lifecycle

**Key Features**:
- Automatic display type detection based on board
- Dynamic driver instantiation
- Configuration management
- Fallback handling for unsupported displays

**Detection Logic**:
1. Board-specific display mapping
2. I2C device scanning for OLED displays
3. SPI communication testing for TFT displays
4. Fallback to generic configurations

### HMI Framework

**Purpose**: Provides user interface components and interaction logic

**Components**:
- **Menu System**: Hierarchical navigation with customizable items
- **Status Display**: Real-time system information
- **Input Handling**: Button/touch input processing
- **Auto-off Timer**: Power-saving display management

**Menu Structure**:
```
Main Menu
├── Camera Settings
│   ├── Resolution
│   ├── Quality
│   ├── Flash
│   └── Timer
├── System Info
│   ├── Board Info
│   ├── Memory
│   └── Version
├── Power Status
│   ├── Battery Level
│   ├── Charging Status
│   └── Solar Voltage
├── File Browser
│   ├── Image List
│   ├── Video List
│   └── Log Files
└── Network Settings
    ├── WiFi Config
    ├── LoRa Settings
    └── Remote Access
```

### Board Integration

**Purpose**: Connects HMI system with board-specific hardware

**Integration Points**:
- GPIO pin mapping for displays
- Power management coordination
- Hardware capability detection
- Board-specific optimizations

## Supported Displays

### SSD1306 OLED (128x64)

**Specifications**:
- Resolution: 128x64 pixels
- Colors: Monochrome (white/blue)
- Interface: I2C
- Power: ~10mA active, <1mA standby
- Brightness: Software contrast control

**Typical Boards**: TTGO T-Camera, generic ESP32-CAM with OLED add-on

**Advantages**:
- Low power consumption
- High contrast
- Wide viewing angle
- Fast response time

**Limitations**:
- Limited resolution
- Monochrome only
- No backlight control

### ST7789 TFT (240x240)

**Specifications**:
- Resolution: 240x240 pixels
- Colors: 65K colors (16-bit RGB)
- Interface: SPI
- Power: ~30mA active, ~5mA standby
- Brightness: PWM backlight control

**Typical Boards**: LilyGO T-Camera Plus

**Advantages**:
- Full color display
- High resolution
- Smooth graphics
- Backlight control

**Limitations**:
- Higher power consumption
- More complex initialization
- Requires more GPIO pins

## Configuration

### Board Profiles

Each supported board has a display profile defining:

```cpp
struct DisplayProfile {
    bool has_display;              // Board has built-in display
    uint8_t display_type;          // Display type (SSD1306, ST7789, etc.)
    int display_sda_pin;           // I2C SDA pin for OLED
    int display_scl_pin;           // I2C SCL pin for OLED
    int display_cs_pin;            // SPI CS pin for TFT
    int display_dc_pin;            // SPI DC pin for TFT
    int display_rst_pin;           // Reset pin
    int display_bl_pin;            // Backlight pin
    uint8_t display_i2c_addr;      // I2C address for OLED
    uint32_t display_i2c_freq;     // I2C frequency
    uint32_t display_spi_freq;     // SPI frequency
    uint16_t display_width;        // Display width in pixels
    uint16_t display_height;       // Display height in pixels
};
```

### Runtime Configuration

HMI behavior can be configured via config.h:

```cpp
// HMI System Configuration
#define HMI_ENABLED true                 // Enable HMI system
#define HMI_AUTO_BRIGHTNESS true         // Enable automatic brightness
#define HMI_DEFAULT_BRIGHTNESS 200       // Default brightness (0-255)
#define HMI_AUTO_OFF_TIMEOUT 30000       // Auto-off timeout (ms)
#define HMI_UPDATE_INTERVAL 1000         // Update interval (ms)

// Display Configuration
#define DISPLAY_AUTO_DETECT true         // Auto-detect display type
#define DISPLAY_I2C_SPEED 400000         // I2C speed for OLED
#define DISPLAY_SPI_SPEED 27000000       // SPI speed for TFT
```

## Power Management

### Display Power States

1. **Active**: Display on, normal brightness, frequent updates
2. **Dimmed**: Reduced brightness, normal updates
3. **Standby**: Display off, minimal power, wake on input
4. **Deep Sleep**: Complete shutdown, wake on system event

### Auto-off Timer

- Configurable timeout (default: 30 seconds)
- Reset on user input
- Graceful transition to standby mode
- Instant wake on interaction

### Brightness Control

**OLED (SSD1306)**:
- Software contrast adjustment
- No true brightness control
- Power consumption varies minimally

**TFT (ST7789)**:
- PWM backlight control
- Linear brightness scaling
- Significant power impact

## Performance Characteristics

### Memory Usage

| Component | RAM Usage | Flash Usage |
|-----------|-----------|-------------|
| Display Interface | 1KB | 8KB |
| SSD1306 Driver | 1KB | 12KB |
| ST7789 Driver | 2KB | 15KB |
| HMI Framework | 4KB | 20KB |
| **Total** | **~8KB** | **~55KB** |

### Update Performance

| Display Type | Frame Rate | Update Time |
|--------------|------------|-------------|
| SSD1306 OLED | 30 FPS | ~33ms |
| ST7789 TFT | 60 FPS | ~16ms |

### Power Consumption

| State | OLED Power | TFT Power |
|-------|------------|-----------|
| Active | 10mA | 30mA |
| Dimmed | 8mA | 15mA |
| Standby | <1mA | 5mA |
| Off | 0mA | 0mA |

## Integration Examples

### Basic Integration

```cpp
#include "display/hmi_system.h"

HMISystem hmi;
std::unique_ptr<CameraBoard> board;

void setup() {
    // Initialize board detection
    BoardDetector detector;
    board = detector.createBoard(detector.detectBoard());
    
    // Initialize HMI
    if (!hmi.init(board.get())) {
        Serial.println("No display available");
        return;
    }
    
    // Show initial status
    hmi.showStatus();
}

void loop() {
    // Update HMI system
    hmi.updateDisplay();
    hmi.processInput();
    
    // Handle button input
    if (digitalRead(BUTTON_PIN) == LOW) {
        hmi.handleButtonPress(0);  // Button 0 pressed
    }
}
```

### Custom Menu Integration

```cpp
// Custom menu handler
bool handleCustomAction(void* context) {
    HMISystem* hmi = static_cast<HMISystem*>(context);
    
    // Perform custom action
    Serial.println("Custom action executed");
    
    // Update display
    hmi->showStatus();
    return true;
}

// Add custom menu item
MenuItem customItem = {
    "Custom Action",
    MENU_MAIN,
    handleCustomAction,
    true
};
```

### Status Integration

```cpp
void updateSystemStatus() {
    // Get current system status
    SystemStatus status = hmi.getSystemStatus();
    
    // Log to serial
    Serial.printf("Battery: %d%% (%.2fV)\n", 
                  status.battery_percentage, 
                  status.battery_voltage);
    
    // Update display if needed
    if (status.battery_percentage < 20) {
        hmi.showError("Low Battery!");
    }
}
```

## Extension Points

### Adding New Display Types

1. **Create Driver Class**:
   ```cpp
   class CustomDisplay : public DisplayInterface {
       // Implement required methods
   };
   ```

2. **Update Display Manager**:
   ```cpp
   case DISPLAY_CUSTOM:
       return std::make_unique<CustomDisplay>(config);
   ```

3. **Add Board Profile**:
   ```cpp
   const DisplayProfile CUSTOM_BOARD_DISPLAY_PROFILE = {
       .has_display = true,
       .display_type = DISPLAY_CUSTOM,
       // ... other configuration
   };
   ```

### Custom UI Elements

1. **Define UI Component**:
   ```cpp
   class ProgressBar {
       void draw(DisplayInterface* display, int x, int y, int value);
   };
   ```

2. **Integrate with HMI**:
   ```cpp
   void HMISystem::showProgress(int percentage) {
       ProgressBar progress;
       progress.draw(display_manager->getDisplay(), 0, 50, percentage);
   }
   ```

### Input Extensions

1. **Touch Input**:
   ```cpp
   void HMISystem::handleTouchInput(int x, int y) {
       // Process touch coordinates
       // Map to menu actions
   }
   ```

2. **Rotary Encoder**:
   ```cpp
   void HMISystem::handleEncoderInput(int direction) {
       MenuAction action = (direction > 0) ? ACTION_DOWN : ACTION_UP;
       navigateMenu(action);
   }
   ```

## Troubleshooting

### Common Issues

1. **Display Not Detected**
   - Check board profile configuration
   - Verify GPIO connections
   - Test I2C/SPI communication

2. **Garbled Display**
   - Verify clock speeds
   - Check power supply stability
   - Confirm pin assignments

3. **Poor Performance**
   - Reduce update frequency
   - Optimize drawing operations
   - Enable hardware acceleration

4. **High Power Consumption**
   - Enable auto-off timer
   - Reduce brightness
   - Use low-power display modes

### Debug Tools

1. **Serial Logging**:
   ```cpp
   #define DEBUG_HMI_ENABLED 1
   DEBUG_HMI_INFO("Display initialized: %s", display->getDisplayName());
   ```

2. **Display Test Patterns**:
   ```cpp
   void testDisplay() {
       display->clear();
       display->drawRect(0, 0, width, height, 0xFFFF);
       display->display();
   }
   ```

3. **Performance Monitoring**:
   ```cpp
   unsigned long start = millis();
   hmi.updateDisplay();
   unsigned long duration = millis() - start;
   Serial.printf("Update time: %lu ms\n", duration);
   ```