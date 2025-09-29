# Satellite Communication Wiring Diagrams

## ESP32-CAM Pinout for Satellite Communication

### Standard Pin Assignments
```
ESP32-CAM Pin    Function                Satellite Module Pin
GPIO 25          Satellite RX            TX (Satellite Module)
GPIO 26          Satellite TX            RX (Satellite Module)  
GPIO 27          Satellite Sleep/Enable  SLEEP/EN
GPIO 33          Ring Indicator          RING (Iridium only)
3.3V             Power Supply            VCC
GND              Ground                  GND
```

## Iridium 9603N Modem Wiring

### Basic Connection
```
ESP32-CAM                    Iridium 9603N
┌─────────────────┐         ┌─────────────────┐
│                 │         │                 │
│ GPIO 25    ─────┼─────────┼─── RX_IN        │
│ GPIO 26    ─────┼─────────┼─── TX_OUT       │
│ GPIO 27    ─────┼─────────┼─── SLEEP        │
│ GPIO 33    ─────┼─────────┼─── RING         │
│ 3.3V       ─────┼─────────┼─── VCC          │
│ GND        ─────┼─────────┼─── GND          │
│                 │         │                 │
└─────────────────┘         └─────────────────┘
```

### Power Supply Considerations
- Iridium requires up to 2.8W during transmission
- Use a 3.3V regulator capable of at least 1A
- Consider power switching circuit for power management

```
Battery/Solar ──► [3.3V Regulator] ──► Iridium Module
                         │
                         └──► ESP32-CAM
```

## RockBLOCK Mk2/Mk3 Wiring

### RockBLOCK Connection (same as Iridium)
```
ESP32-CAM                    RockBLOCK
┌─────────────────┐         ┌─────────────────┐
│                 │         │                 │
│ GPIO 25    ─────┼─────────┼─── RX           │
│ GPIO 26    ─────┼─────────┼─── TX           │
│ GPIO 27    ─────┼─────────┼─── SLEEP        │
│ GPIO 33    ─────┼─────────┼─── RING         │
│ 5V         ─────┼─────────┼─── 5V_IN        │
│ GND        ─────┼─────────┼─── GND          │
│                 │         │                 │
└─────────────────┘         └─────────────────┘
```

Note: RockBLOCK typically requires 5V input with onboard regulation

## Swarm M138 Modem Wiring

### Swarm M138 Connection
```
ESP32-CAM                    Swarm M138
┌─────────────────┐         ┌─────────────────┐
│                 │         │                 │
│ GPIO 25    ─────┼─────────┼─── RX           │
│ GPIO 26    ─────┼─────────┼─── TX           │
│ GPIO 27    ─────┼─────────┼─── ENABLE       │
│ 3.3V       ─────┼─────────┼─── VCC (3.3V)   │
│ GND        ─────┼─────────┼─── GND          │
│                 │         │                 │
└─────────────────┘         └─────────────────┘
```

## Complete System Wiring Diagram

### Wildlife Camera with Satellite Communication
```
                    ┌─────────────────┐
                    │   Solar Panel   │
                    └─────────┬───────┘
                              │
                    ┌─────────▼───────┐
                    │ Charge Controller│
                    │  & Battery Mgmt  │
                    └─────────┬───────┘
                              │
              ┌───────────────┼───────────────┐
              │               │               │
    ┌─────────▼─────┐  ┌─────▼─────┐  ┌─────▼─────┐
    │   ESP32-CAM   │  │ Satellite │  │  Battery  │
    │               │  │  Module   │  │   Pack    │
    │ GPIO25 ──────►├──┤ RX        │  │           │
    │ GPIO26 ◄──────├──┤ TX        │  │           │
    │ GPIO27 ──────►├──┤ SLEEP     │  │           │
    │ GPIO33 ◄──────├──┤ RING      │  │           │
    └───────────────┘  └───────────┘  └───────────┘
            │
    ┌───────▼────────┐
    │  Camera Module │
    │   (OV2640)     │
    └────────────────┘
```

## Antenna Connections

### Iridium/RockBLOCK Antenna
```
Satellite Module         Antenna
┌─────────────┐         ┌─────────────┐
│             │         │             │
│  ANT   ─────┼─────────┼─── SMA      │
│             │         │             │
└─────────────┘         └─────────────┘
```

### Swarm M138 Antenna
```
Swarm Module             Antenna  
┌─────────────┐         ┌─────────────┐
│             │         │             │
│  RF_OUT ────┼─────────┼─── U.FL     │
│             │         │             │
└─────────────┘         └─────────────┘
```

## Power Management Circuit

