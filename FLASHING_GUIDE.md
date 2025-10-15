# WildCAM ESP32 Flashing Guide

## Supported Boards
- AI-Thinker ESP32-CAM
- ESP32-S3-CAM
- TTGO T-Camera

## Prerequisites
- Install [esptool.py](https://github.com/espressif/esptool):
  ```bash
  pip install esptool
  ```
- USB cable connected to ESP32
- Drivers installed for USB-to-Serial chip

## Flashing Instructions

### Windows
```powershell
esptool.py --chip esp32 --port COM3 --baud 460800 ^
  --before default_reset --after hard_reset write_flash ^
  -z --flash_mode dio --flash_freq 40m --flash_size detect ^
  0x1000 bootloader.bin ^
  0x8000 partitions.bin ^
  0x10000 wildcam_<board>_<version>.bin
```

### Linux/Mac
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  -z --flash_mode dio --flash_freq 40m --flash_size detect \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 wildcam_<board>_<version>.bin
```

## Troubleshooting
- If upload fails, hold BOOT button while connecting USB
- Check correct COM port: `esptool.py --list-ports`
- Try lower baud rate: change 460800 to 115200
- Ensure no other program is using the serial port

## After Flashing
- Press RESET button on ESP32
- Open serial monitor at 115200 baud
- You should see startup messages
- Insert formatted SD card (FAT32)
- Configure WiFi in `config_local.h` if needed

## First Time Setup
1. Insert formatted SD card (FAT32, 32GB recommended)
2. Connect PIR sensor to GPIO 13
3. Power on and check serial output (115200 baud)
4. Configure WiFi credentials if needed

## Documentation
See [README.md](README.md) for complete setup guide.

## Support
Open an issue if you encounter problems.
