# Hardware Troubleshooting Guide

## Common Assembly and Hardware Issues

### Power Issues

#### No Power / Device Won't Start
**Symptoms**: LED indicators off, no serial output, device unresponsive

**Causes & Solutions**:
- **Battery Connection**: Check polarity, clean contacts, verify voltage (>3.2V)
- **Solar Panel**: Verify connections, check panel output in sunlight (>5V)
- **Power Management**: Test charging circuit, check fuse/protection components
- **ESP32-CAM Board**: Verify 5V and 3.3V rail voltages

#### Intermittent Power Loss
**Symptoms**: Random resets, brownout detections, unstable operation

**Solutions**:
- Increase capacitance on power rail
- Check for loose connections
- Verify battery health and capacity
- Improve solar panel positioning

### Camera Issues

#### Camera Not Detected
**Symptoms**: "Camera init failed" error, no image capture

**Solutions**:
1. **Connection Check**: Verify camera module is properly seated
2. **Pin Verification**: Confirm GPIO assignments match board type
3. **Power Supply**: Ensure 3.3V rail is stable (>3.2V under load)
4. **PSRAM Check**: Verify PSRAM is enabled for larger frame buffers

#### Poor Image Quality
**Symptoms**: Blurry images, wrong colors, artifacts

**Solutions**:
- Clean camera lens and remove protective film
- Adjust focus ring on camera module
- Check for electromagnetic interference
- Verify proper camera settings in firmware

#### Camera Crashes/Hangs
**Symptoms**: Watchdog resets during capture, frozen camera operations

**Solutions**:
- Reduce frame size or quality settings
- Increase camera initialization delay
- Check for memory leaks in capture loop
- Verify PSRAM allocation

### Motion Detection Issues

#### PIR Sensor Not Working
**Symptoms**: No motion triggers, constant triggers

**Solutions**:
- **Power Check**: Verify 3.3V supply to PIR sensor
- **Sensitivity Adjustment**: Turn potentiometer on PIR module
- **Positioning**: Avoid heat sources, point away from sun
- **GPIO Configuration**: Verify interrupt setup and pin assignment

#### False Motion Detections
**Symptoms**: Triggers without animal presence

**Solutions**:
- Adjust PIR sensitivity and timeout settings
- Shield sensor from wind-moved vegetation
- Check for thermal interference (sunlight, electronics)
- Implement software debouncing

### Storage Issues

#### SD Card Not Detected
**Symptoms**: "SD init failed" error, no file operations

**Solutions**:
1. **Card Compatibility**: Use Class 10, 32GB max, FAT32 format
2. **Connection Check**: Verify SD card slot contacts
3. **Power Supply**: Ensure sufficient current for SD operations
4. **Formatting**: Reformat card with proper cluster size

#### Storage Full / Write Errors
**Symptoms**: Cannot save images, corrupted files

**Solutions**:
- Implement automatic file cleanup
- Check for write protection
- Verify card health with diagnostic tools
- Reduce image quality or implement compression

### Environmental Issues

#### Moisture/Condensation
**Symptoms**: Fogged camera lens, water damage, corrosion

**Solutions**:
- **Gasket Replacement**: Install fresh O-rings, apply silicone sealant
- **Ventilation**: Ensure proper airflow through grilles
- **Desiccant**: Add moisture-absorbing packets
- **Drainage**: Verify drain holes are clear

#### Temperature Problems
**Symptoms**: Device shutdown in extreme temperatures

**Solutions**:
- **Cold Weather**: Use Li-ion batteries rated for low temperature
- **Hot Weather**: Improve ventilation, add heat sinks
- **Thermal Management**: Implement temperature monitoring and shutdown

### Connectivity Issues

#### WiFi Connection Failures
**Symptoms**: Cannot connect to network, frequent disconnections

**Solutions**:
- Check signal strength and antenna positioning
- Verify credentials and network compatibility
- Implement connection retry logic
- Consider external antenna for better range

#### LoRa Communication Problems
**Symptoms**: No data transmission, range issues

**Solutions**:
- Verify antenna connection and SWR
- Check frequency and spreading factor settings
- Test in open area without obstacles
- Verify power output settings

### Mechanical Issues

#### Enclosure Damage
**Symptoms**: Cracks, UV degradation, mounting failures

**Solutions**:
- **Material Selection**: Use UV-resistant filaments (PETG, ASA)
- **Design Improvements**: Add stress relief, increase wall thickness
- **Mounting**: Use vibration dampening, check fastener torque

#### Mounting Problems
**Symptoms**: Camera movement, strap failure, bracket loosening

**Solutions**:
- Use stainless steel hardware
- Implement anti-rotation features
- Regular maintenance schedule
- Consider alternative mounting methods

## Diagnostic Tools

### Hardware Testing
- Multimeter for voltage/continuity testing
- Oscilloscope for signal integrity
- Thermal camera for hot spot detection
- Current meter for power consumption analysis

### Software Debugging
- Serial monitor for real-time diagnostics
- SD card logging for field debugging
- WiFi-based remote diagnostics
- Built-in test modes and self-diagnostics

## Preventive Maintenance

### Monthly Checks
- Battery voltage measurement
- Visual inspection for damage
- Clean camera lens and solar panel
- Check mounting security

### Seasonal Maintenance
- Replace moisture-absorbing packets
- Update firmware if available
- Deep clean all components
- Test all functionality

### Annual Overhaul
- Replace O-ring gaskets
- Check all electrical connections
- Verify weatherproofing integrity
- Performance benchmarking

## Emergency Procedures

### Field Recovery
1. **Power Cycle**: Disconnect battery for 30 seconds
2. **Safe Mode**: Boot with minimal functionality
3. **Factory Reset**: Restore default settings
4. **Emergency Contact**: Document issue and contact support

### Data Recovery
- Remove SD card and backup data
- Use card recovery tools if corrupted
- Check backup/sync status
- Document any data loss

---
*For issues not covered here, consult the technical documentation or submit a detailed issue report.*