### Advanced Power Switching
```
                    ┌─────────────────┐
                    │   Solar Panel   │
                    └─────────┬───────┘
                              │
                    ┌─────────▼───────┐
                    │ Charge Controller│
                    └─────────┬───────┘
                              │
                    ┌─────────▼───────┐
                    │    Battery      │
                    └─────────┬───────┘
                              │
                    ┌─────────▼───────┐
                    │   3.3V Reg      │
                    └─────┬───┬───────┘
                          │   │
             ┌────────────┘   └────────────┐
             │                             │
   ┌─────────▼─────┐             ┌─────────▼─────┐
   │   ESP32-CAM   │             │ Satellite Pwr │
   │               │             │    Switch     │
   │ GPIO27 ──────►├─────────────┤ Control       │
   └───────────────┘             └─────────┬─────┘
                                           │
                                 ┌─────────▼─────┐
                                 │ Satellite     │
                                 │ Module        │
                                 └───────────────┘
```

## Enclosure and Environmental Protection

### Weatherproof Enclosure Layout
```
     ┌─────────────────────────────────────┐
     │                                     │ ← Sealed Enclosure
     │  ┌─────────┐    ┌─────────────┐    │
     │  │ESP32-CAM│    │  Satellite  │    │
     │  │         │    │   Module    │    │
     │  └─────────┘    └─────────────┘    │
     │                                     │
     │  ┌─────────────────────────────┐    │
     │  │        Battery Pack         │    │
     │  └─────────────────────────────┘    │
     │                                     │
     └──┬──────────────────────────────┬───┘
        │                              │
   ┌────▼────┐                   ┌────▼────┐
   │Antenna  │                   │ Solar   │
   │ Conn.   │                   │ Panel   │
   └─────────┘                   │ Cable   │
                                 └─────────┘
```

## Cable Specifications

### Signal Cables
- **Serial Communication**: Use shielded twisted pair
- **Maximum Length**: 3 meters recommended
- **Connector Type**: JST-XH or screw terminals

### Power Cables
- **Wire Gauge**: 18 AWG minimum for power lines
- **Voltage Drop**: Calculate for cable length
- **Fusing**: 2A fuse recommended

### Antenna Cables
- **Coaxial Type**: RG-174 or RG-316
- **Maximum Length**: 10 meters (with appropriate connectors)
- **Loss Budget**: Consider signal loss in long cable runs

## Grounding and RF Considerations

### Ground Plane
```
                    Antenna
                       │
              ┌────────▼────────┐
              │  Satellite Mod  │
              └────────┬────────┘
                       │
              ┌────────▼────────┐
              │   ESP32 PCB     │
              │                 │
              │  ┌──────────┐   │
              │  │          │   │ ← Ground plane
              │  │   ESP32  │   │
              │  │          │   │
              │  └──────────┘   │
              └─────────────────┘
```

### EMI Shielding
- Use shielded enclosure for electronics
- Separate power and signal cables
- Use ferrite cores on power cables near RF modules

## Testing and Validation

### Signal Quality Test Points
1. **Power Supply**: Measure voltage stability during transmission
2. **Serial Communication**: Verify UART signals with oscilloscope  
3. **Antenna Connection**: Check SWR and impedance matching
4. **GPS/Timing**: Verify satellite timing signals (if applicable)

### Field Testing Setup
```
Test Equipment          Connection
┌─────────────┐        ┌─────────────┐
│ Oscilloscope├────────┤ UART Lines  │
└─────────────┘        └─────────────┘

┌─────────────┐        ┌─────────────┐
│ Multimeter  ├────────┤ Power Rails │
└─────────────┘        └─────────────┘

┌─────────────┐        ┌─────────────┐
│ SWR Meter   ├────────┤ Antenna     │
└─────────────┘        └─────────────┘
```

## Troubleshooting Common Issues

### Power Problems
- **Symptom**: Module doesn't respond
- **Check**: Power supply voltage and current capacity
- **Solution**: Use adequate power supply and decoupling capacitors

### Communication Issues  
- **Symptom**: No UART response
- **Check**: Baud rate, wiring, signal levels
- **Solution**: Verify UART configuration and connections

### Antenna Issues
- **Symptom**: Poor signal quality
- **Check**: Antenna placement, cable connections, SWR
- **Solution**: Improve antenna positioning and cable integrity

## Safety Considerations

### RF Exposure
- Maintain 20cm minimum distance from antenna during transmission
- Use proper antenna mounting to ensure radiation pattern clearance
- Follow local regulations for RF exposure limits

### Electrical Safety
- Use appropriate fusing and circuit protection
- Ensure proper grounding of all metal enclosures
- Follow electrical codes for outdoor installations