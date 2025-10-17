# Power Save Mode - Quick Reference

## Overview
Automatic power saving when battery voltage drops below 3.0V.

## State Transitions

```
Normal Mode (Battery > 3.4V)
├── CPU: 240 MHz
├── Sleep: 300s (5 min)
├── WiFi: Enabled
└── Camera: Normal quality

    ↓ Battery < 3.0V

Power Save Mode (Battery < 3.0V)
├── CPU: 80 MHz (-67%)
├── Sleep: 600s (10 min)
├── WiFi: Disabled
└── Camera: Reduced quality*

    ↑ Battery > 3.4V

Normal Mode Restored
```

## Key Variables

```cpp
// Global
uint32_t deep_sleep_duration = 300; // Adjusted 300s ↔ 600s

// SystemState
system_state.power_save_mode = false;  // Tracks mode
system_state.battery_level = 3.7f;     // Current voltage
```

## Thresholds

| Threshold | Voltage | Action |
|-----------|---------|--------|
| Enter | < 3.0V | Activate power save |
| Stay | 3.0V - 3.4V | Maintain current mode |
| Exit | > 3.4V | Deactivate power save |

## Power Savings

| Component | Reduction | Savings |
|-----------|-----------|---------|
| CPU | 67% | High |
| Active Time | 50% | High |
| WiFi | 100% | Very High |
| **Total** | **>40%** | **Very High** |

## Code Location
**File**: `firmware/main.cpp`  
**Function**: `powerManagementTask()`  
**Lines**: ~368-413

## Testing

```cpp
// Simulate low battery
power_status.battery_voltage = 2.8f;  // Triggers power save

// Simulate recovery
power_status.battery_voltage = 3.5f;  // Exits power save
```

## Monitoring

```cpp
// Check current mode
if (system_state.power_save_mode) {
    Serial.println("Power save mode active");
}

// Check sleep duration
Serial.printf("Sleep: %ds\n", deep_sleep_duration);

// Check battery
Serial.printf("Battery: %.2fV\n", system_state.battery_level);
```

## Log Messages

```
⚠️  Entering power save mode (battery: 2.95V)
✓ CPU frequency reduced to 80MHz
✓ Deep sleep duration increased to 600 seconds
✓ WiFi disabled for power saving
✓ Power save mode activated

✓ Exiting power save mode (battery: 3.45V)
✓ CPU frequency restored to 240MHz
✓ Deep sleep duration restored to 300 seconds
✓ Power save mode deactivated - normal operation resumed
```

## Configuration

```cpp
// config.h
#define WIFI_ENABLED true  // Set to false to disable WiFi globally

// main.cpp
uint32_t deep_sleep_duration = 300;  // Adjust default sleep time
```

## Future Enhancements

- [ ] Camera quality/framerate reduction
- [ ] LED brightness control
- [ ] Additional sensor disable
- [ ] NVS persistence across reboots
- [ ] Power consumption telemetry

## Safety Features

1. **Hysteresis**: 0.4V gap prevents oscillation
2. **State Guard**: Prevents duplicate actions
3. **Conditional Compilation**: WiFi code only when enabled
4. **Connection Check**: Verifies WiFi state before operations
5. **Comprehensive Logging**: All transitions logged

## Performance

- **Entry Time**: < 1ms
- **Exit Time**: < 1ms
- **Memory Overhead**: Minimal (1 bool + 1 uint32)
- **CPU Overhead**: None (only during battery check every 5s)

---

**Quick Start**: Just deploy! Power save mode activates automatically when battery is low.

**Documentation**: See `POWER_SAVE_MODE_VALIDATION.md` for full details.

**Example**: See `examples/power_save_mode_example.cpp` for demonstration.
