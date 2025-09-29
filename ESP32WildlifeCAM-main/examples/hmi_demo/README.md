# HMI Demo for ESP32 Wildlife Camera

This example demonstrates the Human Machine Interface (HMI) system for LilyGO T-Camera and TTGO T-Camera boards with built-in displays.

## Supported Boards

### LilyGO T-Camera Plus
- **Display**: 1.14" TFT LCD (ST7789)
- **Resolution**: 240x240 pixels  
- **Interface**: SPI
- **Features**: Full color display, backlight control

### TTGO T-Camera
- **Display**: 0.96" OLED (SSD1306)
- **Resolution**: 128x64 pixels
- **Interface**: I2C
- **Features**: Monochrome display, low power

## Features Demonstrated

- **Automatic Display Detection**: Detects board type and initializes appropriate display driver
- **Status Display**: Shows battery level, WiFi status, system information
- **Menu System**: Navigate through camera settings, system info, power status
- **Power Management**: Display auto-off, brightness control
- **Real-time Monitoring**: Live battery voltage, charging status, memory usage

## Hardware Requirements

1. **LilyGO T-Camera Plus** or **TTGO T-Camera** board
2. USB cable for programming and power
3. Optional: Solar panel and battery for power monitoring demonstration

## Pin Configurations

### LilyGO T-Camera Plus (ST7789 TFT)
- CS: GPIO 12
- DC: GPIO 13  
- RST: GPIO 14
- BL: GPIO 15 (Backlight)
- SPI: Hardware SPI

### TTGO T-Camera (SSD1306 OLED)  
- SDA: GPIO 21
- SCL: GPIO 22
- RST: GPIO 16
- I2C Address: 0x3C

## Building and Running

1. **Install PlatformIO**:
   ```bash
   pip install platformio
   ```

2. **Navigate to demo directory**:
   ```bash
   cd examples/hmi_demo/
   ```

3. **Build the project**:
   ```bash
   pio run
   ```

4. **Upload to board**:
   ```bash
   pio run --target upload
   ```

5. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

## Serial Commands

Once running, you can control the HMI system via serial commands:

- `u` - Navigate Up in menus
- `d` - Navigate Down in menus  
- `s` - Select/Enter menu item
- `b` - Back/Return to previous menu
- `i` - Show System Information
- `p` - Show Power Status
- `h` - Show Status Display
- `+` - Increase Display Brightness
- `-` - Decrease Display Brightness

## Display States

### Status Display
Shows current system status including:
- Camera readiness
- Battery percentage and voltage
- Free memory
- System uptime
- WiFi connection status

### Main Menu
Navigate through:
- Camera Settings
- System Info  
- Power Status
- File Browser
- Network Settings

### Power Status
Detailed power information:
- Battery voltage and percentage
- Charging status
- Solar panel voltage (if connected)
- Power state (Full/Good/Low/Critical)

### System Info
Hardware and software information:
- Board type and model
- Chip information
- PSRAM availability
- Memory usage

## Troubleshooting

### No Display Detected
- Verify board type is supported
- Check display connections
- Ensure proper power supply
- Try different I2C/SPI speeds in configuration

### Compilation Errors
- Ensure all dependencies are installed
- Check PlatformIO platform version
- Verify library versions in platformio.ini

### Display Issues
- Check pin configurations in board profiles
- Verify display initialization
- Test with different brightness levels
- Check power supply voltage

## Customization

### Adding New Display Types
1. Create new display driver class inheriting from `DisplayInterface`
2. Add display type to `DisplayType` enum
3. Update `DisplayManager` detection logic
4. Add board-specific configuration

### Custom Menu Items
1. Define new menu items in `HMISystem::menu_items`
2. Implement action handlers
3. Add navigation logic

### Display Themes
1. Modify color schemes in display drivers
2. Customize icons and graphics
3. Adjust text sizes and fonts

## Integration with Main Application

The HMI system is designed to integrate seamlessly with the main wildlife camera application:

```cpp
#include "display/hmi_system.h"

HMISystem hmi;
std::unique_ptr<CameraBoard> board;

void setup() {
    // Detect board
    BoardDetector detector;
    board = detector.createBoard(detector.detectBoard());
    
    // Initialize HMI
    hmi.init(board.get());
}

void loop() {
    // Update HMI
    hmi.updateDisplay();
    hmi.processInput();
}
```

## Performance Notes

- **Update Frequency**: Display updates every 1 second by default
- **Auto-off Timeout**: Display turns off after 30 seconds of inactivity
- **Memory Usage**: ~8KB RAM for display buffers
- **Power Consumption**: 
  - OLED: ~10mA active, <1mA standby
  - TFT: ~30mA active, ~5mA standby

## Future Enhancements

- Touch input support for TFT displays
- Image preview for captured photos
- Real-time camera preview
- Remote control via LoRa/WiFi
- Configuration management
- Data logging and export