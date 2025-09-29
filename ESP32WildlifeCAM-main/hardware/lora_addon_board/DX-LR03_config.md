# DX-LR03 Module Configuration

## Pinout
| Pin Name  | Function             |
|-----------|----------------------|
| VCC       | Power Supply (3.3V)  |
| GND       | Ground               |
| TX        | Transmit Data        |
| RX        | Receive Data         |
| GPIO0     | GPIO for Control     |

## Specifications
- **Frequency**: 433 MHz / 868 MHz / 915 MHz
- **Modulation**: LoRa (Long Range)
- **Chipset**: Semtech SX1276
- **Range**: Up to 15 km (line of sight)
- **Power Consumption**: Low power mode available

## Code Examples
```cpp
#include <LoRa.h>

void setup() {
  Serial.begin(9600);
  LoRa.begin(433E6);
}

void loop() {
  LoRa.beginPacket();
  LoRa.print("Hello, World!");
  LoRa.endPacket();
  delay(1000);
}
```

## Integration Guide for ESP32-CAM Wildlife Camera Project
1. **Wiring**: Connect the DX-LR03 module to the ESP32-CAM as follows:
   - VCC to ESP32 3.3V
   - GND to ESP32 GND
   - TX to ESP32 RX pin
   - RX to ESP32 TX pin

2. **Programming**: Upload the above code examples to your ESP32-CAM.

3. **Testing**: Ensure the LoRa module is correctly configured and test the transmission.
