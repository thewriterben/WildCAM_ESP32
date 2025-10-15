#!/bin/bash
set -e
cd ESP32WildlifeCAM-main/WildCAM_ESP32
for board in esp32cam esp32s3cam ttgo-t-camera; do
  echo "Building for $board..."
  pio run -e $board
  mkdir -p release/$board
  cp .pio/build/$board/firmware.bin release/$board/wildcam_${board}_localtest.bin
  cp .pio/build/$board/bootloader.bin release/$board/bootloader.bin 2>/dev/null || true
  cp .pio/build/$board/partitions.bin release/$board/partitions.bin 2>/dev/null || true
  cat > release/$board/build_info.txt <<EOF
WildCAM ESP32 Firmware
Board: $board
Version: localtest
Build Date: $(date -u +"%Y-%m-%d %H:%M:%S UTC")
Commit: $(git rev-parse HEAD)
EOF
  cat > release/$board/FLASH_INSTRUCTIONS.txt <<'EOF'
WildCAM ESP32 Flashing Instructions
====================================
Prerequisites:
- esptool.py installed: pip install esptool
- USB cable connected to ESP32
- Drivers installed for USB-to-Serial chip
Windows:
--------
esptool.py --chip esp32 --port COM3 --baud 460800 ^
  --before default_reset --after hard_reset write_flash ^
  -z --flash_mode dio --flash_freq 40m --flash_size detect ^
  0x1000 bootloader.bin ^
  0x8000 partitions.bin ^
  0x10000 wildcam_${board}_localtest.bin
Linux/Mac:
----------
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 \
  --before default_reset --after hard_reset write_flash \
  -z --flash_mode dio --flash_freq 40m --flash_size detect \
  0x1000 bootloader.bin \
  0x8000 partitions.bin \
  0x10000 wildcam_${board}_localtest.bin
Troubleshooting:
----------------
- If upload fails, hold BOOT button while connecting USB
- Check correct COM port: "esptool.py --list-ports"
- Try lower baud rate: change 460800 to 115200
- Ensure no other program is using the serial port
After Flashing:
---------------
- Press RESET button on ESP32
- Open serial monitor at 115200 baud
- You should see startup messages
- Insert formatted SD card (FAT32)
- Configure WiFi in config_local.h if needed
EOF
done
